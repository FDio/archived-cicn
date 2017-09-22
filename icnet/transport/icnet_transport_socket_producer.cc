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

#include "icnet_transport_socket_producer.h"

namespace icnet {

namespace transport {

ProducerSocket::ProducerSocket(Name prefix)
    : portal_(new Portal()),
      name_prefix_(prefix),
      data_packet_size_(default_values::content_object_packet_size),
      content_object_expiry_time_(default_values::content_object_expiry_time),
      registration_status_(REGISTRATION_NOT_ATTEMPTED),
      making_manifest_(false),
      signature_type_(SHA_256),
      key_locator_size_(default_values::key_locator_size),
      output_buffer_(default_values::producer_socket_output_buffer_size),
      input_buffer_capacity_(default_values::producer_socket_input_buffer_size),
      input_buffer_size_(0),
      processing_thread_stop_(false),
      listening_thread_stop_(false),
      on_interest_input_(VOID_HANDLER),
      on_interest_dropped_input_buffer_(VOID_HANDLER),
      on_interest_inserted_input_buffer_(VOID_HANDLER),
      on_interest_satisfied_output_buffer_(VOID_HANDLER),
      on_interest_process_(VOID_HANDLER),
      on_new_segment_(VOID_HANDLER),
      on_content_object_to_sign_(VOID_HANDLER),
      on_content_object_in_output_buffer_(VOID_HANDLER),
      on_content_object_output_(VOID_HANDLER),
      on_content_object_evicted_from_output_buffer_(VOID_HANDLER) {
  listening_thread_stop_ = false;
  key_locator_size_ = default_values::key_locator_size;
}

ProducerSocket::~ProducerSocket() {
  processing_thread_stop_ = true;
  portal_->stopEventsLoop();

  if (processing_thread_.joinable()) {
    processing_thread_.join();
  }

  if (listening_thread_.joinable()) {
    listening_thread_.join();
  }
}

void ProducerSocket::attach() {
  listening_thread_ = std::thread(std::bind(&ProducerSocket::listen, this));
  //  processing_thread_ = boost::thread(bind(&ProducerTransport::processIncomingInterest, this));
}

void ProducerSocket::serveForever() {
  if (listening_thread_.joinable()) {
    listening_thread_.join();
  }
}

void ProducerSocket::stop() {
  portal_->stopEventsLoop();
}

void ProducerSocket::dispatch() {
  // Check that the INTEREST_INPUT callback is set.
  if (on_interest_input_ == VOID_HANDLER) {
    std::cerr << "Warning: the dispatcher function needs a dispatcher callback! "
        "You need to set INTEREST_INPUT callback" << std::endl;
  }

  listening_thread_ = std::thread(std::bind(&ProducerSocket::listen, this));
}

void ProducerSocket::listen() {
  registration_status_ = REGISTRATION_IN_PROGRESS;

  portal_
      ->bind(name_prefix_, std::bind(&ProducerSocket::onInterest, this, std::placeholders::_1, std::placeholders::_2));

  portal_->runEventsLoop();
}

void ProducerSocket::passContentObjectToCallbacks(const std::shared_ptr<ContentObject> &content_object) {
  if (content_object) {
    if (on_new_segment_ != VOID_HANDLER) {
      on_new_segment_(*this, *content_object);
    }

    if (on_content_object_to_sign_ != VOID_HANDLER) {
      if (!making_manifest_) {
        on_content_object_to_sign_(*this, *content_object);
      } else {
        if (content_object->getPayloadType() == PayloadType::MANIFEST) {
          on_content_object_to_sign_(*this, *content_object);
        } else {
          content_object->signWithSha256(key_locator_);
        }
      }
    } else {
      content_object->signWithSha256(key_locator_);
    }

    if (on_content_object_in_output_buffer_ != VOID_HANDLER) {
      on_content_object_in_output_buffer_(*this, *content_object);
    }

    output_buffer_.insert(content_object);

    if (on_content_object_output_ != VOID_HANDLER) {
      on_content_object_output_(*this, *content_object);
    }

    if (content_object->getName().get(-1).toSegment() == 0) {
      portal_->sendContentObject(*content_object);
    }
  }
}

void ProducerSocket::produce(ContentObject &content_object) {
  if (!name_prefix_.isPrefixOf(content_object.getName())) {
    return;
  }

  if (on_content_object_in_output_buffer_ != VOID_HANDLER) {
    on_content_object_in_output_buffer_(*this, content_object);
  }

  if (on_content_object_output_ != VOID_HANDLER) {
    on_content_object_output_(*this, content_object);
  }

  portal_->sendContentObject(content_object);
}

void ProducerSocket::produce(Name name, const uint8_t *buf, size_t buffer_size, const int response_id, bool is_last) {

  if (buffer_size == 0) {
    return;
  }

  if (name.empty() || !name_prefix_.isPrefixOf(name)) {
    return;
  }

  int bytes_segmented = 0;

  std::cout << name.toString() << std::endl;

  size_t bytes_occupied_by_name = name.size();

  int digestSize = default_values::digest_size; // SHA_256 as default
  int signatureSize = default_values::signature_size;
  uint64_t free_space_for_content = 0;

  free_space_for_content = data_packet_size_ - bytes_occupied_by_name - digestSize - default_values::limit_guard;

  uint64_t number_of_segments = uint64_t(std::ceil(double(buffer_size) / double(free_space_for_content)));

  if (free_space_for_content * number_of_segments < buffer_size) {
    number_of_segments++;
  }

  uint64_t current_segment = 0;

  if (seq_number_map_.find(name.toString()) != seq_number_map_.end()
      && seq_number_map_[name.toString()].find(response_id) != seq_number_map_[name.toString()].end()) {
    current_segment = seq_number_map_[name.toString()][response_id];
  } else {
    seq_number_map_[name.toString()][response_id] = current_segment;
  }


  if (making_manifest_) {

    std::shared_ptr<ContentObject> content_object_segment;
    std::shared_ptr<Manifest> manifest_segment;
    bool manifest_segment_needed = true;

    uint64_t free_space_for_manifest =
        data_packet_size_ - bytes_occupied_by_name - signatureSize - default_values::limit_guard;

    for (unsigned int packaged_segments = 0; packaged_segments < number_of_segments;) {

      if (manifest_segment_needed) {

        Name manifest_name(name_prefix_);

        if (!name.empty()) {
          manifest_name.append(name);
        }

        manifest_name.appendSegment(current_segment);

        if (manifest_segment) {
          manifest_segment->encode();
          passContentObjectToCallbacks(manifest_segment);
        }

        manifest_segment = std::make_shared<Manifest>(manifest_name);

        if (is_last) {
          manifest_segment->setFinalChunkNumber(current_segment + number_of_segments - packaged_segments);
        }

        // finalSegment = current_segment;
        manifest_segment_needed = false;
        current_segment++;

        key_locator_.clear();
        key_locator_.setName(const_cast<Name &>(manifest_segment->getName()));
      }

      Name full_name = name;

      content_object_segment = std::make_shared<ContentObject>(std::move(full_name.appendSegment(current_segment)));
      content_object_segment->setExpiryTime((uint64_t) content_object_expiry_time_);

      if (packaged_segments == number_of_segments - 1) {
        content_object_segment->setContent(&buf[bytes_segmented], buffer_size - bytes_segmented);
        bytes_segmented += buffer_size - bytes_segmented;
      } else {
        content_object_segment->setContent(&buf[bytes_segmented], free_space_for_content);
        bytes_segmented += free_space_for_content;
      }

      if (is_last) {
        content_object_segment->setFinalChunkNumber(current_segment + number_of_segments - packaged_segments - 1);
      }

      passContentObjectToCallbacks(content_object_segment);

      size_t manifestSize = manifest_segment->estimateManifestSize();
      Name &content_object_name = (Name &) content_object_segment->getName();
      size_t fullNameSize = content_object_name.size();

      // TODO Signature

      if (manifestSize + 2 * fullNameSize > free_space_for_manifest) {
        manifest_segment_needed = true;
      }

      // TODO Manifest and hashes!
      //      Array block = content_object_segment->getContent();
      //      icn-interface::ConstBufferPtr implicitDigest = icn-interface::crypto::sha256(block.data(), block.size());
      //
      //      //add implicit digest to the manifest
      //      manifest_segment->addNameToCatalogue(Name(std::to_string(current_segment)), implicitDigest->buf(),
      //                                          implicitDigest->size());

      packaged_segments++;
      current_segment++;

      if (packaged_segments == number_of_segments) {
        manifest_segment->encode();
        passContentObjectToCallbacks(manifest_segment);
      }
    }
  } else {

    for (unsigned int packaged_segments = 0; packaged_segments < number_of_segments; packaged_segments++) {
      Name fullName = name;

      std::shared_ptr<ContentObject>
          content_object = std::make_shared<ContentObject>(std::move(fullName.appendSegment(current_segment)));

      // TODO If we set the throughput will decrease.. to investigate
      content_object->setExpiryTime((uint64_t)content_object_expiry_time_);

      if (is_last) {
        content_object->setFinalChunkNumber(current_segment + number_of_segments - packaged_segments - 1);
      }

      if (packaged_segments == number_of_segments - 1) {
        content_object->setContent(&buf[bytes_segmented], buffer_size - bytes_segmented);
        bytes_segmented += buffer_size - bytes_segmented;
      } else {
        content_object->setContent(&buf[bytes_segmented], free_space_for_content);
        bytes_segmented += free_space_for_content;
      }

      current_segment++;
      passContentObjectToCallbacks(content_object);
    }
  }

  seq_number_map_[name.toString()][response_id] = current_segment;

  if (is_last) {
    seq_number_map_[name.toString()].erase(response_id);
    if (seq_number_map_[name.toString()].empty()) {
      seq_number_map_.erase(name.toString());
    }
  }
}

void ProducerSocket::asyncProduce(ContentObject &content_object) {
  std::shared_ptr<ContentObject> c_object = std::make_shared<ContentObject>(content_object);
  std::thread t([c_object, this]() { produce(*c_object); });
  t.detach();
}

void ProducerSocket::asyncProduce(Name suffix,
                                  const uint8_t *buf,
                                  size_t buffer_size,
                                  const int response_id,
                                  bool is_last) {
  std::thread t([suffix, buf, buffer_size, response_id, is_last, this]() {
    produce(suffix, buf, buffer_size, response_id, is_last);
  });
  t.detach();
}

void ProducerSocket::onInterest(const Name &name, const Interest &interest) {
  if (on_interest_input_ != VOID_HANDLER) {
    on_interest_input_(*this, interest);
  }

  const std::shared_ptr<ContentObject> &content_object = output_buffer_.find(interest);

  if (content_object) {

    if (on_interest_satisfied_output_buffer_ != VOID_HANDLER) {
      on_interest_satisfied_output_buffer_(*this, interest);
    }

    if (on_content_object_output_ != VOID_HANDLER) {
      on_content_object_output_(*this, *content_object);
    }

    portal_->sendContentObject(*content_object);
  } else {
    if (on_interest_process_ != VOID_HANDLER) {
      on_interest_process_(*this, interest);
    }
  }
}

int ProducerSocket::setSocketOption(int socket_option_key, int socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::DATA_PACKET_SIZE:
      if (socket_option_value < default_values::max_content_object_size && socket_option_value > 0) {
        data_packet_size_ = socket_option_value;
        return SOCKET_OPTION_SET;
      } else {
        return SOCKET_OPTION_NOT_SET;
      }

    case GeneralTransportOptions::INPUT_BUFFER_SIZE:
      if (socket_option_value >= 1) {
        input_buffer_capacity_ = socket_option_value;
        return SOCKET_OPTION_SET;
      } else {
        return SOCKET_OPTION_NOT_SET;
      }

    case GeneralTransportOptions::OUTPUT_BUFFER_SIZE:
      if (socket_option_value >= 0) {
        output_buffer_.setLimit(socket_option_value);
        return SOCKET_OPTION_SET;
      } else {
        return SOCKET_OPTION_NOT_SET;
      }

    case GeneralTransportOptions::CONTENT_OBJECT_EXPIRY_TIME:
      content_object_expiry_time_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case GeneralTransportOptions::SIGNATURE_TYPE:
      if (socket_option_value == SOCKET_OPTION_DEFAULT) {
        signature_type_ = SHA_256;
      } else {
        signature_type_ = socket_option_value;
      }

      if (signature_type_ == SHA_256 || signature_type_ == RSA_256) {
        signature_size_ = 32;
      }

    case ProducerCallbacksOptions::INTEREST_INPUT:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_input_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::INTEREST_DROP:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_dropped_input_buffer_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::INTEREST_PASS:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_inserted_input_buffer_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::CACHE_HIT:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_satisfied_output_buffer_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::CACHE_MISS:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_process_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::NEW_CONTENT_OBJECT:
      if (socket_option_value == VOID_HANDLER) {
        on_new_segment_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::CONTENT_OBJECT_SIGN:
      if (socket_option_value == VOID_HANDLER) {
        on_content_object_to_sign_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::CONTENT_OBJECT_READY:
      if (socket_option_value == VOID_HANDLER) {
        on_content_object_in_output_buffer_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ProducerCallbacksOptions::CONTENT_OBJECT_OUTPUT:
      if (socket_option_value == VOID_HANDLER) {
        on_content_object_output_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ProducerSocket::setSocketOption(int socket_option_key, double socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ProducerSocket::setSocketOption(int socket_option_key, bool socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::MAKE_MANIFEST:
      making_manifest_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ProducerSocket::setSocketOption(int socket_option_key, Name socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::NAME_PREFIX:
      name_prefix_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ProducerSocket::setSocketOption(int socket_option_key, ProducerContentObjectCallback socket_option_value) {
  switch (socket_option_key) {
    case ProducerCallbacksOptions::NEW_CONTENT_OBJECT:
      on_new_segment_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ProducerCallbacksOptions::CONTENT_OBJECT_SIGN:
      on_content_object_to_sign_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ProducerCallbacksOptions::CONTENT_OBJECT_READY:
      on_content_object_in_output_buffer_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ProducerCallbacksOptions::CONTENT_OBJECT_OUTPUT:
      on_content_object_output_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ProducerSocket::setSocketOption(int socket_option_key, ProducerInterestCallback socket_option_value) {
  switch (socket_option_key) {
    case ProducerCallbacksOptions::INTEREST_INPUT:
      on_interest_input_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ProducerCallbacksOptions::INTEREST_DROP:
      on_interest_dropped_input_buffer_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ProducerCallbacksOptions::INTEREST_PASS:
      on_interest_inserted_input_buffer_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ProducerCallbacksOptions::CACHE_HIT:
      on_interest_satisfied_output_buffer_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ProducerCallbacksOptions::CACHE_MISS:
      on_interest_process_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ProducerSocket::setSocketOption(int socket_option_key, ConsumerContentObjectCallback socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ProducerSocket::setSocketOption(int socket_option_key,
                                    ConsumerContentObjectVerificationCallback socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ProducerSocket::setSocketOption(int socket_option_key, ConsumerInterestCallback socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ProducerSocket::setSocketOption(int socket_option_key, ConsumerContentCallback socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ProducerSocket::setSocketOption(int socket_option_key, ConsumerManifestCallback socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ProducerSocket::setSocketOption(int socket_option_key, KeyLocator socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ProducerSocket::getSocketOption(int socket_option_key, int &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::INPUT_BUFFER_SIZE:
      socket_option_value = input_buffer_capacity_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::OUTPUT_BUFFER_SIZE:
      socket_option_value = output_buffer_.getLimit();
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::DATA_PACKET_SIZE:
      socket_option_value = data_packet_size_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::CONTENT_OBJECT_EXPIRY_TIME:
      socket_option_value = content_object_expiry_time_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::SIGNATURE_TYPE:
      socket_option_value = signature_type_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ProducerSocket::getSocketOption(int socket_option_key, double &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::getSocketOption(int socket_option_key, bool &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::MAKE_MANIFEST:
      socket_option_value = making_manifest_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ProducerSocket::getSocketOption(int socket_option_key, Name &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::NAME_PREFIX:
      socket_option_value = name_prefix_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ProducerSocket::getSocketOption(int socket_option_key, ProducerContentObjectCallback &socket_option_value) {
  switch (socket_option_key) {
    case ProducerCallbacksOptions::NEW_CONTENT_OBJECT:
      socket_option_value = on_new_segment_;
      return SOCKET_OPTION_GET;

    case ProducerCallbacksOptions::CONTENT_OBJECT_SIGN:
      socket_option_value = on_content_object_to_sign_;
      return SOCKET_OPTION_GET;

    case ProducerCallbacksOptions::CONTENT_OBJECT_READY:
      socket_option_value = on_content_object_in_output_buffer_;
      return SOCKET_OPTION_GET;

    case ProducerCallbacksOptions::CONTENT_OBJECT_OUTPUT:
      socket_option_value = on_content_object_output_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ProducerSocket::getSocketOption(int socket_option_key, ProducerInterestCallback &socket_option_value) {
  switch (socket_option_key) {
    case ProducerCallbacksOptions::INTEREST_INPUT:
      socket_option_value = on_interest_input_;
      return SOCKET_OPTION_GET;

    case ProducerCallbacksOptions::INTEREST_DROP:
      socket_option_value = on_interest_dropped_input_buffer_;
      return SOCKET_OPTION_GET;

    case ProducerCallbacksOptions::INTEREST_PASS:
      socket_option_value = on_interest_inserted_input_buffer_;
      return SOCKET_OPTION_GET;

    case CACHE_HIT:
      socket_option_value = on_interest_satisfied_output_buffer_;
      return SOCKET_OPTION_GET;

    case CACHE_MISS:
      socket_option_value = on_interest_process_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ProducerSocket::getSocketOption(int socket_option_key, ConsumerContentObjectCallback &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::getSocketOption(int socket_option_key,
                                    ConsumerContentObjectVerificationCallback &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::getSocketOption(int socket_option_key, ConsumerInterestCallback &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::getSocketOption(int socket_option_key, ConsumerContentCallback &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::getSocketOption(int socket_option_key, ConsumerManifestCallback &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::setSocketOption(int socket_option_key, size_t socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::INPUT_BUFFER_SIZE:
      if (input_buffer_capacity_ >= 1) {
        input_buffer_capacity_ = socket_option_value;
        return SOCKET_OPTION_SET;
      }

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ProducerSocket::getSocketOption(int socket_option_key, size_t &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::INPUT_BUFFER_SIZE:
      socket_option_value = input_buffer_capacity_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::OUTPUT_BUFFER_SIZE:
      socket_option_value = output_buffer_.size();
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ProducerSocket::getSocketOption(int socket_option_key, KeyLocator &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::getSocketOption(int socket_option_key, std::shared_ptr<Portal> &socket_option_value) {
  switch (socket_option_key) {
    case PORTAL:
      socket_option_value = portal_;
      return SOCKET_OPTION_GET;
  }

  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::getSocketOption(int socket_option_key, IcnObserver **socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ProducerSocket::setSocketOption(int socket_option_key, IcnObserver *socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

} // end namespace transport

} // end namespace icnet
