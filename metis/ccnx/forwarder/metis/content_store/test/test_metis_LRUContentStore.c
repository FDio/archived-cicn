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

#include "../metis_LRUContentStore.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>


LONGBOW_TEST_RUNNER(metis_LRUContentStore)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_LRUContentStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_LRUContentStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Log);

    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Create_ZeroCapacity);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Fetch_ByName);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Fetch_ByNameAndKeyId);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Fetch_ByNameAndObjectHash);

    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Remove_Content);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Remove_NonExistentContent);

    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Fetch_Lru);

    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_ZeroCapacity);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_CapacityLimit);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_WithoutEviction);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_WithEviction);

    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_WithEvictionByExpiryTime);
    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_WithEvictionByRCT);

    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_ExpiredContent);

    LONGBOW_RUN_TEST_CASE(Global, metisLRUContentStore_Save_DuplicateHash);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static MetisContentStoreInterface *
_createLRUContentStore(size_t capacity)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);

    MetisContentStoreConfig config = {
        .objectCapacity = capacity,
    };

    MetisContentStoreInterface *store = metisLRUContentStore_Create(&config, logger);

    metisLogger_Release(&logger);

    return store;
}

static MetisMessage *
_createUniqueMetisMessage(MetisLogger *logger, int tweakNumber, uint8_t *template, size_t templateSize, int nameOffset)
{
    PARCBuffer *buffer = parcBuffer_Allocate(templateSize);
    memcpy(parcBuffer_Overlay(buffer, 0), template, templateSize);     // Copy the template to new memory

    // Tweak the encoded object's name so the name hash varies each time.
    uint8_t *bufPtr = parcBuffer_Overlay(buffer, 0);
    bufPtr[nameOffset] = 'a' + tweakNumber;

    MetisMessage *result = metisMessage_CreateFromArray(bufPtr, templateSize, 1, 2, logger);
    parcBuffer_Release(&buffer);

    return result;
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Create_Destroy)
{
    MetisContentStoreInterface *store = _createLRUContentStore(10);
    assertNotNull(store, "Expected to init a content store");
    metisContentStoreInterface_Release(&store);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Log)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 20;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    for (int i = 1; i <= capacity; i++) {
        int offsetOfNameInEncodedObject = metisTestDataV0_EncodedObject_name.offset + 4;

        MetisMessage *object = _createUniqueMetisMessage(logger, i,
                                                         metisTestDataV0_EncodedObject,
                                                         sizeof(metisTestDataV0_EncodedObject),
                                                         offsetOfNameInEncodedObject);

        bool success = metisContentStoreInterface_PutContent(store, object, 1);
        metisMessage_Release(&object);

        assertTrue(success, "Unexpectedly failed to add entry to ContentStore");
    }

    metisContentStoreInterface_Log(store);

    metisLogger_Release(&logger);
    metisContentStoreInterface_Release(&store);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Create_ZeroCapacity)
{
    size_t capacity = 10;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    assertTrue(metisContentStoreInterface_GetObjectCapacity(store) == capacity, "Wrong capacity, expected %zu got %zu",
               capacity, metisContentStoreInterface_GetObjectCapacity(store));

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 0, "Wrong initial count, expected %u got %zu", 0,
               metisContentStoreInterface_GetObjectCount(store));
    metisContentStoreInterface_Release(&store);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Fetch_ByName)
{
    size_t capacity = 10;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    uint64_t expiryTime = 300l;
    uint64_t rct = 200;
    uint64_t now = 100;

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    metisMessage_SetExpiryTimeTicks(object_1, expiryTime);
    metisMessage_SetRecommendedCacheTimeTicks(object_1, rct);

    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);
    metisMessage_SetExpiryTimeTicks(object_2, expiryTime);
    metisMessage_SetRecommendedCacheTimeTicks(object_2, rct);

    metisMessage_SetExpiryTimeTicks(object_1, expiryTime);
    metisMessage_SetRecommendedCacheTimeTicks(object_1, rct);

    metisContentStoreInterface_PutContent(store, object_1, now);
    metisContentStoreInterface_PutContent(store, object_2, now);

    MetisMessage *interestByName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName,
                                                                sizeof(metisTestDataV0_InterestWithName), 3, 5, logger);


    MetisMessage *testObject = metisContentStoreInterface_MatchInterest(store, interestByName);
    assertNotNull(testObject, "Fetch did not find match when it should have");

    uint64_t expiryTimeOut = metisMessage_GetExpiryTimeTicks(testObject);
    uint64_t rctOut = metisMessage_GetRecommendedCacheTimeTicks(testObject);
    bool hasExpiryTime = metisMessage_HasExpiryTime(testObject);
    bool hasRCT = metisMessage_HasRecommendedCacheTime(testObject);

    assertTrue(hasRCT, "Expected object to have an RCT");
    assertTrue(hasExpiryTime, "Expected object to have an ExpiryTime");
    assertTrue(expiryTime == expiryTimeOut, "Expected the same expiryTime to be retrieved");
    assertTrue(rct == rctOut, "Expected the same RCT to be retrieved");

    // two objects with same name, 1st one will win
    assertTrue(testObject == object_1, "Fetch returned wrong object, expecting %p got %p",
               (void *) object_1, (void *) testObject);

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
    metisMessage_Release(&interestByName);
}


