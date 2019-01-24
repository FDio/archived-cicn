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

#include <stdio.h>
#include <string.h>

#include <parc/assert/parc_Assert.h>

#include <parc/security/parc_KeyId.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Hash.h>

struct parc_keyid {
    PARCBuffer *keyid;
    PARCHashCode hashcode;
};

static void
_parcKeyId_Destroy(PARCKeyId **keyIdPtr)
{
    PARCKeyId *keyId = *keyIdPtr;

    parcBuffer_Release(&keyId->keyid);
}

parcObject_ExtendPARCObject(PARCKeyId, _parcKeyId_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

void
parcKeyId_AssertValid(const PARCKeyId *keyId)
{
    parcAssertNotNull(keyId, "Pointer must be a non-null pointer to a PARCKeyId");
}

PARCKeyId *
parcKeyId_Create(PARCBuffer *preComputedKeyId)
{
    PARCKeyId *keyid = parcObject_CreateInstance(PARCKeyId);

    if (keyid != NULL) {
        keyid->keyid = parcBuffer_Acquire(preComputedKeyId);
        keyid->hashcode = parcBuffer_HashCode(preComputedKeyId);
    }
    return keyid;
}

parcObject_ImplementAcquire(parcKeyId, PARCKeyId);
parcObject_ImplementRelease(parcKeyId, PARCKeyId);

bool
parcKeyId_Equals(const PARCKeyId *keyidA, const PARCKeyId *keyidB)
{
    if (keyidA == keyidB) {
        return true;
    }

    if (keyidA == NULL || keyidB == NULL) {
        return false;
    }

    return parcBuffer_Equals(keyidA->keyid, keyidB->keyid);
}

PARCHashCode
parcKeyId_HashCode(const PARCKeyId *keyid)
{
    parcKeyId_OptionalAssertValid(keyid);

    return keyid->hashcode;
}

PARCHashCode
parcKeyId_HashCodeFromVoid(const void *keyid)
{
    parcKeyId_OptionalAssertValid(keyid);

    return ((PARCKeyId *) keyid)->hashcode;
}

const PARCBuffer *
parcKeyId_GetKeyId(const PARCKeyId *keyid)
{
    parcKeyId_OptionalAssertValid(keyid);

    return keyid->keyid;
}

PARCKeyId *
parcKeyId_Copy(const PARCKeyId *original)
{
    parcKeyId_OptionalAssertValid(original);

    PARCBuffer *bufferCopy = parcBuffer_Copy(original->keyid);
    PARCKeyId *result = parcKeyId_Create(bufferCopy);
    parcBuffer_Release(&bufferCopy);
    return result;
}

PARCBufferComposer *
parcKeyId_BuildString(const PARCKeyId *keyid, PARCBufferComposer *composer)
{
    // output format = "0x<hex>\00"
    parcBufferComposer_Format(composer, "0x");
    for (int i = 0; i < parcBuffer_Capacity(keyid->keyid); i += 2) {
        parcBufferComposer_Format(composer, "%02X", parcBuffer_GetAtIndex(keyid->keyid, i));
    }
    return composer;
}

char *
parcKeyId_ToString(const PARCKeyId *keyid)
{
    char *result = NULL;

    PARCBufferComposer *composer = parcBufferComposer_Create();
    if (composer != NULL) {
        parcKeyId_BuildString(keyid, composer);
        PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
        result = parcBuffer_ToString(tempBuffer);
        parcBuffer_Release(&tempBuffer);
        parcBufferComposer_Release(&composer);
    }
    return result;
}
