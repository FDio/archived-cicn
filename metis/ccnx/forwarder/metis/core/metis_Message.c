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
 * The implementation of metisMessage_Slice() copies data, it needs to do this by reference.
 *
 */
#include <config.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/core/metis_StreamBuffer.h>
#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

#include <ccnx/forwarder/metis/core/metis_Wldr.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Hash.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_EventBuffer.h>

struct metis_message {
    MetisLogger *logger;

    MetisTicks receiveTime;
    unsigned ingressConnectionId;

    PARCEventBuffer *messageBytes;
    uint8_t *messageHead;

    unsigned refcount;

    struct tlv_skeleton skeleton;

    bool hasKeyId;
    uint32_t keyIdHash;
    bool isKeyIdVerified;

    bool hasContentObjectHash;
    // may be null, even if hasContentObjectHash true due to lazy calculation
    PARCBuffer *contentObjectHash;

    PARCBuffer *certificate;

    PARCBuffer *publicKey;

    bool hasInterestLifetime;
    uint64_t interestLifetimeTicks;

    bool hasExpiryTimeTicks;
    uint64_t expiryTimeTicks;

    bool hasRecommendedCacheTimeTicks;
    uint64_t recommendedCacheTimeTicks;

    bool hasName;
    MetisTlvName *name;

    bool hasFragmentPayload;

    MetisMessagePacketType packetType;


    bool hasPathLabel;
    uint64_t pathLabel;

    bool hasWldr;
    //the following fields are valid only if hasWldr is true
    uint8_t wldrType;
    uint16_t wldrLbl;           //if wldrType == WLDR_LBL this indicates the message label
                                //if wldrType == WLDR_NOTIFICATION this indicates the expected message label
    uint16_t wldrLastReceived;  //this field is valid only when wldrType == WLDR_NOTIFICATION. In this case,
                                //all the messages between wldrLbl (included) and wldrLastReceived (excluded)
                                //are considered lost
};

static void
_setupWldr(MetisMessage *message)
{
    uint8_t wldr_header = 0;
    parcEventBuffer_copyOut(message->messageBytes, &wldr_header, 1);
    if (wldr_header == WLDR_HEADER) {
        message->hasWldr = true;
        parcEventBuffer_Read(message->messageBytes, NULL, 1);
        parcEventBuffer_Read(message->messageBytes, &(message->wldrType), 1);
        if (message->wldrType == WLDR_LBL) {
            parcEventBuffer_Read(message->messageBytes, &(message->wldrLbl), 2);
            parcEventBuffer_Read(message->messageBytes, NULL, 2);
        } else if (message->wldrType == WLDR_NOTIFICATION) {
            parcEventBuffer_Read(message->messageBytes, &(message->wldrLbl), 2);
            parcEventBuffer_Read(message->messageBytes, &(message->wldrLastReceived), 2);
        } else  {
            //find a better way to exit (look into longBow)
            printf("Error, Unknown WLDR Type\n");
            exit(0);
        }
        message->messageHead = parcEventBuffer_Pullup(message->messageBytes, -1);
    } else  {
        message->hasWldr = false;
    }
}

static void
_setupName(MetisMessage *message)
{
    MetisTlvExtent extent = metisTlvSkeleton_GetName(&message->skeleton);
    if (extent.offset > 0) {
        message->hasName = true;
        message->name = metisTlvName_Create(&message->messageHead[extent.offset], extent.length);
    } else {
        message->hasName = false;
        message->name = NULL;
    }
}

