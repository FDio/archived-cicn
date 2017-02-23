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
 * Has one hash table indexed by the ContentObjectHash which stores the objects.
 *
 * Has a MetisMatchingRulesTable used for index lookups.  The stored data points to the
 * object in the storage table.
 *
 * LRU used to manage evictions.
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_HashCodeTable.h>
#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/processor/metis_ContentStore.h>
#include <ccnx/forwarder/metis/processor/metis_ContentStoreEntry.h>
#include <ccnx/forwarder/metis/processor/metis_MatchingRulesTable.h>
#include <ccnx/forwarder/metis/processor/metis_HashTableFunction.h>
#include <ccnx/forwarder/metis/processor/metis_LruList.h>

typedef struct metis_contentstore_stats {
    uint64_t countLruEvictions;
    uint64_t countAdds;
    uint64_t countHits;
    uint64_t countMisses;
} _MetisContentStoreStats;

struct metis_contentstore {
    PARCHashCodeTable *storageByObjectHash;
    MetisMatchingRulesTable *indexTable;

    size_t objectCapacity;
    size_t objectCount;
    MetisLruList *lruList;

    _MetisContentStoreStats stats;
    MetisLogger *logger;
};


// ========================================================================================

static void
_hashTableFunction_ContentStoreEntryDestroyer(void **dataPtr)
{
    metisContentStoreEntry_Release((MetisContentStoreEntry **) dataPtr);
}

static void
_metisContentStore_EvictIfNecessary(MetisContentStore *store)
{
    if (store->objectCount >= store->objectCapacity) {
        MetisLruListEntry *lruEntry = metisLruList_PopTail(store->lruList);
        MetisContentStoreEntry *storeEntry = (MetisContentStoreEntry *) metisLruList_EntryGetData(lruEntry);
        MetisMessage *evictedMessage = metisContentStoreEntry_GetMessage(storeEntry);

        metisMatchingRulesTable_RemoveFromAll(store->indexTable, evictedMessage);

        // This calls the destroyer on storeEntry, which only has a refcount 1 between the LRU and the
        // storageByObjectHash table.  if there's a higher refcount, its someone else holding a copy.
        parcHashCodeTable_Del(store->storageByObjectHash, evictedMessage);

        store->stats.countLruEvictions++;
        store->objectCount--;

        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "ContentStore %p evict message %p (evictions %" PRIu64 ")",
                            (void *) store, (void *) evictedMessage, store->stats.countLruEvictions);
        }

        metisMessage_Release(&evictedMessage);
    }
}

// ==========================================================================================

MetisContentStore *
metisContentStore_Create(size_t objectCapacity, MetisLogger *logger)
{
    size_t initialSize = objectCapacity * 2;
    MetisContentStore *store = parcMemory_AllocateAndClear(sizeof(MetisContentStore));
    assertNotNull(store, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisContentStore));
    memset(&store->stats, 0, sizeof(_MetisContentStoreStats));

    store->logger = metisLogger_Acquire(logger);
    store->lruList = metisLruList_Create();
    store->objectCapacity = objectCapacity;
    store->objectCount = 0;

    // initial size must be at least 1 or else the data structures break.
    initialSize = (initialSize == 0) ? 1 : initialSize;

    store->storageByObjectHash = parcHashCodeTable_Create_Size(metisHashTableFunction_MessageNameAndObjectHashEquals,
                                                               metisHashTableFunction_MessageNameAndObjectHashHashCode,
                                                               NULL,
                                                               _hashTableFunction_ContentStoreEntryDestroyer,
                                                               initialSize);

    // no destroyer on the Rules Table.  They objects are stored in the storage table.
    store->indexTable = metisMatchingRulesTable_Create(NULL);

    if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "ContentStore %p created with capacity %zu",
                        (void *) store, objectCapacity);
    }
    return store;
}

