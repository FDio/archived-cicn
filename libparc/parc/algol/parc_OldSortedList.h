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
 * @file parc_SortedList.h
 * @ingroup datastructures
 * @brief A sorted list
 *
 */
#ifndef libparc_parc_SortedList_h
#define libparc_parc_SortedList_h

#include <stdlib.h>

struct parc_sorted_list;

typedef struct parc_sorted_list PARCSortedList;

/**
 * This is a compare function that must be defined for the sorted list to sort
 *
 * @param [in] object1 The first object to compare.
 * @param [in] object2 The second object to compare.
 * @return -1 if object1 < object2,  0 if object1 == object2, 1 if object1 > object2.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef int (*parcSortedList_Compare)(const void *object1, const void *object2);

/**
 * Create a sorted list
 * This list will be sorted from smallest to largest based on the compare function.
 *
 * @param [in] compareFunction A compare function to determine how elements are sorted.
 * @return An allocated `PARCSortedList`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSortedList *parcSortedList_Create(parcSortedList_Compare compareFunction);

/**
 * Destroy an allocated sorted list
 *
 * @param [in,out] parcSortedListPointer A pointer to the allocated sorted list to destroy. The pointer will be set to NULL.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcSortedList_Destroy(PARCSortedList **parcSortedListPointer);

/**
 * Add an element to the sorted list.
 *
 * @param [in,out] parcSortedList A pointer to the `PARCSortedList` to modify.
 * @param newItem The new item to add to the list. This item must be comparable by the compare function.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcSortedList_Add(PARCSortedList *parcSortedList, void *newItem);

/**
 * Return the length of the list
 *
 * @param [in] parcSortedList a pointer to an allocated sorted list.
 * @return return the length of the list, the number of elements.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcSortedList_Length(PARCSortedList *parcSortedList);

/**
 * Pop the first element on the  sorted list. This will remove the element from the list and return it to the caller.
 *
 * @param [in,out] parcSortedList A pointer to the `PARCSortedList` to modify.
 * @return The first element of @p parcSortedList.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcSortedList_PopFirst(PARCSortedList *parcSortedList);

/**
 * Get the first element on the  sorted list. This will NOT remove the element from the list.
 *
 * @param [in] parcSortedList A pointer to the `PARCSortedList` .
 * @return The first element of the sorted list.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcSortedList_GetFirst(PARCSortedList *parcSortedList);
#endif // libparc_parc_SortedList_h
