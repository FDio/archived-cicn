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
#include "../metisControl_ListInterfaces.c"
#include "testrig_MetisControl.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metisControl_ListInterfaces)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metisControl_ListInterfaces)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metisControl_ListInterfaces)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisControlListInterfaces_HelpCreate);
    LONGBOW_RUN_TEST_CASE(Global, metisControlListInterfaces_Create);
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

LONGBOW_TEST_CASE(Global, metisControlListInterfaces_HelpCreate)
{
    testCommandCreate(testCase, &metisControlListInterfaces_HelpCreate, __func__);
}

LONGBOW_TEST_CASE(Global, metisControlListInterfaces_Create)
{
    testCommandCreate(testCase, &metisControlListInterfaces_Create, __func__);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisControl_Help_ListInterfaces_Execute);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ListInterfaces_Execute_WrongArgCount);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ListInterfaces_Execute_Good);
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

LONGBOW_TEST_CASE(Local, metisControl_Help_ListInterfaces_Execute)
{
    testHelpExecute(testCase, &metisControlListInterfaces_HelpCreate, __func__, MetisCommandReturn_Success);
}

LONGBOW_TEST_CASE(Local, metisControl_ListInterfaces_Execute)
{
    testUnimplemented("");
}

static CCNxControl *
customWriteReadResponse(void *userdata, CCNxMetaMessage *messageToWrite)
{
    CPIInterfaceSet *set = cpiInterfaceSet_Create();
    cpiInterfaceSet_Add(set, cpiInterface_Create("abc0", 1, false, true, 1500));
    cpiInterfaceSet_Add(set, cpiInterface_Create("abc1", 2, false, true, 1500));
    PARCJSON *setJson = cpiInterfaceSet_ToJson(set);

    CCNxControl *inboundControlMessage = ccnxMetaMessage_GetControl(messageToWrite);

    // Create a response to the inbound Control message.
    CCNxControl *outboundControlMessage = cpi_CreateResponse(inboundControlMessage, setJson);
    parcJSON_Release(&setJson);

    ccnxControl_Release(&inboundControlMessage);
    cpiInterfaceSet_Destroy(&set);

    return outboundControlMessage;
}

static MetisCommandReturn
testListInterfaces(const LongBowTestCase *testCase, int argc)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    metisControlState_SetDebug(data->state, true);
    data->customWriteReadReply = &customWriteReadResponse;

    const char *argv[] = { "list", "connections" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, argc, (void **) &argv[0]);

    MetisCommandOps *ops = metisControlListInterfaces_Create(data->state);

    MetisCommandReturn result = ops->execute(data->state->parser, ops, args);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);
    return result;
}

LONGBOW_TEST_CASE(Local, metisControl_ListInterfaces_Execute_WrongArgCount)
{
    // argc is wrong, needs to be 2.
    MetisCommandReturn result = testListInterfaces(testCase, 3);

    assertTrue(result == MetisCommandReturn_Failure,
               "metisControl_ListInterfaces with wrong argc should return %d, got %d", MetisCommandReturn_Failure, result);
}

LONGBOW_TEST_CASE(Local, metisControl_ListInterfaces_Execute_Good)
{
    MetisCommandReturn result = testListInterfaces(testCase, 2);

    assertTrue(result == MetisCommandReturn_Success,
               "metisControl_ListInterfaces should return %d, got %d", MetisCommandReturn_Success, result);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metisControl_ListInterfaces);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
