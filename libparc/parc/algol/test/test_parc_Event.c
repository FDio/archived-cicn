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

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Event.c"

LONGBOW_TEST_RUNNER(parc_Event)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Event)
{
    parcEvent_EnableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Event)
{
    parcEvent_DisableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_Event_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parc_Event_Start);
    LONGBOW_RUN_TEST_CASE(Global, parc_Event_Stop);
    LONGBOW_RUN_TEST_CASE(Global, parc_Event_Poll);
    LONGBOW_RUN_TEST_CASE(Global, parc_Event_SetPriority);
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

LONGBOW_TEST_CASE(Global, parc_Event_Create_Destroy)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEvent *parcEvent = parcEvent_Create(parcEventScheduler, fds[0], PARCEventType_Write, _test_event, NULL);
    assertNotNull(parcEvent, "parcEvent_Create returned a null reference");
    parcEvent_Destroy(&parcEvent);
    parcEventScheduler_Destroy(&parcEventScheduler);

    close(fds[0]);
    close(fds[1]);
}

LONGBOW_TEST_CASE(Global, parc_Event_Start)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEvent *parcEvent = parcEvent_Create(parcEventScheduler, fds[0], PARCEventType_Read | PARCEventType_Write, _test_event, NULL);
    assertNotNull(parcEvent, "parcEvent_Create returned a null reference");

    _test_event_called = 0;
    parcEvent_Start(parcEvent);
    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    assertTrue(_test_event_called == 1, "Event never called.");

    parcEvent_Destroy(&parcEvent);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

static int _test_stop_event_called = 0;

static void
_test_stop_event(int fd, PARCEventType flags, void *data)
{
    PARCEvent **parcEvent = (PARCEvent **) data;
    _test_stop_event_called++;
    parcEvent_Stop(*parcEvent);
}

LONGBOW_TEST_CASE(Global, parc_Event_Stop)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEvent *parcEvent = parcEvent_Create(parcEventScheduler, fds[0], PARCEventType_Write | PARCEventType_Persist, _test_stop_event, &parcEvent);
    assertNotNull(parcEvent, "parcEvent_Create returned a null reference");

    parcEvent_Start(parcEvent);
    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    assertTrue(_test_stop_event_called > 0, "Event never called.");
    assertFalse(_test_stop_event_called != 1, "Event called more than once.");

    parcEvent_Destroy(&parcEvent);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

LONGBOW_TEST_CASE(Global, parc_Event_Poll)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEvent *parcEvent = parcEvent_Create(parcEventScheduler, fds[0], PARCEventType_Write, _test_event, NULL);
    assertNotNull(parcEvent, "parcEvent_Create returned a null reference");

    result = parcEvent_Poll(parcEvent, PARCEventType_Read);
    // should be no outstanding events
    assertTrue(result == 0, "parcEvent_Poll returned %d\n", result);

    parcEvent_Destroy(&parcEvent);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

static int _test_writeMaxPriority_event_called = 0;

static void
_test_writeMaxPriority_event(int fd, PARCEventType flags, void *data)
{
    PARCEvent *parcEvent = *((PARCEvent **) data);
    parcEvent_Stop(parcEvent);
    _test_writeMaxPriority_event_called++;
}

static int _test_writeMinPriority_event_called = 0;

static void
_test_writeMinPriority_event(int fd, PARCEventType flags, void *data)
{
    PARCEvent *parcEvent = *((PARCEvent **) data);
    parcEvent_Stop(parcEvent);
    _test_writeMinPriority_event_called++;
}

LONGBOW_TEST_CASE(Global, parc_Event_SetPriority)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    //
    // First event to be called back disables its partner's event
    //
    PARCEvent *parcEventWriteMax, *parcEventWriteMin;
    parcEventWriteMax = parcEvent_Create(parcEventScheduler, fds[0],
                                         PARCEventType_Write,
                                         _test_writeMaxPriority_event,
                                         (void *) &parcEventWriteMin);
    assertNotNull(parcEventWriteMax, "parcEvent_Create returned a null reference");
    parcEventWriteMin = parcEvent_Create(parcEventScheduler, fds[1],
                                         PARCEventType_Write,
                                         _test_writeMinPriority_event,
                                         (void *) &parcEventWriteMax);
    assertNotNull(parcEventWriteMin, "parcEvent_Create returned a null reference");

    result = parcEvent_SetPriority(parcEventWriteMin, PARCEventPriority_Minimum);
    assertTrue(result == 0, "parcEvent_SetPriority write returned %d\n", result);
    result = parcEvent_SetPriority(parcEventWriteMax, PARCEventPriority_Maximum);
    assertTrue(result == 0, "parcEvent_SetPriority read returned %d\n", result);

    parcEvent_Start(parcEventWriteMin);
    parcEvent_Start(parcEventWriteMax);

    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_NonBlocking);

    assertTrue(_test_writeMaxPriority_event_called == 1, "Read event called before priority write event handled");
    assertTrue(_test_writeMinPriority_event_called == 0, "Write event never triggered");

    parcEvent_Destroy(&parcEventWriteMax);
    parcEvent_Destroy(&parcEventWriteMin);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Event);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
