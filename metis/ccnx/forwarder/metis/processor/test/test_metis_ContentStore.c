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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_ContentStore.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

LONGBOW_TEST_RUNNER(metis_ContentStore)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_ContentStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_ContentStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Create_ZeroCapacity);

    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Fetch_ByName);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Fetch_ByNameAndKeyId);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Fetch_ByNameAndObjectHash);

    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Fetch_Lru);

    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Save_ZeroCapacity);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Save_CapacityLimit);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Save_WithoutEviction);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Save_WithEviction);

    LONGBOW_RUN_TEST_CASE(Global, metisContentStore_Save_DuplicateHash);
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

LONGBOW_TEST_CASE(Global, metisContentStore_Create_Destroy)
{
    size_t objectCapacity = 10;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);
    metisLogger_Release(&logger);

    assertTrue(store->objectCapacity == objectCapacity, "Wrong capacity, expected %zu got %zu", objectCapacity, store->objectCapacity);
    assertTrue(store->objectCount == 0, "Wrong initial count, expected %u got %zu", 0, store->objectCount);
    metisContentStore_Destroy(&store);

    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Memory imbalance after create/destroy, expected %u got %u", 0, parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, metisContentStore_Create_ZeroCapacity)
{
    size_t objectCapacity = 0;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);
    assertTrue(store->objectCapacity == objectCapacity, "Wrong capacity, expected %zu got %zu", objectCapacity, store->objectCapacity);
    assertTrue(store->objectCount == 0, "Wrong initial count, expected %u got %zu", 0, store->objectCount);
    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
}

LONGBOW_TEST_CASE(Global, metisContentStore_Fetch_ByName)
{
    size_t objectCapacity = 10;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);
    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStore_Save(store, object_1);
    metisContentStore_Save(store, object_2);

    MetisMessage *interestByName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 3, 5, logger);
    MetisMessage *testObject = metisContentStore_Fetch(store, interestByName);
    assertNotNull(testObject, "Fetch did not find match when it should have");

    // two objects with same name, 1st one will win
    assertTrue(testObject == object_1, "Fetch returned wrong object, expecting %p got %p", (void *) object_1, (void *) testObject);

    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
    metisMessage_Release(&testObject);
    metisMessage_Release(&interestByName);
}

LONGBOW_TEST_CASE(Global, metisContentStore_Fetch_ByNameAndKeyId)
{
    size_t objectCapacity = 10;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);
    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStore_Save(store, object_1);
    metisContentStore_Save(store, object_2);

    MetisMessage *interestByNameKeyId = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 3, 5, logger);
    MetisMessage *testObject = metisContentStore_Fetch(store, interestByNameKeyId);
    assertNotNull(testObject, "Fetch did not find match when it should have");

    // two objects with same name, 1st one will win
    assertTrue(testObject == object_1, "Fetch returned wrong object, expecting %p got %p", (void *) object_1, (void *) testObject);

    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
    metisMessage_Release(&testObject);
    metisMessage_Release(&interestByNameKeyId);
}

LONGBOW_TEST_CASE(Global, metisContentStore_Fetch_ByNameAndObjectHash)
{
    size_t objectCapacity = 10;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);
    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStore_Save(store, object_1);
    metisContentStore_Save(store, object_2);

    MetisMessage *interestByNameObjectHash = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 3, 5, logger);

    // this should retrieve object_1 because that is the one whose
    // content object hash matches the interest
    MetisMessage *testObject = metisContentStore_Fetch(store, interestByNameObjectHash);
    assertNotNull(testObject, "Fetch did not find match when it should have");

    // two objects with same name, 1st one will win
    assertTrue(testObject == object_1, "Fetch returned wrong object, expecting %p got %p", (void *) object_1, (void *) testObject);

    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
    metisMessage_Release(&testObject);
    metisMessage_Release(&interestByNameObjectHash);
}

/*
 * Create an cache and access objects to make sure the LRU is evicting the right way
 */
LONGBOW_TEST_CASE(Global, metisContentStore_Fetch_Lru)
{
    const size_t objectCapacity = 2;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);

    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);

    MetisMessage *object1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object2 = metisMessage_CreateFromArray(metisTestDataV0_object_with_othername, sizeof(metisTestDataV0_object_with_othername), 2, 2, logger);

    metisContentStore_Save(store, object1);
    metisContentStore_Save(store, object2);

    // object 2 sould be at top of LRU (was saved last).  Fetch object 1, then evict object 2.

    // interest_with_name will match the name in object1.
    MetisMessage *interestByName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 3, 5, logger);
    MetisMessage *testObject = metisContentStore_Fetch(store, interestByName);

    assertTrue(testObject == object1, "Fetch returned wrong object, expecting %p got %p", (void *) object1, (void *) testObject);

    // objectcapacity = 2, so object 3 will evict bottom of LRU
    MetisMessage *object3 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 4, 2, logger);
    metisContentStore_Save(store, object3);

    // object 2 should be evicted
    MetisMessage *interestOtherName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithOtherName, sizeof(metisTestDataV0_InterestWithOtherName), 5, 5, logger);
    MetisMessage *testEvictedObject = metisContentStore_Fetch(store, interestOtherName);
    assertNull(testEvictedObject, "object with  othername should have been evicted");

    // as final sanity check, make sure object 1 is still in the list
    MetisMessage *testObject1Again = metisContentStore_Fetch(store, interestByName);
    assertNotNull(testObject1Again, "Did not retrieve object1 from the content store");

    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
    metisMessage_Release(&testObject1Again);
    metisMessage_Release(&object1);
    metisMessage_Release(&object2);
    metisMessage_Release(&object3);
    metisMessage_Release(&testObject);
    metisMessage_Release(&interestByName);
    metisMessage_Release(&interestOtherName);
}


