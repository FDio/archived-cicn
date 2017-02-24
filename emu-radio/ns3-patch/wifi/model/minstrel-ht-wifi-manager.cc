/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Ghada Badawy
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
 * Author: Ghada Badawy <gbadawy@gmail.com>
 *
 * Some Comments:
 *
 * 1) Segment Size is declared for completeness but not used  because it has
 *    to do more with the requirement of the specific hardware.
 *
 * 2) By default, Minstrel applies the multi-rate retry(the core of Minstrel
 *    algorithm). Otherwise, please use ConstantRateWifiManager instead.
 *
 * 3) 40Mhz can't fall back to 20MHz
 *
 * reference: http://lwn.net/Articles/376765/
 */

#include "minstrel-ht-wifi-manager.h"
#include "wifi-phy.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/wifi-mac.h"
#include "ns3/assert.h"
#include <vector>

//newly added
#include "ns3/ht-wifi-mac-helper.h"

#define Min(a,b) ((a < b) ? a : b)




namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MinstrelHtWifiManager");

struct MinstrelHtWifiRemoteStation : public WifiRemoteStation
{
  void DisposeStation ();
  Time m_nextStatsUpdate;  ///< 10 times every second

  /**
   * To keep track of the current position in the our random sample table
   * going row by row from 1st column until the 10th column(Minstrel defines 10)
   * then we wrap back to the row 1 col 1.
   * note: there are many other ways to do this.
   */
  uint32_t m_col, m_index;
  uint32_t m_maxTpRate;  ///< the current throughput rate
  uint32_t m_maxTpRate2;  ///< second highest throughput rate
  uint32_t m_maxProbRate;  ///< rate with highest prob of success
  uint8_t m_maxTpStreams; ///< number of streams for max TP
  uint8_t m_maxTp2Streams;///< number of streams for max TP2
  uint8_t m_maxProbStreams;

  int m_packetCount;  ///< total number of packets as of now
  int m_sampleCount;  ///< how many packets we have sample so far

  bool m_isSampling;  ///< a flag to indicate we are currently sampling
  uint32_t m_sampleRate;  ///< current sample rate
  bool  m_sampleRateSlower;  ///< a flag to indicate sample rate is slower
  uint32_t m_currentRate;  ///< current rate we are using
  uint32_t m_sampleGroup; ///< the group that the sample rate belong to
  uint8_t m_sampleStreams; ///< the number of streams to use with the sample rate

  uint32_t m_shortRetry;  ///< short retries such as control packts
  uint32_t m_longRetry;  ///< long retries such as data packets
  uint32_t m_retry;  ///< total retries short + long
  uint32_t m_err;  ///< retry errors
  uint32_t m_txrate;  ///< current transmit rate
  uint8_t m_txstreams; ///<current transmit streams

  bool m_initialized;  ///< for initializing tables

 // MinstrelRate m_minstrelTable;  ///< minstrel table
  HtSampleRate m_sampleTable;  ///< sample table
  McsGroup m_mcsTable; ///< MCS groups table
  
  //added by zeng
  bool m_isNewPacketSent;
  uint32_t m_txrateToUse;
};

void
MinstrelHtWifiRemoteStation::DisposeStation ()
{
  std::vector<std::vector<uint32_t> >().swap(m_sampleTable);
  for (uint8_t j=0; j< m_mcsTable.size();j++)
    std::vector<struct HtRateInfo>().swap(m_mcsTable[j].m_minstrelTable);
  std::vector<struct GroupInfo> ().swap(m_mcsTable);
  
}

NS_OBJECT_ENSURE_REGISTERED (MinstrelHtWifiManager);

