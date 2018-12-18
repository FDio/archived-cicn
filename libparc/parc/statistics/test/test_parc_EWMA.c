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
#include "../parc_EWMA.c"
#include <stdio.h>
#include <inttypes.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_EWMA)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_EWMA)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_EWMA)
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
    PARCEWMA *instance = parcEWMA_Create(0.75);
    assertNotNull(instance, "Expected non-null result from parcEWMA_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcEWMA_Acquire, instance);

    parcEWMA_Release(&instance);
    assertNull(instance, "Expected null result from parcEWMA_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcEWMA_ToString);
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

LONGBOW_TEST_CASE(Object, parcEWMA_Compare)
{
    PARCEWMA *ewma = parcEWMA_Create(0.75);
    PARCEWMA *lesser = parcEWMA_Create(0.75);
    PARCEWMA *greater = parcEWMA_Create(0.75);

    parcEWMA_Update(ewma, 5);
    parcEWMA_Update(lesser, 1);
    parcEWMA_Update(greater, 10);

    parcObjectTesting_AssertCompareToImpl((PARCObjectCompare *) parcEWMA_Compare,
                                          ewma,
                                          (const void *[]) { ewma, NULL },
                                          (const void *[]) { lesser, NULL },
                                          (const void *[]) { greater, NULL });

    parcEWMA_Release(&ewma);
    parcEWMA_Release(&lesser);
    parcEWMA_Release(&greater);
}

LONGBOW_TEST_CASE(Object, parcEWMA_Copy)
{
    PARCEWMA *instance = parcEWMA_Create(0.75);
    PARCEWMA *copy = parcEWMA_Copy(instance);
    assertTrue(parcEWMA_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcEWMA_Release(&instance);
    parcEWMA_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcEWMA_Display)
{
    PARCEWMA *instance = parcEWMA_Create(0.75);
    parcEWMA_Display(instance, 0);
    parcEWMA_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcEWMA_Equals)
{
    PARCEWMA *x = parcEWMA_Create(0.75);
    PARCEWMA *y = parcEWMA_Create(0.75);
    PARCEWMA *z = parcEWMA_Create(0.75);

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcEWMA_Release(&x);
    parcEWMA_Release(&y);
    parcEWMA_Release(&z);
}

LONGBOW_TEST_CASE(Object, parcEWMA_HashCode)
{
    PARCEWMA *x = parcEWMA_Create(0.75);
    PARCEWMA *y = parcEWMA_Create(0.75);

    parcObjectTesting_AssertHashCode(x, y);

    parcEWMA_Release(&x);
    parcEWMA_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcEWMA_IsValid)
{
    PARCEWMA *instance = parcEWMA_Create(0.75);
    assertTrue(parcEWMA_IsValid(instance), "Expected parcEWMA_Create to result in a valid instance.");

    parcEWMA_Release(&instance);
    assertFalse(parcEWMA_IsValid(instance), "Expected parcEWMA_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcEWMA_ToJSON)
{
    PARCEWMA *instance = parcEWMA_Create(0.75);

    PARCJSON *json = parcEWMA_ToJSON(instance);

    parcJSON_Release(&json);

    parcEWMA_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcEWMA_ToString)
{
    PARCEWMA *instance = parcEWMA_Create(0.75);

    char *string = parcEWMA_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcEWMA_ToString");

    parcMemory_Deallocate((void **) &string);
    parcEWMA_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, parcEWMA_Update);
    LONGBOW_RUN_TEST_CASE(Specialization, parcEWMA_GetValue);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Specialization, parcEWMA_Update)
{
    PARCEWMA *instance = parcEWMA_Create(0.25);

    for (int64_t i = 0; i < 100; i++) {
        int64_t value = parcEWMA_Update(instance, i);
        printf("%" PRId64 " %" PRId64 " %" PRId64 "\n", value, i, value - i);
    }

    parcEWMA_Release(&instance);
}

LONGBOW_TEST_CASE(Specialization, parcEWMA_GetValue)
{
    PARCEWMA *instance = parcEWMA_Create(0.75);

    parcEWMA_Update(instance, 0);

    int64_t value = parcEWMA_GetValue(instance);

    assertTrue(value == 0, "Expected 0, actual %" PRId64 "", value);

    parcEWMA_Release(&instance);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_EWMA);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


