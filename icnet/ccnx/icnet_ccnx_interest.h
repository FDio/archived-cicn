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

#ifndef ICNET_CCNX_INTEREST_H_
#define ICNET_CCNX_INTEREST_H_

#include "icnet_ccnx_common.h"
#include "icnet_utils_array.h"
#include "icnet_ccnx_name.h"

extern "C" {
#include <ccnx/common/ccnx_Interest.h>
};

namespace icnet {

namespace ccnx {

typedef CCNxInterest CCNxInterestStruct;

class Interest : public std::enable_shared_from_this<Interest> {
 public:
  Interest(const Name &interest_name);

  Interest(Name &&name);

  Interest(CCNxInterestStruct *interest);

  Interest(const Interest &other_interest);

  Interest(Interest &&other_interest);

  ~Interest();

  bool operator==(const Interest &interest);

  Interest &operator=(const Interest &other_interest);

  const Name &getName() const;

  void setInterestLifetime(uint32_t lifetime);

  const uint32_t getInterestLifetime() const;

  bool setKeyId(const PARCBuffer *keyId);

  PARCBuffer *getKeyId();

  PARCBuffer *getContentHash();

  bool setContentHash(PARCBuffer *hash);

  std::string toString();

  bool setPayload(const PARCBuffer *payload);

  bool setPayload(const uint8_t *buffer, std::size_t size);

  bool setPayloadAndId(const PARCBuffer *payload);

  bool setPayloadWithId(const PARCBuffer *payload, const CCNxInterestPayloadId *payload_id);

  utils::Array getPayload() const;

  void setHopLimit(uint32_t hop_limit);

  uint32_t getHopLimit();

  CCNxInterestStruct *getWrappedStructure() const;

 private:

  Name name_;
  CCNxInterestStruct *interest_;
};

} // end namespace ccnx

} // end namespace icnet

#endif // ICNET_CCNX_INTEREST_H_
