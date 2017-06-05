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

#pragma once

#include <cstdint>
#include <cstddef>

namespace icnet {

namespace utils {

//const uint32_t FNV1A_PRIME_32 = 0x01000193;
//const uint32_t FNV1A_OFFSET_32 = 0x811C9DC5;
//const uint64_t FNV1A_PRIME_64 = 0x00000100000001B3ULL;
//const uint64_t FNV1A_OFFSET_64 = 0xCBF29CE484222325ULL;

class Hash {
 public:
  static uint32_t cumulativeHash32(const void *data, std::size_t len, uint32_t lastValue);
  static uint64_t cumulativeHash64(const void *data, std::size_t len, uint64_t lastValue);
  static uint32_t hash32(const void *data, std::size_t len);
  static uint64_t hash64(const void *data, std::size_t len);
 private:

};

}

}