LONGBOW_TEST_CASE(Global, metisContentStore_Save_WithoutEviction)
{
    size_t objectCapacity = 10;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);
    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStore_Save(store, object_1);
    metisContentStore_Save(store, object_2);

    assertTrue(store->stats.countAdds == 2, "Wrong countAdds, expected %u got %" PRIu64, 2, store->stats.countAdds);
    assertTrue(store->stats.countLruEvictions == 0, "Wrong countLruEvictions, expected %u got %" PRIu64, 0, store->stats.countLruEvictions);
    assertTrue(metisLruList_Length(store->lruList) == 2, "Wrong metisLruList_Length, expected %u got %zu", 2, metisLruList_Length(store->lruList));

    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
}

LONGBOW_TEST_CASE(Global, metisContentStore_Save_WithEviction)
{
    size_t objectCapacity = 1;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);
    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *object_2 = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 2, logger);

    metisContentStore_Save(store, object_1);

    assertTrue(store->objectCount == 1, "Wrong objectCount. Expected %u, got %zu", 1, store->objectCount);

    metisContentStore_Save(store, object_2);

    // Capacity is 1, so we should never grow bigger than that.
    assertTrue(store->objectCount == 1, "Wrong objectCount. Expected %u, got %zu", 1, store->objectCount);

    assertTrue(store->stats.countAdds == 2, "Wrong countAdds, expected %u got %" PRIu64, 2, store->stats.countAdds);
    assertTrue(store->stats.countLruEvictions == 1, "Wrong countLruEvictions, expected %u got %" PRIu64, 1, store->stats.countLruEvictions);
    assertTrue(metisLruList_Length(store->lruList) == 1, "Wrong metisLruList_Length, expected %u got %zu", 1, metisLruList_Length(store->lruList));

    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
    metisMessage_Release(&object_1);
    metisMessage_Release(&object_2);
}

LONGBOW_TEST_CASE(Global, metisContentStore_Save_ZeroCapacity)
{
    size_t objectCapacity = 0;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(objectCapacity, logger);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    bool success = metisContentStore_Save(store, object_1);
    assertFalse(success, "Should have returned failure with 0 capacity object store saving something");

    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisContentStore_Destroy(&store);
}

static MetisMessage *
_createUniqueMetisMessage(int tweakNumber, uint8_t *template, size_t templateSize, int nameOffset)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    PARCBuffer *buffer = parcBuffer_Allocate(templateSize);
    memcpy(parcBuffer_Overlay(buffer, 0), template, templateSize); // Copy the template to new memory

    // Tweak the encoded object's name so the name hash varies each time.
    uint8_t *bufPtr = parcBuffer_Overlay(buffer, 0);
    bufPtr[nameOffset] = 'a' + tweakNumber;

    MetisMessage *result = metisMessage_CreateFromArray(bufPtr, templateSize, 1, 2, logger);
    metisLogger_Release(&logger);
    parcBuffer_Release(&buffer);

    return result;
}

LONGBOW_TEST_CASE(Global, metisContentStore_Save_CapacityLimit)
{
    size_t storeCapacity = 5;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(storeCapacity, logger);

    for (int i = 1; i < storeCapacity * 2; i++) {

        int offsetOfNameInEncodedObject = metisTestDataV0_EncodedObject.offset + 4;
        MetisMessage *object = _createUniqueMetisMessage(i, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), offsetOfNameInEncodedObject);

        bool success = metisContentStore_Save(store, object);

        assertTrue(success, "Unexpectedly failed to add entry to ContentStore");

        if (i < store->objectCapacity) {
            assertTrue(store->objectCount == i, "Unexpected value for store->objectCount");
        } else {
            assertTrue(store->objectCount == store->objectCapacity, "Unexpected value (%zu) for store->objectCount (%zu)",
                       store->objectCount, store->objectCapacity);
        }

        if (success) {
            metisMessage_Release(&object);
        }
    }
    metisLogger_Release(&logger);
    metisContentStore_Destroy(&store);
}

LONGBOW_TEST_CASE(Global, metisContentStore_Save_DuplicateHash)
{
    size_t storeCapacity = 5;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisContentStore *store = metisContentStore_Create(storeCapacity, logger);

    MetisMessage *object_1 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    bool success = metisContentStore_Save(store, object_1);

    for (int i = 0; i < 10; i++) {
        MetisMessage *object_1_dup = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

        success = metisContentStore_Save(store, object_1_dup);

        assertFalse(success, "Unexpectedly added duplicated entry to ContentStore");

        assertTrue(store->objectCount == 1, "ObjectCount should be 1");

        metisMessage_Release(&object_1_dup);
    }

    metisLogger_Release(&logger);
    metisMessage_Release(&object_1);
    metisContentStore_Destroy(&store);
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, hashTableFunction_ContentStoreEntryDestroyer);
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

LONGBOW_TEST_CASE(Local, hashTableFunction_ContentStoreEntryDestroyer)
{
    testUnimplemented("This test is unimplemented");
}

// ============================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_ContentStore);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
