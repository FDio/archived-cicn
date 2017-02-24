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
#include <sys/queue.h>

#include <ccnx/forwarder/metis/content_store/metis_LruList.h>
#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

struct metis_lru_list_entry {
    void *userData;

    // always set to the list
    MetisLruList *parentList;

    // indicates if the Entry is currently in the list
    bool inList;

    TAILQ_ENTRY(metis_lru_list_entry)  list;
};

// this defines the TAILQ structure so we can access the tail pointer
TAILQ_HEAD(metis_lru_s, metis_lru_list_entry);

struct metis_lru_list {
    struct metis_lru_s head;
    size_t itemsInList;
};

void
metisLruList_EntryDestroy(MetisLruListEntry **entryPtr)
{
    assertNotNull(entryPtr, "Parameter entryPtr must be non-null double pointer");

    MetisLruListEntry *entry = *entryPtr;
    if (entry->inList) {
        TAILQ_REMOVE(&entry->parentList->head, entry, list);
        assertTrue(entry->parentList->itemsInList > 0, "Invalid state, removed entry from list, but itemsInList is 0");
        entry->parentList->itemsInList--;
    }

    parcMemory_Deallocate((void **) &entry);
    *entryPtr = NULL;
}

void
metisLruList_EntryMoveToHead(MetisLruListEntry *entry)
{
    assertNotNull(entry, "Parameter entry must be non-null");

    TAILQ_REMOVE(&entry->parentList->head, entry, list);
    TAILQ_INSERT_HEAD(&entry->parentList->head, entry, list);
}

void *
metisLruList_EntryGetData(MetisLruListEntry *entry)
{
    return entry->userData;
}

MetisLruList *
metisLruList_Create()
{
    MetisLruList *list = parcMemory_AllocateAndClear(sizeof(MetisLruList));
    assertNotNull(list, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisLruList));
    list->itemsInList = 0;
    TAILQ_INIT(&list->head);
    return list;
}

void
metisLruList_Destroy(MetisLruList **lruPtr)
{
    assertNotNull(lruPtr, "Parameter lruPtr must be non-null double pointer");

    MetisLruList *lru = *lruPtr;

    MetisLruListEntry *entry = TAILQ_FIRST(&lru->head);
    while (entry != NULL) {
        MetisLruListEntry *next = TAILQ_NEXT(entry, list);
        metisLruList_EntryDestroy(&entry);
        entry = next;
    }

    parcMemory_Deallocate((void **) &lru);
    *lruPtr = NULL;
}

MetisLruListEntry *
metisLruList_NewHeadEntry(MetisLruList *lru, void *data)
{
    assertNotNull(lru, "Parameter lru must be non-null");
    assertNotNull(data, "Parameter data must be non-null");

    MetisLruListEntry *entry = parcMemory_AllocateAndClear(sizeof(MetisLruListEntry));
    assertNotNull(entry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisLruListEntry));
    entry->userData = data;
    entry->parentList = lru;
    entry->inList = true;

    TAILQ_INSERT_HEAD(&lru->head, entry, list);
    lru->itemsInList++;

    return entry;
}

MetisLruListEntry *
metisLruList_PopTail(MetisLruList *lru)
{
    assertNotNull(lru, "Parameter lru must be non-null");

    MetisLruListEntry *entry = TAILQ_LAST(&lru->head, metis_lru_s);

    if (entry) {
        assertTrue(lru->itemsInList > 0, "Invalid state, removed entry from list, but itemsInList is 0");
        lru->itemsInList--;
        TAILQ_REMOVE(&lru->head, entry, list);
        entry->inList = false;
    }

    return entry;
}

size_t
metisLruList_Length(const MetisLruList *lru)
{
    assertNotNull(lru, "Parameter lru must be non-null");
    return lru->itemsInList;
}
