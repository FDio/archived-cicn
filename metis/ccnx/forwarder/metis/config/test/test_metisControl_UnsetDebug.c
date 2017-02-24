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
#include "../metisControl_UnsetDebug.c"
#include "testrig_MetisControl.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metisControl_UnsetDebug)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metisControl_UnsetDebug)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metisControl_UnsetDebug)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisControlUnsetDebug_HelpCreate);
    LONGBOW_RUN_TEST_CASE(Global, metisControlUnsetDebug_Create);
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

LONGBOW_TEST_CASE(Global, metisControlUnsetDebug_HelpCreate)
{
    testCommandCreate(testCase, &metisControlUnsetDebug_HelpCreate, __func__);
}

LONGBOW_TEST_CASE(Global, metisControlUnsetDebug_Create)
{
    testCommandCreate(testCase, &metisControlUnsetDebug_Create, __func__);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisControl_Help_UnsetDebug_Execute);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_UnsetDebug_Execute_WrongArgCount);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_UnsetDebug_Execute_Good);
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

LONGBOW_TEST_CASE(Local, metisControl_Help_UnsetDebug_Execute)
{
    testHelpExecute(testCase, &metisControlUnsetDebug_HelpCreate, __func__, MetisCommandReturn_Success);
}

static MetisCommandReturn
testDebug(const LongBowTestCase *testCase, MetisCommandOps * (*create)(MetisControlState * state), int argc, bool initialDebugSetting, bool expectedDebugSetting)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "blah", "blah" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, argc, (void **) &argv[0]);

    metisControlState_SetDebug(data->state, initialDebugSetting);
    MetisCommandOps *ops = create(data->state);
    MetisCommandReturn result = ops->execute(data->state->parser, ops, args);
    if (result == MetisCommandReturn_Success) {
        assertTrue(data->state->debugFlag == expectedDebugSetting,
                   "Debug flag wrong, expected %d got %d",
                   expectedDebugSetting,
                   data->state->debugFlag);
    }

    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);
    return result;
}

LONGBOW_TEST_CASE(Local, metisControl_UnsetDebug_Execute_WrongArgCount)
{
    MetisCommandReturn result = testDebug(testCase, metisControlUnsetDebug_Create, 3, true, false);
    assertTrue(result == MetisCommandReturn_Failure,
               "metisControl_UnsetDebug_Execute should return %d, got %d", MetisCommandReturn_Failure, result);
}


LONGBOW_TEST_CASE(Local, metisControl_UnsetDebug_Execute_Good)
{
    MetisCommandReturn result = testDebug(testCase, metisControlUnsetDebug_Create, 2, true, false);
    assertTrue(result == MetisCommandReturn_Success,
               "metisControl_UnsetDebug_Execute should return %d, got %d", MetisCommandReturn_Success, result);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metisControl_UnsetDebug);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
