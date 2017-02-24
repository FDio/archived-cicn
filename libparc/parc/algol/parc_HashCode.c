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

/**
 */
#include <config.h>

#include <parc/algol/parc_HashCode.h>

#if PARCHashCodeSize == 64
static const PARCHashCode _fnv1a_prime = 0x00000100000001B3ULL;
const PARCHashCode parcHashCode_InitialValue = 0xCBF29CE484222325ULL;
#else
static const PARCHashCode _fnv1a_prime = 0x01000193;
const PARCHashCode parcHashCode_InitialValue = 0x811C9DC5;
#endif

PARCHashCode
parcHashCode_HashImpl(const uint8_t *memory, size_t length, PARCHashCode initialValue)
{
    // Standard FNV 64-bit prime: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param

    PARCHashCode hash = initialValue;

    for (size_t i = 0; i < length; i++) {
        hash = hash ^ memory[i];
        hash = hash * _fnv1a_prime;
    }

    return hash;
}

PARCHashCode
parcHashCode_HashHashCode(PARCHashCode initialValue, PARCHashCode update)
{
    return parcHashCode_HashImpl((uint8_t *) &update, sizeof(PARCHashCode), initialValue);
}
