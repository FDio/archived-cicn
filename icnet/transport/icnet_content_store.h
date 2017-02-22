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

#ifndef ICNET_CONTENT_STORE_H_
#define ICNET_CONTENT_STORE_H_

#include "icnet_socket.h"

#include <mutex>

namespace icnet {

typedef std::pair<std::shared_ptr<ContentObject>, std::list<std::reference_wrapper<const Name>>::iterator>
    CcnxContentStoreEntry;
typedef std::list<std::reference_wrapper<const Name>> LRUList;
typedef std::unordered_map<Name, CcnxContentStoreEntry> ContentStoreHashTable;

class ContentStore {
 public:

  explicit ContentStore(std::size_t max_packets = 65536);

  ~ContentStore();

  void insert(const std::shared_ptr<ContentObject> &content_object);

  const std::shared_ptr<ContentObject> &find(const Interest &interest);

  void erase(const Name &exact_name);

  void setLimit(size_t max_packets);

  size_t getLimit() const;

  size_t size() const;

 private:
  ContentStoreHashTable content_store_hash_table_;
  LRUList lru_list_;
  std::shared_ptr<ContentObject> empty_reference_;
  std::size_t max_content_store_size_;
  std::mutex cs_mutex_;
};

} // end namespace icnet


#endif // ICNET_CONTENT_STORE_H_