void
metisContentStore_Destroy(MetisContentStore **storePtr)
{
    assertNotNull(storePtr, "Parameter must be non-null double pointer");
    assertNotNull(*storePtr, "Parameter must dereference to non-null pointer");

    MetisContentStore *store = *storePtr;

    if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "ContentStore %p destroyed",
                        (void *) store);
    }

    metisLogger_Release(&store->logger);
    metisMatchingRulesTable_Destroy(&store->indexTable);
    parcHashCodeTable_Destroy(&store->storageByObjectHash);
    metisLruList_Destroy(&store->lruList);
    parcMemory_Deallocate((void **) &store);
    *storePtr = NULL;
}

bool
metisContentStore_Save(MetisContentStore *store, MetisMessage *objectMessage)
{
    bool result = false;

    assertNotNull(store, "Parameter store must be non-null");
    assertNotNull(objectMessage, "Parameter objectMessage must be non-null");
    assertTrue(metisMessage_GetType(objectMessage) == MetisMessagePacketType_ContentObject,
               "Parameter objectMessage must be a Content Object");

    if (store->objectCapacity == 0) {
        return false;
    }

    // if we're at capacity, this will pop the tail off the list and call metisContentStoreEntry_Destroy() on it.
    _metisContentStore_EvictIfNecessary(store);

    // This will add it to the LRU list at the head
    MetisContentStoreEntry *entry = metisContentStoreEntry_Create(objectMessage, store->lruList);

    // adds it to the canonical storage table.  There is only a "1" refcount on the MetisContentStoreEntry, but it
    // is stored in both the LRU and in the storageByObjectHash table

    if (parcHashCodeTable_Add(store->storageByObjectHash, objectMessage, entry)) {
        // index in all the lookup tables the content object ByName, ByNameAndKeyId, and ByNameAndObjectHash
        metisMatchingRulesTable_AddToAllTables(store->indexTable, objectMessage, entry);

        store->objectCount++;
        store->stats.countAdds++;
        result = true;

        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "ContentStore %p saved message %p (object count %" PRIu64 ")",
                            (void *) store, (void *) objectMessage, store->objectCount);
        }
    } else {
        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Warning)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Warning, __func__,
                            "ContentStore %p failed to add message %p to hash table",
                            (void *) store, (void *) objectMessage);
        }


        // Free what we just created, but did not add. 'entry' has ownership of 'copy', and so will
        // call _Release() on it.
        metisContentStoreEntry_Release(&entry);
    }

    return result;
}

MetisMessage *
metisContentStore_Fetch(MetisContentStore *store, MetisMessage *interestMessage)
{
    assertNotNull(store, "Parameter store must be non-null");
    assertNotNull(interestMessage, "Parameter interestMessage must be non-null");
    assertTrue(metisMessage_GetType(interestMessage) == MetisMessagePacketType_Interest,
               "Parameter interestMessage must be an Interest");

    // This will do the most restrictive lookup.
    // a) If the interest has a ContentObjectHash restriction, it will look only in the ByNameAndObjectHash table.
    // b) If it has a KeyId, it will look only in the ByNameAndKeyId table.
    // c) otherwise, it looks only in the ByName table.

    MetisContentStoreEntry *storeEntry = metisMatchingRulesTable_Get(store->indexTable, interestMessage);
    MetisMessage *copy = NULL;

    if (storeEntry) {
        metisContentStoreEntry_MoveToHead(storeEntry);
        copy = metisContentStoreEntry_GetMessage(storeEntry);
        store->stats.countHits++;

        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "ContentStore %p matched interest %p (hits %" PRIu64 ", misses %" PRIu64 ")",
                            (void *) store, (void *) interestMessage, store->stats.countHits, store->stats.countMisses);
        }
    } else {
        store->stats.countMisses++;
        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "ContentStore %p missed interest %p (hits %" PRIu64 ", misses %" PRIu64 ")",
                            (void *) store, (void *) interestMessage, store->stats.countHits, store->stats.countMisses);
        }
    }

    return copy;
}

