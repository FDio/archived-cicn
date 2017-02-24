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
#include "../metisControl_AddRoute.c"
#include "testrig_MetisControl.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metisControl_AddRoute)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metisControl_AddRoute)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metisControl_AddRoute)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisControlAddRoute_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisControlAddRoute_HelpCreate);
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

LONGBOW_TEST_CASE(Global, metisControlAddRoute_Create)
{
    testCommandCreate(testCase, &metisControlAddRoute_Create, __func__);
}

LONGBOW_TEST_CASE(Global, metisControlAddRoute_HelpCreate)
{
    testCommandCreate(testCase, &metisControlAddRoute_HelpCreate, __func__);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisControl_AddRoute_Execute_WrongArgCount);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_AddRoute_Execute_ZeroCost);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_AddRoute_Execute_BadPrefix);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_AddRoute_Execute_Good);

    LONGBOW_RUN_TEST_CASE(Local, metisControl_Help_AddRoute_Execute);
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

static MetisCommandReturn
testAddRoute(const LongBowTestCase *testCase, int argc, const char *prefix, const char *nexthop, const char *cost)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    metisControlState_SetDebug(data->state, true);

    const char *argv[] = { "add", "route", nexthop, prefix, cost };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, argc, (void **) &argv[0]);

    MetisCommandOps *ops = metisControlAddRoute_Create(data->state);

    MetisCommandReturn result = ops->execute(data->state->parser, ops, args);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);
    return result;
}

LONGBOW_TEST_CASE(Local, metisControl_AddRoute_Execute_WrongArgCount)
{
    // argc is wrong, needs to be 5.
    MetisCommandReturn result = testAddRoute(testCase, 2, "lci:/foo", "703", "1");

    assertTrue(result == MetisCommandReturn_Failure,
               "metisControl_AddRoute with wrong argc should return %d, got %d", MetisCommandReturn_Failure, result);
}

LONGBOW_TEST_CASE(Local, metisControl_AddRoute_Execute_ZeroCost)
{
    MetisCommandReturn result = testAddRoute(testCase, 5, "lci:/foo", "702", "0");

    assertTrue(result == MetisCommandReturn_Failure,
               "metisControl_AddRoute with zero cost should return %d, got %d", MetisCommandReturn_Failure, result);
}

LONGBOW_TEST_CASE(Local, metisControl_AddRoute_Execute_BadPrefix)
{
    MetisCommandReturn result = testAddRoute(testCase, 5, "blah", "701", "1");

    assertTrue(result == MetisCommandReturn_Failure,
               "metisControl_AddRoute with zero cost should return %d, got %d", MetisCommandReturn_Failure, result);
}

LONGBOW_TEST_CASE(Local, metisControl_AddRoute_Execute_Good)
{
    MetisCommandReturn result = testAddRoute(testCase, 5, "lci:/foo", "700", "1");

    assertTrue(result == MetisCommandReturn_Success,
               "metisControl_AddRoute should return %d, got %d", MetisCommandReturn_Success, result);
}

LONGBOW_TEST_CASE(Local, metisControl_Help_AddRoute_Execute)
{
    testHelpExecute(testCase, metisControlAddRoute_HelpCreate, __func__, MetisCommandReturn_Success);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metisControl_AddRoute);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
