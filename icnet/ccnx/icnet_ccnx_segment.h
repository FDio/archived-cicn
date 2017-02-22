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

#ifndef ICNET_CCNX_SEGMENT_H_
#define ICNET_CCNX_SEGMENT_H_

#include "icnet_ccnx_common.h"

extern "C" {
#include <ccnx/common/ccnx_NameLabel.h>
#include <ccnx/common/ccnx_NameSegment.h>
#include <ccnx/common/ccnx_NameSegmentNumber.h>
};

typedef CCNxNameSegment CCNxNameSegmentStructure;

namespace icnet {

namespace ccnx {

class Segment : public std::enable_shared_from_this<Segment> // : public api::Component
{
 public:
  Segment(CCNxNameLabelType type, std::string &segment_value);

  Segment(const Segment &segment);

  Segment(Segment &&otherSegment);

  Segment(CCNxNameSegmentStructure *segment);

  ~Segment();

  Segment &operator=(const Segment &segment);

  bool operator==(const Segment &segment);

  const std::string &toString();

  std::size_t getSize();

  CCNxNameLabelType getType();

  CCNxNameSegmentStructure *getWrappedStructure() const;

  bool isSegment() const;

  uint64_t toSegment() const;

 private:
  CCNxNameSegmentStructure *name_segment_;
  std::string name_segment_string_;
};

} // end namespace ccnx

} // end namespace icnet

#endif // ICNET_CCNX_PORTAL_H_
