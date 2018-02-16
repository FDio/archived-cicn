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

#include "icnet_http_server_publisher.h"

namespace icnet {

namespace http {

HTTPServerPublisher::HTTPServerPublisher(const transport::Name &content_name)
    : content_name_(content_name) {
}

HTTPServerPublisher::~HTTPServerPublisher() {
  if (this->timer_) {
    this->timer_->cancel();
  }
}

HTTPServerPublisher& HTTPServerPublisher::attachPublisher() {
  // Create a new publisher
  producer_ = std::unique_ptr<transport::ProducerSocket>(new transport::ProducerSocket(content_name_));
  producer_->attach();
  return *this;
}

transport::ProducerSocket& HTTPServerPublisher::getProducer() {
  return *producer_;
}

HTTPServerPublisher &HTTPServerPublisher::setTimeout(const std::chrono::milliseconds &timeout, bool timeout_renewal) {
  std::shared_ptr<transport::Portal> portal;
  producer_->getSocketOption(transport::GeneralTransportOptions::PORTAL, portal);
  timer_ = std::unique_ptr<boost::asio::steady_timer>(new boost::asio::steady_timer(portal->getIoService(), timeout));

  wait_callback_ = [this](const boost::system::error_code e) {
    if (!e) {
      producer_->stop();
    }
  };

  if (timeout_renewal) {
    interest_enter_callback_ = [this, timeout](transport::ProducerSocket &p, const transport::Interest &interest) {
      this->timer_->cancel();
      this->timer_->expires_from_now(timeout);
      this->timer_->async_wait(wait_callback_);
    };

    producer_->setSocketOption(transport::ProducerCallbacksOptions::INTEREST_INPUT,
                               (transport::ProducerInterestCallback) interest_enter_callback_);
  }

  timer_->async_wait(wait_callback_);

  return *this;
}

void HTTPServerPublisher::publishContent(const uint8_t *buf, size_t buffer_size, std::chrono::milliseconds content_lifetime, const int response_id, bool is_last) {
  if (producer_) {
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "HTTP_SERVER_PUBLISHER", "Replying to %s", const_cast<transport::Name &>(content_name_).toString().c_str());
#else
    std::cout << "Replying to " << content_name_ << std::endl;
#endif
    producer_->setSocketOption(transport::GeneralTransportOptions::CONTENT_OBJECT_EXPIRY_TIME, static_cast<int>(content_lifetime.count()));

    producer_->produce(content_name_, buf, buffer_size, response_id, is_last);
  }
}

void HTTPServerPublisher::serveClients() {
  producer_->serveForever();
}

void HTTPServerPublisher::stop() {
  std::shared_ptr<transport::Portal> portal_ptr;
  producer_->getSocketOption(transport::GeneralTransportOptions::PORTAL, portal_ptr);
  portal_ptr->getIoService().stop();
}

}

}
