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

#include "icnet_ccnx_portal.h"

#define UNSET_CALLBACK 0
#define MAX_ARRAY_SIZE 16000

namespace icnet {

namespace ccnx {

Portal::Portal(std::string forwarder_ip_address, std::string forwarder_port)
    : is_running_(true),
      clear_(false),
      on_interest_callback_(UNSET_CALLBACK),
      connector_(io_service_,
                 forwarder_ip_address,
                 forwarder_port,
                 std::bind(&Portal::processIncomingMessages, this, std::placeholders::_1),
                 served_name_list_) {
  io_service_.reset();
}

Portal::~Portal() {
  connector_.close();
  stopEventsLoop();
  clear();
}

void Portal::sendInterest(const Interest &interest,
                          const OnContentObjectCallback &on_content_object,
                          const OnInterestTimeoutCallback &on_interest_timeout) {
  std::shared_ptr<Interest> _interest = const_cast<Interest &>(interest).shared_from_this();

  // Create new message
  CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(_interest->getWrappedStructure());

  // Send it
  connector_.send(message);
  clear_ = false;
  std::function<void(const boost::system::error_code &)> timer_callback;

  PendingInterest *pend_interest = new PendingInterest(_interest, io_service_, on_content_object, on_interest_timeout);
  const Name &name = _interest->getName();

  pending_interest_hash_table_[name] = std::unique_ptr<PendingInterest>(pend_interest);

  timer_callback = [this, name](const boost::system::error_code &ec) {

    if (clear_ || !is_running_) {
      return;
    }

    if (ec.value() != boost::system::errc::operation_canceled) {
      std::unordered_map<Name, std::unique_ptr<PendingInterest>>::iterator it = pending_interest_hash_table_.find(name);
      if (it != pending_interest_hash_table_.end()) {
        it->second->getOnTimeoutCallback()(*it->second->getInterest());
      }
    }
  };

  pend_interest->startCountdown(timer_callback);

  ccnxMetaMessage_Release(&message);
}

void Portal::bind(Name &name, const OnInterestCallback &on_interest_callback) {
  on_interest_callback_ = on_interest_callback;
  served_name_list_.push_back(name);
  work_ = std::shared_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(io_service_));
  connector_.bind(name);
}

void Portal::sendContentObject(const ContentObject &content_object) {
  ContentObject &ccnx_data = const_cast<ContentObject &>(content_object);
  CCNxMetaMessageStructure *message = ccnxMetaMessage_CreateFromContentObject(ccnx_data.getWrappedStructure());

  ccnxContentObject_AssertValid(ccnx_data.getWrappedStructure());

  connector_.send(message);

  ccnxMetaMessage_Release(&message);
}

void Portal::runEventsLoop() {
  if (io_service_.stopped()) {
    io_service_.reset(); // ensure that run()/poll() will do some work
  }

  is_running_ = true;
  this->io_service_.run();
}

void Portal::stopEventsLoop() {
  is_running_ = false;
  work_.reset();
  io_service_.stop();
}

void Portal::clear() {
  pending_interest_hash_table_.clear();
  clear_ = true;
}

void Portal::processInterest(CCNxMetaMessage *response) {
  // Interest for a producer
  CCNxInterest *interest_ptr = ccnxInterest_Acquire(ccnxMetaMessage_GetInterest(response));

  if (on_interest_callback_ != UNSET_CALLBACK) {

    Interest interest(interest_ptr);
    if (on_interest_callback_) {
      on_interest_callback_(interest.getName(), interest);
    }
    ccnxInterest_Release((CCNxInterest **) &interest_ptr);
  }
}

void Portal::processControlMessage(CCNxMetaMessage *response) {
  // Control message as response to the route set by a producer

  CCNxControl *control_message = ccnxMetaMessage_GetControl(response);

  if (ccnxControl_IsACK(control_message)) {
    std::cout << "Route set correctly!" << std::endl;
  } else {
    std::cout << "Failed to set the route." << std::endl;
  }
}

void Portal::processContentObject(CCNxMetaMessage *response) {
  // Content object for a consumer

  CCNxContentObject *content_object = ccnxContentObject_Acquire(ccnxMetaMessage_GetContentObject(response));
  CCNxName *n = ccnxContentObject_GetName(content_object);

  PendingInterestHashTable::iterator it = pending_interest_hash_table_.find(Name(n));

  if (it != pending_interest_hash_table_.end()) {

    std::unique_ptr<PendingInterest> &interest_ptr = it->second;

    interest_ptr->cancelTimer();
    std::shared_ptr<ContentObject> data_ptr = std::make_shared<ContentObject>(content_object);

    if (!interest_ptr->isReceived()) {
      interest_ptr->setReceived();
      interest_ptr->getOnDataCallback()(*interest_ptr->getInterest(), *data_ptr);
      pending_interest_hash_table_.erase(interest_ptr->getInterest()->getName());
    }
  }

  ccnxContentObject_Release((CCNxContentObject **) &content_object);
}

void Portal::processIncomingMessages(CCNxMetaMessage *response) {
  if (clear_ || !is_running_) {
    return;
  }

  if (response) {
    if (ccnxMetaMessage_IsContentObject(response)) {
      processContentObject(response);
    } else if (ccnxMetaMessage_IsInterest(response)) {
      processInterest(response);
    } else if (ccnxMetaMessage_IsControl(response)) {
      processControlMessage(response);
    }
    ccnxMetaMessage_Release(&response);
  }

}

boost::asio::io_service &Portal::getIoService() {
  return io_service_;
}

} // end namespace ccnx

} // end namespace icnet