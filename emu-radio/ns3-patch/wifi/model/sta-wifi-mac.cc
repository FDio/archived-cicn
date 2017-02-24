/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 */

#include "sta-wifi-mac.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "qos-tag.h"
#include "mac-low.h"
#include "dcf-manager.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "wifi-mac-header.h"
#include "msdu-aggregator.h"
#include "amsdu-subframe-header.h"
#include "mgt-headers.h"
#include "ht-capabilities.h"
#include "vht-capabilities.h"

//added:+++++
#include "yans-wifi-phy.h"
#include "ap-info-collection.h"
//for removing packets for old AP
#include "wifi-mac-queue.h"


#define MAX_NUM_PROBEREQ 4
#define HYSTERESIS_THRESHOLD 4
//end++++++++++

/*
 * The state machine for this STA is:
 --------------                                          -----------
 | Associated |   <--------------------      ------->    | Refused |
 --------------                        \    /            -----------
    \                                   \  /
     \    -----------------     -----------------------------
      \-> | Beacon Missed | --> | Wait Association Response |
          -----------------     -----------------------------
                \                       ^
                 \                      |
                  \    -----------------------
                   \-> | Wait Probe Response |
                       -----------------------
 */

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StaWifiMac");

NS_OBJECT_ENSURE_REGISTERED (StaWifiMac);

TypeId
StaWifiMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StaWifiMac")
    .SetParent<RegularWifiMac> ()
    .SetGroupName ("Wifi")
    .AddConstructor<StaWifiMac> ()
    .AddAttribute ("ProbeRequestTimeout", "The interval between two consecutive probe request attempts.",
                   TimeValue (Seconds (0.05)),
                   MakeTimeAccessor (&StaWifiMac::m_probeRequestTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("AssocRequestTimeout", "The interval between two consecutive assoc request attempts.",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&StaWifiMac::m_assocRequestTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("MaxMissedBeacons",
                   "Number of beacons which much be consecutively missed before "
                   "we attempt to restart association.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&StaWifiMac::m_maxMissedBeacons),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ActiveProbing",
                   "If true, we send probe requests. If false, we don't."
                   "NOTE: if more than one STA in your simulation is using active probing, "
                   "you should enable it at a different simulation time for each STA, "
                   "otherwise all the STAs will start sending probes at the same time resulting in collisions. "
                   "See bug 1060 for more info.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&StaWifiMac::SetActiveProbing, &StaWifiMac::GetActiveProbing),
                   MakeBooleanChecker ())
    .AddTraceSource ("Assoc", "Associated with an access point.",
                     MakeTraceSourceAccessor (&StaWifiMac::m_assocLogger),
                     "ns3::Mac48Address::TracedCallback")
    .AddTraceSource ("DeAssoc", "Association with an access point lost.",
                     MakeTraceSourceAccessor (&StaWifiMac::m_deAssocLogger),
                     "ns3::Mac48Address::TracedCallback")
  ;
  return tid;
}

StaWifiMac::StaWifiMac ()
  : m_state (BEACON_MISSED),
    m_probeRequestEvent (),
    m_assocRequestEvent (),
    m_beaconWatchdogEnd (Seconds (0.0))
    ,m_isSelectingAP(false)
    ,m_currentRssi(0.0)
    ,m_packetCounts(0)
    ,m_probeRequestCount(0)
    ,m_isEverAssoicated(true)
{
  NS_LOG_FUNCTION (this);

  //Let the lower layers know that we are acting as a non-AP STA in
  //an infrastructure BSS.
  SetTypeOfStation (STA);
}

StaWifiMac::~StaWifiMac ()
{
  NS_LOG_FUNCTION (this);
}

void
StaWifiMac::SetMaxMissedBeacons (uint32_t missed)
{
  NS_LOG_FUNCTION (this << missed);
  m_maxMissedBeacons = missed;
}

void
StaWifiMac::SetProbeRequestTimeout (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);
  m_probeRequestTimeout = timeout;
}

void
StaWifiMac::SetAssocRequestTimeout (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);
  m_assocRequestTimeout = timeout;
}

void
StaWifiMac::StartActiveAssociation (void)
{
  NS_LOG_FUNCTION (this);
  TryToEnsureAssociated ();
}

void
StaWifiMac::SetActiveProbing (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  if (enable)
    {
      Simulator::ScheduleNow (&StaWifiMac::TryToEnsureAssociated, this);
    }
  else
    {
      m_probeRequestEvent.Cancel ();
    }
  m_activeProbing = enable;
}

