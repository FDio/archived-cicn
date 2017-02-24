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
#include "../ccnx_TransportConfig.c"

#include <LongBow/unit-test.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>
#include <parc/algol/parc_SafeMemory.h>


LONGBOW_TEST_RUNNER(ccnx_TransportConfig)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_TransportConfig)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_TransportConfig)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_Copy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_CreateDestroy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_GetConnectionConfig);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_GetStackConfig);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_IsValid_True);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTransportConfig_IsValid_False);
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

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_AssertValid)
{
    CCNxStackConfig *stack = ccnxStackConfig_Create();
    CCNxConnectionConfig *connection = ccnxConnectionConfig_Create();

    CCNxTransportConfig *x = ccnxTransportConfig_Create(stack, connection);

    ccnxTransportConfig_AssertValid(x);

    ccnxStackConfig_Release(&stack);

    ccnxTransportConfig_Destroy(&x);
}

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_Copy)
{
    CCNxStackConfig *stack = ccnxStackConfig_Create();
    CCNxConnectionConfig *connection = ccnxConnectionConfig_Create();

    CCNxTransportConfig *x = ccnxTransportConfig_Create(stack, connection);
    assertNotNull(x, "Expected non-null result from ccnxTransportConfig_Create");
    ccnxStackConfig_Release(&stack);

    CCNxTransportConfig *y = ccnxTransportConfig_Copy(x);

    assertTrue(ccnxTransportConfig_Equals(x, y), "Expected ccnxTransportConfig_Copy result to be equal to the original");

    ccnxTransportConfig_Destroy(&x);
    ccnxTransportConfig_Destroy(&y);
}

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_Equals)
{
    CCNxStackConfig *stack = ccnxStackConfig_Create();
    CCNxConnectionConfig *connection = ccnxConnectionConfig_Create();

    CCNxTransportConfig *x = ccnxTransportConfig_Create(stack, ccnxConnectionConfig_Copy(connection));
    CCNxTransportConfig *y = ccnxTransportConfig_Create(stack, ccnxConnectionConfig_Copy(connection));
    CCNxTransportConfig *z = ccnxTransportConfig_Create(stack, ccnxConnectionConfig_Copy(connection));

    CCNxStackConfig *otherStack = ccnxStackConfig_Create();
    PARCJSONValue *val = parcJSONValue_CreateFromNULL();
    ccnxStackConfig_Add(otherStack, "key", val);
    CCNxTransportConfig *u1 = ccnxTransportConfig_Create(otherStack, ccnxConnectionConfig_Copy(connection));

    CCNxConnectionConfig *otherConnection = ccnxConnectionConfig_Create();
    ccnxConnectionConfig_Add(otherConnection, "key", val);

    CCNxTransportConfig *u2 = ccnxTransportConfig_Create(stack, otherConnection);

    parcObjectTesting_AssertEqualsFunction(ccnxTransportConfig_Equals, x, y, z, u1, u2);

    assertTrue(ccnxTransportConfig_Equals(x, y), "Expected ccnxTransportConfig_Copy result to be equal to the original");

    parcJSONValue_Release(&val);
    ccnxStackConfig_Release(&stack);
    ccnxStackConfig_Release(&otherStack);
    ccnxConnectionConfig_Destroy(&connection);

    ccnxTransportConfig_Destroy(&x);
    ccnxTransportConfig_Destroy(&y);
    ccnxTransportConfig_Destroy(&z);
    ccnxTransportConfig_Destroy(&u1);
    ccnxTransportConfig_Destroy(&u2);
}

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_CreateDestroy)
{
    CCNxStackConfig *stack = ccnxStackConfig_Create();
    CCNxConnectionConfig *connection = ccnxConnectionConfig_Create();

    CCNxTransportConfig *x = ccnxTransportConfig_Create(stack, connection);
    assertNotNull(x, "Expected non-null result from ccnxTransportConfig_Create");
    ccnxStackConfig_Release(&stack);

    ccnxTransportConfig_Destroy(&x);
    assertNull(x, "Expected null result from ccnxStackConfig_Release");
}

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_GetConnectionConfig)
{
    CCNxStackConfig *stack = ccnxStackConfig_Create();
    CCNxConnectionConfig *connection = ccnxConnectionConfig_Create();

    CCNxTransportConfig *config = ccnxTransportConfig_Create(stack, connection);
    ccnxStackConfig_Release(&stack);

    CCNxConnectionConfig *actual = ccnxTransportConfig_GetConnectionConfig(config);

    assertTrue(connection == actual, "Expected ccnxTransportConfig_GetConnectionConfig to return the original.");

    ccnxTransportConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_GetStackConfig)
{
    CCNxStackConfig *stack = ccnxStackConfig_Create();
    CCNxConnectionConfig *connection = ccnxConnectionConfig_Create();

    CCNxTransportConfig *config = ccnxTransportConfig_Create(stack, connection);

    CCNxStackConfig *actual = ccnxTransportConfig_GetStackConfig(config);

    assertTrue(stack == actual, "Expected ccnxTransportConfig_GetStackConfig to return the original.");

    ccnxStackConfig_Release(&stack);
    ccnxTransportConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_IsValid_True)
{
    CCNxStackConfig *stack = ccnxStackConfig_Create();
    CCNxConnectionConfig *connection = ccnxConnectionConfig_Create();

    CCNxTransportConfig *x = ccnxTransportConfig_Create(stack, connection);
    assertTrue(ccnxTransportConfig_IsValid(x), "Expected ccnxTransportConfig_Create to return a valid instance.");
    ccnxStackConfig_Release(&stack);

    ccnxTransportConfig_Destroy(&x);
}

LONGBOW_TEST_CASE(Global, ccnxTransportConfig_IsValid_False)
{
    assertFalse(ccnxTransportConfig_IsValid(NULL), "Expected NULL to be an invalid instance.");
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_TransportConfig);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
