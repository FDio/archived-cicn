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
 * @file metis_Message.h
 * @brief MetisMessage is the unit of forwarding, i.e. the packets being switched
 *
 */
#ifndef Metis_metis_Message_h
#define Metis_metis_Message_h

#include <config.h>
#include <ccnx/forwarder/metis/core/metis_MessagePacketType.h>
#include <ccnx/forwarder/metis/core/metis_StreamBuffer.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvName.h>
#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>

#include <parc/algol/parc_EventQueue.h>
#include <parc/algol/parc_EventBuffer.h>

#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_ControlMessage.h>

#include <ccnx/forwarder/metis/core/metis_Ticks.h>

struct metis_message;
typedef struct metis_message MetisMessage;

/**
 * @function metisMessage_ReadFromBuffer
 * @abstract Read bytes from the input buffer and create a MetisMessage
 * @discussion
 *   There must be bytesToRead bytes available.
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisMessage *metisMessage_ReadFromBuffer(unsigned ingressConnectionId, MetisTicks receiveTime, PARCEventBuffer *input, size_t bytesToRead, MetisLogger *logger);

/**
 * @function metisMessage_CreateFromBuffer
 * @abstract Takes ownership of the input buffer, which comprises one complete message
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisMessage *metisMessage_CreateFromBuffer(unsigned ingressConnectionId, MetisTicks receiveTime, PARCEventBuffer *input, MetisLogger *logger);

/**
 * @function metisMessage_CreateFromArray
 * @abstract Copies the input buffer into the message.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisMessage *metisMessage_CreateFromArray(const uint8_t *data, size_t dataLength, unsigned ingressConnectionId, MetisTicks receiveTime, MetisLogger *logger);

/**
 * @function metisMessage_CreateFromParcBuffer
 * @abstract Creates a message from the byte buffer
 * @discussion
 *   Caller retains owership of the buffer
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisMessage *metisMessage_CreateFromParcBuffer(PARCBuffer *buffer, unsigned ingressConnectionId, MetisTicks receiveTime, MetisLogger *logger);

/**
 * @function metisMessage_Copy
 * @abstract Get a reference counted copy
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisMessage *metisMessage_Acquire(const MetisMessage *message);

/**
 * Releases the message and frees the memory
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
void metisMessage_Release(MetisMessage **messagePtr);

/**
 * Writes the message to the queue
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
bool metisMessage_Write(PARCEventQueue *parcEventQueue, const MetisMessage *message);

/**
 * Appends the message to the buffer
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
bool metisMessage_Append(PARCEventBuffer *parcEventBuffer, const MetisMessage *message);

/**
 * Returns the total byte length of the message
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
size_t metisMessage_Length(const MetisMessage *message);


bool metisMessage_HasWldr(const MetisMessage *message);

unsigned metisMessage_GetWldrType(const MetisMessage *message);

unsigned metisMessage_GetWldrLabel(const MetisMessage *message);

unsigned metisMessage_GetWldrLastReceived(const MetisMessage *message);

void metisMessage_SetWldrLabel(MetisMessage *message, uint16_t label);

void metisMessage_SetWldrNotification(MetisMessage *message, uint16_t expected, uint16_t lastReceived);

/**
 * Returns the connection id of the packet input
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
unsigned metisMessage_GetIngressConnectionId(const MetisMessage *message);

/**
 * Returns the receive time (in router ticks) of the message
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
MetisTicks metisMessage_GetReceiveTime(const MetisMessage *message);

/**
 * Returns the PacketType from the FixedHeader
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] message A parsed message
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisMessagePacketType  metisMessage_GetType(const MetisMessage *message);

/**
 * Determines if the message has a hop limit
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval true A hop limit exists
 * @retval false A hop limit does not exist
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisMessage_HasHopLimit(const MetisMessage *message);

void metisMessage_UpdatePathLabel(MetisMessage *message, uint8_t outFace);
void metisMessage_ResetPathLabel(MetisMessage *message);

/**
 * Returns the hoplimit of the message
 *
 * Will assert if the message does not have a hoplimit.  Use metisMessage_HasHopLimit() first
 * to determine if there is a hop limit.
 *
 * @param [in] message An allocated MetisMessage
 *
 * @retval number The hop limit
 *
 * Example:
 * @code
 * {
 *    if (metisMessage_HasHopLimit(message)) {
 *       uint8_t hoplimit = metisMessage_GetHopLimit(message);
 *       if (hoplimit == 0) {
 *          // drop packet
 *       } else {
 *          metisMessage_SetHopLimit(message, hoplimit - 1);
 *       }
 *    }
 * }
 * @endcode
 */