TypeId
MinstrelHtWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MinstrelHtWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .AddConstructor<MinstrelHtWifiManager> ()
    .AddAttribute ("UpdateStatistics",
                   "The interval between updating statistics table ",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&MinstrelHtWifiManager::m_updateStats),
                   MakeTimeChecker ())
    .AddAttribute ("LookAroundRate",
                   "the percentage to try other rates",
                   DoubleValue (10),
                   MakeDoubleAccessor (&MinstrelHtWifiManager::m_lookAroundRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("EWMA",
                   "EWMA level",
                   DoubleValue (75),
                   MakeDoubleAccessor (&MinstrelHtWifiManager::m_ewmaLevel),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SegmentSize",
                   "The largest allowable segment size packet",
                   DoubleValue (6000),
                   MakeDoubleAccessor (&MinstrelHtWifiManager::m_segmentSize),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("SampleColumn",
                   "The number of columns used for sampling",
                   DoubleValue (10),
                   MakeDoubleAccessor (&MinstrelHtWifiManager::m_sampleCol),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("PacketLength",
                   "The packet length used for calculating mode TxTime",
                   DoubleValue (65536),
                   MakeDoubleAccessor (&MinstrelHtWifiManager::m_pktLen),
                   MakeDoubleChecker <double> ())
    .AddTraceSource ("RateChange",
                     "The transmission rate has changed",
                     MakeTraceSourceAccessor (&MinstrelHtWifiManager::m_rateChange),
                     "ns3::MinstrelHtWifiManager::RateChangeTracedCallback")
  ;
  return tid;
}

MinstrelHtWifiManager::MinstrelHtWifiManager ()
{
  m_nsupported = 0;
}

MinstrelHtWifiManager::~MinstrelHtWifiManager ()
{
  m_calcTxTime.clear();
}

void
MinstrelHtWifiManager::SetupPhy (Ptr<WifiPhy> phy)
{
  //8
  uint32_t nModes = phy->GetNMcs ();
  for (uint32_t i = 0; i < nModes; i++)
    {
      //WifiMode mode = phy->GetMode (i);
      
      StringValue DataRate = HtWifiMacHelper::DataRateForMcs (i);
      WifiTxVector txvector;
      txvector.SetMode(WifiMode(DataRate.Get()));
      txvector.SetTxPowerLevel(0);
      txvector.SetShortGuardInterval(phy->GetGuardInterval());
      txvector.SetNss(1);
      txvector.SetNess(0);
      //txvector.SetStbc(phy->GetStbc());
      txvector.SetStbc(false);
      WifiPreamble preamble;
      if (HasHtSupported())
        preamble= WIFI_PREAMBLE_HT_MF;
      else
         preamble= WIFI_PREAMBLE_LONG;
      AddCalcTxTime (txvector.GetMode(), phy->CalculateTxDuration (m_pktLen, txvector, preamble, phy->GetFrequency(),0,0));
    }
  WifiRemoteStationManager::SetupPhy (phy);
}

Time
MinstrelHtWifiManager::GetCalcTxTime (WifiMode mode) const
{

  for (TxTime::const_iterator i = m_calcTxTime.begin (); i != m_calcTxTime.end (); i++)
    {
      if (mode == i->second)
        {
          return i->first;
        }
    }
  NS_ASSERT (false);
  return Seconds (0);
}

void
MinstrelHtWifiManager::AddCalcTxTime (WifiMode mode, Time t)
{
  m_calcTxTime.push_back (std::make_pair (t, mode));
}

WifiRemoteStation *
MinstrelHtWifiManager::DoCreateStation (void) const
{
  MinstrelHtWifiRemoteStation *station = new MinstrelHtWifiRemoteStation ();

  station->m_nextStatsUpdate = Simulator::Now () + m_updateStats;
  station->m_col = 0;
  station->m_index = 0;
  station->m_maxTpRate = 0;
  station->m_maxTpRate2 = 0;
  station->m_maxProbRate = 0;
  station->m_packetCount = 0;
  station->m_sampleCount = 0;
  station->m_isSampling = false;
  station->m_sampleRate = 0;
  station->m_sampleRateSlower = false;
  station->m_currentRate = 0;
  station->m_shortRetry = 0;
  station->m_longRetry = 0;
  station->m_retry = 0;
  station->m_err = 0;
  station->m_txrate = 0;
  station->m_initialized = false;
  station->m_sampleGroup = 0;
  station->m_txstreams = 1;
  station->m_maxTpStreams = 1;
  station->m_maxTp2Streams = 1;
  station->m_maxProbStreams = 1;
  station->m_sampleStreams = 1;
  
  //added by zeng
  station->m_isNewPacketSent=false;
  station->m_txrateToUse = 0;
  
  
  return station;
}

void
MinstrelHtWifiManager::CheckInit (MinstrelHtWifiRemoteStation *station)
{
  if (!station->m_initialized && GetNMcsSupported (station) > 1)
    {
      // Note: we appear to be doing late initialization of the table
      // to make sure that the set of supported rates has been initialized
      // before we perform our own initialization.
      m_nsupported = GetNMcsSupported(station);
      //NOTE: to be fixed later, it is hard coded now
      m_nGroups=1;
      station->m_mcsTable = McsGroup (m_nGroups);
      station->m_sampleTable = HtSampleRate (8, std::vector<uint32_t> (m_sampleCol));
      InitSampleTable (station);
      RateInit (station);
      station->m_initialized = true;

    }
}

void
MinstrelHtWifiManager::DoReportRxOk (WifiRemoteStation *st,
                                   double rxSnr, WifiMode txMode)
{
  NS_LOG_DEBUG ("DoReportRxOk m_txrate=" << txMode);
}

void
MinstrelHtWifiManager::DoReportRtsFailed (WifiRemoteStation *st)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *)st;
  NS_LOG_DEBUG ("DoReportRtsFailed m_txrate=" << station->m_txrate);

  station->m_shortRetry++;
}

void
MinstrelHtWifiManager::DoReportRtsOk (WifiRemoteStation *st, double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_DEBUG ("self=" << st << " rts ok");
}

void
MinstrelHtWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *st)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *)st;
  NS_LOG_DEBUG ("Final RTS failed");
  UpdateRetry (station);
  station->m_err++;
}

uint32_t  
MinstrelHtWifiManager::GetRateId(uint32_t rate)
{
  uint32_t id;
  id = rate % 8;
  return id;
}
uint32_t  
MinstrelHtWifiManager::GetGroupId(uint32_t rate, WifiRemoteStation *st, uint8_t txstreams)
{
  uint32_t id;
  //we have 2 groups if we have SGI else we only have 1 group for 40 or 20MHZ channel BW
  //NOTE: to be fixed later, it is hard coded now
  id=0;
  return id;
}

