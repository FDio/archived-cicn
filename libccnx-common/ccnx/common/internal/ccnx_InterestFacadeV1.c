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
#include <stdlib.h>
#include <LongBow/runtime.h>

#include <ccnx/common/internal/ccnx_InterestFacadeV1.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>
#include <ccnx/common/internal/ccnx_InterestDefault.h>


// =====================

static void
_assertInvariants(const CCNxTlvDictionary *interestDictionary)
{
    assertNotNull(interestDictionary, "Dictionary is null");
    assertTrue((ccnxTlvDictionary_IsInterest(interestDictionary) ||
                ccnxTlvDictionary_IsInterestReturn(interestDictionary)), "Dictionary is not an interest");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(interestDictionary) == CCNxTlvDictionary_SchemaVersion_V1,
               "Dictionary is wrong schema Interest, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(interestDictionary), CCNxTlvDictionary_SchemaVersion_V1);
}

static uint32_t
_fetchUint32(const CCNxTlvDictionary *interestDictionary, uint32_t key, uint32_t defaultValue)
{
    if (ccnxTlvDictionary_IsValueInteger(interestDictionary, key)) {
        return (uint32_t) ccnxTlvDictionary_GetInteger(interestDictionary, key);
    }
    return defaultValue;
}

static bool
_ccnxInterestFacadeV1_SetContentObjectHashRestriction(CCNxTlvDictionary *interestDictionary, const PARCBuffer *contentObjectHash)
{
    _assertInvariants(interestDictionary);
    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, contentObjectHash);
    bool result = ccnxTlvDictionary_PutObject(interestDictionary,
                                              CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION,
                                              hash);
    parcCryptoHash_Release(&hash);
    return result;
}

// forward declaration. Body below, in Getters.
static CCNxName *
_ccnxInterestFacadeV1_GetName(const CCNxTlvDictionary *interestDictionary);

static bool
_ccnxInterestFacadeV1_SetPayloadWithId(CCNxTlvDictionary *interestDictionary, const PARCBuffer *payload, const CCNxInterestPayloadId *payloadId)
{
    bool result = false;

    if (payload) {
        result = ccnxTlvDictionary_PutBuffer(interestDictionary,
                                             CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, payload);
        if (payloadId != NULL) {
            CCNxName *name = _ccnxInterestFacadeV1_GetName(interestDictionary);
            ccnxName_Append(name, ccnxInterestPayloadId_GetNameSegment(payloadId));
        }
    }
    return result;
}

static bool
_ccnxInterestFacadeV1_SetPayloadAndId(CCNxTlvDictionary *interestDictionary, const PARCBuffer *payload)
{
    bool result = false;

    if (payload) {
        CCNxInterestPayloadId *pid = ccnxInterestPayloadId_CreateAsSHA256Hash(payload);
        _ccnxInterestFacadeV1_SetPayloadWithId(interestDictionary, payload, pid);
        ccnxInterestPayloadId_Release(&pid);
    }
    return result;
}

static bool
_ccnxInterestFacadeV1_SetPayload(CCNxTlvDictionary *interestDictionary, const PARCBuffer *payload)
{
    bool result = false;

    if (payload) {
        _ccnxInterestFacadeV1_SetPayloadWithId(interestDictionary, payload, NULL);
    }
    return result;
}

static bool
_ccnxInterestFacadeV1_SetLifetime(CCNxTlvDictionary *interestDictionary, uint32_t lifetimeInMillis)
{
    return ccnxTlvDictionary_PutInteger(interestDictionary,
                                        CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime, lifetimeInMillis);
}

static bool
_ccnxInterestFacadeV1_SetKeyIdRestriction(CCNxTlvDictionary *interestDictionary, const PARCBuffer *keyId)
{
    _assertInvariants(interestDictionary);
    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, keyId);
    bool result = ccnxTlvDictionary_PutObject(interestDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION, hash);
    parcCryptoHash_Release(&hash);
    return result;
}

static bool
_ccnxInterestFacadeV1_SetHopLimit(CCNxTlvDictionary *interestDictionary, uint32_t hopLimit)
{
    _assertInvariants(interestDictionary);
    return ccnxTlvDictionary_PutInteger(interestDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT, hopLimit);
}

// =====================
// Getters

static CCNxName *
_ccnxInterestFacadeV1_GetName(const CCNxTlvDictionary *interestDictionary)
{
    int key = CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME;
    _assertInvariants(interestDictionary);
    if (ccnxTlvDictionary_IsValueName(interestDictionary, key)) {
        return ccnxTlvDictionary_GetName(interestDictionary, key);
    }
    return NULL;
}

static uint32_t
_ccnxInterestFacadeV1_GetLifetime(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    return _fetchUint32(interestDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime, CCNxInterestDefault_LifetimeMilliseconds);
}

static PARCBuffer *
_ccnxInterestFacadeV1_GetKeyIdRestriction(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    PARCCryptoHash *hash = ccnxTlvDictionary_GetObject(interestDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION);
    if (hash != NULL) {
        return parcCryptoHash_GetDigest(hash);
    } else {
        return NULL;
    }
}

