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
 * @file metis_MessageProcessor.h
 * @brief Executes the set of rules dictated by the PacketType
 *
 * This is a "run-to-completion" handling of a message based on the PacketType.
 *
 * The MessageProcessor also owns the PIT and FIB tables.
 *
 */

#ifndef Metis_metis_MessageProcessor_h
#define Metis_metis_MessageProcessor_h

#include <ccnx/api/control/cpi_RouteEntry.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/processor/metis_Tap.h>
#include <ccnx/forwarder/metis/content_store/metis_ContentStoreInterface.h>

struct metis_message_processor;
typedef struct metis_message_processor MetisMessageProcessor;

/**
 * Allocates a MessageProcessor along with PIT, FIB and ContentStore tables
 *
 * The metis pointer is primarily used for logging (metisForwarder_Log), getting the
 * configuration, and accessing the connection table.
 *
 * @param [in] metis Pointer to owning Metis process
 *
 * @retval non-null An allocated message processor
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisMessageProcessor *metisMessageProcessor_Create(MetisForwarder *metis);

/**
 * Deallocates a message processor an all internal tables
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in,out] processorPtr Pointer to message processor to de-allocate, will be NULL'd.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisMessageProcessor_Destroy(MetisMessageProcessor **processorPtr);

/**
 * @function metisMessageProcessor_Receive
 * @abstract Process the message, takes ownership of the memory.
 * @discussion
 *   Will call destroy on the memory when done with it, so if the caller wants to
 *   keep it, make a reference counted copy.
 *
 *   Receive may modify some fields in the message, such as the HopLimit field.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisMessageProcessor_Receive(MetisMessageProcessor *procesor, MetisMessage *message);

/**
 * @function metisMessageProcessor_AddTap
 * @abstract Add a tap to see messages.  Only one allowed. caller must remove and free it.
 * @discussion
 *   The tap will see messages on Receive, Drop, or Send, based on the properties of the Tap.
 *   The caller owns the memory and must remove and free it.
 *
 *   Currently only supports one tap.  If one is already set, its replaced.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisMessageProcessor_AddTap(MetisMessageProcessor *procesor, MetisTap *tap);

/**
 * @function metisMessageProcessor_RemoveTap
 * @abstract Removes the tap from the message path.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisMessageProcessor_RemoveTap(MetisMessageProcessor *procesor, const MetisTap *tap);

/**
 * Adds or updates a route in the FIB
 *
 * If the route already exists, it is replaced
 *
 * @param [in] procesor An allocated message processor
 * @param [in] route The route to update
 *
 * @retval true added or updated
 * @retval false An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisMessageProcessor_AddOrUpdateRoute(MetisMessageProcessor *procesor, CPIRouteEntry *route);

/**
 * Removes a route from the FIB
 *
 * Removes a specific nexthop for a route.  If there are no nexthops left after the
 * removal, the entire route is deleted from the FIB.
 *
 * @param [in] procesor An allocated message processor
 * @param [in] route The route to remove
 *
 * @retval true Route completely removed
 * @retval false There is still a nexthop for the route
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisMessageProcessor_RemoveRoute(MetisMessageProcessor *procesor, CPIRouteEntry *route);

/**
 * Removes a given connection id from all FIB entries
 *
 * Iterates the FIB and removes the given connection ID from every route.
 * If a route is left with no nexthops, it stays in the FIB, but packets that match it will
 * not be forwarded.  IS THIS THE RIGHT BEHAVIOR?
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisMessageProcessor_RemoveConnectionIdFromRoutes(MetisMessageProcessor *processor, unsigned connectionId);

/**
 * Returns a list of all FIB entries
 *
 * You must destroy the list.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval non-null The list of FIB entries
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisFibEntryList *metisMessageProcessor_GetFibEntries(MetisMessageProcessor *processor);

/**
 * Adjusts the ContentStore to the given size.
 *
 * This will destroy and re-create the content store, so any cached objects will be lost.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisMessageProcessor_SetContentObjectStoreSize(MetisMessageProcessor *processor, size_t maximumContentStoreSize);

/**
 * Return the interface to the currently instantiated ContentStore, if any.
 *
 * @param [in] processor the `MetisMessageProcessor` from which to return the ContentStoreInterface.
 *
 * Example:
 * @code
 * {
 *     MetisContentStoreInterface *storeImpl = metisMessageProcessor_GetContentObjectStore(processor);
 *     size_t capacity = metisContentStoreInterface_GetObjectCapacity(storeImpl);
 * }
 * @endcode
 */
MetisContentStoreInterface *metisMessageProcessor_GetContentObjectStore(const MetisMessageProcessor *processor);

void metisMessageProcessor_SetCacheStoreFlag(MetisMessageProcessor *processor, bool val);

bool metisMessageProcessor_GetCacheStoreFlag(MetisMessageProcessor *processor);

void metisMessageProcessor_SetCacheServeFlag(MetisMessageProcessor *processor, bool val);

bool metisMessageProcessor_GetCacheServeFlag(MetisMessageProcessor *processor);

void metisMessageProcessor_ClearCache(MetisMessageProcessor *processor);

void metisProcessor_SetStrategy(MetisMessageProcessor *processor, CCNxName *prefix, const char *strategy);

#endif // Metis_metis_MessageProcessor_h
