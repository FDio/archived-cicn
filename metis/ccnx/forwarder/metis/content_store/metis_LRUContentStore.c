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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_HashCodeTable.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/assert/parc_Assert.h>

#include <ccnx/forwarder/metis/core/metis_Logger.h>

#include <ccnx/forwarder/metis/content_store/metis_LRUContentStore.h>

#include <ccnx/forwarder/metis/content_store/metis_ContentStoreInterface.h>
#include <ccnx/forwarder/metis/content_store/metis_ContentStoreEntry.h>
#include <ccnx/forwarder/metis/content_store/metis_LruList.h>
#include <ccnx/forwarder/metis/content_store/metis_TimeOrderedList.h>

#include <ccnx/forwarder/metis/processor/metis_HashTableFunction.h>

typedef struct metis_lru_contentstore_stats {
    uint64_t countExpiryEvictions;
    uint64_t countRCTEvictions;
    uint64_t countLruEvictions;
    uint64_t countAdds;
    uint64_t countHits;
    uint64_t countMisses;
} _MetisLRUContentStoreStats;


typedef struct metis_lru_contentstore_data {
    size_t objectCapacity;
    size_t objectCount;

    MetisLogger *logger;

    // This LRU is just for keeping track of insertion and access order.
    MetisLruList *lru;

    // These are indexes by name and key ID hash
    PARCHashCodeTable *indexByNameHash;
    PARCHashCodeTable *indexByNameAndKeyIdHash;

    // These are indexes by time.
    MetisTimeOrderedList *indexByRecommendedCacheTime;
    MetisTimeOrderedList *indexByExpirationTime;

    // This table is responsible for Releasing our ContentStoreEntries.
    PARCHashCodeTable *storageByNameAndObjectHashHash;

    _MetisLRUContentStoreStats stats;
} _MetisLRUContentStore;



static void
_destroyIndexes(_MetisLRUContentStore *store)
{
    if (store->indexByNameHash != NULL) {
        parcHashCodeTable_Destroy(&(store->indexByNameHash));
    }

    if (store->indexByNameAndKeyIdHash != NULL) {
        parcHashCodeTable_Destroy(&(store->indexByNameAndKeyIdHash));
    }

    if (store->indexByRecommendedCacheTime != NULL) {
        metisTimeOrderedList_Release(&(store->indexByRecommendedCacheTime));
    }

    if (store->indexByExpirationTime != NULL) {
        metisTimeOrderedList_Release(&(store->indexByExpirationTime));
    }

    // This tables must go last. It holds the references to the MetisMessage.
    if (store->storageByNameAndObjectHashHash != NULL) {
        parcHashCodeTable_Destroy(&(store->storageByNameAndObjectHashHash));
    }

    if (store->lru != NULL) {
        metisLruList_Destroy(&(store->lru));
    }
}

static void
_MetisContentStoreInterface_Destroy(MetisContentStoreInterface **storeImplPtr)
{
    _MetisLRUContentStore *store = metisContentStoreInterface_GetPrivateData(*storeImplPtr);

    parcObject_Release((PARCObject **) &store);
}

static bool
_MetisLRUContentStore_Destructor(_MetisLRUContentStore **storePtr)
{
    _MetisLRUContentStore *store = *storePtr;
   
    _destroyIndexes(store);
    metisLogger_Release(&store->logger);
   
    return true;
}

parcObject_Override(_MetisLRUContentStore, PARCObject,
                    .destructor = (PARCObjectDestructor *) _MetisLRUContentStore_Destructor
                    );

