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
#include "../parc_FutureTask.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_FutureTask)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_FutureTask)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_FutureTask)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease_PARCObject);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    int initialAllocations = longBowTestCase_GetInt(testCase, "initialAllocations");

    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

static void *
_function(PARCFutureTask *task, void *parameter)
{
    return parameter;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCFutureTask *instance = parcFutureTask_Create(_function, _function);
    assertNotNull(instance, "Expected non-null result from parcFutureTask_Create(_function, _function);");

    parcObjectTesting_AssertAcquireReleaseContract(parcFutureTask_Acquire, instance);

    parcFutureTask_Release(&instance);
    assertNull(instance, "Expected null result from parcFutureTask_Release();");
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease_PARCObject)
{
    PARCBuffer *object = parcBuffer_Allocate(10);

    PARCFutureTask *instance = parcFutureTask_Create(_function, object);
    parcBuffer_Release(&object);

    assertNotNull(instance, "Expected non-null result from parcFutureTask_Create(_function, object);");

    parcObjectTesting_AssertAcquireReleaseContract(parcFutureTask_Acquire, instance);

    parcFutureTask_Release(&instance);
    assertNull(instance, "Expected null result from parcFutureTask_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcFutureTask_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Object)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Object)
{
    int initialAllocations = longBowTestCase_GetInt(testCase, "initialAllocations");

    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Object, parcFutureTask_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Object, parcFutureTask_Copy)
{
    PARCFutureTask *instance = parcFutureTask_Create(_function, _function);
    PARCFutureTask *copy = parcFutureTask_Copy(instance);
    assertTrue(parcFutureTask_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcFutureTask_Release(&instance);
    parcFutureTask_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcFutureTask_Display)
{
    PARCFutureTask *instance = parcFutureTask_Create(_function, _function);
    parcFutureTask_Display(instance, 0);
    parcFutureTask_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcFutureTask_Equals)
{
    PARCFutureTask *x = parcFutureTask_Create(_function, _function);
    PARCFutureTask *y = parcFutureTask_Create(_function, _function);
    PARCFutureTask *z = parcFutureTask_Create(_function, _function);
    PARCFutureTask *u1 = parcFutureTask_Create(_function, NULL);

    parcObjectTesting_AssertEquals(x, y, z, u1, NULL);

    parcFutureTask_Release(&x);
    parcFutureTask_Release(&y);
    parcFutureTask_Release(&z);
    parcFutureTask_Release(&u1);
}

LONGBOW_TEST_CASE(Object, parcFutureTask_HashCode)
{
    PARCFutureTask *x = parcFutureTask_Create(_function, _function);
    PARCFutureTask *y = parcFutureTask_Create(_function, _function);

    parcObjectTesting_AssertHashCode(x, y);

    parcFutureTask_Release(&x);
    parcFutureTask_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcFutureTask_IsValid)
{
    PARCFutureTask *instance = parcFutureTask_Create(_function, _function);
    assertTrue(parcFutureTask_IsValid(instance), "Expected parcFutureTask_Create to result in a valid instance.");

    parcFutureTask_Release(&instance);
    assertFalse(parcFutureTask_IsValid(instance), "Expected parcFutureTask_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcFutureTask_ToJSON)
{
    PARCFutureTask *instance = parcFutureTask_Create(_function, _function);

    PARCJSON *json = parcFutureTask_ToJSON(instance);

    parcJSON_Release(&json);

    parcFutureTask_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcFutureTask_ToString)
{
    PARCFutureTask *instance = parcFutureTask_Create(_function, _function);

    char *string = parcFutureTask_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcFutureTask_ToString");

    parcMemory_Deallocate((void **) &string);
    parcFutureTask_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, parcFutureTask_Cancel);
    LONGBOW_RUN_TEST_CASE(Specialization, parcFutureTask_Get);
    LONGBOW_RUN_TEST_CASE(Specialization, parcFutureTask_IsCancelled);
    LONGBOW_RUN_TEST_CASE(Specialization, parcFutureTask_IsDone);
    LONGBOW_RUN_TEST_CASE(Specialization, parcFutureTask_Run);
    LONGBOW_RUN_TEST_CASE(Specialization, parcFutureTask_RunAndReset);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    int initialAllocations = longBowTestCase_GetInt(testCase, "initialAllocations");

    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Specialization, parcFutureTask_Cancel)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    bool parcFutureTask_Cancel(PARCFutureTask *task, bool mayInterruptIfRunning);

    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Specialization, parcFutureTask_Get)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    PARCFutureTaskResult result = parcFutureTask_Get(task, PARCTimeout_Immediate);

    assertTrue(parcExecution_Is(result.execution, PARCExecution_Timeout), "Expected Timeout, actual %s",
               parcExecution_GetMessage(result.execution));
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Specialization, parcFutureTask_IsCancelled)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    bool actual = parcFutureTask_IsCancelled(task);
    assertFalse(actual, "Expected true.");

    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Specialization, parcFutureTask_IsDone)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    bool actual = parcFutureTask_IsDone(task);

    assertFalse(actual, "Expected false.");

    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Specialization, parcFutureTask_Run)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    parcFutureTask_Run(task);

    PARCFutureTaskResult actual = parcFutureTask_Get(task, PARCTimeout_Immediate);

    assertTrue(parcFutureTask_IsDone(task), "Expected parcFutureTask_IsDone to be true.");
    assertTrue(parcExecution_Is(actual.execution, PARCExecution_OK),
               "Expected OK, actual %s", parcExecution_GetMessage(actual.execution));
    assertTrue(actual.value == _function, "Expected actual to point to _function");
    parcFutureTask_Release(&task);
}

LONGBOW_TEST_CASE(Specialization, parcFutureTask_RunAndReset)
{
    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    bool actual = parcFutureTask_RunAndReset(task);

    assertTrue(actual, "Expectd parcFutureTask_RunAndReset to return true.");
    assertFalse(parcFutureTask_IsDone(task), "Expected parcFutureTask_IsDone to be false");
    parcFutureTask_Release(&task);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_FutureTask);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
