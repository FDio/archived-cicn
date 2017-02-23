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
#include "../rta_CommandCreateProtocolStack.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(rta_CommandCreateProtocolStack)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_CommandCreateProtocolStack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_CommandCreateProtocolStack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_Create);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_IsValid_NULL);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_IsValid_BadCCNxStackConfig);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_GetConfig);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_GetStackId);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_GetStackConfig);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandCreateProtocolStack_Release);
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

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_Acquire)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);

    parcObjectTesting_AssertAcquire(createStack);

    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_Create)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
    assertNotNull(createStack, "Expected rtaCommandCreateProtocolStack_Create to return non-NULL.");

    assertTrue(createStack->stackId == stackId, "Expected stackId %d, actual %d", stackId, createStack->stackId);

    assertTrue(ccnxStackConfig_Equals(config, createStack->config),
               "ProtocolStackConfig instances are not equal");
    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_IsValid)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);

    assertTrue(rtaCommandCreateProtocolStack_IsValid(createStack),
               "Expected rtaCommandCreateProtocolStack_Create to return a valid instance.");

    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_IsValid_NULL)
{
    RtaCommandCreateProtocolStack *createStack = NULL;

    assertFalse(rtaCommandCreateProtocolStack_IsValid(createStack),
                "Expected rtaCommandCreateProtocolStack_Create to return a valid instance.");
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_IsValid_BadCCNxStackConfig)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
    CCNxStackConfig *original = createStack->config;
    createStack->config = NULL; // Make it bad.
    assertFalse(rtaCommandCreateProtocolStack_IsValid(createStack),
                "Expected rtaCommandCreateProtocolStack_Create to return a valid instance.");
    createStack->config = original;
    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_AssertValid)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);

    rtaCommandCreateProtocolStack_AssertValid(createStack);

    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_GetStackConfig)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);

    CCNxStackConfig *actual = rtaCommandCreateProtocolStack_GetStackConfig(createStack);

    assertTrue(ccnxStackConfig_Equals(config, actual),
               "CCNxStackConfig instances are not equal");

    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_GetConfig)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);

    assertTrue(ccnxStackConfig_Equals(config, createStack->config),
               "ProtocolStackConfig instances are not equal");

    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_GetStackId)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);

    int testStackId = rtaCommandCreateProtocolStack_GetStackId(createStack);
    assertTrue(testStackId == stackId, "Wrong value, got %d expected %d", testStackId, stackId);

    rtaCommandCreateProtocolStack_Release(&createStack);
    ccnxStackConfig_Release(&config);
}

LONGBOW_TEST_CASE(Global, rtaCommandCreateProtocolStack_Release)
{
    int stackId = 7;
    CCNxStackConfig *config = ccnxStackConfig_Create();
    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);

    RtaCommandCreateProtocolStack *second = rtaCommandCreateProtocolStack_Acquire(createStack);
    size_t secondRefCount = parcObject_GetReferenceCount(second);

    rtaCommandCreateProtocolStack_Release(&createStack);
    size_t thirdRefCount = parcObject_GetReferenceCount(second);

    assertTrue(thirdRefCount == secondRefCount - 1,
               "Wrong refcount after release, got %zu expected %zu", thirdRefCount, secondRefCount - 1);

    rtaCommandCreateProtocolStack_Release(&second);
    ccnxStackConfig_Release(&config);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_CommandCreateProtocolStack);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
