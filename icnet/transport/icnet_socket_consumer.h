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

#ifndef ICNET_CONSUMER_SOCKET_H_
#define ICNET_CONSUMER_SOCKET_H_

#include "icnet_common.h"
#include "icnet_socket.h"
#include "icnet_transport.h"
#include "icnet_transport_raaqm.h"
#include "icnet_transport_vegas.h"

#define CONSUMER_READY 0
#define CONSUMER_BUSY  1

namespace icnet {

class ConsumerSocket : public Socket {
 public:
  explicit ConsumerSocket(const Name prefix, int protocol);

  ~ConsumerSocket();

  int consume(Name suffix);

  int asyncConsume(Name suffix);

  void stop();

  int setSocketOption(int socket_option_key, int socket_option_value);

  int setSocketOption(int socket_option_key, double socket_option_value);

  int setSocketOption(int socket_option_key, bool socket_option_value);

  int setSocketOption(int socket_option_key, size_t socket_option_value);

  int setSocketOption(int socket_option_key, Name socket_option_value);

  int setSocketOption(int socket_option_key, ProducerContentObjectCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerContentObjectVerificationCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerContentObjectCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerInterestCallback socket_option_value);

  int setSocketOption(int socket_option_key, ProducerInterestCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerContentCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerManifestCallback socket_option_value);

  int setSocketOption(int socket_option_key, KeyLocator socket_option_value);

  int setSocketOption(int socket_option_key, IcnObserver *socket_option_value);

  int getSocketOption(int socket_option_key, int &socket_option_value);

  int getSocketOption(int socket_option_key, double &socket_option_value);

  int getSocketOption(int socket_option_key, size_t &socket_option_value);

  int getSocketOption(int socket_option_key, bool &socket_option_value);

  int getSocketOption(int socket_option_key, Name &socket_option_value);

  int getSocketOption(int socket_option_key, ProducerContentObjectCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerContentObjectVerificationCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerContentObjectCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerInterestCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ProducerInterestCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerContentCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerManifestCallback &socket_option_value);

  int getSocketOption(int socket_option_key, KeyLocator &socket_option_value);

  int getSocketOption(int socket_option_key, std::shared_ptr<Portal> &socket_option_value);

  int getSocketOption(int socket_option_key, IcnObserver **socket_option_value);

 private:

  void postponedConsume(Name name_suffix);

 private:
  // context inner state variables
  bool is_running_;
  std::shared_ptr<Portal> portal_;
  std::shared_ptr<TransportProtocol> transport_protocol_;

  Name name_prefix_;
  Name name_suffix_;

  int interest_lifetime_;

  double min_window_size_;
  double max_window_size_;
  double current_window_size_;
  int max_retransmissions_;
  size_t output_buffer_size_;
  size_t input_buffer_size_;

  // RAAQM Parameters

  double minimum_drop_probability_;
  unsigned int sample_number_;
  double gamma_;
  double beta_;
  double drop_factor_;

  //Rate estimation parameters
  double rate_estimation_alpha_;
  IcnObserver *rate_estimation_observer_;
  int rate_estimation_batching_parameter_;
  int rate_estimation_choice_;

  bool is_async_;

  KeyLocator key_locator_;

  ConsumerInterestCallback on_interest_retransmission_;
  ConsumerInterestCallback on_interest_output_;
  ConsumerInterestCallback on_interest_timeout_;
  ConsumerInterestCallback on_interest_satisfied_;

  ConsumerContentObjectCallback on_content_object_input_;
  ConsumerContentObjectVerificationCallback on_content_object_verification_;

  ConsumerContentObjectCallback on_content_object_;
  ConsumerManifestCallback on_manifest_;

  ConsumerContentCallback on_payload_retrieved_;

  // Virtual download for traffic generator

  bool virtual_download_;
  bool rtt_stats_;
};

} // end namespace icnet

#endif // ICNET_CONSUMER_SOCKET_H_
