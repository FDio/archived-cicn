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
#include "../ccnx_StackConfig.c"
#include <LongBow/unit-test.h>

#include <inttypes.h>
#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(ccnx_StackConfig)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_StackConfig)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_StackConfig)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_AddGet);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_Copy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_CreateAcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_Display);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_GetJson);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, ccnxStackConfig_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_AddGet)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();

    PARCJSONValue *expected = parcJSONValue_CreateFromNULL();
    ccnxStackConfig_Add(instance, "key", expected);

    PARCJSONValue *actual = ccnxStackConfig_Get(instance, "key");

    assertTrue(parcJSONValue_Equals(expected, actual), "ccnxStackConfig_Get did not return what was 'added'");

    parcJSONValue_Release(&expected);

    ccnxStackConfig_Release(&instance);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_AssertValid)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();
    ccnxStackConfig_AssertValid(instance);

    ccnxStackConfig_Release(&instance);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_Copy)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();
    CCNxStackConfig *copy = ccnxStackConfig_Copy(instance);
    assertTrue(ccnxStackConfig_Equals(instance, copy), "Expected the copy to be equal to the original");

    ccnxStackConfig_Release(&instance);
    ccnxStackConfig_Release(&copy);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_CreateAcquireRelease)
{
    CCNxStackConfig *config = ccnxStackConfig_Create();
    assertNotNull(config, "Expected non-NULL result from ccnxConnectionConfig_Create.");

    CCNxStackConfig *reference = ccnxStackConfig_Acquire(config);

    ccnxStackConfig_Release(&config);
    assertNull(config, "Expected NULL result from ccnxConnectionConfig_Destroy");
    ccnxStackConfig_Release(&reference);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_Display)
{
    CCNxStackConfig *config = ccnxStackConfig_Create();
    ccnxStackConfig_Display(config, 1);

    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_Equals)
{
    CCNxStackConfig *x = ccnxStackConfig_Create();
    CCNxStackConfig *y = ccnxStackConfig_Create();
    CCNxStackConfig *z = ccnxStackConfig_Create();

    CCNxStackConfig *u1 = ccnxStackConfig_Create();
    PARCJSONValue *val = parcJSONValue_CreateFromNULL();
    ccnxStackConfig_Add(u1, "key", val);
    parcJSONValue_Release(&val);

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    ccnxStackConfig_Release(&x);
    ccnxStackConfig_Release(&y);
    ccnxStackConfig_Release(&z);
    ccnxStackConfig_Release(&u1);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_HashCode)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();
    uint64_t hashCode = ccnxStackConfig_HashCode(instance);
    printf("%" PRIu64 "\n", hashCode);
    ccnxStackConfig_Release(&instance);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_GetJson)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();
    PARCJSON *json = ccnxStackConfig_GetJson(instance);

    assertNotNull(json, "Expected non-null JSON");
    ccnxStackConfig_Release(&instance);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_IsValid)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();
    assertTrue(ccnxStackConfig_IsValid(instance), "Expected ccnxStackConfig_Create to result in a valid instance.");

    ccnxStackConfig_Release(&instance);
    assertFalse(ccnxStackConfig_IsValid(instance), "Expected ccnxStackConfig_Create to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_ToJSON)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();
    PARCJSON *json = ccnxStackConfig_ToJSON(instance);
    assertNotNull(json, "Expected non-null JSON");

    ccnxStackConfig_Release(&instance);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_ToString)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();
    char *string = ccnxStackConfig_ToString(instance);
    assertNotNull(string, "Expected non-null ccnxStackConfig_ToString");

    parcMemory_Deallocate((void **) &string);
    ccnxStackConfig_Release(&instance);
}

LONGBOW_TEST_CASE(Global, ccnxStackConfig_Get)
{
    CCNxStackConfig *instance = ccnxStackConfig_Create();

    ccnxStackConfig_Release(&instance);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_StackConfig);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
