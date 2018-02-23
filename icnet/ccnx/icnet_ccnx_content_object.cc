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

#include "icnet_ccnx_content_object.h"

extern "C" {
#include <ccnx/common/ccnx_WireFormatMessage.h>
};

namespace icnet {

namespace ccnx {

ContentObject::ContentObject()
    : name_(ccnxName_Create()),
      ccnx_content_object_(ccnxContentObject_CreateWithNameAndPayload(name_.getWrappedStructure(), NULL)),
      content_type_(PayloadType::DATA) {
}

ContentObject::ContentObject(const Name &name, const uint8_t *payload, std::size_t size)
    : name_(name), content_type_(PayloadType::DATA) {
  PARCBuffer *buffer = parcBuffer_CreateFromArray(payload, size);
  buffer = parcBuffer_Flip(buffer);
  ccnx_content_object_ = ccnxContentObject_CreateWithNameAndPayload(name.getWrappedStructure(), buffer);
  parcBuffer_Release(&buffer);
}

ContentObject::ContentObject(const CCNxContentObjectStructure *content_object)
    : name_(ccnxContentObject_GetName(content_object)),
      ccnx_content_object_(ccnxContentObject_Acquire(content_object)),
      content_type_((PayloadType) ccnxContentObject_GetPayloadType(content_object)) {
}

ContentObject::ContentObject(const Name &name)
    : name_(name),
      ccnx_content_object_(ccnxContentObject_CreateWithNameAndPayload(name.getWrappedStructure(), NULL)),
      content_type_(PayloadType::DATA) {
}

ContentObject::ContentObject(Name &&name)
    : name_(std::move(name)),
      ccnx_content_object_(ccnxContentObject_CreateWithNameAndPayload(name.getWrappedStructure(), NULL)),
      content_type_(PayloadType::DATA) {
}

ContentObject::~ContentObject() {
  ccnxContentObject_Release(&ccnx_content_object_);
}

bool ContentObject::operator==(const ContentObject &content_object) {
  return ccnxContentObject_Equals(ccnx_content_object_, content_object.ccnx_content_object_);
}

PayloadType ContentObject::getPayloadType() const {
  return (PayloadType) ccnxContentObject_GetPayloadType(ccnx_content_object_);
}

void ContentObject::setPayloadType(PayloadType payload_type) {
  content_type_ = payload_type;
}

bool ContentObject::setContent(PayloadType content_type, const uint8_t *buffer, size_t buffer_size) {
  content_type_ = content_type;
  return setContent(buffer, buffer_size);
}

bool ContentObject::setContent(const uint8_t *buffer, size_t buffer_size) {
  bool ret;
  PARCBuffer *parc_buffer = parcBuffer_CreateFromArray(buffer, buffer_size);
  parc_buffer = parcBuffer_Flip(parc_buffer);

  if (content_type_ != PayloadType::DATA) {
    ret = ccnxContentObject_SetPayload(ccnx_content_object_, (CCNxPayloadType) content_type_, parc_buffer);
  } else {
    ret = ccnxContentObject_SetPayload(ccnx_content_object_, (CCNxPayloadType) PayloadType::DATA, parc_buffer);
  }

  parcBuffer_Release(&parc_buffer);

  return ret;
}

Array ContentObject::getContent() const {
  PARCBuffer *buffer = ccnxContentObject_GetPayload(ccnx_content_object_);
  return Array(parcBuffer_Overlay(buffer, 0), parcBuffer_Remaining(buffer));
}

void ContentObject::setSignature() {

}

void ContentObject::signWithSha256(KeyLocator &key_locator) {
  // ccnxValidationCRC32C_Set(ccnx_content_object_);
}

void ContentObject::setFinalChunkNumber(uint64_t final_chunk_number) {
  ccnxContentObject_SetFinalChunkNumber(ccnx_content_object_, final_chunk_number);
}

bool ContentObject::hasFinalChunkNumber() {
  return ccnxContentObject_HasFinalChunkNumber(ccnx_content_object_);
}

uint64_t ContentObject::getFinalChunkNumber() {
  return ccnxContentObject_GetFinalChunkNumber(ccnx_content_object_);
}

void ContentObject::setExpiryTime(uint64_t expiry_time) {
  std::chrono::milliseconds
      ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
  uint64_t expiration = ms.count() + expiry_time;
  ccnxContentObject_SetExpiryTime(ccnx_content_object_, expiration);
}

uint64_t ContentObject::getExpiryTime() {
  return ccnxContentObject_GetExpiryTime(ccnx_content_object_);
}

const Name &ContentObject::getName() const {
  return name_;
}

std::size_t ContentObject::getPacketSize() const {
  PARCBuffer *packet = ccnxWireFormatMessage_GetWireFormatBuffer(ccnx_content_object_);
  std::size_t ret = parcBuffer_Remaining(packet);
  return ret;
}

void ContentObject::setName(const Name &name) {
  PARCBuffer *buffer = parcBuffer_Acquire(ccnxContentObject_GetPayload(ccnx_content_object_));
  ccnxContentObject_Release(&ccnx_content_object_);
  ccnx_content_object_ = ccnxContentObject_CreateWithNameAndPayload(name.getWrappedStructure(), buffer);
  parcBuffer_Release(&buffer);
  name_ = std::move(name);
}

void ContentObject::setName(Name &&name) {
  PARCBuffer *buffer = parcBuffer_Acquire(ccnxContentObject_GetPayload(ccnx_content_object_));
  ccnxContentObject_Release(&ccnx_content_object_);
  ccnx_content_object_ = ccnxContentObject_CreateWithNameAndPayload(name.getWrappedStructure(), buffer);
  parcBuffer_Release(&buffer);
  name_ = std::move(name);
}

CCNxContentObjectStructure *ContentObject::getWrappedStructure() {
  return ccnx_content_object_;
}

uint8_t ContentObject::getPathLabel() const {
  if (ccnxContentObject_HasPathLabel(ccnx_content_object_)) {
    return (uint8_t) ccnxContentObject_GetPathLabel(ccnx_content_object_);
  }

  return 0;
}

Array::Array(const void *array, size_t size) {
  this->array_ = array;
  this->size_ = size;
}

Array::Array() {
  this->array_ = nullptr;
  this->size_ = 0;
}

const void *Array::data() {
  return array_;
}

std::size_t Array::size() {
  return size_;
}

Array &Array::setData(const void *data) {
  array_ = data;
  return *this;
}

Array &Array::setSize(std::size_t size) {
  size_ = size;
  return *this;
}

} // end namespace ccnx

} // end namespace icnet
