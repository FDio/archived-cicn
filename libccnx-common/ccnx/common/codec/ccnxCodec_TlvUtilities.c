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
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_HashCodec.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>
#include <LongBow/runtime.h>

bool
ccnxCodecTlvUtilities_DecodeContainer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, bool (*typeDecoder)(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length))
{
    while (ccnxCodecTlvDecoder_EnsureRemaining(decoder, 4)) {
        uint16_t type = ccnxCodecTlvDecoder_GetType(decoder);
        uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

        if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, length)) {
            bool success = typeDecoder(decoder, packetDictionary, type, length);
            if (!success) {
                return false;
            }
        } else {
            // overflow!  The TLV length goes beyond the end of the container
            return false;
        }
    }

    // Make sure we used up the whole buffer.  If we're at the end,
    // then it was a successful decode, otherwise something is wrong.
    return ccnxCodecTlvDecoder_IsEmpty(decoder);
}

bool
ccnxCodecTlvUtilities_DecodeSubcontainer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t key, uint16_t length,
                                         bool (*subcontainerDecoder)(CCNxCodecTlvDecoder *, CCNxTlvDictionary *))
{
    bool success = false;
    CCNxCodecTlvDecoder *innerDecoder = ccnxCodecTlvDecoder_GetContainer(decoder, length);
    if (innerDecoder) {
        success = subcontainerDecoder(innerDecoder, packetDictionary);
        ccnxCodecTlvDecoder_Destroy(&innerDecoder);
    }
    return success;
}

bool
ccnxCodecTlvUtilities_PutAsInteger(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int dictionaryKey)
{
    uint64_t value;
    if (ccnxCodecTlvDecoder_GetVarInt(decoder, length, &value)) {
        return ccnxTlvDictionary_PutInteger(packetDictionary, dictionaryKey, value);
    }
    return false;
}

bool
ccnxCodecTlvUtilities_PutAsName(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int arrayKey)
{
    bool success = false;
    CCNxName *name = ccnxCodecSchemaV1NameCodec_DecodeValue(decoder, length);
    success = ccnxTlvDictionary_PutName(packetDictionary, arrayKey, name);
    ccnxName_Release(&name);
    return success;
}

bool
ccnxCodecTlvUtilities_PutAsBuffer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int arrayKey)
{
    PARCBuffer *buffer = ccnxCodecTlvDecoder_GetValue(decoder, length);
    bool success = ccnxTlvDictionary_PutBuffer(packetDictionary, arrayKey, buffer);
    parcBuffer_Release(&buffer);
    return success;
}

bool
ccnxCodecTlvUtilities_PutAsHash(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int arrayKey)
{
    PARCCryptoHash *hash = ccnxCodecSchemaV1HashCodec_DecodeValue(decoder, length);
    bool success = false;
    if (hash != NULL) {
        success = ccnxTlvDictionary_PutObject(packetDictionary, arrayKey, (const PARCObject *) hash);
        parcCryptoHash_Release(&hash);
    }
    return success;
}

bool
ccnxCodecTlvUtilities_PutAsListBuffer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int listKey)
{
    PARCBuffer *buffer = ccnxCodecTlvDecoder_GetValue(decoder, length);
    bool success = ccnxTlvDictionary_PutListBuffer(packetDictionary, listKey, type, buffer);
    parcBuffer_Release(&buffer);
    return success;
}

ssize_t
ccnxCodecTlvUtilities_NestedEncode(CCNxCodecTlvEncoder *outerEncoder, CCNxTlvDictionary *packetDictionary, uint32_t nestedType,
                                   ssize_t (*nestedEncoderFunction)(CCNxCodecTlvEncoder *protoInfoEncoder, CCNxTlvDictionary *packetDictionary))
{
    size_t startPosition = ccnxCodecTlvEncoder_Position(outerEncoder);

    ccnxCodecTlvEncoder_AppendContainer(outerEncoder, nestedType, 0);
    ssize_t nestedLength = nestedEncoderFunction(outerEncoder, packetDictionary);
    if (nestedLength > 0) {
        ccnxCodecTlvEncoder_SetContainerLength(outerEncoder, startPosition, nestedLength);
    } else {
        // rewind the container
        ccnxCodecTlvEncoder_SetPosition(outerEncoder, startPosition);
        return nestedLength;
    }

    size_t endPosition = ccnxCodecTlvEncoder_Position(outerEncoder);
    return endPosition - startPosition;
}

ssize_t
ccnxCodecTlvUtilities_EncodeCustomList(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary, int listKey)
{
    ssize_t length = 0;

    size_t size = ccnxTlvDictionary_ListSize(packetDictionary, listKey);
    for (int i = 0; i < size; i++) {
        PARCBuffer *buffer;
        uint32_t key;

        ssize_t innerLength = -1;
        bool success = ccnxTlvDictionary_ListGetByPosition(packetDictionary, listKey, i, &buffer, &key);
        if (success) {
            innerLength = ccnxCodecTlvEncoder_AppendBuffer(encoder, key, buffer);
        }

        if (innerLength < 0) {
            return -1;
        }

        length += innerLength;
    }

    return length;
}

bool
ccnxCodecTlvUtilities_GetVarInt(PARCBuffer *input, size_t length, uint64_t *output)
{
    assertNotNull(input, "Parameter input must be non-null");
    assertNotNull(output, "Parameter output must be non-null");

    bool success = false;
    if (length >= 1 && length <= 8 && parcBuffer_Remaining(input) >= length) {
        uint64_t value = 0;
        for (int i = 0; i < length; i++) {
            value = value << 8 | parcBuffer_GetUint8(input);
        }
        *output = value;
        success = true;
    }
    return success;
}
