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
 * @file cpi_InterfaceEthernet.h
 * @brief Specialization of InterfaceGeneric to Ethernet
 *
 * <#Detailed Description#>
 *
 */
#ifndef libccnx_cpi_InterfaceEthernet_h
#define libccnx_cpi_InterfaceEthernet_h

#include <ccnx/api/control/cpi_InterfaceType.h>
#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_AddressList.h>


struct cpi_interface_ethernet;
typedef struct cpi_interface_ethernet CPIInterfaceEthernet;

/**
 * Creates an Ethernet-like interface abstraction. Takes ownership of addresses.
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIInterfaceEthernet *cpiInterfaceEthernet_Create(unsigned ifidx, CPIAddressList *addresses);

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
CPIInterfaceEthernet *cpiInterfaceEthernet_Copy(const CPIInterfaceEthernet *ethernet);

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
void cpiInterfaceEthernet_Destroy(CPIInterfaceEthernet **ethernetPtr);

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
void cpiInterfaceEthernet_SetState(CPIInterfaceEthernet *ethernet, CPIInterfaceStateType state);

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
unsigned cpiInterfaceEthernet_GetIndex(const CPIInterfaceEthernet *ethernet);

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
const CPIAddressList *cpiInterfaceEthernet_GetAddresses(const CPIInterfaceEthernet *ethernet);

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
CPIInterfaceStateType cpiInterfaceEthernet_GetState(const CPIInterfaceEthernet *ethernet);

/**
 * Determine if two CPIInterfaceEthernet instances are equal.
 *
 * Two CPIInterfaceEthernet instances are equal if, and only if, the same state and index and addresses
 * are equal in the same order.
 *
 * The following equivalence relations on non-null `CPIInterfaceEthernet` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIInterfaceEthernet_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiInterfaceEthernet_Equals(x, y)` must return true if and only if
 *        `cpiInterfaceEthernet_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiInterfaceEthernet_Equals(x, y)` returns true and
 *        `cpiInterfaceEthernet_Equals(y, z)` returns true,
 *        then  `cpiInterfaceEthernet_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiInterfaceEthernet_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiInterfaceEthernet_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIInterfaceEthernet` instance.
 * @param b A pointer to a `CPIInterfaceEthernet` instance.
 * @return true if the two `CPIInterfaceEthernet` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIInterfaceEthernet *a = cpiInterfaceEthernet_Create();
 *    CPIInterfaceEthernet *b = cpiInterfaceEthernet_Create();
 *
 *    if (cpiInterfaceEthernet_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool cpiInterfaceEthernet_Equals(const CPIInterfaceEthernet *a, const CPIInterfaceEthernet *b);

/**
 * JSON representation
 *
 * <code>
 * { "ETHERNET" :
 * { "IFIDX" : ifidx,
 *   ["STATE" : "UP" | "DOWN", ]
 *   "ADDRS" : [ CPIAddress encodings ]
 * }
 * }
 * </code>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *cpiInterfaceEthernet_ToJson(const CPIInterfaceEthernet *ethernet);

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
CPIInterfaceEthernet *cpiInterfaceEthernet_CreateFromJson(PARCJSON *json);
#endif // libccnx_cpi_InterfaceEthernet_h
