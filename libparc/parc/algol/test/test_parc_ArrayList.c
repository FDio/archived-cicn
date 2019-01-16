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

#include "../parc_ArrayList.c"
#include <LongBow/unit-test.h>

#include <stdio.h>
#include <string.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

#include <parc/algol/parc_Buffer.h>

LONGBOW_TEST_RUNNER(PARC_ArrayList)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

LONGBOW_TEST_RUNNER_SETUP(PARC_ArrayList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(PARC_ArrayList)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Add);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_AddAll);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Copy);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Destroy);

    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_CustomDestroyer);

    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Equals_Contract);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Equals_Contract_Deep);

    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_FromInitialCapacity);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Get);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_New);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Size);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Remove_AtIndex_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Remove_AtIndex);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_Remove_AtIndex_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_RemoveAndDestroy_AtIndex_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_RemoveAndDestroy_AtIndex);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_RemoveAndDestroy_AtIndex_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_ArrayList_IsEmpty);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Add)
{
    PARCArrayList *array = parcArrayList_Create(parcArrayList_StdlibFreeFunction);

    parcArrayList_Add(array, 0);
    size_t actual = parcArrayList_Size(array);

    assertTrue(1 == actual, "Expected=%d, actual=%zu", 1, actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_AddAll)
{
    PARCArrayList *array = parcArrayList_Create(parcArrayList_StdlibFreeFunction);

    void *elements[] = {
        strdup("a"),
        strdup("b"),
        strdup("c"),
    };

    parcArrayList_AddAll(array, elements, 3);
    size_t actual = parcArrayList_Size(array);

    assertTrue(3 == actual, "Expected=%d, actual=%zu", 3, actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Copy)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";

    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, a);
    parcArrayList_Add(array, b);
    parcArrayList_Add(array, c);

    PARCArrayList *copy = parcArrayList_Copy(array);
    assertTrue(parcArrayList_Equals(array, copy), "Expected arrays to be equal.");

    parcArrayList_Destroy(&copy);
    parcArrayList_Destroy(&array);
}

static void
testCustomDestroyer(void **bufferVoidPtr)
{
    PARCBuffer **bufferPtr = (PARCBuffer **) bufferVoidPtr;
    parcBuffer_Release(bufferPtr);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_CustomDestroyer)
{
    size_t before = parcMemory_Outstanding();

    PARCArrayList *array = parcArrayList_Create(testCustomDestroyer);
    PARCBuffer *buffer = parcBuffer_Allocate(20);
    parcArrayList_Add(array, parcBuffer_Acquire(buffer));
    parcBuffer_Release(&buffer);
    parcArrayList_Destroy(&array);

    size_t after = parcMemory_Outstanding();

    assertTrue(before == after, "Memory imbalance after using custom destroy, expected %zu got %zu", before, after);
}


LONGBOW_TEST_CASE(Global, PARC_ArrayList_Destroy)
{
    PARCArrayList *array = parcArrayList_Create(parcArrayList_StdlibFreeFunction);

    parcArrayList_Destroy(&array);
    assertNull(array, "Expected null.");
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Equals_Empty)
{
    PARCArrayList *a = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    PARCArrayList *b = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    assertTrue(parcArrayList_Equals(a, b), "Equal values were expected to be equal");

    parcArrayList_Destroy(&a);
    parcArrayList_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Equals_Same)
{
    PARCArrayList *a = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    assertTrue(parcArrayList_Equals(a, a), "Expected the same array list to be equal to itself.");

    parcArrayList_Destroy(&a);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Equals_Contract)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";
    char d[] = "potato";

    PARCArrayList *x = parcArrayList_Create(NULL);
    parcArrayList_Add(x, a);
    parcArrayList_Add(x, b);
    parcArrayList_Add(x, c);

    PARCArrayList *y = parcArrayList_Create(NULL);
    parcArrayList_Add(y, a);
    parcArrayList_Add(y, b);
    parcArrayList_Add(y, c);

    PARCArrayList *z = parcArrayList_Create(NULL);
    parcArrayList_Add(z, a);
    parcArrayList_Add(z, b);
    parcArrayList_Add(z, c);

    PARCArrayList *u1 = parcArrayList_Create(NULL);
    parcArrayList_Add(u1, a);
    parcArrayList_Add(u1, b);

    PARCArrayList *u2 = parcArrayList_Create(NULL);
    parcArrayList_Add(u1, a);
    parcArrayList_Add(u2, b);
    parcArrayList_Add(u2, c);
    parcArrayList_Add(u2, c);

    PARCArrayList *u3 = parcArrayList_Create(NULL);
    parcArrayList_Add(u3, a);
    parcArrayList_Add(u3, b);
    parcArrayList_Add(u3, d);

    parcObjectTesting_AssertEqualsFunction(parcArrayList_Equals, x, y, z, u1, u2, u3);

    parcArrayList_Destroy(&x);
    parcArrayList_Destroy(&y);
    parcArrayList_Destroy(&z);
    parcArrayList_Destroy(&u1);
    parcArrayList_Destroy(&u2);
    parcArrayList_Destroy(&u3);
}

