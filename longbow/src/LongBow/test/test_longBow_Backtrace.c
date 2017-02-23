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

#include "../longBow_Backtrace.h"

#include <stdio.h>
#include "../testing.h"
#include <LongBow/private/longBow_Memory.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.

LONGBOW_TEST_RUNNER(longBow_Backtrace)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(longBow_Backtrace)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(longBow_Backtrace)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, LongBowBacktrace_ToString);
    LONGBOW_RUN_TEST_CASE(Global, longBowBacktrace_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, LongBowBacktrace_ToString)
{
    LongBowBacktrace *backtrace = longBowBacktrace_Create(100, 0);
    char *result = longBowBacktrace_ToString(backtrace);

    assertNotNull(result, "Expected non-null result from LongBowBacktrace_ToString()");

    longBowMemory_Deallocate((void **) &result);
    longBowBacktrace_Destroy(&backtrace);
}

LONGBOW_TEST_CASE(Global, longBowBacktrace_Create)
{
    LongBowBacktrace *backtrace = longBowBacktrace_Create(100, 0);
    assertNotNull(backtrace, "Expected non-null result from longBowBacktrace_Create()");
    longBowBacktrace_Destroy(&backtrace);
    assertNull(backtrace, "Expected LongBowBacktrace_Destroy() to set the pointer to null");

    backtrace = longBowBacktrace_Create(100, 1);
    longBowBacktrace_Destroy(&backtrace);
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
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(longBow_Backtrace);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);

    exit(exitStatus);
}