void
MinstrelHtWifiManager::DoReportDataFailed (WifiRemoteStation *st)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *)st;
  /**
   *
   * Retry Chain table is implemented here
   *
   * Try |         LOOKAROUND RATE              | NORMAL RATE
   *     | random < best    | random > best     |
   * --------------------------------------------------------------
   *  1  | Best throughput  | Random rate       | Best throughput
   *  2  | Random rate      | Best throughput   | Next best throughput
   *  3  | Best probability | Best probability  | Best probability
   *  4  | Lowest Baserate  | Lowest baserate   | Lowest baserate
   *
   * Note: For clarity, multiple blocks of if's and else's are used
   * After a failing 7 times, DoReportFinalDataFailed will be called
   */

  CheckInit (station);
  if (!station->m_initialized)
    {
      return;
    }

  //******************
//   if(!station->m_isNewPacketSent)//if a new packet is not sent yet, don't update the counter and txrate, the idea is to view all AMPDU packets in one A-MPDU as one packet.
//     return;
  
  station->m_longRetry++;
  uint32_t rateid = GetRateId (station->m_txrate);
  uint32_t groupid = GetGroupId (station->m_txrate,station,station->m_txstreams);
  uint32_t maxtprateid = GetRateId (station->m_maxTpRate);
  uint32_t maxtpgroupid = GetGroupId (station->m_maxTpRate,station,station->m_maxTpStreams);
  uint32_t maxtp2rateid = GetRateId (station->m_maxTpRate2);
  uint32_t maxtp2groupid = GetGroupId (station->m_maxTpRate2,station,station->m_maxTp2Streams);
  uint32_t samplerateid = GetRateId (station->m_sampleRate);
  uint32_t samplegroupid = GetGroupId (station->m_sampleRate,station,station->m_sampleStreams);

  station->m_mcsTable[groupid].m_minstrelTable[rateid].numRateAttempt++; // for some reason kept track at FinalDataFail!!!

  NS_LOG_DEBUG ("DoReportDataFailed " << station << "\t rate " << station->m_txrate << "\tlongRetry \t" << station->m_longRetry);


  /// for normal rate, we're not currently sampling random rates
  if (!station->m_isSampling)
    {
      /// use best throughput rate
      if (station->m_longRetry <  station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount)
        {
            NS_LOG_DEBUG ("Not Sampling use the same rate again");
            station->m_txrateToUse = station->m_maxTpRate;  ///<  there's still a few retries left
        }

      /// use second best throughput rate
      else if (station->m_longRetry <= ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                         station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount))
        {
           NS_LOG_DEBUG ("Not Sampling use the Max TP2");
          station->m_txrateToUse = station->m_maxTpRate2;
        }

      /// use best probability rate
      else if (station->m_longRetry <= ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                         station->m_mcsTable[maxtp2groupid].m_minstrelTable[maxtp2rateid].adjustedRetryCount +
                                         station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount))
        {
           NS_LOG_DEBUG ("Not Sampling use Max Prob");
          station->m_txrateToUse = station->m_maxProbRate;
        }

      /// use lowest base rate
      else if (station->m_longRetry > ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                        station->m_mcsTable[maxtp2groupid].m_minstrelTable[maxtp2rateid].adjustedRetryCount +
                                        station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount))
        {
           NS_LOG_DEBUG ("Not Sampling use MCS0");
          station->m_txrateToUse = 0;
        }
    }

  /// for look-around rate, we're currently sampling random rates
  else
    {
      /// current sampling rate is slower than the current best rate
      if (station->m_sampleRateSlower)
        {
          /// use best throughput rate
          if (station->m_longRetry <  station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount)
            {
                 NS_LOG_DEBUG ("Sampling use the same rate again");
               station->m_txrateToUse = station->m_maxTpRate;///<  there are a few retries left
            }

          ///	use random rate
          else if (station->m_longRetry <= ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                             station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount))
            {
                NS_LOG_DEBUG ("Sampling use the sample rate");
              station->m_txrateToUse = station->m_sampleRate;
            }

          /// use max probability rate
          else if (station->m_longRetry <= ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                             station->m_mcsTable[samplegroupid].m_minstrelTable[samplerateid].adjustedRetryCount +
                                             station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount ))
            {
                NS_LOG_DEBUG ("Sampling use Max prob");
              station->m_txrateToUse =  station->m_maxProbRate;
            }

          /// use lowest base rate
          else if (station->m_longRetry > ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                            station->m_mcsTable[samplegroupid].m_minstrelTable[samplerateid].adjustedRetryCount +
                                            station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount))
            {
                NS_LOG_DEBUG ("Sampling use MCS0");
              station->m_txrateToUse = 0;
            }
        }

      /// current sampling rate is better than current best rate
      else
        {
          /// use random rate
          if (station->m_longRetry <  station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount)
            {
                 NS_LOG_DEBUG ("Sampling use the same sample rate");
               station->m_txrateToUse = station->m_sampleRate;    ///< keep using it
            }

          /// use the best rate
          else if (station->m_longRetry <= ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                             station->m_mcsTable[samplegroupid].m_minstrelTable[samplerateid].adjustedRetryCount))
            {
                NS_LOG_DEBUG ("Sampling use the MaxTP rate");
              station->m_txrateToUse = station->m_maxTpRate;
            }

          /// use the best probability rate
          else if (station->m_longRetry <= ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                             station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount +
                                             station->m_mcsTable[samplegroupid].m_minstrelTable[samplerateid].adjustedRetryCount))
            {
                NS_LOG_DEBUG ("Sampling use the MaxProb rate");
              station->m_txrateToUse =station->m_maxProbRate;
            }

          /// use the lowest base rate
          else if (station->m_longRetry > ( station->m_mcsTable[groupid].m_minstrelTable[rateid].adjustedRetryCount +
                                            station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount +
                                            station->m_mcsTable[samplegroupid].m_minstrelTable[samplerateid].adjustedRetryCount))
            {
              NS_LOG_DEBUG ("Sampling use the MCS0");
              station->m_txrateToUse = 0;
            }
        }
    }
  NS_LOG_DEBUG ("Txrate = " << station->m_txrateToUse  );
  
   //****************
