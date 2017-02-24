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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../rta_CommandDestroyProtocolStack.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(rta_CommandDestroyProtocolStack)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_CommandDestroyProtocolStack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_CommandDestroyProtocolStack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandDestroyProtocolStack_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandDestroyProtocolStack_Create);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandDestroyProtocolStack_GetStackId);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandDestroyProtocolStack_Release);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, rtaCommandDestroyProtocolStack_Acquire)
{
    int stackId = 7;
    RtaCommandDestroyProtocolStack *destroyStack = rtaCommandDestroyProtocolStack_Create(stackId);
    size_t firstRefCount = parcObject_GetReferenceCount(destroyStack);

    RtaCommandDestroyProtocolStack *second = rtaCommandDestroyProtocolStack_Acquire(destroyStack);
    size_t secondRefCount = parcObject_GetReferenceCount(second);

    assertTrue(secondRefCount == firstRefCount + 1, "Wrong refcount after acquire, got %zu expected %zu", secondRefCount, firstRefCount + 1);

    rtaCommandDestroyProtocolStack_Release(&destroyStack);
    rtaCommandDestroyProtocolStack_Release(&second);
}

LONGBOW_TEST_CASE(Global, rtaCommandDestroyProtocolStack_Create)
{
    int stackId = 7;
    RtaCommandDestroyProtocolStack *destroyStack = rtaCommandDestroyProtocolStack_Create(stackId);
    assertNotNull(destroyStack, "Got null from create");
    assertTrue(destroyStack->stackId == stackId, "Internal stackId wrong, got %d expected %d", destroyStack->stackId, stackId);
    rtaCommandDestroyProtocolStack_Release(&destroyStack);
}

LONGBOW_TEST_CASE(Global, rtaCommandDestroyProtocolStack_GetStackId)
{
    int stackId = 7;
    RtaCommandDestroyProtocolStack *destroyStack = rtaCommandDestroyProtocolStack_Create(stackId);

    int testStackId = rtaCommandDestroyProtocolStack_GetStackId(destroyStack);
    assertTrue(testStackId == stackId, "Wrong value, got %d expected %d", testStackId, stackId);

    rtaCommandDestroyProtocolStack_Release(&destroyStack);
}

LONGBOW_TEST_CASE(Global, rtaCommandDestroyProtocolStack_Release)
{
    int stackId = 7;
    RtaCommandDestroyProtocolStack *destroyStack = rtaCommandDestroyProtocolStack_Create(stackId);

    RtaCommandDestroyProtocolStack *second = rtaCommandDestroyProtocolStack_Acquire(destroyStack);
    size_t secondRefCount = parcObject_GetReferenceCount(second);

    rtaCommandDestroyProtocolStack_Release(&destroyStack);
    size_t thirdRefCount = parcObject_GetReferenceCount(second);

    assertTrue(thirdRefCount == secondRefCount - 1, "Wrong refcount after release, got %zu expected %zu", thirdRefCount, secondRefCount - 1);

    rtaCommandDestroyProtocolStack_Release(&second);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_CommandDestroyProtocolStack);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
