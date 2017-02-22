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

#include "icnet_ccnx_segment.h"

namespace icnet {

namespace ccnx {

Segment::Segment(CCNxNameLabelType type, std::string &segment_value) : name_segment_(
    ccnxNameSegment_CreateTypeValueArray(type, segment_value.length(), segment_value.c_str())) {
}

Segment::Segment(CCNxNameSegmentStructure *segment) : name_segment_(ccnxNameSegment_Acquire(segment)) {
}

Segment::Segment(const Segment &segment) : name_segment_(ccnxNameSegment_Copy(segment.name_segment_)) {
}

Segment::Segment(Segment &&otherSegment) : name_segment_(ccnxNameSegment_Acquire(otherSegment.name_segment_)) {
}

Segment::~Segment() {
  ccnxNameSegment_Release(&name_segment_);
}

Segment &Segment::operator=(const Segment &segment) {
  ccnxNameSegment_Release(&name_segment_);
  this->name_segment_ = ccnxNameSegment_Copy(segment.name_segment_);
  return *this;
}

bool Segment::operator==(const Segment &segment) {
  return ccnxNameSegment_Equals(this->name_segment_, segment.name_segment_);
}

const std::string &Segment::toString() {
  char *str = ccnxNameSegment_ToString(name_segment_);
  name_segment_string_ = std::string(str);
  free(str);
  return name_segment_string_;
}

std::size_t Segment::getSize() {
  return ccnxNameSegment_Length(name_segment_);
}

CCNxNameLabelType Segment::getType() {
  return ccnxNameSegment_GetType(name_segment_);
}

CCNxNameSegmentStructure *Segment::getWrappedStructure() const {
  return this->name_segment_;
}

bool Segment::isSegment() const {
  return ccnxNameSegmentNumber_IsValid(name_segment_);
}

uint64_t Segment::toSegment() const {
  return ccnxNameSegmentNumber_Value(name_segment_);
}

} // end namespace ccnx

} // end namespace icnet
