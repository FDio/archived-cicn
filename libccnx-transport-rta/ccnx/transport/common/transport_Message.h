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
 * @file transport_Message.h
 * @brief <#Brief Description#>
 *
 * NOTE: TransportMessage is being phased out for the CCNxTlvDictionary
 *
 */
#ifndef Libccnx_transport_Message_h
#define Libccnx_transport_Message_h

#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

struct transport_message;
/**
 *
 * @see TransportMessage_CreateFromCcnxMessage
 * @see TransportMessage_CreateFromMessage
 */
typedef struct transport_message TransportMessage;

/**
 * Stores a reference to the given dictionary
 *
 * The caller is responsible for releasing 'dictionary' as the transport message stores its own reference.
 *
 * @param [in] dictionary A pointer to a valid `CCNxTlvDictionary`
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
TransportMessage *transportMessage_CreateFromDictionary(CCNxTlvDictionary *dictionary);

bool transportMessage_isValid(const TransportMessage *message);

#ifdef TransportLibrary_DISABLE_VALIDATION
#  define transportMessage_OptionalAssertValid(_instance_)
#else
#  define transportMessage_OptionalAssertValid(_instance_) transportMessage_AssertValid(_instance_)
#endif
/**
 * Assert that the given `TransportMessage` instance is valid.
 *
 * @param [in] message A pointer to a valid TransportMessage instance.
 *
 * Example:
 * @code
 * {
 *     TransportMessage *a = transportMessage_CreateFromDictionary(dictionary);
 *
 *     transportMessage_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     transportMessage_Release(&b);
 * }
 * @endcode
 */
void transportMessage_AssertValid(const TransportMessage *message);

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
CCNxTlvDictionary *transportMessage_GetDictionary(TransportMessage *tm);

/**
 * Destroy the transport message and everything inside it
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
void transportMessage_Destroy(TransportMessage **tmPtr);

typedef void (TransportMessage_Free)(void **);

/**
 * <#One Line Description#>
 *
 * Add some stack payload to a transport message.
 * Will not be freed.
 * This is typically used to put a pointer to the RtaConnection in the message.
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
void transportMessage_SetInfo(TransportMessage *tm, void *info, TransportMessage_Free *freefunc);

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
void *transportMessage_GetInfo(const TransportMessage *tm);

bool transportMessage_IsControl(const TransportMessage *tm);
bool transportMessage_IsInterest(const TransportMessage *tm);
bool transportMessage_IsContentObject(const TransportMessage *tm);

/**
 * If in DEBUG mode, returns how long the message has been in the system
 *
 * If not in DEBUG mode, will always be {.tv_sec = 0, .tv_usec = 0}.  The time is based
 * on gettimeofday().
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
struct timeval transportMessage_GetDelay(const TransportMessage *tm);
#endif // Libccnx_transport_Message_h