LONGBOW_TEST_CASE(Global, metisLRUContentStore_Fetch_ByNameAndKeyId)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 10;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStoreInterface_PutContent(store, object_1, 1);
    metisContentStoreInterface_PutContent(store, object_2, 1);

    MetisMessage *interestByNameKeyId = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid,
                                                                     sizeof(metisTestDataV0_InterestWithName_keyid),
                                                                     3, 5, logger);

    MetisMessage *testObject = metisContentStoreInterface_MatchInterest(store, interestByNameKeyId);
    assertNotNull(testObject, "Fetch did not find match when it should have");

    // two objects with same name, 1st one will win
    assertTrue(testObject == object_1, "Fetch returned wrong object, expecting %p got %p",
               (void *) object_1, (void *) testObject);

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
    metisMessage_Release(&interestByNameKeyId);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Fetch_ByNameAndObjectHash)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 10;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    uint64_t expiryTime = 300l;
    uint64_t rct = 200l;
    uint64_t now = 100l;

    metisMessage_SetExpiryTimeTicks(object_1, expiryTime);
    metisMessage_SetRecommendedCacheTimeTicks(object_1, rct);

    metisContentStoreInterface_PutContent(store, object_1, now);
    metisContentStoreInterface_PutContent(store, object_2, now);

    MetisMessage *interestByNameObjectHash =
        metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash,
                                     sizeof(metisTestDataV0_InterestWithName_objecthash), 3, 5, logger);

    // this should retrieve object_1 because that is the one whose
    // content object hash matches the interest

    MetisMessage *testObject = metisContentStoreInterface_MatchInterest(store, interestByNameObjectHash);

    assertTrue(expiryTime == metisMessage_GetExpiryTimeTicks(testObject), "Expected the same expiryTime to be retrieved");
    assertTrue(rct == metisMessage_GetRecommendedCacheTimeTicks(testObject), "Expected the same RCT to be retrieved");

    assertNotNull(testObject, "Fetch did not find match when it should have");

    // two objects with same name, 1st one will win
    assertTrue(testObject == object_1, "Fetch returned wrong object, expecting %p got %p", (void *) object_1, (void *) testObject);

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
    metisMessage_Release(&interestByNameObjectHash);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Remove_Content)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 10;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisMessage_SetExpiryTimeTicks(object_1, 200);
    metisMessage_SetRecommendedCacheTimeTicks(object_1, 100);

    metisContentStoreInterface_PutContent(store, object_1, 10);
    metisContentStoreInterface_PutContent(store, object_2, 10);

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 2, "Expected 2 objects in the metisContentStoreInterface_");

    MetisMessage *interestByNameObjectHash =
        metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash,
                                     sizeof(metisTestDataV0_InterestWithName_objecthash), 3, 5, logger);

    // this should retrieve object_1 because that is the one whose
    // content object hash matches the interest
    MetisMessage *testObject = metisContentStoreInterface_MatchInterest(store, interestByNameObjectHash);


    assertNotNull(testObject, "Fetch did not find match when it should have");

    // two objects with same name, 1st one will win
    assertTrue(testObject == object_1, "Fetch returned wrong object, expecting %p got %p",
               (void *) object_1, (void *) testObject);

    // Now remove it.
    metisContentStoreInterface_RemoveContent(store, object_1);     // Releases the contained Message

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Expected 1 object in the store, got %zu",
               metisContentStoreInterface_GetObjectCount(store));

    MetisMessage *nullObject = metisContentStoreInterface_MatchInterest(store, interestByNameObjectHash);

    assertNull(nullObject, "Fetch found a match when it should not have");

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
    metisMessage_Release(&interestByNameObjectHash);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Remove_NonExistentContent)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 10;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStoreInterface_PutContent(store, object_1, 1);

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Expected 1 object in the metisContentStoreInterface_");

    // Try to remove one that is not in the store.
    bool result = metisContentStoreInterface_RemoveContent(store, object_2);     // Releases the contained Message
    assertFalse(result, "Expected to NOT remove object_2");

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Expected 1 object in the metisContentStoreInterface_");

    result = metisContentStoreInterface_RemoveContent(store, object_1);     // Releases the contained Message
    assertTrue(result, "Expected to remove object_1");

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 0, "Expected 0 objects in the metisContentStoreInterface_");

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
}

