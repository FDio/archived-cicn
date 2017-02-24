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

#include "config.h"

#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_WireFormatMessage.h>

void
ccnxWireFormatMessage_AssertValid(const CCNxWireFormatMessage *message)
{
    assertNotNull(message, "Must be a non-null pointer to a CCNxWireFormatMessage.");

    // Check for required fields in the underlying dictionary. Case 1036
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);
    assertNotNull(impl, "WireFormatMessage must have an valid implementation pointer.");
    if (impl->assertValid) {
        impl->assertValid(message);
    }
}

static CCNxWireFormatMessage *
_ccnxWireFormatMessage_CreateWithImpl(const CCNxWireFormatMessageInterface *impl, const PARCBuffer *wireFormatBuffer)
{
    CCNxWireFormatMessage *result = impl->create(wireFormatBuffer);
    return result;
}

CCNxWireFormatMessage *
ccnxWireFormatMessage_Create(const PARCBuffer *wireFormat)
{
    CCNxWireFormatMessage *result = NULL;

    uint8_t version = parcBuffer_GetAtIndex(wireFormat, 0);
    switch (version) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            result = _ccnxWireFormatMessage_CreateWithImpl(&CCNxWireFormatFacadeV1_Implementation, wireFormat);
            break;
        default:
            // no action, will return NULL
            break;
    }

    return result;
}

static CCNxWireFormatMessageInterface *
_getImplForSchema(const CCNxTlvDictionary_SchemaVersion schemaVersion)
{
    CCNxWireFormatMessageInterface *result = NULL;
    switch (schemaVersion) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            result = &CCNxWireFormatFacadeV1_Implementation;
            break;
        default:
            trapIllegalValue(data->fwd_state->nextMessage.version, "Unknown schema version: %d", schemaVersion);
    }
    return result;
}

CCNxWireFormatMessage *
ccnxWireFormatMessage_FromInterestPacketType(const CCNxTlvDictionary_SchemaVersion schemaVersion, const PARCBuffer *wireFormat)
{
    CCNxWireFormatMessageInterface *impl = _getImplForSchema(schemaVersion);
    CCNxWireFormatMessage *result = NULL;

    if (impl->fromInterestPacketType != NULL) {
        result = impl->fromInterestPacketType(wireFormat);
    }

    return result;
}

CCNxWireFormatMessage *
ccnxWireFormatMessage_FromInterestPacketTypeIoVec(const CCNxTlvDictionary_SchemaVersion schemaVersion, const CCNxCodecNetworkBufferIoVec *vec)
{
    CCNxWireFormatMessageInterface *impl = _getImplForSchema(schemaVersion);
    CCNxWireFormatMessage *result = NULL;

    if (impl->fromInterestPacketTypeIoVec != NULL) {
        result = impl->fromInterestPacketTypeIoVec(vec);
    }

    return result;
}

CCNxWireFormatMessage *
ccnxWireFormatMessage_FromContentObjectPacketType(const CCNxTlvDictionary_SchemaVersion schemaVersion, const PARCBuffer *wireFormat)
{
    CCNxWireFormatMessageInterface *impl = _getImplForSchema(schemaVersion);
    CCNxWireFormatMessage *result = NULL;

    if (impl->fromContentObjectPacketType != NULL) {
        result = impl->fromContentObjectPacketType(wireFormat);
    }

    return result;
}

CCNxWireFormatMessage *
ccnxWireFormatMessage_FromControlPacketType(const CCNxTlvDictionary_SchemaVersion schemaVersion, const PARCBuffer *wireFormat)
{
    CCNxWireFormatMessageInterface *impl = _getImplForSchema(schemaVersion);
    CCNxWireFormatMessage *result = NULL;

    if (impl->fromControlPacketType != NULL) {
        result = impl->fromControlPacketType(wireFormat);
    }

    return result;
}

CCNxTlvDictionary *
ccnxWireFormatMessage_GetDictionary(const CCNxWireFormatMessage *message)
{
    return (CCNxTlvDictionary *) message;
}