bool StaWifiMac::GetActiveProbing (void) const
{
  return m_activeProbing;
}

void
StaWifiMac::SendProbeRequest (void)
{
//       std::cout<<Simulator::Now().ToDouble(Time::S)<<", send probe request="<<m_probeRequestCount <<"\n";

  NS_LOG_FUNCTION (this);
  WifiMacHeader hdr;
  hdr.SetProbeReq ();
  hdr.SetAddr1 (Mac48Address::GetBroadcast ());
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (Mac48Address::GetBroadcast ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();
  Ptr<Packet> packet = Create<Packet> ();
  MgtProbeRequestHeader probe;
  probe.SetSsid (GetSsid ());
  probe.SetSupportedRates (GetSupportedRates ());
  if (m_htSupported || m_vhtSupported)
    {
      probe.SetHtCapabilities (GetHtCapabilities ());
      hdr.SetNoOrder ();
    }
  if (m_vhtSupported)
    {
      probe.SetVhtCapabilities (GetVhtCapabilities ());
    }
  packet->AddHeader (probe);

  //The standard is not clear on the correct queue for management
  //frames if we are a QoS AP. The approach taken here is to always
  //use the DCF for these regardless of whether we have a QoS
  //association or not.
  m_dca->Queue (packet, hdr);


}

void
StaWifiMac::SendAssociationRequest (void)
{
//        std::cout<<Simulator::Now().ToDouble(Time::S)<<", send assoc request to="<<GetBssid ()<<"by="<<GetAddress ()<<"\n";

  NS_LOG_FUNCTION (this << GetBssid ());
  WifiMacHeader hdr;
  hdr.SetAssocReq ();
  hdr.SetAddr1 (GetBssid ());
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (GetBssid ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();
  Ptr<Packet> packet = Create<Packet> ();
  MgtAssocRequestHeader assoc;
  assoc.SetSsid (GetSsid ());
  assoc.SetSupportedRates (GetSupportedRates ());
  if (m_htSupported || m_vhtSupported)
    {
      assoc.SetHtCapabilities (GetHtCapabilities ());
      hdr.SetNoOrder ();
    }
  if (m_vhtSupported)
    {
      assoc.SetVhtCapabilities (GetVhtCapabilities ());
    }
  packet->AddHeader (assoc);

  //The standard is not clear on the correct queue for management
  //frames if we are a QoS AP. The approach taken here is to always
  //use the DCF for these regardless of whether we have a QoS
  //association or not.
  m_dca->Queue (packet, hdr);

  if (m_assocRequestEvent.IsRunning ())
    {
      m_assocRequestEvent.Cancel ();
    }
  m_assocRequestEvent = Simulator::Schedule (m_assocRequestTimeout,
                                             &StaWifiMac::AssocRequestTimeout, this);
}

void
StaWifiMac::sendDissassociationRequest (void)
{
//        std::cout<<Simulator::Now().ToDouble(Time::S)<<", send assoc request to="<<GetBssid ()<<"by="<<GetAddress ()<<"\n";

  NS_LOG_FUNCTION (this << GetBssid ());
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_MGT_DISASSOCIATION);
  hdr.SetAddr1 (GetBssid ());
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (GetBssid ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();
  Ptr<Packet> packet = Create<Packet> ();
 
  //The standard is not clear on the correct queue for management
  //frames if we are a QoS AP. The approach taken here is to always
  //use the DCF for these regardless of whether we have a QoS
  //association or not.
  m_dca->Queue (packet, hdr);


}

void
StaWifiMac::TryToEnsureAssociated (void)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
    {
    case ASSOCIATED:
      return;
      break;
    case WAIT_PROBE_RESP:
      /* we have sent a probe request earlier so we
         do not need to re-send a probe request immediately.
         We just need to wait until probe-request-timeout
         or until we get a probe response
       */
      break;
    case BEACON_MISSED:
      /* we were associated but we missed a bunch of beacons
       * so we should assume we are not associated anymore.
       * We try to initiate a probe request now.
       */
      m_linkDown ();
      if (m_activeProbing)
        {
          SetState (WAIT_PROBE_RESP);
	  if (m_probeRequestEvent.IsRunning ())
	    {
	      m_probeRequestEvent.Cancel ();
	    }
	  m_probeRequestEvent = Simulator::Schedule (m_probeRequestTimeout,
                                          &StaWifiMac::ProbeRequestTimeout, this);
	  if(m_isEverAssoicated)
	  {
	     sendBurstOfProbeRequest ();
	  }
	  else
	  {
	    SendProbeRequest();
	  }
        }
      break;
    case WAIT_ASSOC_RESP:
      /* we have sent an assoc request so we do not need to
         re-send an assoc request right now. We just need to
         wait until either assoc-request-timeout or until
         we get an assoc response.
       */
      break;
    case REFUSED:
      /* we have sent an assoc request and received a negative
         assoc resp. We wait until someone restarts an
         association with a given ssid.
       */
      break;
    }
}

void
StaWifiMac::AssocRequestTimeout (void)
{
  NS_LOG_FUNCTION (this);
  //SetState (WAIT_ASSOC_RESP);
  //std::cout<<Simulator::Now().ToDouble(Time::S)<<", assoc request timed out" <<"\n";
  //SendAssociationRequest ();
  
  SetState(BEACON_MISSED);
  TryToEnsureAssociated();
}

void
StaWifiMac::ProbeRequestTimeout (void)
{
  NS_LOG_FUNCTION (this);
 

  if (m_probeRequestBurstEvent.IsRunning ())
     {
       m_probeRequestBurstEvent.Cancel ();
     }
  m_probeRequestCount=0;
   
  if(!m_isSelectingAP)//no response has been received before timeout
  {
    SetState (WAIT_PROBE_RESP);
    if (m_probeRequestEvent.IsRunning ())
     {
       m_probeRequestEvent.Cancel ();
     }
    m_probeRequestEvent = Simulator::Schedule (m_probeRequestTimeout,
                                             &StaWifiMac::ProbeRequestTimeout, this);
    sendBurstOfProbeRequest ();
  }
  else
  {

      //destroy all block ack agreement with old AP (by pretending to receive a delba frame from AP).
  //clean up packets for the old AP in the meanwhile
    cleanUpPacketsAndAgreementsWithOldAp();
   
    SetState (WAIT_ASSOC_RESP);
    double candidateRssi=-1.0;
    SupportedRates rates;
    
    ApInfoCollection::iterator it;
        for(it=m_apInfos.begin();it!=m_apInfos.end();it++)
           {
             if(it->getAverageRssi()>candidateRssi)
               {
                 candidateRssi=it->getAverageRssi();
                 m_delayFromProbResp=it->getDelayFromProbResp();
                 SetBssid(it->getBssid());
                 rates=it->getSupportedRates();
               }
           }
           
           //++++++++++++
          //remove obsolete remote station states if exists, before reassoicate to the station
         // m_stationManager->RemoveStation(GetBssid());   
          //++++++++++++++ 
	  
	loadSupportedRatesOfAp(rates,GetBssid());
    //RestartBeaconWatchdog (delayFromProbResp);
    SendAssociationRequest ();
  }
  
}

void
StaWifiMac::MissedBeacons (void)
{
  NS_LOG_FUNCTION (this);
  if (m_beaconWatchdogEnd > Simulator::Now ())
    {
      if (m_beaconWatchdog.IsRunning ())
        {
          m_beaconWatchdog.Cancel ();
        }
      m_beaconWatchdog = Simulator::Schedule (m_beaconWatchdogEnd - Simulator::Now (),
                                              &StaWifiMac::MissedBeacons, this);
      return;
    }
  NS_LOG_DEBUG ("beacon missed");
//  std::cout<<Simulator::Now().ToDouble(Time::S)<<", beacon missed"<<"\n";


#ifdef WITH_HYSTERESIS_HANDOVER_TRIGGER
  m_currentApRrssiMeasures.clear();
  m_candidadteApRssiMeasures.clear();
#endif
  SetState (BEACON_MISSED);
  //disassociate from old AP
  sendDissassociationRequest();
  TryToEnsureAssociated ();
}

void
StaWifiMac::RestartBeaconWatchdog (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_beaconWatchdogEnd = std::max (Simulator::Now () + delay, m_beaconWatchdogEnd);
  if (Simulator::GetDelayLeft (m_beaconWatchdog) < delay
      && m_beaconWatchdog.IsExpired ())
    {
      NS_LOG_DEBUG ("really restart watchdog.");
      m_beaconWatchdog = Simulator::Schedule (delay, &StaWifiMac::MissedBeacons, this);
    }
}

bool
StaWifiMac::IsAssociated (void) const
{
  return m_state == ASSOCIATED;
}

bool
StaWifiMac::IsWaitAssocResp (void) const
{
  return m_state == WAIT_ASSOC_RESP;
}

void
StaWifiMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);
  if (!IsAssociated ())
    {
      NotifyTxDrop (packet);
      TryToEnsureAssociated ();
      return;
    }
  WifiMacHeader hdr;

  //If we are not a QoS AP then we definitely want to use AC_BE to
  //transmit the packet. A TID of zero will map to AC_BE (through \c
  //QosUtilsMapTidToAc()), so we use that as our default here.
  uint8_t tid = 0;

  //For now, an AP that supports QoS does not support non-QoS
  //associations, and vice versa. In future the AP model should
  //support simultaneously associated QoS and non-QoS STAs, at which
  //point there will need to be per-association QoS state maintained
  //by the association state machine, and consulted here.
  if (m_qosSupported)
    {
      hdr.SetType (WIFI_MAC_QOSDATA);
      hdr.SetQosAckPolicy (WifiMacHeader::NORMAL_ACK);
      hdr.SetQosNoEosp ();
      hdr.SetQosNoAmsdu ();
      //Transmission of multiple frames in the same TXOP is not
      //supported for now
      hdr.SetQosTxopLimit (0);

      //Fill in the QoS control field in the MAC header
      tid = QosUtilsGetTidForPacket (packet);
      //Any value greater than 7 is invalid and likely indicates that
      //the packet had no QoS tag, so we revert to zero, which'll
      //mean that AC_BE is used.
      if (tid > 7)
        {
          tid = 0;
        }
      hdr.SetQosTid (tid);
    }
  else
    {
      hdr.SetTypeData ();
    }
  if (m_htSupported || m_vhtSupported)
    {
      hdr.SetNoOrder ();
    }

  hdr.SetAddr1 (GetBssid ());
  hdr.SetAddr2 (m_low->GetAddress ());
  hdr.SetAddr3 (to);
  hdr.SetDsNotFrom ();
  hdr.SetDsTo ();

  if (m_qosSupported)
    {
      //Sanity check that the TID is valid
      NS_ASSERT (tid < 8);
      m_edca[QosUtilsMapTidToAc (tid)]->Queue (packet, hdr);
    }
  else
    {
      m_dca->Queue (packet, hdr);
    }
}