//   station->m_isNewPacketSent=false;
}

void
MinstrelHtWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                     double ackSnr, WifiMode ackMode, double dataSnr)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *) st;
  station->m_isSampling = false;
  station->m_sampleRateSlower = false;

  CheckInit (station);
  if (!station->m_initialized)
    {
      return;
    }
 
 //******************
//   if(!station->m_isNewPacketSent)//if a new packet is not sent yet, don't update the counter and txrate, the idea is to view all AMPDU packets in one A-MPDU as one packet.
//     return;
  
  uint32_t rateid = GetRateId(station->m_txrate);
  uint32_t groupid = GetGroupId(station->m_txrate,station, station->m_txstreams);
  station->m_mcsTable[groupid].m_minstrelTable[rateid].numRateSuccess++;
  station->m_mcsTable[groupid].m_minstrelTable[rateid].numRateAttempt++;

  UpdateRetry (station);

  //station->m_minstrelTable[station->m_txrate].numRateAttempt += station->m_retry;
  station->m_packetCount++;

  if (m_nsupported >= 1)
    {
      station->m_txrateToUse = FindRate (station);
    }
  NS_LOG_DEBUG ("Data OK - Txrate = " << station->m_txrate  );
  
  //****************
//   station->m_isNewPacketSent=false;
}

void
MinstrelHtWifiManager::DoReportFinalDataFailed (WifiRemoteStation *st)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *) st;
  NS_LOG_DEBUG ("DoReportFinalDataFailed m_txrate=" << station->m_txrate);

  //******************
//   if(!station->m_isNewPacketSent)//if a new packet is not sent yet, don't update the counter and txrate, the idea is to view all AMPDU packets in one A-MPDU as one packet.
//     return;
  
  station->m_isSampling = false;
  station->m_sampleRateSlower = false;

  UpdateRetry (station);

  CheckInit (station);
  if (!station->m_initialized)
    {
      return;
    }

  uint32_t rateid = GetRateId(station->m_txrate);
  uint32_t groupid = GetGroupId(station->m_txrate,station,station->m_txstreams );
  station->m_mcsTable[groupid].m_minstrelTable[rateid].numRateAttempt++;
  station->m_err++;

  if (m_nsupported >= 1)
    {
      station->m_txrateToUse = FindRate (station);
    }
   NS_LOG_DEBUG ("Txrate = " << station->m_txrate  );
  
   //****************
//   station->m_isNewPacketSent=false;
}

void
MinstrelHtWifiManager::UpdateRetry (MinstrelHtWifiRemoteStation *station)
{
  station->m_retry = station->m_shortRetry + station->m_longRetry;
  station->m_shortRetry = 0;
  station->m_longRetry = 0;
}

WifiTxVector
MinstrelHtWifiManager::DoGetDataTxVector (WifiRemoteStation *st,
                                    uint32_t size)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *) st;
  station->m_txrate=station->m_txrateToUse;

  if (!station->m_isSampling)
  {
    m_rateChange (station->m_txrate, station->m_state->m_address);
  }

  if (!station->m_initialized)
    {
     
      CheckInit (station);
      if (!station->m_initialized)
        {
             station->m_txrate = 0;
         }
      else
         {
           /// start the rate at half way
           station->m_txrate = m_nsupported / 2;
         }
    }
    
     //****************
//   station->m_isNewPacketSent=true;
  NS_LOG_DEBUG ("DoGetDataMode m_txrate=" << station->m_txrate << " m_nsupported " << m_nsupported);
  UpdateStats (station);
  //if station->txrate == something then this is a 2x2 stream
  return WifiTxVector (GetMcsSupported (station, station->m_txrate), GetDefaultTxPowerLevel (), st->m_slrc, GetShortGuardInterval (st), 1, 0, GetChannelWidth (st), GetAggregation (st), false);
}

WifiTxVector
MinstrelHtWifiManager::DoGetRtsTxVector (WifiRemoteStation *st)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *) st;
    station->m_txrate=station->m_txrateToUse;
  NS_LOG_DEBUG ("DoGetRtsMode m_txrate=" << station->m_txrate);

       //****************
