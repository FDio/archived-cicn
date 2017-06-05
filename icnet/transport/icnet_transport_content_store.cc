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

#include "icnet_transport_content_store.h"

namespace icnet {

namespace transport {

ContentStore::ContentStore(std::size_t max_packets)
    : max_content_store_size_(max_packets) {
}

ContentStore::~ContentStore() {
  content_store_hash_table_.clear();
}

void ContentStore::insert(const std::shared_ptr<ContentObject> &content_object) {
  std::unique_lock<std::mutex> lock(cs_mutex_);
  if (content_store_hash_table_.size() >= max_content_store_size_) {
    // Evict item
    content_store_hash_table_.erase(lru_list_.back());
    lru_list_.pop_back();
  }

  // Insert new item
  lru_list_.push_back(std::cref(content_object->getName()));
  LRUList::iterator pos = lru_list_.end();
  content_store_hash_table_[content_object->getName()] = CcnxContentStoreEntry(content_object, pos);

}

const std::shared_ptr<ContentObject> &ContentStore::find(const Interest &interest) {
  std::unique_lock<std::mutex> lock(cs_mutex_);
  ContentStoreHashTable::iterator it = content_store_hash_table_.find(interest.getName());
  if (it != content_store_hash_table_.end()) {
    if (it->second.second != lru_list_.begin()) {
      // Move element to the top of the LRU list
      lru_list_.splice(lru_list_.begin(), lru_list_, it->second.second);
    }
    return it->second.first;
  } else {
    return empty_reference_;
  }
}

void ContentStore::erase(const Name &exact_name) {
  std::unique_lock<std::mutex> lock(cs_mutex_);
  ContentStoreHashTable::iterator it = content_store_hash_table_.find(exact_name);
  lru_list_.erase(it->second.second);
  content_store_hash_table_.erase(exact_name);
}

void ContentStore::setLimit(size_t max_packets) {
  max_content_store_size_ = max_packets;
}

std::size_t ContentStore::getLimit() const {
  return max_content_store_size_;
}

std::size_t ContentStore::size() const {
  return content_store_hash_table_.size();
}

}

} // end namespace icnet