uint8_t metisMessage_GetHopLimit(const MetisMessage *message);

/**
 * Sets the message hop limit to the specified value
 *
 * Will assert if the message does not already have a hop limit. Use metisMessage_HasHopLimit() first
 * to determine if there is a hop limit.
 *
 * @param [in] message An allocated MetisMessage
 * @param [in] hoplimit The value to set in the packet
 *
 * Example:
 * @code
 * {
 *    if (metisMessage_HasHopLimit(message)) {
 *       uint8_t hoplimit = metisMessage_GetHopLimit(message);
 *       if (hoplimit == 0) {
 *          // drop packet
 *       } else {
 *          metisMessage_SetHopLimit(message, hoplimit - 1);
 *       }
 *    }
 * }
 * @endcode
 */
void metisMessage_SetHopLimit(MetisMessage *message, uint8_t hoplimit);

// ===========================================================
// Accessors used to index and compare messages

/**
 * @function metisMessage_GetName
 * @abstract The name in the CCNx message
 * @discussion
 *   The name of the Interest or Content Object.  If the caller will store the
 *   name, he should make a reference counted copy.
 *
 * @param <#param1#>
 * @return The name as stored in the message object.
 */
MetisTlvName *metisMessage_GetName(const MetisMessage *message);

/**
 * @function metisMessage_GetKeyIdHash
 * @abstract Non-cryptographic hash of the KeyId
 * @discussion
 *   If the message does not have a KeyId, the output pointer is left unchanged.
 *
 * @param hashOutput will be filled in with the hash, if a KeyId exists.
 * @return true if object has a KeyId
 */
bool metisMessage_GetKeyIdHash(const MetisMessage *message, uint32_t *hashOutput);


/**
 * Determine if the KeyIds of two Metis Messages are equal.
 *
 * The following equivalence relations on non-null KeyIds in `MetisMessage` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `metisMessage_KeyIdEquals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisMessage_KeyIdEquals(x, y)` must return true if and only if
 *        `metisMessage_KeyIdEquals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisMessage_KeyIdEquals(x, y)` returns true and
 *        `metisMessage_KeyIdEquals(y, z)` returns true,
 *        then  `metisMessage_KeyIdEquals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisMessage_KeyIdEquals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `metisMessage_KeyIdEquals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `MetisMessage` instance.
 * @param b A pointer to a `MetisMessage` instance.
 * @return true if the KeyIds of the two `MetisMessage` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisMessage *a = metisMessage_Create();
 *    MetisMessage *b = metisMessage_Create();
 *
 *    if (metisMessage_KeyIdEquals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool metisMessage_KeyIdEquals(const MetisMessage *a, const MetisMessage *b);

/**
 * Determine if the ContentObjectHashes of two `Metis Messages` are equal.
 *
 * The ContentObjectHashes of two `MetisMessage` instances are equal if, and only if,
 * a ContentObjectHash exists in both `MetisMessage` and are equal.
 *
 *
 * The following equivalence relations on non-null KeyIds in `MetisMessage` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `metisMessage_ObjectHashEquals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisMessage_ObjectHashEquals(x, y)` must return true if and only if
 *        `metisMessage_ObjectHashEquals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisMessage_ObjectHashEquals(x, y)` returns true and
 *        `metisMessage_ObjectHashEquals(y, z)` returns true,
 *        then  `metisMessage_ObjectHashEquals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisMessage_ObjectHashEquals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `metisMessage_ObjectHashEquals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `MetisMessage` instance.
 * @param b A pointer to a `MetisMessage` instance.
 * @return true if the KeyIds of the two `MetisMessage` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisMessage *a = metisMessage_Create();
 *    MetisMessage *b = metisMessage_Create();
 *
 *    if (metisMessage_ObjectHashEquals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool metisMessage_ObjectHashEquals(MetisMessage *a, MetisMessage *b);

/**
 * @function metisMessage_GetContentObjectHashHash
 * @abstract Non-cryptographic hash of the ContentObjectHash
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return true if message has a contentobject hash and the output updated.
 */
