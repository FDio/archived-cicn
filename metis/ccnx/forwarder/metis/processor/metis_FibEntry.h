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
 * @file metis_FibEntry.h
 * @brief A forwarding entry in the FIB table
 *
 * A Forwarding Information Base (FIB) entry (MetisFibEntry) is a
 * set of nexthops for a name.  It also indicates the forwarding strategy.
 *
 * Each nexthop contains the ConnectionId assocaited with it.  This could be
 * something specific like a MAC address or point-to-point tunnel.  Or, it
 * could be something general like a MAC group address or ip multicast overlay.
 *
 * See metis/strategies/metis_Strategy.h for a description of forwarding strategies.
 * In short, a strategy is the algorithm used to select one or more nexthops from
 * the set of available nexthops.
 *
 * Each nexthop also contains a void* to a forwarding strategy data container.
 * This allows a strategy to keep proprietary information about each nexthop.
 *
 *
 */

#ifndef Metis_metis_FibEntry_h
#define Metis_metis_FibEntry_h

#include <ccnx/forwarder/metis/tlv/metis_TlvName.h>
#include <ccnx/forwarder/metis/strategies/metis_StrategyImpl.h>

struct metis_fib_entry;
typedef struct metis_fib_entry MetisFibEntry;

MetisFibEntry *metisFibEntry_Create(MetisTlvName *name, const char *fwdStrategy);

/**
 * Decrements the reference count by one, and destroys the memory after last release
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in,out] fibEntryPtr A pointer to a MetisFibEntry, will be NULL'd
 *
 * Example:
 * @code
 * {
 *     MetisFibEntry *fibEntry = metisFibEntry(name);
 *     metisFibEntry_Release(&fibEntry);
 * }
 * @endcode
 */
void metisFibEntry_Release(MetisFibEntry **fibEntryPtr);

/**
 * Returns a reference counted copy of the fib entry
 *
 * The reference count is increased by one.  The returned value must be
 * released via metisFibEnty_Release().
 *
 * @param [in] fibEntry An allocated MetisFibEntry
 *
 * @return non-null A reference counted copy of the fibEntry
 *
 * Example:
 * @code
 * {
 *     MetisFibEntry *fibEntry = metisFibEntry(name);
 *     MetisFibEntry *copy = metisFibEntry_Acquire(fibEntry);
 *     metisFibEntry_Release(&copy);
 *     metisFibEntry_Release(&fibEntry);
 * }
 * @endcode
 */
MetisFibEntry *metisFibEntry_Acquire(const MetisFibEntry *fibEntry);

void metisFibEntry_SetStrategy(MetisFibEntry *fibEntry, const char *strategy);
void metisFibEntry_AddNexthop(MetisFibEntry *fibEntry, CPIRouteEntry *route);
void metisFibEntry_RemoveNexthopByRoute(MetisFibEntry *fibEntry, CPIRouteEntry *route);
void metisFibEntry_RemoveNexthopByConnectionId(MetisFibEntry *fibEntry, unsigned connectionId);


size_t metisFibEntry_NexthopCount(const MetisFibEntry *fibEntry);

/**
 * @function metisFibEntry_GetNexthops
 * @abstract Returns the nexthop set of the FIB entry.  You must Acquire if it will be saved.
 * @discussion
 *   Returns the next hop set for the FIB entry.
 *
 * @param <#param1#>
 * @return <#return#>
 */
const MetisNumberSet *metisFibEntry_GetNexthops(const MetisFibEntry *fibEntry);
const MetisNumberSet *metisFibEntry_GetNexthopsFromForwardingStrategy(const MetisFibEntry *fibEntry, const MetisMessage *interestMessage);

void metisFibEntry_ReceiveObjectMessage(const MetisFibEntry *fibEntry, const MetisNumberSet *egressId, const MetisMessage *objectMessage, MetisTicks rtt);
void metisFibEntry_OnTimeout(const MetisFibEntry *fibEntry, const MetisNumberSet *egressId);
const char *metisFibEntry_GetFwdStrategyType(const MetisFibEntry *fibEntry);
MetisStrategyImpl *metisFibEntry_GetFwdStrategy(const MetisFibEntry *fibEntry);

/**
 * @function metisFibEntry_GetPrefix
 * @abstract Returns a copy of the prefix.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return A reference counted copy that you must destroy
 */
MetisTlvName *metisFibEntry_GetPrefix(const MetisFibEntry *fibEntry);
#endif // Metis_metis_FibEntry_h
