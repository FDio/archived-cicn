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
 * @file metis_TlvExtent.h
 * @brief Defines the extent structure used to map a TLV packet
 *
 * In MetisTlvSkeleton, all the pertinent fields used by Metis are stored by their extent range in the
 * received packet buffer.  An extent is (offset, length).
 *
 */
#ifndef Metis_metis_tlv_Extent_h
#define Metis_metis_tlv_Extent_h

#include <stdint.h>
#include <stdbool.h>

/**
 * Stores the location of a field within a buffer.
 * The values are usually in host byte order.
 */
typedef struct metis_tlv_extent {
    uint16_t offset;
    uint16_t length;
} MetisTlvExtent;

/**
 * Used to detect a "not found" or "not present" condition.
 * Equal to { 0x0, 0x0 }, which is an invalid extent value.
 */
extern const MetisTlvExtent metisTlvExtent_NotFound;


/**
 * Determine if two MetisTlvExtent instances are equal.
 *
 * The following equivalence relations on non-null `MetisTlvExtent` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `MetisTlvExtent_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisTlvExtent_Equals(x, y)` must return true if and only if
 *        `metisTlvExtent_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisTlvExtent_Equals(x, y)` returns true and
 *        `metisTlvExtent_Equals(y, z)` returns true,
 *        then  `metisTlvExtent_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisTlvExtent_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `metisTlvExtent_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `MetisTlvExtent` instance.
 * @param b A pointer to a `MetisTlvExtent` instance.
 * @return true if the two `MetisTlvExtent` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisTlvExtent *a = { .offset = 5, .length = 7 };
 *    MetisTlvExtent *b = { .offset = 5, .length = 8 };
 *
 *    if (metisTlvExtent_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false (this is the block executed)
 *    }
 * }
 * @endcode
 */
bool metisTlvExtent_Equals(const MetisTlvExtent *a, const MetisTlvExtent *b);
#endif // Metis_metis_tlv_Extent_h
