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

#include "../parc_Deque.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_StdlibMemory.h>

#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(parc_Deque)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(AcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Performance);
//    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Deque)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Deque)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(AcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcDeque_CreateRelease);
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcDeque_CreateRelease_WithEquals);
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcDeque_AcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(AcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(AcquireRelease)
{
    bool leaked = parcMemoryTesting_ExpectedOutstanding(0, "%s leaks memory \n", longBowTestCase_GetName(testCase)) != true;
    if (leaked) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("bailing\n");
        exit(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(AcquireRelease, parcDeque_CreateRelease)
{
    PARCDeque *deque = parcDeque_Create();
    assertNotNull(deque, "Expected non-null result from parcDeque_Create()");

    parcDeque_Release(&deque);
    assertNull(deque, "Expected parcDeque_Release to null the pointer");
}

LONGBOW_TEST_CASE(AcquireRelease, parcDeque_CreateRelease_WithEquals)
{
    PARCDeque *deque = parcDeque_CreateCustom(NULL, NULL);
    assertNotNull(deque, "Expected non-null result from parcDeque_Create()");

    parcDeque_Release(&deque);
    assertNull(deque, "Expected parcDeque_Release to null the pointer");
}

LONGBOW_TEST_CASE(AcquireRelease, parcDeque_AcquireRelease)
{
    PARCDeque *original = parcDeque_Create();
    assertNotNull(original, "Expected non-null result from parcDeque_Create()");

    parcObjectTesting_AssertAcquireReleaseContract(parcDeque_Acquire, original);

    PARCDeque *reference = parcDeque_Acquire(original);
    assertTrue(original == reference, "Expected the reference to be equal to the original.");

    parcDeque_Release(&original);
    assertNull(original, "Expected parcDeque_Release to null the pointer");

    parcDeque_Append(reference, (void *) 1);
    size_t expected = 1;
    size_t actual = parcDeque_Size(reference);
    assertTrue(expected == actual,
               "Expected size %zd, actual %zd", expected, actual);
    parcDeque_Release(&reference);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Append_One);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Append_Two);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_CreateDestroy);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_PeekFirst);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_PeekLast);

    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Prepend_One);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Prepend_Two);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Prepend_Three);

    LONGBOW_RUN_TEST_CASE(Global, parcDeque_IsEmpty);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_GetAtIndex);

    LONGBOW_RUN_TEST_CASE(Global, parcDeque_RemoveFirst);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_RemoveFirst_SingleElement);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_RemoveLast);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Size);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Display_NULL);

    LONGBOW_RUN_TEST_CASE(Global, parcDeque_Iterator);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    bool leaked = parcMemoryTesting_ExpectedOutstanding(0, "%s leaks memory \n", longBowTestCase_GetName(testCase)) != true;
    if (leaked) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcDeque_Append_One)
{
    PARCDeque *deque = parcDeque_Create();
    PARCDeque *actual = parcDeque_Append(deque, "element 1");

    assertTrue(deque == actual, "Expected parcDeque_Append to return its argument.");
    assertTrue(parcDeque_Size(deque) == 1, "Expected size of 1, actual %zd", parcDeque_Size(deque));

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_Append_Two)
{
    PARCDeque *deque = parcDeque_Create();
    parcDeque_Append(deque, "element 1");
    PARCDeque *actual = parcDeque_Append(deque, "element 2");

    assertTrue(deque == actual, "Expected parcDeque_Append to return its argument.");
    assertTrue(parcDeque_Size(deque) == 2, "Expected size of 2, actual %zd", parcDeque_Size(deque));

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_CreateDestroy)
{
    PARCDeque *deque = parcDeque_Create();
    assertNotNull(deque, "Expected non-null result from parcDeque_Create()");

    parcDeque_Release(&deque);
    assertNull(deque, "Expected parcDeque_Destroy to null the pointer");
}

