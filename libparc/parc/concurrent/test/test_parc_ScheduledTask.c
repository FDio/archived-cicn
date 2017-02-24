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
#include "../parc_ScheduledTask.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_ScheduledTask)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_ScheduledTask)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_ScheduledTask)
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

void *
_function(PARCFutureTask *task, void *parameter)
{
    return parameter;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *instance = parcScheduledTask_Create(task, 0);
    assertNotNull(instance, "Expected non-null result from parcScheduledTask_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcScheduledTask_Acquire, instance);

    parcScheduledTask_Release(&instance);
    assertNull(instance, "Expected null result from parcScheduledTask_Release();");
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledTask_ToString);
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

LONGBOW_TEST_CASE(Object, parcScheduledTask_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Object, parcScheduledTask_Copy)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *instance = parcScheduledTask_Create(task, 0);
    PARCScheduledTask *copy = parcScheduledTask_Copy(instance);
    assertTrue(parcScheduledTask_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcScheduledTask_Release(&instance);
    parcScheduledTask_Release(&copy);
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Object, parcScheduledTask_Display)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *instance = parcScheduledTask_Create(task, 0);
    parcScheduledTask_Display(instance, 0);
    parcScheduledTask_Release(&instance);
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Object, parcScheduledTask_Equals)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *x = parcScheduledTask_Create(task, 0);
    PARCScheduledTask *y = parcScheduledTask_Create(task, 0);
    PARCScheduledTask *z = parcScheduledTask_Create(task, 0);

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcScheduledTask_Release(&x);
    parcScheduledTask_Release(&y);
    parcScheduledTask_Release(&z);
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Object, parcScheduledTask_HashCode)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *x = parcScheduledTask_Create(task, 0);
    PARCScheduledTask *y = parcScheduledTask_Create(task, 0);

    parcObjectTesting_AssertHashCode(x, y);

    parcScheduledTask_Release(&x);
    parcScheduledTask_Release(&y);
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Object, parcScheduledTask_IsValid)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *instance = parcScheduledTask_Create(task, 0);
    assertTrue(parcScheduledTask_IsValid(instance), "Expected parcScheduledTask_Create to result in a valid instance.");

    parcScheduledTask_Release(&instance);
    assertFalse(parcScheduledTask_IsValid(instance), "Expected parcScheduledTask_Release to result in an invalid instance.");

    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Object, parcScheduledTask_ToJSON)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *instance = parcScheduledTask_Create(task, 0);

    PARCJSON *json = parcScheduledTask_ToJSON(instance);

    parcJSON_Release(&json);

    parcScheduledTask_Release(&instance);
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Object, parcScheduledTask_ToString)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCScheduledTask *instance = parcScheduledTask_Create(task, 0);

    char *string = parcScheduledTask_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcScheduledTask_ToString");

    parcMemory_Deallocate((void **) &string);
    parcScheduledTask_Release(&instance);
    parcFutureTask_Release(&task);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_ScheduledTask);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


