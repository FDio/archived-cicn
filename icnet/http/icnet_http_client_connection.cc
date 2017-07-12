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

#include "icnet_http_client_connection.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

#define DEFAULT_BETA 0.99
#define DEFAULT_GAMMA 0.07

namespace icnet {

namespace http {

using namespace transport;

HTTPClientConnection::HTTPClientConnection()
    : consumer_(Name("ccnx:"), transport::TransportProtocolAlgorithms::RAAQM) {

  consumer_.setSocketOption(ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY,
                            (ConsumerContentObjectVerificationCallback) std::bind(&HTTPClientConnection::verifyData,
                                                                                  this,
                                                                                  std::placeholders::_1,
                                                                                  std::placeholders::_2));

  consumer_.setSocketOption(ConsumerCallbacksOptions::CONTENT_RETRIEVED,
                            (ConsumerContentCallback) std::bind(&HTTPClientConnection::processPayload,
                                                                this,
                                                                std::placeholders::_1,
                                                                std::placeholders::_2));
}

HTTPClientConnection &HTTPClientConnection::get(std::string &url, HTTPHeaders headers, HTTPPayload payload) {

  HTTPRequest request(GET, url, headers, payload);

  std::string &request_string = request.getRequestString();
  std::string &locator = request.getLocator();
  std::string &path = request.getPath();

  consumer_.setSocketOption(ConsumerCallbacksOptions::INTEREST_OUTPUT,
                            (ConsumerInterestCallback) std::bind(&HTTPClientConnection::processLeavingInterest,
                                                                 this,
                                                                 std::placeholders::_1,
                                                                 std::placeholders::_2,
                                                                 request_string));

  // Send content to producer piggybacking it through first interest (to fix)

  response_.clear();

  // Factor icn name

  std::stringstream stream;

  stream << "ccnx:/";
  stream << locator << "/get";
  stream << path;

  consumer_.consume(Name(stream.str()));

  consumer_.stop();

  return *this;
}

HTTPResponse &&HTTPClientConnection::response() {
  return std::move(response_);
}

void HTTPClientConnection::processPayload(transport::ConsumerSocket &c,
                                          std::vector<uint8_t> &&payload) {
  response_ = std::move(payload);
}

bool HTTPClientConnection::verifyData(transport::ConsumerSocket &c, const ContentObject &contentObject) {
  if (contentObject.getPayloadType() == PayloadType::DATA) {
    std::cout << "VERIFY CONTENT" << std::endl;
  } else if (contentObject.getPayloadType() == PayloadType::MANIFEST) {
    std::cout << "VERIFY MANIFEST" << std::endl;
  }

  return true;
}

void HTTPClientConnection::processLeavingInterest(transport::ConsumerSocket &c,
                                                  const Interest &interest,
                                                  std::string &payload) {
  if (interest.getName().get(-1).toSegment() == 0) {
    Interest &int2 = const_cast<Interest &>(interest);
    int2.setPayload((const uint8_t *) payload.data(), payload.size());
  }
}

HTTPClientConnection& HTTPClientConnection::stop() {
  // This is thread safe and can be called from another thread
  consumer_.stop();

  return *this;
}

transport::ConsumerSocket& HTTPClientConnection::getConsumer() {
  return consumer_;
}

}

}