static void
_setupValidationParams(MetisMessage *message)
{
    MetisTlvExtent extent = metisTlvSkeleton_GetKeyId(&message->skeleton);
    if (extent.offset > 0) {
        message->hasKeyId = true;
        message->keyIdHash = parcHash32_Data(&message->messageHead[extent.offset], extent.length);
    } else {
        message->hasKeyId = false;
        message->keyIdHash = 0;
    }
    message->isKeyIdVerified = false;

    extent = metisTlvSkeleton_GetCertificate(&message->skeleton);
    if (extent.offset > 0) {
        message->certificate = parcBuffer_Flip(parcBuffer_CreateFromArray(&message->messageHead[extent.offset], extent.length));
    } else {
        message->certificate = NULL;
    }

    extent = metisTlvSkeleton_GetPublicKey(&message->skeleton);
    if (extent.offset > 0) {
        message->publicKey = parcBuffer_Flip(parcBuffer_CreateFromArray(&message->messageHead[extent.offset], extent.length));
    } else {
        message->publicKey = NULL;
    }
}

static void
_setupContentObjectHash(MetisMessage *message)
{
    if (metisTlvSkeleton_IsPacketTypeInterest(&message->skeleton)) {
        MetisTlvExtent extent = metisTlvSkeleton_GetObjectHash(&message->skeleton);
        // pre-compute it for an interest
        if (extent.offset > 0) {
            message->hasContentObjectHash = true;
            message->contentObjectHash =
                parcBuffer_Flip(parcBuffer_CreateFromArray(&message->messageHead[extent.offset], extent.length));
        } else {
            message->hasContentObjectHash = false;
        }
    } else if (metisTlvSkeleton_IsPacketTypeContentObject(&message->skeleton)) {
        // we will compute this on demand
        message->hasContentObjectHash = true;
        message->contentObjectHash = NULL;
    } else {
        message->hasContentObjectHash = false;
    }
}

static void
_setupInterestLifetime(MetisMessage *message)
{
    MetisTlvExtent extent = metisTlvSkeleton_GetInterestLifetime(&message->skeleton);
    message->hasInterestLifetime = false;
    if (metisTlvSkeleton_IsPacketTypeInterest(&message->skeleton) && extent.offset > 0) {
        message->hasInterestLifetime = true;
        uint64_t lifetimeMilliseconds;
        metisTlv_ExtentToVarInt(message->messageHead, &extent, &lifetimeMilliseconds);
        message->interestLifetimeTicks = metisForwarder_NanosToTicks(lifetimeMilliseconds * 1000000);
    }
}

static void
_setupPathLabel(MetisMessage *message)
{
    MetisTlvExtent extent = metisTlvSkeleton_GetPathLabel(&message->skeleton);
    message->hasPathLabel = false;
    if (metisTlvSkeleton_IsPacketTypeContentObject(&message->skeleton) && extent.offset > 0) {
        message->hasPathLabel = true;
        uint64_t pathLabel;
        metisTlv_ExtentToVarInt(message->messageHead, &extent, &pathLabel);
        message->pathLabel = pathLabel;
    }
}


static void
_setupFragmentPayload(MetisMessage *message)
{
    MetisTlvExtent extent = metisTlvSkeleton_GetFragmentPayload(&message->skeleton);
    if (extent.offset > 0) {
        message->hasFragmentPayload = true;
    } else {
        message->hasFragmentPayload = true;
    }
}

static void
_setupExpiryTime(MetisMessage *message)
{
    MetisTlvExtent expiryTimeExtent = metisTlvSkeleton_GetExpiryTime(&message->skeleton);

    message->hasExpiryTimeTicks = false;

    if (metisTlvSkeleton_IsPacketTypeContentObject(&message->skeleton)) {
        if (!metisTlvExtent_Equals(&expiryTimeExtent, &metisTlvExtent_NotFound)) {
            uint64_t expiryTimeUTC = 0;
            if (metisTlv_ExtentToVarInt(metisTlvSkeleton_GetPacket(&message->skeleton), &expiryTimeExtent, &expiryTimeUTC)) {
                message->hasExpiryTimeTicks = true;

                // Convert it to ticks that we can use for expiration checking.
                uint64_t metisWallClockTime = parcClock_GetTime(parcClock_Wallclock());
                uint64_t currentTimeInTicks = parcClock_GetTime(parcClock_Monotonic());

                message->expiryTimeTicks = expiryTimeUTC - metisWallClockTime + currentTimeInTicks;
            }
        }
    }
}

