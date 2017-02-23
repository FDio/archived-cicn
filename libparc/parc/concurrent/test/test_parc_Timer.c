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
#include "../parc_Timer.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_Timer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Timer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Timer)
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
    PARCTimer *instance = parcTimer_Create();
    assertNotNull(instance, "Expected non-null result from parcTimer_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcTimer_Acquire, instance);

    parcTimer_Release(&instance);
    assertNull(instance, "Expected null result from parcTimer_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcTimer_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Object)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Object)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Object, parcTimer_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Object, parcTimer_Copy)
{
    PARCTimer *instance = parcTimer_Create();
    PARCTimer *copy = parcTimer_Copy(instance);
    assertTrue(parcTimer_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcTimer_Release(&instance);
    parcTimer_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcTimer_Display)
{
    PARCTimer *instance = parcTimer_Create();
    parcTimer_Display(instance, 0);
    parcTimer_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcTimer_Equals)
{
    PARCTimer *x = parcTimer_Create();
    PARCTimer *y = parcTimer_Create();
    PARCTimer *z = parcTimer_Create();

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcTimer_Release(&x);
    parcTimer_Release(&y);
    parcTimer_Release(&z);
}

LONGBOW_TEST_CASE(Object, parcTimer_HashCode)
{
    PARCTimer *x = parcTimer_Create();
    PARCTimer *y = parcTimer_Create();

    parcObjectTesting_AssertHashCode(x, y);

    parcTimer_Release(&x);
    parcTimer_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcTimer_IsValid)
{
    PARCTimer *instance = parcTimer_Create();
    assertTrue(parcTimer_IsValid(instance), "Expected parcTimer_Create to result in a valid instance.");

    parcTimer_Release(&instance);
    assertFalse(parcTimer_IsValid(instance), "Expected parcTimer_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcTimer_ToJSON)
{
    PARCTimer *instance = parcTimer_Create();

    PARCJSON *json = parcTimer_ToJSON(instance);

    parcJSON_Release(&json);

    parcTimer_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcTimer_ToString)
{
    PARCTimer *instance = parcTimer_Create();

    char *string = parcTimer_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcTimer_ToString");

    parcMemory_Deallocate((void **) &string);
    parcTimer_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Timer);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


