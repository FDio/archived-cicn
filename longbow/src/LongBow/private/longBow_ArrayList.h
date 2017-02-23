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
 * @file longBow_ArrayList.h
 * @ingroup internals
 * @brief A simple, list implementation using a dynamic array.
 *
 */
#ifndef LongBow_ARRAYLIST_H
#define LongBow_ARRAYLIST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @struct longbow_array_list
 * @brief  A LongBow_ArrayList is a (dynamic) array of <code>void *</code> pointers;
 */
struct longbow_array_list;

/**
 * @typedef LongBowArrayList
 * @brief The struct longbow_array_list
 */
typedef struct longbow_array_list LongBowArrayList;

/**
 * Assert that a LongBowArrayList instance is valid.
 *
 * @param [in] array A pointer to a valid LongBowArrayList instance.
 *
 */
void longBowArrayList_AssertValid(const LongBowArrayList *array);
/**
 * Add a pointer to an element to the given LongBowArrayList.
 *
 * If the list was constructed with a destroyer,
 * the pointer will be destroyed when element is removed or the list is destroyed.
 *
 * @param [in] array A pointer to a LongBowArrayList instance.
 * @param [in] pointer An arbitrary value to store.
 *
 * @return The input array pointer.
 */
LongBowArrayList *longBowArrayList_Add(LongBowArrayList *array, const void *pointer);

/**
 * Remove an element at a specific index from an Array List.
 *
 * The element is destroyed via the function provided when calling <code>longBowArrayList_Create</code>.
 *
 * @param [in] array A pointer to a LongBowArrayList instance.
 * @param [in] index The index of the element to remove.
 *
 * @return A pointer to the modified LongBowArrayList.
 */
LongBowArrayList *longBowArrayList_RemoveAtIndex(LongBowArrayList *array, size_t index);

/**
 * Add an element at the index location. Elements will be moved up if required.
 * If the index is higher than the current Length the Array will be grown to that size
 *
 * @param [in] array A pointer to a LongBowArrayList instance.
 * @param [in] pointer An arbitrary value to store.
 * @param [in] index The position that the value will be stored after.
 * @return A pointer to the modified LongBowArrayList.
 */
LongBowArrayList *longBowArrayList_Add_AtIndex(LongBowArrayList *array, const void *pointer, size_t index);

/**
 * Create an instance of an empty LongBowArrayList.
 *
 * @param [in] destroyElement
 *      A pointer to a function that will destroy (or equivalent) the element pointed to by <code>element</code>
 * @return A pointer to a LongBowArrayList instance, or NULL if no memory could be allocated.
 */
LongBowArrayList *longBowArrayList_Create(void (*destroyElement)(void **elementAddress));

/**
 * Create an instance of a LongBowArrayList pre-provisioned to contain the specified number of elements.
 *
 * @param [in] size
 *      The number of initial elements to provision for.
 * @param [in] destroyElement
 *      A pointer to a function that will destroy (or equivalent) the element pointed to by <code>element</code>
 * @return A pointer to a LongBowArrayList instance, or NULL if no memory could be allocated.
 */
LongBowArrayList *longBowArrayList_Create_Capacity(void (*destroyElement)(void **elementAddress), size_t size);

/**
 * Get an array of void * pointers.
 * Return a pointer to an array of void * pointers contained in this Array List.
 * The returned value may be the actual backing array for the Array List.
 *
 * @param [in] array The LongBow_ArrayList
 * @return A pointer to an array of void * pointers contained in this Array List.
 *
 */
void **longBowArrayList_GetArray(const LongBowArrayList *array);

/**
 * Copy a LongBowArrayList instance.
 * Create a new LongBowArrayList instance with the same structure and content as the original.
 *
 * @param [in] array A pointer to a LongBowArrayList instance to copy.
 * @return A pointer to a LongBowArrayList instance with a copy of the original, or NULL if no memory could be allocated.
 */
LongBowArrayList *longBowArrayList_Copy(const LongBowArrayList *array);

/**
 * Destroy a LongBowArrayList instance.
 *
 * Destroy the given LongBowArrayList by freeing all memory used by it.
 *
 * @param [in,out] arrayPtr A pointer to a LongBowArrayList pointer.
 */
void longBowArrayList_Destroy(LongBowArrayList **arrayPtr);

/**
 * Get an element from the given list at a specified index.
 * The index must be 0 <= index < length.
 *
 * @return A pointer (void *) to the element in the list.
 *
 * @param [in] array A pointer to a LongBowArrayList instance.
 * @param [in] index The index of the required element.
 */
void *longBowArrayList_Get(const LongBowArrayList *array, size_t index);

/**
 * Return the number of elements in the given LongBowArrayList.
 *
 * @param [in] array A pointer to a LongBowArrayList instance.
 * @return A size_t of the number of elements in the given LongBowArrayList.
 */
size_t longBowArrayList_Length(const LongBowArrayList *array);


/**
 * Determine if two LongBowArrayList instances are equal.
 *
 * Two LongBowArrayList instances are equal if, and only if, they both contain the same pointers in the same order.
 *
 * The following equivalence relations on non-null `LongBowArrayList` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `LongBowArrayList_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `longBowArrayList_Equals(x, y)` must return true if and only if
 *        `longBowArrayList_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `longBowArrayList_Equals(x, y)` returns true and
 *        `longBowArrayList_Equals(y, z)` returns true,
 *        then  `longBowArrayList_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `longBowArrayList_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `longBowArrayList_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `LongBowArrayList` instance.
 * @param b A pointer to a `LongBowArrayList` instance.
 * @return true if the two `LongBowArrayList` instances are equal.
 *
 * Example:
 * @code
 * {
 *    LongBowArrayList *a = longBowArrayList_Create();
 *    LongBowArrayList *b = longBowArrayList_Create();
 *
 *    if (longBowArrayList_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */

bool longBowArrayList_Equals(const LongBowArrayList *a, const LongBowArrayList *b);

/**
 * Standard library free(3) wrapper for use as a destructor function for elements of a LongBowArrayList.
 *
 * The create functions for LongBowArrayList have an input parameter that is a pointer to a function that
 * will be called for each element when the Array List is destroyed, and when an element is removed via longBowArrayList_RemoveAtIndex.
 * This destroy function has a different calling signature than the standard library's free(3) function.
 * This function is a wrapper providing the correct facade for the standard library free(3) function.
 *
 * @param [in,out] element A pointer to the pointer to an element to be destroyed.
 */
void longBowArrayList_StdlibFreeFunction(void **element);

/**
 * Replace the first occurance of an existing element in the given LongBowArrayList.
 *
 * Paragraphs Of Explanation
 *
 * @param [in] array A pointer to a LongBowArrayList instance.
 * @param [in] old A pointer to an element in the list to replace.
 * @param [in] new A pointer to an element that will replace old.
 *
 * @return true If the element was found and replaced.
 * @return false If the element was not found.
 */
bool longBowArrayList_Replace(LongBowArrayList *array, const void *old, void *new);
#endif // LongBow_ARRAYLIST_H
