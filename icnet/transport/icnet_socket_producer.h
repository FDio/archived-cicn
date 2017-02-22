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

#ifndef ICNET_PRODUCER_SOCKET_H_
#define ICNET_PRODUCER_SOCKET_H_

#include "icnet_socket.h"
#include "icnet_content_store.h"

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>

#define REGISTRATION_NOT_ATTEMPTED 0
#define REGISTRATION_SUCCESS 1
#define REGISTRATION_FAILURE 2
#define REGISTRATION_IN_PROGRESS 3

namespace icnet {

class ProducerSocket : public Socket {
 public:

  explicit ProducerSocket(Name prefix);

  ~ProducerSocket();

  void attach();

  void dispatch();

  void produce(Name suffix, const uint8_t *buffer, size_t buffer_size, const int request_id = 0, bool is_last = false);

  void produce(ContentObject &content_object);

  void asyncProduce(Name suffix, const uint8_t *buf, size_t buffer_size, const int response_id, bool is_last);

  void asyncProduce(ContentObject &content_object);

  void serveForever();

  void onInterest(const Name &name, const Interest &interest);

  int setSocketOption(int socket_option_key, int socket_option_value);

  int setSocketOption(int socket_option_key, double socket_option_value);

  int setSocketOption(int socket_option_key, bool socket_option_value);

  int setSocketOption(int socket_option_key, size_t socket_option_value);

  int setSocketOption(int socket_option_key, Name socket_option_value);

  int setSocketOption(int socket_option_key, ProducerContentObjectCallback socket_option_value);

  int setSocketOption(int socket_option_key, ProducerInterestCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerContentObjectVerificationCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerContentObjectCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerInterestCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerContentCallback socket_option_value);

  int setSocketOption(int socket_option_key, ConsumerManifestCallback socket_option_value);

  int setSocketOption(int socket_option_key, KeyLocator socket_option_value);

  int setSocketOption(int socket_option_key, IcnObserver *obs);

  int getSocketOption(int socket_option_key, int &socket_option_value);

  int getSocketOption(int socket_option_key, double &socket_option_value);

  int getSocketOption(int socket_option_key, bool &socket_option_value);

  int getSocketOption(int socket_option_key, size_t &socket_option_value);

  int getSocketOption(int socket_option_key, Name &socket_option_value);

  int getSocketOption(int socket_option_key, ProducerContentObjectCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ProducerInterestCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerContentObjectVerificationCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerContentObjectCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerInterestCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerContentCallback &socket_option_value);

  int getSocketOption(int socket_option_key, ConsumerManifestCallback &socket_option_value);

  int getSocketOption(int socket_option_key, KeyLocator &socket_option_value);

  int getSocketOption(int socket_option_key, std::shared_ptr<Portal> &socket_option_value);

  int getSocketOption(int socket_option_key, IcnObserver **socket_option_value);

 private:

  std::shared_ptr<Portal> portal_;
  boost::asio::io_service io_service_;

  Name name_prefix_;

  int data_packet_size_;
  int content_object_expiry_time_;
  int registration_status_;

  bool making_manifest_;

  // map for storing sequence numbers for several calls of the publish function
  std::unordered_map<std::string, std::unordered_map<int, uint64_t>> seq_number_map_;

  int signature_type_;
  int signature_size_;
  int key_locator_size_;
  KeyLocator key_locator_;

  // buffers
  ContentStore output_buffer_;

  std::queue<std::shared_ptr<const Interest> > input_buffer_;
  std::mutex input_buffer_mutex_;
  std::atomic_size_t input_buffer_capacity_;
  std::atomic_size_t input_buffer_size_;

  // threads
  std::thread listening_thread_;
  std::thread processing_thread_;
  volatile bool processing_thread_stop_;
  volatile bool listening_thread_stop_;

  // callbacks
  ProducerInterestCallback on_interest_input_;
  ProducerInterestCallback on_interest_dropped_input_buffer_;
  ProducerInterestCallback on_interest_inserted_input_buffer_;
  ProducerInterestCallback on_interest_satisfied_output_buffer_;
  ProducerInterestCallback on_interest_process_;

  ProducerContentObjectCallback on_new_segment_;
  ProducerContentObjectCallback on_content_object_to_sign_;
  ProducerContentObjectCallback on_content_object_in_output_buffer_;
  ProducerContentObjectCallback on_content_object_output_;
  ProducerContentObjectCallback on_content_object_evicted_from_output_buffer_;

 private:
  void listen();

  void passContentObjectToCallbacks(const std::shared_ptr<ContentObject> &content_object);

};

}

#endif // ICNET_PRODUCER_SOCKET_H_
