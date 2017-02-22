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

#include "icnet_ccnx_pending_interest.h"

namespace icnet {

namespace ccnx {

PendingInterest::PendingInterest(std::shared_ptr<Interest> &interest,
                                 boost::asio::io_service &portal_io_service,
                                 const OnContentObjectCallback &on_content_object,
                                 const OnInterestTimeoutCallback &on_interest_timeout)
    : interest_(interest),
      io_service_(portal_io_service),
      timer_(io_service_),
      on_content_object_callback_(on_content_object),
      on_interest_timeout_callback_(on_interest_timeout),
      received_(false),
      valid_(true) {
}

PendingInterest::~PendingInterest() {
  interest_.reset();
}

void PendingInterest::startCountdown(BoostCallback &cb) {
  timer_.expires_from_now(boost::posix_time::milliseconds(interest_->getInterestLifetime()));
  timer_.async_wait(cb);
}

void PendingInterest::cancelTimer() {
  timer_.cancel();
}

bool PendingInterest::isReceived() const {
  return received_;
}

void PendingInterest::setReceived() {
  received_ = true;
}

const std::shared_ptr<Interest> &PendingInterest::getInterest() const {
  return interest_;
}

void PendingInterest::setInterest(const std::shared_ptr<Interest> &interest) {
  PendingInterest::interest_ = interest;
}

const OnContentObjectCallback &PendingInterest::getOnDataCallback() const {
  return on_content_object_callback_;
}

void PendingInterest::setOnDataCallback(const OnContentObjectCallback &on_content_object) {
  PendingInterest::on_content_object_callback_ = on_content_object;
}

const OnInterestTimeoutCallback &PendingInterest::getOnTimeoutCallback() const {
  return on_interest_timeout_callback_;
}

void PendingInterest::setOnTimeoutCallback(const OnInterestTimeoutCallback &on_interest_timeout) {
  PendingInterest::on_interest_timeout_callback_ = on_interest_timeout;
}

void PendingInterest::setReceived(bool received) {
  PendingInterest::received_ = received;
}

bool PendingInterest::isValid() const {
  return valid_;
}

void PendingInterest::setValid(bool valid) {
  PendingInterest::valid_ = valid;
}

} // end namespace ccnx

} // end namespace icnet