//   station->m_isNewPacketSent=true;
  return WifiTxVector (GetMcsSupported (station, 0), GetDefaultTxPowerLevel (), st->m_ssrc, GetShortGuardInterval (st), 1, 0, GetChannelWidth (st), GetAggregation (st), false);
}

bool
MinstrelHtWifiManager::DoNeedDataRetransmission (WifiRemoteStation *st, Ptr<const Packet> packet, bool normally)
{
  MinstrelHtWifiRemoteStation *station = (MinstrelHtWifiRemoteStation *)st;
  uint32_t maxprobrateid = GetRateId (station->m_maxProbRate);
  uint32_t maxprobgroupid = GetGroupId (station->m_maxProbRate,station, station->m_maxProbStreams);
  uint32_t maxtprateid = GetRateId (station->m_maxTpRate);
  uint32_t maxtpgroupid = GetGroupId (station->m_maxTpRate,station,station->m_maxTpStreams );
  uint32_t maxtp2rateid = GetRateId (station->m_maxTpRate2);
  uint32_t maxtp2groupid = GetGroupId (station->m_maxTpRate2,station,station->m_maxTp2Streams);
  uint32_t samplerateid = GetRateId (station->m_sampleRate);
  uint32_t samplegroupid = GetGroupId (station->m_sampleRate,station,station->m_sampleStreams);

  CheckInit (station);
  if (!station->m_initialized)
    {
      return normally;
    }

  if (!station->m_isSampling)
    {
      if (station->m_longRetry > (station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount +
                                  station->m_mcsTable[maxtp2groupid].m_minstrelTable[maxtp2rateid].adjustedRetryCount +
                                  station->m_mcsTable[maxprobgroupid].m_minstrelTable[maxprobrateid].adjustedRetryCount +
                                  station->m_mcsTable[0].m_minstrelTable[0].adjustedRetryCount))
        {
         NS_LOG_DEBUG ("No re-transmission allowed" );
          return false;
        }
      else
        {
          NS_LOG_DEBUG ("Re-tranmsit" );
          return true;
        }
    }
  else
    {
      if (station->m_longRetry > (station->m_mcsTable[samplegroupid].m_minstrelTable[samplerateid].adjustedRetryCount +
                                  station->m_mcsTable[maxtpgroupid].m_minstrelTable[maxtprateid].adjustedRetryCount +
                                  station->m_mcsTable[maxprobgroupid].m_minstrelTable[maxprobrateid].adjustedRetryCount +
                                  station->m_mcsTable[0].m_minstrelTable[0].adjustedRetryCount))
        {
          NS_LOG_DEBUG ("No re-transmission allowed" );
          return false;
        }
      else
        {
          NS_LOG_DEBUG ("Re-tranmsit" );
          return true;
        }
    }
}

bool
MinstrelHtWifiManager::IsLowLatency (void) const
{
  return true;
}
uint8_t
MinstrelHtWifiManager::GetStreams (uint32_t groupId, MinstrelHtWifiRemoteStation *station)
{
  uint8_t nstreams = 1;
  if (GetShortGuardInterval(station) && m_nGroups > 2)
    {
      //SGI is supported and we have more than one stream
      if (groupId > 2) //group 0 and 1 are SGI and LGI 1 stream
        nstreams = 2;
    }
  else if (!GetShortGuardInterval(station) && m_nGroups > 1)
   {
     //SGI is not supported and we have more than one stream
     if (groupId == 1)
       nstreams = 2;
   }
  return nstreams;
}
uint32_t
MinstrelHtWifiManager::GetNextSample (MinstrelHtWifiRemoteStation *station)
{
  uint32_t sampleindex;
  uint32_t bitrate;
  sampleindex = station->m_sampleTable[station->m_mcsTable[station->m_sampleGroup].m_index][station->m_mcsTable[station->m_sampleGroup].m_col];
  bitrate = GetTxRate(station->m_sampleGroup,sampleindex);
  station->m_mcsTable[station->m_sampleGroup].m_index++;
  station->m_sampleStreams = GetStreams (station->m_sampleGroup, station);
  station->m_sampleGroup++;
  /// bookeeping for m_index and m_col variables
  station->m_sampleGroup %= m_nGroups;
  if (station->m_mcsTable[station->m_sampleGroup].m_index > 6)
    {
      station->m_mcsTable[station->m_sampleGroup].m_index = 0;
      station->m_mcsTable[station->m_sampleGroup].m_col++;
      if (station->m_mcsTable[station->m_sampleGroup].m_col >= m_sampleCol)
        {
          station->m_mcsTable[station->m_sampleGroup].m_col = 0;
        }
    }
  NS_LOG_DEBUG ("Next Sample is " << bitrate );
  return bitrate;
}