parcObject_ExtendPARCObject(MetisContentStoreInterface,
                            _MetisContentStoreInterface_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

static parcObject_ImplementAcquire(_metisLRUContentStore, MetisContentStoreInterface);
static parcObject_ImplementRelease(_metisLRUContentStore, MetisContentStoreInterface);

static void
_hashTableFunction_ContentStoreEntryDestroyer(void **dataPtr)
{
    metisContentStoreEntry_Release((MetisContentStoreEntry **) dataPtr);
}

static bool
_metisLRUContentStore_Init(_MetisLRUContentStore *store, MetisContentStoreConfig *config, MetisLogger *logger)
{
    bool result = false;

    store->logger = metisLogger_Acquire(logger);

    size_t initialSize = config->objectCapacity * 2;
    memset(&store->stats, 0, sizeof(_MetisLRUContentStoreStats));

    store->objectCapacity = config->objectCapacity;
    store->objectCount = 0;

    // initial size must be at least 1 or else the data structures break.
    initialSize = (initialSize == 0) ? 1 : initialSize;

    store->indexByExpirationTime =
        metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);

    store->indexByRecommendedCacheTime =
        metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareRecommendedCacheTime);

    store->indexByNameHash = parcHashCodeTable_Create_Size(metisHashTableFunction_MessageNameEquals,
                                                           metisHashTableFunction_MessageNameHashCode,
                                                           NULL,
                                                           NULL,
                                                           initialSize);

    store->indexByNameAndKeyIdHash = parcHashCodeTable_Create_Size(metisHashTableFunction_MessageNameAndKeyIdEquals,
                                                                   metisHashTableFunction_MessageNameAndKeyIdHashCode,
                                                                   NULL,
                                                                   NULL,
                                                                   initialSize);

    store->storageByNameAndObjectHashHash = parcHashCodeTable_Create_Size(metisHashTableFunction_MessageNameAndObjectHashEquals,
                                                                          metisHashTableFunction_MessageNameAndObjectHashHashCode,
                                                                          NULL,
                                                                          _hashTableFunction_ContentStoreEntryDestroyer,
                                                                          initialSize);

    store->lru = metisLruList_Create();

    // If any of the index tables couldn't be allocated, we can't continue.
    if ((store->indexByExpirationTime == NULL)
        || (store->indexByNameAndKeyIdHash == NULL)
        || (store->indexByNameHash == NULL)
        || (store->indexByRecommendedCacheTime == NULL)
        || (store->storageByNameAndObjectHashHash == NULL)
        || (store->lru == NULL)) {
        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Error)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Error, __func__,
                            "LRUContentStore could not be created. Could not allocate all index tables.",
                            (void *) store, store->objectCapacity);
        }

        _destroyIndexes(store);
        result = false;
    } else {
        result = true;
    }
    return result;
}

/**
 * Remove a MetisContentStoreEntry from all tables and indices.
 */
static void
_metisLRUContentStore_PurgeStoreEntry(_MetisLRUContentStore *store, MetisContentStoreEntry *entryToPurge)
{
    if (metisContentStoreEntry_HasExpiryTimeTicks(entryToPurge)) {
        metisTimeOrderedList_Remove(store->indexByExpirationTime, entryToPurge);
    }

    if (metisContentStoreEntry_HasRecommendedCacheTimeTicks(entryToPurge)) {
        metisTimeOrderedList_Remove(store->indexByRecommendedCacheTime, entryToPurge);
    }

    MetisMessage *content = metisContentStoreEntry_GetMessage(entryToPurge);
    parcHashCodeTable_Del(store->indexByNameHash, content);

    if (metisMessage_HasKeyId(content)) {
        parcHashCodeTable_Del(store->indexByNameAndKeyIdHash, content);
    }

    // This _Del call will call the Release/Destroy on the ContentStoreEntry,
    // which will remove it from the LRU as well.
    parcHashCodeTable_Del(store->storageByNameAndObjectHashHash, content);

    store->objectCount--;
}

static bool
_metisLRUContentStore_RemoveLeastUsed(_MetisLRUContentStore *store)
{
    bool result = false;

    if (store->objectCount > 0) {
        MetisLruListEntry *lruEntry = metisLruList_PopTail(store->lru);
        MetisContentStoreEntry *storeEntry =
            (MetisContentStoreEntry *) metisLruList_EntryGetData(lruEntry);

        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "ContentStore %p evict message %p by LRU (LRU evictions %" PRIu64 ")",
                            (void *) store, (void *) metisContentStoreEntry_GetMessage(storeEntry),
                            store->stats.countLruEvictions);
        }

        _metisLRUContentStore_PurgeStoreEntry(store, storeEntry);

        result = true;
    }
    return result;
}

static void
_evictByStorePolicy(_MetisLRUContentStore *store, uint64_t currentTimeInMetisTicks)
{
    // We need to make room. Here's the plan:
    //  1) Check to see if anything has expired. If so, remove it and we're done. If not,
    //  2) Check to see if anything has exceeded it's recommended cache time. If so, remove it and we're done. If not,
    //  3) Remove the least recently used item.

    MetisContentStoreEntry *entry = metisTimeOrderedList_GetOldest(store->indexByExpirationTime);
    if (entry
        && metisContentStoreEntry_HasExpiryTimeTicks(entry)
        && (currentTimeInMetisTicks > metisContentStoreEntry_GetExpiryTimeTicks(entry))) {
        // Found an expired entry. Remove it, and we're done.

        store->stats.countExpiryEvictions++;
        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "ContentStore %p evict message %p by ExpiryTime (ExpiryTime evictions %" PRIu64 ")",
                            (void *) store, (void *) metisContentStoreEntry_GetMessage(entry),
                            store->stats.countExpiryEvictions);
        }

        _metisLRUContentStore_PurgeStoreEntry(store, entry);
    } else {
        // Check for entries that have exceeded RCT
        entry = metisTimeOrderedList_GetOldest(store->indexByRecommendedCacheTime);
        if (entry
            && metisContentStoreEntry_HasRecommendedCacheTimeTicks(entry)
            && (currentTimeInMetisTicks > metisContentStoreEntry_GetRecommendedCacheTimeTicks(entry))) {
            // Found an entry passed it's RCT. Remove it, and we're done.

            store->stats.countRCTEvictions++;
            if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
                metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                                "ContentStore %p evict message %p by RCT (RCT evictions %" PRIu64 ")",
                                (void *) store, (void *) metisContentStoreEntry_GetMessage(entry),
                                store->stats.countRCTEvictions);
            }

            _metisLRUContentStore_PurgeStoreEntry(store, entry);
        } else {
            store->stats.countLruEvictions++;
            _metisLRUContentStore_RemoveLeastUsed(store);
        }
    }
}

