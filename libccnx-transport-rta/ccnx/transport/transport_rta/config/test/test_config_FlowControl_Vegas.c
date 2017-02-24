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
 * Rta component configuration class unit test
 *
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../config_FlowControl_Vegas.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include "testrig_RtaConfigCommon.c"

LONGBOW_TEST_RUNNER(config_FlowControl_Vegas)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(config_FlowControl_Vegas)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(config_FlowControl_Vegas)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, FlowControl_Vegas_ConnectionConfig_JsonKey);
    LONGBOW_RUN_TEST_CASE(Global, FlowControl_Vegas_ConnectionConfig_ReturnValue);
    LONGBOW_RUN_TEST_CASE(Global, FlowControl_Vegas_GetName);
    LONGBOW_RUN_TEST_CASE(Global, FlowControl_Vegas_ProtocolStackConfig_JsonKey);
    LONGBOW_RUN_TEST_CASE(Global, FlowControl_Vegas_ProtocolStackConfig_ReturnValue);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, testRtaConfiguration_CommonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    testRtaConfiguration_CommonTeardown(longBowTestCase_GetClipBoardData(testCase));
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, FlowControl_Vegas_ConnectionConfig_ReturnValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxConnectionConfig *test = vegasFlowController_ConnectionConfig(data->connConfig);

    assertTrue(test == data->connConfig,
               "Did not return pointer to argument for chaining, got %p expected %p",
               (void *) test, (void *) data->connConfig);
}

LONGBOW_TEST_CASE(Global, FlowControl_Vegas_ConnectionConfig_JsonKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    testRtaConfiguration_ConnectionJsonKey(vegasFlowController_ConnectionConfig(data->connConfig),
                                           vegasFlowController_GetName());
}

LONGBOW_TEST_CASE(Global, FlowControl_Vegas_GetName)
{
    testRtaConfiguration_ComponentName(vegasFlowController_GetName, RtaComponentNames[FC_VEGAS]);
}

LONGBOW_TEST_CASE(Global, FlowControl_Vegas_ProtocolStackConfig_JsonKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    testRtaConfiguration_ProtocolStackJsonKey(vegasFlowController_ProtocolStackConfig(data->stackConfig),
                                              vegasFlowController_GetName());
}

LONGBOW_TEST_CASE(Global, FlowControl_Vegas_ProtocolStackConfig_ReturnValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxStackConfig *test = vegasFlowController_ProtocolStackConfig(data->stackConfig);

    assertTrue(test == data->stackConfig,
               "Did not return pointer to argument for chaining, got %p expected %p",
               (void *) test, (void *) data->stackConfig);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(config_FlowControl_Vegas);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
