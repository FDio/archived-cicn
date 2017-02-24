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

#include <stdio.h>
#include <inttypes.h>
#include <LongBow/testing.h>
#include <LongBow/private/longBow_Memory.h>

LONGBOW_TEST_RUNNER(longBow_Fixture)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateDestroy);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(longBow_Fixture)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(longBow_Fixture)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateDestroy)
{
    LONGBOW_RUN_TEST_CASE(CreateDestroy, LongBowTestFixture_Create_Destroy);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateDestroy)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateDestroy)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateDestroy, LongBowTestFixture_Create_Destroy)
{
    uint64_t allocations = longBowMemory_OutstandingAllocations();
    LongBowTestRunner *runner = longBowTestRunner_Create("runner", NULL, NULL, NULL);

    LongBowTestFixture *fixture = longBowTestFixture_Create(runner, "fixture", NULL, NULL, NULL);
    assertNotNull(fixture, "Expected non-null result from LongBowTestFixture_Create");
    longBowTestFixture_Destroy(&fixture);
    longBowTestRunner_Destroy(&runner);
    assertTrue(longBowMemory_OutstandingAllocations() == allocations,
               "Memory leaks %" PRId64, longBowMemory_OutstandingAllocations());
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, longBowTestFixture_GetRunner);
    LONGBOW_RUN_TEST_CASE(Global, longBowTestFixture_GetClipBoard);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    LongBowTestRunner *runner = longBowTestRunner_Create("testRunner", NULL, NULL, NULL);
    LongBowTestFixture *fixture = longBowTestFixture_Create(runner, "testFixture", NULL, NULL, NULL);

    longBowClipBoard_Set(testClipBoard, "runner", runner);
    longBowClipBoard_Set(testClipBoard, "fixture", fixture);
    longBowTestCase_SetClipBoardData(testCase, fixture);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    LongBowTestFixture *fixture = longBowTestCase_GetClipBoardData(testCase);

    LongBowTestRunner *runner = longBowTestFixture_GetRunner(fixture);

    longBowTestRunner_Destroy(&runner);
    longBowTestFixture_Destroy(&fixture);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, longBowTestFixture_GetRunner)
{
    LongBowTestRunner *xrunner = longBowClipBoard_Get(testClipBoard, "runner");
    LongBowTestFixture *xfixture = longBowClipBoard_Get(testClipBoard, "fixture");
    LongBowTestFixture *fixture = longBowTestCase_GetClipBoardData(testCase);
    LongBowTestRunner *runner = longBowTestFixture_GetRunner(fixture);

    assertTrue(xrunner == runner, "Expected runner to be equal.");
    assertTrue(xfixture == fixture, "Expected runner to be equal.");

    assertNotNull(runner, "Expected the test runner to not be null");
}

LONGBOW_TEST_CASE(Global, longBowTestFixture_GetClipBoard)
{
    LongBowTestFixture *fixture = longBowTestCase_GetClipBoardData(testCase);
    LongBowClipBoard *clipboard = longBowTestFixture_GetClipBoard(fixture);
    assertNotNull(clipboard, "Expected non-null result from longBowTestFixture_GetClipBoard");
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
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(longBow_Fixture);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