void
StaWifiMac::Receive (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION (this << packet << hdr);
  NS_ASSERT (!hdr->IsCtl ());
  if (hdr->GetAddr3 () == GetAddress ())
    {
      NS_LOG_LOGIC ("packet sent by us.");
      return;
    }
  else if (hdr->GetAddr1 () != GetAddress ()
           && !hdr->GetAddr1 ().IsGroup ())
    {
      NS_LOG_LOGIC ("packet is not for us");
      NotifyRxDrop (packet);
      return;
    }
  else if (hdr->IsData ())
    {
      if (!IsAssociated ())
        {
          NS_LOG_LOGIC ("Received data frame while not associated: ignore");
          NotifyRxDrop (packet);
          return;
        }
      if (!(hdr->IsFromDs () && !hdr->IsToDs ()))
        {
          NS_LOG_LOGIC ("Received data frame not from the DS: ignore");
          NotifyRxDrop (packet);
          return;
        }
      if (hdr->GetAddr2 () != GetBssid ())
        {
          NS_LOG_LOGIC ("Received data frame not from the BSS we are associated with: ignore");
          NotifyRxDrop (packet);
          return;
        }
//here the rssi could be measured +++++++++++++++++++++++
//end+++++++++++++++++++
      if (hdr->IsQosData ())
        {
          if (hdr->IsQosAmsdu ())
            {
              NS_ASSERT (hdr->GetAddr3 () == GetBssid ());
              DeaggregateAmsduAndForward (packet, hdr);
              packet = 0;
            }
          else
            {
              ForwardUp (packet, hdr->GetAddr3 (), hdr->GetAddr1 ());
            }
        }
      else
        {
          ForwardUp (packet, hdr->GetAddr3 (), hdr->GetAddr1 ());
        }
      return;
    }
  else if (hdr->IsProbeReq ()
           || hdr->IsAssocReq ())
    {
      //This is a frame aimed at an AP, so we can safely ignore it.
      NotifyRxDrop (packet);
      return;
    }
  else if (hdr->IsBeacon ())
    {
      MgtBeaconHeader beacon;
      packet->RemoveHeader (beacon);
      bool goodBeacon = false;
      if (GetSsid ().IsBroadcast ()
          || beacon.GetSsid ().IsEqual (GetSsid ()))
        {
          goodBeacon = true;
        }
      SupportedRates rates = beacon.GetSupportedRates ();
      for (uint32_t i = 0; i < m_phy->GetNBssMembershipSelectors (); i++)
        {
          uint32_t selector = m_phy->GetBssMembershipSelector (i);
          if (!rates.IsSupportedRate (selector))
            {
              goodBeacon = false;
            }
        }

#ifdef WITH_HYSTERESIS_HANDOVER_TRIGGER
      ///background scanning, measuring current or candidate AP beacon's rssi in the same channel
      if(goodBeacon && IsAssociated ())
      {
	 RssiTag t;
	 double rssi;
	 if (packet->PeekPacketTag (t))
	 {
	   rssi=t.Get();	   
	 }
	 else
	 {
	   std::cout<<"ERROR:cannot find Rssi for beacon\n";
	   exit(1);
	 }
	if(hdr->GetAddr3 ()==GetBssid ()) //beacon from current AP
	{
	  m_currentApRrssiMeasures.setBssid(GetBssid ());
	  m_currentApRrssiMeasures.addRssi(rssi);
	}
	else//beacon from a candidate AP
	{
	  RssiMeasureInfoCollection::iterator it;
          for(it=m_candidadteApRssiMeasures.begin();it!=m_candidadteApRssiMeasures.end();it++)
	  {
	    if(it->getBssid()==hdr->GetAddr3 ()) //find a record for this AP, old candidate AP
	      break;	
	  }
	  if(it==m_candidadteApRssiMeasures.end()) // new candidate AP
	    {
	      RssiMeasureInfo newApRssiMeasure(hdr->GetAddr3 ());
	      newApRssiMeasure.addRssi(rssi);
	      m_candidadteApRssiMeasures.push_back(newApRssiMeasure);	      
	    }
	  else//old AP
	  {
	    it->addRssi(rssi);//ad rssi sample
	    double avgRssi=it->getAverageRssi();
	    double currentApavgRssi=m_currentApRrssiMeasures.getAverageRssi();
	    if(avgRssi!=-1 && currentApavgRssi!=-1 && avgRssi/currentApavgRssi >HYSTERESIS_THRESHOLD)//check if we have at least 4 samples
	    {
	      m_currentApRrssiMeasures.clear();
	      m_candidadteApRssiMeasures.clear();
	      m_beaconWatchdog.Cancel();
	      //std::cout<<"NOTE: handover is triggered by Hysteresis algorithm\n";

// 	      SetState(BEACON_MISSED);
//  	      TryToEnsureAssociated();
	      
	      //swith to the ap we find:
	       cleanUpPacketsAndAgreementsWithOldAp();
	       
	       //disassociate with old AP
	       sendDissassociationRequest();
	       
	       //switch:
	       SetState (WAIT_ASSOC_RESP);
	       SetBssid(hdr->GetAddr3 ());
	       loadSupportedRatesOfAp(rates,GetBssid());
	       m_sendAssocReqEvent=Simulator::Schedule (Time("0.024s"),
                                             &StaWifiMac::SendAssociationRequest, this);
	      return;
	    }
	  }
	}
      }
#endif
      if ((IsWaitAssocResp () || IsAssociated ()) && hdr->GetAddr3 () != GetBssid ())
        {
          goodBeacon = false;
        }
      if(m_activeProbing)
	{

	  if (goodBeacon && IsAssociated ())
	  {
//here the rssi could be measured +++++++++++++++++++++++
//end+++++++++++++++++++
          Time delay = MicroSeconds (beacon.GetBeaconIntervalUs () * m_maxMissedBeacons);
          RestartBeaconWatchdog (delay);
          SetBssid (hdr->GetAddr3 ());
//	  std::cout<<Simulator::Now().ToDouble(Time::S)<<", received beacon, restart watchdog" <<"\n";    
	  }
	}
       else
	{
	  if (goodBeacon)
	  {
	    Time delay = MicroSeconds (beacon.GetBeaconIntervalUs () * m_maxMissedBeacons);
	    RestartBeaconWatchdog (delay);
	    SetBssid (hdr->GetAddr3 ());	    
	  }
	  if (goodBeacon && m_state == BEACON_MISSED)
	  {
	    SetState (WAIT_ASSOC_RESP);
	    SendAssociationRequest ();
	  }
	}

      return;
    }
  else if (hdr->IsProbeResp ())
    {
      if (m_state == WAIT_PROBE_RESP)
        {
          MgtProbeResponseHeader probeResp;
          packet->RemoveHeader (probeResp);
          if (!probeResp.GetSsid ().IsEqual (GetSsid ()))
            {
              //not a probe resp for our ssid.
              return;
            }
          SupportedRates rates = probeResp.GetSupportedRates ();
          for (uint32_t i = 0; i < m_phy->GetNBssMembershipSelectors (); i++)
            {
              uint32_t selector = m_phy->GetBssMembershipSelector (i);
              if (!rates.IsSupportedRate (selector))
                {
                  return;
                }
            }
//added++++++++++++++++++++++++++

         if(!m_isSelectingAP)
	       {
             m_isSelectingAP=true;
           }
           
            //std::cout<<Simulator::Now().ToDouble(Time::S)<<", received probe response from="<<hdr->GetAddr3 () <<"\n";

	      RssiTag t;
	  
	      if (packet->PeekPacketTag (t))
	      {
		double rssi=t.Get();
		Time delayFromProbResp=MicroSeconds (probeResp.GetBeaconIntervalUs () * m_maxMissedBeacons);
		Mac48Address bssid=hdr->GetAddr3 ();
        ApInfoCollection::iterator it;
        for(it=m_apInfos.begin();it!=m_apInfos.end();it++)
           {
             if(it->getBssid()==bssid)
               break;
           }
        if(it!=m_apInfos.end())	
           {
             it->addRssi(rssi);
           }
         else
           {
              ApInfo newApInfo(bssid,delayFromProbResp,rssi,rates);
              m_apInfos.push_back(newApInfo);
           }
	      }
	      else
	      {
		std::cout<<"cannot find RSSi TAG for probe response\n";
		exit(1);		
	      } 
	      
          //---------------end-------------------------  

        }
      return;
    }
  else if (hdr->IsAssocResp ())
    {
      if (m_state == WAIT_ASSOC_RESP)
        {
          MgtAssocResponseHeader assocResp;
          packet->RemoveHeader (assocResp);
          if (m_assocRequestEvent.IsRunning ())
            {
              m_assocRequestEvent.Cancel ();
            }
#ifdef WITH_HYSTERESIS_HANDOVER_TRIGGER
          if(m_sendAssocReqEvent.IsRunning ())
            {
              m_sendAssocReqEvent.Cancel ();
            }
#endif
          if (assocResp.GetStatusCode ().IsSuccess ())
            {
              SetState (ASSOCIATED);
	      //added++++++++
if(m_activeProbing)
	{
	      m_isSelectingAP=false;
	      
	      
	      m_currentRssi=0;
	      m_packetCounts=0;
              
	      
	      //ap info becomes unuseful:
              m_apInfos.clear();
	      if(!m_isEverAssoicated)
		m_isEverAssoicated=true;
	      
	      RestartBeaconWatchdog (m_delayFromProbResp);
// 	      std::cout<<Simulator::Now().ToDouble(Time::S)<<", assoc completed"<<m_probeRequestCount <<"\n";
	}
	      //+++end+++++++
              NS_LOG_DEBUG ("assoc completed");
              SupportedRates rates = assocResp.GetSupportedRates ();
              if (m_htSupported)
                {
                  HtCapabilities htcapabilities = assocResp.GetHtCapabilities ();
                  m_stationManager->AddStationHtCapabilities (hdr->GetAddr2 (),htcapabilities);
                }
              if (m_vhtSupported)
                {
                  VhtCapabilities vhtcapabilities = assocResp.GetVhtCapabilities ();
                  m_stationManager->AddStationVhtCapabilities (hdr->GetAddr2 (), vhtcapabilities);
                }

              for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
                {
                  WifiMode mode = m_phy->GetMode (i);
                  if (rates.IsSupportedRate (mode.GetDataRate (m_phy->GetChannelWidth (), false, 1)))
                    {
                      m_stationManager->AddSupportedMode (hdr->GetAddr2 (), mode);
                      if (rates.IsBasicRate (mode.GetDataRate (m_phy->GetChannelWidth (), false, 1)))
                        {
                          m_stationManager->AddBasicMode (mode);
                        }
                    }
                }
              if (m_htSupported)
                {
                  HtCapabilities htcapabilities = assocResp.GetHtCapabilities ();
                  for (uint32_t i = 0; i < m_phy->GetNMcs (); i++)
                    {
                      WifiMode mcs = m_phy->GetMcs (i);
                      if (mcs.GetModulationClass () == WIFI_MOD_CLASS_HT && htcapabilities.IsSupportedMcs (mcs.GetMcsValue ()))
                        {
                          m_stationManager->AddSupportedMcs (hdr->GetAddr2 (), mcs);
                          //here should add a control to add basic MCS when it is implemented
                        }
                    }
                }
              if (m_vhtSupported)
                {
                  VhtCapabilities vhtcapabilities = assocResp.GetVhtCapabilities ();
                  for (uint32_t i = 0; i < m_phy->GetNMcs (); i++)
                    {
                      WifiMode mcs = m_phy->GetMcs (i);
                      if (mcs.GetModulationClass () == WIFI_MOD_CLASS_VHT && vhtcapabilities.IsSupportedTxMcs (mcs.GetMcsValue ()))
                        {
                          m_stationManager->AddSupportedMcs (hdr->GetAddr2 (), mcs);
                          //here should add a control to add basic MCS when it is implemented
                        }
                    }
                }
              if (!m_linkUp.IsNull ())
                {
                  m_linkUp ();
                }
            }
          else
            {
              NS_LOG_DEBUG ("assoc refused");
              SetState (REFUSED);
            }
        }

      return;
    }

  //Invoke the receive handler of our parent class to deal with any
  //other frames. Specifically, this will handle Block Ack-related
  //Management Action frames.
  RegularWifiMac::Receive (packet, hdr);
}

