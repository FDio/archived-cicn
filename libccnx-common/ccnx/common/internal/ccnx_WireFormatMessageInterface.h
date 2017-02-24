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
 * @brief The definition of the interface used to call into a CCNxWireFormatFacade implementation.
 *
 */

#ifndef CCNx_Common_ccnx_internal_WireFormatMessageInterface_h
#define CCNx_Common_ccnx_internal_WireFormatMessageInterface_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

typedef struct ccnx_wireformatmessage_interface {
    char              *description;      // A human-readable label for this implementation

    /** @see ccnxWireFormatMessage_Create */
    CCNxTlvDictionary *(*create)(const PARCBuffer * wireFormat);

    /** @see ccnxWireFormatMessage_FromInterestPacketType */
    CCNxTlvDictionary *(*fromInterestPacketType)(const PARCBuffer * wireFormat);

    /** @see ccnxWireFormatMessage_FromInterestPacketTypeIoVec */
    CCNxTlvDictionary *(*fromInterestPacketTypeIoVec)(const CCNxCodecNetworkBufferIoVec * vec);

    /** @see ccnxWireFormatMessage_FromContentObjectPacketType */
    CCNxTlvDictionary *(*fromContentObjectPacketType)(const PARCBuffer * wireFormat);

    /** @see ccnxWireFormatMessage_FromControlPacketType */
    CCNxTlvDictionary *(*fromControlPacketType)(const PARCBuffer * wireFormat);

    /** @see ccnxWireFormatMessage_GetWireFormatBuffer */
    PARCBuffer        *(*getWireFormatBuffer)(const CCNxTlvDictionary * dictionary);

    /** @see ccnxWireFormatMessage_GetIoVec */
    CCNxCodecNetworkBufferIoVec *(*getIoVec)(const CCNxTlvDictionary * dictionary);

    /** @see ccnxWireFormatMessage_PutWireFormatBuffer */
    bool (*putWireFormatBuffer)(CCNxTlvDictionary *dictionary, PARCBuffer *buffer);

    /** @see ccnxWireFormatMessage_PutIoVec */
    bool (*putIoVec)(CCNxTlvDictionary *dictionary, CCNxCodecNetworkBufferIoVec *vec);

    /** @see ccnxWireFormatMessage_WriteToFile */
    void (*writeToFile)(const CCNxTlvDictionary *dictionary, const char *filename);

    /** @see ccnxWireFormatMessage_SetProtectedRegionStart */
    bool (*setProtectedRegionStart)(CCNxTlvDictionary *dictionary, size_t startPosition);

    /** @see ccnxWireFormatMessage_SetProtectedRegionLength */
    bool (*setProtectedRegionLength)(CCNxTlvDictionary *dictionary, size_t length);

    /** @see ccnxWireFormatMessage_SetContentObjectHashRegionStart */
    bool (*setContentObjectHashRegionStart)(CCNxTlvDictionary *dictionary, size_t startPosition);

    /** @see ccnxWireFormatMessage_SetContentObjectHashRegionLength */
    bool (*setContentObjectHashRegionLength)(CCNxTlvDictionary *dictionary, size_t length);

    /** @see ccnxWireFormatMessage_HashProtectedRegion */
    PARCCryptoHash    *(*hashProtectedRegion)(const CCNxTlvDictionary * dictionary, PARCCryptoHasher * hasher);

    /** @see ccnxWireFormatMessage_SetHopLimit */
    bool (*setHopLimit)(CCNxTlvDictionary *dictionary, uint32_t hopLimit);

    /** @see ccnxWireFormatMessage_AssertValid*/
    void (*assertValid)(const CCNxTlvDictionary *dictionary);

    /** @see ccnxWireFormatMessage_CreateContentObjectHash */
    PARCCryptoHash    *(*computeContentObjectHash)(CCNxTlvDictionary * dictionary);

    /** @see ccnxWireFormatMessage_ConvertInterestToInterestReturn */
    bool (*convertInterestToInterestReturn)(CCNxTlvDictionary *dictionary, uint8_t returnCode);
} CCNxWireFormatMessageInterface;

/**
 * The SchemaV0 WireFormatMessage implementaton
 */
extern CCNxWireFormatMessageInterface CCNxWireFormatFacadeV0_Implementation;

/**
 * The SchemaV1 WireFormatMessage implementaton
 */
extern CCNxWireFormatMessageInterface CCNxWireFormatFacadeV1_Implementation;

/**
 * Given a CCNxTlvDictionary representing a CCNxWireFormatMessage, return the address of the CCNxWireFormatMessageInterface
 * instance that should be used to access the WireFormatMessage.
 *
 * @param interestDictionary - a {@link CCNxTlvDictionary} representing a CCNxInterestReturn.
 * @return the address of the `CCNxWireFormatMessageInterface` instance that should be used to access the CCNxWireFormatMessage.
 *
 * Example:
 * @code
 * {
 *
 *     CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromInterestPacketType(schemaVersion, wireFormatBuffer);
 *
 *     //V1 test
 *     if (ccnxWireFormatMessageInterface_GetInterface(interestReturn) == &CCNxWireFormatFacadeV1_Implementation) {
 *         printf("Using a V1 CCNxWireFormatFacadeV1_Implementation \n");
 *     }
 *
 *     ...
 *
 *     ccnxWireFormatMessage_Release(&message);
 * } * @endcode
 */
CCNxWireFormatMessageInterface *ccnxWireFormatMessageInterface_GetInterface(const CCNxTlvDictionary *contentDictionary);
#endif
