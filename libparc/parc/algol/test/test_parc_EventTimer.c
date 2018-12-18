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
#include <pthread.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_EventTimer.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_EventTimer.c"

LONGBOW_TEST_RUNNER(parc_EventTimer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_EventTimer)
{
    parcEventTimer_EnableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_EventTimer)
{
    parcEventTimer_DisableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_EventTimer_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventTimer_Start);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventTimer_Stop);
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

static int _test_event_called = 0;

static void
_test_event(int fd, PARCEventType flags, void *data)
{
    _test_event_called++;
}

LONGBOW_TEST_CASE(Global, parc_EventTimer_Create_Destroy)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventTimer *parcEventTimer = parcEventTimer_Create(parcEventScheduler, PARCEventType_None, _test_event, NULL);
    assertNotNull(parcEventTimer, "parcEventTimer_Create returned a null reference");

    parcEventTimer_Destroy(&parcEventTimer);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventTimer_Start)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventTimer *parcEventTimer = parcEventTimer_Create(parcEventScheduler, PARCEventType_None, _test_event, NULL);
    assertNotNull(parcEventTimer, "parcEventTimer_Create returned a null reference");

    struct timeval timeout = { 1, 0 };
    parcEventTimer_Start(parcEventTimer, &timeout);
    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    assertTrue(_test_event_called == 1, "Event never called.");

    parcEventTimer_Destroy(&parcEventTimer);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

static void
_test_stop_event(int fd, PARCEventType flags, void *data)
{
    PARCEventTimer **parcEventTimer = (PARCEventTimer **) data;
    _test_event_called++;
    parcEventTimer_Stop(*parcEventTimer);
}

LONGBOW_TEST_CASE(Global, parc_EventTimer_Stop)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventTimer *parcEventTimer = parcEventTimer_Create(parcEventScheduler, PARCEventType_None | PARCEventType_Persist, _test_stop_event, &parcEventTimer);
    assertNotNull(parcEventTimer, "parcEventTimer_Create returned a null reference");

    _test_event_called = 0;
    struct timeval timeout = { 1, 0 };
    parcEventTimer_Start(parcEventTimer, &timeout);
    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    assertTrue(_test_event_called == 1, "Event never called.");

    parcEventTimer_Destroy(&parcEventTimer);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_EventTimer);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
