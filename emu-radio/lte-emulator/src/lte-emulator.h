/**
 * Copyright (c) 2016, Cisco Systems
 *
 * @author Mauro Sardara (msardara@cisco.com)
 */

#ifndef Lte_EMULATOR_LteEMULATOR_H
#define Lte_EMULATOR_LteEMULATOR_H

#include "../extensions/lte-tap-helper.h"
#include <unordered_map>

#include "emulator.h"

#define CONSTANT_POSITION "constant_position"
#define RANDOM_WAYPOINT   "random_waypoint"

//NS_LOG_COMPONENT_DEFINE ("Lte-emulator");

#define SSID "ns-3-ssid"

namespace ns3
{
namespace emulator
{

class LteEmulator : public Emulator {

 public:
  explicit LteEmulator (std::unordered_map <std::string, ns3::Ptr<ns3::Node>> &mapNameNode, Ptr <LteTapHelper> ltehelper);

  std::pair<double, double> getStationCoordinates (const std::string &station);

  bool setStationCoordinates (const std::string &station, double x, double y);

  bool setStationXCoordinate (const std::string &station, double x);

  bool getStationYCoordinate (const std::string &station, double *y);

  bool getStationXCoordinate (const std::string &station, double *x);

  bool setStationYCoordinate (const std::string &station, double y);

  LteEmulator &
  moveStationAlongSegment (const std::string &station, double start_x, double start_y, double end_x, double end_y, double duration);

  bool getTransmissionRate (const std::string &station, double *transmissionRate);

 private:

  std::unordered_map <std::string, ns3::Ptr<ns3::Node>> m_mapNameNs3node;

  Ptr <LteTapHelper> m_lteTapHelper;

};

} // End namespace emulator

} // End namespace ns3

#endif //Lte_EMULATOR_LteEMULATOR_H