SupportedRates
StaWifiMac::GetSupportedRates (void) const
{
  SupportedRates rates;
  if (m_htSupported || m_vhtSupported)
    {
      for (uint32_t i = 0; i < m_phy->GetNBssMembershipSelectors (); i++)
        {
          rates.SetBasicRate (m_phy->GetBssMembershipSelector (i));
        }
    }
  for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
    {
      WifiMode mode = m_phy->GetMode (i);
      rates.AddSupportedRate (mode.GetDataRate (m_phy->GetChannelWidth (), false, 1));
    }
  return rates;
}

HtCapabilities
StaWifiMac::GetHtCapabilities (void) const
{
  HtCapabilities capabilities;
  capabilities.SetHtSupported (1);
  if (m_htSupported)
    {
      capabilities.SetLdpc (m_phy->GetLdpc ());
      capabilities.SetSupportedChannelWidth (m_phy->GetChannelWidth () == 40);
      capabilities.SetShortGuardInterval20 (m_phy->GetGuardInterval ());
      capabilities.SetShortGuardInterval40 (m_phy->GetChannelWidth () == 40 && m_phy->GetGuardInterval ());
      capabilities.SetGreenfield (m_phy->GetGreenfield ());
      capabilities.SetMaxAmsduLength (1); //hardcoded for now (TBD)
      capabilities.SetLSigProtectionSupport (!m_phy->GetGreenfield ());
      capabilities.SetMaxAmpduLength (3); //hardcoded for now (TBD)
      uint64_t maxSupportedRate = 0; //in bit/s
      for (uint8_t i = 0; i < m_phy->GetNMcs (); i++)
        {
          WifiMode mcs = m_phy->GetMcs (i);
          capabilities.SetRxMcsBitmask (mcs.GetMcsValue ());
          if (mcs.GetDataRate (m_phy->GetGuardInterval (), m_phy->GetGuardInterval (), 1) > maxSupportedRate)
            {
              maxSupportedRate = mcs.GetDataRate (m_phy->GetGuardInterval (), m_phy->GetGuardInterval (), 1);
            }
        }
      capabilities.SetRxHighestSupportedDataRate (maxSupportedRate / 1e6); //in Mbit/s
      capabilities.SetTxMcsSetDefined (m_phy->GetNMcs () > 0);
      capabilities.SetTxMaxNSpatialStreams (m_phy->GetNumberOfTransmitAntennas ());
    }
  return capabilities;
}

