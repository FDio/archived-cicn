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
 * Create a non-threaded framework to test comopnent functions.
 *
 */
#define DEBUG_OUTPUT 1
#include "../rta_Component.c"

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/config/config_All.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_Commands.c>
#include <ccnx/transport/test_tools/traffic_tools.h>

#include <sys/socket.h>
#include <errno.h>

#define PAIR_OTHER 0
#define PAIR_TRANSPORT 1

typedef struct test_data {
    PARCRingBuffer1x1 *commandRingBuffer;
    PARCNotifier *commandNotifier;

    int api_fds[2];
    RtaFramework *framework;
    RtaProtocolStack *stack;
    RtaConnection *connection;
} TestData;

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    int error = socketpair(AF_UNIX, SOCK_STREAM, 0, data->api_fds);
    assertFalse(error, "Error creating socket pair: (%d) %s", errno, strerror(errno));

    data->commandRingBuffer = parcRingBuffer1x1_Create(128, NULL);
    data->commandNotifier = parcNotifier_Create();
    data->framework = rtaFramework_Create(data->commandRingBuffer, data->commandNotifier);

    assertNotNull(data->framework, "rtaFramework_Create returned null");

    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();
    apiConnector_ProtocolStackConfig(stackConfig);
    testingLower_ProtocolStackConfig(stackConfig);
    protocolStack_ComponentsConfigArgs(stackConfig, apiConnector_GetName(), testingLower_GetName(), NULL);

    rtaFramework_NonThreadedStepCount(data->framework, 10);

    int stackId = 1;
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, stackConfig);
    _rtaFramework_ExecuteCreateStack(data->framework, createStack);
    rtaCommandCreateProtocolStack_Release(&createStack);

    rtaFramework_NonThreadedStepCount(data->framework, 10);
    data->stack = (rtaFramework_GetProtocolStackByStackId(data->framework, stackId))->stack;

    CCNxConnectionConfig *connConfig = ccnxConnectionConfig_Create();
    apiConnector_ConnectionConfig(connConfig);

    tlvCodec_ConnectionConfig(connConfig);

    testingLower_ConnectionConfig(connConfig);

    RtaCommandOpenConnection *openConnection = rtaCommandOpenConnection_Create(stackId,
                                                                               data->api_fds[PAIR_OTHER],
                                                                               data->api_fds[PAIR_TRANSPORT],
                                                                               ccnxConnectionConfig_GetJson(connConfig));

    rtaFramework_NonThreadedStepCount(data->framework, 10);
    _rtaFramework_ExecuteOpenConnection(data->framework, openConnection);
    rtaCommandOpenConnection_Release(&openConnection);

    rtaFramework_NonThreadedStepCount(data->framework, 10);
    data->connection = rtaConnectionTable_GetByApiFd(data->framework->connectionTable, data->api_fds[PAIR_OTHER]);

    ccnxConnectionConfig_Destroy(&connConfig);
    ccnxStackConfig_Release(&stackConfig);

    return data;
}

static void
_commonTeardown(TestData *data)
{
    rtaFramework_Teardown(data->framework);

    parcRingBuffer1x1_Release(&data->commandRingBuffer);
    parcNotifier_Release(&data->commandNotifier);
    rtaFramework_Destroy(&data->framework);

    close(data->api_fds[0]);
    close(data->api_fds[1]);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(rta_Component)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_Component)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_Component)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, rtaComponent_GetOutputQueue);

    LONGBOW_RUN_TEST_CASE(Global, rtaComponent_PutMessage_ClosedConnection);
    LONGBOW_RUN_TEST_CASE(Global, rtaComponent_PutMessage_OpenConnection);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, rtaComponent_GetOutputQueue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCEventQueue *queue = rtaComponent_GetOutputQueue(data->connection, API_CONNECTOR, RTA_DOWN);
    assertNotNull(queue, "Got null queue for API_CONNECTOR DOWN queue");
}

LONGBOW_TEST_CASE(Global, rtaComponent_PutMessage_ClosedConnection)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    rtaConnection_SetState(data->connection, CONN_CLOSED);

    // Create the TransportMessage to put on the queue
    TransportMessage *tm = trafficTools_CreateTransportMessageWithDictionaryControl(data->connection, CCNxTlvDictionary_SchemaVersion_V1);

    // Send it down from the API connector to the Testing Lower component
    PARCEventQueue *outputQueue = rtaComponent_GetOutputQueue(data->connection, API_CONNECTOR, RTA_DOWN);

    int success = rtaComponent_PutMessage(outputQueue, tm);
    assertFalse(success, "Error putting message on API Connector's down queue");

    // check that we got it
    PARCEventQueue *inputQueue = rtaComponent_GetOutputQueue(data->connection, TESTING_LOWER, RTA_UP);

    TransportMessage *test_tm = rtaComponent_GetMessage(inputQueue);
    assertNull(test_tm, "Should have returned NULL on a closed connection");

    // The transport message was destroyed by PutMessage because the connection
    // was closed.  Don't need to destroy the transport message.

    // set state back to OPEN so the connection is properly disposed of
    rtaConnection_SetState(data->connection, CONN_OPEN);
}

LONGBOW_TEST_CASE(Global, rtaComponent_PutMessage_OpenConnection)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // Create the TransportMessage to put on the queue
    TransportMessage *tm = trafficTools_CreateTransportMessageWithDictionaryControl(data->connection, CCNxTlvDictionary_SchemaVersion_V1);

    // Send it down from the API connector to the Testing Lower component
    PARCEventQueue *outputQueue = rtaComponent_GetOutputQueue(data->connection, API_CONNECTOR, RTA_DOWN);

    int success = rtaComponent_PutMessage(outputQueue, tm);
    assertTrue(success, "Error putting message on API Connector's down queue");

    // check that we got it
    PARCEventQueue *inputQueue = rtaComponent_GetOutputQueue(data->connection, TESTING_LOWER, RTA_UP);

    TransportMessage *test_tm = rtaComponent_GetMessage(inputQueue);
    assertTrue(test_tm == tm, "Got wrong message, got %p expected %p", (void *) test_tm, (void *) tm);

    transportMessage_Destroy(&tm);
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
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_Component);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