static bool
stringEquals(void *x, void *y)
{
    return strcmp((char *) x, (char *) y) == 0;
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Equals_Contract_Deep)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";
    char d[] = "potato";

    PARCArrayList *x = parcArrayList_Create_Capacity(stringEquals, NULL, 0);
    parcArrayList_Add(x, a);
    parcArrayList_Add(x, b);
    parcArrayList_Add(x, c);

    PARCArrayList *y = parcArrayList_Create_Capacity(stringEquals, NULL, 0);
    parcArrayList_Add(y, a);
    parcArrayList_Add(y, b);
    parcArrayList_Add(y, c);

    PARCArrayList *z = parcArrayList_Create_Capacity(stringEquals, NULL, 0);
    parcArrayList_Add(z, a);
    parcArrayList_Add(z, b);
    parcArrayList_Add(z, c);

    PARCArrayList *u1 = parcArrayList_Create_Capacity(stringEquals, NULL, 0);
    parcArrayList_Add(u1, a);
    parcArrayList_Add(u1, b);

    PARCArrayList *u2 = parcArrayList_Create_Capacity(stringEquals, NULL, 0);
    parcArrayList_Add(u2, a);
    parcArrayList_Add(u2, b);
    parcArrayList_Add(u2, c);
    parcArrayList_Add(u2, c);

    PARCArrayList *u3 = parcArrayList_Create_Capacity(stringEquals, NULL, 0);
    parcArrayList_Add(u3, a);
    parcArrayList_Add(u3, b);
    parcArrayList_Add(u3, d);

    parcObjectTesting_AssertEqualsFunction(parcArrayList_Equals, x, y, z, u1, u2, u3);

    parcArrayList_Destroy(&x);
    parcArrayList_Destroy(&y);
    parcArrayList_Destroy(&z);
    parcArrayList_Destroy(&u1);
    parcArrayList_Destroy(&u2);
    parcArrayList_Destroy(&u3);
}

