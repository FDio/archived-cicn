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
 * @file cpi_RouteEntryList.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef libccnx_cpi_RouteEntryList_h
#define libccnx_cpi_RouteEntryList_h

struct cpi_route_entry_list;
typedef struct cpi_route_entry_list CPIRouteEntryList;

#include <ccnx/api/control/cpi_RouteEntry.h>

CPIRouteEntryList *cpiRouteEntryList_Create();
void cpiRouteEntryList_Destroy(CPIRouteEntryList **listPtr);

/**
 * Adds a route entry to the list.
 *
 *   Appends <code>entry</code> to the list.  Takes ownership of the entry
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void cpiRouteEntryList_Append(CPIRouteEntryList *list, CPIRouteEntry *entry);

/**
 * Determine if two CPIRouteEntryList instances are equal.
 *
 * Two CPIRouteEntryList instances are equal if, and only if,
 * * ...
 *
 * The following equivalence relations on non-null `CPIRouteEntryList` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIRouteEntryList_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiRouteEntryList_Equals(x, y)` must return true if and only if
 *        `cpiRouteEntryList_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiRouteEntryList_Equals(x, y)` returns true and
 *        `cpiRouteEntryList_Equals(y, z)` returns true,
 *        then  `cpiRouteEntryList_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiRouteEntryList_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiRouteEntryList_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIRouteEntryList` instance.
 * @param b A pointer to a `CPIRouteEntryList` instance.
 * @return true if the two `CPIRouteEntryList` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIRouteEntryList *a = cpiRouteEntryList_Create();
 *    CPIRouteEntryList *b = cpiRouteEntryList_Create();
 *
 *    if (cpiRouteEntryList_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool cpiRouteEntryList_Equals(const CPIRouteEntryList *a, const CPIRouteEntryList *b);

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
size_t cpiRouteEntryList_Length(const CPIRouteEntryList *list);

/**
 * Returns a reference counted copy of the route entry.
 *
 *   Caller must destroy the returned value.
 *   Will assert if you go beyond the end of the list.
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIRouteEntry *cpiRouteEntryList_Get(CPIRouteEntryList *list, size_t index);


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
PARCJSON *cpiRouteEntryList_ToJson(const CPIRouteEntryList *list);

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
CPIRouteEntryList *cpiRouteEntryList_FromJson(PARCJSON *json);
#endif // libccnx_cpi_RouteEntryList_h
