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

#include "icnet_ccnx_network_message.h"

namespace icnet {

namespace ccnx {

TransportMessage::TransportMessage()
    : packet_length_(0) {
}

const uint8_t *TransportMessage::data() const {
  return data_;
}

uint8_t *TransportMessage::data() {
  return data_;
}

std::size_t TransportMessage::length() const {
  return packet_length_;
}

const uint8_t *TransportMessage::body() const {
  return data_ + header_length;
}

uint8_t *TransportMessage::body() {
  return data_ + header_length;
}

std::size_t TransportMessage::bodyLength() const {
  return packet_length_ - header_length;
}

void TransportMessage::bodyLength(std::size_t new_length) {
  packet_length_ = new_length;
  if (packet_length_ > max_packet_length) {
    packet_length_ = max_packet_length;
  }
}

bool TransportMessage::decodeHeader() {
  // General checks

  uint8_t message_version = data_[0];

  if (message_version != 1) {
    std::cerr << "Illegal packet version " << message_version << std::endl;
    return false;
  }

  // Get packet length

  packet_length_ = data_[2];
  packet_length_ <<= 8;
  packet_length_ |= data_[3];

  return true;
}

CCNxMetaMessage *TransportMessage::decodeMessage() {
  std::size_t total_length = packet_length_;
  PARCBuffer *buffer = parcBuffer_CreateFromArray((void *) data_, total_length);
  buffer = parcBuffer_Flip(buffer);
  CCNxMetaMessage *ret = ccnxMetaMessage_CreateFromWireFormatBuffer(buffer);
  parcBuffer_Release((PARCBuffer **) &buffer);

  return ret;

}

} // end namespace ccnx

} // end namespace icnet