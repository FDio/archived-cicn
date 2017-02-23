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
#include "../metis_SymbolicNameTable.c"

#include <stdio.h>
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metis_SymbolicNameTable)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_SymbolicNameTable)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_SymbolicNameTable)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==============================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisSymbolicNameTable_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisSymbolicNameTable_Exists_True);
    LONGBOW_RUN_TEST_CASE(Global, metisSymbolicNameTable_Exists_False);
    LONGBOW_RUN_TEST_CASE(Global, metisSymbolicNameTable_Add_Unique);
    LONGBOW_RUN_TEST_CASE(Global, metisSymbolicNameTable_Add_Duplicate);
    LONGBOW_RUN_TEST_CASE(Global, metisSymbolicNameTable_Get_Exists);
    LONGBOW_RUN_TEST_CASE(Global, metisSymbolicNameTable_Get_Missing);
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

LONGBOW_TEST_CASE(Global, metisSymbolicNameTable_Create)
{
    MetisSymbolicNameTable *table = metisSymbolicNameTable_Create();
    assertNotNull(table, "Got null table");
    assertNotNull(table->symbolicNameTable, "Table did not have an inner hash table allocated");
    metisSymbolicNameTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, metisSymbolicNameTable_Exists_True)
{
    MetisSymbolicNameTable *table = metisSymbolicNameTable_Create();
    metisSymbolicNameTable_Add(table, "foo", 3);
    bool exists = metisSymbolicNameTable_Exists(table, "foo");
    assertTrue(exists, "Failed to find existing key");
    metisSymbolicNameTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, metisSymbolicNameTable_Exists_False)
{
    MetisSymbolicNameTable *table = metisSymbolicNameTable_Create();
    bool exists = metisSymbolicNameTable_Exists(table, "foo");
    assertFalse(exists, "Found non-existent key!");
    metisSymbolicNameTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, metisSymbolicNameTable_Add_Unique)
{
    MetisSymbolicNameTable *table = metisSymbolicNameTable_Create();
    bool success = metisSymbolicNameTable_Add(table, "foo", 3);
    assertTrue(success, "Failed to add a unique key");
    metisSymbolicNameTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, metisSymbolicNameTable_Add_Duplicate)
{
    MetisSymbolicNameTable *table = metisSymbolicNameTable_Create();
    metisSymbolicNameTable_Add(table, "foo", 3);
    bool failure = metisSymbolicNameTable_Add(table, "foo", 4);
    assertFalse(failure, "Should have failed to add a duplicate key");
    metisSymbolicNameTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, metisSymbolicNameTable_Get_Exists)
{
    MetisSymbolicNameTable *table = metisSymbolicNameTable_Create();
    metisSymbolicNameTable_Add(table, "foo", 3);
    unsigned value = metisSymbolicNameTable_Get(table, "foo");
    assertTrue(value == 3, "Wrong value, expected %u got %u", 3, value);
    metisSymbolicNameTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, metisSymbolicNameTable_Get_Missing)
{
    MetisSymbolicNameTable *table = metisSymbolicNameTable_Create();
    unsigned value = metisSymbolicNameTable_Get(table, "foo");
    assertTrue(value == UINT32_MAX, "Wrong value, expected %u got %u", UINT32_MAX, value);
    metisSymbolicNameTable_Destroy(&table);
}


// ==============================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_SymbolicNameTable);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
