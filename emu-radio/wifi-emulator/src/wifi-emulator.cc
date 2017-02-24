/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wifi-emulator.h"

namespace ns3
{
namespace emulator
{

std::map<uint64_t, double> WifiEmulator::McsRateMap = {{0, 15.0},
                                                       {1, 30.0},
                                                       {2, 45.0},
                                                       {3, 60.0},
                                                       {4, 90.0},
                                                       {5, 120.0},
                                                       {6, 135.0},
                                                       {7, 150.0}};

WifiEmulator::WifiEmulator (unsigned int accessPointNumber, unsigned int stationNumber)
    : m_accessPointNumber (accessPointNumber), m_stationNumber (stationNumber), m_channel (YansWifiChannelHelper::Default ()), m_phy (YansWifiPhyHelper::Default ()), m_wifi (WifiHelper::Default ()), m_mac (HtWifiMacHelper::Default ()), m_ssid (SSID), m_avgTransmissionRate (0), m_alpha (0.1)
{
  //
  // We are interacting with the outside, real, world.  This means we have to
  // interact in real-time and therefore means we have to use the real-time
  // simulator and take the time to calculate checksums.
  //
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  m_wifiStaNodes.Create (stationNumber);
  m_wifiApNodes.Create (accessPointNumber);
}

WifiEmulator &WifiEmulator::setWifi (WifiPhyStandard standard)
{
  switch (standard)
    {
      case WIFI_PHY_STANDARD_80211n_5GHZ:
        installNWifi (standard);
      break;
      default:
        std::cerr << "The wifi standard requested is not available!" << std::endl;
      break;
    }

  return *this;
}

void WifiEmulator::installNWifi (WifiPhyStandard standard)
{
  // Default propagation loss model.
  m_channel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel", "m0", DoubleValue (1.0), "m1", DoubleValue (1.0), "m2", DoubleValue (1.0));

  m_phy.SetChannel (m_channel.Create ());
  m_phy.Set ("ShortGuardEnabled", BooleanValue (1));

  m_wifi.SetStandard (standard);

  m_wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");

  m_mac.SetMpduAggregatorForAc (AC_BE, "ns3::MpduStandardAggregator"); // A-MPDU  of max length 65535 bytes
  m_mac.SetMsduAggregatorForAc (AC_BE, "ns3::MsduStandardAggregator"); // A-MSDU of max length 7935 bytes
  m_mac.SetBlockAckThresholdForAc (AC_BE, 2); // block acknowledgement of 5 MPDU
  m_mac.SetBlockAckInactivityTimeoutForAc (AC_BE, 400);

  m_mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (m_ssid), "ActiveProbing", BooleanValue (false));

  m_staDevices = m_wifi.Install (m_phy, m_mac, m_wifiStaNodes);

  m_mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (m_ssid));

  m_apDevices = m_wifi.Install (m_phy, m_mac, m_wifiApNodes);

  // Set callbacks and values
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (40));

  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/$ns3::MinstrelHtWifiManager/RateChange", MakeCallback (&WifiEmulator::logNewTransmissionRate, this));
}

WifiEmulator &WifiEmulator::setMobility (double bs_x, double bs_y, double initialDistance)
{
  // Access point mobility
  MobilityHelper apMobility;
  Ptr <ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
  apPositionAlloc->Add (Vector (bs_x, bs_y, 0.0));
  apMobility.SetPositionAllocator (apPositionAlloc);
  apMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  apMobility.Install (m_wifiApNodes);

  // Station mobility. By default the stations start from the same position of the access point
  MobilityHelper staMobility;
  Ptr <ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();
  staPositionAlloc->Add (Vector (bs_x, bs_y + initialDistance, 0.0));
  staMobility.SetPositionAllocator (staPositionAlloc);
  staMobility.SetMobilityModel ("ns3::WaypointMobilityModel", "InitialPositionIsWaypoint", BooleanValue (false));
  staMobility.Install (m_wifiStaNodes);

  return *this;
}

WifiEmulator &
WifiEmulator::setTapDevices (std::list <std::string> &ap_list, std::list <std::string> &station_list, std::list <std::string> &sta_taps_list, std::list <std::string> &ap_taps_list, std::list <std::string> &sta_macs_list, std::list <std::string> &ap_macs_list)
{
  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute ("Mode", StringValue ("UseLocal"));

  std::list<std::string>::const_iterator station;
  std::list<std::string>::const_iterator ap;
  std::list<std::string>::const_iterator sta_tap;
  std::list<std::string>::const_iterator sta_mac;
  std::list<std::string>::const_iterator ap_tap;
  std::list<std::string>::const_iterator ap_mac;

  uint32_t i;

  assert (station_list.size () == sta_taps_list.size () && sta_taps_list.size () == sta_macs_list.size ());
  assert (ap_list.size () == ap_taps_list.size () && ap_taps_list.size () == ap_macs_list.size ());

  for (sta_tap = sta_taps_list.begin (), sta_mac = sta_macs_list.begin (), station = station_list.begin (), i = 0;
       sta_tap != sta_taps_list.end () && sta_mac != sta_macs_list.end ()
       && station != station_list.end (); sta_tap++, sta_mac++, station++, i++)
    {
      m_mapNameNs3node[*station] = m_wifiStaNodes.Get (i);

      if (sta_mac->compare (""))
        {
          m_staDevices.Get (i)->SetAddress (Mac48Address (sta_mac->c_str ()));
        }

      tapBridge.SetAttribute ("DeviceName", StringValue (sta_tap->c_str ()));
      tapBridge.Install (m_wifiStaNodes.Get (i), m_staDevices.Get (i));
    }

  for (ap_tap = ap_taps_list.begin (), ap_mac = ap_macs_list.begin (), ap = ap_list.begin (), i = 0;
       ap_tap != ap_taps_list.end () && ap_mac != ap_macs_list.end ()
       && ap != ap_list.end (); ap_tap++, ap_mac++, ap++, i++)
    {
      m_mapNameNs3node[*ap] = m_wifiApNodes.Get (i);

      if (ap_mac->compare (""))
        {
          m_apDevices.Get (i)->SetAddress (Mac48Address (ap_mac->c_str ()));
        }

      tapBridge.SetAttribute ("DeviceName", StringValue (ap_tap->c_str ()));
      tapBridge.Install (m_wifiApNodes.Get (i), m_apDevices.Get (i));
    }

  return *this;
}