/*
 * Create an cache and access objects to make sure the LRU is evicting the right way
 */
LONGBOW_TEST_CASE(Global, metisLRUContentStore_Fetch_Lru)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);

    size_t capacity = 2;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                         sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object2 = metisMessage_CreateFromArray(metisTestDataV0_object_with_othername,
                                                         sizeof(metisTestDataV0_object_with_othername), 2, 2, logger);

    metisContentStoreInterface_PutContent(store, object1, 1);
    metisContentStoreInterface_PutContent(store, object2, 1);

    // object 2 sould be at top of LRU (was saved last).  Fetch object 1, then evict object 2.

    // interest_with_name will match the name in object1.
    MetisMessage *interestByName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName,
                                                                sizeof(metisTestDataV0_InterestWithName), 3, 5, logger);

    MetisMessage *testObject = metisContentStoreInterface_MatchInterest(store, interestByName);

    assertTrue(testObject == object1, "Fetch returned wrong object, expecting %p got %p", (void *) object1, (void *) testObject);

    // objectcapacity = 2, so object 3 will evict bottom of LRU
    MetisMessage *object3 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                         sizeof(metisTestDataV0_SecondObject), 4, 2, logger);
    metisContentStoreInterface_PutContent(store, object3, 1);

    // object 2 should be evicted
    MetisMessage *interestOtherName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithOtherName,
                                                                   sizeof(metisTestDataV0_InterestWithOtherName), 5, 5, logger);
    MetisMessage *testEvictedObject = metisContentStoreInterface_MatchInterest(store, interestOtherName);


    assertNull(testEvictedObject, "object with  othername should have been evicted");

    // as final sanity check, make sure object 1 is still in the list
    MetisMessage *testObject1Again = metisContentStoreInterface_MatchInterest(store, interestByName);

    assertNotNull(testObject1Again, "Did not retrieve object1 from the content store");

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object1);
    metisMessage_Release(&object2);
    metisMessage_Release(&object3);
    metisMessage_Release(&interestByName);
    metisMessage_Release(&interestOtherName);
}


LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_WithoutEviction)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 10;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStoreInterface_PutContent(store, object_1, 10);
    metisContentStoreInterface_PutContent(store, object_2, 10);

    _MetisLRUContentStore *internalStore = ( _MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(store);
    _MetisLRUContentStoreStats *stats = &internalStore->stats;

    assertTrue(stats->countAdds == 2, "Wrong countAdds, expected %u got %" PRIu64, 2, stats->countAdds);
    assertTrue(stats->countLruEvictions == 0, "Wrong countLruEvictions, expected %u got %" PRIu64, 0, stats->countLruEvictions);
    assertTrue(metisLruList_Length(internalStore->lru) == 2, "Wrong metisLruList_Length, expected %u got %zu", 2,
               metisLruList_Length(internalStore->lru));

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_WithEviction)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 1;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *content_1 = _createUniqueMetisMessage(logger, 1, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject),
                                                        metisTestDataV0_EncodedObject_name.offset + 4);

    MetisMessage *content_2 = _createUniqueMetisMessage(logger, 2, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject),
                                                        metisTestDataV0_EncodedObject_name.offset + 4);

    MetisMessage *content_3 = _createUniqueMetisMessage(logger, 3, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject),
                                                        metisTestDataV0_EncodedObject_name.offset + 4);


    metisContentStoreInterface_PutContent(store, content_1, 1);

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Wrong objectCount. Expected %u, got %zu", 1, metisContentStoreInterface_GetObjectCount(store));

    metisContentStoreInterface_PutContent(store, content_2, 1);
    metisContentStoreInterface_PutContent(store, content_3, 1);

    _MetisLRUContentStore *internalStore = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(store);
    _MetisLRUContentStoreStats *stats = &internalStore->stats;

    // Capacity is 1, so we should never grow bigger than that.
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Wrong objectCount. Expected %u, got %zu", 1, metisContentStoreInterface_GetObjectCount(store));

    assertTrue(stats->countAdds == 3, "Wrong countAdds, expected %u got %" PRIu64, 2,
               stats->countAdds);
    assertTrue(stats->countLruEvictions == 2, "Wrong countLruEvictions, expected %u got %" PRIu64, 1, stats->countLruEvictions);
    assertTrue(metisLruList_Length(internalStore->lru) == 1, "Wrong metisLruList_Length, expected %u got %zu", 1,
               metisLruList_Length(internalStore->lru));

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&content_1);
    metisMessage_Release(&content_2);
    metisMessage_Release(&content_3);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_ExpiredContent)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 1;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisMessage_SetRecommendedCacheTimeTicks(object_1, 50);

    assertFalse(metisContentStoreInterface_PutContent(store, object_1, 51), "Should not be able to insert content past its recommended cache time");
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 0, "Wrong objectCount. Expected 0, got %zu", metisContentStoreInterface_GetObjectCount(store));

    metisMessage_SetExpiryTimeTicks(object_2, 100);

    assertFalse(metisContentStoreInterface_PutContent(store, object_2, 101), "Should not be able to insert content past its expiry time");
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 0, "Wrong objectCount. Expected 0, got %zu", metisContentStoreInterface_GetObjectCount(store));

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
}


LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_WithEvictionByExpiryTime)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 1;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    uint64_t currentMetisTime = 150;
    uint64_t expiryTime = 200;

    metisMessage_SetExpiryTimeTicks(object_1, expiryTime);

    // This should add the object, as currentMetisTime is less than expiry or RCT.
    metisContentStoreInterface_PutContent(store, object_1, currentMetisTime);
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Wrong objectCount. Expected 1, got %zu", metisContentStoreInterface_GetObjectCount(store));

    // No expiry time.
    metisContentStoreInterface_PutContent(store, object_2, expiryTime + 10); // Add this one after expiration of first one.

    _MetisLRUContentStore *internalStore = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(store);
    _MetisLRUContentStoreStats *stats = &internalStore->stats;

    // Capacity is 1, so we should never grow bigger than that.
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Wrong objectCount. Expected 1, got %zu", metisContentStoreInterface_GetObjectCount(store));

    assertTrue(stats->countAdds == 2, "Wrong countAdds, expected %u got %" PRIu64, 2,
               stats->countAdds);
    assertTrue(stats->countExpiryEvictions == 1, "Wrong countLruEvictions, expected %u got %" PRIu64, 1, stats->countLruEvictions);
    assertTrue(metisLruList_Length(internalStore->lru) == 1, "Wrong metisLruList_Length, expected 1 got %zu",
               metisLruList_Length(internalStore->lru));
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Expected a store count of 1");

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_WithEvictionByRCT)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 1;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject,
                                                          sizeof(metisTestDataV0_SecondObject), 1, 2, logger);


    uint64_t recommendedCacheTime = 1000;

    metisMessage_SetRecommendedCacheTimeTicks(object_1, recommendedCacheTime);
    metisContentStoreInterface_PutContent(store, object_1, recommendedCacheTime - 100); // Add it.

    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Wrong objectCount. Expected 1, got %zu",
               metisContentStoreInterface_GetObjectCount(store));

    metisContentStoreInterface_PutContent(store, object_2, recommendedCacheTime + 1); // Add this one after the first one's RCT.

    _MetisLRUContentStore *internalStore = (_MetisLRUContentStore *) metisContentStoreInterface_GetPrivateData(store);
    _MetisLRUContentStoreStats *stats = &internalStore->stats;

    // Capacity is 1, so we should never grow bigger than that.
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Wrong objectCount. Expected 1, got %zu",
               metisContentStoreInterface_GetObjectCount(store));

    assertTrue(stats->countAdds == 2, "Wrong countAdds, expected %u got %" PRIu64, 2,
               stats->countAdds);
    assertTrue(stats->countExpiryEvictions == 0, "Wrong countLruEvictions, expected 1got %" PRIu64, stats->countLruEvictions);
    assertTrue(stats->countRCTEvictions == 1, "Wrong countLruEvictions, expected 1 got %" PRIu64, stats->countLruEvictions);

    assertTrue(metisLruList_Length(internalStore->lru) == 1, "Wrong metisLruList_Length, expected 1 got %zu", metisLruList_Length(internalStore->lru));
    assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "Expected a store count of 1");

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_ZeroCapacity)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 0;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    bool success = metisContentStoreInterface_PutContent(store, object_1, 1);
    assertFalse(success, "Should have returned failure with 0 capacity object store saving something");

    metisMessage_Release(&object_1);
    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}


LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_CapacityLimit)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 5;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    for (int i = 1; i < capacity * 2; i++) {
        int offsetOfNameInEncodedObject = metisTestDataV0_EncodedObject_name.offset + 4;

        MetisMessage *object = _createUniqueMetisMessage(logger, i,
                                                         metisTestDataV0_EncodedObject,
                                                         sizeof(metisTestDataV0_EncodedObject),
                                                         offsetOfNameInEncodedObject);

        bool success = metisContentStoreInterface_PutContent(store, object, 1);

        assertTrue(success, "Unexpectedly failed to add entry to ContentStore");

        if (i < metisContentStoreInterface_GetObjectCapacity(store)) {
            assertTrue(metisContentStoreInterface_GetObjectCount(store) == i, "Unexpected value for metisContentStoreInterface_objectCount");
        } else {
            assertTrue(metisContentStoreInterface_GetObjectCount(store) == metisContentStoreInterface_GetObjectCapacity(store),
                       "Unexpected value (%zu) for metisContentStoreInterface_objectCount (%zu)",
                       metisContentStoreInterface_GetObjectCount(store), metisContentStoreInterface_GetObjectCapacity(store));
        }

        if (success) {
            metisMessage_Release(&object);
        }
    }
    metisLogger_Release(&logger);
    metisContentStoreInterface_Release(&store);
}

LONGBOW_TEST_CASE(Global, metisLRUContentStore_Save_DuplicateHash)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    size_t capacity = 5;
    MetisContentStoreInterface *store = _createLRUContentStore(capacity);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                          sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);


    bool success = metisContentStoreInterface_PutContent(store, object_1, 1);
    assertTrue(success, "Expected to add object_1 to store");

    for (int i = 0; i < 10; i++) {
        MetisMessage *object_1_dup = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                                  sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

        success = metisContentStoreInterface_PutContent(store, object_1_dup, 1l);

        assertFalse(success, "Unexpectedly added duplicated entry to ContentStore");

        assertTrue(metisContentStoreInterface_GetObjectCount(store) == 1, "ObjectCount should be 1");

        metisMessage_Release(&object_1_dup);
    }

    metisMessage_Release(&object_1);
    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


// ============================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_LRUContentStore);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