VhtCapabilities
StaWifiMac::GetVhtCapabilities (void) const
{
  VhtCapabilities capabilities;
  capabilities.SetVhtSupported (1);
  if (m_vhtSupported)
    {
      if (m_phy->GetChannelWidth () == 160)
        {
          capabilities.SetSupportedChannelWidthSet (1);
        }
      else
        {
          capabilities.SetSupportedChannelWidthSet (0);
        }
      capabilities.SetMaxMpduLength (2); //hardcoded for now (TBD)
      capabilities.SetRxLdpc (m_phy->GetLdpc ());
      capabilities.SetShortGuardIntervalFor80Mhz ((m_phy->GetChannelWidth () == 80) && m_phy->GetGuardInterval ());
      capabilities.SetShortGuardIntervalFor160Mhz ((m_phy->GetChannelWidth () == 160) && m_phy->GetGuardInterval ());
      capabilities.SetMaxAmpduLengthExponent (7); //hardcoded for now (TBD)
      uint8_t maxMcs = 0;
      for (uint8_t i = 0; i < m_phy->GetNMcs (); i++)
        {
          WifiMode mcs = m_phy->GetMcs (i);
          if (mcs.GetMcsValue () > maxMcs)
            {
              maxMcs = mcs.GetMcsValue ();
            }
        }
      capabilities.SetRxMcsMap (maxMcs, 1); //Only 1 SS is currently supported
      capabilities.SetTxMcsMap (maxMcs, 1); //Only 1 SS is currently supported
    }
  return capabilities;
}

