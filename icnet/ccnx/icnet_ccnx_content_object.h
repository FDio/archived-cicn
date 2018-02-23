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

#ifndef ICNET_CCNX_CONTENT_OBJECT_H_
#define ICNET_CCNX_CONTENT_OBJECT_H_

#include "icnet_ccnx_common.h"
#include "icnet_ccnx_name.h"
#include "icnet_ccnx_key_locator.h"
#include "icnet_ccnx_payload_type.h"

extern "C" {
#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/validation/ccnxValidation_CRC32C.h>
#include <ccnx/common/codec/ccnxCodec_TlvPacket.h>
};

namespace icnet {

namespace ccnx {

typedef CCNxContentObject CCNxContentObjectStructure;

// This class is used just to transfer buffer pointers
// without making a copy, as std::vector<> would do

class Array {
 public:
  explicit Array(const void *array, size_t size);

  Array();

  const void *data();

  std::size_t size();

  Array &setData(const void *data);

  Array &setSize(std::size_t size);

 private:
  std::size_t size_;
  const void *array_;
};

class ContentObject : public std::enable_shared_from_this<ContentObject> {
 public:
  ContentObject();

  ContentObject(const Name &name, const uint8_t *payload, std::size_t size);

  ContentObject(const CCNxContentObjectStructure *content_object);

  ContentObject(const Name &name);

  ContentObject(Name &&name);

  ~ContentObject();

  bool operator==(const ContentObject &content_object);

  PayloadType getPayloadType() const;

  bool setContent(PayloadType content_type, const uint8_t *buffer, size_t buffer_size);

  bool setContent(const uint8_t *buffer, size_t buffer_size);

  void setPayloadType(PayloadType payload_type);

  Array getContent() const;

  void setSignature();

  void signWithSha256(KeyLocator &key_locator);

  void setFinalChunkNumber(uint64_t final_chunk_number);

  bool hasFinalChunkNumber();

  uint64_t getFinalChunkNumber();

  void setExpiryTime(uint64_t expiry_time);

  uint64_t getExpiryTime();

  const Name &getName() const;

  std::size_t getPacketSize() const;

  void setName(const Name &name);

  void setName(Name &&name);

  CCNxContentObjectStructure *getWrappedStructure();

  uint8_t getPathLabel() const;

 protected:

  Name name_;
  CCNxContentObjectStructure *ccnx_content_object_;
  PayloadType content_type_;
};

} // end namespace ccnx

} // end namespace icnet

#endif //CP_API_CCNXDATA_H_
