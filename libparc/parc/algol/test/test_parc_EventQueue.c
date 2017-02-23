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

#include <arpa/inet.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_EventQueue.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_EventQueue.c"

LONGBOW_TEST_RUNNER(parc_EventQueue)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_EventQueue)
{
    parcEventQueue_EnableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_EventQueue)
{
    parcEventQueue_DisableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_SetFileDescriptor_GetFileDecriptor);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_Get_Enable_Disable);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_SetCallbacks);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_Flush);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_Finished);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_SetWatermark);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_ReadWrite);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_SetPriority);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_Printf);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_GetEvBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_ConnectSocket);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_Create_Destroy_Pair);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventQueue_GetUpDownQueue);
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

LONGBOW_TEST_CASE(Global, parc_EventQueue_Create_Destroy)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    parcEventQueue_Destroy(&parcEventQueue);
    assertNull(parcEventQueue, "parcEventQueue_Destroy did not clear reference");
    parcEventScheduler_Destroy(&parcEventScheduler);
    assertNull(parcEventScheduler, "parcEventScheduler_Destroy did not clear reference");
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_Get_Enable_Disable)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    PARCEventType defaultEvents = parcEventQueue_GetEnabled(parcEventQueue);

    parcEventQueue_Enable(parcEventQueue, PARCEventType_Read);
    PARCEventType newEvents = parcEventQueue_GetEnabled(parcEventQueue);
    assertTrue(newEvents == (defaultEvents | PARCEventType_Read),
               "parcEventQueue_GetEnabled returned incorrect event set 0x%x != 0x%x",
               newEvents, defaultEvents | PARCEventType_Read);

    parcEventQueue_Disable(parcEventQueue, PARCEventType_Read);

    newEvents = parcEventQueue_GetEnabled(parcEventQueue);
    assertTrue(defaultEvents == newEvents, "parcEventQueue_GetEnabled returned incorrect event set 0x%x != 0x%x", newEvents, defaultEvents);

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_SetFileDescriptor_GetFileDecriptor)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned null");

    result = parcEventQueue_SetFileDescriptor(parcEventQueue, fds[0]);
    assertTrue(result == 0, " parcEventQueue_SetFileDescriptor call failed");

    result = parcEventQueue_GetFileDescriptor(parcEventQueue);
    assertTrue(result == fds[0], "parcEventQueue_GetFileDescriptor failed");

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_Create_Destroy_Pair)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned null");

    PARCEventQueuePair *parcEventQueuePair = parcEventQueue_CreateConnectedPair(parcEventScheduler);
    assertNotNull(parcEventQueuePair, "parcEventQueue_CreateConnectedPair returned a null pair");

    parcEventQueue_DestroyConnectedPair(&parcEventQueuePair);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_GetUpDownQueue)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();

    PARCEventQueuePair *parcEventQueuePair = parcEventQueue_CreateConnectedPair(parcEventScheduler);

    assertNotNull(parcEventQueue_GetConnectedUpQueue(parcEventQueuePair), "parcEventQueue_GetUpQueue returned null");
    assertNotNull(parcEventQueue_GetConnectedDownQueue(parcEventQueuePair), "parcEventQueue_GetDownQueue returned null");

    parcEventQueue_DestroyConnectedPair(&parcEventQueuePair);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

static int _queue_callback_count = 0;

static void
_queue_callback(PARCEventQueue *event, PARCEventType type, void *data)
{
    _queue_callback_count++;
}

static int _queue_event_callback_count = 0;

static void
_queue_event_callback(PARCEventQueue *event, PARCEventQueueEventType type, void *data)
{
    _queue_event_callback_count++;
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_SetCallbacks)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    parcEventQueue_SetCallbacks(parcEventQueue,
                                _queue_callback,
                                _queue_callback,
                                _queue_event_callback,
                                NULL);

    _parc_queue_read_callback(NULL, parcEventQueue);
    assertTrue(_queue_callback_count == 1, "Callback count expected 1 got %d", _queue_callback_count);

    _parc_queue_write_callback(NULL, parcEventQueue);
    assertTrue(_queue_callback_count == 2, "Callback count expected 2 got %d", _queue_callback_count);

    _parc_queue_event_callback(NULL, PARCEventQueueEventType_EOF, parcEventQueue);
    assertTrue(_queue_event_callback_count == 1, "Callback event count expected 1 got %d", _queue_event_callback_count);

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_Flush)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    int result = parcEventQueue_Flush(parcEventQueue, PARCEventType_Read);
    assertTrue(result == 0, "parcEventQueue_Flush failed with %d", result);

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_Finished)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    int result = parcEventQueue_Finished(parcEventQueue, PARCEventType_Read);
    assertTrue(result == 0, "parcEventQueue_Finished failed with %d", result);

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_SetWatermark)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    parcEventQueue_SetWatermark(parcEventQueue, PARCEventType_Read, 0, 0);

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_ReadWrite)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    char *data = "Hello World\n";
    int length = strlen(data);
    int result = parcEventQueue_Write(parcEventQueue, data, length);
    assertTrue(result == 0, "parcEventQueue_Write failed.");

    char buffer[64];
    result = parcEventQueue_Read(parcEventQueue, buffer, 64);
    assertTrue(result == 0, "parcEventQueue_Read failed.");

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

