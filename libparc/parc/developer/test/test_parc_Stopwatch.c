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
#include "../parc_Stopwatch.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <stdio.h>

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
    PARCStopwatch *instance = parcStopwatch_Create();
    assertNotNull(instance, "Expected non-null result from parcStopwatch_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcStopwatch_Acquire, instance);

    parcStopwatch_Release(&instance);
    assertNull(instance, "Expected null result from parcStopwatch_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcStopwatch_ToString);
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

LONGBOW_TEST_CASE(Object, parcStopwatch_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Object, parcStopwatch_Copy)
{
    PARCStopwatch *instance = parcStopwatch_Create();
    PARCStopwatch *copy = parcStopwatch_Copy(instance);
    assertTrue(parcStopwatch_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcStopwatch_Release(&instance);
    parcStopwatch_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcStopwatch_Display)
{
    PARCStopwatch *instance = parcStopwatch_Create();
    parcStopwatch_Display(instance, 0);
    parcStopwatch_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcStopwatch_Equals)
{
    PARCStopwatch *x = parcStopwatch_Create();
    PARCStopwatch *y = parcStopwatch_Create();
    PARCStopwatch *z = parcStopwatch_Create();

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcStopwatch_Release(&x);
    parcStopwatch_Release(&y);
    parcStopwatch_Release(&z);
}

LONGBOW_TEST_CASE(Object, parcStopwatch_HashCode)
{
    PARCStopwatch *x = parcStopwatch_Create();
    PARCStopwatch *y = parcStopwatch_Create();

    parcObjectTesting_AssertHashCode(x, y);

    parcStopwatch_Release(&x);
    parcStopwatch_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcStopwatch_IsValid)
{
    PARCStopwatch *instance = parcStopwatch_Create();
    assertTrue(parcStopwatch_IsValid(instance), "Expected parcStopwatch_Create to result in a valid instance.");

    parcStopwatch_Release(&instance);
    assertFalse(parcStopwatch_IsValid(instance), "Expected parcStopwatch_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcStopwatch_ToJSON)
{
    PARCStopwatch *instance = parcStopwatch_Create();

    PARCJSON *json = parcStopwatch_ToJSON(instance);

    parcJSON_Release(&json);

    parcStopwatch_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcStopwatch_ToString)
{
    PARCStopwatch *instance = parcStopwatch_Create();

    char *string = parcStopwatch_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcStopwatch_ToString");

    parcMemory_Deallocate((void **) &string);
    parcStopwatch_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, parcStopwatch_Multi);
    LONGBOW_RUN_TEST_CASE(Specialization, parcStopwatch_ElapsedTimeNanos);
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

LONGBOW_TEST_CASE(Specialization, parcStopwatch_Multi)
{
    PARCStopwatch *a = parcStopwatch_Create();
    PARCStopwatch *b = parcStopwatch_Create();
    PARCStopwatch *c = parcStopwatch_Create();

    parcStopwatch_Start(a, b, c);
    sleep(2);
    uint64_t nanos = parcStopwatch_ElapsedTimeNanos(a);
    printf("%lu %lu\n", nanos, nanos / 1000000000);
    if (nanos > (3000000000)) {
        parcStopwatch_Display(a, 0);
    }

    parcStopwatch_Release(&a);
    parcStopwatch_Release(&b);
    parcStopwatch_Release(&c);
}

LONGBOW_TEST_CASE(Specialization, parcStopwatch_ElapsedTimeNanos)
{
    PARCStopwatch *instance = parcStopwatch_Create();

    parcStopwatch_StartImpl(instance, NULL);
    sleep(2);
    uint64_t nanos = parcStopwatch_ElapsedTimeNanos(instance);
    printf("%lu %lu\n", nanos, nanos / 1000000000);
    if (nanos > (3000000000)) {
        parcStopwatch_Display(instance, 0);
    }

    parcStopwatch_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Timer);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


