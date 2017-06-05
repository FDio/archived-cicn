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

#include "icnet_ccnx_interest.h"

namespace icnet {

namespace ccnx {

Interest::Interest(const Name &interest_name)
    : name_(interest_name), interest_(ccnxInterest_CreateSimple(name_.getWrappedStructure())) {
}

Interest::Interest(Name &&interestName)
    : name_(std::move(interestName)), interest_(ccnxInterest_CreateSimple(name_.getWrappedStructure())) {
}

Interest::Interest(CCNxInterestStruct *interest)
    : name_(ccnxInterest_GetName(interest)), interest_(ccnxInterest_Acquire(interest)) {
}

Interest::Interest(const Interest &other_interest)
    : name_(other_interest.name_), interest_(ccnxInterest_CreateSimple(other_interest.name_.getWrappedStructure())) {
  PARCBuffer *buffer = nullptr;

  // Key Id restriction
  buffer = ccnxInterest_GetKeyIdRestriction(other_interest.interest_);
  if (buffer) {
    ccnxInterest_SetKeyIdRestriction(interest_, buffer);
  }

  // Content Hash restriction
  buffer = ccnxInterest_GetContentObjectHashRestriction(other_interest.interest_);
  if (buffer) {
    ccnxInterest_SetContentObjectHashRestriction(interest_, buffer);
  }

  // Optional Payload
  buffer = ccnxInterest_GetPayload(other_interest.interest_);
  if (buffer) {
    ccnxInterest_SetPayload(interest_, buffer);
  }

  ccnxInterest_SetHopLimit(interest_, ccnxInterest_GetHopLimit(other_interest.interest_));
  ccnxInterest_SetLifetime(interest_, ccnxInterest_GetLifetime(other_interest.interest_));
}

Interest::Interest(Interest &&other_interest)
    : name_(std::move(other_interest.name_)), interest_(ccnxInterest_Acquire(other_interest.interest_)) {
}

Interest &Interest::operator=(const Interest &other_interest) {
  ccnxInterest_Release(&interest_);
  name_ = other_interest.name_;
  interest_ = ccnxInterest_CreateSimple(name_.getWrappedStructure());
  return *this;
}

Interest::~Interest() {
  ccnxInterest_Release(&interest_);
}

bool Interest::operator==(const Interest &interest) {
  return ccnxInterest_Equals(interest_, interest.interest_);
}

const Name &Interest::getName() const {
  return name_;
}

void Interest::setInterestLifetime(uint32_t lifetime) {
  ccnxInterest_SetLifetime(interest_, lifetime);
}

const uint32_t Interest::getInterestLifetime() const {
  return ccnxInterest_GetLifetime(interest_);
}

bool Interest::setKeyId(const PARCBuffer *keyId) {
  return ccnxInterest_SetKeyIdRestriction(interest_, keyId);
}

PARCBuffer *Interest::getKeyId() {
  return ccnxInterest_GetKeyIdRestriction(interest_);
}

PARCBuffer *Interest::getContentHash() {
  return ccnxInterest_GetContentObjectHashRestriction(interest_);
}

bool Interest::setContentHash(PARCBuffer *hash) {
  return ccnxInterest_SetContentObjectHashRestriction(interest_, hash);
}

std::string Interest::toString() {
  char *str = ccnxInterest_ToString(interest_);
  std::string ret(str);

  free(str);

  return ret;
}

bool Interest::setPayload(const PARCBuffer *payload) {
  return ccnxInterest_SetPayload(interest_, payload);
}

bool Interest::setPayload(const uint8_t *buffer, std::size_t size) {
  PARCBuffer *pbuffer = parcBuffer_CreateFromArray(buffer, size);
  bool ret = setPayload(pbuffer);
  parcBuffer_Release(&pbuffer);
  return ret;
}

bool Interest::setPayloadAndId(const PARCBuffer *payload) {
  return ccnxInterest_SetPayloadAndId(interest_, payload);
}

bool Interest::setPayloadWithId(const PARCBuffer *payload, const CCNxInterestPayloadId *payload_id) {
  return ccnxInterest_SetPayloadWithId(interest_, payload, payload_id);
}

utils::Array Interest::getPayload() const {
  PARCBuffer *buffer = ccnxInterest_GetPayload(interest_);
  return utils::Array(parcBuffer_Overlay(buffer, 0), parcBuffer_Remaining(buffer));
}

void Interest::setHopLimit(uint32_t hop_limit) {
  ccnxInterest_SetHopLimit(interest_, hop_limit);
}

uint32_t Interest::getHopLimit() {
  return ccnxInterest_GetHopLimit(interest_);
}

CCNxInterestStruct *Interest::getWrappedStructure() const {
  return interest_;
}

} // end namespace ccnx

} // end namespace icnet