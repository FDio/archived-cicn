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
// This permits internal static functions to be visible to this Test Runner.

#include <config.h>
#include <stdlib.h>  //  for rand()

#include "../metis_TimeOrderedList.c"

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

LONGBOW_TEST_RUNNER(metis_TimeOrderedList)
{
    srand(5150); // A fixed seed for the RNG for consistency.
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_TimeOrderedList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_TimeOrderedList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisTimeOrderedList_CreateRelease);
    LONGBOW_RUN_TEST_CASE(Global, metisTimeOrderedList_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, metisTimeOrderedList_AddRemove);
    LONGBOW_RUN_TEST_CASE(Global, metisTimeOrderedList_GetOldest);
    LONGBOW_RUN_TEST_CASE(Global, metisTimeOrderedList_Length);
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

static MetisLogger *
_createLogger()
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    return logger;
}

LONGBOW_TEST_CASE(Global, metisTimeOrderedList_CreateRelease)
{
    MetisLogger *logger = _createLogger();

    MetisTimeOrderedList *list = metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
    MetisLruList *lruList = metisLruList_Create();
    MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, 2, logger);

    metisMessage_SetRecommendedCacheTimeTicks(message, 100);
    metisMessage_SetExpiryTimeTicks(message, 200);
    MetisContentStoreEntry *entry = metisContentStoreEntry_Create(message, lruList);

    metisTimeOrderedList_Add(list, entry);

    metisTimeOrderedList_Release(&list);

    metisContentStoreEntry_Release(&entry);
    metisMessage_Release(&message);  // We must release the message, as it's not acquired by the TimeOrderedList.
    metisLruList_Destroy(&lruList);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisTimeOrderedList_AcquireRelease)
{
    MetisTimeOrderedList *list = metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
    MetisTimeOrderedList *ref = metisTimeOrderedList_Acquire(list);

    assertTrue(ref == list, "Expected ref and original to be the same");

    metisTimeOrderedList_Release(&list);
    metisTimeOrderedList_Release(&ref);
}

LONGBOW_TEST_CASE(Global, metisTimeOrderedList_AddRemove)
{
    MetisLogger *logger = _createLogger();
    MetisTimeOrderedList *list = metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
    MetisLruList *lruList = metisLruList_Create();

    size_t numEntries = 100;
    MetisContentStoreEntry **contentEntryList = parcMemory_AllocateAndClear(numEntries * sizeof(MetisContentStoreEntry *));

    for (int i = 1; i <= numEntries; i++) {
        MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, i, logger);

        metisMessage_SetRecommendedCacheTimeTicks(message, i % 10); // i % 10 will ensure that there are duplicate time entries.
        metisMessage_SetExpiryTimeTicks(message, i % 10);

        contentEntryList[i - 1] = metisContentStoreEntry_Create(message, lruList);
        metisTimeOrderedList_Add(list, contentEntryList[i - 1]);

        assertTrue(metisTimeOrderedList_Length(list) == i, "Got wrong TimeOrderedList object count");
    }

    for (int i = 1; i <= numEntries; i++) {
        MetisContentStoreEntry *contentEntry = contentEntryList[i - 1];
        metisTimeOrderedList_Remove(list, contentEntry);
        MetisMessage *message = metisContentStoreEntry_GetMessage(contentEntry);
        metisMessage_Release(&message);

        size_t count = metisTimeOrderedList_Length(list);
        assertTrue(count == numEntries - i, "Got wrong TimeOrderedList object count");
        metisContentStoreEntry_Release(&contentEntry);
    }

    parcMemory_Deallocate((void **) &contentEntryList);
    metisTimeOrderedList_Release(&list);
    metisLruList_Destroy(&lruList);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisTimeOrderedList_GetOldest)
{
    MetisLogger *logger = _createLogger();

    // We're going to use the ExpiryTime as the sorting comparison for GetOldest().
    MetisTimeOrderedList *list = metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);

    // Add some entries, with randomly ordered ExpiryTimes.
    for (int i = 0; i < 100; i++) {
        uint64_t time = (uint64_t) rand() + 1;

        MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, i, logger);

        metisMessage_SetRecommendedCacheTimeTicks(message, 100); // constant RCT
        metisMessage_SetExpiryTimeTicks(message, time);          // random expiry time.

        MetisContentStoreEntry *entry = metisContentStoreEntry_Create(message, NULL);

        metisTimeOrderedList_Add(list, entry);

        assertTrue(metisTimeOrderedList_Length(list) == i + 1, "Got wrong TimeOrderedList object count");
    }

    // Ensure that GetOldest() always returns the oldest.

    uint64_t lastTime = 0;
    MetisContentStoreEntry *entry = NULL;
    while ((entry = metisTimeOrderedList_GetOldest(list)) != NULL) {
        MetisMessage *message = metisContentStoreEntry_GetMessage(entry);
        uint64_t messageTime = metisMessage_GetExpiryTimeTicks(message);

        // We should always retrieve a time later than the previous time we retrieved.
        assertTrue(messageTime > lastTime, "Received out of order message");

        lastTime = messageTime;
        metisTimeOrderedList_Remove(list, entry);
        metisMessage_Release(&message);
        metisContentStoreEntry_Release(&entry);
    }

    metisTimeOrderedList_Release(&list);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisTimeOrderedList_Length)
{
    MetisLogger *logger = _createLogger();

    MetisTimeOrderedList *list = metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);

    // Add some entries with duplicate times to make sure that duplicate time stamps work.
    uint64_t times[] = { 1, 2, 3, 100, 100, 100, 4, 4, 3, 2, 1, 5, 6, 7, 8, 9 };
    size_t numEntriesToMake = sizeof(times) / sizeof(uint64_t);

    for (int i = 0; i < numEntriesToMake; i++) {
        MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, times[i], logger);
        metisMessage_SetExpiryTimeTicks(message, times[i]);
        MetisContentStoreEntry *entry = metisContentStoreEntry_Create(message, NULL);
        metisTimeOrderedList_Add(list, entry);
        assertTrue(metisTimeOrderedList_Length(list) == i + 1, "Got wrong TimeOrderedList object count");
    }

    // Clean up the messages we allocated.
    int i = 0;
    MetisContentStoreEntry *entry = NULL;
    while ((entry = metisTimeOrderedList_GetOldest(list)) != NULL) {
        assertTrue(metisTimeOrderedList_Length(list) == numEntriesToMake - i, "Got wrong TimeOrderedList object count");

        MetisMessage *message = metisContentStoreEntry_GetMessage(entry);
        metisTimeOrderedList_Remove(list, entry);
        metisMessage_Release(&message);
        metisContentStoreEntry_Release(&entry);
        i++;
    }

    metisTimeOrderedList_Release(&list);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_FIXTURE(Static)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_TimeOrderedList);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}