LONGBOW_TEST_CASE(Global, parcDeque_PeekFirst)
{
    char *expected = "element 2";
    PARCDeque *deque = parcDeque_Create();
    parcDeque_Append(deque, expected);
    parcDeque_Append(deque, "element 2");
    parcDeque_Append(deque, "element 3");

    char *actual = parcDeque_PeekFirst(deque);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_PeekLast)
{
    char *expected = "element 2";
    PARCDeque *deque = parcDeque_Create();
    parcDeque_Append(deque, "element 1");
    parcDeque_Append(deque, "element 2");
    parcDeque_Append(deque, expected);

    char *actual = parcDeque_PeekLast(deque);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_Prepend_One)
{
    PARCDeque *deque = parcDeque_Create();
    PARCDeque *actual = parcDeque_Prepend(deque, "element 1");

    assertTrue(deque == actual, "Expected parcDeque_Append to return its argument.");
    assertTrue(parcDeque_Size(deque) == 1, "Expected size of 1, actual %zd", parcDeque_Size(deque));
    assertTrue(deque->head != NULL, "Expected head to be not null.");
    assertTrue(deque->head == deque->tail, "Expected head to be equal to the tail.");

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_Prepend_Two)
{
    PARCDeque *deque = parcDeque_Create();
    parcDeque_Prepend(deque, "element 2");
    PARCDeque *actual = parcDeque_Prepend(deque, "element 1");

    assertTrue(deque == actual, "Expected parcDeque_Prepend to return its argument.");
    assertTrue(parcDeque_Size(deque) == 2, "Expected size of 2, actual %zd", parcDeque_Size(deque));

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_Prepend_Three)
{
    char *expectedFirst = "expected first";
    char *expectedLast = "expected last";

    PARCDeque *deque = parcDeque_Create();
    parcDeque_Prepend(deque, expectedLast);
    parcDeque_Prepend(deque, "element 2");
    PARCDeque *actual = parcDeque_Prepend(deque, expectedFirst);

    assertTrue(deque == actual, "Expected parcDeque_Prepend to return its argument.");
    assertTrue(parcDeque_Size(deque) == 3, "Expected size of 3, actual %zd", parcDeque_Size(deque));

    char *peek = parcDeque_PeekFirst(deque);
    assertTrue(strcmp(expectedFirst, peek) == 0, "Expected '%s' actual '%s'", expectedFirst, peek);

    peek = parcDeque_PeekLast(deque);
    assertTrue(strcmp(expectedLast, peek) == 0, "Expected '%s' actual '%s'", expectedLast, peek);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_RemoveFirst)
{
    char *expectedFirst = "expected 1st";
    char *expectedLast = "expected last";

    PARCDeque *deque = parcDeque_Create();
    parcDeque_Prepend(deque, expectedLast);
    parcDeque_Prepend(deque, "element 2");
    parcDeque_Prepend(deque, expectedFirst);

    char *peek = parcDeque_RemoveFirst(deque);
    assertTrue(strcmp(expectedFirst, peek) == 0, "Expected '%s' actual '%s'", expectedFirst, peek);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_RemoveFirst_SingleElement)
{
    char *expectedFirst = "expected 1st";

    PARCDeque *deque = parcDeque_Create();
    parcDeque_Prepend(deque, expectedFirst);

    char *peek = parcDeque_RemoveFirst(deque);
    assertTrue(strcmp(expectedFirst, peek) == 0,
               "Expected '%s' actual '%s'", expectedFirst, peek);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_RemoveLast)
{
    char *expectedFirst = "expected 1st";
    char *expectedLast = "expected last";

    PARCDeque *deque = parcDeque_Create();
    parcDeque_Prepend(deque, expectedLast);
    parcDeque_Prepend(deque, "element 2");
    parcDeque_Prepend(deque, expectedFirst);

    char *peek = parcDeque_RemoveLast(deque);
    assertTrue(strcmp(expectedLast, peek) == 0,
               "Expected '%s' actual '%s'", expectedLast, peek);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_RemoveLast_SingleElement)
{
    char *expectedFirst = "expected 1st";

    PARCDeque *deque = parcDeque_Create();
    parcDeque_Prepend(deque, expectedFirst);

    char *peek = parcDeque_RemoveLast(deque);
    assertTrue(strcmp(expectedFirst, peek) == 0,
               "Expected '%s' actual '%s'", expectedFirst, peek);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_Size)
{
    char *expectedFirst = "expected 1st";
    char *expectedLast = "expected last";

    PARCDeque *deque = parcDeque_Create();
    parcDeque_Prepend(deque, expectedLast);
    parcDeque_Prepend(deque, "element 2");
    parcDeque_Prepend(deque, expectedFirst);

    assertTrue(parcDeque_Size(deque) == 3,
               "Expected 3, actual %zd", parcDeque_Size(deque));
    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_IsEmpty)
{
    char *expectedLast = "expected last";

    PARCDeque *deque = parcDeque_Create();

    assertTrue(parcDeque_IsEmpty(deque), "Expected true.");
    parcDeque_Prepend(deque, expectedLast);
    assertFalse(parcDeque_IsEmpty(deque), "Expected false.");
    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_GetAtIndex)
{
    char *expected0 = "expected 1";
    char *expected1 = "expected 2";
    char *expected2 = "expected 3";

    PARCDeque *deque = parcDeque_Create();
    parcDeque_Append(deque, expected0);
    parcDeque_Append(deque, expected1);
    parcDeque_Append(deque, expected2);

    char *actual;
    actual = parcDeque_GetAtIndex(deque, 0);
    assertTrue(strcmp(actual, expected0) == 0, "Expected '%s', actual '%s", expected0, actual);
    actual = parcDeque_GetAtIndex(deque, 1);
    assertTrue(strcmp(actual, expected1) == 0, "Expected '%s', actual '%s", expected1, actual);
    actual = parcDeque_GetAtIndex(deque, 2);
    assertTrue(strcmp(actual, expected2) == 0, "Expected '%s', actual '%s", expected2, actual);

    parcDeque_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcDeque_Equals)
{
    PARCDeque *x = parcDeque_Create();
    parcDeque_Append(x, (void *) 0);
    parcDeque_Append(x, (void *) 1);
    parcDeque_Append(x, (void *) 2);
    parcDeque_Append(x, (void *) 3);
    parcDeque_Append(x, (void *) 4);
    parcDeque_Append(x, (void *) 5);
    PARCDeque *y = parcDeque_Create();
    parcDeque_Append(y, (void *) 0);
    parcDeque_Append(y, (void *) 1);
    parcDeque_Append(y, (void *) 2);
    parcDeque_Append(y, (void *) 3);
    parcDeque_Append(y, (void *) 4);
    parcDeque_Append(y, (void *) 5);
    PARCDeque *z = parcDeque_Create();
    parcDeque_Append(z, (void *) 0);
    parcDeque_Append(z, (void *) 1);
    parcDeque_Append(z, (void *) 2);
    parcDeque_Append(z, (void *) 3);
    parcDeque_Append(z, (void *) 4);
    parcDeque_Append(z, (void *) 5);
    PARCDeque *u1 = parcDeque_Create();
    parcDeque_Append(u1, (void *) 0);
    parcDeque_Append(u1, (void *) 1);
    parcDeque_Append(u1, (void *) 2);
    parcDeque_Append(u1, (void *) 3);
    parcDeque_Append(u1, (void *) 4);
    PARCDeque *u2 = parcDeque_Create();
    parcDeque_Append(u2, (void *) 0);
    parcDeque_Append(u2, (void *) 1);
    parcDeque_Append(u2, (void *) 2);
    parcDeque_Append(u2, (void *) 3);
    parcDeque_Append(u2, (void *) 4);
    parcDeque_Append(u2, (void *) 4);

    parcObjectTesting_AssertEqualsFunction(parcDeque_Equals, x, y, z, u1, u2, NULL);

    parcDeque_Release(&x);
    parcDeque_Release(&y);
    parcDeque_Release(&z);
    parcDeque_Release(&u1);
    parcDeque_Release(&u2);
}

