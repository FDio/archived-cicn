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

/** *
 */
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_MemoryTesting.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>

LONGBOW_TEST_RUNNER(test_parc_MemoryTest)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_MemoryTest)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_MemoryTest)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcMemoryTesting_ExpectedOutstanding);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcMemoryTesting_ExpectedOutstanding)
{
    void *memory = parcMemory_Allocate(4);

    assertTrue(parcMemoryTesting_ExpectedOutstanding(1, "Expected 1 outstanding allocation"),
               "Failed 1 expected allocation");

    parcMemory_Deallocate(&memory);
    assertTrue(parcMemoryTesting_ExpectedOutstanding(0, "Expected 0 outstanding allocations"),
               "Failed 0 expected allocations");

    assertFalse(parcMemoryTesting_ExpectedOutstanding(1, "Expected 1 outstanding allocations"),
                "Failed 1 expected llocations");
    assertFalse(parcMemoryTesting_ExpectedOutstanding(-1, "Expected -1 outstanding allocations"),
                "Failed -1 expected allocations");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_MemoryTest);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
