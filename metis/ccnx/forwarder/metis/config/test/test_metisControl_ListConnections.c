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
// This permits internal static functions to be visible to this Test Framework.
#include "../metisControl_ListConnections.c"
#include "testrig_MetisControl.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metisControl_ListConnections)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metisControl_ListConnections)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metisControl_ListConnections)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisControlListConnections_HelpCreate);
    LONGBOW_RUN_TEST_CASE(Global, metisControlListConnections_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    testrigMetisControl_commonSetup(testCase);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    testrigMetisControl_CommonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisControlListConnections_HelpCreate)
{
    testCommandCreate(testCase, &metisControlListConnections_HelpCreate, __func__);
}

LONGBOW_TEST_CASE(Global, metisControlListConnections_Create)
{
    testCommandCreate(testCase, &metisControlListConnections_Create, __func__);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisControl_Help_ListConnections_Execute);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ListConnections_Execute_WrongArgCount);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ListConnections_Execute_Good);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    testrigMetisControl_commonSetup(testCase);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    testrigMetisControl_CommonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, metisControl_Help_ListConnections_Execute)
{
    testHelpExecute(testCase, &metisControlListConnections_HelpCreate, __func__, MetisCommandReturn_Success);
}

static CCNxControl *
customWriteReadResponse(void *userdata, CCNxMetaMessage *messageToWrite)
{
    CPIConnectionList *connlist = cpiConnectionList_Create();
    CPIConnection *conn = cpiConnection_Create(1, cpiAddress_CreateFromInterface(1), cpiAddress_CreateFromInterface(2), cpiConnection_L2);
    cpiConnectionList_Append(connlist, conn);

    PARCJSON *connectionListAsJson = cpiConnectionList_ToJson(connlist);

    CCNxControl *inboundControlMessage = ccnxMetaMessage_GetControl(messageToWrite);

    // Create a response to the inbound Control message.
    CCNxControl *outboundControlMessage = cpi_CreateResponse(inboundControlMessage, connectionListAsJson);
    parcJSON_Release(&connectionListAsJson);

    ccnxControl_Release(&inboundControlMessage);

    cpiConnectionList_Destroy(&connlist);

    return outboundControlMessage;
}

static MetisCommandReturn
testListConnections(const LongBowTestCase *testCase, int argc)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    metisControlState_SetDebug(data->state, true);
    data->customWriteReadReply = &customWriteReadResponse;

    const char *argv[] = { "list", "interfaces" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, argc, (void **) &argv[0]);

    MetisCommandOps *ops = metisControlListConnections_Create(data->state);

    MetisCommandReturn result = ops->execute(data->state->parser, ops, args);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);
    return result;
}

LONGBOW_TEST_CASE(Local, metisControl_ListConnections_Execute_WrongArgCount)
{
    // argc is wrong, needs to be 2.
    MetisCommandReturn result = testListConnections(testCase, 3);

    assertTrue(result == MetisCommandReturn_Failure,
               "metisControl_ListConnections with wrong argc should return %d, got %d", MetisCommandReturn_Failure, result);
}

LONGBOW_TEST_CASE(Local, metisControl_ListConnections_Execute_Good)
{
    MetisCommandReturn result = testListConnections(testCase, 2);

    assertTrue(result == MetisCommandReturn_Success,
               "metisControl_ListConnections should return %d, got %d", MetisCommandReturn_Success, result);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metisControl_ListConnections);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
