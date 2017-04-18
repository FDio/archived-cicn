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

#include "icnet_ccnx_name.h"

namespace icnet {

namespace ccnx {

Name::Name() {
  name_ = ccnxName_Create();
}

Name::Name(const char *uri)
    : name_(ccnxName_CreateFromCString(uri)) {
  ccnxName_AssertValid(name_);
}

Name::Name(std::string uri)
    : Name(uri.c_str()) {
}

Name::Name(const CCNxNameStructure *name)
    : name_(ccnxName_Acquire(name)) {
  ccnxName_AssertValid(name_);
}

Name::Name(const Name &name)
    : name_(ccnxName_Copy(name.name_)) {
}

Name::Name(Name &&otherName)
    : name_(ccnxName_Acquire(otherName.name_)) {
}

Name &Name::operator=(const Name &name) {
  ccnxName_Release(&this->name_);
  this->name_ = ccnxName_Copy(name.name_);
  return *this;
}

Name &Name::operator=(Name &&name) {
  ccnxName_Release(&this->name_);
  this->name_ = ccnxName_Acquire(name.name_);
  return *this;
}

bool Name::operator==(const Name &name) const {
  return ccnxName_Equals(this->name_, name.name_);
}

Name::~Name() {
  ccnxName_Release(&name_);
}

bool Name::equals(const Name &name) {
  return ccnxName_Equals(this->name_, name.name_);
}

bool Name::isValid() {
  return ccnxName_IsValid(name_);
}

const std::string &Name::toString() {
  char *str = ccnxName_ToString(name_);
  name_string_ = std::string(str);
  free(str);
  return name_string_;
}

void Name::appendComponent(const Segment &suffix) {
  ccnxName_Append(name_, suffix.getWrappedStructure());
}

void Name::append(const Name &suffix) {
  if (ccnxName_IsValid(suffix.name_)) {

    size_t number_of_components = ccnxName_GetSegmentCount(suffix.getWrappedStructure());

    for (uint32_t i = 0; i < number_of_components; i++) {
      ccnxName_Append(name_, ccnxName_GetSegment(suffix.getWrappedStructure(), i));
    }
  }
}

Name Name::getPrefix(ssize_t number_of_components) const {
  std::size_t segment_count = ccnxName_GetSegmentCount(name_);

  if (number_of_components >= 0) {
    assert((std::size_t) number_of_components < segment_count);
    return getSubName(0, number_of_components);
  } else {
    assert(segment_count + number_of_components >= 0);
    return getSubName(0, number_of_components + segment_count);
  }

}

Segment Name::get(ssize_t index) const {
  std::size_t segment_count = ccnxName_GetSegmentCount(name_);
  size_t componentIndex = 0;

  if (index >= 0) {
    assert((size_t) index < segment_count);
    componentIndex = (size_t) index;
  } else {
    assert(segment_count + index >= 0);
    componentIndex = segment_count + index;
  }

  CCNxNameSegment *segment = ccnxName_GetSegment(name_, componentIndex);
  Segment ret(segment);

  return ret;
}

Name Name::getSubName(ssize_t start_component, ssize_t number_of_components) const {

  size_t name_size = ccnxName_GetSegmentCount(name_);
  size_t begin;

  if (start_component >= 0) {
    assert((size_t) start_component < name_size);
    begin = static_cast<size_t>(start_component);
  } else {
    assert((ssize_t) (start_component + name_size) >= 0);
    begin = start_component + name_size;
  }

  size_t end = begin;
  end += number_of_components < 0 ? name_size : static_cast<size_t>(number_of_components);

  if (end >= name_size) {
    end = name_size;
  }

  CCNxName *name = ccnxName_Create();

  for (size_t i = begin; i < end; i++) {
    ccnxName_Append(name, ccnxName_GetSegment(name_, i));
  }

  Name ret(name);
  ccnxName_Release(&name);

  return ret;
}

bool Name::isPrefixOf(const Name &name) {
  Name &ccnx_name = (Name &) name;
  return ccnxName_StartsWith(ccnx_name.name_, name_);
}

Name &Name::appendSegment(const uint64_t chunk_number) {
  CCNxNameSegmentStructure *ns = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, chunk_number);
  name_ = ccnxName_Append(name_, ns);
  ccnxNameSegment_Release(&ns);

  return *this;
}

bool Name::empty() const {
  return ccnxName_GetSegmentCount(name_) == 0;
}

void Name::clear() {
  ccnxName_Release(&name_);
  name_ = ccnxName_Create();
}

std::size_t Name::getSegmentCount() {
  return ccnxName_GetSegmentCount(name_);
}

std::size_t Name::size() {
  std::size_t number_of_segments = ccnxName_GetSegmentCount(name_);
  std::size_t name_bytes = 0;

  for (std::size_t i = 0; i < number_of_segments; i++) {
    name_bytes += ccnxNameSegment_Length(ccnxName_GetSegment(name_, i));
  }

  return name_bytes;
}

std::ostream &operator<<(std::ostream &os, const Name &name) {
  const std::string &str = const_cast<Name &>(name).toString();

  if (name.empty()) {
    os << "ccnx:/";
  } else {
    os << str;
  }

  return os;
}

CCNxNameStructure *Name::getWrappedStructure() const {
  return name_;
}

} // end namespace ccnx

} // end namespace icnet

namespace std {
size_t hash<icnet::ccnx::Name>::operator()(const icnet::ccnx::Name &name) const {
  return ccnxName_HashCode(name.getWrappedStructure());;
}

} // end namespace std