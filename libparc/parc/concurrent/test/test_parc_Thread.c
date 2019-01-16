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

LONGBOW_TEST_RUNNER(parc_Thread)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Thread)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Thread)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    longBowTestCase_Set(testCase, "object", buffer);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    parcBuffer_Release(&buffer);

#if INTPTR_MAX == INT32_MAX
    uint32_t initialAllocations = (uint32_t) longBowTestCase_Get(testCase, "initialAllocations");
#else
    uint64_t initialAllocations = (uint64_t) longBowTestCase_Get(testCase, "initialAllocations");
#endif

    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

static void *
_function(PARCThread *thread, void *parameter)
{
    return NULL;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *thread = parcThread_Create(_function, buffer);

    assertNotNull(thread, "Expected non-null result from parcThread_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcThread_Acquire, thread);

    parcThread_Release(&thread);
    assertNull(thread, "Expected null result from parcThread_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcThread_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Object)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    longBowTestCase_Set(testCase, "object", buffer);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Object)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    parcBuffer_Release(&buffer);

#if INTPTR_MAX == INT32_MAX
    uint32_t initialAllocations = (uint32_t) longBowTestCase_Get(testCase, "initialAllocations");
#else
    uint64_t initialAllocations = (uint64_t) longBowTestCase_Get(testCase, "initialAllocations");
#endif

    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Object, parcThread_Compare)
{
//    testUnimplemented("");
}

LONGBOW_TEST_CASE(Object, parcThread_Copy)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *instance = parcThread_Create(_function, buffer);
    PARCThread *copy = parcThread_Copy(instance);

    assertTrue(parcThread_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcThread_Release(&instance);
    parcThread_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcThread_Display)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *instance = parcThread_Create(_function, buffer);
    parcThread_Display(instance, 0);


    parcThread_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcThread_Equals)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *x = parcThread_Create(_function, buffer);
    PARCThread *y = parcThread_Create(_function, buffer);
    PARCThread *z = parcThread_Create(_function, buffer);

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcThread_Release(&x);
    parcThread_Release(&y);
    parcThread_Release(&z);
}

LONGBOW_TEST_CASE(Object, parcThread_HashCode)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *x = parcThread_Create(_function, buffer);
    PARCThread *y = parcThread_Create(_function, buffer);

    parcObjectTesting_AssertHashCode(x, y);

    parcThread_Release(&x);
    parcThread_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcThread_IsValid)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *instance = parcThread_Create(_function, buffer);
    assertTrue(parcThread_IsValid(instance), "Expected parcThread_Create to result in a valid instance.");


    parcThread_Release(&instance);
    assertFalse(parcThread_IsValid(instance), "Expected parcThread_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcThread_ToJSON)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *instance = parcThread_Create(_function, buffer);

    PARCJSON *json = parcThread_ToJSON(instance);

    parcJSON_Release(&json);


    parcThread_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcThread_ToString)
{
    PARCBuffer *buffer = longBowTestCase_Get(testCase, "object");
    PARCThread *instance = parcThread_Create(_function, buffer);

    char *string = parcThread_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcThread_ToString");

    parcMemory_Deallocate((void **) &string);

    parcThread_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    LONGBOW_RUN_TEST_CASE(Object, parcThread_Execute);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
#if INTPTR_MAX == INT32_MAX
    uint32_t initialAllocations = (uint32_t) longBowTestCase_Get(testCase, "initialAllocations");
#else
    uint64_t initialAllocations = (uint64_t) longBowTestCase_Get(testCase, "initialAllocations");
#endif

    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Object, parcThread_Execute)
{
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Thread);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


