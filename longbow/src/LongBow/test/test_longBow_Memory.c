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
 */
#include "../private/longBow_Memory.c"

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

LONGBOW_TEST_RUNNER(test_longBow_Memory)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_longBow_Memory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_longBow_Memory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, longBowMemory_Allocate);
    LONGBOW_RUN_TEST_CASE(Global, longBowMemory_Reallocate);
    LONGBOW_RUN_TEST_CASE(Global, longBowMemory_StringCopy);
}

static uint64_t _setupAllocations;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _setupAllocations = longBowMemory_OutstandingAllocations();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (longBowMemory_OutstandingAllocations() != _setupAllocations) {
        printf("%s Memory leak\n", longBowTestCase_GetFullName(testCase));
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, longBowMemory_Allocate)
{
    void *memory = longBowMemory_Allocate(10);
    assertNotNull(memory, "Return value from longBowMemory_Allocate(10) cannot be NULL.");
    longBowMemory_Deallocate((void **) &memory);
    assertNull(memory, "longBowMemory_Deallocated must NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, longBowMemory_Reallocate)
{
    void *memory = longBowMemory_Allocate(10);
    assertNotNull(memory, "Return value from longBowMemory_Allocate(10) cannot be NULL.");

    memory = longBowMemory_Reallocate(memory, 100);
    assertNotNull(memory, "Return value from longBowMemory_Reallocate cannot be NULL.");

    longBowMemory_Deallocate((void **) &memory);
    assertNull(memory, "longBowMemory_Deallocated must NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, longBowMemory_Reallocate_NULL)
{
    void *memory = longBowMemory_Reallocate(NULL, 100);
    assertNotNull(memory, "Return value from longBowMemory_Reallocate cannot be NULL.");

    longBowMemory_Deallocate((void **) &memory);
    assertNull(memory, "longBowMemory_Deallocated must NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, longBowMemory_StringCopy)
{
    char *expected = "Hello World";

    char *actual = longBowMemory_StringCopy(expected);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    longBowMemory_Deallocate((void **) &actual);
}

int
main(int argc, char *argv[])
{
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_longBow_Memory);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
