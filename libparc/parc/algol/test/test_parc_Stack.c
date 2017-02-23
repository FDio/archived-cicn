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
 * @header <#Headline Name#>
 * @abstract <#Abstract#>
 * @discussion
 *     <#Discussion#>
 *
 */
#include "../parc_Stack.c"

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.

#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Deque.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(test_parc_Stack)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_Stack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_Stack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcStack_IsEmpty_PARCDeque);
    LONGBOW_RUN_TEST_CASE(Global, parcStack_IsEmpty_PARCArrayList);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcStack_IsEmpty_PARCDeque)
{
    PARCStackInterface dequeAsStack = {
        .parcStack_Release = (void (*)(void **))parcDeque_Release,
        .parcStack_IsEmpty = (bool (*)(const void *))parcDeque_IsEmpty,
        .parcStack_Peek    = (void *(*)(const void *))parcDeque_PeekLast,
        .parcStack_Pop     = (void *(*)(void *))parcDeque_RemoveLast,
        .parcStack_Push    = (void *(*)(void *,                          void *))parcDeque_Append,
        .parcStack_Search  = NULL
    };

    PARCStack *stack = parcStack(parcDeque_Create(), &dequeAsStack);

    bool actual = parcStack_IsEmpty(stack);
    parcStack_Release(&stack);
    assertTrue(actual, "Expected the stack to be empty.");
}

LONGBOW_TEST_CASE(Global, parcStack_IsEmpty_PARCArrayList)
{
    PARCStackInterface arrayListAsStack = {
        .parcStack_Release = (void (*)(void **))parcArrayList_Destroy,
        .parcStack_IsEmpty = (bool (*)(const void *))parcArrayList_IsEmpty,
        .parcStack_Peek    = (void *(*)(const void *))parcArrayList_Peek,
        .parcStack_Pop     = (void *(*)(void *))parcArrayList_Pop,
        .parcStack_Push    = (void *(*)(void *,                            void *))parcArrayList_Add,
        .parcStack_Search  = NULL
    };

    PARCStack *stack = parcStack(parcArrayList_Create(NULL), &arrayListAsStack);

    bool actual = parcStack_IsEmpty(stack);
    parcStack_Release(&stack);
    assertTrue(actual, "Expected the stack to be empty.");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_Stack);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
