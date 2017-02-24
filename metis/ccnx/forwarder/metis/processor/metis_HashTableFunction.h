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
 * @file metis_HashTableFunction.h
 * @brief These functions are used in PARCHashCodeTables by the
 * MatchingRulesTable and ContentStore and PIT. They perform the equality
 * and has generation needed by the PARCHashCodeTable.
 *
 */
#ifndef Metis_metis_HashTableFunction_h
#define Metis_metis_HashTableFunction_h

#include <parc/algol/parc_HashCodeTable.h>

// ==========================================================
// These functions operate on a MetisMessage as the key in the HashTable.
// The functions use void * rather than MetisMessage instances in the function
// signature because it is using generic has code tables from PARC Library

/**
 * Determine if the Names of two `MetisMessage` instances are equal.
 *
 * The following equivalence relations on non-null `MetisMessage` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `metisHashTableFunction_MessageNameEquals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `MetisMessage_Equals(x, y)` must return true if and only if
 *        `metisHashTableFunction_MessageNameEquals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisHashTableFunction_MessageNameEquals(x, y)` returns true and
 *        `metisHashTableFunction_MessageNameEquals(y, z)` returns true,
 *        then  `metisHashTableFunction_MessageNameEquals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisHashTableFunction_MessageNameEquals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `metisHashTableFunction_MessageNameEquals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `MetisMessage` instance.
 * @param b A pointer to a `MetisMessage` instance.
 * @return true if the names of the two `MetisMessage` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisMessage *a = MetisMessage_Create();
 *    MetisMessage *b = MetisMessage_Create();
 *
 *    if (metisHashTableFunction_MessageNameEquals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool metisHashTableFunction_MessageNameEquals(const void *metisMessageA, const void *metisMessageB);

/**
 * @function hashTableFunction_NameHashCode
 * @abstract Computes the hash of the entire name in a MetisMessage
 * @discussion
 *   <#Discussion#>
 *
 * @param metisMessageA is a MetisMessage
 * @return A non-cryptographic hash of Name
 */
HashCodeType metisHashTableFunction_MessageNameHashCode(const void *metisMessageA);

/**
 * Determine if the Names and KeyIds of two MetisMessage instances are equal.
 *
 *
 * The following equivalence relations on non-null `MetisMessage` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `metisHashTableFunction_MessageNameAndKeyIdEquals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisHashTableFunction_MessageNameAndKeyIdEquals(x, y)` must return true if and only if
 *        `metisHashTableFunction_MessageNameAndKeyIdEquals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisHashTableFunction_MessageNameAndKeyIdEquals(x, y)` returns true and
 *        `metisHashTableFunction_MessageNameAndKeyIdEquals(y, z)` returns true,
 *        then  `metisHashTableFunction_MessageNameAndKeyIdEquals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisHashTableFunction_MessageNameAndKeyIdEquals(x, y)` consistently
 *      return true or consistently return false.
 *
 *  * For any non-null reference value x, `metisHashTableFunction_MessageNameAndKeyIdEquals(x, NULL)`
 *      must return false.
 *
 * @param a A pointer to a `MetisMessage` instance.
 * @param b A pointer to a `MetisMessage` instance.
 * @return true if the Name and KeyId tuple of the two `MetisMessage` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisMessage *a = MetisMessage_Create();
 *    MetisMessage *b = MetisMessage_Create();
 *
 *    if (metisHashTableFunction_MessageNameAndKeyIdEquals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */

bool metisHashTableFunction_MessageNameAndKeyIdEquals(const void *metisMessageA, const void *metisMessageB);

/**
 * @function hashTableFunction_NameAndKeyIdHashCode
 * @abstract Generates a hash code on the tuple (Name, KeyId)
 * @discussion
 *   <#Discussion#>
 *
 * @param metisMessageA is a MetisMessage
 * @return A non-cryptographic hash of (Name, KeyId)
 */
HashCodeType metisHashTableFunction_MessageNameAndKeyIdHashCode(const void *metisMessageA);

