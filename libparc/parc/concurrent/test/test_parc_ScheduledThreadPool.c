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
#include "../parc_ScheduledThreadPool.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_ScheduledThreadPool)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_ScheduledThreadPool)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_ScheduledThreadPool)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    longBowTestCase_SetInt(testCase, "initalAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    int initialAllocations = longBowTestCase_GetInt(testCase, "initalAllocations");
    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCScheduledThreadPool *instance = parcScheduledThreadPool_Create(3);
    assertNotNull(instance, "Expected non-null result from parcScheduledThreadPool_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcScheduledThreadPool_Acquire, instance);
    parcScheduledThreadPool_ShutdownNow(instance);

    assertTrue(parcObject_GetReferenceCount(instance) == 1, "Expected 1 reference count. Actual %llu", parcObject_GetReferenceCount(instance));

    parcScheduledThreadPool_Release(&instance);
    assertNull(instance, "Expected null result from parcScheduledThreadPool_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcScheduledThreadPool_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Object)
{
    longBowTestCase_SetInt(testCase, "initalAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Object)
{
    int initialAllocations = longBowTestCase_GetInt(testCase, "initalAllocations");
    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_Compare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_Copy)
{
    PARCScheduledThreadPool *instance = parcScheduledThreadPool_Create(3);
    PARCScheduledThreadPool *copy = parcScheduledThreadPool_Copy(instance);
    assertTrue(parcScheduledThreadPool_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcScheduledThreadPool_ShutdownNow(instance);
    parcScheduledThreadPool_ShutdownNow(copy);

    parcScheduledThreadPool_Release(&instance);
    parcScheduledThreadPool_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_Display)
{
    PARCScheduledThreadPool *instance = parcScheduledThreadPool_Create(2);
    parcScheduledThreadPool_Display(instance, 0);
    parcScheduledThreadPool_ShutdownNow(instance);
    parcScheduledThreadPool_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_Equals)
{
    PARCScheduledThreadPool *x = parcScheduledThreadPool_Create(2);
    PARCScheduledThreadPool *y = parcScheduledThreadPool_Create(2);
    PARCScheduledThreadPool *z = parcScheduledThreadPool_Create(2);
    PARCScheduledThreadPool *u1 = parcScheduledThreadPool_Create(3);

    parcObjectTesting_AssertEquals(x, y, z, u1, NULL);

    parcScheduledThreadPool_ShutdownNow(x);
    parcScheduledThreadPool_ShutdownNow(y);
    parcScheduledThreadPool_ShutdownNow(z);
    parcScheduledThreadPool_ShutdownNow(u1);

    parcScheduledThreadPool_Release(&x);
    parcScheduledThreadPool_Release(&y);
    parcScheduledThreadPool_Release(&z);
    parcScheduledThreadPool_Release(&u1);
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_HashCode)
{
    PARCScheduledThreadPool *x = parcScheduledThreadPool_Create(2);
    PARCScheduledThreadPool *y = parcScheduledThreadPool_Create(2);

    parcObjectTesting_AssertHashCode(x, y);

    parcScheduledThreadPool_ShutdownNow(x);
    parcScheduledThreadPool_ShutdownNow(y);

    parcScheduledThreadPool_Release(&x);
    parcScheduledThreadPool_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_IsValid)
{
    PARCScheduledThreadPool *instance = parcScheduledThreadPool_Create(2);
    assertTrue(parcScheduledThreadPool_IsValid(instance), "Expected parcScheduledThreadPool_Create to result in a valid instance.");

    parcScheduledThreadPool_ShutdownNow(instance);

    parcScheduledThreadPool_Release(&instance);
    assertFalse(parcScheduledThreadPool_IsValid(instance), "Expected parcScheduledThreadPool_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_ToJSON)
{
    PARCScheduledThreadPool *instance = parcScheduledThreadPool_Create(2);

    PARCJSON *json = parcScheduledThreadPool_ToJSON(instance);

    parcJSON_Release(&json);

    parcScheduledThreadPool_ShutdownNow(instance);
    parcScheduledThreadPool_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcScheduledThreadPool_ToString)
{
    PARCScheduledThreadPool *instance = parcScheduledThreadPool_Create(2);

    char *string = parcScheduledThreadPool_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcScheduledThreadPool_ToString");

    parcMemory_Deallocate((void **) &string);
    parcScheduledThreadPool_ShutdownNow(instance);
    parcScheduledThreadPool_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, OneJob);
    LONGBOW_RUN_TEST_CASE(Specialization, Idle);
    LONGBOW_RUN_TEST_CASE(Specialization, parcScheduledThreadPool_Schedule);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    longBowTestCase_SetInt(testCase, "initalAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    int initialAllocations = longBowTestCase_GetInt(testCase, "initalAllocations");
    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Specialization, Idle)
{
    PARCScheduledThreadPool *pool = parcScheduledThreadPool_Create(3);

    sleep(2);

    parcScheduledThreadPool_ShutdownNow(pool);

    parcScheduledThreadPool_Release(&pool);
}

static void *
_function(PARCFutureTask *task, void *parameter)
{
    printf("Hello World\n");
    return parameter;
}

LONGBOW_TEST_CASE(Specialization, OneJob)
{
    PARCScheduledThreadPool *pool = parcScheduledThreadPool_Create(3);

    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    parcScheduledThreadPool_Schedule(pool, task, parcTimeout_MilliSeconds(2000));
    printf("references %lld\n", parcObject_GetReferenceCount(task));
    parcFutureTask_Release(&task);

    sleep(5);

    parcScheduledThreadPool_ShutdownNow(pool);

    parcScheduledThreadPool_Release(&pool);
}

LONGBOW_TEST_CASE(Specialization, parcScheduledThreadPool_Schedule)
{
    PARCScheduledThreadPool *pool = parcScheduledThreadPool_Create(3);

    PARCFutureTask *task = parcFutureTask_Create(_function, _function);

    parcScheduledThreadPool_Schedule(pool, task, parcTimeout_MilliSeconds(2000));

    parcFutureTask_Release(&task);

    parcScheduledThreadPool_Shutdown(pool);
//    parcScheduledThreadPool_AwaitTermination(pool, PARCTimeout_Never);

//    uint64_t count = parcScheduledThreadPool_GetCompletedTaskCount(pool);
//    assertTrue(count == 5, "Expected 5, actual %lld", count);

    parcScheduledThreadPool_ShutdownNow(pool);

    parcScheduledThreadPool_Release(&pool);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_ScheduledThreadPool);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
