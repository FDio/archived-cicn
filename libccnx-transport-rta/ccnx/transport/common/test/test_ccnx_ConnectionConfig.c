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
// This permits internal static functions to be visible to this Test Runner.
#include "../ccnx_ConnectionConfig.c"

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(ccnx_ConnectionConfig)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_ConnectionConfig)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_ConnectionConfig)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_Add);
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_Copy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_CreateDestroy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_Display);
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_GetJson);
    LONGBOW_RUN_TEST_CASE(Global, ccnxConnectionConfig_IsValid);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    parcSafeMemory_ReportAllocation(STDOUT_FILENO);

    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_Add)
{
    CCNxConnectionConfig *config = ccnxConnectionConfig_Create();

    PARCJSONValue *val = parcJSONValue_CreateFromNULL();
    ccnxConnectionConfig_Add(config, "key", val);
    parcJSONValue_Release(&val);

    ccnxConnectionConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_AssertValid)
{
    CCNxConnectionConfig *config = ccnxConnectionConfig_Create();
    ccnxConnectionConfig_AssertValid(config);
    ccnxConnectionConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_Equals)
{
    CCNxConnectionConfig *x = ccnxConnectionConfig_Create();
    CCNxConnectionConfig *y = ccnxConnectionConfig_Create();
    CCNxConnectionConfig *z = ccnxConnectionConfig_Create();
    CCNxConnectionConfig *u1 = ccnxConnectionConfig_Create();
    PARCJSONValue *val = parcJSONValue_CreateFromNULL();
    ccnxConnectionConfig_Add(u1, "key", val);
    parcJSONValue_Release(&val);

    parcObjectTesting_AssertEqualsFunction(ccnxConnectionConfig_Equals, x, y, z, u1);

    ccnxConnectionConfig_Destroy(&x);
    ccnxConnectionConfig_Destroy(&y);
    ccnxConnectionConfig_Destroy(&z);
    ccnxConnectionConfig_Destroy(&u1);
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_Copy)
{
    CCNxConnectionConfig *x = ccnxConnectionConfig_Create();
    PARCJSONValue *val = parcJSONValue_CreateFromNULL();
    ccnxConnectionConfig_Add(x, "key", val);
    parcJSONValue_Release(&val);

    CCNxConnectionConfig *y = ccnxConnectionConfig_Copy(x);
    assertTrue(ccnxConnectionConfig_Equals(x, y), "Expected the copy to be equal to the original");
    ccnxConnectionConfig_Destroy(&x);
    ccnxConnectionConfig_Destroy(&y);
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_CreateDestroy)
{
    CCNxConnectionConfig *config = ccnxConnectionConfig_Create();
    assertNotNull(config, "Expected non-NULL result from ccnxConnectionConfig_Create.");
    ccnxConnectionConfig_Destroy(&config);
    assertNull(config, "Expected NULL result from ccnxConnectionConfig_Destroy");
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_Display)
{
    CCNxConnectionConfig *config = ccnxConnectionConfig_Create();
    ccnxConnectionConfig_Display(config, 0);

    ccnxConnectionConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_GetJson)
{
    CCNxConnectionConfig *config = ccnxConnectionConfig_Create();

    PARCJSON *json = ccnxConnectionConfig_GetJson(config);

    assertNotNull(json, "Expected ccnxConnectionConfig_GetJson result to be non-null.");
    ccnxConnectionConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Global, ccnxConnectionConfig_IsValid)
{
    CCNxConnectionConfig *config = ccnxConnectionConfig_Create();
    assertTrue(ccnxConnectionConfig_IsValid(config), "Expected ccnxConnectionConfig_Create result to be valid.");

    ccnxConnectionConfig_Destroy(&config);
    assertFalse(ccnxConnectionConfig_IsValid(config), "Expected ccnxConnectionConfig_Destroy result to be invalid.");
}

LONGBOW_TEST_FIXTURE(Static)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_ConnectionConfig);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