uint32_t
MinstrelHtWifiManager::FindRate (MinstrelHtWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("FindRate " << "packet=" << station->m_packetCount );

  if ((station->m_sampleCount + station->m_packetCount) == 0)
    {
      return 0;
    }
  
  uint32_t idx;

  /// for determining when to try a sample rate
  Ptr<UniformRandomVariable> coinFlip = CreateObject<UniformRandomVariable> ();
  coinFlip->SetAttribute ("Min", DoubleValue (0));
  coinFlip->SetAttribute ("Max", DoubleValue (100));

  /**
   * if we are below the target of look around rate percentage, look around
   * note: do it randomly by flipping a coin instead sampling
   * all at once until it reaches the look around rate
   */
  if ( (((100 * station->m_sampleCount) / (station->m_sampleCount + station->m_packetCount )) < m_lookAroundRate)
       && ((int)coinFlip->GetValue ()) % 2 == 1 )
    {
       NS_LOG_DEBUG ("Sampling");
      /// now go through the table and find an index rate
      idx = GetNextSample (station);
      NS_LOG_DEBUG ("Sampling rate = " << idx);

      /**
       * This if condition is used to make sure that we don't need to use
       * the sample rate it is the same as our current rate
       */
      if (idx != station->m_maxTpRate && idx != station->m_txrate)
        {

          /// start sample count
          station->m_sampleCount++;

          /// set flag that we are currently sampling
          station->m_isSampling = true;

          /// bookeeping for resetting stuff
          if (station->m_packetCount >= 10000)
            {
              station->m_sampleCount = 0;
              station->m_packetCount = 0;
            }

          /// error check
          if (idx >= m_nsupported)
            {
              NS_LOG_DEBUG ("ALERT!!! ERROR");
            }

          /// set the rate that we're currently sampling
          station->m_sampleRate = idx;

          if (station->m_sampleRate == station->m_maxTpRate)
            {
              station->m_sampleRate = station->m_maxTpRate2;
            }

          /// is this rate slower than the current best rate
          uint32_t idxgroupid = GetGroupId(idx,station, station-> m_sampleStreams);
           uint32_t idxrateid = GetRateId(idx);
          uint32_t maxTpgroupid = GetGroupId(station->m_maxTpRate,station,station->m_maxTpStreams);
           uint32_t maxTprateid = GetRateId(station->m_maxTpRate);
          station->m_sampleRateSlower =
            (station->m_mcsTable[idxgroupid].m_minstrelTable[idxrateid].perfectTxTime > station->m_mcsTable[maxTpgroupid].m_minstrelTable[maxTprateid].perfectTxTime);

          /// using the best rate instead
          if (station->m_sampleRateSlower)
            {
              idx =  station->m_maxTpRate;
            }
        }

    }

  ///	continue using the best rate
  else
    {
      idx = station->m_maxTpRate;
    }


  NS_LOG_DEBUG ("FindRate " << "sample rate=" << idx);

  return idx;
}
uint32_t
MinstrelHtWifiManager::GetTxRate(uint32_t groupid, uint32_t index)
{
  uint32_t rate;
  //Group 0 for LGI group 1 for SGI
  //rate = (groupid*8)+ index;
  rate=index%8;//to avoid MCS_index >7 to happen
  return rate;
}
void
MinstrelHtWifiManager::UpdateStats (MinstrelHtWifiRemoteStation *station)
{
  if (Simulator::Now () <  station->m_nextStatsUpdate)
    {
      return;
    }

  if (!station->m_initialized)
    {
      return;
    }
  NS_LOG_DEBUG ("Updating stats=" << this);

  station->m_nextStatsUpdate = Simulator::Now () + m_updateStats;
 
  Time txTime;
  uint32_t tempProb;

 
  //update throughput and emwa for each rate inside each group
  for (uint32_t j=0 ; j <m_nGroups ; j++)
  {
  for (uint32_t i = 0; i < 8; i++)
    {

      /// calculate the perfect tx time for this rate
      txTime =  station->m_mcsTable[j].m_minstrelTable[i].perfectTxTime;

      /// just for initialization
      if (txTime.GetMicroSeconds () == 0)
        {
          txTime = Seconds (1);
        }

      NS_LOG_DEBUG (i << " " << GetMcsSupported (station, GetTxRate(j,i)) <<
                    "\t attempt=" << station->m_mcsTable[j].m_minstrelTable[i].numRateAttempt <<
                    "\t success=" << station->m_mcsTable[j].m_minstrelTable[i].numRateSuccess);

      /// if we've attempted something
      if (station->m_mcsTable[j].m_minstrelTable[i].numRateAttempt)
        {
          /**
           * calculate the probability of success
           * assume probability scales from 0 to 18000
           */
          tempProb = (station->m_mcsTable[j].m_minstrelTable[i].numRateSuccess * 18000) / station->m_mcsTable[j].m_minstrelTable[i].numRateAttempt;

          /// bookeeping
          station->m_mcsTable[j].m_minstrelTable[i].prob = tempProb;

          /// ewma probability (cast for gcc 3.4 compatibility)
          tempProb = static_cast<uint32_t> (((tempProb * (100 - m_ewmaLevel)) + (station->m_mcsTable[j].m_minstrelTable[i].ewmaProb * m_ewmaLevel) ) / 100);

          station->m_mcsTable[j].m_minstrelTable[i].ewmaProb = tempProb;

          /// calculating throughput
          station->m_mcsTable[j].m_minstrelTable[i].throughput = tempProb * (1000000 / txTime.GetMicroSeconds ());

        }

      /// bookeeping
      station->m_mcsTable[j].m_minstrelTable[i].numRateSuccess = 0;
      station->m_mcsTable[j].m_minstrelTable[i].numRateAttempt = 0;

      /// Sample less often below 10% and  above 95% of success
      if ((station->m_mcsTable[j].m_minstrelTable[i].ewmaProb > 17100) || (station->m_mcsTable[j].m_minstrelTable[i].ewmaProb < 1800))
        {
          /**
           * retry count denotes the number of retries permitted for each rate
           * # retry_count/2
           */
          
          if (station->m_mcsTable[j].m_minstrelTable[i].adjustedRetryCount > 2)
            {
              station->m_mcsTable[j].m_minstrelTable[i].adjustedRetryCount = 2;
            }
          else
            {
              station->m_mcsTable[j].m_minstrelTable[i].adjustedRetryCount = station->m_mcsTable[j].m_minstrelTable[i].retryCount;
            }
        }
      else
        {
          station->m_mcsTable[j].m_minstrelTable[i].adjustedRetryCount = station->m_mcsTable[j].m_minstrelTable[i].retryCount;
        }

      /// if it's 0 allow one retry limit
      if (station->m_mcsTable[j].m_minstrelTable[i].adjustedRetryCount == 0)
        {
          station->m_mcsTable[j].m_minstrelTable[i].adjustedRetryCount = 1;
        }
    }
  }

 //for each group get the max tp and maxtp2
 uint32_t max_prob = 0, index_max_prob = 0, max_tp = 0, index_max_tp = 0, index_max_tp2 = 0;
 uint32_t index_max_prob_streams =1, index_max_tp_streams =1 , index_max_tp2_streams =1;
 for (uint32_t j = 0; j <m_nGroups; j++)
  {
   max_prob = 0;
   index_max_prob = 0;
   max_tp = 0;
   index_max_tp = 0;
   index_max_tp2 = 0;
  /// go find max throughput, second maximum throughput, high probability succ
  for (uint32_t i = 0; i < 8; i++)
    {
      NS_LOG_DEBUG ("throughput" << station->m_mcsTable[j].m_minstrelTable[i].throughput <<
                    "\n ewma" << station->m_mcsTable[j].m_minstrelTable[i].ewmaProb);

      if (max_tp < station->m_mcsTable[j].m_minstrelTable[i].throughput)
        {
          index_max_tp = i;
          max_tp = station->m_mcsTable[j].m_minstrelTable[i].throughput;
        }

      if (max_prob < station->m_mcsTable[j].m_minstrelTable[i].ewmaProb)
        {
          index_max_prob = i;
          max_prob = station->m_mcsTable[j].m_minstrelTable[i].ewmaProb;
        }
    }

  max_tp = 0;
  /// find the second highest max
  for (uint32_t i = 0; i <8; i++)
    {
      if ((i != index_max_tp) && (max_tp < station->m_mcsTable[j].m_minstrelTable[i].throughput))
        {
          index_max_tp2 = i;
          max_tp = station->m_mcsTable[j].m_minstrelTable[i].throughput;
        }
    }

  station->m_mcsTable[j].m_maxTpRate = index_max_tp;
  station->m_mcsTable[j].m_maxTpRate2 = index_max_tp2;
  station->m_mcsTable[j].m_maxProbRate = index_max_prob;
  }

  max_prob = 0;
   index_max_prob = 0;
   max_tp = 0;
   index_max_tp = 0;
   index_max_tp2 = 0;
// get the maxtp and maxtp2 from all groups
 for (uint32_t j = 0; j < m_nGroups; j++)
  {
  /// go find max throughput, second maximum throughput, high probability succ
  
      if (max_tp < station->m_mcsTable[j].m_minstrelTable[station->m_mcsTable[j].m_maxTpRate].throughput)
        {
          index_max_tp = GetTxRate(j, station->m_mcsTable[j].m_maxTpRate);
          max_tp = station->m_mcsTable[j].m_minstrelTable[station->m_mcsTable[j].m_maxTpRate].throughput;
          index_max_tp_streams = GetStreams (j, station);
        }

      if (max_prob < station->m_mcsTable[j].m_minstrelTable[station->m_mcsTable[j].m_maxProbRate].ewmaProb)
        {
          index_max_prob = GetTxRate(j,station->m_mcsTable[j].m_maxProbRate);
          max_prob = station->m_mcsTable[j].m_minstrelTable[station->m_mcsTable[j].m_maxProbRate].ewmaProb;
          index_max_prob_streams = GetStreams (j, station);
        }
}
  max_tp = 0;
  /// find the second highest max
  for (uint32_t i = 0; i <m_nGroups; i++)
    {
      if ((GetTxRate(i,station->m_mcsTable[i].m_maxTpRate) != index_max_tp) && (max_tp < station->m_mcsTable[i].m_minstrelTable[station->m_mcsTable[i].m_maxTpRate].throughput))
        {
          //finds if another group maxtp is better than the max tp2
          index_max_tp2 = GetTxRate(i,station->m_mcsTable[i].m_maxTpRate);
          max_tp = station->m_mcsTable[i].m_minstrelTable[station->m_mcsTable[i].m_maxTpRate].throughput;
          index_max_tp2_streams = GetStreams (i, station);
        }
      if (max_tp < station->m_mcsTable[i].m_minstrelTable[station->m_mcsTable[i].m_maxTpRate2].throughput)
        {
          //find if another group maxtp2 is better than maxtp2
          index_max_tp2 = GetTxRate(i,station->m_mcsTable[i].m_maxTpRate2);
          max_tp = station->m_mcsTable[i].m_minstrelTable[station->m_mcsTable[i].m_maxTpRate2].throughput;
          index_max_tp2_streams = GetStreams (i, station);
        }
    }


  station->m_maxTpRate = index_max_tp;
  station->m_maxTpStreams = index_max_tp_streams;
  station->m_maxTpRate2 = index_max_tp2;
  station->m_maxTp2Streams = index_max_tp2_streams;
  station->m_maxProbRate = index_max_prob;
  station->m_maxProbStreams = index_max_prob_streams;
  station->m_currentRate = index_max_tp;

  ///if the max tp rate is bigger than the current rate and uses the same number of streams
  if ((index_max_tp > station->m_txrate) &&(index_max_tp_streams >= station->m_txstreams) )
    {
      station->m_txrate = index_max_tp;
      station->m_txstreams = index_max_tp_streams;
    }

  NS_LOG_DEBUG ("max tp=" << index_max_tp << "\nmax tp2=" << index_max_tp2 << "\nmax prob=" << index_max_prob);

  /// reset it
  //RateInit (station);
}

