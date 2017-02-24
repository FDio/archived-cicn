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

#include "icnet_transport_vegas.h"
#include "icnet_socket_consumer.h"

namespace icnet {

VegasTransportProtocol::VegasTransportProtocol(Socket *icnet_socket)
    : TransportProtocol(icnet_socket),
      is_final_block_number_discovered_(false),
      final_block_number_(std::numeric_limits<uint64_t>::max()),
      last_reassembled_segment_(0),
      content_buffer_size_(0),
      current_window_size_(default_values::min_window_size),
      interests_in_flight_(0),
      segment_number_(0),
      interest_retransmissions_(default_values::default_buffer_size),
      interest_timepoints_(default_values::default_buffer_size),
      receive_buffer_(default_values::default_buffer_size),
      unverified_segments_(default_values::default_buffer_size),
      verified_manifests_(default_values::default_buffer_size) {
  icnet_socket->getSocketOption(PORTAL, portal_);
}

VegasTransportProtocol::~VegasTransportProtocol() {
  stop();
}

void VegasTransportProtocol::start() {
  is_running_ = true;
  is_final_block_number_discovered_ = false;
  final_block_number_ = std::numeric_limits<uint64_t>::max();
  segment_number_ = 0;
  interests_in_flight_ = 0;
  last_reassembled_segment_ = 0;
  content_buffer_size_ = 0;
  content_buffer_.clear();
  interest_retransmissions_.clear();
  receive_buffer_.clear();
  unverified_segments_.clear();
  verified_manifests_.clear();

  sendInterest();

  bool isAsync = false;
  socket_->getSocketOption(ASYNC_MODE, isAsync);

  bool isContextRunning = false;
  socket_->getSocketOption(RUNNING, isContextRunning);

  if (!isAsync && !isContextRunning) {
    socket_->setSocketOption(RUNNING, true);
    portal_->runEventsLoop();

    // If portal returns, the download (maybe) is finished, so we can remove the pending interests

    removeAllPendingInterests();
  }
}

// TODO Reuse this function for sending an arbitrary interest
void VegasTransportProtocol::sendInterest() {
  Name prefix;
  socket_->getSocketOption(GeneralTransportOptions::NAME_PREFIX, prefix);

  Name suffix;
  socket_->getSocketOption(GeneralTransportOptions::NAME_SUFFIX, suffix);

  if (!suffix.empty()) {
    prefix.append(suffix);
  }

  prefix.appendSegment(segment_number_);

  std::shared_ptr<Interest> interest = std::make_shared<Interest>(std::move(prefix));

  int interestLifetime = default_values::interest_lifetime;
  socket_->getSocketOption(GeneralTransportOptions::INTEREST_LIFETIME, interestLifetime);
  interest->setInterestLifetime(uint32_t(interestLifetime));

  ConsumerInterestCallback on_interest_output = VOID_HANDLER;

  socket_->getSocketOption(ConsumerCallbacksOptions::INTEREST_OUTPUT, on_interest_output);
  if (on_interest_output != VOID_HANDLER) {
    on_interest_output(*dynamic_cast<ConsumerSocket *>(socket_), *interest);
  }

  if (!is_running_) {
    return;
  }

  interests_in_flight_++;
  interest_retransmissions_[segment_number_ % default_values::default_buffer_size] = 0;
  interest_timepoints_[segment_number_ % default_values::default_buffer_size] = std::chrono::steady_clock::now();

  portal_->sendInterest(*interest,
                        bind(&VegasTransportProtocol::onContentSegment, this, _1, _2),
                        bind(&VegasTransportProtocol::onTimeout, this, _1));
  segment_number_++;
}

void VegasTransportProtocol::stop() {
  is_running_ = false;
  portal_->stopEventsLoop();
}

void VegasTransportProtocol::onContentSegment(const Interest &interest, ContentObject &content_object) {
  uint64_t segment = interest.getName().get(-1).toSegment();

  if (is_running_ == false /*|| input_buffer_[segment]*/) {
    return;
  }

  interests_in_flight_--;

  changeInterestLifetime(segment);
  ConsumerContentObjectCallback on_data_input = VOID_HANDLER;
  socket_->getSocketOption(ConsumerCallbacksOptions::CONTENT_OBJECT_INPUT, on_data_input);
  if (on_data_input != VOID_HANDLER) {
    on_data_input(*dynamic_cast<ConsumerSocket *>(socket_), content_object);
  }

  ConsumerInterestCallback on_interest_satisfied = VOID_HANDLER;
  socket_->getSocketOption(INTEREST_SATISFIED, on_interest_satisfied);
  if (on_interest_satisfied != VOID_HANDLER) {
    on_interest_satisfied(*dynamic_cast<ConsumerSocket *>(socket_), const_cast<Interest &>(interest));
  }

  if (content_object.getContentType() == PayloadType::MANIFEST) {
    onManifest(interest, content_object);
  } else if (content_object.getContentType() == PayloadType::DATA) {
    onContentObject(interest, content_object);
  } // TODO InterestReturn

  scheduleNextInterests();
}

void VegasTransportProtocol::afterContentReception(const Interest &interest, const ContentObject &content_object) {
  increaseWindow();
}

void VegasTransportProtocol::afterDataUnsatisfied(uint64_t segment) {
  decreaseWindow();
}

void VegasTransportProtocol::scheduleNextInterests() {
  if (segment_number_ == 0) {
    current_window_size_ = final_block_number_;

    double maxWindowSize = -1;
    socket_->getSocketOption(MAX_WINDOW_SIZE, maxWindowSize);

    if (current_window_size_ > maxWindowSize) {
      current_window_size_ = maxWindowSize;
    }

    while (interests_in_flight_ < current_window_size_) {
      if (is_final_block_number_discovered_) {
        if (segment_number_ <= final_block_number_) {
          sendInterest();
        } else {
          break;
        }
      } else {
        sendInterest();
      }
    }
  } else {
    if (is_running_) {
      while (interests_in_flight_ < current_window_size_) {
        if (is_final_block_number_discovered_) {
          if (segment_number_ <= final_block_number_) {
            sendInterest();
          } else {
            break;
          }
        } else {
          sendInterest();
        }
      }
    }
  }
}

void VegasTransportProtocol::decreaseWindow() {
  double min_window_size = -1;
  socket_->getSocketOption(MIN_WINDOW_SIZE, min_window_size);
  if (current_window_size_ > min_window_size) {
    current_window_size_ = std::ceil(current_window_size_ / 2);

    socket_->setSocketOption(CURRENT_WINDOW_SIZE, current_window_size_);
  }
}

void VegasTransportProtocol::increaseWindow() {
  double max_window_size = -1;
  socket_->getSocketOption(MAX_WINDOW_SIZE, max_window_size);
  if (current_window_size_ < max_window_size) {
    current_window_size_++;
    socket_->setSocketOption(CURRENT_WINDOW_SIZE, current_window_size_);
  }
};

void VegasTransportProtocol::changeInterestLifetime(uint64_t segment) {
  std::chrono::steady_clock::duration duration = std::chrono::steady_clock::now() - interest_timepoints_[segment];
  rtt_estimator_.addMeasurement(std::chrono::duration_cast<std::chrono::microseconds>(duration));

  RtoEstimator::Duration rto = rtt_estimator_.computeRto();
  std::chrono::milliseconds lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(rto);

  socket_->setSocketOption(INTEREST_LIFETIME, (int) lifetime.count());
}

void VegasTransportProtocol::onManifest(const Interest &interest, ContentObject &content_object) {
  if (!is_running_) {
    return;
  }

  if (verifyManifest(content_object)) {
    // TODO Retrieve piece of data using manifest
  }
}

bool VegasTransportProtocol::verifyManifest(ContentObject &content_object) {
  ConsumerContentObjectVerificationCallback on_manifest_to_verify = VOID_HANDLER;
  socket_->getSocketOption(ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY, on_manifest_to_verify);

  bool is_data_secure = false;

  if (on_manifest_to_verify == VOID_HANDLER) {
    // TODO Perform manifest verification
  } else if (on_manifest_to_verify(*dynamic_cast<ConsumerSocket *>(socket_), content_object)) {
    is_data_secure = true;
  }

  return is_data_secure;
}

bool VegasTransportProtocol::requireInterestWithHash(const Interest &interest,
                                                     const ContentObject &content_object,
                                                     Manifest &manifest) {
  // TODO Require content object with specific hash.
  return true;
}

// TODO Add the name in the digest computation!
void VegasTransportProtocol::onContentObject(const Interest &interest, ContentObject &content_object) {
  if (verifyContentObject(interest, content_object)) {
    checkForFastRetransmission(interest);

    uint64_t segment = interest.getName().get(-1).toSegment();

    if (interest_retransmissions_[segment % default_values::default_buffer_size] == 0) {
      afterContentReception(interest, content_object);
    }

    if (content_object.hasFinalChunkNumber()) {
      is_final_block_number_discovered_ = true;
      final_block_number_ = content_object.getFinalChunkNumber();
    }

    bool virtualDownload = false;

    socket_->getSocketOption(VIRTUAL_DOWNLOAD, virtualDownload);

    if (!virtualDownload) {
      receive_buffer_[segment % default_values::default_buffer_size] = content_object.shared_from_this();
      reassemble();
    } else {
      if (segment == final_block_number_) {
        portal_->stopEventsLoop();
      }
    }
  }
}

bool VegasTransportProtocol::verifyContentObject(const Interest &interest, ContentObject &content_object) {
  // TODO Check content object using manifest
  return true;
}

// TODO move inside manifest
bool VegasTransportProtocol::pointsToManifest(ContentObject &content_object) {
  // TODO Check content objects using manifest
  return true;
}

void VegasTransportProtocol::onTimeout(const Interest &interest) {
  if (!is_running_) {
    return;
  }

  interests_in_flight_--;

  std::cerr << "Timeout on " << interest.getName() << std::endl;

  ConsumerInterestCallback on_interest_timeout = VOID_HANDLER;
  socket_->getSocketOption(INTEREST_EXPIRED, on_interest_timeout);
  if (on_interest_timeout != VOID_HANDLER) {
    on_interest_timeout(*dynamic_cast<ConsumerSocket *>(socket_), const_cast<Interest &>(interest));
  }

  uint64_t segment = interest.getName().get(-1).toSegment();

  // Do not retransmit interests asking contents that do not exist.
  if (is_final_block_number_discovered_) {
    if (segment > final_block_number_) {
      return;
    }
  }

  afterDataUnsatisfied(segment);

  int max_retransmissions;
  socket_->getSocketOption(ConsumerCallbacksOptions::INTEREST_RETRANSMISSION, max_retransmissions);

  if (interest_retransmissions_[segment % default_values::default_buffer_size] < max_retransmissions) {

    ConsumerInterestCallback on_interest_retransmission = VOID_HANDLER;
    socket_->getSocketOption(ConsumerCallbacksOptions::INTEREST_RETRANSMISSION, on_interest_retransmission);

    if (on_interest_retransmission != VOID_HANDLER) {
      on_interest_retransmission(*dynamic_cast<ConsumerSocket *>(socket_), interest);
    }

    ConsumerInterestCallback on_interest_output = VOID_HANDLER;

    socket_->getSocketOption(ConsumerCallbacksOptions::INTEREST_OUTPUT, on_interest_output);
    if (on_interest_output != VOID_HANDLER) {
      on_interest_output(*dynamic_cast<ConsumerSocket *>(socket_), interest);
    }

    if (!is_running_) {
      return;
    }

    //retransmit
    interests_in_flight_++;
    interest_retransmissions_[segment % default_values::default_buffer_size]++;

    portal_->sendInterest(interest,
                          bind(&VegasTransportProtocol::onContentSegment, this, _1, _2),
                          bind(&VegasTransportProtocol::onTimeout, this, _1));
  } else {
    is_running_ = false;

    bool virtual_download = false;
    socket_->getSocketOption(VIRTUAL_DOWNLOAD, virtual_download);

    if (!virtual_download) {
      reassemble();
    }

    portal_->stopEventsLoop();
  }

}

void VegasTransportProtocol::copyContent(ContentObject &content_object) {
  Array a = content_object.getContent();

  content_buffer_.insert(content_buffer_.end(), (uint8_t *) a.data(), (uint8_t *) a.data() + a.size());

  if ((content_object.getName().get(-1).toSegment() == final_block_number_) || (!is_running_)) {

    // return content to the user
    ConsumerContentCallback on_payload = VOID_HANDLER;
    socket_->getSocketOption(CONTENT_RETRIEVED, on_payload);
    if (on_payload != VOID_HANDLER) {
      on_payload(*dynamic_cast<ConsumerSocket *>(socket_),
                 (uint8_t *) (content_buffer_.data()),
                 content_buffer_.size());
    }

    //reduce window size to prevent its speculative growth in case when consume() is called in loop
    int current_window_size = -1;
    socket_->getSocketOption(CURRENT_WINDOW_SIZE, current_window_size);
    if ((uint64_t) current_window_size > final_block_number_) {
      socket_->setSocketOption(CURRENT_WINDOW_SIZE, (int) (final_block_number_));
    }

    is_running_ = false;
    portal_->stopEventsLoop();
  }
}

void VegasTransportProtocol::reassemble() {
  uint64_t index = last_reassembled_segment_ % default_values::default_buffer_size;

  while (receive_buffer_[index % default_values::default_buffer_size]) {
    if (receive_buffer_[index % default_values::default_buffer_size]->getContentType() == PayloadType::DATA) {
      copyContent(*receive_buffer_[index % default_values::default_buffer_size]);
    }

    receive_buffer_[index % default_values::default_buffer_size].reset();

    last_reassembled_segment_++;
    index = last_reassembled_segment_ % default_values::default_buffer_size;
  }
}

bool VegasTransportProtocol::verifySegmentUsingManifest(Manifest &manifestSegment, ContentObject &content_object) {
  // TODO Content object verification exploiting manifest
  return true;
}

void VegasTransportProtocol::checkForFastRetransmission(const Interest &interest) {
  uint64_t segNumber = interest.getName().get(-1).toSegment();
  received_segments_[segNumber] = true;
  fast_retransmitted_segments.erase(segNumber);

  uint64_t possibly_lost_segment = 0;
  uint64_t highest_received_segment = received_segments_.rbegin()->first;

  for (uint64_t i = 0; i <= highest_received_segment; i++) {
    if (received_segments_.find(i) == received_segments_.end()) {
      if (fast_retransmitted_segments.find(i) == fast_retransmitted_segments.end()) {
        possibly_lost_segment = i;
        uint8_t out_of_order_segments = 0;
        for (uint64_t j = i; j <= highest_received_segment; j++) {
          if (received_segments_.find(j) != received_segments_.end()) {
            out_of_order_segments++;
            if (out_of_order_segments >= default_values::max_out_of_order_segments) {
              fast_retransmitted_segments[possibly_lost_segment] = true;
              fastRetransmit(interest, possibly_lost_segment);
            }
          }
        }
      }
    }
  }
}

void VegasTransportProtocol::fastRetransmit(const Interest &interest, uint64_t chunk_number) {
  int max_retransmissions;
  socket_->getSocketOption(GeneralTransportOptions::MAX_INTEREST_RETX, max_retransmissions);

  if (interest_retransmissions_[chunk_number % default_values::default_buffer_size] < max_retransmissions) {
    Name name = interest.getName().getPrefix(-1);
    name.appendSegment(chunk_number);

    std::shared_ptr<Interest> retx_interest = std::make_shared<Interest>(name);

    ConsumerInterestCallback on_interest_retransmission = VOID_HANDLER;
    socket_->getSocketOption(ConsumerCallbacksOptions::INTEREST_RETRANSMISSION, on_interest_retransmission);

    if (on_interest_retransmission != VOID_HANDLER) {
      on_interest_retransmission(*dynamic_cast<ConsumerSocket *>(socket_), *retx_interest);
    }

    ConsumerInterestCallback on_interest_output = VOID_HANDLER;

    socket_->getSocketOption(ConsumerCallbacksOptions::INTEREST_OUTPUT, on_interest_output);
    if (on_interest_output != VOID_HANDLER) {
      on_interest_output(*dynamic_cast<ConsumerSocket *>(socket_), *retx_interest);
    }

    if (!is_running_) {
      return;
    }

    interests_in_flight_++;
    interest_retransmissions_[chunk_number % default_values::default_buffer_size]++;
    portal_->sendInterest(*retx_interest,
                          bind(&VegasTransportProtocol::onContentSegment, this, _1, _2),
                          bind(&VegasTransportProtocol::onTimeout, this, _1));
  }
}

void VegasTransportProtocol::removeAllPendingInterests() {
  portal_->clear();
}

} // namespace icn-interface
