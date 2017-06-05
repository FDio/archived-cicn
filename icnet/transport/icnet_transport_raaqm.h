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

#ifndef ICNET_RAAQM_TRANSPORT_PROTOCOL_H_
#define ICNET_RAAQM_TRANSPORT_PROTOCOL_H_

#include "icnet_transport_vegas.h"
#include "icnet_transport_vegas_rto_estimator.h"
#include "icnet_transport_raaqm_data_path.h"
#include "icnet_transport_rate_estimation.h"

namespace icnet {

namespace transport {

class RaaqmTransportProtocol : public VegasTransportProtocol {
 public:
  RaaqmTransportProtocol(Socket *icnet_socket);

  ~RaaqmTransportProtocol();

  void start();

 protected:
  void copyContent(ContentObject &content_object);

 private:

  void init();

  void reset();

  void afterContentReception(const Interest &interest, const ContentObject &content_object);

  void afterDataUnsatisfied(uint64_t segment);

  void increaseWindow();

  void updateRtt(uint64_t segment);

  void decreaseWindow();

  void changeInterestLifetime(uint64_t segment);

  void onTimeout(const Interest &interest);

  void RAAQM();

  void updatePathTable(const ContentObject &content_object);

  void checkForFastRetransmission(const Interest &interest);

  void check_drop_probability();

  void check_for_stale_paths();

  void printRtt();

  /**
   * Current download path
   */
  std::shared_ptr<RaaqmDataPath> cur_path_;

  /**
   * Hash table for path: each entry is a pair path ID(key) - path object
   */
  std::unordered_map<unsigned char, std::shared_ptr<RaaqmDataPath>> path_table_;

  bool set_interest_filter_;
  //for rate-estimation at packet level
  IcnRateEstimator *rate_estimator_;

  //params for autotuning
  bool raaqm_autotune_;
  double default_beta_;
  double default_drop_;
  double beta_wifi_;
  double drop_wifi_;
  double beta_lte_;
  double drop_lte_;
  unsigned int wifi_delay_;
  unsigned int lte_delay_;

  //RTT stats
  double avg_rtt_;
};

} // end namespace transport

} // end namespace icnet

#endif // ICNET_RAAQM_TRANSPORT_PROTOCOL_H_
