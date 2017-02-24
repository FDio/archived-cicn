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

#include <ccnx/common/ccnx_PayloadType.h>
#include <ccnx/common/ccnx_InterestReturn.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_ManifestEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_MessageEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_HashCodec.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>

#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>

#include <ccnx/common/ccnx_Manifest.h>
#include <ccnx/common/ccnx_ManifestHashGroup.h>

static ssize_t
_encodeName(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = -1;
    CCNxName *name = ccnxTlvDictionary_GetName(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
    if (name != NULL) {
        length = ccnxCodecSchemaV1NameCodec_Encode(encoder, CCNxCodecSchemaV1Types_CCNxMessage_Name, name);
    }

    // required field for everything except CCNxContentObjects
    if (!ccnxTlvDictionary_IsContentObject(packetDictionary) && length < 0) {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_MISSING_MANDATORY, __func__, __LINE__, ccnxCodecTlvEncoder_Position(encoder));
        ccnxCodecTlvEncoder_SetError(encoder, error);
        ccnxCodecError_Release(&error);
    } else if (ccnxTlvDictionary_IsContentObject(packetDictionary) && name == NULL) {
        length = 0;
    }

    return length;
}

static ssize_t
_encodeJsonPayload(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    PARCJSON *json = ccnxTlvDictionary_GetJson(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
    if (json != NULL) {
        char *jsonString = parcJSON_ToCompactString(json);
        size_t len = strlen(jsonString);
        length = ccnxCodecTlvEncoder_AppendArray(encoder, CCNxCodecSchemaV1Types_CCNxMessage_Payload, len, (uint8_t *) jsonString);
    }
    return length;
}

static ssize_t
_encodePayload(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
    if (buffer != NULL) {
        length = ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxMessage_Payload, buffer);
    }
    return length;
}

static ssize_t
_encodePayloadType(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    if (ccnxTlvDictionary_IsValueInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE)) {
        CCNxPayloadType payloadType = (CCNxPayloadType) ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE);

        CCNxCodecSchemaV1Types_PayloadType wireFormatType = CCNxCodecSchemaV1Types_PayloadType_Data;

        switch (payloadType) {
            case CCNxPayloadType_KEY:
                wireFormatType = CCNxCodecSchemaV1Types_PayloadType_Key;
                break;

            case CCNxPayloadType_LINK:
                wireFormatType = CCNxCodecSchemaV1Types_PayloadType_Link;
                break;

            default:
                // anything else is encoded as DATA
                break;
        }

        length = ccnxCodecTlvEncoder_AppendUint8(encoder, CCNxCodecSchemaV1Types_CCNxMessage_PayloadType, wireFormatType);
    } else if (ccnxTlvDictionary_IsValueBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE)) {
        PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE);
        length = ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxMessage_PayloadType, buffer);
    }

    return length;
}

static ssize_t
_encodeExpiryTime(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    if (ccnxTlvDictionary_IsValueInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME)) {
        uint64_t millis = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME);
        length = ccnxCodecTlvEncoder_AppendUint64(encoder, CCNxCodecSchemaV1Types_CCNxMessage_ExpiryTime, millis);
    } else if (ccnxTlvDictionary_IsValueBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME)) {
        PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME);
        length = ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxMessage_ExpiryTime, buffer);
    }

    return length;
}

static ssize_t
_encodeEndChunkNumber(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    if (ccnxTlvDictionary_IsValueInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT)) {
        uint64_t endChunkId = ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT);
        length = ccnxCodecTlvEncoder_AppendVarInt(encoder, CCNxCodecSchemaV1Types_CCNxMessage_EndChunkNumber, endChunkId);
    } else {
        PARCBuffer *buffer = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT);
        if (buffer != NULL) {
            length = ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxMessage_EndChunkNumber, buffer);
        }
    }
    return length;
}

static ssize_t
_encodeKeyIdRestriction(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    PARCCryptoHash *hash = ccnxTlvDictionary_GetObject(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION);
    if (hash != NULL) {
        size_t startPosition = ccnxCodecTlvEncoder_Position(encoder);
        ccnxCodecTlvEncoder_AppendContainer(encoder, CCNxCodecSchemaV1Types_CCNxMessage_KeyIdRestriction, 0);
        length = ccnxCodecSchemaV1HashCodec_Encode(encoder, hash);
        if (length < 0) {
            return length;
        }

        ccnxCodecTlvEncoder_SetContainerLength(encoder, startPosition, length);
        length += 4; // this accounts for the TL fields
    }
    return length;
}

static ssize_t
_encodeContentObjectHashRestriction(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    PARCCryptoHash *hash = ccnxTlvDictionary_GetObject(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION);
    if (hash != NULL) {
        size_t startPosition = ccnxCodecTlvEncoder_Position(encoder);
        ccnxCodecTlvEncoder_AppendContainer(encoder, CCNxCodecSchemaV1Types_CCNxMessage_ContentObjectHashRestriction, 0);
        length = ccnxCodecSchemaV1HashCodec_Encode(encoder, hash);
        if (length < 0) {
            return length;
        }

        ccnxCodecTlvEncoder_SetContainerLength(encoder, startPosition, length);
        length += 4; // this accounts for the TL fields
    }
    return length;
}


static ssize_t
_encodeContentObject(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    ssize_t result;

    result = _encodeName(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodePayloadType(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodeExpiryTime(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodeEndChunkNumber(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodePayload(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    return length;
}

static ssize_t
_encodeInterest(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    ssize_t result;

    result = _encodeName(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodeKeyIdRestriction(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodeContentObjectHashRestriction(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodePayload(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    return length;
}

static ssize_t
_encodeControl(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    ssize_t result;

    result = _encodeName(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = _encodeJsonPayload(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    return length;
}

static ssize_t
_encodeManifest(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;
    ssize_t result;

    result = _encodeName(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, packetDictionary);
    if (result < 0) {
        return result;
    }
    length += result;

    return length;
}

ssize_t
ccnxCodecSchemaV1MessageEncoder_Encode(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    assertNotNull(packetDictionary, "Parameter packetDictionary must be non-null");

    ssize_t length = -1;

    if (ccnxTlvDictionary_IsInterest(packetDictionary)) {
        length = _encodeInterest(encoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsInterestReturn(packetDictionary)) {
        length = _encodeInterest(encoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsContentObject(packetDictionary)) {
        length = _encodeContentObject(encoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsControl(packetDictionary)) {
        length = _encodeControl(encoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsManifest(packetDictionary)) {
        length = _encodeManifest(encoder, packetDictionary);
    } else {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_PACKETTYPE, __func__, __LINE__, ccnxCodecTlvEncoder_Position(encoder));
        ccnxCodecTlvEncoder_SetError(encoder, error);
        ccnxCodecError_Release(&error);
        length = -1;
    }


    if (length >= 0) {
        // Put custom fields all last
        ssize_t customLength = ccnxCodecTlvUtilities_EncodeCustomList(encoder, packetDictionary, CCNxCodecSchemaV1TlvDictionary_Lists_MESSAGE_LIST);
        if (customLength < 0) {
            return customLength;
        }
        length += customLength;
    }

    return length;
}
