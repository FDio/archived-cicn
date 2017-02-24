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
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Clock.c"
#include <stdio.h>
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(parc_Clock)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Clock)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Clock)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==========================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Wallclock);
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Wallclock_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Wallclock_GetTime);
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Wallclock_GetTimeval);
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Montonic);
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Monotonic_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Monotonic_GetTime);
    LONGBOW_RUN_TEST_CASE(Global, parcClock_Monotonic_GetTimeval);

    LONGBOW_RUN_TEST_CASE(Global, counterClock_Create);
    LONGBOW_RUN_TEST_CASE(Global, counterClock_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, counterClock_GetTime);
    LONGBOW_RUN_TEST_CASE(Global, counterClock_GetTime_Twice);
    LONGBOW_RUN_TEST_CASE(Global, counterClock_GetTimeval);
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

LONGBOW_TEST_CASE(Global, parcClock_Wallclock)
{
    PARCClock *clock = parcClock_Wallclock();
    assertNotNull(clock, "Got null wall clock");
    parcClock_Release(&clock);
}

LONGBOW_TEST_CASE(Global, parcClock_Wallclock_Acquire)
{
    PARCClock *clock = parcClock_Wallclock();
    PARCClock *copy = parcClock_Acquire(clock);
    assertNotNull(copy, "Got null wall clock");
    parcClock_Release(&copy);
    parcClock_Release(&clock);
}

LONGBOW_TEST_CASE(Global, parcClock_Wallclock_GetTime)
{
    PARCClock *clock = parcClock_Wallclock();
    uint64_t t = parcClock_GetTime(clock);
    parcClock_Release(&clock);
    assertTrue(t > 0, "got 0 time");
}

LONGBOW_TEST_CASE(Global, parcClock_Wallclock_GetTimeval)
{
    PARCClock *clock = parcClock_Wallclock();
    struct timeval tv = { 0, 0 };
    parcClock_GetTimeval(clock, &tv);
    parcClock_Release(&clock);
    assertTrue(tv.tv_sec > 0, "Got 0 seconds");
}

// -----

LONGBOW_TEST_CASE(Global, parcClock_Montonic)
{
    PARCClock *clock = parcClock_Monotonic();
    assertNotNull(clock, "Got null wall clock");
    parcClock_Release(&clock);
}

LONGBOW_TEST_CASE(Global, parcClock_Monotonic_Acquire)
{
    PARCClock *clock = parcClock_Monotonic();
    PARCClock *copy = parcClock_Acquire(clock);
    assertNotNull(copy, "Got null wall clock");
}

LONGBOW_TEST_CASE(Global, parcClock_Monotonic_GetTime)
{
    PARCClock *clock = parcClock_Monotonic();
    uint64_t t = parcClock_GetTime(clock);
    parcClock_Release(&clock);
    assertTrue(t > 0, "got 0 time");
}

LONGBOW_TEST_CASE(Global, parcClock_Monotonic_GetTimeval)
{
    PARCClock *clock = parcClock_Monotonic();
    struct timeval tv = { 0, 0 };
    parcClock_GetTimeval(clock, &tv);
    parcClock_Release(&clock);
    assertTrue(tv.tv_sec > 0, "Got 0 seconds");
}

// -----

LONGBOW_TEST_CASE(Global, counterClock_Create)
{
    PARCClock *clock = parcClock_Counter();
    assertNotNull(clock, "Got null wall clock");
    parcClock_Release(&clock);
}

LONGBOW_TEST_CASE(Global, counterClock_Acquire)
{
    PARCClock *clock = parcClock_Counter();
    PARCClock *copy = parcClock_Acquire(clock);
    assertNotNull(copy, "Got null wall clock");
    parcClock_Release(&copy);
    parcClock_Release(&clock);
}

LONGBOW_TEST_CASE(Global, counterClock_GetTime)
{
    PARCClock *clock = parcClock_Counter();
    uint64_t t = parcClock_GetTime(clock);
    parcClock_Release(&clock);
    assertTrue(t == 1, "On first call should have gotten 1");
}

LONGBOW_TEST_CASE(Global, counterClock_GetTime_Twice)
{
    PARCClock *clock = parcClock_Counter();
    parcClock_GetTime(clock);
    uint64_t t2 = parcClock_GetTime(clock);
    parcClock_Release(&clock);
    assertTrue(t2 == 2, "On second call should have gotten 2");
}

LONGBOW_TEST_CASE(Global, counterClock_GetTimeval)
{
    PARCClock *clock = parcClock_Counter();
    struct timeval tv = { 0, 0 };
    parcClock_GetTimeval(clock, &tv);
    parcClock_Release(&clock);
    assertTrue(tv.tv_usec == 1, "On first call should have gotten 1 usec");
}


// ==========================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Clock);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}

