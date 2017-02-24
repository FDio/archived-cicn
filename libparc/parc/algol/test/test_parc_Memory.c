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

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Memory.c"

LONGBOW_TEST_RUNNER(parc_Memory)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Memory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Memory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_RoundUpToMultiple);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_RoundUpToCacheLine);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_Allocate);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_AllocateAndClear);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_MemAlign);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_Reallocate);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_StringDuplicate);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_Outstanding);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_SetInterface);
    LONGBOW_RUN_TEST_CASE(Global, parcMemory_Format);
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

LONGBOW_TEST_CASE(Global, parcMemory_RoundUpToMultiple)
{
    size_t actual = parcMemory_RoundUpToMultiple(14, 12);
    assertTrue((actual % 12) == 0, "Expected %zd to be a multiple of %d", actual, 12);
    assertTrue(24 == actual, "Expected 24, actual %zd", actual);

    actual = parcMemory_RoundUpToMultiple(14, 20);
    assertTrue((actual % 20) == 0, "Expected %zd to be a multiple of %d", actual, 20);
    assertTrue(20 == actual, "Expected 20, actual %zd", actual);

    actual = parcMemory_RoundUpToMultiple(20, 20);
    assertTrue((actual % 20) == 0, "Expected %zd to be a multiple of %d", actual, 20);
    assertTrue(20 == actual, "Expected 20, actual %zd", actual);

    actual = parcMemory_RoundUpToMultiple(0, 20);
    assertTrue((actual % 20) == 0, "Expected %zd to be a multiple of %d", actual, 20);
    assertTrue(20 == actual, "Expected 20, actual %zd", actual);

    actual = parcMemory_RoundUpToMultiple(8, 0);
    assertTrue(actual == 8, "Expected %d, actual %zd", 8, actual);
}

LONGBOW_TEST_CASE(Global, parcMemory_RoundUpToCacheLine)
{
    size_t actual = parcMemory_RoundUpToCacheLine(LEVEL1_DCACHE_LINESIZE - 1);
    assertTrue((actual % LEVEL1_DCACHE_LINESIZE) == 0,
               "Expected %zd to be a multiple of %d", actual, LEVEL1_DCACHE_LINESIZE);
}

LONGBOW_TEST_CASE(Global, parcMemory_Allocate)
{
    void *pointer;
    pointer = parcMemory_Allocate(sizeof(int));
    assertNotNull(pointer, "Expected pointer to not be NULL");

    parcMemory_Deallocate(&pointer);
    assertNull(pointer, "Expected pointer to not be NULL");
}

LONGBOW_TEST_CASE(Global, parcMemory_MemAlign)
{
    void *pointer;
    int actual = parcMemory_MemAlign(&pointer, sizeof(void *), sizeof(int));
    assertTrue(actual == 0, "Expected successful return value.");
    assertNotNull(pointer, "Expected pointer to not be NULL");

    parcMemory_Deallocate(&pointer);
    assertNull(pointer, "Expected pointer to not be NULL");
}

LONGBOW_TEST_CASE(Global, parcMemory_Reallocate)
{
    void *pointer;
    int actual = parcMemory_MemAlign(&pointer, sizeof(void *), sizeof(int));
    assertTrue(actual == 0, "Expected successful return value.");

    pointer = parcMemory_Reallocate(pointer, sizeof(int) * 2);
    assertNotNull(pointer, "Expected pointer to not be NULL");

    parcMemory_Deallocate(&pointer);
    assertNull(pointer, "Expected pointer to not be NULL");
}

LONGBOW_TEST_CASE(Global, parcMemory_AllocateAndClear)
{
    void *pointer;
    pointer = parcMemory_AllocateAndClear(sizeof(int));
    assertNotNull(pointer, "Expected pointer to not be NULL");

    for (int i = 0; i < sizeof(int); i++) {
        assertTrue(((char *) pointer)[i] == 0, "Expected cell to be zero.");
    }

    parcMemory_Deallocate(&pointer);
    assertNull(pointer, "Expected pointer to not be NULL");
}

LONGBOW_TEST_CASE(Global, parcMemory_StringDuplicate)
{
    char *expected = "Hello";

    char *actual = parcMemory_StringDuplicate(expected, strlen(expected));

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);

    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(Global, parcMemory_Outstanding)
{
    void *pointer;
    pointer = parcMemory_Allocate(sizeof(int));

    size_t expected = 1;
    size_t actual = parcMemory_Outstanding();
    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);

    parcMemory_Deallocate(&pointer);

    expected = 0;
    actual = parcMemory_Outstanding();
    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcMemory_SetInterface)
{
    const PARCMemoryInterface *old = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    parcMemory_SetInterface(old);
}

LONGBOW_TEST_CASE(Global, parcMemory_Format)
{
    char *expected = "Hello World";
    char *actual = parcMemory_Format("Hello %s", "World");

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate(&actual);
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
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Memory);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
