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
 * We use a 2-byte T and a 2-byte L
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Buffer.h>
#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>

#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>

#define NONE_SET  0
#define START_SET 1
#define END_SET   2
#define BOTH_SET  3

struct ccnx_codec_tlv_encoder {
    CCNxCodecNetworkBuffer *buffer;

    // OR of NONE_SET, START_SET, END_SET
    int signatureStartEndSet;

    size_t signatureStart;
    size_t signatureEnd;

    CCNxCodecError *error;
    PARCSigner *signer;
};

CCNxCodecTlvEncoder *
ccnxCodecTlvEncoder_Create(void)
{
    CCNxCodecTlvEncoder *encoder = parcMemory_AllocateAndClear(sizeof(CCNxCodecTlvEncoder));
    assertNotNull(encoder, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CCNxCodecTlvEncoder));

    encoder->buffer = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
    encoder->signatureStartEndSet = NONE_SET;
    encoder->error = NULL;

    return encoder;
}

void
ccnxCodecTlvEncoder_Destroy(CCNxCodecTlvEncoder **encoderPtr)
{
    assertNotNull(encoderPtr, "Parameter must be non-null double pointer");
    assertNotNull(*encoderPtr, "Parameter must dereferecne to non-null pointer");
    CCNxCodecTlvEncoder *encoder = *encoderPtr;

    ccnxCodecNetworkBuffer_Release(&encoder->buffer);

    if (encoder->error) {
        ccnxCodecError_Release(&encoder->error);
    }

    if (encoder->signer) {
        parcSigner_Release(&encoder->signer);
    }

    parcMemory_Deallocate((void **) &encoder);
    *encoderPtr = NULL;
}

CCNxCodecTlvEncoder *
ccnxCodecTlvEncoder_Initialize(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    return encoder;
}

size_t
ccnxCodecTlvEncoder_AppendBuffer(CCNxCodecTlvEncoder *encoder, uint16_t type, PARCBuffer *value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    assertTrue(parcBuffer_Remaining(value) <= UINT16_MAX, "Value length too long, got %zu maximum %u\n", parcBuffer_Remaining(value), UINT16_MAX);

    size_t bytes = 4 + parcBuffer_Remaining(value);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, parcBuffer_Remaining(value));
    ccnxCodecNetworkBuffer_PutBuffer(encoder->buffer, value);

    return bytes;
}

size_t
ccnxCodecTlvEncoder_AppendArray(CCNxCodecTlvEncoder *encoder, uint16_t type, uint16_t length, const uint8_t array[length])
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, length);
    ccnxCodecNetworkBuffer_PutArray(encoder->buffer, length, array);
    return length + 4;
}

size_t
ccnxCodecTlvEncoder_AppendContainer(CCNxCodecTlvEncoder *encoder, uint16_t type, uint16_t length)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");

    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, length);
    return 4;
}

size_t
ccnxCodecTlvEncoder_AppendUint8(CCNxCodecTlvEncoder *encoder, uint16_t type, uint8_t value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, 1);
    ccnxCodecNetworkBuffer_PutUint8(encoder->buffer, value);
    return 5;
}

size_t
ccnxCodecTlvEncoder_AppendUint16(CCNxCodecTlvEncoder *encoder, uint16_t type, uint16_t value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, 2);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, value);
    return 6;
}

size_t
ccnxCodecTlvEncoder_AppendUint32(CCNxCodecTlvEncoder *encoder, uint16_t type, uint32_t value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, 4);
    ccnxCodecNetworkBuffer_PutUint32(encoder->buffer, value);
    return 8;
}

size_t
ccnxCodecTlvEncoder_AppendUint64(CCNxCodecTlvEncoder *encoder, uint16_t type, uint64_t value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, 8);
    ccnxCodecNetworkBuffer_PutUint64(encoder->buffer, value);
    return 12;
}

static unsigned
_ccnxCodecTlvEncoder_ComputeVarIntLength(uint64_t value)
{
    unsigned length = 8;
    if (value <= 0x00000000000000FFULL) {
        length = 1;
    } else if (value <= 0x000000000000FFFFULL) {
        length = 2;
    } else if (value <= 0x0000000000FFFFFFULL) {
        length = 3;
    } else if (value <= 0x00000000FFFFFFFFULL) {
        length = 4;
    } else if (value <= 0x000000FFFFFFFFFFULL) {
        length = 5;
    } else if (value <= 0x0000FFFFFFFFFFFFULL) {
        length = 6;
    } else if (value <= 0x00FFFFFFFFFFFFFFULL) {
        length = 7;
    }

    return length;
}

size_t
ccnxCodecTlvEncoder_AppendVarInt(CCNxCodecTlvEncoder *encoder, uint16_t type, uint64_t value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");

    unsigned length = _ccnxCodecTlvEncoder_ComputeVarIntLength(value);

    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, type);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, length);

    // See case 1007
    bool mustContinue = false;
    for (int byte = 7; byte >= 0; byte--) {
        uint8_t b = (value >> (byte * 8)) & 0xFF;
        if (b != 0 || byte == 0 || mustContinue) {
            ccnxCodecNetworkBuffer_PutUint8(encoder->buffer, b);
            mustContinue = true;
        }
    }

    return length + 4;
}

