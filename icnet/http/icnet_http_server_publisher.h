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

#pragma once

#include "icnet_transport_socket_consumer.h"
#include "icnet_transport_socket_producer.h"
#include "icnet_http_default_values.h"

#include <vector>
#include <functional>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#define HTTP_VERSION "1.0"

namespace icnet {

namespace http {

typedef std::vector<uint8_t> HttpRequest;
typedef std::function<void(const boost::system::error_code e)> DeadlineTimerCallback;

class HTTPServerPublisher {
 public:
  HTTPServerPublisher(const transport::Name &content_name);

  ~HTTPServerPublisher();

  void publishContent(const uint8_t *buf,
                      size_t buffer_size,
                      const int response_id,
                      bool is_last);

  void serveClients();

  void stop();

  HTTPServerPublisher &setTimeout(uint32_t timeout);

  HTTPServerPublisher &attachPublisher();

 private:

  void processIncomingInterest(transport::ProducerSocket &p, const transport::Interest &interest);

  transport::Name content_name_;
  std::unique_ptr<boost::asio::deadline_timer> timer_;
  std::unique_ptr<transport::ProducerSocket> producer_;
  transport::ProducerInterestCallback interest_enter_callback_;
  DeadlineTimerCallback wait_callback_;
};

} // end namespace http

} // end namespace icnet