void
MinstrelHtWifiManager::RateInit (MinstrelHtWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("RateInit=" << station);
 
   for (uint32_t j = 0; j < m_nGroups; j++)
    {
      //should include MCS
      station->m_mcsTable[j].m_minstrelTable = HtMinstrelRate (m_nsupported/m_nGroups);
      station->m_mcsTable[j].m_col = 0;
      station->m_mcsTable[j].m_index = 0;
      for (uint32_t i = 0; i < 8; i++)
       {
         station->m_mcsTable[j].m_minstrelTable[i].numRateAttempt = 0;
         station->m_mcsTable[j].m_minstrelTable[i].numRateSuccess = 0;
         station->m_mcsTable[j].m_minstrelTable[i].prob = 0;
         station->m_mcsTable[j].m_minstrelTable[i].ewmaProb = 0;
         station->m_mcsTable[j].m_minstrelTable[i].prevNumRateAttempt = 0;
         station->m_mcsTable[j].m_minstrelTable[i].prevNumRateSuccess = 0;
         station->m_mcsTable[j].m_minstrelTable[i].successHist = 0;
         station->m_mcsTable[j].m_minstrelTable[i].attemptHist = 0;
         station->m_mcsTable[j].m_minstrelTable[i].throughput = 0;
         station->m_mcsTable[j].m_minstrelTable[i].perfectTxTime = GetCalcTxTime (GetMcsSupported (station, GetTxRate(j,i)));
         station->m_mcsTable[j].m_minstrelTable[i].retryCount = 1;
         station->m_mcsTable[j].m_minstrelTable[i].adjustedRetryCount = 1;
      }  
   }
}

