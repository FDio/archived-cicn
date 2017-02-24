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
#include "../parc_Properties.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_Properties)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Specialized);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Properties)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Properties)
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
    PARCProperties *instance = parcProperties_Create();
    assertNotNull(instance, "Expected non-null result from parcProperties_Create();");

    parcObjectTesting_AssertAcquire(instance);

    parcProperties_Release(&instance);
    assertNull(instance, "Expected null result from parcProperties_Release();");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, parcProperties_ToString);
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

LONGBOW_TEST_CASE(Global, parcProperties_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcProperties_Copy)
{
    PARCProperties *instance = parcProperties_Create();
    PARCProperties *copy = parcProperties_Copy(instance);
    assertTrue(parcProperties_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcProperties_Release(&instance);
    parcProperties_Release(&copy);
}

LONGBOW_TEST_CASE(Global, parcProperties_Display)
{
    PARCProperties *instance = parcProperties_Create();
    parcProperties_SetProperty(instance, "foo", "bar");
    parcProperties_SetProperty(instance, "xyzzy", "plugh");

    parcProperties_Display(instance, 0);
    parcProperties_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcProperties_Equals)
{
    PARCProperties *x = parcProperties_Create();
    PARCProperties *y = parcProperties_Create();
    PARCProperties *z = parcProperties_Create();

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcProperties_Release(&x);
    parcProperties_Release(&y);
    parcProperties_Release(&z);
}

LONGBOW_TEST_CASE(Global, parcProperties_HashCode)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcProperties_IsValid)
{
    PARCProperties *instance = parcProperties_Create();
    assertTrue(parcProperties_IsValid(instance), "Expected parcProperties_Create to result in a valid instance.");

    parcProperties_Release(&instance);
    assertFalse(parcProperties_IsValid(instance), "Expected parcProperties_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcProperties_ToJSON)
{
    PARCProperties *instance = parcProperties_Create();

    parcProperties_SetProperty(instance, "foo", "bar");
    PARCJSON *json = parcProperties_ToJSON(instance);

    parcJSON_Release(&json);

    parcProperties_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcProperties_ToString)
{
    PARCProperties *instance = parcProperties_Create();

    parcProperties_SetProperty(instance, "foo", "bar");
    parcProperties_SetProperty(instance, "bar", "baz");
    char *string = parcProperties_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcProperties_ToString");

    parcMemory_Deallocate((void **) &string);
    parcProperties_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialized)
{
    LONGBOW_RUN_TEST_CASE(Specialized, parcProperties_SetProperty);
    LONGBOW_RUN_TEST_CASE(Specialized, parcProperties_GetProperty);
    LONGBOW_RUN_TEST_CASE(Specialized, parcProperties_GetPropertyDefault);
    LONGBOW_RUN_TEST_CASE(Specialized, parcProperties_GetAsBoolean_true);
    LONGBOW_RUN_TEST_CASE(Specialized, parcProperties_GetAsBoolean_false);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialized)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialized)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Specialized, parcProperties_SetProperty)
{
    PARCProperties *instance = parcProperties_Create();
    char *expected = "bar";
    parcProperties_SetProperty(instance, "foo", expected);

    const char *actual = parcProperties_GetProperty(instance, "foo");
    assertTrue(strcmp("bar", actual) == 0, "Expected %s, actual %s", expected, actual);

    parcProperties_Release(&instance);
}

LONGBOW_TEST_CASE(Specialized, parcProperties_GetProperty)
{
    PARCProperties *instance = parcProperties_Create();
    char *expected = "bar";
    parcProperties_SetProperty(instance, "foo", expected);

    const char *actual = parcProperties_GetProperty(instance, "foo");
    assertTrue(strcmp("bar", actual) == 0, "Expected %s, actual %s", expected, actual);

    parcProperties_Release(&instance);
}

LONGBOW_TEST_CASE(Specialized, parcProperties_GetPropertyDefault)
{
    PARCProperties *instance = parcProperties_Create();
    char *expected = "bar";
    parcProperties_SetProperty(instance, "foo", expected);

    const char *actual = parcProperties_GetPropertyDefault(instance, "blurfl", "defaultValue");
    assertTrue(strcmp("defaultValue", actual) == 0, "Expected %s, actual %s", "defaultValue", actual);

    parcProperties_Release(&instance);
}

LONGBOW_TEST_CASE(Specialized, parcProperties_GetAsBoolean_true)
{
    PARCProperties *instance = parcProperties_Create();
    char *expected = "true";
    parcProperties_SetProperty(instance, "foo", expected);

    bool actual = parcProperties_GetAsBoolean(instance, "foo", false);
    assertTrue(actual, "Expected true");

    parcProperties_Release(&instance);
}

LONGBOW_TEST_CASE(Specialized, parcProperties_GetAsBoolean_false)
{
    PARCProperties *instance = parcProperties_Create();
    char *expected = "false";
    parcProperties_SetProperty(instance, "foo", expected);

    bool actual = parcProperties_GetAsBoolean(instance, "foo", true);
    assertFalse(actual, "Expected false");

    parcProperties_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Properties);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


