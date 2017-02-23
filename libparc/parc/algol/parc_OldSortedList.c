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
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_SortedList.h>

#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Memory.h>

struct parc_sorted_list {
    parcSortedList_Compare compare;
    PARCArrayList *arrayList;
};

PARCSortedList *
parcSortedList_Create(parcSortedList_Compare compareFunction)
{
    PARCSortedList *sortedList = parcMemory_Allocate(sizeof(PARCSortedList));
    assertNotNull(sortedList, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCSortedList));
    sortedList->arrayList = parcArrayList_Create(NULL);
    sortedList->compare = compareFunction;
    return sortedList;
}

void
parcSortedList_Destroy(PARCSortedList **parcSortedListPointer)
{
    parcArrayList_Destroy(&((*parcSortedListPointer)->arrayList));
    parcMemory_Deallocate((void **) parcSortedListPointer);
    *parcSortedListPointer = NULL;
}

void
parcSortedList_Add(PARCSortedList *parcSortedList, void *newItem)
{
    assertNotNull(parcSortedList, "sortedList parameter can't be null");
    assertNotNull(parcSortedList->arrayList, "arrayList can't be null");
    assertNotNull(newItem, "newItem can't be null");

    size_t total_items = parcArrayList_Size(parcSortedList->arrayList);
    for (size_t i = 0; i < total_items; i++) {
        void *oldItem = parcArrayList_Get(parcSortedList->arrayList, i);
        if (parcSortedList->compare(newItem, oldItem) == -1) {
            // The old item at position i is bigger than the new item,
            // we must insert the newItem here.
            parcArrayList_InsertAtIndex(parcSortedList->arrayList, newItem, i);
            return;
        }
    }
    // We reached the end of the list, it must go here...
    parcArrayList_Add(parcSortedList->arrayList, newItem);
}

size_t
parcSortedList_Length(PARCSortedList *parcSortedList)
{
    return parcArrayList_Size(parcSortedList->arrayList);
}

void *
parcSortedList_PopFirst(PARCSortedList *parcSortedList)
{
    assertNotNull(parcSortedList, "sortedList parameter can't be null");
    assertNotNull(parcSortedList->arrayList, "arrayList can't be null");

    if (parcArrayList_Size(parcSortedList->arrayList) == 0) {
        return NULL;
    }
    void *item = parcArrayList_Get(parcSortedList->arrayList, 0);
    parcArrayList_RemoveAndDestroyAtIndex(parcSortedList->arrayList, 0);
    return item;
}

void *
parcSortedList_GetFirst(PARCSortedList *parcSortedList)
{
    assertNotNull(parcSortedList, "sortedList parameter can't be null");
    assertNotNull(parcSortedList->arrayList, "arrayList can't be null");

    if (parcArrayList_Size(parcSortedList->arrayList) == 0) {
        return NULL;
    }
    return parcArrayList_Get(parcSortedList->arrayList, 0);
}
