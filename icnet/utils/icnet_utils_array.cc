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

#include "icnet_utils_array.h"

namespace icnet {

namespace utils {

Array::Array(const void *array, size_t size) {
  this->array_ = array;
  this->size_ = size;
}

Array::Array() {
  this->array_ = nullptr;
  this->size_ = 0;
}

const void *Array::data() const {
  return array_;
}

std::size_t Array::size() const {
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

}

}