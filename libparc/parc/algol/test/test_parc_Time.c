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
#include <config.h>

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Memory.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Time.c"

LONGBOW_TEST_RUNNER(test_parc_Time)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_Time)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_Time)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcTime_TimevalAsString);
    LONGBOW_RUN_TEST_CASE(Global, parcTime_TimevalAsISO8601);
    LONGBOW_RUN_TEST_CASE(Global, parcTime_TimevalAsRFC3339);
    LONGBOW_RUN_TEST_CASE(Global, parcTime_RFC3339_Now);
    LONGBOW_RUN_TEST_CASE(Global, parcTime_NowTimeval);
    LONGBOW_RUN_TEST_CASE(Global, parcTime_NowMicroseconds);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcTime_TimevalAsString)
{
    struct timeval timeval;
    timeval.tv_sec = 0;
    timeval.tv_usec = 1000;

    char *expected = "0.001000";
    char *actual = parcTime_TimevalAsString(timeval);
    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(Global, parcTime_TimevalAsISO8601)
{
    struct timeval timeval;
    timeval.tv_sec = 0;
    timeval.tv_usec = 1000;

    char *expected = "1970-01-01 00:00:00.001000Z";

    char actual[64];
    parcTime_TimevalAsISO8601(&timeval, actual);

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcTime_TimevalAsRFC3339)
{
    struct timeval timeval;
    timeval.tv_sec = 0;
    timeval.tv_usec = 1000;

    char *expected = "1970-01-01T00:00:00.001000Z";

    char actual[64];
    parcTime_TimevalAsRFC3339(&timeval, actual);

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcTime_RFC3339_Now)
{
    struct timeval timeval;
    gettimeofday(&timeval, NULL);

    char actual[64];
    parcTime_TimevalAsRFC3339(&timeval, actual);
    printf("%s\n", actual);
}

LONGBOW_TEST_CASE(Global, parcTime_NowTimeval)
{
    struct timeval result = parcTime_NowTimeval();
    assertTrue(result.tv_sec != 0, "Expected NOW to not be zero.");
}

LONGBOW_TEST_CASE(Global, parcTime_NowMicroseconds)
{
    uint64_t result = parcTime_NowMicroseconds();
    assertTrue(result != 0, "Expected NOW to not be zero.");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_Time);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
