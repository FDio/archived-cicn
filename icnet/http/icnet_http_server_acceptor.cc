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
    auto publisher = std::make_shared<HTTPServerPublisher>(request_name);
    callback_(publisher, (uint8_t *) request.getRequestString().data(), request.getRequestString().size());
  }
}

//void HTTPServerConnection::sendResponse() {
//
//  std::thread t([]() {
//
//    // Get the name of the HTTP method to compute
//    std::string method = request_name.get(1).toString();
//    std::transform(method.begin(), method.end(), method.begin(), ::toupper);
//    std::string path;
//
//    // This is done for getting rid of useless name components such as ccnx: or ndn:
//    if (request_name.getSegmentCount() > 2) {
//      std::string rawPath = request_name.getSubName(2).toString();
//      std::size_t pos = rawPath.find("/");
//      path = rawPath.substr(pos);
//    }
//
//    // Create new producer
//
//    // Create timer for response availability
//    std::shared_ptr<core::Portal> portal;
//    po->getSocketOption(icnet::GeneralTransportOptions::PORTAL, portal);
//    boost::asio::io_service &ioService = portal->getIoService();
//
//    boost::asio::deadline_timer t(ioService, boost::posix_time::seconds(5));
//
//    std::function<void(const boost::system::error_code e)>
//        wait_callback = [&ioService](const boost::system::error_code e) {
//      if (!e) {
//        // Be sure to delete the timer before the io_service, otherwise we'll get some strange behavior!
//        ioService.stop();
//      }
//    };
//
//    std::function<void(transport::ProducerSocket, const core::Interest &interest)>
//        interest_enter_callback = [this, &wait_callback, &t]
//        (transport::ProducerSocket &p, const core::Interest &interest) {
//      t.cancel();
//      t.expires_from_now(boost::posix_time::seconds(5));
//      t.async_wait(wait_callback);
//    };
//
//    p->setSocketOption(transport::ProducerCallbacksOptions::INTEREST_INPUT,
//                       (transport::ProducerInterestCallback) interest_enter_callback);
//
//    t.async_wait(wait_callback);
//
//    p->serveForever();
//
//    std::unique_lock<std::mutex> lock(thread_list_mtx_);
//    icn_producers_.erase(request_name);
//  });
//
//  t.detach();
//}

}

}