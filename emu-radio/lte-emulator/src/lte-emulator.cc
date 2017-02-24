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

#include "lte-emulator.h"
#include <ns3/mobility-module.h>

namespace ns3
{
namespace emulator
{

LteEmulator::LteEmulator (std::unordered_map <std::string, ns3::Ptr<ns3::Node>> &mapNameNode, Ptr <LteTapHelper> ltehelper)
    : m_mapNameNs3node (mapNameNode),
      m_lteTapHelper (ltehelper)
{
}

bool LteEmulator::getTransmissionRate (const std::string &station, double *transmissionRate)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
    {
      int nodeId = (m_mapNameNs3node[station])->GetId ();
      *transmissionRate = m_lteTapHelper->GetLtePhyTxRate (nodeId);

      return true;
    }

  return false;
}

bool LteEmulator::setStationCoordinates (const std::string &station, double x, double y)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
    {
      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <MobilityModel> staMobilityModel = sta_ptr->GetObject<MobilityModel> ();
      staMobilityModel->SetPosition (Vector (x, y, 0.0));
      return true;
    }

  return false;
}

bool LteEmulator::setStationXCoordinate (const std::string &station, double x)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
    {
      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <MobilityModel> staMobilityModel = sta_ptr->GetObject<MobilityModel> ();
      staMobilityModel->SetPosition (Vector (x, staMobilityModel->GetPosition ()
                                                                .y, 0.0));
      return true;
    }

  return false;
}

bool LteEmulator::setStationYCoordinate (const std::string &station, double y)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
    {
      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <MobilityModel> staMobilityModel = sta_ptr->GetObject<MobilityModel> ();
      staMobilityModel->SetPosition (Vector (staMobilityModel->GetPosition ()
                                                             .x, y, 0.0));
      return true;
    }

  return false;
}

std::pair<double, double> LteEmulator::getStationCoordinates (const std::string &station)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
    {
      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <MobilityModel> staMobilityModel = sta_ptr->GetObject<MobilityModel> ();
      return {staMobilityModel->GetPosition ()
                              .x, staMobilityModel->GetPosition ()
                                                  .y};
    }

  return {};
}

bool LteEmulator::getStationXCoordinate (const std::string &station, double *x)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
    {
      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <MobilityModel> staMobilityModel = sta_ptr->GetObject<MobilityModel> ();
      *x = staMobilityModel->GetPosition ()
                           .x;
      return true;
    }

  return false;
}

bool LteEmulator::getStationYCoordinate (const std::string &station, double *y)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
    {
      Ptr <Node> sta_ptr = m_mapNameNs3node[station];
      Ptr <MobilityModel> staMobilityModel = sta_ptr->GetObject<MobilityModel> ();
      *y = staMobilityModel->GetPosition ()
                           .y;
      return true;
    }

  return false;
}

LteEmulator &
LteEmulator::moveStationAlongSegment (const std::string &station, double start_x, double start_y, double end_x, double end_y, double duration)
{
  if (m_mapNameNs3node.find (station) == m_mapNameNs3node.end ())
    {
      std::cerr << "The station [" << station << "] does not exist in this simulation!" << std::endl;
    }
  else
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