static bool
_metisLRUContentStore_PutContent(MetisContentStoreInterface *storeImpl, MetisMessage *content, uint64_t currentTimeTicks)

{
    bool result = false;
    _MetisLRUContentStore *store = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(storeImpl);
    parcAssertNotNull(store, "Parameter store must be non-null");
    parcAssertNotNull(content, "Parameter objectMessage must be non-null");

    parcAssertTrue(metisMessage_GetType(content) == MetisMessagePacketType_ContentObject,
               "Parameter objectMessage must be a Content Object");

    if (store->objectCapacity == 0) {
        return false;
    }

    uint64_t expiryTimeTicks = metisContentStoreEntry_MaxExpiryTime;
    uint64_t recommendedCacheTimeTicks = metisContentStoreEntry_MaxRecommendedCacheTime;

    if (metisMessage_HasExpiryTime(content)) {
        expiryTimeTicks = metisMessage_GetExpiryTimeTicks(content);
    }

    if (metisMessage_HasRecommendedCacheTime(content)) {
        recommendedCacheTimeTicks = metisMessage_GetRecommendedCacheTimeTicks(content);
    }

    // Don't add anything that's already expired or has exceeded RCT.
    if (currentTimeTicks >= expiryTimeTicks || currentTimeTicks >= recommendedCacheTimeTicks) {
        return false;
    }

    if (store->objectCount >= store->objectCapacity) {
        // Store is full. Need to make room.
        _evictByStorePolicy(store, currentTimeTicks);
    }

    // And now add a new entry to the head of the LRU.

    MetisContentStoreEntry *entry = metisContentStoreEntry_Create(content, store->lru);

    if (entry != NULL) {
        if (parcHashCodeTable_Add(store->storageByNameAndObjectHashHash, content, entry)) {
            parcHashCodeTable_Add(store->indexByNameHash, content, entry);

            if (metisMessage_HasKeyId(content)) {
                parcHashCodeTable_Add(store->indexByNameAndKeyIdHash, content, entry);
            }

            if (metisContentStoreEntry_HasExpiryTimeTicks(entry)) {
                metisTimeOrderedList_Add(store->indexByExpirationTime, entry);
            }

            if (metisContentStoreEntry_HasRecommendedCacheTimeTicks(entry)) {
                metisTimeOrderedList_Add(store->indexByRecommendedCacheTime, entry);
            }

            store->objectCount++;
            store->stats.countAdds++;

            if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
                metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                                "LRUContentStore %p saved message %p (object count %" PRIu64 ")",
                                (void *) store, (void *) content, store->objectCount);
            }

            result = true;
        } else {
            // Free what we just created, but did not add. 'entry' has ownership of 'copy', and so will
            // call _Release() on it
            metisContentStoreEntry_Release(&entry);

            if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Warning)) {
                metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Warning, __func__,
                                "LRUContentStore %p failed to add message %p to hash table",
                                (void *) store, (void *) content);
            }
        }
    }

    return result;
}

static MetisMessage *
_metisLRUContentStore_MatchInterest(MetisContentStoreInterface *storeImpl, MetisMessage *interest)
{
    MetisMessage *result = NULL;

    _MetisLRUContentStore *store = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(storeImpl);

    parcAssertNotNull(store, "Parameter store must be non-null");
    parcAssertNotNull(interest, "Parameter interestMessage must be non-null");
    parcAssertTrue(metisMessage_GetType(interest) == MetisMessagePacketType_Interest,
               "Parameter interestMessage must be an Interest");

    // This will do the most restrictive lookup.
    // a) If the interest has a ContentObjectHash restriction, it will look only in the ByNameAndObjectHash table.
    // b) If it has a KeyId, it will look only in the ByNameAndKeyId table.
    // c) otherwise, it looks only in the ByName table.

    PARCHashCodeTable *table;
    if (metisMessage_HasContentObjectHash(interest)) {
        table = store->storageByNameAndObjectHashHash;
    } else if (metisMessage_HasKeyId(interest)) {
        table = store->indexByNameAndKeyIdHash;
    } else {
        table = store->indexByNameHash;
    }

    MetisContentStoreEntry *storeEntry = parcHashCodeTable_Get(table, interest);

    if (storeEntry) {
        metisContentStoreEntry_MoveToHead(storeEntry);
        result = metisContentStoreEntry_GetMessage(storeEntry);

        store->stats.countHits++;

        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "LRUContentStore %p matched interest %p (hits %" PRIu64 ", misses %" PRIu64 ")",
                            (void *) store, (void *) interest, store->stats.countHits, store->stats.countMisses);
        }
    } else {
        store->stats.countMisses++;

        if (metisLogger_IsLoggable(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
            metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                            "LRUContentStore %p missed interest %p (hits %" PRIu64 ", misses %" PRIu64 ")",
                            (void *) store, (void *) interest, store->stats.countHits, store->stats.countMisses);
        }
    }

    return result;
}

