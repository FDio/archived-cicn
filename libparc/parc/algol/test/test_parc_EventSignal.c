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
#include <parc/algol/parc_EventSignal.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_EventSignal.c"

LONGBOW_TEST_RUNNER(parc_EventSignal)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_EventSignal)
{
    parcEventSignal_EnableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_EventSignal)
{
    parcEventSignal_DisableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_EventSignal_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventSignal_Start_Stop);
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

static int _empty_event_called = 0;

static void
_empty_event(int fd, PARCEventType flags, void *data)
{
    _empty_event_called++;
}

LONGBOW_TEST_CASE(Global, parc_EventSignal_Create_Destroy)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventSignal *parcEventSignal = parcEventSignal_Create(parcEventScheduler, SIGUSR1, PARCEventType_Signal | PARCEventType_Persist, _empty_event, NULL);
    assertNotNull(parcEventSignal, "parcEventSignal_Create returned a null reference");

    _parc_event_signal_callback(0, 0, (void *) parcEventSignal);
    assertTrue(_empty_event_called == 1, "Event handler never called.");
    parcEventSignal_Destroy(&parcEventSignal);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

#include <pthread.h>

static void *
_run_scheduler(void *data)
{
    PARCEventScheduler *parcEventScheduler = (PARCEventScheduler *) data;
    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    return NULL;
}

static int _test_event_called = 0;

static void
_signal_event(int fd, PARCEventType flags, void *data)
{
    PARCEventSignal **parcEventSignal = (PARCEventSignal **) data;
    _test_event_called++;
    parcEventSignal_Stop(*parcEventSignal);
}

LONGBOW_TEST_CASE(Global, parc_EventSignal_Start_Stop)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEventSignal *parcEventSignal = parcEventSignal_Create(parcEventScheduler, SIGUSR1, PARCEventType_Signal | PARCEventType_Persist, _signal_event, &parcEventSignal);
    assertNotNull(parcEventSignal, "parcEventSignal_Create returned a null reference");

    parcEventSignal_Start(parcEventSignal);

    pthread_t thread;
    pthread_create(&thread, NULL, _run_scheduler, parcEventScheduler);

    kill(getpid(), SIGUSR1);
    pthread_join(thread, NULL);
    assertTrue(_test_event_called == 1, "Event never called.");

    parcEventSignal_Destroy(&parcEventSignal);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

static void
_test_stop_event(int fd, PARCEventType flags, void *data)
{
    PARCEventSignal **parcEventSignal = (PARCEventSignal **) data;
    _test_event_called++;
    parcEventSignal_Stop(*parcEventSignal);
}

LONGBOW_TEST_CASE(Global, parc_EventSignal_Stop)
{
    _test_event_called = 0;
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEventSignal *parcEventSignal = parcEventSignal_Create(parcEventScheduler, SIGUSR1, PARCEventType_Signal | PARCEventType_Persist, _test_stop_event, &parcEventSignal);
    assertNotNull(parcEventSignal, "parcEventSignal_Create returned a null reference");

    parcEventSignal_Start(parcEventSignal);
    kill(getpid(), SIGUSR1);

    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    assertTrue(_test_event_called == 1, "Event never called.");

    parcEventSignal_Destroy(&parcEventSignal);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_EventSignal);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
