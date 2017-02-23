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
#include <ccnx/forwarder/metis/content_store/metis_ContentStoreEntry.h>

#include <LongBow/runtime.h>

const uint64_t metisContentStoreEntry_MaxExpiryTime = UINT64_MAX;
const uint64_t metisContentStoreEntry_MaxRecommendedCacheTime = UINT64_MAX;

struct metis_contentstore_entry {
    MetisMessage *message;
    MetisLruListEntry *lruEntry;
    unsigned refcount;

    bool hasRecommendedCacheTimeTicks;
    uint64_t recommendedCacheTimeTicks;

    bool hasExpiryTimeTicks;
    uint64_t expiryTimeTicks;
};

MetisContentStoreEntry *
metisContentStoreEntry_Create(MetisMessage *contentMessage, MetisLruList *lruList)
{
    assertNotNull(contentMessage, "Parameter objectMessage must be non-null");

    MetisContentStoreEntry *entry = parcMemory_AllocateAndClear(sizeof(MetisContentStoreEntry));
    assertNotNull(entry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisContentStoreEntry));
    entry->message = metisMessage_Acquire(contentMessage);
    entry->refcount = 1;
    if (lruList != NULL) {
        entry->lruEntry = metisLruList_NewHeadEntry(lruList, entry);
    }

    entry->hasExpiryTimeTicks = metisMessage_HasExpiryTime(contentMessage);
    entry->hasRecommendedCacheTimeTicks = metisMessage_HasRecommendedCacheTime(contentMessage);

    if (entry->hasExpiryTimeTicks) {
        entry->expiryTimeTicks = metisMessage_GetExpiryTimeTicks(contentMessage);
    }

    if (entry->hasRecommendedCacheTimeTicks) {
        entry->recommendedCacheTimeTicks = metisMessage_GetRecommendedCacheTimeTicks(contentMessage);
    }

    return entry;
}

MetisContentStoreEntry *
metisContentStoreEntry_Acquire(const MetisContentStoreEntry *original)
{
    assertNotNull(original, "Parameter must be non-null");
    ((MetisContentStoreEntry *) original)->refcount++;  // cast to break the const.
    return (MetisContentStoreEntry *) original;
}

void
metisContentStoreEntry_Release(MetisContentStoreEntry **entryPtr)
{
    assertNotNull(entryPtr, "Parameter must be non-null double pointer");
    assertNotNull(*entryPtr, "Parameter must dereference to non-null pointer");

    MetisContentStoreEntry *entry = *entryPtr;
    assertTrue(entry->refcount > 0, "Illegal state: has refcount of 0");

    entry->refcount--;
    if (entry->refcount == 0) {
        if (entry->lruEntry) {
            metisLruList_EntryDestroy(&entry->lruEntry);
        }
        metisMessage_Release(&entry->message);
        parcMemory_Deallocate((void **) &entry);
    }
    *entryPtr = NULL;
}

MetisMessage *
metisContentStoreEntry_GetMessage(const MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    return storeEntry->message;
}

bool
metisContentStoreEntry_HasExpiryTimeTicks(const MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    return storeEntry->hasExpiryTimeTicks;
}

uint64_t
metisContentStoreEntry_GetExpiryTimeTicks(const MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    assertTrue(storeEntry->hasExpiryTimeTicks,
               "storeEntry has no ExpiryTimeTicks. Did you call metisContentStoreEntry_HasExpiryTimeTicks() first?");
    return storeEntry->expiryTimeTicks;
}

bool
metisContentStoreEntry_HasRecommendedCacheTimeTicks(const MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    return storeEntry->hasRecommendedCacheTimeTicks;
}

uint64_t
metisContentStoreEntry_GetRecommendedCacheTimeTicks(const MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    assertTrue(storeEntry->hasRecommendedCacheTimeTicks,
               "storeEntry has no RecommendedCacheTimeTicks. Did you call metisContentStoreEntry_HasRecommendedCacheTimeTicks() first?");
    return storeEntry->recommendedCacheTimeTicks;
}

int
metisContentStoreEntry_CompareRecommendedCacheTime(const MetisContentStoreEntry *value1, const MetisContentStoreEntry *value2)
{
    // A signum comparison. negative if key 1 is smaller, 0 if key1 == key2, greater than 0 if key1 is bigger.

    MetisContentStoreEntry *v1 = (MetisContentStoreEntry *) value1;
    MetisContentStoreEntry *v2 = (MetisContentStoreEntry *) value2;

    if (v1->recommendedCacheTimeTicks < v2->recommendedCacheTimeTicks) {
        return -1;
    } else if (v1->recommendedCacheTimeTicks > v2->recommendedCacheTimeTicks) {
        return +1;
    } else {
        // At this point, the times are the same. Use the address of the MetisMessage as the decider.
        // This allows us to store multiple MetisMessages with the same expiry/cache time.
        if (v1->message < v2->message) {
            return -1;
        } else if (v1->message > v2->message) {
            return +1;
        }
    }

    return 0; // The same MetisMessage has been encountered.
}

int
metisContentStoreEntry_CompareExpiryTime(const MetisContentStoreEntry *value1, const MetisContentStoreEntry *value2)
{
    // A signum comparison. negative if key 1 is smaller, 0 if key1 == key2, greater than 0 if key1 is bigger.

    MetisContentStoreEntry *v1 = (MetisContentStoreEntry *) value1;
    MetisContentStoreEntry *v2 = (MetisContentStoreEntry *) value2;

    if (v1->expiryTimeTicks < v2->expiryTimeTicks) {
        return -1;
    } else if (v1->expiryTimeTicks > v2->expiryTimeTicks) {
        return +1;
    } else {
        // At this point, the times are the same. Use the address of the MetisMessage as the decider.
        // This allows us to store multiple MetisMessages with the same expiry/cache time.
        if (v1->message < v2->message) {
            return -1;
        } else if (v1->message > v2->message) {
            return +1;
        }
    }

    return 0; // The same MetisMessage has been encountered.
}

void
metisContentStoreEntry_MoveToHead(MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    assertNotNull(storeEntry->lruEntry, "MetisContentStoreEntry is not attached to an LRUList");
    if (storeEntry->lruEntry) {
        metisLruList_EntryMoveToHead(storeEntry->lruEntry);
    }
}
