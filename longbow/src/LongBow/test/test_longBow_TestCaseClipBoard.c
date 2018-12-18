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

#include <inttypes.h>
#include <LongBow/testing.h>
#include <LongBow/private/longBow_Memory.h>

LONGBOW_TEST_RUNNER(longBow_TestCaseClipBoard)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(longBow_TestCaseClipBoard)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(longBow_TestCaseClipBoard)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, longBowTestCaseClipBoard_CreateDestroy);
    LONGBOW_RUN_TEST_CASE(Global, longBowTestCaseClipBoard_Get);
    LONGBOW_RUN_TEST_CASE(Global, longBowTestCaseClipBoard_Set);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, longBowTestCaseClipBoard_CreateDestroy)
{
    uint64_t allocations = longBowMemory_OutstandingAllocations();
    char *shared = longBowMemory_StringCopy("shared data");

    LongBowTestCaseClipBoard *clipboard = longBowTestCaseClipBoard_Create(shared);
    assertNotNull(clipboard, "Expected non-null result from longBowTestCaseClipBoard_Create");

    longBowTestCaseClipBoard_Destroy(&clipboard);
    longBowMemory_Deallocate((void **) &shared);

    assertTrue(longBowMemory_OutstandingAllocations() == allocations,
               "Memory leaks %" PRId64, longBowMemory_OutstandingAllocations());
}

LONGBOW_TEST_CASE(Global, longBowTestCaseClipBoard_Get)
{
    char *shared = longBowMemory_StringCopy("shared data");

    LongBowTestCaseClipBoard *clipboard = longBowTestCaseClipBoard_Create(shared);

    char *actual = longBowTestCaseClipBoard_Get(clipboard);
    assertTrue(strcmp(shared, actual) == 0, "Expected %s, actual %s", shared, actual);

    longBowTestCaseClipBoard_Destroy(&clipboard);
    longBowMemory_Deallocate((void **) &shared);
}

LONGBOW_TEST_CASE(Global, longBowTestCaseClipBoard_Set)
{
    char *shared = longBowMemory_StringCopy("shared data");

    LongBowTestCaseClipBoard *clipboard = longBowTestCaseClipBoard_Create(shared);

    char *expected = longBowMemory_StringCopy("expected");

    longBowTestCaseClipBoard_Set(clipboard, expected);
    char *actual = longBowTestCaseClipBoard_Get(clipboard);
    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);

    longBowTestCaseClipBoard_Destroy(&clipboard);
    longBowMemory_Deallocate((void **) &shared);
    longBowMemory_Deallocate((void **) &expected);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(longBow_TestCaseClipBoard);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
