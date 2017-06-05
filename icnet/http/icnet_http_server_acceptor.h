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
#include "icnet_http_server_publisher.h"

#include <vector>
#include <functional>

#define HTTP_VERSION "1.0"

namespace icnet {

namespace http {

//typedef std::vector<uint8_t> HTTPResponse;
typedef std::vector<uint8_t> HttpRequest;
typedef std::function<void(std::shared_ptr<HTTPServerPublisher> &, const uint8_t *, std::size_t)> OnHttpRequest;

class HTTPServerAcceptor {
 public:
  HTTPServerAcceptor(std::string &&server_locator, OnHttpRequest callback);
  HTTPServerAcceptor(std::string &server_locator, OnHttpRequest callback);

  void listen(bool async);

  HttpRequest &&request();

//  void asyncSendResponse();

//  HTTPClientConnection& get(std::string &url, HTTPHeaders headers = {}, HTTPPayload payload = {});
//
//  HTTPResponse&& response();

 private:

  void processIncomingInterest(transport::ProducerSocket &p, const transport::Interest &interest);

  OnHttpRequest callback_;
  HttpRequest request_;
  std::shared_ptr<transport::ProducerSocket> acceptor_producer_;
};

} // end namespace http

} // end namespace icnet