/**
 * Determine if the (Name, ContentObjectHash) tuple of two `MetisMessage` instances are equal.
 *
 * The following equivalence relations on non-null `MetisMessage` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `metisHashTableFunction_MessageNameAndObjectHashEquals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisHashTableFunction_MessageNameAndObjectHashEquals(x, y)` must return true if and only if
 *        `metisHashTableFunction_MessageNameAndObjectHashEquals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisHashTableFunction_MessageNameAndObjectHashEquals(x, y)` returns true and
 *        `metisHashTableFunction_MessageNameAndObjectHashEquals(y, z)` returns true,
 *        then  `metisHashTableFunction_MessageNameAndObjectHashEquals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisHashTableFunction_MessageNameAndObjectHashEquals(x, y)` consistently
 *      return true or consistently return false.
 *
 *  * For any non-null reference value x, `metisHashTableFunction_MessageNameAndObjectHashEquals(x, NULL)`
 *      must return false.
 *
 * @param a A pointer to a `MetisMessage` instance.
 * @param b A pointer to a `MetisMessage` instance.
 * @return true if the (Name, ContentObjectHash)tuple of the two `MetisMessage` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisMessage *a = MetisMessage_Create();
 *    MetisMessage *b = MetisMessage_Create();
 *
 *    if (metisHashTableFunction_MessageNameAndObjectHashEquals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool metisHashTableFunction_MessageNameAndObjectHashEquals(const void *metisMessageA, const void *metisMessageB);

/**
 * @function hashTableFunction_NameAndObjectHashHashCode
 * @abstract <#OneLineDescription#>
 * @discussion
 *   <#Discussion#>
 *
 * @param metisMessageA is a MetisMessage
 * @return A non-cryptographic hash of (Name, ContentObjectHash)
 */
HashCodeType metisHashTableFunction_MessageNameAndObjectHashHashCode(const void *metisMessageA);

// ==========================================================
// These functions operate on a MetisTlvName as the key of the hash table

/**
 * Determine if two `MetisTlvName` instances in the keys of the hash table are equal.
 *
 * The following equivalence relations on non-null `MetisTlvName` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `metisHashTableFunction_TlvNameEquals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisHashTableFunction_TlvNameEquals(x, y)` must return true if and only if
 *        `metisHashTableFunction_TlvNameEquals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisHashTableFunction_TlvNameEquals(x, y)` returns true and
 *        `metisHashTableFunction_TlvNameEquals(y, z)` returns true,
 *        then  `metisHashTableFunction_TlvNameEquals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisHashTableFunction_TlvNameEquals(x, y)` consistently
 *      return true or consistently return false.
 *
 *  * For any non-null reference value x, `metisHashTableFunction_TlvNameEquals(x, NULL)`
 *      must return false.
 *
 * @param a A pointer to a `MetisTlvName` instance.
 * @param b A pointer to a `MetisTlvName` instance.
 * @return true if the two `MetisTlvName` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisTlvName *a = metisTlvName_Create();
 *    MetisTlvName *b = metisTlvName_Create();
 *
 *    if (metisHashTableFunction_TlvNameEquals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool metisHashTableFunction_TlvNameEquals(const void *metisTlvNameA, const void *metisTlvNameB);

/**
 * @function hashTableFunction_TlvNameCompare
 * @abstract The key is a MetisTlvName.  Returns the order comparison of two names.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return A < B -> -1, A = B -> 0, A > B -> +1
 */
int metisHashTableFunction_TlvNameCompare(const void *keyA, const void *keyB);

/**
 * @function hashTableFunction_TlvNameHashCode
 * @abstract Computes the hash of the entire name in a MetisTlvName
 * @discussion
 *   <#Discussion#>
 *
 * @param keyA is a MetisTlvName
 * @return A non-cryptographic hash of Name
 */
HashCodeType metisHashTableFunction_TlvNameHashCode(const void *keyA);
#endif // Metis_metis_HashTableFunction_h