bool metisMessage_GetContentObjectHashHash(MetisMessage *message, uint32_t *hashOutput);

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
 */
bool metisMessage_HasName(const MetisMessage *message);

/**
 * Return true if there is a KeyId associated with this `MetisMessage`.  *
 *
 * Note that this will return true if either the underlying message is an Interest that contains
 * a KeyId restriction, or if the underlying message is a ContentObject that contains a KeyId.
 * In the latter case, the KeyId might have been specified by the creator of the content, or
 * it may have been calculated when we verified the public key specified by the ContentObject.
 *
 *
 * @param [in] message the `MetisMessage` to query.
 *
 * @return true if there is a KeyId (or KeyId restriction) associated with this `MetisMessage`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisMessage_HasKeyId(const MetisMessage *message);

/**
 * Return true if there is a KeyId associated with this `MetisMessage`, and that KeyId
 * has been verified.
 *
 * This KeyId may have been specified by the sender, or may have been calculated while
 * verifying the public key associated with this `MetisMessage`s validation data, if any.
 *
 * @param [in] message the `MetisMessage` to query.
 *
 * @return true if there is a KeyId associated with this `MetisMessage`, and that KeyId has been verified.
 * @return false if there is no KeyId associated with this `MetisMessage` or it has not yet been verified.
 * Example:
 * @code
 * {
 *     if (metisMessage_IsKeyIdVerified(message)) {
 *         doSomethingWithMssage(message);
 *     } else {
 *         // KeyId was not verified
 *     }
 * }
 * @endcode
 */
bool metisMessage_IsKeyIdVerified(const MetisMessage *message);

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
 */
bool metisMessage_HasContentObjectHash(const MetisMessage *message);

/**
 * @function metisMessage_GetControlMessage
 * @abstract A TLV_MSG_CPI will return a control message
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
CCNxControl *metisMessage_CreateControlMessage(const MetisMessage *message);

/**
 * Determines if the message has an Interest Lifetime parameter
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] message An allocated and parsed Message
 *
 * @retval true If an Intrerest Lifetime field exists
 * @retval false If no Interest Lifetime exists
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisMessage_HasInterestLifetime(const MetisMessage *message);

/**
 * Returns the Interest lifetime
 *
 * Will trap if the message does not contain an Interest lifetime
 *
 * @param [in] message An allocated and parsed Message
 *
 * @retval integer Lifetime in forwarder Ticks
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint64_t metisMessage_GetInterestLifetimeTicks(const MetisMessage *message);

/**
 * Return true if the specified `MetisMessage` instance contains a RecommendedCacheTime.
 *
 * The Recommended Cache Time (a millisecond timestamp) is a network byte ordered unsigned integer of the number of
 * milliseconds since the epoch in UTC of when the payload expires. It is a 64-bit field.
 *
 * @param message the `MetisMessage` instance to check for RecommendedCacheTime.
 * @return true if the specified `MetisMessage` instance has a RecommendedCacheTime.
 * @return false otherwise.
 * Example:
 * @code
 * {
 *     MetisMessage *metisMessage = <...>;
 *     if (metisMessage_HasRecommendedCacheTime(metisMessage)) {
 *         uint64_t rct = metisMessage_GetRecommendedCacheTimeTicks(metisMessage);
 *     }
 * }
 * @endcode
 * @see metisMessage_GetRecommendedCacheTimeTicks
 */
