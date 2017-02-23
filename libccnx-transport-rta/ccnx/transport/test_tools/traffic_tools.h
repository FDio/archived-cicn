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
 * @file traffic_tools.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_traffic_generators_h
#define Libccnx_traffic_generators_h

#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_ComponentQueue.h>

#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/ccnx_Interest.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * <#One Line Description#>
 *
 * Read the provided queue and verify the received message (Interest or Object)
 * has the given basename (not including segment) and the provided segment number
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
bool trafficTools_ReadAndVerifySegment(PARCEventQueue *queue, CCNxName *basename,
                                       uint64_t expectedSegnum, PARCBuffer *expectedPayload);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
bool trafficTools_GetObjectSegmentFromName(CCNxName *name, uint64_t *outputSegment);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
CCNxContentObject *trafficTools_CreateSignedContentObject();

/**
 * Create a `CCNxUnsignedContentObject` with the given payload.
 *
 * The payload is contained within the given `PARCBuffer` from its position to its limit.
 *
 * @param [in] payload <#description#>
 *
 * @return <#value#> <#explanation#>
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 * @see PARCBuffer
 */
CCNxContentObject *trafficTools_CreateContentObjectWithPayload(PARCBuffer *payload);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
CCNxInterest *trafficTools_CreateInterest(void);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
TransportMessage *trafficTools_CreateTransportMessageWithControlMessage(RtaConnection *connection);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
TransportMessage *trafficTools_CreateTransportMessageWithInterest(RtaConnection *connection);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
TransportMessage *trafficTools_CreateTransportMessageWithSignedContentObject(RtaConnection *connection);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
TransportMessage *trafficTools_CreateTransportMessageWithSignedContentObjectWithName(RtaConnection *connection,
                                                                                     CCNxName *name, const char *keystorePath, const char *keystorePassword);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
TransportMessage *trafficTools_CreateTransportMessageWithRaw(RtaConnection *connection);


/**
 * Creates a Dictionary format RAW message
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
TransportMessage *trafficTools_CreateTransportMessageWithDictionaryRaw(RtaConnection *connection, unsigned schema);

/**
 * Creates a dictionary format Interest message
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
TransportMessage *trafficTools_CreateTransportMessageWithDictionaryInterest(RtaConnection *connection, unsigned schema);

/**
 * Creates a dictioary format Control message
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
TransportMessage *trafficTools_CreateTransportMessageWithDictionaryControl(RtaConnection *connection, unsigned schema);

/**
 * Creates an interest in Dictionary format
 *
 * Does not have a wire format
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxTlvDictionary *trafficTools_CreateDictionaryInterest(void);
#endif // Libccnx_traffic_generators_h
