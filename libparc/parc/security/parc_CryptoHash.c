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
 * @header parcCryptoHash.c
 * <#Abstract#>
 *
 *     <#Discussion#>
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#include <config.h>
#include <stdio.h>

#include <parc/assert/parc_Assert.h>

#include <parc/security/parc_CryptoHash.h>
#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_Object.h>

struct parc_crypto_hash {
    PARCCryptoHashType type;
    PARCBuffer *digestBuffer;
};

static bool
_parcCryptoHash_FinalRelease(PARCCryptoHash **hashP)
{
    PARCCryptoHash *hash = (PARCCryptoHash *) *hashP;
    if (hash->digestBuffer != NULL) {
        parcBuffer_Release(&hash->digestBuffer);
    }
    return true;
}

parcObject_Override(PARCCryptoHash, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcCryptoHash_FinalRelease,
                    .equals = (PARCObjectEquals *) parcCryptoHash_Equals);

parcObject_ImplementAcquire(parcCryptoHash, PARCCryptoHash);

parcObject_ImplementRelease(parcCryptoHash, PARCCryptoHash);

PARCCryptoHash *
parcCryptoHash_Create(PARCCryptoHashType digestType, const PARCBuffer *digestBuffer)
{
    PARCCryptoHash *parcDigest = parcObject_CreateInstance(PARCCryptoHash);
    parcAssertNotNull(parcDigest, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCCryptoHash));
    parcDigest->type = digestType;

    parcDigest->digestBuffer = parcBuffer_Acquire((PARCBuffer *) digestBuffer); // casting to un-const

    return parcDigest;
}

/**
 * Create a digest, copying the buffer
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCCryptoHash *
parcCryptoHash_CreateFromArray(PARCCryptoHashType digestType, const void *buffer, size_t length)
{
    PARCCryptoHash *parcDigest = parcObject_CreateInstance(PARCCryptoHash);
    parcAssertNotNull(parcDigest, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCCryptoHash));
    parcDigest->type = digestType;

    // create a reference counted copy
    parcDigest->digestBuffer =
        parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(length), length, buffer));

    return parcDigest;
}

/**
 * Returns the digest algorithm.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCCryptoHashType
parcCryptoHash_GetDigestType(const PARCCryptoHash *parcDigest)
{
    parcAssertNotNull(parcDigest, "Parameter must be non-null");
    return parcDigest->type;
}

bool
parcCryptoHash_Equals(const PARCCryptoHash *a, const PARCCryptoHash *b)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }

    if (a->type == b->type) {
        if (parcBuffer_Equals(a->digestBuffer, b->digestBuffer)) {
            return true;
        }
    }

    return false;
}

PARCBuffer *
parcCryptoHash_GetDigest(const PARCCryptoHash *parcDigest)
{
    parcAssertNotNull(parcDigest, "Parameter must be non-null");
    return parcDigest->digestBuffer;
}