static PARCBuffer *
_ccnxInterestFacadeV1_GetContentObjectHashRestriction(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    PARCCryptoHash *hash = ccnxTlvDictionary_GetObject(interestDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION);
    if (hash != NULL) {
        return parcCryptoHash_GetDigest(hash);
    } else {
        return NULL;
    }
}

static PARCBuffer *
_ccnxInterestFacadeV1_GetPayload(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    return ccnxTlvDictionary_GetBuffer(interestDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
}

static uint32_t
_ccnxInterestFacadeV1_GetHopLimit(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    return _fetchUint32(interestDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT, CCNxInterestDefault_HopLimit);
}

// =====================
// Miscellaneous

static void
_ccnxInterestFacadeV1_AssertValid(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    assertTrue(ccnxTlvDictionary_IsValueName(interestDictionary,
                                             CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME), "Name field is not a name");
}

static bool
_ccnxInterestFacadeV1_Equals(const CCNxTlvDictionary *a, const CCNxTlvDictionary *b)
{
    return ccnxTlvDictionary_Equals(a, b);
}

static void
_ccnxInterestFacadeV1_Display(const CCNxTlvDictionary *interestDictionary, size_t indentation)
{
    _assertInvariants(interestDictionary);
    ccnxTlvDictionary_Display(interestDictionary, (unsigned) indentation);
}


// =====================
// Creation

static CCNxTlvDictionary *
_ccnxInterestFacadeV1_Create(const CCNxName   *name,                       // required
                             const uint32_t lifetimeMilliseconds,          // may use DefaultLimetimeMilliseconds
                             const PARCBuffer *keyId,                      // may be NULL
                             const PARCBuffer *contentObjectHash,          // may be NULL
                             const uint32_t hopLimit)                      // may be DefaultHopLimit
{
    assertNotNull(name, "Parameter name must be non-null");

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    if (dictionary) {
        ccnxTlvDictionary_PutName(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME, name);

        if (lifetimeMilliseconds != CCNxInterestDefault_LifetimeMilliseconds) {
            _ccnxInterestFacadeV1_SetLifetime(dictionary, lifetimeMilliseconds);
        }

        if (keyId) {
            _ccnxInterestFacadeV1_SetKeyIdRestriction(dictionary, keyId);
        }

        if (contentObjectHash) {
            _ccnxInterestFacadeV1_SetContentObjectHashRestriction(dictionary, contentObjectHash);
        }

        if (hopLimit != CCNxInterestDefault_HopLimit) {
            _ccnxInterestFacadeV1_SetHopLimit(dictionary, hopLimit);
        }
    } else {
        trapOutOfMemory("Could not allocate an Interest");
    }

    return dictionary;
}

static CCNxTlvDictionary *
_ccnxInterestFacadeV1_CreateSimple(const CCNxName *name)
{
    return _ccnxInterestFacadeV1_Create(name,
                                        CCNxInterestDefault_LifetimeMilliseconds,
                                        NULL,                         // keyid
                                        NULL,                         // content object hash
                                        CCNxInterestDefault_HopLimit);
}

CCNxInterestInterface CCNxInterestFacadeV1_Implementation = {
    .description                     = "CCNxInterestFacadeV1_Implementation",

    .createSimple                    = &_ccnxInterestFacadeV1_CreateSimple,
    .create                          = &_ccnxInterestFacadeV1_Create,

    .getName                         = &_ccnxInterestFacadeV1_GetName,

    .setContentObjectHashRestriction = &_ccnxInterestFacadeV1_SetContentObjectHashRestriction,
    .getContentObjectHashRestriction = &_ccnxInterestFacadeV1_GetContentObjectHashRestriction,

    .setLifetime                     = &_ccnxInterestFacadeV1_SetLifetime,
    .getLifetime                     = &_ccnxInterestFacadeV1_GetLifetime,

    .setKeyIdRestriction             = &_ccnxInterestFacadeV1_SetKeyIdRestriction,
    .getKeyIdRestriction             = &_ccnxInterestFacadeV1_GetKeyIdRestriction,

    .getHopLimit                     = &_ccnxInterestFacadeV1_GetHopLimit,
    .setHopLimit                     = &_ccnxInterestFacadeV1_SetHopLimit,

    .getPayload                      = &_ccnxInterestFacadeV1_GetPayload,

    .setPayload                      = &_ccnxInterestFacadeV1_SetPayload,
    .setPayloadAndId                 = &_ccnxInterestFacadeV1_SetPayloadAndId,
    .setPayloadWithId                = &_ccnxInterestFacadeV1_SetPayloadWithId,

    .toString                        = NULL,
    .equals                          = &_ccnxInterestFacadeV1_Equals,
    .display                         = &_ccnxInterestFacadeV1_Display,

    .assertValid                     = &_ccnxInterestFacadeV1_AssertValid,
};
