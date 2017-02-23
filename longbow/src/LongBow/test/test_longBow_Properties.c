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

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <stdio.h>
#include <inttypes.h>

#include <LongBow/longBow_Properties.h>
#include "../private/longBow_Memory.h"

LONGBOW_TEST_RUNNER(test_longBow_Properties)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_longBow_Properties)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_longBow_Properties)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, CreateDestroy);
    LONGBOW_RUN_TEST_CASE(Global, longBowProperties_Set);
    LONGBOW_RUN_TEST_CASE(Global, longBowProperties_Get);
    LONGBOW_RUN_TEST_CASE(Global, longBowProperties_Length);
    LONGBOW_RUN_TEST_CASE(Global, longBowProperties_Exists);
}

static uint64_t _setupAllocations;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _setupAllocations = longBowMemory_OutstandingAllocations();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint64_t leaks = longBowMemory_OutstandingAllocations() - _setupAllocations;
    if (leaks != 0) {
        printf("%s leaks %" PRId64 " allocations.\n", longBowTestCase_GetFullName(testCase), leaks);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, CreateDestroy)
{
    LongBowProperties *properties = longBowProperties_Create();
    longBowProperties_Destroy(&properties);
}

LONGBOW_TEST_CASE(Global, longBowProperties_Set)
{
    LongBowProperties *properties = longBowProperties_Create();
    longBowProperties_Set(properties, "name", "value");
    longBowProperties_Destroy(&properties);
}

LONGBOW_TEST_CASE(Global, longBowProperties_Get)
{
    LongBowProperties *properties = longBowProperties_Create();
    char *expected = "value";
    longBowProperties_Set(properties, "name", expected);

    const char *actual = longBowProperties_Get(properties, "name");

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    longBowProperties_Destroy(&properties);
}

LONGBOW_TEST_CASE(Global, longBowProperties_Length)
{
    LongBowProperties *properties = longBowProperties_Create();
    assertTrue(longBowProperties_Length(properties) == 0, "Expected empty longBowProperties to be 0 length");

    char *expected = "value";
    longBowProperties_Set(properties, "name", expected);
    assertTrue(longBowProperties_Length(properties) == 1, "Expected longBowProperties to be 1 length");

    const char *actual = longBowProperties_Get(properties, "name");

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    longBowProperties_Destroy(&properties);
}

LONGBOW_TEST_CASE(Global, longBowProperties_Exists)
{
    char *expected = "value";

    LongBowProperties *properties = longBowProperties_Create();
    assertFalse(longBowProperties_Exists(properties, expected), "Expected longBowProperties_Exists to be false");

    longBowProperties_Set(properties, "name", expected);
    assertTrue(longBowProperties_Exists(properties, "name"), "Expected longBowProperties_Exists to be true");

    longBowProperties_Destroy(&properties);
}

int
main(int argc, char *argv[argc])
{
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_longBow_Properties);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