size_t
ccnxCodecTlvEncoder_Position(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    return ccnxCodecNetworkBuffer_Position(encoder->buffer);
}

size_t
ccnxCodecTlvEncoder_SetPosition(CCNxCodecTlvEncoder *encoder, size_t position)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    assertTrue(position <= ccnxCodecNetworkBuffer_Limit(encoder->buffer),
               "position beyond end of buffer, got %zu maximum %zu",
               position, ccnxCodecNetworkBuffer_Limit(encoder->buffer));
    ccnxCodecNetworkBuffer_SetPosition(encoder->buffer, position);
    return position;
}

void
ccnxCodecTlvEncoder_SetContainerLength(CCNxCodecTlvEncoder *encoder, size_t offset, uint16_t length)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");

    size_t currentPosition = ccnxCodecNetworkBuffer_Position(encoder->buffer);

    // +2 because we skip over the Type field
    ccnxCodecNetworkBuffer_SetPosition(encoder->buffer, offset + 2);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, length);

    ccnxCodecNetworkBuffer_SetPosition(encoder->buffer, currentPosition);
}

void
ccnxCodecTlvEncoder_Finalize(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");

    // set the limit to whatever our current possition is.  That will truncate the
    // packet in case we wrote beyond where we are now.

    ccnxCodecNetworkBuffer_Finalize(encoder->buffer);
}

PARCBuffer *
ccnxCodecTlvEncoder_CreateBuffer(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");

    PARCBuffer *output = ccnxCodecNetworkBuffer_CreateParcBuffer(encoder->buffer);
    return output;
}

CCNxCodecNetworkBufferIoVec *
ccnxCodecTlvEncoder_CreateIoVec(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    return ccnxCodecNetworkBuffer_CreateIoVec(encoder->buffer);
}

size_t
ccnxCodecTlvEncoder_AppendRawArray(CCNxCodecTlvEncoder *encoder, size_t length, uint8_t array[length])
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    ccnxCodecNetworkBuffer_PutArray(encoder->buffer, length, array);
    return length;
}

size_t
ccnxCodecTlvEncoder_PutUint8(CCNxCodecTlvEncoder *encoder, size_t offset, uint8_t value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    size_t position = ccnxCodecNetworkBuffer_Position(encoder->buffer);
    ccnxCodecNetworkBuffer_SetPosition(encoder->buffer, offset);
    ccnxCodecNetworkBuffer_PutUint8(encoder->buffer, value);
    ccnxCodecNetworkBuffer_SetPosition(encoder->buffer, position);
    return 1;
}

size_t
ccnxCodecTlvEncoder_PutUint16(CCNxCodecTlvEncoder *encoder, size_t offset, uint16_t value)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    size_t position = ccnxCodecNetworkBuffer_Position(encoder->buffer);
    ccnxCodecNetworkBuffer_SetPosition(encoder->buffer, offset);
    ccnxCodecNetworkBuffer_PutUint16(encoder->buffer, value);
    ccnxCodecNetworkBuffer_SetPosition(encoder->buffer, position);
    return 2;
}

void
ccnxCodecTlvEncoder_MarkSignatureStart(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    encoder->signatureStart = ccnxCodecNetworkBuffer_Position(encoder->buffer);
    encoder->signatureStartEndSet |= START_SET;
}

void
ccnxCodecTlvEncoder_MarkSignatureEnd(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    encoder->signatureEnd = ccnxCodecNetworkBuffer_Position(encoder->buffer);
    encoder->signatureStartEndSet |= END_SET;
}

PARCSignature *
ccnxCodecTlvEncoder_ComputeSignature(CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    assertTrue(encoder->signatureStartEndSet == BOTH_SET, "Did not set both start and end positions");

    return ccnxCodecNetworkBuffer_ComputeSignature(encoder->buffer, encoder->signatureStart, encoder->signatureEnd, encoder->signer);
}

bool
ccnxCodecTlvEncoder_HasError(const CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    if (encoder->error) {
        return true;
    }

    return false;
}

bool
ccnxCodecTlvEncoder_SetError(CCNxCodecTlvEncoder *encoder, CCNxCodecError *error)
{
    if (ccnxCodecTlvEncoder_HasError(encoder)) {
        return false;
    }

    encoder->error = ccnxCodecError_Acquire(error);
    return true;
}

void
ccnxCodecTlvEncoder_ClearError(CCNxCodecTlvEncoder *encoder)
{
    if (ccnxCodecTlvEncoder_HasError(encoder)) {
        ccnxCodecError_Release(&encoder->error);
    }
}

CCNxCodecError *
ccnxCodecTlvEncoder_GetError(const CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    return encoder->error;
}

void
ccnxCodecTlvEncoder_SetSigner(CCNxCodecTlvEncoder *encoder, PARCSigner *signer)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    if (encoder->signer) {
        parcSigner_Release(&encoder->signer);
    }

    if (signer) {
        encoder->signer = parcSigner_Acquire(signer);
    } else {
        encoder->signer = NULL;
    }
}

PARCSigner *
ccnxCodecTlvEncoder_GetSigner(const CCNxCodecTlvEncoder *encoder)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    return encoder->signer;
}

