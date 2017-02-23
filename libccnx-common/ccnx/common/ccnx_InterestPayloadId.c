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

#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_InterestPayloadId.h>
#include <ccnx/common/ccnx_NameSegment.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Object.h>

#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_CryptoHash.h>
#include <parc/security/parc_CryptoHasher.h>

struct ccnx_interest_payload_id {
    CCNxNameSegment *nameSegment;
    uint8_t type;
};

static void
_destroy(CCNxInterestPayloadId **idP)
{
    CCNxInterestPayloadId *id = *idP;
    ccnxNameSegment_Release(&id->nameSegment);
}

parcObject_ExtendPARCObject(CCNxInterestPayloadId, _destroy, ccnxInterestPayloadId_Copy, ccnxInterestPayloadId_ToString,
                            ccnxInterestPayloadId_Equals, ccnxInterestPayloadId_Compare, NULL, NULL);

parcObject_ImplementAcquire(ccnxInterestPayloadId, CCNxInterestPayloadId);

parcObject_ImplementRelease(ccnxInterestPayloadId, CCNxInterestPayloadId);

CCNxInterestPayloadId *
ccnxInterestPayloadId_Create(const PARCBuffer *data, uint8_t type)
{
    CCNxInterestPayloadId *result = parcObject_CreateInstance(CCNxInterestPayloadId);
    parcBuffer_AssertValid(data);

    PARCBuffer *buffer = parcBuffer_Allocate(parcBuffer_Capacity(data) + 1);
    assertTrue(type > CCNxInterestPayloadId_TypeCode_App, "App type must be greater than 0x80");

    parcBuffer_PutUint8(buffer, type);
    parcBuffer_PutBuffer(buffer, data);
    parcBuffer_Flip(buffer);
    result->nameSegment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_PAYLOADID, buffer);
    parcBuffer_Release(&buffer);

    return result;
}

CCNxInterestPayloadId *
ccnxInterestPayloadId_CreateAsSHA256Hash(const PARCBuffer *data)
{
    CCNxInterestPayloadId *result = parcObject_CreateInstance(CCNxInterestPayloadId);

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBuffer(hasher, data);
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    parcCryptoHasher_Release(&hasher);

    PARCBuffer *hashData = parcCryptoHash_GetDigest(hash);
    PARCBuffer *codedHash = parcBuffer_Allocate(parcBuffer_Capacity(hashData) + 1);
    parcBuffer_PutUint8(codedHash, CCNxInterestPayloadId_TypeCode_RFC6920_SHA256);
    parcBuffer_PutBuffer(codedHash, hashData);
    parcBuffer_Flip(codedHash);

    result->nameSegment =
        ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_PAYLOADID, codedHash);

    parcBuffer_Release(&codedHash);
    parcCryptoHash_Release(&hash);

    return result;
}

static CCNxInterestPayloadId *
_ccnxInterestPayloadId_CreateFromNameSegment(const CCNxNameSegment *nameSegment)
{
    ccnxNameSegment_AssertValid(nameSegment);
    assertTrue(ccnxNameSegment_GetType(nameSegment) == CCNxNameLabelType_PAYLOADID,
               "ccnxInterestPayloadId_CreateFromNameSegment: supplied nameSegment is not a PayloadId");
    if (ccnxNameSegment_GetType(nameSegment) != CCNxNameLabelType_PAYLOADID) {
        return NULL;
    }
    CCNxInterestPayloadId *result = parcObject_CreateInstance(CCNxInterestPayloadId);
    result->nameSegment = ccnxNameSegment_Acquire(nameSegment);

    return result;
}

CCNxInterestPayloadId *
ccnxInterestPayloadId_CreateFromSegmentInName(const CCNxName *name)
{
    ccnxName_AssertValid(name);

    CCNxInterestPayloadId *result = NULL;
    size_t count = ccnxName_GetSegmentCount(name);
    for (size_t i = 0; i < count; ++i) {
        CCNxNameSegment *segment = ccnxName_GetSegment(name, i);
        if (ccnxNameSegment_GetType(segment) == CCNxNameLabelType_PAYLOADID) {
            result = _ccnxInterestPayloadId_CreateFromNameSegment(segment);
            break;
        }
    }

    return result;
}



const CCNxNameSegment *
ccnxInterestPayloadId_GetNameSegment(const CCNxInterestPayloadId *id)
{
    return id->nameSegment;
}

PARCBuffer *
ccnxInterestPayloadId_GetValue(const CCNxInterestPayloadId *id)
{
    PARCBuffer *data = ccnxNameSegment_GetValue(id->nameSegment);
    parcBuffer_Rewind(data);
    parcBuffer_SetPosition(data, 1);
    return data;
}

uint8_t
ccnxInterestPayloadId_GetType(const CCNxInterestPayloadId *id)
{
    PARCBuffer *data = ccnxNameSegment_GetValue(id->nameSegment);
    parcBuffer_Rewind(data);
    uint8_t type = parcBuffer_GetUint8(data);
    return type;
}


void
ccnxInterestPayloadId_AssertValid(const CCNxInterestPayloadId *id)
{
    trapIllegalValueIf(ccnxInterestPayloadId_IsValid(id) == false, "CCNxName instance is not valid.");
}

bool
ccnxInterestPayloadId_IsValid(const CCNxInterestPayloadId *id)
{
    bool result = false;
    if (ccnxNameSegment_IsValid(id->nameSegment)) {
        result = true;
    }
    return result;
}

char *
ccnxInterestPayloadId_ToString(const CCNxInterestPayloadId *id)
{
    ccnxInterestPayloadId_AssertValid(id);

    return ccnxNameSegment_ToString(id->nameSegment);
}

CCNxInterestPayloadId *
ccnxInterestPayloadId_Copy(const CCNxInterestPayloadId *sourceId)
{
    ccnxInterestPayloadId_AssertValid(sourceId);

    CCNxInterestPayloadId *result =
        _ccnxInterestPayloadId_CreateFromNameSegment(sourceId->nameSegment);
    return result;
}

bool
ccnxInterestPayloadId_Equals(const CCNxInterestPayloadId *id1, const CCNxInterestPayloadId *id2)
{
    ccnxInterestPayloadId_AssertValid(id1);
    ccnxInterestPayloadId_AssertValid(id2);

    bool result = ((id1 == id2) ||
                   (ccnxNameSegment_Equals(id1->nameSegment, id2->nameSegment)));
    return result;
}

uint32_t
ccnxInterestPayloadId_HashCode(const CCNxInterestPayloadId *id)
{
    ccnxInterestPayloadId_AssertValid(id);
    return ccnxNameSegment_HashCode(id->nameSegment);
}


int
ccnxInterestPayloadId_Compare(const CCNxInterestPayloadId *id1, const CCNxInterestPayloadId *id2)
{
    ccnxInterestPayloadId_AssertValid(id1);
    ccnxInterestPayloadId_AssertValid(id2);

    int result = ccnxNameSegment_Compare(id1->nameSegment, id2->nameSegment);
    return result;
}