void WifiEmulator::logNewTransmissionRate (std::string context, const uint64_t rate, const Mac48Address remoteAddress)
{
  // TODO Linear search! Improve it with an hash table when there will be more stations!

  for (auto station : m_mapNameNs3node)
    {
      if (station.second->GetDevice (0)->GetAddress () == remoteAddress)
        {
          const std::string &sta = station.first;

          // Compute EWMA of the physical rate
          m_avgTransmissionRate[sta] = m_alpha * McsRateMap[rate] + (1.0 - m_alpha) * m_avgTransmissionRate[sta];
        }
    }
}

bool WifiEmulator::getTransmissionRate (const std::string &station, double *transmissionRate)
{
  if (m_avgTransmissionRate.find (station) != m_avgTransmissionRate.end ())
    {
      *transmissionRate = m_avgTransmissionRate[station];
      return true;
    }

  return false;
}

WifiEmulator &WifiEmulator::runEmulation (bool async)
{
  m_simulationHandle = std::async (std::launch::async, [] ()
  {
    Simulator::Stop ();
    Simulator::Run ();
    Simulator::Destroy ();
  });

  if (!async)
    {
      m_simulationHandle.get ();
    }

  return *this;
}

WifiEmulator &WifiEmulator::stopEmulation ()
{
  Simulator::Stop ();
//  Simulator::Destroy();
}

bool WifiEmulator::checkIfExist (const std::string &station)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
      return false;
    }

  return true;
}

Ptr <MobilityModel> WifiEmulator::getMobilityModel (const std::string &station)
{
  if (checkIfExist (station))
    {
      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <MobilityModel> staMobilityModel = sta_ptr->GetObject<MobilityModel> ();
      return staMobilityModel;
    }
  else
    {
      return nullptr;
    }
}

bool WifiEmulator::setStationCoordinates (const std::string &station, double x, double y)
{
  if (checkIfExist (station))
    {
      Ptr <MobilityModel> staMobilityModel = getMobilityModel (station);
      staMobilityModel->SetPosition (Vector (x, y, 0.0));
      return true;
    }

  return false;
}

bool WifiEmulator::setStationXCoordinate (const std::string &station, double x)
{
  if (checkIfExist (station))
    {
      Ptr <MobilityModel> staMobilityModel = getMobilityModel (station);
      staMobilityModel->SetPosition (Vector (x, staMobilityModel->GetPosition ().y, 0.0));
      return true;
    }

  return false;
}

bool WifiEmulator::setStationYCoordinate (const std::string &station, double y)
{
  if (checkIfExist (station))
    {
      Ptr <MobilityModel> staMobilityModel = getMobilityModel (station);
      staMobilityModel->SetPosition (Vector (staMobilityModel->GetPosition ().x, y, 0.0));
      return true;
    }

  return false;
}

std::pair<double, double> WifiEmulator::getStationCoordinates (const std::string &station)
{
  if (checkIfExist (station))
    {
      Ptr <MobilityModel> staMobilityModel = getMobilityModel (station);
      return {staMobilityModel->GetPosition ().x, staMobilityModel->GetPosition ().y};
    }

  return {};
}

bool WifiEmulator::getStationXCoordinate (const std::string &station, double *x)
{
  if (checkIfExist (station))
    {
      Ptr <MobilityModel> staMobilityModel = getMobilityModel (station);
      *x = staMobilityModel->GetPosition ().x;
      return true;
    }

  return false;
}

bool WifiEmulator::getStationYCoordinate (const std::string &station, double *y)
{
  if (checkIfExist (station))
    {
      Ptr <MobilityModel> staMobilityModel = getMobilityModel (station);
      *y = staMobilityModel->GetPosition ().y;
      return true;
    }

  return false;
}

WifiEmulator &
WifiEmulator::moveStationAlongSegment (const std::string &station, double start_x, double start_y, double end_x, double end_y, double duration)
{
  if (checkIfExist (station))
    {

      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <WaypointMobilityModel> staMobilityModel = sta_ptr->GetObject<WaypointMobilityModel> ();

      if (staMobilityModel)
        {

          staMobilityModel->SetPosition (Vector (start_x, start_y, 0.0));
          staMobilityModel->AddWaypoint (Waypoint (Seconds (Simulator::Now ().GetSeconds ()), Vector3D (start_x, start_y, 0.0)));
          staMobilityModel->AddWaypoint (Waypoint (
              Seconds (Simulator::Now ().GetSeconds ()) + Seconds (duration), Vector3D (end_x, end_y, 0.0)));
        }
      else
        {
          std::cerr << "Access point has a constant position mobility model. Impossible to move it" << std::endl;
        }
    }

  return *this;
}

} // End namespace ns3
} // End namespace emulator