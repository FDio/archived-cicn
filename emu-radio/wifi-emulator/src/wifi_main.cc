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
#include "connection-pool.h"
#include "query.h"
#include "communication-protocol.h"

#define DEFAULT_EXPERIMENT_ID "wifi-emulation"
#define N_AP            1

using namespace ns3;

namespace ns3
{
namespace emulator
{

typedef struct EmulationParameters {

  std::string bs_name = "";
  std::string bs_tap = "";
  std::string sta_list_str = "";
  std::string sta_taps_str = "";
  std::string sta_macs_str = "";
  std::string bs_mac_str = "";
  std::string experiment_id_str = DEFAULT_EXPERIMENT_ID;

  double bs_x = 0;
  double bs_y = 0;
  double distance = 0;
  uint32_t n_sta = 0;
  uint16_t control_port = 0;

  void parseParameters (int argc, char **argv)
  {
    CommandLine cmd;
    cmd.AddValue<std::string> ("bs-tap", "Name of the tap between NS3 and the base station", bs_tap);
    cmd.AddValue<std::string> ("sta-list", "List of the stations of the simulation", sta_list_str);
    cmd.AddValue<std::string> ("sta-taps", "List of the taps between NS3 and the mobile stations", sta_taps_str);
    cmd.AddValue<std::string> ("sta-macs", "List of the macs of the mobile stations", sta_macs_str);
    cmd.AddValue<double> ("bs-x", "X position of the Base Station", bs_x);
    cmd.AddValue<double> ("bs-y", "Y position of the Base Station", bs_y);
    cmd.AddValue<std::string> ("experiment-id", "Distance between the station and the base station", experiment_id_str);
    cmd.AddValue<std::string> ("bs-name", "Index of the base station", bs_name);
    cmd.AddValue<std::string> ("bs-mac", "Base station MAC address", bs_mac_str);
    cmd.AddValue<uint16_t> ("control-port", "Control port for dynamically managing the stations movement", control_port);
    cmd.AddValue<double> ("distance", "Initial distance between the bs and the other stations", distance);
    cmd.AddValue<uint32_t> ("n-sta", "Number of stations in the simulation", n_sta);

    cmd.Parse (argc, argv);
  }

  bool checkMissingParameters ()
  {
    if (bs_tap == "" || n_sta == 0 || sta_list_str == "" || sta_taps_str == "" || sta_macs_str == "" || bs_name == ""
        || control_port == 0)
      {
        return false;
      }

    return true;
  }

} EmulationParameters;

int main (int argc, char **argv)
{
  EmulationParameters emulationParameters;

  emulationParameters.parseParameters (argc, argv);

  if (!emulationParameters.checkMissingParameters ())
    {
      std::cerr << "Important parameters are missing!" << std::endl;
      return -1;
    }

  WifiEmulator emulator (N_AP, emulationParameters.n_sta);

  std::list <std::string> ap_list;
  std::list <std::string> station_list;
  std::list <std::string> sta_taps_list;
  std::list <std::string> ap_taps_list;
  std::list <std::string> sta_macs_list;
  std::list <std::string> ap_macs_list;

  boost::split (ap_list, emulationParameters.bs_name, boost::is_any_of (","));

  boost::split (station_list, emulationParameters.sta_list_str, boost::is_any_of (","));

  boost::split (sta_macs_list, emulationParameters.sta_macs_str, boost::is_any_of (","));

  boost::split (ap_macs_list, emulationParameters.bs_mac_str, boost::is_any_of (","));

  boost::split (sta_taps_list, emulationParameters.sta_taps_str, boost::is_any_of (","));

  boost::split (ap_taps_list, emulationParameters.bs_tap, boost::is_any_of (","));

  emulator
      .setWifi (WIFI_PHY_STANDARD_80211n_5GHZ)
      .setMobility (emulationParameters.bs_x, emulationParameters.bs_y, emulationParameters.distance)
      .setTapDevices (ap_list, station_list, sta_taps_list, ap_taps_list, sta_macs_list, ap_macs_list).runEmulation (true);

  // Handler function for outcoming connections

  CommunicationProtocol protocol;

  HandlerFunction handler = [&emulator, &protocol] (Server *s, websocketpp::connection_hdl hdl, message_ptr msg, const uint8_t *data, std::size_t size)
  {

    std::string command ((char *) data, size);
    boost::trim (command);

    std::cout << command << std::endl;

    Query query = Query::fromJsonString (command);

    protocol.processQuery (s, hdl, msg, emulator, query);
  };

  ConnectionPool connPool (emulationParameters.control_port, 9000);

  std::cout << "Starting listeners" << std::endl;

  connPool.startListeners (handler).processEvents ();

  // If we reach this point the control servers have stopped, that means we can also stop the simulation.

  emulator.stopEmulation ();

}

} // End namespace emulator
} // End namespace ns3

int main (int argc, char *argv[])
{
  return ns3::emulator::main (argc, argv);
}
