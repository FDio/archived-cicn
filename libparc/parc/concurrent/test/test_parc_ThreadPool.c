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
#include "../parc_ThreadPool.c"

#include <stdio.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_ThreadPool)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_ThreadPool)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_ThreadPool)
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
    PARCThreadPool *pool = parcThreadPool_Create(6);
    assertNotNull(pool, "Expected non-null result from parcThreadPool_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcThreadPool_Acquire, pool);

    parcThreadPool_ShutdownNow(pool);

    parcThreadPool_Release(&pool);
    assertNull(pool, "Expected null result from parcThreadPool_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_ToString);
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

LONGBOW_TEST_CASE(Object, parcThreadPool_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Object, parcThreadPool_Copy)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    PARCThreadPool *copy = parcThreadPool_Copy(instance);
    assertTrue(parcThreadPool_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcThreadPool_ShutdownNow(instance);
    parcThreadPool_ShutdownNow(copy);

    parcThreadPool_Release(&instance);
    parcThreadPool_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_Display)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    parcThreadPool_Display(instance, 0);

    parcThreadPool_ShutdownNow(instance);

    parcThreadPool_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_Equals)
{
    PARCThreadPool *x = parcThreadPool_Create(6);
    PARCThreadPool *y = parcThreadPool_Create(6);
    PARCThreadPool *z = parcThreadPool_Create(6);

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcThreadPool_ShutdownNow(x);
    parcThreadPool_ShutdownNow(y);
    parcThreadPool_ShutdownNow(z);

    parcThreadPool_Release(&x);
    parcThreadPool_Release(&y);
    parcThreadPool_Release(&z);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_HashCode)
{
    PARCThreadPool *x = parcThreadPool_Create(6);
    PARCThreadPool *y = parcThreadPool_Create(6);

    parcObjectTesting_AssertHashCode(x, y);

    parcThreadPool_ShutdownNow(x);
    parcThreadPool_ShutdownNow(y);

    parcThreadPool_Release(&x);
    parcThreadPool_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_IsValid)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    assertTrue(parcThreadPool_IsValid(instance), "Expected parcThreadPool_Create to result in a valid instance.");

    parcThreadPool_ShutdownNow(instance);

    parcThreadPool_Release(&instance);
    assertFalse(parcThreadPool_IsValid(instance), "Expected parcThreadPool_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcThreadPool_ToJSON)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);

    PARCJSON *json = parcThreadPool_ToJSON(instance);

    parcJSON_Release(&json);

    parcThreadPool_ShutdownNow(instance);

    parcThreadPool_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_ToString)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);

    char *string = parcThreadPool_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcThreadPool_ToString");

    parcMemory_Deallocate((void **) &string);

    parcThreadPool_ShutdownNow(instance);

    parcThreadPool_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Execute);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

void *
_function(PARCFutureTask *task, void *parameter)
{
    printf("Hello World\n");
    return parameter;
}

LONGBOW_TEST_CASE(Object, parcThreadPool_Execute)
{
    dprintf(1, "-----\n");
    PARCThreadPool *pool = parcThreadPool_Create(6);

    PARCFutureTask *task = parcFutureTask_Create(_function, _function);
    parcThreadPool_Execute(pool, task);
    parcThreadPool_Execute(pool, task);
    parcThreadPool_Execute(pool, task);
    parcThreadPool_Execute(pool, task);
    parcThreadPool_Execute(pool, task);
    parcFutureTask_Release(&task);

    parcThreadPool_Shutdown(pool);
    bool shutdownSuccess = parcThreadPool_AwaitTermination(pool, PARCTimeout_Never);

    assertTrue(shutdownSuccess, "parcThreadPool_AwaitTermination timed-out");

    uint64_t count = parcThreadPool_GetCompletedTaskCount(pool);
    assertTrue(count == 5, "Expected 5, actual %lu", count);


    parcThreadPool_Release(&pool);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_ThreadPool);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


