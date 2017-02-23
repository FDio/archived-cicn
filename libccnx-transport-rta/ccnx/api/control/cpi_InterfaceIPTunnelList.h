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
 * @file cpi_InterfaceIPTunnelList.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef libccnx_cpi_InterfaceIPTunnelList_h
#define libccnx_cpi_InterfaceIPTunnelList_h

struct cpi_interface_iptunnel_list;
typedef struct cpi_interface_iptunnel_list CPIInterfaceIPTunnelList;

#include <ccnx/api/control/cpi_InterfaceIPTunnel.h>

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
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
 * @see <#references#>
 */
CPIInterfaceIPTunnelList *cpiInterfaceIPTunnelList_Create(void);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
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
 * @see <#references#>
 */
void cpiInterfaceIPTunnelList_Destroy(CPIInterfaceIPTunnelList **listPtr);

/**
 * Adds a iptunnel entry to the list.
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
void cpiInterfaceIPTunnelList_Append(CPIInterfaceIPTunnelList *list, CPIInterfaceIPTunnel *entry);

/**
 * Determine if two CPIInterfaceIPTunnelList instances are equal.
 *
 * Two CPIInterfaceIPTunnelList instances are equal if, and only if,
 * the size of the lists are equal and every element is equal and in the same order.
 *
 * The following equivalence relations on non-null `CPIInterfaceIPTunnelList` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIInterfaceIPTunnelList_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiInterfaceIPTunnelList_Equals(x, y)` must return true if and only if
 *        `cpiInterfaceIPTunnelList_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiInterfaceIPTunnelList_Equals(x, y)` returns true and
 *        `cpiInterfaceIPTunnelList_Equals(y, z)` returns true,
 *        then  `cpiInterfaceIPTunnelList_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiInterfaceIPTunnelList_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiInterfaceIPTunnelList_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIInterfaceIPTunnelList` instance.
 * @param b A pointer to a `CPIInterfaceIPTunnelList` instance.
 * @return true if the two `CPIInterfaceIPTunnelList` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIInterfaceIPTunnelList *a = cpiInterfaceIPTunnelList_Create();
 *    CPIInterfaceIPTunnelList *b = cpiInterfaceIPTunnelList_Create();
 *
 *    if (cpiInterfaceIPTunnelList_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool cpiInterfaceIPTunnelList_Equals(const CPIInterfaceIPTunnelList *a, const CPIInterfaceIPTunnelList *b);

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
size_t cpiInterfaceIPTunnelList_Length(const CPIInterfaceIPTunnelList *list);

/**
 * Returns a reference counted copy of the iptunnel entry.
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
CPIInterfaceIPTunnel *cpiInterfaceIPTunnelList_Get(CPIInterfaceIPTunnelList *list, size_t index);

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
PARCJSON *cpiInterfaceIPTunnelList_ToJson(const CPIInterfaceIPTunnelList *list);

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
CPIInterfaceIPTunnelList *cpiInterfaceIPTunnelList_FromJson(PARCJSON *json);
#endif // libccnx_cpi_InterfaceIPTunnelList_h