bool metisMessage_HasRecommendedCacheTime(const MetisMessage *message);

/**
 * Return the RecommendedCacheTime of the specified `MetisMessage`, if available, in Metis ticks. If not, it will trap.
 * Before calling this function, call {@link metisMessage_HasRecommendedCacheTime} first.
 *
 * The Recommended Cache Time (a millisecond timestamp) is a network byte ordered unsigned integer of the number of
 * milliseconds since the epoch in UTC of when the payload expires. It is a 64-bit field.
 *
 * The Recommended Cache Time is initially set from the referenced tlv_skeleton, but may be assigned by calling
 * {@link metisMessage_SetRecommendedCacheTimeTicks}.
 *
 * @param message the `MetisMessage` instance to check for RecommendedCacheTime.
 * @return the RecommendedCacheTime of the specified `MetisMessage`.
 *
 * Example:
 * @code
 * {
 *     MetisMessage *metisMessage = <...>;
 *     if (metisMessage_HasRecommendedCacheTime(metisMessage)) {
 *         uint64_t rct = metisMessage_GetRecommendedCacheTimeTicks(metisMessage);
 *     }
 * }
 * @endcode
 *
 * @see metisMessage_HasRecommendedCacheTime
 * @see metisMessage_SetRecommendedCacheTimeTicks
 */
uint64_t metisMessage_GetRecommendedCacheTimeTicks(const MetisMessage *message);

/**
 * Return true if the specified `MetisMessage` instance contains an ExpiryTime.
 *
 * The ExpiryTime is the time at which the Payload expires, as expressed by a timestamp containing the number of milliseconds
 * since the epoch in UTC. It is a network byte order unsigned integer in a 64-bit field. A cache or end system should not
 * respond with a Content Object past its ExpiryTime. Routers forwarding a Content Object do not need to check the ExpiryTime.
 * If the ExpiryTime field is missing, the Content Object has no expressed expiration and a cache or end system may use the
 * Content Object for as long as desired.
 *
 * @param message the `MetisMessage` instance to check for ExpiryTime.
 * @return true if the specified `MetisMessage` instance has an ExpiryTime.
 * @return false otherwise.
 *
 * Example:
 * @code
 * {
 *     MetisMessage *metisMessage = <...>;
 *     if (metisMessage_HasExpiryTime(metisMessage)) {
 *         uint64_t expiryTime = metisMessage_GetExpiryTimeTicks(metisMessage);
 *     }
 * }
 * @endcode
 *
 * @see metisMessage_GetExpiryTimeTicks
 * @see metisMessage_SetExpiryTimeTicks
 */
bool metisMessage_HasExpiryTime(const MetisMessage *message);

/**
 * Return the ExpiryTime of the specified `MetisMessage`, if available, in Metis ticks. If not, it will trap.
 * Before calling this function, call {@link metisMessage_HasExpiryTime} first.
 *
 * The ExpiryTime is the time at which the Payload expires, as expressed by a timestamp containing the number of milliseconds
 * since the epoch in UTC. It is a network byte order unsigned integer in a 64-bit field. A cache or end system should not
 * respond with a Content Object past its ExpiryTime. Routers forwarding a Content Object do not need to check the ExpiryTime.
 * If the ExpiryTime field is missing, the Content Object has no expressed expiration and a cache or end system may use the
 * Content Object for as long as desired.
 *
 * The ExpiryTime is initially set from the referenced tlv_skeleton, but may be assigned by calling
 * {@link metisMessage_SetExpiryTimeTicks}.
 *
 * @param message the `MetisMessage` instance to check for ExpiryTime.
 * @return the ExpiryTime of the specified `MetisMessage`.
 *
 * Example:
 * @code
 * {
 *     MetisMessage *metisMessage = <...>;
 *     if (metisMessage_HasExpiryTime(metisMessage)) {
 *         uint64_t rct = metisMessage_GetExpiryTimeTicks(metisMessage);
 *     }
 * }
 * @endcode
 *
 * @see metisMessage_HasExpiryTime
 * @see metisMessage_SetExpiryTimeTicks
 */
