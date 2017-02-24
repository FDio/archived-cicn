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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>

static bool
_decodeType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
{
    bool success = false;
    switch (type) {
        case CCNxCodecSchemaV1Types_OptionalHeaders_InterestFragment:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG);
            break;

        case CCNxCodecSchemaV1Types_OptionalHeaders_ContentObjectFragment:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_OBJFRAG);
            break;

        case CCNxCodecSchemaV1Types_OptionalHeaders_InterestLifetime:
            // its a time, so use an Integer
            success = ccnxCodecTlvUtilities_PutAsInteger(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime);
            break;

        case CCNxCodecSchemaV1Types_OptionalHeaders_RecommendedCacheTime:
            // its a time, so use an Integer
            success = ccnxCodecTlvUtilities_PutAsInteger(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_RecommendedCacheTime);
            break;
        case CCNxCodecSchemaV1Types_OptionalHeaders_PathLabel:
            success = ccnxCodecTlvUtilities_PutAsInteger(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel);
            break;

        default: {
            // if we do not know the TLV type, put it in this container's unknown list
            success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_Lists_HEADERS);
        }
        break;
    }
    if (!success) {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
        ccnxCodecTlvDecoder_SetError(decoder, error);
        ccnxCodecError_Release(&error);
    }
    return success;
}

/*
 * We are given a decoder that points to the first TLV of a list of TLVs.  We keep walking the
 * list until we come to the end of the decoder.
 */
bool
ccnxCodecSchemaV1OptionalHeadersDecoder_Decode(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary)
{
    return ccnxCodecTlvUtilities_DecodeContainer(decoder, packetDictionary, _decodeType);
}

// ==== Getters

PARCBuffer *
ccnxCodecSchemaV1OptionalHeadersDecoder_GetInterestFragmentHeader(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG);
    return buffer;
}

PARCBuffer *
ccnxCodecSchemaV1OptionalHeadersDecoder_GetContentObjectFragmentHeader(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_OBJFRAG);
    return buffer;
}

uint64_t
ccnxCodecSchemaV1OptionalHeadersDecoder_GetInterestLifetimeHeader(CCNxTlvDictionary *packetDictionary)
{
    uint64_t lifetime = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime);
    return lifetime;
}

uint64_t
ccnxCodecSchemaV1OptionalHeadersDecoder_GetRecommendedCacheTimeHeader(CCNxTlvDictionary *packetDictionary)
{
    uint64_t cachetime = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_RecommendedCacheTime);
    return cachetime;
}

uint64_t
ccnxCodecSchemaV1OptionalHeadersDecoder_GetPathLabel(CCNxTlvDictionary *packetDictionary)
{
    uint64_t pathLabel = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel);
    return pathLabel;
}

PARCBuffer *
ccnxCodecSchemaV1OptionalHeadersDecoder_GetCustomType(CCNxTlvDictionary *packetDictionary, uint32_t key)
{
    PARCBuffer *buffer = ccnxTlvDictionary_ListGetByType(packetDictionary, CCNxCodecSchemaV1TlvDictionary_Lists_HEADERS, key);
    return buffer;
}
