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

#ifndef ICNET_VEGAS_TRANSPORT_PROTOCOL_RTT_ESTIMATOR_H_
#define ICNET_VEGAS_TRANSPORT_PROTOCOL_RTT_ESTIMATOR_H_

#include "icnet_common.h"

// Implementation inspired from RFC6298 (https://tools.ietf.org/search/rfc6298#ref-JK88)

namespace icnet {

class RtoEstimator {
 public:
  typedef std::chrono::microseconds Duration;

  static Duration getInitialRtt() {
    return std::chrono::seconds(1);
  }

  RtoEstimator(Duration min_rto = std::chrono::seconds(1));

  void addMeasurement(Duration measure);

  Duration computeRto() const;

 private:
  double smoothed_rtt_;
  double rtt_variation_;
  bool first_measurement_;
  double last_rto_;
};

} // end namespace icnet


#endif // ICNET_VEGAS_TRANSPORT_PROTOCOL_RTT_ESTIMATOR_H_