void
StaWifiMac::SetState (MacState value)
{
  if (value == ASSOCIATED
      && m_state != ASSOCIATED)
    {
      m_assocLogger (GetBssid ());
    }
  else if (value != ASSOCIATED
           && m_state == ASSOCIATED)
    {
      m_deAssocLogger (GetBssid ());
    }
  m_state = value;
}

void
StaWifiMac::sendBurstOfProbeRequest()
{
  SendProbeRequest();
  m_probeRequestCount++;
  if(m_probeRequestCount >= MAX_NUM_PROBEREQ)
    return;
  else
    m_probeRequestBurstEvent=Simulator::Schedule ((m_probeRequestTimeout-Time("0.02s"))/(MAX_NUM_PROBEREQ-1),
                                             &StaWifiMac::sendBurstOfProbeRequest, this);
}

void 
StaWifiMac::loadSupportedRatesOfAp(SupportedRates rates, Mac48Address bssid)
{
  for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
            {
              WifiMode mode = m_phy->GetMode (i);
              if (rates.IsSupportedRate (mode.GetDataRate (m_phy->GetChannelWidth (), false, 1)))
                {
                  m_stationManager->AddSupportedMode (bssid, mode);
                  if (rates.IsBasicRate (mode.GetDataRate (m_phy->GetChannelWidth (), false, 1)))
                    {
                      m_stationManager->AddBasicMode (mode);
                    }
                }
            }
}

