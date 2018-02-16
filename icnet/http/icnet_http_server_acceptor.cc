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

#include "icnet_http_server_acceptor.h"
#include "icnet_http_request.h"
#include "icnet_errors.h"
#include "icnet_utils_uri.h"
#include "icnet_utils_hash.h"

namespace icnet {

namespace http {

HTTPServerAcceptor::HTTPServerAcceptor(std::string &&server_locator, OnHttpRequest callback)
    : HTTPServerAcceptor(server_locator, callback) {
}

HTTPServerAcceptor::HTTPServerAcceptor(std::string &server_locator, OnHttpRequest callback)
    : callback_(callback) {
  utils::Uri uri;

  uri.parseProtocolAndLocator(server_locator);
  std::string protocol = uri.getProtocol();
  std::string locator = uri.getLocator();

  std::transform(locator.begin(),
                 locator.end(),
                 locator.begin(),
                 ::tolower);

  std::transform(protocol.begin(),
                 protocol.end(),
                 protocol.begin(),
                 ::tolower);

  if (protocol != "http") {
    throw errors::RuntimeException("Malformed server_locator. The locator format should be in the form http://locator");
  }

  std::stringstream ss;

  ss << "ccnx:/";
  ss << locator;

  acceptor_producer_ = std::make_shared<transport::ProducerSocket>(ss.str());
}

void HTTPServerAcceptor::listen(bool async) {
  acceptor_producer_->setSocketOption(icnet::transport::ProducerCallbacksOptions::INTEREST_INPUT,
                                      (icnet::transport::ProducerInterestCallback) bind(&HTTPServerAcceptor::processIncomingInterest,
                                                                                        this,
                                                                                        std::placeholders::_1,
                                                                                        std::placeholders::_2));
  acceptor_producer_->dispatch();

  if (!async) {
    acceptor_producer_->serveForever();
  }
}

HttpRequest &&HTTPServerAcceptor::request() {
  return std::move(request_);
}

void HTTPServerAcceptor::processIncomingInterest(transport::ProducerSocket &p, const transport::Interest &interest) {
  // Temporary solution

  transport::Name complete_name = interest.getName();

  transport::Name request_name = complete_name.get(-1).isSegment() ? complete_name.getPrefix(-1) : complete_name;
  transport::Name prefix;
  acceptor_producer_->getSocketOption(transport::GeneralTransportOptions::NAME_PREFIX, prefix);

  // Get the name of the HTTP method to compute
  std::string method = request_name.get(prefix.getSegmentCount()).toString();
  std::transform(method.begin(), method.end(), method.begin(), ::toupper);
  std::string path;
  std::string url_begin;

  // This is done for getting rid of useless name components such as ccnx: or ndn:
  if (request_name.getSegmentCount() > 2) {
    std::string raw_path = request_name.getSubName(prefix.getSegmentCount() + 1).toString();
    std::size_t pos = raw_path.find("/");
    path = raw_path.substr(pos);
    url_begin = prefix.getSubName(0).toString().substr(pos);
  }

  std::stringstream ss;
  ss << "http:/" << url_begin << path;

  std::string url = ss.str();
  HTTPHeaders headers = {};
  HTTPPayload payload = {};

  if (method == "GET") {
    HTTPRequest request(GET, url, headers, payload);

    int request_id = utils::Hash::hash32(request.getRequestString().data(), request.getRequestString().size());

    if (publishers_.find(request_id) != publishers_.end()) {
      if (publishers_[request_id]) {
        publishers_[request_id]->getProducer().onInterest(interest.getName(), interest);
        return;
      }
    } else {
      std::cout << "Request ID" << request_id << std::endl;
    }

    publishers_[request_id] = std::make_shared<HTTPServerPublisher>(request_name);
    callback_(publishers_[request_id],
              (uint8_t *) request.getRequestString().data(),
              request.getRequestString().size(),
              request_id);
  }
}

std::map<int, std::shared_ptr<HTTPServerPublisher>>& HTTPServerAcceptor::getPublishers() {
  return publishers_;
}

}

}