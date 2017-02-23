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
 * This hash is based on FNV-1a, using different lengths.  Please see the FNV-1a
 * website for details on the algorithm: http://www.isthe.com/chongo/tech/comp/fnv
 *
 */
#include <config.h>

#include <stdint.h>
#include <unistd.h>

#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_Object.h>

struct parc_hash_32bits {
    uint32_t accumulator;
};

parcObject_ExtendPARCObject(PARCHash32Bits, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

PARCHash32Bits *
parcHash32Bits_Create(void)
{
    PARCHash32Bits *result = parcObject_CreateInstance(PARCHash32Bits);
    if (result != NULL) {
        result->accumulator = 0;
    }

    return result;
}

PARCHash32Bits *
parcHash32Bits_Update(PARCHash32Bits *hash, const void *data, size_t length)
{
    hash->accumulator = parcHash32_Data_Cumulative(data, length, hash->accumulator);
    return hash;
}

PARCHash32Bits *
parcHash32Bits_UpdateUint32(PARCHash32Bits *hash, uint32_t value)
{
    hash->accumulator = parcHash32_Data_Cumulative(&value, sizeof(value), hash->accumulator);
    return hash;
}

uint32_t
parcHash32Bits_Hash(PARCHash32Bits *hash)
{
    return hash->accumulator;
}

parcObject_ImplementAcquire(parcHash32Bits, PARCHash32Bits);

parcObject_ImplementRelease(parcHash32Bits, PARCHash32Bits);

/*
 * Based on 64-bit FNV-1a
 */
uint64_t
parcHash64_Data(const void *data, size_t len)
{
    // Standard FNV 64-bit offset: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
    const uint64_t fnv1a_offset = 0xCBF29CE484222325ULL;
    return parcHash64_Data_Cumulative(data, len, fnv1a_offset);
}

uint64_t
parcHash64_Data_Cumulative(const void *data, size_t len, uint64_t lastValue)
{
    // Standard FNV 64-bit prime: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
    const uint64_t fnv1a_prime = 0x00000100000001B3ULL;
    uint64_t hash = lastValue;
    const char *chardata = data;

    for (size_t i = 0; i < len; i++) {
        hash = hash ^ chardata[i];
        hash = hash * fnv1a_prime;
    }

    return hash;
}

uint64_t
parcHash64_Int64(uint64_t int64)
{
    return parcHash64_Data(&int64, sizeof(uint64_t));
}

uint64_t
parcHash64_Int32(uint32_t int32)
{
    return parcHash64_Data(&int32, sizeof(uint32_t));
}

uint32_t
parcHash32_Data(const void *data, size_t len)
{
    // Standard FNV 32-bit offset: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
    const uint32_t fnv1a_offset = 0x811C9DC5;
    return parcHash32_Data_Cumulative(data, len, fnv1a_offset);
}

uint32_t
parcHash32_Data_Cumulative(const void *data, size_t len, uint32_t lastValue)
{
    // Standard FNV 32-bit prime: see http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
    const uint32_t fnv1a_prime = 0x01000193;
    uint32_t hash = lastValue;

    const char *chardata = data;

    for (size_t i = 0; i < len; i++) {
        hash = hash ^ chardata[i];
        hash = hash * fnv1a_prime;
    }

    return hash;
}

uint32_t
parcHash32_Int64(uint64_t int64)
{
    return parcHash32_Data(&int64, sizeof(uint64_t));
}

uint32_t
parcHash32_Int32(uint32_t int32)
{
    return parcHash32_Data(&int32, sizeof(uint32_t));
}