void 
StaWifiMac::cleanUpPacketsAndAgreementsWithOldAp()
{
  
      //destroy all block ack agreement with old AP (by pretending to receive a delba frame from AP).
  //clean up packets for the old AP in the meanwhile
  for(uint8_t tid=0; tid<=7;tid++)
  {
    MgtDelBaHeader delbaHdr;
    delbaHdr.SetTid (tid);
    AcIndex ac = QosUtilsMapTidToAc (tid);
     m_edca[ac]->GotDelBaFrame(&delbaHdr, GetBssid());
//     //also destroy aggreement at mac low immediately:
     m_low->DestroyBlockAckAgreement (GetBssid(), tid);
        
     WifiMacHeader dequeuedHdr;
    
     //for edca:
    Ptr<WifiMacQueue > edcaQueue=m_edca[ac]->GetEdcaQueue ();

     Ptr<const Packet> dequeuedPacket = edcaQueue->DequeueByTidAndAddress (&dequeuedHdr, tid,
                                                                             WifiMacHeader::ADDR1,
                                                                             GetBssid());
//     int count=0;
     while (dequeuedPacket != 0)
                {
// 		  count++;
                  dequeuedPacket = edcaQueue->DequeueByTidAndAddress (&dequeuedHdr, tid,
                                                                             WifiMacHeader::ADDR1,
                                                                             GetBssid());
                }
    
//    if(count!=0)
//      std::cout<<"EDCA "<<count<<"\n";
    
//    count=0;
   // for dca
    Ptr<WifiMacQueue > dcaQueue=m_dca->GetQueue ();
    dequeuedPacket = dcaQueue->DequeueByTidAndAddress (&dequeuedHdr, tid,
                                                                             WifiMacHeader::ADDR1,
                                                                             GetBssid());
     
    while (dequeuedPacket != 0)
                {
// 		  count++;
                  dequeuedPacket = dcaQueue->DequeueByTidAndAddress (&dequeuedHdr, tid,
                                                                             WifiMacHeader::ADDR1,
                                                                             GetBssid());
                }
//    if(count!=0)
//      std::cout<<"DCA "<<count<<"\n";             
    
   }
}


} //namespace ns3
