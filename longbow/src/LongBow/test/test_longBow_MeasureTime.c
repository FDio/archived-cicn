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
// This permits internal static functions to be visible to this Test Runner.
#include "../longBow_MeasureTime.c"

#include <stdint.h>
#include <inttypes.h>
#include <LongBow/private/longBow_Memory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(longBow_MeasureTime)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(longBow_MeasureTime)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(longBow_MeasureTime)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, longBowMeasureTime_CountDown);
    LONGBOW_RUN_TEST_CASE(Global, longBowMeasureTime_Report);
    LONGBOW_RUN_TEST_CASE(Global, longBowMeasureTime_Start);
    LONGBOW_RUN_TEST_CASE(Global, longBowMeasureTime_Stop);
    LONGBOW_RUN_TEST_CASE(Global, longBowMeasureTime_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, longBowMeasureTime);
}

uint64_t _setupAllocations;

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

LONGBOW_TEST_CASE(Global, longBowMeasureTime_CountDown)
{
    LongBowMeasureTime *measure = longBowMeasureTime_Start(1);
    assertTrue(measure->iterations == 1, "Expected iterations to be 1, actual %d", measure->iterations);

    longBowMeasureTime_CountDown(measure);
    assertTrue(measure->iterations == 0, "Expected iterations to be 0, actual %d", measure->iterations);

    longBowMeasureTime_Destroy(&measure);
}

LONGBOW_TEST_CASE(Global, longBowMeasureTime_Report)
{
    LongBowMeasureTime *measure = longBowMeasureTime_Start(1);
    assertTrue(measure->iterations == 1, "Expected iterations to be 1, actual %d", measure->iterations);

    longBowMeasureTime_Report(measure, __FILE__, __func__, __LINE__);

    longBowMeasureTime_Destroy(&measure);
}

LONGBOW_TEST_CASE(Global, longBowMeasureTime_Start)
{
    LongBowMeasureTime *measure = longBowMeasureTime_Start(1);
    assertNotNull(measure, "Expected longBowMeasureTime_Start to return non-NULL result.");

    longBowMeasureTime_Destroy(&measure);
}

LONGBOW_TEST_CASE(Global, longBowMeasureTime_Stop)
{
    LongBowMeasureTime *measure = longBowMeasureTime_Start(1);
    assertNotNull(measure, "Expected longBowMeasureTime_Start to return non-NULL result.");
    sleep(2);
    longBowMeasureTime_Stop(measure);

    uint64_t nanos = longBowMeasureTime_GetNanoseconds(measure);
    assertTrue(nanos >= 1000000000ULL, "Expected more than 1,000,000 ns to have elapsed.");

    longBowMeasureTime_Destroy(&measure);
}

LONGBOW_TEST_CASE(Global, longBowMeasureTime_Destroy)
{
    LongBowMeasureTime *measure = longBowMeasureTime_Start(1);
    assertNotNull(measure, "Expected longBowMeasureTime_Start to return non-NULL result.");

    longBowMeasureTime_Destroy(&measure);
    assertNull(measure, "Expected longBowMeasureTime_Destroy to NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, longBowMeasureTime)
{
    longBowMeasureTime(1)
    {
        sleep(2);
    }
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
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(longBow_MeasureTime);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
