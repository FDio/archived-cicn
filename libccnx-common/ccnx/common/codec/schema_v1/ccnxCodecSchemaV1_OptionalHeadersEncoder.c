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

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>

static ssize_t
_EncodeInterestLifetime(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;

    // allow either encoding as an Integer or as a Buffer

    if (ccnxTlvDictionary_IsValueInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime)) {
        uint64_t lifetime = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime);
        length = ccnxCodecTlvEncoder_AppendVarInt(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_InterestLifetime, lifetime);
    } else if (ccnxTlvDictionary_IsValueBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime)) {
        PARCBuffer *lifetime = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime);
        length = ccnxCodecTlvEncoder_AppendBuffer(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_InterestLifetime, lifetime);
    }

    return length;
}

static ssize_t
_EncodeRecommendedCacheTime(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;

    // allow either encoding as an Integer or as a Buffer

    if (ccnxTlvDictionary_IsValueInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_RecommendedCacheTime)) {
        uint64_t cacheTime = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_RecommendedCacheTime);
        length = ccnxCodecTlvEncoder_AppendVarInt(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_RecommendedCacheTime, cacheTime);
    } else if (ccnxTlvDictionary_IsValueBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_RecommendedCacheTime)) {
        PARCBuffer *cacheTime = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_RecommendedCacheTime);
        length = ccnxCodecTlvEncoder_AppendBuffer(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_RecommendedCacheTime, cacheTime);
    }

    return length;
}

static ssize_t
_EncodePathLabel(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;

    if (ccnxTlvDictionary_IsValueInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel)) {
        uint16_t pathLabel = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel);
        length = ccnxCodecTlvEncoder_AppendVarInt(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_PathLabel, pathLabel);
    } else if (ccnxTlvDictionary_IsValueBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel)) {
        PARCBuffer *pathLabel = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel);
        length = ccnxCodecTlvEncoder_AppendBuffer(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_PathLabel, pathLabel);
    }

    return length;
}

static ssize_t
_EncodeInterestFrag(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG);
    if (buffer != NULL) {
        length = ccnxCodecTlvEncoder_AppendBuffer(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_InterestFragment, buffer);
    }
    return length;
}

static ssize_t
_EncodeContentObjectFrag(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_OBJFRAG);
    if (buffer != NULL) {
        length = ccnxCodecTlvEncoder_AppendBuffer(optionalHeadersEncoder, CCNxCodecSchemaV1Types_OptionalHeaders_ContentObjectFragment, buffer);
    }
    return length;
}

static ssize_t
_EncodeInterestHeaders(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    ssize_t intFragLength = _EncodeInterestFrag(optionalHeadersEncoder, packetDictionary);
    if (intFragLength < 0) {
        return intFragLength;
    }
    length += intFragLength;

    ssize_t intLifeLength = _EncodeInterestLifetime(optionalHeadersEncoder, packetDictionary);
    if (intLifeLength < 0) {
        return intLifeLength;
    }
    length += intLifeLength;

    ssize_t customLength = ccnxCodecTlvUtilities_EncodeCustomList(optionalHeadersEncoder, packetDictionary, CCNxCodecSchemaV1TlvDictionary_Lists_HEADERS);
    if (customLength < 0) {
        return customLength;
    }
    length += customLength;

    return length;
}

static ssize_t
_EncodeContentObjectHeaders(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    ssize_t result;

    result = _EncodeContentObjectFrag(optionalHeadersEncoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _EncodeRecommendedCacheTime(optionalHeadersEncoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _EncodePathLabel(optionalHeadersEncoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = ccnxCodecTlvUtilities_EncodeCustomList(optionalHeadersEncoder, packetDictionary, CCNxCodecSchemaV1TlvDictionary_Lists_HEADERS);
    if (result < 0) {
        return result;
    }
    length += result;

    return length;
}


ssize_t
ccnxCodecSchemaV1OptionalHeadersEncoder_Encode(CCNxCodecTlvEncoder *optionalHeadersEncoder, CCNxTlvDictionary *packetDictionary)
{
    assertNotNull(optionalHeadersEncoder, "Parameter optionalHeadersEncoder must be non-null");
    assertNotNull(packetDictionary, "Parameter packetDictionary must be non-null");

    ssize_t result = 0;
    if (ccnxTlvDictionary_IsInterest(packetDictionary) || ccnxTlvDictionary_IsInterestReturn(packetDictionary)) {
        result = _EncodeInterestHeaders(optionalHeadersEncoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsContentObject(packetDictionary) || ccnxTlvDictionary_IsManifest(packetDictionary)) {
        result = _EncodeContentObjectHeaders(optionalHeadersEncoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsControl(packetDictionary)) {
        result = ccnxCodecTlvUtilities_EncodeCustomList(optionalHeadersEncoder, packetDictionary, CCNxCodecSchemaV1TlvDictionary_Lists_HEADERS);
    } else {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_PACKETTYPE, __func__, __LINE__, ccnxCodecTlvEncoder_Position(optionalHeadersEncoder));
        ccnxCodecTlvEncoder_SetError(optionalHeadersEncoder, error);
        ccnxCodecError_Release(&error);
        result = -1;
    }

    return result;
}