LONGBOW_TEST_CASE(Local, PARC_ArrayList_EnsureRemaining_Empty)
{
    PARCArrayList *array = parcArrayList_Create(NULL);
    size_t expected = 4;
    _ensureRemaining(array, expected);

    size_t actual = _remaining(array);

    assertTrue(actual >= expected, "Expected >= %zd, actual=%zd", expected, actual);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Local, PARC_ArrayList_EnsureRemaining_NonEmpty)
{
    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, 0);

    size_t expected = 5;
    _ensureRemaining(array, expected);

    size_t actual = _remaining(array);

    assertTrue(actual >= expected, "Expected >= %zd, actual=%zd", expected, actual);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_FromInitialCapacity)
{
    PARCArrayList *array = parcArrayList_Create_Capacity(NULL, parcArrayList_StdlibFreeFunction, 10);
    size_t actual = parcArrayList_Size(array);

    assertTrue(0 == actual, "Expected=%d, actual=%zu", 0, actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Get)
{
    PARCArrayList *array = parcArrayList_Create(parcArrayList_StdlibFreeFunction);

    char *expected = strdup("Hello World");
    parcArrayList_Add(array, expected);

    char *actual = parcArrayList_Get(array, 0);

    assertTrue(expected == actual, "Expected=%p, actual=%p", (void *) expected, (void *) actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_New)
{
    PARCArrayList *array = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    size_t size = parcArrayList_Size(array);
    assertTrue(0 == size, "Expected %d actual=%zu", 0, size);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Size)
{
    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, 0);

    size_t size = parcArrayList_Size(array);
    assertTrue(1 == size, "Expected %d actual=%zu", 1, size);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_IsEmpty)
{
    PARCArrayList *array = parcArrayList_Create(NULL);
    assertTrue(parcArrayList_IsEmpty(array), "Expected a new array to be empty.");

    parcArrayList_Add(array, 0);
    assertFalse(parcArrayList_IsEmpty(array), "Expected an array with more than zero elements to be empty.");

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex)
{
    PARCArrayList *array = parcArrayList_Create(NULL);

    parcArrayList_Add(array, (void *) 1);
    parcArrayList_Add(array, (void *) 2);
    size_t actual = parcArrayList_Size(array);

    assertTrue(2 == actual, "Expected=%d, actual=%zu", 2, actual);

    parcArrayList_InsertAtIndex(array, 1, (void *) 3);

    actual = parcArrayList_Size(array);
    assertTrue(3 == actual, "Expected=%d, actual=%zu", 3, actual);

    void *element0 = parcArrayList_Get(array, 0);
    assertTrue(element0 == (void *) 1, "Element 1 moved?");

    void *element1 = parcArrayList_Get(array, 1);
    assertTrue(element1 == (void *) 3, "Element 1 moved?");

    void *element2 = parcArrayList_Get(array, 2);
    assertTrue(element2 == (void *) 2, "Element 1 moved?");

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex_Empty)
{
    PARCArrayList *array = parcArrayList_Create(NULL);

    parcArrayList_InsertAtIndex(array, 0, (void *) 3);

    size_t actual = parcArrayList_Size(array);

    assertTrue(1 == actual, "Expected=%d, actual=%zu", 1, actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex_First)
{
    PARCArrayList *array = parcArrayList_Create(NULL);

    parcArrayList_Add(array, (void *) 1);
    parcArrayList_InsertAtIndex(array, 0, (void *) 2);
    size_t actual = parcArrayList_Size(array);

    assertTrue(2 == actual, "Expected=%d, actual=%zu", 2, actual);

    void *element0 = parcArrayList_Get(array, 0);
    assertTrue(element0 == (void *) 2, "Element 1 moved?");

    void *element1 = parcArrayList_Get(array, 1);
    assertTrue(element1 == (void *) 1, "Element 1 moved?");

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_InsertAtIndex_Last)
{
    PARCArrayList *array = parcArrayList_Create(NULL);

    parcArrayList_Add(array, (void *) 1);
    parcArrayList_Add(array, (void *) 2);
    size_t actual = parcArrayList_Size(array);

    assertTrue(2 == actual, "Expected=%d, actual=%zu", 2, actual);

    parcArrayList_InsertAtIndex(array, 2, (void *) 3);

    actual = parcArrayList_Size(array);
    assertTrue(3 == actual, "Expected=%d, actual=%zu", 3, actual);

    void *element0 = parcArrayList_Get(array, 0);
    assertTrue(element0 == (void *) 1, "Element 1 moved?");

    void *element1 = parcArrayList_Get(array, 1);
    assertTrue(element1 == (void *) 2, "Element 1 moved?");

    void *element2 = parcArrayList_Get(array, 2);
    assertTrue(element2 == (void *) 3, "Element 1 moved?");

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Remove_AtIndex_First)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";

    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, a);
    parcArrayList_Add(array, b);
    parcArrayList_Add(array, c);

    PARCArrayList *expected = parcArrayList_Create(NULL);
    parcArrayList_Add(expected, b);
    parcArrayList_Add(expected, c);

    void *removedElement = parcArrayList_RemoveAtIndex(array, 0);

    assertTrue(removedElement == a, "Expected ");
    assertTrue(parcArrayList_Equals(expected, array), "Expected ");

    parcArrayList_Destroy(&expected);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_Remove_AtIndex)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";

    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, a);
    parcArrayList_Add(array, b);
    parcArrayList_Add(array, c);

    PARCArrayList *expected = parcArrayList_Create(NULL);
    parcArrayList_Add(expected, a);
    parcArrayList_Add(expected, c);

    void *removedElement = parcArrayList_RemoveAtIndex(array, 1);

    assertTrue(removedElement == b, "Expected ");
    assertTrue(parcArrayList_Equals(expected, array), "Expected ");

    parcArrayList_Destroy(&expected);
    parcArrayList_Destroy(&array);
}


