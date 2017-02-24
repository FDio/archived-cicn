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

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>

#include <ccnx/forwarder/metis/core/metis_ConnectionList.h>
#include <LongBow/runtime.h>

struct metis_connection_list {
    PARCArrayList *listOfConnections;
};

/**
 * PARCArrayList entry destroyer
 */

static void
metisConnectionList_ArrayDestroyer(void **voidPtr)
{
    MetisConnection **entryPtr = (MetisConnection **) voidPtr;
    metisConnection_Release(entryPtr);
}

MetisConnectionList *
metisConnectionList_Create()
{
    MetisConnectionList *list = parcMemory_AllocateAndClear(sizeof(MetisConnectionList));
    assertNotNull(list, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisConnectionList));
    list->listOfConnections = parcArrayList_Create(metisConnectionList_ArrayDestroyer);
    return list;
}

void
metisConnectionList_Destroy(MetisConnectionList **listPtr)
{
    assertNotNull(listPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listPtr, "Parameter must dereference to non-null pointer");
    MetisConnectionList *list = *listPtr;
    parcArrayList_Destroy(&list->listOfConnections);
    parcMemory_Deallocate((void **) &list);
    *listPtr = NULL;
}

void
metisConnectionList_Append(MetisConnectionList *list, MetisConnection *entry)
{
    assertNotNull(list, "Parameter list must be non-null");
    assertNotNull(entry, "Parameter entry must be non-null");

    parcArrayList_Add(list->listOfConnections, metisConnection_Acquire(entry));
}

size_t
metisConnectionList_Length(const MetisConnectionList *list)
{
    assertNotNull(list, "Parameter list must be non-null");
    return parcArrayList_Size(list->listOfConnections);
}

MetisConnection *
metisConnectionList_Get(MetisConnectionList *list, size_t index)
{
    assertNotNull(list, "Parameter list must be non-null");
    MetisConnection *original = (MetisConnection *) parcArrayList_Get(list->listOfConnections, index);
    return original;
}