static int _test_writeMaxPriority_event_called = 0;
static void
_test_writeMaxPriority_callback(PARCEventQueue *parcEventQueue, PARCEventType event, void *data)
{
    PARCEventQueue *parcEventQueuePartner = *((PARCEventQueue **) data);
    parcEventQueue_Disable(parcEventQueuePartner, event);
    _test_writeMaxPriority_event_called++;
}

static int _test_writeMinPriority_event_called = 0;
static void
_test_writeMinPriority_callback(PARCEventQueue *parcEventQueue, PARCEventType event, void *data)
{
    PARCEventQueue *parcEventQueuePartner = *((PARCEventQueue **) data);
    parcEventQueue_Disable(parcEventQueuePartner, event);
    _test_writeMinPriority_event_called++;
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_SetPriority)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    //
    // First queue to be called back disables its partner's queue
    //
    PARCEventQueue *parcEventQueueMin, *parcEventQueueMax;
    parcEventQueueMin = parcEventQueue_Create(parcEventScheduler, fds[0], PARCEventQueueOption_CloseOnFree);
    assertNotNull(parcEventQueueMin, "parcEventQueue_Create returned a null reference");
    parcEventQueue_SetCallbacks(parcEventQueueMin, NULL, _test_writeMinPriority_callback, NULL, (void *) &parcEventQueueMax);

    parcEventQueueMax = parcEventQueue_Create(parcEventScheduler, fds[0], PARCEventQueueOption_CloseOnFree);
    assertNotNull(parcEventQueueMax, "parcEventQueue_Create returned a null reference");
    parcEventQueue_SetCallbacks(parcEventQueueMax, NULL, _test_writeMaxPriority_callback, NULL, (void *) &parcEventQueueMin);

    result = parcEventQueue_SetPriority(parcEventQueueMin, PARCEventPriority_Minimum);
    assertTrue(result == 0, "parcEventQueue_SetPriority Minimum priority failed.");
    result = parcEventQueue_SetPriority(parcEventQueueMax, PARCEventPriority_Maximum);
    assertTrue(result == 0, "parcEventQueue_SetPriority Maximum priority failed.");

    parcEventQueue_Enable(parcEventQueueMin, PARCEventType_Write);
    parcEventQueue_Enable(parcEventQueueMax, PARCEventType_Write);

    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_NonBlocking);

    assertTrue(_test_writeMaxPriority_event_called == 1, "Read event called before priority write event handled");
    assertTrue(_test_writeMinPriority_event_called == 0, "Write event never triggered");

    parcEventQueue_Destroy(&parcEventQueueMin);
    parcEventQueue_Destroy(&parcEventQueueMax);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_Printf)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    int result = parcEventQueue_Printf(parcEventQueue, "%s %s\n", "Hello", "World");
    assertTrue(result == 12, "parcEventQueue_Printf didn't write expected length %d != %d", result, 12);

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_GetEvBuffer)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, 0, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    struct evbuffer *result = internal_parcEventQueue_GetEvInputBuffer(parcEventQueue);
    assertTrue(result != NULL, "parcEventQueue_GetEvInputBuffer failed.");

    result = internal_parcEventQueue_GetEvOutputBuffer(parcEventQueue);
    assertTrue(result != NULL, "parcEventQueue_GetEvOutputBuffer failed.");

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventQueue_ConnectSocket)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, -1, 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    struct sockaddr_in address;
    int addressLength = sizeof(address);
    memset(&address, 0, addressLength);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
    address.sin_port = htons(8080); /* Port 8080 */

    int result = parcEventQueue_ConnectSocket(parcEventQueue, (struct sockaddr *) &address, addressLength);
    assertTrue(result == 0, "parcEventQueue_ConnectSocket returned %d", result);

    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_EventQueue);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
