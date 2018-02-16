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

#include "icnet_utils_hash.h"

namespace utils {

uint32_t Hash::hash32(const void *data, std::size_t len) {
  const uint32_t fnv1a_offset = 0x811C9DC5;
  return Hash::cumulativeHash32(data, len, fnv1a_offset);
}

uint32_t Hash::cumulativeHash32(const void *data, std::size_t len, uint32_t lastValue) {
  // Standard FNV 32-bit prime: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
  const uint32_t fnv1a_prime = 0x01000193;
  uint32_t hash = lastValue;

  const char *chardata = (char *) data;

  for (std::size_t i = 0; i < len; i++) {
    hash = hash ^ chardata[i];
    hash = hash * fnv1a_prime;
  }

  return hash;
}

uint64_t Hash::hash64(const void *data, std::size_t len) {
  // Standard FNV 64-bit offset: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
  const uint64_t fnv1a_offset = 0xCBF29CE484222325ULL;
  return cumulativeHash64(data, len, fnv1a_offset);
}

uint64_t Hash::cumulativeHash64(const void *data, std::size_t len, uint64_t lastValue) {
  // Standard FNV 64-bit prime: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
  const uint64_t fnv1a_prime = 0x00000100000001B3ULL;
  uint64_t hash = lastValue;
  const char *chardata = (char *) data;

  for (std::size_t i = 0; i < len; i++) {
    hash = hash ^ chardata[i];
    hash = hash * fnv1a_prime;
  }

  return hash;
}

}