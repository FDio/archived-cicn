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


#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Memory.h>
#include <ccnx/forwarder/metis/processor/metis_FibEntryList.h>
#include <LongBow/runtime.h>

struct metis_fib_entry_list {
    PARCArrayList *listOfFibEntries;
};

static void
metisFibEntryList_ListDestroyer(void **voidPtr)
{
    MetisFibEntry **entryPtr = (MetisFibEntry **) voidPtr;
    metisFibEntry_Release(entryPtr);
}

MetisFibEntryList *
metisFibEntryList_Create()
{
    MetisFibEntryList *fibEntryList = parcMemory_AllocateAndClear(sizeof(MetisFibEntryList));
    assertNotNull(fibEntryList, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisFibEntryList));
    fibEntryList->listOfFibEntries = parcArrayList_Create(metisFibEntryList_ListDestroyer);
    return fibEntryList;
}

void
metisFibEntryList_Destroy(MetisFibEntryList **listPtr)
{
    assertNotNull(listPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listPtr, "Parameter must dereference to non-null pointer");

    MetisFibEntryList *list = *listPtr;
    parcArrayList_Destroy(&list->listOfFibEntries);
    parcMemory_Deallocate((void **) &list);
    listPtr = NULL;
}

void
metisFibEntryList_Append(MetisFibEntryList *list, MetisFibEntry *fibEntry)
{
    assertNotNull(list, "Parameter list must be non-null pointer");
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null pointer");

    MetisFibEntry *copy = metisFibEntry_Acquire(fibEntry);
    parcArrayList_Add(list->listOfFibEntries, copy);
}

size_t
metisFibEntryList_Length(const MetisFibEntryList *list)
{
    assertNotNull(list, "Parameter list must be non-null pointer");
    return parcArrayList_Size(list->listOfFibEntries);
}


const MetisFibEntry *
metisFibEntryList_Get(const MetisFibEntryList *list, size_t index)
{
    assertNotNull(list, "Parameter list must be non-null pointer");
    MetisFibEntry *entry = parcArrayList_Get(list->listOfFibEntries, index);
    return entry;
}
