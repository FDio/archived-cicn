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
 * Used to identify a connection between a specific local address and
 * a specific remote address.
 */

#ifndef Metis_metis_AddressPair_h
#define Metis_metis_AddressPair_h

#include <ccnx/api/control/cpi_Address.h>

struct metis_address_pair;
typedef struct metis_address_pair MetisAddressPair;

/**
 * @function metisAddressPair_Create
 * @abstract Creates and address pair.  There is no restriction on the address types.
 * @discussion
 *   Creates an ordered pair of addresses, where the first is considered the "local" address
 *   and the second is the "remote" address.  Those designations are purely a convention used
 *   to name them, and does not imply any specifici types of operations.
 *
 *   The two addresses may be of any address types (e.g. IPv4, IPv6, Local, Ethernet).
 *   However, some functions that use an AddressPair may require that the local and remote
 *   addresses be the same type.
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisAddressPair *metisAddressPair_Create(const CPIAddress *local, const CPIAddress *remote);

/**
 * Returns a reference counted copy of the address pair
 *
 * Increments the reference count and returns the same address pair
 *
 * @param [in] addressPair An allocated address pair
 *
 * @retval non-null A reference counted copy
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisAddressPair *metisAddressPair_Acquire(const MetisAddressPair *addressPair);


/**
 * Releases a reference count to the object
 *
 * Decrements the reference count and destroys the object when it reaches 0.
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
void metisAddressPair_Release(MetisAddressPair **pairPtr);

/**
 * Determine if two MetisAddressPair instances are equal.
 *
 * Two MetisAddressPair instances are equal if, and only if, the local and remote addresses are identical.
 * Equality is determined by cpiAddress_Equals(a->local, b->local) and
 * cpiAdress_Equals(a->remote, b->remote).
 *
 * The following equivalence relations on non-null `MetisAddressPair` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `MetisAddressPair_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisAddressPair_Equals(x, y)` must return true if and only if
 *        `metisAddressPair_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisAddressPair_Equals(x, y)` returns true and
 *        `metisAddressPair_Equals(y, z)` returns true,
 *        then  `metisAddressPair_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisAddressPair_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `metisAddressPair_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `MetisAddressPair` instance.
 * @param b A pointer to a `MetisAddressPair` instance.
 * @return true if the two `MetisAddressPair` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisAddressPair *a = metisAddressPair_Create();
 *    MetisAddressPair *b = metisAddressPair_Create();
 *
 *    if (metisAddressPair_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool metisAddressPair_Equals(const MetisAddressPair *a, const MetisAddressPair *b);

/**
 * @function metisAddressPair_EqualsAddresses
 * @abstract As MetisAddressEquals, but "b" is broken out
 * @discussion
 *   Equality is determined by cpiAddress_Equals(a->local, local) and
 *   cpiAdress_Equals(a->remote, remote).
 *
 * @param <#param1#>
 * @return <#return#>
 */
bool metisAddressPair_EqualsAddresses(const MetisAddressPair *a, const CPIAddress *local, const CPIAddress *remote);

const CPIAddress *metisAddressPair_GetLocal(const MetisAddressPair *pair);
const CPIAddress *metisAddressPair_GetRemote(const MetisAddressPair *pair);

/**
 * @function metisAddressPair_HashCode
 * @abstract Hash useful for tables.  Consistent with Equals.
 * @discussion
 *   Returns a non-cryptographic hash that is consistent with equals.  That is,
 *   if a == b, then hash(a) == hash(b).
 *
 * @param <#param1#>
 * @return <#return#>
 */
PARCHashCode metisAddressPair_HashCode(const MetisAddressPair *pair);

/**
 * @function metisAddressPair_ToString
 * @abstract Human readable string representation.  Caller must use free(3).
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
char *metisAddressPair_ToString(const MetisAddressPair *pair);
#endif // Metis_metis_AddressPair_h