void
MinstrelHtWifiManager::InitSampleTable (MinstrelHtWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("InitSampleTable=" << this);

  station->m_col = station->m_index = 0;

  /// for off-seting to make rates fall between 0 and numrates
  uint32_t numSampleRates = 8;

  uint32_t newIndex;
  for (uint32_t col = 0; col < m_sampleCol; col++)
    {
      for (uint32_t i = 0; i < numSampleRates; i++ )
        {

          /**
           * The next two lines basically tries to generate a random number
           * between 0 and the number of available rates
           */
          Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
          uv->SetAttribute ("Min", DoubleValue (0));
          uv->SetAttribute ("Max", DoubleValue (numSampleRates));
        
          newIndex = (i + (uint32_t)uv->GetValue ()) % numSampleRates;

          /// this loop is used for filling in other uninitilized places
          while (station->m_sampleTable[newIndex][col] != 0)
            {
              newIndex = (newIndex + 1) % 8;
            }
          station->m_sampleTable[newIndex][col] = i;

        }
    }
}

void
MinstrelHtWifiManager::PrintSampleTable (MinstrelHtWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("PrintSampleTable=" << station);

  uint32_t numSampleRates = 8;
  for (uint32_t i = 0; i < numSampleRates; i++)
    {
      for (uint32_t j = 0; j < m_sampleCol; j++)
        {
          std::cout << station->m_sampleTable[i][j] << "\t";
        }
      std::cout << std::endl;
    }
}

void
MinstrelHtWifiManager::PrintTable (MinstrelHtWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("PrintTable=" << station);
  for (uint32_t j = 0; j < m_nGroups; j++)
   {
     for (uint32_t i = 0; i < 8; i++)
      {
        std::cout << "index(" << i << ") = " << station->m_mcsTable[j].m_minstrelTable[i].perfectTxTime << "\n";
      }
   }
}

} // namespace ns3