CCNxCodecNetworkBufferIoVec *
ccnxWireFormatMessage_GetIoVec(const CCNxWireFormatMessage *message)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    CCNxCodecNetworkBufferIoVec *result = NULL;

    if (impl->getIoVec != NULL) {
        result = impl->getIoVec(message);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_GetIoVec");
    }

    return result;
}

bool
ccnxWireFormatMessage_PutIoVec(CCNxWireFormatMessage *message, CCNxCodecNetworkBufferIoVec *vec)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    bool result = false;

    if (impl->putIoVec != NULL) {
        result = impl->putIoVec(message, vec);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_PutIoVec");
    }

    return result;
}

bool
ccnxWireFormatMessage_PutWireFormatBuffer(CCNxWireFormatMessage *message, PARCBuffer *buffer)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    bool result = false;

    if (impl->putWireFormatBuffer != NULL) {
        result = impl->putWireFormatBuffer(message, buffer);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_PutWireFormatBuffer");
    }

    return result;
}

PARCBuffer *
ccnxWireFormatMessage_GetWireFormatBuffer(const CCNxWireFormatMessage *message)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    PARCBuffer *result = NULL;

    if (impl->getWireFormatBuffer != NULL) {
        result = impl->getWireFormatBuffer(message);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_GetWireFormatBuffer");
    }

    return result;
}

void
ccnxWireFormatMessage_WriteToFile(const CCNxWireFormatMessage *message, const char *filename)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    if (impl->writeToFile != NULL) {
        impl->writeToFile(message, filename);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_WriteToFile");
    }
}

bool
ccnxWireFormatMessage_SetProtectedRegionStart(CCNxWireFormatMessage *message, size_t startPosition)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    bool result = false;

    if (impl->setProtectedRegionStart != NULL) {
        result = impl->setProtectedRegionStart(message, startPosition);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_SetProtectedRegionStart");
    }

    return result;
}

bool
ccnxWireFormatMessage_SetProtectedRegionLength(CCNxWireFormatMessage *message, size_t length)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    bool result = false;

    if (impl->setProtectedRegionLength != NULL) {
        result = impl->setProtectedRegionLength(message, length);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_SetProtectedRegionLength");
    }

    return result;
}


PARCCryptoHash *
ccnxWireFormatMessage_HashProtectedRegion(const CCNxWireFormatMessage *message, PARCCryptoHasher *hasher)
{
    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    PARCCryptoHash *result = NULL;

    if (impl->hashProtectedRegion != NULL) {
        result = impl->hashProtectedRegion(message, hasher);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_HashProtectedRegion");
    }

    return result;
}

PARCCryptoHash *
ccnxWireFormatMessage_CreateContentObjectHash(CCNxWireFormatMessage *message)
{
    ccnxWireFormatMessage_AssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    PARCCryptoHash *result = NULL;

    if (impl->computeContentObjectHash != NULL) {
        result = impl->computeContentObjectHash(message);
    } else {
        trapNotImplemented("ccnxWireFormatMessage_ComputeContentObjectHash");
    }

    return result;
}

CCNxWireFormatMessage *
ccnxWireFormatMessage_Acquire(const CCNxWireFormatMessage *message)
{
    return ccnxTlvDictionary_Acquire(message);
}

void
ccnxWireFormatMessage_Release(CCNxWireFormatMessage **message)
{
    ccnxTlvDictionary_Release(message);
}

bool
ccnxWireFormatMessage_SetHopLimit(CCNxWireFormatMessage *message, uint32_t hoplimit)
{
    bool result = false;

    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    if (impl != NULL) {
        result = impl->setHopLimit(message, hoplimit);
    }

    return result;
}

bool
ccnxWireFormatMessage_ConvertInterestToInterestReturn(CCNxWireFormatMessage *message, uint8_t returnCode)
{
    bool result = false;

    ccnxWireFormatMessage_OptionalAssertValid(message);
    CCNxWireFormatMessageInterface *impl = ccnxWireFormatMessageInterface_GetInterface(message);

    if (impl != NULL) {
        result = impl->convertInterestToInterestReturn(message, returnCode);
    }
    return result;
}

