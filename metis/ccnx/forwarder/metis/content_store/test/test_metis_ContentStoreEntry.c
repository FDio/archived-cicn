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

#include "../metis_ContentStoreEntry.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>


LONGBOW_TEST_RUNNER(metis_ContentStoreEntry)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_ContentStoreEntry)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_ContentStoreEntry)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_Create_Destroy_Memory);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_Create_Destroy_State);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_GetMessage);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_MoveToHead);

    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_GetExpiryTimeInTicks);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_GetRecommendedCacheTimeInTicks);

    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_CompareRecommendedCacheTime);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreEntry_CompareExpiryTime);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}
static MetisLogger *
_createLogger()
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);

    return logger;
}
LONGBOW_TEST_CASE(Global, metisContentStoreEntry_Create_Destroy_Memory)
{
    MetisLruList *lruList = metisLruList_Create();
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    size_t beforeMemory = parcMemory_Outstanding();
    MetisContentStoreEntry *storeEntry = metisContentStoreEntry_Create(object, lruList);
    metisContentStoreEntry_Release(&storeEntry);
    size_t afterMemory = parcMemory_Outstanding();

    metisLruList_Destroy(&lruList);
    metisMessage_Release(&object);

    assertTrue(afterMemory == beforeMemory, "Imbalance on create/destroy, expected %zu got %zu", beforeMemory, afterMemory);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_Create_Destroy_State)
{
    MetisLogger *logger = _createLogger();

    MetisLruList *lruList = metisLruList_Create();
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    MetisContentStoreEntry *storeEntry = metisContentStoreEntry_Create(object, lruList);

    assertTrue(storeEntry->refcount == 1, "lruEntry has wrong refcount, expected %u got %u", 1, storeEntry->refcount);

    metisContentStoreEntry_Release(&storeEntry);
    metisLruList_Destroy(&lruList);
    metisMessage_Release(&object);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_Acquire)
{
    MetisLogger *logger = _createLogger();
    MetisLruList *lruList = metisLruList_Create();
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    MetisContentStoreEntry *storeEntry = metisContentStoreEntry_Create(object, lruList);
    MetisContentStoreEntry *copy = metisContentStoreEntry_Acquire(storeEntry);

    assertTrue(copy->refcount == 2, "incorrect refcount after copy, expected %u got %u", 2, copy->refcount);
    metisContentStoreEntry_Release(&storeEntry);
    assertTrue(copy->refcount == 1, "incorrect refcount after destroy, expected %u got %u", 1, copy->refcount);

    metisContentStoreEntry_Release(&copy);
    metisLruList_Destroy(&lruList);
    metisMessage_Release(&object);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_GetMessage)
{
    MetisLogger *logger = _createLogger();

    MetisLruList *lruList = metisLruList_Create();
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    MetisContentStoreEntry *storeEntry = metisContentStoreEntry_Create(object, lruList);

    MetisMessage *copy = metisContentStoreEntry_GetMessage(storeEntry);
    assertTrue(copy == object, "Incorrect mesage, expected %p got %p", (void *) object, (void *) copy);

    metisContentStoreEntry_Release(&storeEntry);

    metisLruList_Destroy(&lruList);
    metisMessage_Release(&object);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_MoveToHead)
{
    MetisLogger *logger = _createLogger();
    MetisLruList *lruList = metisLruList_Create();

    MetisMessage *object[3];
    MetisContentStoreEntry *storeEntry[3];

    for (int i = 0; i < 3; i++) {
        object[i] = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), i, 2, logger);
        storeEntry[i] = metisContentStoreEntry_Create(object[i], lruList);
    }

    // object 2 is at top of list and object 0 is at bottom.  move 0 to top and 1 should be at bottom
    metisContentStoreEntry_MoveToHead(storeEntry[0]);

    MetisLruListEntry *bottom = metisLruList_PopTail(lruList);

    assertTrue(bottom == storeEntry[1]->lruEntry, "Incorrect mesage, expected %p got %p", (void *) storeEntry[1]->lruEntry, (void *) bottom);
    for (int i = 0; i < 3; i++) {
        metisContentStoreEntry_Release(&storeEntry[i]);
        metisMessage_Release(&object[i]);
    }

    metisLogger_Release(&logger);
    metisLruList_Destroy(&lruList);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_GetExpiryTimeInTicks)
{
    MetisLogger *logger = _createLogger();

    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                        sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    uint64_t expiryTime = 101l;
    metisMessage_SetExpiryTimeTicks(object, expiryTime);

    MetisContentStoreEntry *storeEntry = metisContentStoreEntry_Create(object, NULL);

    assertTrue(metisContentStoreEntry_HasExpiryTimeTicks(storeEntry), "Expected entry to have expiry time");
    assertTrue(metisContentStoreEntry_GetExpiryTimeTicks(storeEntry) == expiryTime, "Got unexpected expiry time");

    metisContentStoreEntry_Release(&storeEntry);
    metisMessage_Release(&object);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_GetRecommendedCacheTimeInTicks)
{
    MetisLogger *logger = _createLogger();

    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                        sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    uint64_t rct = 202l;
    metisMessage_SetRecommendedCacheTimeTicks(object, rct);

    MetisContentStoreEntry *storeEntry = metisContentStoreEntry_Create(object, NULL);

    assertTrue(metisContentStoreEntry_HasRecommendedCacheTimeTicks(storeEntry), "Expected entry to have expiry time");
    assertTrue(metisContentStoreEntry_GetRecommendedCacheTimeTicks(storeEntry) == rct, "Got unexpected cache time");

    metisContentStoreEntry_Release(&storeEntry);
    metisMessage_Release(&object);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_CompareExpiryTime)
{
    MetisLogger *logger = _createLogger();

    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *message2 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 3, 4, logger);

    metisMessage_SetExpiryTimeTicks(message, 100);
    MetisContentStoreEntry *entry1 = metisContentStoreEntry_Create(message, NULL);

    metisMessage_SetExpiryTimeTicks(message, 200);
    MetisContentStoreEntry *entry2 = metisContentStoreEntry_Create(message, NULL);

    // Different message, same times as entry1
    metisMessage_SetExpiryTimeTicks(message2, 100);
    MetisContentStoreEntry *entry3 = metisContentStoreEntry_Create(message2, NULL);

    // Same message, same times as entry2
    metisMessage_SetExpiryTimeTicks(message, 200);
    MetisContentStoreEntry *entry4 = metisContentStoreEntry_Create(message, NULL);

    assertTrue(metisContentStoreEntry_CompareExpiryTime(entry1, entry2) == -1, "Expected -1");
    assertTrue(metisContentStoreEntry_CompareExpiryTime(entry2, entry1) == 1, "Expected +1");
    assertTrue(metisContentStoreEntry_CompareExpiryTime(entry1, entry1) == 0, "Expected 0");

    //  Compare same expiry time, but different Message addressed.
    if (message < message2) {
        assertTrue(metisContentStoreEntry_CompareExpiryTime(entry1, entry3) == -1, "Expected -1");
        assertTrue(metisContentStoreEntry_CompareExpiryTime(entry3, entry1) == 1, "Expected +1");
    } else {
        assertTrue(metisContentStoreEntry_CompareExpiryTime(entry1, entry3) == 1, "Expected 1");
        assertTrue(metisContentStoreEntry_CompareExpiryTime(entry3, entry1) == -1, "Expected -1");
    }

    assertTrue(metisContentStoreEntry_CompareExpiryTime(entry2, entry4) == 0, "Expected 0");

    metisContentStoreEntry_Release(&entry1);
    metisContentStoreEntry_Release(&entry2);
    metisContentStoreEntry_Release(&entry3);
    metisContentStoreEntry_Release(&entry4);

    metisMessage_Release(&message);
    metisMessage_Release(&message2);

    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreEntry_CompareRecommendedCacheTime)
{
    MetisLogger *logger = _createLogger();

    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *message2 = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 3, 4, logger);

    metisMessage_SetRecommendedCacheTimeTicks(message, 200);
    MetisContentStoreEntry *entry1 = metisContentStoreEntry_Create(message, NULL);

    metisMessage_SetRecommendedCacheTimeTicks(message, 100);
    MetisContentStoreEntry *entry2 = metisContentStoreEntry_Create(message, NULL);

    // Different message, same times as entry1
    metisMessage_SetRecommendedCacheTimeTicks(message2, 200);
    MetisContentStoreEntry *entry3 = metisContentStoreEntry_Create(message2, NULL);

    // Same message, same times as entry2
    metisMessage_SetRecommendedCacheTimeTicks(message, 100);
    MetisContentStoreEntry *entry4 = metisContentStoreEntry_Create(message, NULL);

    assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry1, entry2) == 1, "Expected 1");
    assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry2, entry1) == -1, "Expected -1");
    assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry1, entry1) == 0, "Expected 0");

    //  Compare same RCT, but different Message addressed.
    if (message < message2) {
        assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry1, entry3) == -1, "Expected -1");
        assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry3, entry1) == 1, "Expected +1");
    } else {
        assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry1, entry3) == 1, "Expected 1");
        assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry3, entry1) == -1, "Expected -1");
    }

    assertTrue(metisContentStoreEntry_CompareRecommendedCacheTime(entry2, entry4) == 0, "Expected 0");

    metisContentStoreEntry_Release(&entry1);
    metisContentStoreEntry_Release(&entry2);
    metisContentStoreEntry_Release(&entry3);
    metisContentStoreEntry_Release(&entry4);

    metisMessage_Release(&message);
    metisMessage_Release(&message2);

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
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_ContentStoreEntry);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
