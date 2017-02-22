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

#ifndef ICNET_CCNX_NAME_H_
#define ICNET_CCNX_NAME_H_

#include "icnet_ccnx_common.h"

#include <unordered_map>
#include <assert.h>
#include <list>

extern "C" {
#include <ccnx/common/ccnx_Name.h>
};

//#include "name.hpp"
#include "icnet_ccnx_segment.h"
#include <vector>

typedef CCNxName CCNxNameStructure;

namespace icnet {

namespace ccnx {

class Name : public std::enable_shared_from_this<Name> // : public api::Name
{
 public:
  Name();

  /**
   * @brief Create name
   * @param uri The null-terminated URI string
   */
  Name(const char *uri);

  /**
   * @brief Create name from @p uri (ICN URI scheme)
   * @param uri The URI string
   */
  Name(std::string uri);

  Name(const Name &name);

  Name(Name &&otherName);

  Name(const CCNxNameStructure *name);

  Name &operator=(const Name &name);

  Name &operator=(Name &&name);

  bool operator==(const Name &name) const;

  bool isValid();

  const std::string &toString();

  void appendComponent(const Segment &component);

  void append(const Name &suffix);

  Name getPrefix(ssize_t number_of_components) const;

  Name &appendSegment(const uint64_t chunk_number);

  bool isPrefixOf(const Name &name);

  bool equals(const Name &name);

  Segment get(ssize_t index) const;

  Name getSubName(ssize_t start_component, ssize_t number_of_components = -1) const;

  bool empty() const;

  void clear();

  std::size_t getSegmentCount();

  std::size_t size();

  ~Name();

  CCNxNameStructure *getWrappedStructure() const;

 private:

  CCNxNameStructure *name_;
  std::string name_string_;
};

std::ostream &operator<<(std::ostream &os, const Name &name);

} // end namespace ccnx

} // end namespace icnet

namespace std {
template<>
struct hash<icnet::ccnx::Name> {
  size_t operator()(const icnet::ccnx::Name &name) const;
};

} // end namespace std

#endif // ICNET_CCNX_NAME_H_