uint64_t metisMessage_GetExpiryTimeTicks(const MetisMessage *message);

/**
 * Assign the ExpiryTime of the specified `MetisMessage`, in Metis ticks. This will not update the
 * referenced tlv_skeleton.
 *
 * The ExpiryTime is the time at which the Payload expires, as expressed by a timestamp containing the number of milliseconds
 * since the epoch in UTC. It is a network byte order unsigned integer in a 64-bit field. A cache or end system should not
 * respond with a Content Object past its ExpiryTime. Routers forwarding a Content Object do not need to check the ExpiryTime.
 * If the ExpiryTime field is missing, the Content Object has no expressed expiration and a cache or end system may use the
 * Content Object for as long as desired.
 *
 * @param message the `MetisMessage` instance to check for ExpiryTime.
 * @param expiryTimeTicks the time, in ticks, that this message's payload will expire.
 *
 * Example:
 * @code
 * {
 *     MetisMessage *metisMessage = <...>;
 *     uint64_t timeInTicks = <...>;
 *     metisMessage_SetExpiryTimeTicks(metisMessage, timeInTicks);
 * }
 * @endcode
 *
 * @see metisMessage_HasExpiryTime
 * @see metisMessage_GetExpiryTimeTicks
 */
void metisMessage_SetExpiryTimeTicks(MetisMessage *message, uint64_t expiryTimeTicks);

/**
 * Assign the RecommendedCacheTime of the specified `MetisMessage`, in Metis ticks. This will not update the
 * referenced tlv_skeleton.
 *
 * The Recommended Cache Time (a millisecond timestamp) is a network byte ordered unsigned integer of the number of
 * milliseconds since the epoch in UTC of when the payload expires. It is a 64-bit field.
 *
 * @param message the `MetisMessage` instance to check for ExpiryTime.
 * @param expiryTimeTicks the time, in ticks, that this message's payload will expire.
 *
 * Example:
 * @code
 * {
 *     MetisMessage *metisMessage = <...>;
 *     uint64_t timeInTicks = <...>;
 *     metisMessage_SetRecommendedCacheTimeTicks(metisMessage, timeInTicks);
 * }
 * @endcode
 *
 * @see metisMessage_HasExpiryTime
 */
void metisMessage_SetRecommendedCacheTimeTicks(MetisMessage *message, uint64_t recommendedCacheTimeTicks);

/**
 * Return true if there is a public key associated with the specified `MetisMessage`.
 *
 * @param message the `MetisMessage` instance to check for a public key.
 *
 * @return true if there is a public key in this `MetisMessage`.
 * @return false otherwise.
 */
bool metisMessage_HasPublicKey(const MetisMessage *message);

/**
 * Return true if there is a certificate associated with the specified `MetisMessage`.
 *
 * @param message the `MetisMessage` instance to check for certificate.
 *
 * @return true if there is a certificate in this `MetisMessage`.
 * @return false otherwise.
 */
bool metisMessage_HasCertificate(const MetisMessage *message);

/**
 * Return the public key associated with the specified `MetisMessage`, if it exists.
 *
 * @param message the `MetisMessage` instance to check for a public key.
 *
 * @return a pointer to a PARCBuffer containing the public key of this `MetisMessage`.
 * @return NULL if no public key exists.
 */
PARCBuffer *metisMessage_GetPublicKey(const MetisMessage *message);

/**
 * Return the certificate associated with the specified `MetisMessage`, if it exists.
 *
 * @param message the `MetisMessage` instance to check for a certificate.
 *
 * @return a pointer to a PARCBuffer containing the certificate of this `MetisMessage`.
 * @return NULL if no certificate exists.
 */
PARCBuffer *metisMessage_GetCertificate(const MetisMessage *message);

