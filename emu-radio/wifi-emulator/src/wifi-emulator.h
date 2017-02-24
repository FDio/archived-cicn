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

#ifndef WIFI_EMULATOR_WIFIEMULATOR_H
#define WIFI_EMULATOR_WIFIEMULATOR_H

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/wifi-module.h>
#include <ns3/mobility-module.h>
#include <ns3/tap-bridge-module.h>
#include <boost/algorithm/string.hpp>
#include <future>
#include <unordered_map>

#include "emulator.h"

#define CONSTANT_POSITION "constant_position"
#define RANDOM_WAYPOINT   "random_waypoint"

// NS_LOG_COMPONENT_DEFINE ("wifi-emulator");

#define SSID "ns-3-ssid"

namespace ns3
{
namespace emulator
{

class WifiEmulator : public Emulator {

 public:
  explicit WifiEmulator (unsigned int accessPointNumber, unsigned int stationNumber);

  WifiEmulator &setWifi (WifiPhyStandard standard);

  WifiEmulator &setMobility (double bs_x = 0, double bs_y = 0, double initialDistance = 0);

  WifiEmulator &
  setTapDevices (std::list <std::string> &ap_list, std::list <std::string> &station_list, std::list <std::string> &sta_taps_list, std::list <std::string> &ap_taps_list, std::list <std::string> &sta_macs_list, std::list <std::string> &ap_macs_list);

  WifiEmulator &runEmulation (bool async = false);

  WifiEmulator &stopEmulation ();

  std::pair<double, double> getStationCoordinates (const std::string &station);

  bool setStationCoordinates (const std::string &station, double x, double y);

  bool setStationXCoordinate (const std::string &station, double x);

  bool getStationYCoordinate (const std::string &station, double *y);

  bool getStationXCoordinate (const std::string &station, double *x);

  bool setStationYCoordinate (const std::string &station, double y);

  WifiEmulator &
  moveStationAlongSegment (const std::string &station, double start_x, double start_y, double end_x, double end_y, double duration);

  bool getTransmissionRate (const std::string &station, double *transmissionRate);

 public:

  static std::map<uint64_t, double> McsRateMap;

 private:

  Ptr <MobilityModel> getMobilityModel (const std::string &station);

  void installNWifi (WifiPhyStandard standard = WIFI_PHY_STANDARD_80211n_5GHZ);

  void logNewTransmissionRate (std::string context, const uint64_t rate, const Mac48Address remoteAddress);

  bool checkIfExist (const std::string &node);

 private:
  unsigned int m_accessPointNumber;
  unsigned int m_stationNumber;

  NodeContainer m_wifiStaNodes;
  NodeContainer m_wifiApNodes;

  NetDeviceContainer m_staDevices;
  NetDeviceContainer m_apDevices;

  MobilityHelper m_apMobility;
  MobilityHelper m_staMobility;

  Ptr <ListPositionAllocator> m_apPositionAlloc;
  Ptr <ListPositionAllocator> m_staPositionAlloc;

  YansWifiChannelHelper m_channel;
  YansWifiPhyHelper m_phy;
  WifiHelper m_wifi;
  HtWifiMacHelper m_mac;
  Ssid m_ssid;

  std::unordered_map <std::string, ns3::Ptr<ns3::Node>> m_mapNameNs3node;
  std::future<void> m_simulationHandle;

  std::unordered_map<std::string, double> m_avgTransmissionRate;

  double m_alpha;

};

} // End namespace emulator

} // End namespace ns3

#endif //WIFI_EMULATOR_WIFIEMULATOR_H
