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

#ifndef ICNET_CCNX_NETWORK_MESSAGE_H_
#define ICNET_CCNX_NETWORK_MESSAGE_H_

#include "icnet_ccnx_common.h"
#include <iostream>

extern "C" {
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketDecoder.h>
#include <parc/algol/parc_Buffer.h>
#include <ccnx/transport/common/transport_MetaMessage.h>
};

namespace icnet {

namespace ccnx {

class TransportMessage {
 public:
  enum {
    header_length = 8
  };
  enum {
    max_packet_length = 1500
  };

  TransportMessage();

  const uint8_t *data() const;

  uint8_t *data();

  std::size_t length() const;

  const uint8_t *body() const;

  uint8_t *body();

  std::size_t bodyLength() const;

  void bodyLength(std::size_t new_length);

  bool decodeHeader();

  CCNxMetaMessage *decodeMessage();

 private:
  uint8_t data_[max_packet_length];
  std::size_t packet_length_;
};

} // end namespace ccnx

} // end namespace icnet

#endif // ICNET_CCNX_NETWORK_MESSAGE_H_