/**
 * Tests if the packet has a fragment payload
 *
 * The fragment payload is part of a larger packet
 *
 * @param [in] message An allocated and parsed Message
 *
 * @return true The packet contains a portion of another packet
 * @return false There is no fragment payload
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool metisMessage_HasFragmentPayload(const MetisMessage *message);

/**
 * Appends the fragment payload to the given buffer
 *
 * Will append the fragment payload from the message to the given buffer.
 * This is a non-destructive copy.  If there is no fragment payload in the
 * message, 0 bytes will be copied and 0 returned.
 *
 * @param [in] message An allocated and parsed Message
 * @param [in] buffer The buffer to append to
 *
 * @return number The number of bytes appended
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
size_t metisMessage_AppendFragmentPayload(const MetisMessage *message, PARCEventBuffer *buffer);

/**
 * Returns a pointer to the beginning of the FixedHeader
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] message An allocated and parsed Message
 *
 * @return non-null The fixed header memory
 * @return null No fixed header or an error
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const uint8_t *metisMessage_FixedHeader(const MetisMessage *message);

/**
 * Creates a new MetisMessage from a slice of a first message
 *
 * The new MetisMessage will be the header byte array prefix followed by the slice extent.
 * The resulting MetisMessage must be freed by calling metisMessage_Release().
 *
 * Depending on the implementation, this may make a reference to the first message and access the
 * memory by reference.
 *
 * The offset + length must be less than or equal to metisMessage_Length().  It is an error
 * to call with a 0 length.
 *
 * @param [in] message The original message to slice
 * @param [in] offset The offset within the message to start the slice
 * @param [in] length The length after the offset to include in the slice (must be positive)
 * @param [in] headerLength The length of the header to prepend (may be 0)
 * @param [in] header The header to prepend (may be NULL with 0 length)
 *
 * @return non-null A new MetisMessage that has the header plus slice
 * @return null An error
 *
 * Example:
 * @code
 * {
 *    uint8_t interestToFragment[] = {
 *       0x01, 0x00, 0x00,   24,     // ver = 1, type = interest, length = 24
 *       0x20, 0x00, 0x11,    8,     // HopLimit = 32, reserved = 0, flags = 0x11, header length = 8
 *       0x00, 0x01, 0x00,   12,     // type = interest, length = 12
 *       0x00, 0x00, 0x00,    8,     // type = name, length = 8
 *       0x00, 0x02, 0x00,    4,     // type = binary, length = 4
 *       'c',  'o',  'o',  'l',      // "cool"
 *       };
 *
 *    MetisMessage *firstMessage = metisMessage_CreateFromArray(interestToFragment, sizeof(interestToFragment), 1, 100, logger);
 *
 *    uint8_t fragmentHeader[] = {
 *       0x01, 0x05, 0x00,  12,     // hop-by-hop fragment
 *       0x40, 0x00, 0x01,   8,     // B flag, seqnum = 1
 *       0x00, 0x05, 0x00,   8,     // fragment data, length = 8
 *     };
 *
 *    MetisMessage *secondMessage = metisMessage_Slice(firstMessage, 0, 8, sizeof(fragmentHeader), fragmentHeader,);
 *    // the secondMessage message contains the fragmentHeader followed by the first 8 bytes
 *    // of the first message, as showin in wireFormat below.
 *
 *    uint8_t wireFormat[] = {
 *       0x01, 0x05, 0x00,  12,     // hop-by-hop fragment
 *       0x40, 0x00, 0x01,   8,     // B flag, seqnum = 1
 *       0x00, 0x05, 0x00,   8,     // fragment data, length = 8
 *       0x01, 0x00, 0x00,  24,     // ver = 1, type = interest, length = 24
 *       0x20, 0x00, 0x11,   8,     // HopLimit = 32, reserved = 0, flags = 0x11, header length = 8
 *     };
 *
 *    metisMessage_Release(&firstMessage);
 *    metisMessage_Release(&secondMessage);
 * }
 * @endcode
 */
MetisMessage *metisMessage_Slice(const MetisMessage *message, size_t offset, size_t length, size_t headerLength, const uint8_t header[headerLength]);
#endif // Metis_metis_Message_h