static bool
_metisLRUContentStore_RemoveContent(MetisContentStoreInterface *storeImpl, MetisMessage *content)
{
    bool result = false;
    _MetisLRUContentStore *store = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(storeImpl);

    MetisContentStoreEntry *storeEntry = parcHashCodeTable_Get(store->storageByNameAndObjectHashHash, content);

    if (storeEntry != NULL) {
        _metisLRUContentStore_PurgeStoreEntry(store, storeEntry);
        result = true;
    }

    return result;
}

static void
_metisLRUContentStore_Log(MetisContentStoreInterface *storeImpl)
{
    _MetisLRUContentStore *store = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(storeImpl);

    metisLogger_Log(store->logger, MetisLoggerFacility_Processor, PARCLogLevel_All, __func__,
                    "MetisLRUContentStore @%p {count = %zu, capacity = %zu {"
                    "stats = @%p {adds = %" PRIu64 ", hits = %" PRIu64 ", misses = %" PRIu64 ", LRUEvictons = %" PRIu64
                    ", ExpiryEvictions = %" PRIu64 ", RCTEvictions = %" PRIu64 "} }",
                    store,
                    store->objectCount,
                    store->objectCapacity,
                    &store->stats,
                    store->stats.countAdds,
                    store->stats.countHits,
                    store->stats.countMisses,
                    store->stats.countLruEvictions,
                    store->stats.countExpiryEvictions,
                    store->stats.countRCTEvictions);
}

static size_t
_metisLRUContentStore_GetObjectCapacity(MetisContentStoreInterface *storeImpl)
{
    _MetisLRUContentStore *store = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(storeImpl);
    return store->objectCapacity;
}

static size_t
_metisLRUContentStore_GetObjectCount(MetisContentStoreInterface *storeImpl)
{
    _MetisLRUContentStore *store = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(storeImpl);
    return store->objectCount;
}

static size_t
_metisLRUContentStore_SetObjectCapacity(MetisContentStoreInterface *storeImpl, size_t newCapacity)
{
    _MetisLRUContentStore *store = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(storeImpl);
    return store->objectCapacity = newCapacity;
}

MetisContentStoreInterface *
metisLRUContentStore_Create(MetisContentStoreConfig *config, MetisLogger *logger)
{
    MetisContentStoreInterface *storeImpl = NULL;

    parcAssertNotNull(logger, "MetisLRUContentStore requires a non-NULL logger");

    storeImpl = parcObject_CreateAndClearInstance(MetisContentStoreInterface);

    if (storeImpl != NULL) {
        storeImpl->_privateData = parcObject_CreateAndClearInstance(_MetisLRUContentStore);

        if (_metisLRUContentStore_Init(storeImpl->_privateData, config, logger)) {
            storeImpl->putContent = &_metisLRUContentStore_PutContent;
            storeImpl->removeContent = &_metisLRUContentStore_RemoveContent;

            storeImpl->matchInterest = &_metisLRUContentStore_MatchInterest;

            storeImpl->getObjectCount = &_metisLRUContentStore_GetObjectCount;
            storeImpl->getObjectCapacity = &_metisLRUContentStore_GetObjectCapacity;

            storeImpl->log = &_metisLRUContentStore_Log;

            storeImpl->acquire = &_metisLRUContentStore_Acquire;
            storeImpl->release = &_metisLRUContentStore_Release;

            // Initialize from the config passed to us.
            _metisLRUContentStore_SetObjectCapacity(storeImpl, config->objectCapacity);

            if (metisLogger_IsLoggable(logger, MetisLoggerFacility_Processor, PARCLogLevel_Info)) {
                metisLogger_Log(logger, MetisLoggerFacility_Processor, PARCLogLevel_Info, __func__,
                                "LRUContentStore %p created with capacity %zu",
                                (void *) storeImpl, metisContentStoreInterface_GetObjectCapacity(storeImpl));
            }
        }
    } else {
        parcObject_Release((void **) &storeImpl);
    }

    return storeImpl;
}

