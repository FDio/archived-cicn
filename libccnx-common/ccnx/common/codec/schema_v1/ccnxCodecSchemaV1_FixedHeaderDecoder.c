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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>

static const size_t _fixedHeaderBytes = 8;

static const int _fixedHeader_VersionOffset = 0;
static const int _fixedHeader_PacketTypeOffset = 1;
static const int _fixedHeader_PacketLengthOffset = 2;
static const int _fixedHeader_HopLimitOffset = 4;
static const int _fixedHeader_ReturnCodeOffset = 5;
static const int _fixedHeader_FlagsOffset = 6;
static const int _fixedHeader_HeaderLengthOffset = 7;

bool
ccnxCodecSchemaV1FixedHeaderDecoder_Decode(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary)
{
    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, _fixedHeaderBytes)) {
        PARCBuffer *buffer = ccnxCodecTlvDecoder_GetValue(decoder, _fixedHeaderBytes);
        bool success = ccnxTlvDictionary_PutBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader, buffer);

        // validation
        parcBuffer_SetPosition(buffer, _fixedHeader_VersionOffset);
        uint8_t version = parcBuffer_GetUint8(buffer);

        parcBuffer_SetPosition(buffer, _fixedHeader_PacketLengthOffset);
        uint16_t packetLength = parcBuffer_GetUint16(buffer);

        parcBuffer_SetPosition(buffer, _fixedHeader_ReturnCodeOffset);
        uint8_t interestReturnCode = parcBuffer_GetUint8(buffer);

        parcBuffer_SetPosition(buffer, _fixedHeader_HopLimitOffset);
        uint8_t hopLimit = parcBuffer_GetUint8(buffer);

        parcBuffer_SetPosition(buffer, _fixedHeader_HeaderLengthOffset);
        uint8_t headerLength = parcBuffer_GetUint8(buffer);

        if (version != 1) {
            CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_VERSION, __func__, __LINE__, _fixedHeader_VersionOffset);
            ccnxCodecTlvDecoder_SetError(decoder, error);
            ccnxCodecError_Release(&error);
            success = false;
        } else if (packetLength < _fixedHeaderBytes) {
            CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_PACKETLENGTH_TOO_SHORT, __func__, __LINE__, _fixedHeader_PacketTypeOffset);
            ccnxCodecTlvDecoder_SetError(decoder, error);
            ccnxCodecError_Release(&error);
            success = false;
        } else if (headerLength < _fixedHeaderBytes) {
            CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_HEADERLENGTH_TOO_SHORT, __func__, __LINE__, _fixedHeader_HeaderLengthOffset);
            ccnxCodecTlvDecoder_SetError(decoder, error);
            ccnxCodecError_Release(&error);
            success = false;
        } else if (packetLength < headerLength) {
            CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_PACKETLENGTHSHORTER, __func__, __LINE__, _fixedHeader_PacketTypeOffset);
            ccnxCodecTlvDecoder_SetError(decoder, error);
            ccnxCodecError_Release(&error);
            success = false;
        }

        // decoder now points to just past the fixed header
        parcBuffer_Release(&buffer);

        // Set the hoplimit in the dictionary.
        ccnxTlvDictionary_PutInteger(packetDictionary,
                                     CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT, hopLimit);

        // Set the InterestReturn code in the dictionary.
        ccnxTlvDictionary_PutInteger(packetDictionary,
                                     CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestReturnCode,
                                     interestReturnCode);

        return success;
    } else {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
        ccnxCodecTlvDecoder_SetError(decoder, error);
        ccnxCodecError_Release(&error);
        return false;
    }
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetVersion(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *fixedHeader = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader);
    if (fixedHeader != NULL) {
        parcBuffer_SetPosition(fixedHeader, _fixedHeader_VersionOffset);
        uint8_t version = parcBuffer_GetUint8(fixedHeader);
        return version;
    }

    return -1;
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetPacketType(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *fixedHeader = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader);
    if (fixedHeader != NULL) {
        parcBuffer_SetPosition(fixedHeader, _fixedHeader_PacketTypeOffset);
        uint8_t packetType = parcBuffer_GetUint8(fixedHeader);
        return packetType;
    }

    return -1;
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetPacketLength(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *fixedHeader = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader);
    if (fixedHeader != NULL) {
        parcBuffer_SetPosition(fixedHeader, _fixedHeader_PacketLengthOffset);
        uint16_t payloadLength = parcBuffer_GetUint16(fixedHeader);
        return payloadLength;
    }

    return -1;
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetHeaderLength(CCNxTlvDictionary *packetDictionary)
{
    int length = -1;
    PARCBuffer *fixedHeader = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader);
    if (fixedHeader != NULL) {
        parcBuffer_SetPosition(fixedHeader, _fixedHeader_HeaderLengthOffset);
        uint8_t headerLength = parcBuffer_GetUint8(fixedHeader);

        // 8 is the minimum size of headerLength
        if (headerLength >= _fixedHeaderBytes) {
            length = headerLength;
        }
    }

    return length;
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetOptionalHeaderLength(CCNxTlvDictionary *packetDictionary)
{
    int headerLength = ccnxCodecSchemaV1FixedHeaderDecoder_GetHeaderLength(packetDictionary);
    return headerLength - _fixedHeaderBytes;
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetHopLimit(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *fixedHeader = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader);
    if (fixedHeader != NULL) {
        parcBuffer_SetPosition(fixedHeader, _fixedHeader_HopLimitOffset);
        uint8_t hopLimit = parcBuffer_GetUint8(fixedHeader);
        return hopLimit;
    }

    return -1;
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetReturnCode(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *fixedHeader = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader);
    if (fixedHeader != NULL) {
        parcBuffer_SetPosition(fixedHeader, _fixedHeader_ReturnCodeOffset);
        uint8_t returnCode = parcBuffer_GetUint8(fixedHeader);
        return returnCode;
    }

    return -1;
}

int
ccnxCodecSchemaV1FixedHeaderDecoder_GetFlags(CCNxTlvDictionary *packetDictionary)
{
    PARCBuffer *fixedHeader = ccnxTlvDictionary_GetBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader);
    if (fixedHeader != NULL) {
        parcBuffer_SetPosition(fixedHeader, _fixedHeader_FlagsOffset);
        uint8_t flags = parcBuffer_GetUint8(fixedHeader);
        return flags;
    }

    return -1;
}