static void
_setupRecommendedCacheTime(MetisMessage *message)
{
    MetisTlvExtent cacheTimeExtent = metisTlvSkeleton_GetCacheTimeHeader(&message->skeleton);

    message->hasRecommendedCacheTimeTicks = false;

    if (metisTlvSkeleton_IsPacketTypeContentObject(&message->skeleton)) {
        if (!metisTlvExtent_Equals(&cacheTimeExtent, &metisTlvExtent_NotFound)) {
            uint64_t recommendedCacheTime = 0;
            if (metisTlv_ExtentToVarInt(metisTlvSkeleton_GetPacket(&message->skeleton), &cacheTimeExtent, &recommendedCacheTime)) {
                message->hasRecommendedCacheTimeTicks = true;

                // Convert it to ticks that we can use for expiration checking.
                uint64_t metisWallClockTime = parcClock_GetTime(parcClock_Wallclock());
                uint64_t currentTimeInTicks = parcClock_GetTime(parcClock_Monotonic());

                message->recommendedCacheTimeTicks = recommendedCacheTime - metisWallClockTime + currentTimeInTicks;
            }
        }
    }
}

/**
 * Parse the TLV skeleton and setup message pointers
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] message An allocated messae with the message->messagaeHead pointer setup.
 *
 * @retval false Error parsing message
 * @retval true Good parse
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_setupInternalData(MetisMessage *message)
{
    // -1 means linearize the whole buffer
    message->messageHead = parcEventBuffer_Pullup(message->messageBytes, -1);
    message->packetType = MetisMessagePacketType_Unknown;

    _setupWldr(message);
    if (message->hasWldr == true && message->wldrType == WLDR_NOTIFICATION) {
        //this is a WLDR notification message, all the other fields are meaningless because the
        //packet will be dropped immmedialtly after the retransmissions. For this reason we can
        //immediatly retrun and avoid the parsing of the message
        return true;
    }
    //now the WLDR header is removed from the packet and the parsing shuould work as usual

    bool goodSkeleton = metisTlvSkeleton_Parse(&message->skeleton, message->messageHead, message->logger);

    if (goodSkeleton) {
        _setupName(message);
        _setupValidationParams(message);
        _setupContentObjectHash(message);
        _setupInterestLifetime(message);
        _setupPathLabel(message);
        _setupFragmentPayload(message);
        _setupExpiryTime(message);
        _setupRecommendedCacheTime(message);

        // set the packet type
        bool requiresName = false;
        if (metisTlvSkeleton_IsPacketTypeInterest(&message->skeleton)) {
            message->packetType = MetisMessagePacketType_Interest;
            requiresName = true;
        } else if (metisTlvSkeleton_IsPacketTypeContentObject(&message->skeleton)) {
            message->packetType = MetisMessagePacketType_ContentObject;
            requiresName = true;
        } else if (metisTlvSkeleton_IsPacketTypeHopByHopFragment(&message->skeleton)) {
            message->packetType = MetisMessagePacketType_HopByHopFrag;
        } else if (metisTlvSkeleton_IsPacketTypeControl(&message->skeleton)) {
            message->packetType = MetisMessagePacketType_Control;
        } else if (metisTlvSkeleton_IsPacketTypeInterestReturn(&message->skeleton)) {
            message->packetType = MetisMessagePacketType_InterestReturn;
        }

        if (requiresName && !metisMessage_HasName(message)) {
            goodSkeleton = false;
        }
    }

    return goodSkeleton;
}

MetisMessage *
metisMessage_Acquire(const MetisMessage *message)
{
    MetisMessage *copy = (MetisMessage *) message;
    copy->refcount++;
    return copy;
}

MetisMessage *
metisMessage_CreateFromParcBuffer(PARCBuffer *buffer, unsigned ingressConnectionId, MetisTicks receiveTime, MetisLogger *logger)
{
    MetisMessage *message = parcMemory_AllocateAndClear(sizeof(MetisMessage));
    assertNotNull(message, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMessage));
    message->receiveTime = receiveTime;
    message->ingressConnectionId = ingressConnectionId;
    message->messageBytes = parcEventBuffer_Create();
    message->refcount = 1;
    message->logger = metisLogger_Acquire(logger);

    // this copies the data
    int failure = parcEventBuffer_Append(message->messageBytes, parcBuffer_Overlay(buffer, 0), parcBuffer_Remaining(buffer));
    assertFalse(failure, "Got failure copying data into buffer: (%d) %s", errno, strerror(errno));

    bool goodSkeleton = _setupInternalData(message);
    if (goodSkeleton) {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                            "Message %p created ingress %u",
                            (void *) message, ingressConnectionId);
        }
    } else {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning, __func__,
                            "Error setting up skeleton for buffer %p ingress %u",
                            (void *) buffer, ingressConnectionId);
        }

        metisMessage_Release(&message);
    }
    return message;
}

MetisMessage *
metisMessage_CreateFromArray(const uint8_t *data, size_t dataLength, unsigned ingressConnectionId, MetisTicks receiveTime, MetisLogger *logger)
{
    MetisMessage *message = parcMemory_AllocateAndClear(sizeof(MetisMessage));
    assertNotNull(message, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMessage));
    message->receiveTime = receiveTime;
    message->ingressConnectionId = ingressConnectionId;
    message->messageBytes = parcEventBuffer_Create();
    message->refcount = 1;
    message->logger = metisLogger_Acquire(logger);

    // this copies the data
    int failure = parcEventBuffer_Append(message->messageBytes, (void *) data, dataLength);
    assertFalse(failure, "Got failure copying data into PARCEventBuffer: (%d) %s", errno, strerror(errno));

    bool goodSkeleton = _setupInternalData(message);
    if (goodSkeleton) {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                            "Message %p created ingress %u",
                            (void *) message, ingressConnectionId);
        }
    } else {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning, __func__,
                            "Error setting up skeleton for array %p ingress %u",
                            (void *) data, ingressConnectionId);
        }

        metisMessage_Release(&message);
    }

    return message;
}

MetisMessage *
metisMessage_ReadFromBuffer(unsigned ingressConnectionId, MetisTicks receiveTime, PARCEventBuffer *input, size_t bytesToRead, MetisLogger *logger)
{
    MetisMessage *message = parcMemory_AllocateAndClear(sizeof(MetisMessage));
    assertNotNull(message, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMessage));
    message->receiveTime = receiveTime;
    message->ingressConnectionId = ingressConnectionId;
    message->messageBytes = parcEventBuffer_Create();
    message->refcount = 1;
    message->logger = metisLogger_Acquire(logger);

    // dequeue into packet buffer.  This is a near-zero-copy operation from
    // one buffer to another.  It only copies if the message falls across iovec
    // boundaries.
    int bytesRead = parcEventBuffer_ReadIntoBuffer(input, message->messageBytes, bytesToRead);
    assertTrue(bytesRead == bytesToRead, "Partial read, expected %zu got %d", bytesToRead, bytesRead);

    bool goodSkeleton = _setupInternalData(message);
    if (goodSkeleton) {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                            "Message %p created ingress %u",
                            (void *) message, ingressConnectionId);
        }
    } else {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning, __func__,
                            "Error setting up skeleton for buffer %p ingress %u",
                            (void *) input, ingressConnectionId);
        }

        metisMessage_Release(&message);
    }
    return message;
}

MetisMessage *
metisMessage_CreateFromBuffer(unsigned ingressConnectionId, MetisTicks receiveTime, PARCEventBuffer *input, MetisLogger *logger)
{
    assertNotNull(input, "Parameter input must be non-null");
    MetisMessage *message = parcMemory_AllocateAndClear(sizeof(MetisMessage));
    assertNotNull(message, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMessage));
    message->receiveTime = receiveTime;
    message->ingressConnectionId = ingressConnectionId;
    message->messageBytes = input;
    message->refcount = 1;
    message->logger = metisLogger_Acquire(logger);

    bool goodSkeleton = _setupInternalData(message);
    if (goodSkeleton) {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                            "Message %p created ingress %u",
                            (void *) message, ingressConnectionId);
        }
    } else {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning, __func__,
                            "Error setting up skeleton for buffer %p ingress %u",
                            (void *) input, ingressConnectionId);
        }

        metisMessage_Release(&message);
    }
    return message;
}

void
metisMessage_Release(MetisMessage **messagePtr)
{
    assertNotNull(messagePtr, "Parameter must be non-null double pointer");
    assertNotNull(*messagePtr, "Parameter must dereference to non-null pointer");

    MetisMessage *message = *messagePtr;
    assertTrue(message->refcount > 0, "Invalid state: metisMessage_Release called on message with 0 references %p", (void *) message);

    message->refcount--;
    if (message->refcount == 0) {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                            "Message %p destroyed",
                            (void *) message);
        }

        if (message->contentObjectHash) {
            parcBuffer_Release(&message->contentObjectHash);
        }

        if (message->name) {
            metisTlvName_Release(&message->name);
        }

        if (message->publicKey) {
            parcBuffer_Release(&message->publicKey);
        }

        if (message->certificate) {
            parcBuffer_Release(&message->certificate);
        }

        metisLogger_Release(&message->logger);
        parcEventBuffer_Destroy(&(message->messageBytes));
        parcMemory_Deallocate((void **) &message);
    }
    *messagePtr = NULL;
}

bool
metisMessage_Write(PARCEventQueue *parcEventQueue, const MetisMessage *message)
{
    assertNotNull(message, "Message parameter must be non-null");
    assertNotNull(parcEventQueue, "Buffer parameter must be non-null");

    return parcEventQueue_Write(parcEventQueue, message->messageHead, parcEventBuffer_GetLength(message->messageBytes));
}

bool
metisMessage_Append(PARCEventBuffer *writeBuffer, const MetisMessage *message)
{
    assertNotNull(message, "Message parameter must be non-null");
    assertNotNull(writeBuffer, "Buffer parameter must be non-null");

    if (message->messageBytes == NULL) {
        //this is an error that we need to handle (just dropping the packet?)
        //right now we log the event, drop the packet and return false
        //should I release the message as well?
        if (message->logger != NULL && metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                            "Message %p has a null message buffer inside!",
                            (void *) message);
        }
        return false;
    }

    return parcEventBuffer_Append(writeBuffer, message->messageHead, metisMessage_Length(message));
}

size_t
metisMessage_Length(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    return parcEventBuffer_GetLength(message->messageBytes);
}

bool
metisMessage_HasWldr(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    return message->hasWldr;
}

unsigned
metisMessage_GetWldrType(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    assertTrue(message->hasWldr == true, "This message does not contains a WLDR header");
    return message->wldrType;
}

unsigned
metisMessage_GetWldrLabel(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    assertTrue(message->hasWldr == true, "This message does not contains a WLDR header");
    return message->wldrLbl;
}

unsigned
metisMessage_GetWldrLastReceived(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    assertTrue(message->hasWldr == true, "This message does not contains a WLDR header");
    return message->wldrLastReceived;
}

static void
_removeOldWldrHeader(MetisMessage *message)
{
    uint8_t wldr_header = 0;
    parcEventBuffer_copyOut(message->messageBytes, &wldr_header, 1);
    if (wldr_header == WLDR_HEADER) {
        parcEventBuffer_Read(message->messageBytes, NULL, WLDR_HEADER_SIZE);
    }
}

void
metisMessage_SetWldrLabel(MetisMessage *message, uint16_t label)
{
    assertNotNull(message, "Parameter must be non-null");
    _removeOldWldrHeader(message);
    message->hasWldr = true;
    message->wldrType = WLDR_LBL;
    message->wldrLbl = label;
    uint8_t wldr_header[6];
    wldr_header[0] = WLDR_HEADER;
    wldr_header[1] = WLDR_LBL;
    wldr_header[2] = label & 0xFF;
    wldr_header[3] = (label >> 8UL) & 0xFF;
    wldr_header[4] = 0;
    wldr_header[5] = 0;
    parcEventBuffer_Prepend(message->messageBytes, &wldr_header, sizeof(wldr_header));
    uint8_t *newMessage = parcEventBuffer_Pullup(message->messageBytes, -1);
    bool goodSkeleton = metisTlvSkeleton_Parse(&message->skeleton, newMessage + WLDR_HEADER_SIZE, message->logger);
    if (goodSkeleton) {
        message->messageHead = newMessage;
    } else  {
        trapNotImplemented("[metis_Message.c] message parsing after WLDR header insertion failed");
    }
}

void
metisMessage_SetWldrNotification(MetisMessage *message, uint16_t expected, uint16_t lastReceived)
{
    assertNotNull(message, "Parameter must be non-null");
    _removeOldWldrHeader(message);
    message->hasWldr = true;
    message->wldrType = WLDR_NOTIFICATION;
    message->wldrLbl = expected;
    message->wldrLastReceived = lastReceived;
    uint8_t wldr_header[6];
    wldr_header[0] = WLDR_HEADER;
    wldr_header[1] = WLDR_NOTIFICATION;
    wldr_header[2] = expected & 0xFF;
    wldr_header[3] = (expected >> 8U) & 0xFF;
    wldr_header[4] = lastReceived & 0xFF;
    wldr_header[5] = (lastReceived >> 8U) & 0xFF;
    parcEventBuffer_Prepend(message->messageBytes, &wldr_header, sizeof(wldr_header));
    message->messageHead = parcEventBuffer_Pullup(message->messageBytes, -1);
}

unsigned
metisMessage_GetIngressConnectionId(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    return message->ingressConnectionId;
}

MetisTicks
metisMessage_GetReceiveTime(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    return message->receiveTime;
}

bool
metisMessage_HasHopLimit(const MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    MetisTlvExtent extent = metisTlvSkeleton_GetHopLimit(&message->skeleton);

    if (extent.offset > 0) {
        return true;
    }
    return false;
}

uint8_t
metisMessage_GetHopLimit(const MetisMessage *message)
{
    bool hasHopLimit = metisMessage_HasHopLimit(message);
    assertTrue(hasHopLimit, "Message does not have a HopLimit field");

    MetisTlvExtent extent = metisTlvSkeleton_GetHopLimit(&message->skeleton);
    uint8_t hopLimit = message->messageHead[extent.offset];
    return hopLimit;
}

void
metisMessage_SetHopLimit(MetisMessage *message, uint8_t hoplimit)
{
    assertNotNull(message, "Parameter must be non-null");
    metisTlvSkeleton_UpdateHopLimit(&message->skeleton, hoplimit);
}

void
metisMessage_UpdatePathLabel(MetisMessage *message, uint8_t outFace)
{
    assertNotNull(message, "Parameter must be non-null");
    metisTlvSkeleton_UpdatePathLabel(&message->skeleton, outFace);
}
void
metisMessage_ResetPathLabel(MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    metisTlvSkeleton_ResetPathLabel(&message->skeleton);
}
int
metisMessage_GetPathLabel(MetisMessage *message)
{
    assertNotNull(message, "Parameter must be non-null");
    return metisTlvSkeleton_GetPathLabelValue(&message->skeleton);
}

void
metisMessage_SetPathLabel(MetisMessage *message, uint8_t pathLabel)
{
    assertNotNull(message, "Parameter must be non-null");
    return metisTlvSkeleton_SetPathLabelValue(&message->skeleton, pathLabel);
}


MetisMessagePacketType
metisMessage_GetType(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->packetType;
}

MetisTlvName *
metisMessage_GetName(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->name;
}

bool
metisMessage_GetKeyIdHash(const MetisMessage *message, uint32_t *hashOutput)
{
    assertNotNull(message, "Parameter message must be non-null");
    if (message->hasKeyId) {
        *hashOutput = message->keyIdHash;
        return true;
    }
    return false;
}

PARCBuffer *
metisMessage_GetCertificate(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->certificate;
}

PARCBuffer *
metisMessage_GetPublicKey(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->publicKey;
}

bool
metisMessage_KeyIdEquals(const MetisMessage *a, const MetisMessage *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");

    if (a->hasKeyId && b->hasKeyId) {
        MetisTlvExtent ae = metisTlvSkeleton_GetKeyId(&a->skeleton);
        MetisTlvExtent be = metisTlvSkeleton_GetKeyId(&b->skeleton);

        if (ae.length == be.length) {
            return memcmp(&a->messageHead[ae.offset], &b->messageHead[be.offset], ae.length) == 0;
        }
    }
    return false;
}

bool
metisMessage_ObjectHashEquals(MetisMessage *a, MetisMessage *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");

    if (a->hasContentObjectHash && b->hasContentObjectHash) {
        if (a->contentObjectHash == NULL) {
            PARCCryptoHash *hash = metisTlvSkeleton_ComputeContentObjectHash(&a->skeleton);
            a->contentObjectHash = parcBuffer_Acquire(parcCryptoHash_GetDigest(hash));
            parcCryptoHash_Release(&hash);
        }

        if (b->contentObjectHash == NULL) {
            PARCCryptoHash *hash = metisTlvSkeleton_ComputeContentObjectHash(&b->skeleton);
            b->contentObjectHash = parcBuffer_Acquire(parcCryptoHash_GetDigest(hash));
            parcCryptoHash_Release(&hash);
        }

        return parcBuffer_Equals(a->contentObjectHash, b->contentObjectHash);
    }

    return false;
}

bool
metisMessage_GetContentObjectHashHash(MetisMessage *message, uint32_t *hashOutput)
{
    assertNotNull(message, "Parameter message must be non-null");
    assertNotNull(hashOutput, "Parameter hashOutput must be non-null");

    if (message->hasContentObjectHash) {
        if (message->contentObjectHash == NULL) {
            PARCCryptoHash *hash = metisTlvSkeleton_ComputeContentObjectHash(&message->skeleton);
            message->contentObjectHash = parcBuffer_Acquire(parcCryptoHash_GetDigest(hash));
            parcCryptoHash_Release(&hash);
        }

        *hashOutput = (uint32_t) parcBuffer_HashCode(message->contentObjectHash);
        return true;
    }
    return false;
}

bool
metisMessage_HasPublicKey(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return (message->publicKey != NULL);
}

bool
metisMessage_HasCertificate(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return (message->certificate != NULL);
}

bool
metisMessage_HasName(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->hasName;
}

bool
metisMessage_HasKeyId(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->hasKeyId;
}

bool
metisMessage_IsKeyIdVerified(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->isKeyIdVerified;
}

bool
metisMessage_HasContentObjectHash(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->hasContentObjectHash;
}

CCNxControl *
metisMessage_CreateControlMessage(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    assertTrue(metisMessage_GetType(message) == MetisMessagePacketType_Control,
               "Wrong type of message, expected %02X got %02X",
               MetisMessagePacketType_Control,
               metisMessage_GetType(message));

    MetisTlvExtent extent = metisTlvSkeleton_GetCPI(&message->skeleton);
    assertTrue(extent.offset > 0, "Message does not have a CPI TLV field!");

    PARCJSON *json = parcJSON_ParseString((char *) &message->messageHead[ extent.offset ]);
    CCNxControl *control = ccnxControl_CreateCPIRequest(json);
    parcJSON_Release(&json);
    return control;
}

bool
metisMessage_HasInterestLifetime(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->hasInterestLifetime;
}

uint64_t
metisMessage_GetInterestLifetimeTicks(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->interestLifetimeTicks;
}

bool
metisMessage_HasFragmentPayload(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->hasFragmentPayload;
}

size_t
metisMessage_AppendFragmentPayload(const MetisMessage *message, PARCEventBuffer *buffer)
{
    size_t bytesAppended = 0;
    if (message->hasFragmentPayload) {
        MetisTlvExtent extent = metisTlvSkeleton_GetFragmentPayload(&message->skeleton);
        parcEventBuffer_Append(buffer, message->messageHead + extent.offset, extent.length);
        bytesAppended = extent.length;
    }
    return bytesAppended;
}

const uint8_t *
metisMessage_FixedHeader(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->messageHead;
}

MetisMessage *
metisMessage_Slice(const MetisMessage *original, size_t offset, size_t length, size_t headerLength, const uint8_t header[headerLength])
{
    assertNotNull(original, "Parameter original must be non-null");
    assertTrue(length > 0, "Parameter length must be positive");
    assertTrue(offset + length <= parcEventBuffer_GetLength(original->messageBytes),
               "Slice extends beyond end, maximum %zu got %zu",
               parcEventBuffer_GetLength(original->messageBytes),
               offset + length);

    MetisMessage *message = parcMemory_AllocateAndClear(sizeof(MetisMessage));
    assertNotNull(message, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMessage));
    message->receiveTime = original->receiveTime;
    message->ingressConnectionId = original->ingressConnectionId;
    message->messageBytes = parcEventBuffer_Create();
    message->refcount = 1;
    message->logger = metisLogger_Acquire(original->logger);

    if (headerLength > 0) {
        assertNotNull(header, "Cannot have a positive headerLength and NULL header");

        // this copies the data
        int failure = parcEventBuffer_Append(message->messageBytes, (void *) header, headerLength);
        assertFalse(failure, "Got failure adding header data into PARCEventBuffer: (%d) %s", errno, strerror(errno));
    }

    // this copies the data
    int failure = parcEventBuffer_Append(message->messageBytes, (uint8_t *) original->messageHead + offset, length);
    assertFalse(failure, "Got failure adding slice data into PARCEventBuffer: (%d) %s", errno, strerror(errno));

    bool goodSkeleton = _setupInternalData(message);
    if (goodSkeleton) {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                            "Message %p created slice(%p, %zu, %zu)",
                            (void *) message, (void *) original, offset, length);
        }
    } else {
        if (metisLogger_IsLoggable(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning)) {
            metisLogger_Log(message->logger, MetisLoggerFacility_Message, PARCLogLevel_Warning, __func__,
                            "Error setting up skeleton for original %p and header %p",
                            (void *) original, (void *) header);
        }

        metisMessage_Release(&message);
    }

    return message;
}

bool
metisMessage_HasRecommendedCacheTime(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->hasRecommendedCacheTimeTicks;
}

uint64_t
metisMessage_GetRecommendedCacheTimeTicks(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    assertTrue(message->hasRecommendedCacheTimeTicks, "MetisMessage does not have a RecommendedCacheTime. Call metisMessage_HasRecommendedCacheTime() first.");
    return message->recommendedCacheTimeTicks;
}

void
metisMessage_SetRecommendedCacheTimeTicks(MetisMessage *message, uint64_t recommendedCacheTimeTicks)
{
    assertNotNull(message, "Parameter message must be non-null");
    message->recommendedCacheTimeTicks = recommendedCacheTimeTicks;
    message->hasRecommendedCacheTimeTicks = true;
}

bool
metisMessage_HasExpiryTime(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    return message->hasExpiryTimeTicks;
}

uint64_t
metisMessage_GetExpiryTimeTicks(const MetisMessage *message)
{
    assertNotNull(message, "Parameter message must be non-null");
    assertTrue(message->hasExpiryTimeTicks, "MetisMessage does not have an ExpiryTime. Call metisMessage_HasExpiryTime() first.");
    return message->expiryTimeTicks;
}

void
metisMessage_SetExpiryTimeTicks(MetisMessage *message, uint64_t expiryTimeTicks)
{
    assertNotNull(message, "Parameter message must be non-null");
    message->expiryTimeTicks = expiryTimeTicks;
    message->hasExpiryTimeTicks = true;
}
