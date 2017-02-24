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

#include "../transport_Stack.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(transport_Stack)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(transport_Stack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(transport_Stack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    TransportStack *instance = transportStack_Create();
    assertNotNull(instance, "Expected non-null result from transportStack_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(instance);

    transportStack_Release(&instance);
    assertNull(instance, "Expected null result from transportStack_Release();");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, transportStack_Compare);
    LONGBOW_RUN_TEST_CASE(Global, transportStack_Copy);
    LONGBOW_RUN_TEST_CASE(Global, transportStack_Display);
    LONGBOW_RUN_TEST_CASE(Global, transportStack_Equals);
    LONGBOW_RUN_TEST_CASE(Global, transportStack_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, transportStack_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, transportStack_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, transportStack_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, transportStack_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, transportStack_Copy)
{
    TransportStack *instance = transportStack_Create();
    TransportStack *copy = transportStack_Copy(instance);
    assertTrue(transportStack_Equals(instance, copy), "Expected the copy to be equal to the original");

    transportStack_Release(&instance);
    transportStack_Release(&copy);
}

LONGBOW_TEST_CASE(Global, transportStack_Display)
{
    TransportStack *instance = transportStack_Create();
    transportStack_Display(instance, 0);
    transportStack_Release(&instance);
}

LONGBOW_TEST_CASE(Global, transportStack_Equals)
{
    TransportStack *x = transportStack_Create();
    TransportStack *y = transportStack_Create();
    TransportStack *z = transportStack_Create();

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    transportStack_Release(&x);
    transportStack_Release(&y);
    transportStack_Release(&z);
}

LONGBOW_TEST_CASE(Global, transportStack_HashCode)
{
    TransportStack *x = transportStack_Create();
    TransportStack *y = transportStack_Create();

    parcObjectTesting_AssertHashCode(x, y);

    transportStack_Release(&x);
    transportStack_Release(&y);
}

LONGBOW_TEST_CASE(Global, transportStack_IsValid)
{
    TransportStack *instance = transportStack_Create();
    assertTrue(transportStack_IsValid(instance), "Expected transportStack_Create to result in a valid instance.");

    transportStack_Release(&instance);
    assertFalse(transportStack_IsValid(instance), "Expected transportStack_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, transportStack_ToJSON)
{
    TransportStack *instance = transportStack_Create();

    PARCJSON *json = transportStack_ToJSON(instance);

    parcJSON_Release(&json);

    transportStack_Release(&instance);
}

LONGBOW_TEST_CASE(Global, transportStack_ToString)
{
    TransportStack *instance = transportStack_Create();

    char *string = transportStack_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from transportStack_ToString");

    parcMemory_Deallocate((void **) &string);
    transportStack_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(transport_Stack);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


