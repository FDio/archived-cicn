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
 * @file metis+NumberList.h
 * @brief Stores a set of numbers.
 *
 * Useful for things like the reverse path of a PIT
 * or the forward paths of a FIB.  Does not allow duplicates.
 *
 */

#ifndef Metis_metis_NumberSet_h
#define Metis_metis_NumberSet_h

#include <stdlib.h>
#include <stdbool.h>

struct metis_number_set;
typedef struct metis_number_set MetisNumberSet;

typedef uint32_t MetisNumber;

/**
 * @function metisNumberList_Create
 * @abstract A new list of numbers
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisNumberSet *metisNumberSet_Create(void);

/**
 * Obtains a reference counted copy of the original
 *
 * The reference count is increased by one.  It must be released with MetisNumberSet_Release().
 *
 * @param [in] original An allocated MetisNumberSet
 *
 * @return non-null The reference counted copy
 *
 * Example:
 * @code
 * {
 *     MetisNumberSet *set = metisNumberSet_Create();
 *     MetisNumberSet *copy = metisNumberSet_Acquire(set);
 *     metisNumberSet_Release(&copy);
 *     metisNumberSet_Release(&set);
 * }
 * @endcode
 */
MetisNumberSet *metisNumberSet_Acquire(const MetisNumberSet *original);

/**
 * Releases one reference count and destroys the memory after last release
 *
 * The pointer will be NULLed after release regardless if the memory was destroyed.
 *
 * @param [in,out] setPtr A pointer to a MetisNumberSet.  Will be NULL'd after release.
 *
 * Example:
 * @code
 * {
 *     MetisNumberSet *set = metisNumberSet_Create();
 *     metisNumberSet_Release(&set);
 * }
 * @endcode
 */
void metisNumberSet_Release(MetisNumberSet **setPtr);

/**
 * @function metisNumberList_Append
 * @abstract Add a number to the end of the list
 * @discussion
 *   No check for duplicates is done
 *
 * @param <#param1#>
 * @return true if added, false if a duplicate
 */
bool metisNumberSet_Add(MetisNumberSet *set, MetisNumber number);

/**
 * @function metisNumberList_Length
 * @abstract The count of numbers in the list
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
size_t metisNumberSet_Length(const MetisNumberSet *set);

/**
 * @function metisNumberSet_GetItem
 * @abstract Retrieves an item based on the ordinal index
 * @discussion
 *   Will assert if the ordinalIndex is out of bounds.
 *
 * @param <#param1#>
 * @return the number
 */
MetisNumber metisNumberSet_GetItem(const MetisNumberSet *set, size_t ordinalIndex);

/**
 * @function metisNumberSet_Contains
 * @abstract Checks for set membership
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return true if the set contains the number, false otherwise
 */
bool metisNumberSet_Contains(const MetisNumberSet *set, MetisNumber number);

/**
 * @function metisNumberSet_AddSet
 * @abstract Adds one set to another set
 * @discussion
 *   Adds <code>setToAdd</code> to <code>destinationSet</code>
 *
 * @param <#param1#>
 * @return true if the set contains the number, false otherwise
 */
void metisNumberSet_AddSet(MetisNumberSet *destinationSet, const MetisNumberSet *setToAdd);

/**
 * @function metisNumberSet_Subtract
 * @abstract Computes set difference <code>difference = minuend - subtrahend</code>, returns a new number set.
 * @discussion
 *   <code>minuend</code> and <code>subtrahend</code> are not modified.  A new difference set is created.
 *
 *   Returns the elements in <code>minuend</code> that are not in <code>subtrahend</code>.
 *
 * @param minuend The set from which to subtract
 * @param subrahend The set begin removed from minuend
 * @return The set difference.  May be empty, but will not be NULL.
 */
MetisNumberSet *metisNumberSet_Subtract(const MetisNumberSet *minuend, const MetisNumberSet *subtrahend);

/**
 * Determine if two MetisNumberSet instances are equal.
 *
 * Two MetisNumberSet instances are equal if, and only if,
 *   they are the same size and contain the same elements.  Empty sets are equal.
 *   NULL equals NULL, but does not equal non-NULL.
 *
 * The following equivalence relations on non-null `MetisNumberSet` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `MetisNumberSet_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `metisNumberSet_Equals(x, y)` must return true if and only if
 *        `metisNumberSet_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `metisNumberSet_Equals(x, y)` returns true and
 *        `metisNumberSet_Equals(y, z)` returns true,
 *        then  `metisNumberSet_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `metisNumberSet_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `metisNumberSet_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `MetisNumberSet` instance.
 * @param b A pointer to a `MetisNumberSet` instance.
 * @return true if the two `MetisNumberSet` instances are equal.
 *
 * Example:
 * @code
 * {
 *    MetisNumberSet *a = metisNumberSet_Create();
 *    MetisNumberSet *b = metisNumberSet_Create();
 *
 *    if (metisNumberSet_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool metisNumberSet_Equals(const MetisNumberSet *a, const MetisNumberSet *b);

/**
 * @function metisNumberSet_Remove
 * @abstract Removes the number from the set
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisNumberSet_Remove(MetisNumberSet *set, MetisNumber number);
#endif // Metis_metis_NumberSet_h
