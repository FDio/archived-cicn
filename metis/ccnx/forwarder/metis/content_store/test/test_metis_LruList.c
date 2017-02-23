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

#include "../metis_LruList.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_LruList)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_LruList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_LruList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisLruList_Create_Destroy);

    LONGBOW_RUN_TEST_CASE(Global, MetisLruListEntry_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisLruEntry_GetData);
    LONGBOW_RUN_TEST_CASE(Global, metisLruEntry_MoveToHead);

    LONGBOW_RUN_TEST_CASE(Global, metisLruList_NewHeadEntry);
    LONGBOW_RUN_TEST_CASE(Global, metisLruList_PopTail);
    LONGBOW_RUN_TEST_CASE(Global, MetisLruList_Length);
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

LONGBOW_TEST_CASE(Global, MetisLruListEntry_Destroy)
{
    MetisLruList *lru = metisLruList_Create();

    size_t beforeMemory = parcMemory_Outstanding();
    MetisLruListEntry *entry = (MetisLruListEntry *) metisLruList_NewHeadEntry(lru, (void *) 0x02);
    metisLruList_PopTail(lru);
    metisLruList_EntryDestroy(&entry);
    size_t afterMemory = parcMemory_Outstanding();

    metisLruList_Destroy(&lru);

    assertTrue(afterMemory == beforeMemory, "Memory imbalance for LruEntry_Destroy, expected %zu got %zu", beforeMemory, afterMemory);
}

LONGBOW_TEST_CASE(Global, metisLruEntry_GetData)
{
    void *data = (void *) 99;
    MetisLruList *lru = metisLruList_Create();
    MetisLruListEntry *entry = (MetisLruListEntry *) metisLruList_NewHeadEntry(lru, data);
    void *p = metisLruList_EntryGetData(entry);

    assertTrue(p == data, "Data did not match, expected %p, got %p", data, p);
    metisLruList_Destroy(&lru);
}

LONGBOW_TEST_CASE(Global, metisLruEntry_MoveToHead)
{
    int loops = 10;

    MetisLruList *lru = metisLruList_Create();

    for (size_t i = 1; i <= loops; i++) {
        metisLruList_NewHeadEntry(lru, (void *) i);
    }

    MetisLruListEntry *tail = metisLruList_PopTail(lru);
    metisLruList_EntryMoveToHead(tail);

    MetisLruListEntry *test = TAILQ_FIRST(&lru->head);
    assertTrue(test == tail, "Head element not moved, expected %p got %p", (void *) tail, (void *) test);

    metisLruList_Destroy(&lru);
}

LONGBOW_TEST_CASE(Global, metisLruList_Create_Destroy)
{
    size_t baselineMemory = parcMemory_Outstanding();

    MetisLruList *lru = metisLruList_Create();
    metisLruList_Destroy(&lru);

    assertTrue(parcMemory_Outstanding() == baselineMemory, "Memory imbalance on create/destroy: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, metisLruList_Length)
{
    int loops = 10;

    MetisLruList *lru = metisLruList_Create();

    for (size_t i = 1; i <= loops; i++) {
        MetisLruListEntry *entry = (MetisLruListEntry *) metisLruList_NewHeadEntry(lru, (void *) i);
        assertTrue(lru->itemsInList == i, "Incorrect list length element %zu, expected %zu got %zu", i, i, lru->itemsInList);
        assertTrue(metisLruList_Length(lru) == i, "Incorrect length encountered");

        MetisLruListEntry *test = TAILQ_FIRST(&lru->head);
        assertTrue(test == entry, "Head element not new entry, expected %p got %p", (void *) entry, (void *) test);
    }
    metisLruList_Destroy(&lru);
}

LONGBOW_TEST_CASE(Global, metisLruList_NewHeadEntry)
{
    int loops = 10;

    MetisLruList *lru = metisLruList_Create();

    for (size_t i = 1; i <= loops; i++) {
        MetisLruListEntry *entry = (MetisLruListEntry *) metisLruList_NewHeadEntry(lru, (void *) i);
        assertTrue(lru->itemsInList == i, "Incorrect list length element %zu, expected %zu got %zu", i, i, lru->itemsInList);

        MetisLruListEntry *test = TAILQ_FIRST(&lru->head);
        assertTrue(test == entry, "Head element not new entry, expected %p got %p", (void *) entry, (void *) test);
    }
    metisLruList_Destroy(&lru);
}

LONGBOW_TEST_CASE(Global, metisLruList_PopTail)
{
    int loops = 10;

    MetisLruList *lru = metisLruList_Create();

    for (size_t i = 1; i <= loops; i++) {
        metisLruList_NewHeadEntry(lru, (void *) i);
    }

    for (size_t i = 1; i <= loops; i++) {
        MetisLruListEntry *entry = metisLruList_PopTail(lru);
        void *data = metisLruList_EntryGetData(entry);
        assertTrue(data == (void *) i, "Got wrong data, expected %p got %p", (void *) i, (void *) data);

        metisLruList_EntryDestroy(&entry);
    }

    metisLruList_Destroy(&lru);
}


LONGBOW_TEST_CASE(Global, MetisLruList_Length)
{
    int loops = 10;

    MetisLruList *lru = metisLruList_Create();

    for (size_t i = 1; i <= loops; i++) {
        metisLruList_NewHeadEntry(lru, (void *) i);
        assertTrue(metisLruList_Length(lru) == i, "Unexpected LruList length");
    }

    metisLruList_Destroy(&lru);
}

// ============================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_LruList);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
