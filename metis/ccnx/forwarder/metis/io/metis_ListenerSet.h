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
 * @file metis_ListenerSet.h
 * @brief A listener set is unique on (MetisEncapType, localAddress)
 *
 * Keeps track of all the running listeners.  The set is unique on the
 * encapsulation type and the local address.  For example, with TCP encapsulation and
 * local address 127.0.0.1 or Ethernet encapsulation and MAC address 00:11:22:33:44:55.
 *
 * NOTE: This does not allow multiple EtherType on the same interface because the CPIAddress for
 * a LINK address does not include an EtherType.
 *
 */

#ifndef Metis_metis_ListenerSet_h
#define Metis_metis_ListenerSet_h

#include <ccnx/forwarder/metis/io/metis_Listener.h>

struct metis_listener_set;
typedef struct metis_listener_set MetisListenerSet;

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisListenerSet *metisListenerSet_Create(void);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisListenerSet_Destroy(MetisListenerSet **setPtr);

/**
 * @function metisListenerSet_Add
 * @abstract Adds the listener to the set
 * @discussion
 *     Unique set based on pair (MetisEncapType, localAddress).
 *     Takes ownership of the ops memory if added.
 *
 * @param <#param1#>
 * @return true if added, false if not
 */
bool metisListenerSet_Add(MetisListenerSet *set, MetisListenerOps *ops);

/**
 * The number of listeners in the set
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] set An allocated listener set
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t metisListenerSet_Length(const MetisListenerSet *set);

/**
 * Returns the listener at the given index
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] set An allocated listener set
 * @param [in] index The index position (0 <= index < metisListenerSet_Lenght)
 *
 * @retval non-null The listener at index
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisListenerOps *metisListenerSet_Get(const MetisListenerSet *set, size_t index);

/**
 * Looks up a listener by its key (EncapType, LocalAddress)
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] set An allocated listener set
 * @param [in] encapType the listener type
 * @param [in] localAddress The local bind address (e.g. MAC address or TCP socket)
 *
 * @retval non-null The listener matching the query
 * @retval null Does not exist
 *
 * Example:
 * @code
 *
 * @endcode
 */
MetisListenerOps *metisListenerSet_Find(const MetisListenerSet *set, MetisEncapType encapType, const CPIAddress *localAddress);
#endif