LONGBOW_TEST_CASE(Global, parcDeque_Copy)
{
    PARCDeque *x = parcDeque_Create();
    parcDeque_Append(x, (void *) 0);
    parcDeque_Append(x, (void *) 1);
    parcDeque_Append(x, (void *) 2);
    parcDeque_Append(x, (void *) 3);
    parcDeque_Append(x, (void *) 4);
    parcDeque_Append(x, (void *) 5);

    PARCDeque *y = parcDeque_Copy(x);

    assertTrue(parcDeque_Equals(x, y), "Expected the copy to be equal to the original.");

    parcDeque_Release(&y);
    parcDeque_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcDeque_Display)
{
    PARCDeque *x = parcDeque_Create();
    parcDeque_Append(x, (void *) 0);
    parcDeque_Append(x, (void *) 1);
    parcDeque_Append(x, (void *) 2);
    parcDeque_Append(x, (void *) 3);
    parcDeque_Append(x, (void *) 4);
    parcDeque_Append(x, (void *) 5);

    parcDeque_Display(x, 0);

    parcDeque_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcDeque_Display_NULL)
{
    parcDeque_Display(NULL, 0);
}

LONGBOW_TEST_CASE(Global, parcDeque_Iterator)
{
    PARCDeque *x = parcDeque_Create();
    for (size_t i = 0; i < 100; i++) {
        parcDeque_Append(x, (void *) i);
    }

    PARCIterator *iterator = parcDeque_Iterator(x);
    size_t expected = 0;
    while (parcIterator_HasNext(iterator)) {
        size_t actual = (size_t) parcIterator_Next(iterator);
        assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
        expected++;
    }
    parcIterator_Release(&iterator);

    parcDeque_Release(&x);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _parcDequeNode_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    bool leaked = parcMemoryTesting_ExpectedOutstanding(0, "%s leaks memory \n", longBowTestCase_GetName(testCase)) != true;
    if (leaked) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _parcDequeNode_Create)
{
    void *element = "element";
    struct parc_deque_node *previous = NULL;
    struct parc_deque_node *next = NULL;

    struct parc_deque_node *actual = _parcDequeNode_Create(element, previous, next);
    _parcDequeNode_Destroy(NULL, &actual);
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, parcDeque_GetAtIndex_OutOfBounds);
}

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    PARCDeque *deque = parcDeque_Create();
    longBowTestCase_SetClipBoardData(testCase, deque);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    PARCDeque *deque = longBowTestCase_GetClipBoardData(testCase);
    parcDeque_Release(&deque);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcDeque_GetAtIndex_OutOfBounds, .event = &LongBowTrapOutOfBounds)
{
    char *expected0 = "expected 1";
    char *expected1 = "expected 2";
    char *expected2 = "expected 3";

    PARCDeque *deque = longBowTestCase_GetClipBoardData(testCase);
    parcDeque_Append(deque, expected0);
    parcDeque_Append(deque, expected1);
    parcDeque_Append(deque, expected2);

    parcDeque_GetAtIndex(deque, 3);
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcQueue_Append);
    LONGBOW_RUN_TEST_CASE(Performance, parcQueue_N2);
    LONGBOW_RUN_TEST_CASE(Performance, parcQueue_Iterator);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    parcMemory_SetInterface(&PARCStdlibMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Performance, parcQueue_Append)
{
    PARCDeque *x = parcDeque_Create();

    for (size_t i = 0; i < 100000; i++) {
        parcDeque_Append(x, (void *) i);
    }

    parcDeque_Release(&x);
}

LONGBOW_TEST_CASE(Performance, parcQueue_N2)
{
    PARCDeque *x = parcDeque_Create();
    for (size_t i = 0; i < 100000; i++) {
        parcDeque_Append(x, (void *) i);
    }

    for (size_t expected = 0; expected < parcDeque_Size(x); expected++) {
        size_t actual = (size_t) parcDeque_GetAtIndex(x, expected);
        assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
    }

    parcDeque_Release(&x);
}

LONGBOW_TEST_CASE(Performance, parcQueue_Iterator)
{
    PARCDeque *x = parcDeque_Create();
    for (size_t i = 0; i < 100000; i++) {
        parcDeque_Append(x, (void *) i);
    }

    PARCIterator *iterator = parcDeque_Iterator(x);
    size_t expected = 0;
    while (parcIterator_HasNext(iterator)) {
        size_t actual = (size_t) parcIterator_Next(iterator);
        assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
        expected++;
    }
    parcIterator_Release(&iterator);

    parcDeque_Release(&x);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Deque);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
