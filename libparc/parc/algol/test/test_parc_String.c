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
 */
#include "../parc_String.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_String)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_String)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_String)
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
    PARCString *instance = parcString_Create("Hello World");
    assertNotNull(instance, "Expected non-null result from parcString_Create();");
    parcObjectTesting_AssertAcquireReleaseImpl(instance);

    parcString_Release(&instance);
    assertNull(instance, "Expected null result from parcString_Release();");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcString_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcString_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcString_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcString_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcString_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcString_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, parcString_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, parcString_ToString);
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

LONGBOW_TEST_CASE(Global, parcString_Compare)
{
    PARCString *string = parcString_Create("Hello1");
    PARCString *equivalent[2] = {
        parcString_Create("Hello1"),
        NULL
    };
    PARCString *greater[2] = {
        parcString_Create("Hello2"),
        NULL
    };
    PARCString *lesser[2] = {
        parcString_Create("Hello0"),
        NULL
    };

    parcObjectTesting_AssertCompareTo(parcString_Compare, string, equivalent, lesser, greater);
    parcString_Release(&string);
    parcString_Release(&equivalent[0]);
    parcString_Release(&greater[0]);
    parcString_Release(&lesser[0]);
}

LONGBOW_TEST_CASE(Global, parcString_Copy)
{
    PARCString *instance = parcString_Create("Hello World");
    PARCString *copy = parcString_Copy(instance);
    assertTrue(parcString_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcString_Release(&instance);
    parcString_Release(&copy);
}

LONGBOW_TEST_CASE(Global, parcString_Display)
{
    PARCString *instance = parcString_Create("Hello World");
    parcString_Display(instance, 0);
    parcString_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcString_Equals)
{
    PARCString *x = parcString_Create("Hello World");
    PARCString *y = parcString_Create("Hello World");
    PARCString *z = parcString_Create("Hello World");

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcString_Release(&x);
    parcString_Release(&y);
    parcString_Release(&z);
}

LONGBOW_TEST_CASE(Global, parcString_HashCode)
{
    PARCString *x = parcString_Create("Hello World");
    PARCString *y = parcString_Create("Hello World");

    parcObjectTesting_AssertHashCode(x, y);

    parcString_Release(&x);
    parcString_Release(&y);
}

LONGBOW_TEST_CASE(Global, parcString_IsValid)
{
    PARCString *instance = parcString_Create("Hello World");
    assertTrue(parcString_IsValid(instance), "Expected parcString_Create to result in a valid instance.");

    parcString_Release(&instance);
    assertFalse(parcString_IsValid(instance), "Expected parcString_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcString_ToJSON)
{
    PARCString *instance = parcString_Create("Hello World");

    PARCJSON *json = parcString_ToJSON(instance);

    parcJSON_Release(&json);

    parcString_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcString_ToString)
{
    PARCString *instance = parcString_Create("Hello World");

    char *string = parcString_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcString_ToString");

    parcMemory_Deallocate((void **) &string);
    parcString_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_String);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


