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
#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/content_store/metis_TimeOrderedList.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_TreeRedBlack.h>

/**
 * A list of MetisContentStoreEntrys, kept in sorted order by time. The ordering is calculated by a
 * key compare function (e.g. {@link MetisTimeOrderList_KeyCompare}), passed in.
 *
 * This container does not hold references to the objects that it contains. In other words, it does not Acquire()
 * the MetisMessages that are placed in it. That reference count is managed by the owning ContentStore. This is
 * purely an index, and provides an easy to way index MetisMessages based on a specified time value. Typically,
 * that would be their Recommended Cache Time, or Expiration Time.
 *
 * It maintains a tree, sorted by the time values passed in to the Add() function. It does not manage capacity,
 * and can grow uncontrollably if the owning ContentStore does not manage it. Items are indexed first by time, then
 * address of the MetisMessage (just as a distringuishing attribute). This allows us to store multiple items with
 * the same expiration time.
 */

struct metis_timeordered_list {
    PARCTreeRedBlack *timeOrderedTree;
};

static void
_finalRelease(MetisTimeOrderedList **listP)
{
    MetisTimeOrderedList *list = *listP;
    parcTreeRedBlack_Destroy(&list->timeOrderedTree);
}

parcObject_ExtendPARCObject(MetisTimeOrderedList, _finalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(metisTimeOrderedList, MetisTimeOrderedList);

parcObject_ImplementRelease(metisTimeOrderedList, MetisTimeOrderedList);


MetisTimeOrderedList *
metisTimeOrderedList_Create(MetisTimeOrderList_KeyCompare *keyCompareFunction)
{
    MetisTimeOrderedList *result = parcObject_CreateInstance(MetisTimeOrderedList);
    if (NULL != result) {
        result->timeOrderedTree = parcTreeRedBlack_Create(keyCompareFunction, // keyCompare
                                                          NULL,               // keyFree
                                                          NULL,               // keyCopy
                                                          NULL,               // valueEquals
                                                          NULL,               // valueFree
                                                          NULL);              // valueCopy
    }
    return result;
}

void
metisTimeOrderedList_Add(MetisTimeOrderedList *list, MetisContentStoreEntry *entry)
{
    parcTreeRedBlack_Insert(list->timeOrderedTree, entry, entry);
}

MetisContentStoreEntry *
metisTimeOrderedList_GetOldest(MetisTimeOrderedList *list)
{
    return parcTreeRedBlack_FirstKey(list->timeOrderedTree);
}

bool
metisTimeOrderedList_Remove(MetisTimeOrderedList *list, MetisContentStoreEntry *storeEntry)
{
    bool result = false;

    MetisContentStoreEntry *entry = (MetisContentStoreEntry *) parcTreeRedBlack_Remove(list->timeOrderedTree, storeEntry);
    if (entry != NULL) {
        result = true;
    }
    return result;
}

size_t
metisTimeOrderedList_Length(MetisTimeOrderedList *list)
{
    return (size_t) parcTreeRedBlack_Size(list->timeOrderedTree);
}


