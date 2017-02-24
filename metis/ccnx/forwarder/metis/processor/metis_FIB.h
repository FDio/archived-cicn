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
 * The Forwarding Information Base (FIB) table is a map from a name to a MetisFibEntry.
 *
 * Each MetisFibEntry has a set of nexthops and a MetisStrategy to pick a nexthop.
 *
 * The strategy may be changed.  It will wipe out all the previous state for the last
 * strategy and the new strategy will need to start from scratch.  changing the strategy does
 * not change the nexthops, but it does wipe any stragegy-specific state in each nexthop.
 *
 * So, the FIB table is make up of rows like this:
 *   name -> { strategy, { {nexthop_1, strategyState_1}, {nexthop_2, strategyState_2}, ... } }
 *
 * The "strategy" is a MetisStrategyImpl function structure (see strategies/metis_Strategy.h).
 * Some strategies might allocate an implementation per row, others might use one implementation
 * for the whole table.  Its up to the strategy implementation.
 *
 *
 */

#ifndef Metis_metis_FIB_h
#define Metis_metis_FIB_h

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/api/control/cpi_RouteEntry.h>

#include <ccnx/forwarder/metis/core/metis_NumberSet.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/processor/metis_FibEntryList.h>
#include <ccnx/forwarder/metis/processor/metis_FibEntry.h>
#include <ccnx/forwarder/metis/core/metis_Logger.h>

struct metis_fib;
typedef struct metis_fib MetisFIB;

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
MetisFIB *metisFIB_Create(MetisLogger *logger);

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
void metisFIB_Destroy(MetisFIB **fibPtr);

/**
 * @function metisFib_AddOrUpdate
 * @abstract Adds or updates a route
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return true if added/updated, false if a problem.
 */
bool metisFIB_AddOrUpdate(MetisFIB *fib, CPIRouteEntry *route, const char * fwdStrategy);

/**
 * Removes a route
 *
 * Removes a specific nexthop for a route.  If there are no nexthops left after the
 * removal, the entire route is deleted from the FIB.
 *
 * @param [in] fib The FIB to modify
 * @param [in] route The route to remove
 *
 * @retval true Route completely removed
 * @retval false There are still other nexthops for the route
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisFIB_Remove(MetisFIB *fib, CPIRouteEntry *route);

/**
 * Removes the given connection ID from all routes
 *
 * Removes the given connection ID from all routes.  If that leaves a route
 * with no nexthops, the route remains in the table with an empty nexthop set.
 *
 * @param [in] fib The forwarding table
 * @param [in] connectionId The connection to remove.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisFIB_RemoveConnectionIdFromRoutes(MetisFIB *fib, unsigned connectionId);

/**
 * @function metisFib_Match
 * @abstract Lookup the interest in the FIB, returns set of connection ids to forward over
 * @discussion
 *   This is the internal state of the FIB entry.  If you will store a copy you must acquire a reference.
 *
 * @param <#param1#>
 * @return May be empty, should not be null
 */
MetisFibEntry *metisFIB_Match(MetisFIB *fib, const MetisMessage *interestMessage);
/**
 * @function metisFib_Length
 * @abstract The number of entries in the forwarding table
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
size_t metisFIB_Length(const MetisFIB *fib);

/**
 * @function metisFib_GetEntries
 * @abstract Returns a list of the current FIB entries.
 * @discussion
 *   Caller must destroy the list
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisFibEntryList *metisFIB_GetEntries(const MetisFIB *fib);
#endif // Metis_metis_FIB_h