LONGBOW_TEST_CASE(Global, PARC_ArrayList_Remove_AtIndex_Last)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";

    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, a);
    parcArrayList_Add(array, b);
    parcArrayList_Add(array, c);

    PARCArrayList *expected = parcArrayList_Create(NULL);
    parcArrayList_Add(expected, a);
    parcArrayList_Add(expected, b);

    void *removedElement = parcArrayList_RemoveAtIndex(array, 2);

    assertTrue(removedElement == c, "Expected ");
    assertTrue(parcArrayList_Equals(expected, array), "Expected ");

    parcArrayList_Destroy(&expected);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_RemoveAndDestroy_AtIndex_First)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";

    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, a);
    parcArrayList_Add(array, b);
    parcArrayList_Add(array, c);

    PARCArrayList *expected = parcArrayList_Create(NULL);
    parcArrayList_Add(expected, b);
    parcArrayList_Add(expected, c);

    parcArrayList_RemoveAndDestroyAtIndex(array, 0);

    assertTrue(parcArrayList_Equals(expected, array), "Expected ");

    parcArrayList_Destroy(&expected);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_RemoveAndDestroy_AtIndex)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";

    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, a);
    parcArrayList_Add(array, b);
    parcArrayList_Add(array, c);

    PARCArrayList *expected = parcArrayList_Create(NULL);
    parcArrayList_Add(expected, a);
    parcArrayList_Add(expected, c);

    parcArrayList_RemoveAndDestroyAtIndex(array, 1);

    assertTrue(parcArrayList_Equals(expected, array), "Expected ");

    parcArrayList_Destroy(&expected);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARC_ArrayList_RemoveAndDestroy_AtIndex_Last)
{
    char a[] = "apple";
    char b[] = "bananna";
    char c[] = "cherry";

    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, a);
    parcArrayList_Add(array, b);
    parcArrayList_Add(array, c);

    PARCArrayList *expected = parcArrayList_Create(NULL);
    parcArrayList_Add(expected, a);
    parcArrayList_Add(expected, b);

    parcArrayList_RemoveAndDestroyAtIndex(array, 2);

    assertTrue(parcArrayList_Equals(expected, array), "Expected ");

    parcArrayList_Destroy(&expected);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, PARC_ArrayList_EnsureRemaining_Empty);
    LONGBOW_RUN_TEST_CASE(Local, PARC_ArrayList_EnsureRemaining_NonEmpty);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, PARC_ArrayList_InsertAtIndex_OutOfCapacity);
}

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    PARCArrayList *array = parcArrayList_Create(NULL);

    longBowTestCase_SetClipBoardData(testCase, array);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    PARCArrayList *array = longBowTestCase_GetClipBoardData(testCase);
    parcArrayList_Destroy(&array);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Errors, PARC_ArrayList_InsertAtIndex_OutOfCapacity, .event = &LongBowAssertEvent)
{
    PARCArrayList *array = longBowTestCase_GetClipBoardData(testCase);
    printf("1\n");
    parcArrayList_Add(array, (void *) 1);
        printf("2\n");
    parcArrayList_Add(array, (void *) 2);
        printf("3\n");

    parcArrayList_InsertAtIndex(array, 200, (void *) 3);
        printf("4\n");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARC_ArrayList);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
