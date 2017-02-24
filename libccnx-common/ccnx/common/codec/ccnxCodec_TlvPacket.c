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
#include <LongBow/runtime.h>
#include <arpa/inet.h>
#include <ccnx/common/codec/ccnxCodec_TlvPacket.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketEncoder.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h>

static CCNxTlvDictionary *
_decodeV1(PARCBuffer *packetBuffer)
{
    CCNxTlvDictionary *packetDictionary = NULL;

    CCNxCodecSchemaV1Types_PacketType packetType = (CCNxCodecSchemaV1Types_PacketType) parcBuffer_GetAtIndex(packetBuffer, 1);

    switch (packetType) {
        case CCNxCodecSchemaV1Types_PacketType_Interest:
            packetDictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
            break;

        case CCNxCodecSchemaV1Types_PacketType_ContentObject:
            packetDictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
            break;

        case CCNxCodecSchemaV1Types_PacketType_InterestReturn:
            // not implemented yet
            break;

        case CCNxCodecSchemaV1Types_PacketType_Control:
            packetDictionary = ccnxCodecSchemaV1TlvDictionary_CreateControl();
            break;

        default:
            // unknown type
            break;
    }

    if (packetDictionary) {
        // The packetBuffer may be padded or have extraneous content after the CCNx message.
        // Ensure that the buffer limit reflects the CCNx packet length as the decoder uses
        // the that limit, not the packetLength from the header, to determine when to stop parsing.
        size_t packetBufferLength = ccnxCodecTlvPacket_GetPacketLength(packetBuffer);
        assertTrue(packetBufferLength <= parcBuffer_Remaining(packetBuffer), "Short packet buffer");
        parcBuffer_SetLimit(packetBuffer, packetBufferLength);
        bool success = ccnxCodecSchemaV1PacketDecoder_BufferDecode(packetBuffer, packetDictionary);
        if (!success) {
            ccnxTlvDictionary_Release(&packetDictionary);
        }
    }
    return packetDictionary;
}

CCNxTlvDictionary *
ccnxCodecTlvPacket_Decode(PARCBuffer *packetBuffer)
{
    return _decodeV1(packetBuffer);
}

bool
ccnxCodecTlvPacket_BufferDecode(PARCBuffer *packetBuffer, CCNxTlvDictionary *packetDictionary)
{
    // Determine the version from the first byte of the buffer
    uint8_t version = parcBuffer_GetAtIndex(packetBuffer, 0);

    // The packetBuffer may be padded or have extraneous content after the CCNx message.
    // Ensure that the buffer limit reflects the CCNx packet length as the decoder uses
    // the that limit, not the packetLength from the header, to determine when to stop parsing.
    size_t packetBufferLength = ccnxCodecTlvPacket_GetPacketLength(packetBuffer);
    assertTrue(packetBufferLength <= parcBuffer_Remaining(packetBuffer), "Short packet buffer");
    parcBuffer_SetLimit(packetBuffer, packetBufferLength);

    bool success = false;
    switch (version) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            success = ccnxCodecSchemaV1PacketDecoder_BufferDecode(packetBuffer, packetDictionary);
            break;

        default:
            // will return false
            break;
    }

    return success;
}

/*
 * We don't have an iovec based decoder yet, so linearize the memory and use a PARCBuffer
 * See case 903.
 */
bool
ccnxCodecTlvPacket_IoVecDecode(CCNxCodecNetworkBufferIoVec *vec, CCNxTlvDictionary *packetDictionary)
{
    size_t iovcnt = ccnxCodecNetworkBufferIoVec_GetCount(vec);
    const struct iovec *array = ccnxCodecNetworkBufferIoVec_GetArray(vec);

    PARCBuffer *buffer = NULL;
    if (iovcnt == 1) {
        buffer = parcBuffer_Wrap(array[0].iov_base, array[0].iov_len, 0, array[0].iov_len);
    } else if (iovcnt > 1) {
        // figure out total size, then linearize it
        size_t totalbytes = 0;
        for (int i = 0; i < iovcnt; i++) {
            totalbytes += array[i].iov_len;
        }

        buffer = parcBuffer_Allocate(totalbytes);
        for (int i = 0; i < iovcnt; i++) {
            parcBuffer_PutArray(buffer, array[i].iov_len, array[i].iov_base);
        }

        parcBuffer_Flip(buffer);
    } else {
        return false;
    }

    bool success = ccnxCodecTlvPacket_BufferDecode(buffer, packetDictionary);
    parcBuffer_Release(&buffer);
    return success;
}

CCNxCodecNetworkBufferIoVec *
ccnxCodecTlvPacket_DictionaryEncode(CCNxTlvDictionary *packetDictionary, PARCSigner *signer)
{
    CCNxTlvDictionary_SchemaVersion version = ccnxTlvDictionary_GetSchemaVersion(packetDictionary);

    CCNxCodecNetworkBufferIoVec *iovec = NULL;
    switch (version) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            iovec = ccnxCodecSchemaV1PacketEncoder_DictionaryEncode(packetDictionary, signer);
            break;

        default:
            // will return NULL
            break;
    }
    return iovec;
}

size_t
ccnxCodecTlvPacket_GetPacketLength(PARCBuffer *packetBuffer)
{
    size_t length = 0;

    // Determine the version from the first byte of the buffer
    uint8_t *header = parcBuffer_Overlay(packetBuffer, 0);

    switch (header[0]) {
        case CCNxTlvDictionary_SchemaVersion_V1: { // V1 - from metis_TlvSchemaV1.c:_totalPacketLength
            CCNxCodecSchemaV1FixedHeader *headerV1 = (CCNxCodecSchemaV1FixedHeader *) header;
            length = htons(headerV1->packetLength);
            break;
        }
        default:
            break;
    }

    return length;
}

// When new versions are created they need to be incorporated here so enough header information
// can be read to determine how to proceed.
size_t
ccnxCodecTlvPacket_MinimalHeaderLength()
{
    size_t minimumHeaderLength;
    minimumHeaderLength = sizeof(CCNxCodecSchemaV1FixedHeader);
    return minimumHeaderLength;
}
