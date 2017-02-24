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

#ifndef RADIO_EMULATION_EMULATOR_H
#define RADIO_EMULATION_EMULATOR_H

namespace ns3
{
namespace emulator
{

class Emulator {
 public:
  virtual ~Emulator ()
  {
  }

  virtual std::pair<double, double> getStationCoordinates (const std::string &station) = 0;

  virtual bool setStationCoordinates (const std::string &station, double x, double y) = 0;

  virtual bool setStationXCoordinate (const std::string &station, double x) = 0;

  virtual bool getStationYCoordinate (const std::string &station, double *y) = 0;

  virtual bool getStationXCoordinate (const std::string &station, double *x) = 0;

  virtual bool setStationYCoordinate (const std::string &station, double y) = 0;

  virtual Emulator &
  moveStationAlongSegment (const std::string &station, double start_x, double start_y, double end_x, double end_y, double duration) = 0;

  virtual bool getTransmissionRate (const std::string &station, double *transmissionRate) = 0;

};

}
}

#endif //RADIO_EMULATION_EMULATOR_H
