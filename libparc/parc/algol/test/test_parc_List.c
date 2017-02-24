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

#include "../parc_List.c"
#include <LongBow/unit-test.h>

#include <stdio.h>
#include <string.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(PARCList)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(PARCList);
    LONGBOW_RUN_TEST_FIXTURE(Local);
//    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

LONGBOW_TEST_RUNNER_SETUP(PARCList)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(PARCList)
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
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Add);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_AddAll);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcList_Release);

    LONGBOW_RUN_TEST_CASE(Global, PARCList_Equals_Contract);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Equals_Contract_Deep);

    LONGBOW_RUN_TEST_CASE(Global, PARCList_FromInitialCapacity);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Get);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_New);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Length);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Remove_AtIndex_First);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Remove_AtIndex);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_Remove_AtIndex_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_RemoveAndDestroy_AtIndex_First);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_RemoveAndDestroy_AtIndex);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_RemoveAndDestroy_AtIndex_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_InsertAtIndex);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_InsertAtIndex_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_InsertAtIndex_First);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_InsertAtIndex_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARCList_IsEmpty);
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

LONGBOW_TEST_CASE(Global, PARCList_Add)
{
    PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);

    parcList_Add(list, 0);
    size_t actual = parcList_Size(list);
    assertTrue(1 == actual, "Expected=%d, actual=%zu", 1, actual);

    parcList_Release(&list);
}

LONGBOW_TEST_CASE(Global, PARCList_AddAll)
{
    PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);

    void *elements[] = {
        strdup("a"),
        strdup("b"),
        strdup("c"),
    };

    parcList_AddAll(list, 3, elements);
    size_t actual = parcList_Size(list);

    assertTrue(3 == actual, "Expected=%d, actual=%zu", 3, actual);

    parcList_Release(&list);
}

LONGBOW_TEST_CASE(Global, PARCList_Copy)
{
    char *a = strdup("apple");
    char *b = strdup("bananna");
    char *c = strdup("cherry");

    PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);

    parcList_Add(list, a);
    parcList_Add(list, b);
    parcList_Add(list, c);

    parcList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcList_Release)
{
    PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);

    parcList_Release(&list);
    assertNull(list, "Expected null.");
}

LONGBOW_TEST_CASE(Global, PARCList_Equals_Empty)
{
    PARCArrayList *a = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    PARCArrayList *b = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    assertTrue(parcArrayList_Equals(a, b), "Equal values were expected to be equal");

    parcArrayList_Destroy(&a);
    parcArrayList_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, PARCList_Equals_Same)
{
    PARCArrayList *a = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    assertTrue(parcArrayList_Equals(a, a), "Expected the same array list to be equal to itself.");

    parcArrayList_Destroy(&a);
}

LONGBOW_TEST_CASE(Global, PARCList_Equals_Contract)
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

LONGBOW_TEST_CASE(Global, PARCList_Equals_Contract_Deep)
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

LONGBOW_TEST_CASE(Global, PARCList_FromInitialCapacity)
{
    PARCArrayList *array = parcArrayList_Create_Capacity(NULL, parcArrayList_StdlibFreeFunction, 10);
    size_t actual = parcArrayList_Size(array);

    assertTrue(0 == actual, "Expected=%d, actual=%zu", 0, actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARCList_Get)
{
    PARCArrayList *array = parcArrayList_Create(parcArrayList_StdlibFreeFunction);

    char *expected = strdup("Hello World");
    parcArrayList_Add(array, expected);

    char *actual = parcArrayList_Get(array, 0);

    assertTrue(expected == actual, "Expected=%p, actual=%p", (void *) expected, (void *) actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARCList_New)
{
    PARCArrayList *array = parcArrayList_Create(parcArrayList_StdlibFreeFunction);
    size_t size = parcArrayList_Size(array);
    assertTrue(0 == size, "Expected %d actual=%zd", 0, size);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARCList_Length)
{
    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, 0);

    size_t size = parcArrayList_Size(array);
    assertTrue(1 == size, "Expected %d actual=%zd", 1, size);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARCList_IsEmpty)
{
    PARCArrayList *array = parcArrayList_Create(NULL);
    assertTrue(parcArrayList_IsEmpty(array), "Expected a new array to be empty.");

    parcArrayList_Add(array, 0);
    assertFalse(parcArrayList_IsEmpty(array), "Expected an array with more than zero elements to be empty.");

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARCList_InsertAtIndex)
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

LONGBOW_TEST_CASE(Global, PARCList_InsertAtIndex_Empty)
{
    PARCArrayList *array = parcArrayList_Create(NULL);

    parcArrayList_InsertAtIndex(array, 0, (void *) 3);

    size_t actual = parcArrayList_Size(array);

    assertTrue(1 == actual, "Expected=%d, actual=%zu", 1, actual);

    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(Global, PARCList_InsertAtIndex_First)
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

LONGBOW_TEST_CASE(Global, PARCList_InsertAtIndex_Last)
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

LONGBOW_TEST_CASE(Global, PARCList_Remove_AtIndex_First)
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

LONGBOW_TEST_CASE(Global, PARCList_Remove_AtIndex)
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


LONGBOW_TEST_CASE(Global, PARCList_Remove_AtIndex_Last)
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

LONGBOW_TEST_CASE(Global, PARCList_RemoveAndDestroy_AtIndex_First)
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

LONGBOW_TEST_CASE(Global, PARCList_RemoveAndDestroy_AtIndex)
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

LONGBOW_TEST_CASE(Global, PARCList_RemoveAndDestroy_AtIndex_Last)
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
    LONGBOW_RUN_TEST_CASE(Errors, PARCList_InsertAtIndex_OutOfCapacity);
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

LONGBOW_TEST_CASE_EXPECTS(Errors, PARCList_InsertAtIndex_OutOfCapacity, .event = &LongBowAssertEvent)
{
    PARCArrayList *array = longBowTestCase_GetClipBoardData(testCase);

    parcArrayList_Add(array, (void *) 1);
    parcArrayList_Add(array, (void *) 2);

    parcArrayList_InsertAtIndex(array, 200, (void *) 3);
}


LONGBOW_TEST_FIXTURE(PARCList)
{
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Add);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_AddCollection);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_AddCollectionAtIndex);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Contains);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_ContainsCollection);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Equals);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_IsEmpty);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_GetAtIndex);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Remove);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_RemoveCollection);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_RetainCollection);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_HashCode);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_IndexOf);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_LastIndexOf);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Copy);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Clear);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Destroy);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_RemoveAtIndex);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_SetAtIndex);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_Size);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_SubList);
    LONGBOW_RUN_TEST_CASE(PARCList, parcList_ToArray);
}

LONGBOW_TEST_FIXTURE_SETUP(PARCList)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());
    longBowTestCase_Set(testCase, "linkedList", parcLinkedList_Create());
    longBowTestCase_Set(testCase, "list", parcLinkedList_AsPARCList(longBowTestCase_Get(testCase, "linkedList")));
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(PARCList)
{
    PARCLinkedList *linkedList = longBowTestCase_Get(testCase, "linkedList");
    parcLinkedList_Release(&linkedList);
    PARCList *list = longBowTestCase_Get(testCase, "list");
    parcList_Release(&list);

    int initialAllocations = longBowTestCase_GetInt(testCase, "initalAllocations");
    if (!parcMemoryTesting_ExpectedOutstanding(initialAllocations, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

//#include "test_parc_List_modular.c"

LONGBOW_TEST_CASE(PARCList, parcList_Add)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");

    PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), 1));
    parcList_Add(list, buffer);
    parcBuffer_Release(&buffer);

    size_t actual = parcList_Size(list);
    assertTrue(1 == actual, "Expected=%d, actual=%zu", 1, actual);
}

LONGBOW_TEST_CASE(PARCList, parcList_AddCollection)
{
}

LONGBOW_TEST_CASE(PARCList, parcList_AddCollectionAtIndex)
{
}

LONGBOW_TEST_CASE(PARCList, parcList_Contains)
{
}

LONGBOW_TEST_CASE(PARCList, parcList_ContainsCollection)
{
}

LONGBOW_TEST_CASE(PARCList, parcList_Equals)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");
    PARCList *copy = parcList_Copy(list);

    assertTrue(parcList_Equals(list, copy), "Expected copy to be equal to the original.");

    parcList_Release(&copy);
}

LONGBOW_TEST_CASE(PARCList, parcList_IsEmpty)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");
    assertTrue(parcList_IsEmpty(list), "Expected list to be empty.");
}

LONGBOW_TEST_CASE(PARCList, parcList_GetAtIndex)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), i));
        parcList_Add(list, buffer);
        parcBuffer_Release(&buffer);
    }

    uint32_t actual = parcBuffer_GetUint32(parcList_GetAtIndex(list, 0));

    assertTrue(actual == 0, "Expected %u, actual %u\n", 0, actual);
}

LONGBOW_TEST_CASE(PARCList, parcList_Remove)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");

    PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), 1));
    parcList_Add(list, buffer);
    parcBuffer_Release(&buffer);

    buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), 1));

    bool actual = parcList_Remove(list, buffer);
    assertTrue(actual, "Expected element to have been found and removed.");

    parcBuffer_Release(&buffer);

    buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), 3));

    actual = parcList_Remove(list, buffer);
    assertFalse(actual, "Expected element to have not been found and removed.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(PARCList, parcList_RemoveCollection)
{
}

LONGBOW_TEST_CASE(PARCList, parcList_RetainCollection)
{
}

LONGBOW_TEST_CASE(PARCList, parcList_HashCode)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");
    parcList_HashCode(list);
}

LONGBOW_TEST_CASE(PARCList, parcList_IndexOf)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), i));
        parcList_Add(list, buffer);
        parcBuffer_Release(&buffer);
    }

    uint32_t expected = 10;
    PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), 10));
    size_t actual = parcList_IndexOf(list, buffer);

    parcBuffer_Release(&buffer);

    assertTrue(expected == actual, "Expected %u, actual %zu", expected, actual);
}

LONGBOW_TEST_CASE(PARCList, parcList_LastIndexOf)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), 1));
        parcList_Add(list, buffer);
        parcBuffer_Release(&buffer);
    }

    uint32_t expected = 999;
    PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), 1));
    size_t actual = parcList_LastIndexOf(list, buffer);

    parcBuffer_Release(&buffer);

    assertTrue(expected == actual, "Expected %u, actual %zu", expected, actual);
}

LONGBOW_TEST_CASE(PARCList, parcList_Copy)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");
    PARCList *copy = parcList_Copy(list);

    assertTrue(parcList_Equals(list, copy), "Expected copy to be equal to the original.");

    parcList_Release(&copy);
}

LONGBOW_TEST_CASE(PARCList, parcList_Clear)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");
    parcList_Clear(list);

    assertTrue(parcList_IsEmpty(list), "Expected list to be empty.");
}

LONGBOW_TEST_CASE(PARCList, parcList_Destroy)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");
    PARCList *copy = parcList_Copy(list);

    parcList_Release(&copy);
}

LONGBOW_TEST_CASE(PARCList, parcList_RemoveAtIndex)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), i));
        parcList_Add(list, buffer);
        parcBuffer_Release(&buffer);
    }

    PARCBuffer *buffer = parcList_RemoveAtIndex(list, 0);
    uint32_t actual = parcBuffer_GetUint32(buffer);
    assertTrue(actual == 0, "Expected buffer 0.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(PARCList, parcList_SetAtIndex)
{
    PARCList *list = longBowTestCase_Get(testCase, "list");

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), i));
        parcList_Add(list, buffer);
        parcBuffer_Release(&buffer);
    }

    PARCBuffer *buffer = parcBuffer_WrapCString("1");

    PARCBuffer *oldValue = parcList_SetAtIndex(list, 50, buffer);

    PARCBuffer *actual = parcList_GetAtIndex(list, 50);
    assertTrue(parcBuffer_Equals(buffer, actual), "parcList_SetAtIndex set the wrong location.");
    parcBuffer_Release(&buffer);
    parcBuffer_Release(&oldValue);
}

LONGBOW_TEST_CASE(PARCList, parcList_Size)
{
    PARCArrayList *array = parcArrayList_Create(NULL);
    parcArrayList_Add(array, 0);

    size_t size = parcArrayList_Size(array);
    assertTrue(1 == size, "Expected %d actual=%zd", 1, size);
    parcArrayList_Destroy(&array);
}

LONGBOW_TEST_CASE(PARCList, parcList_SubList)
{
}

LONGBOW_TEST_CASE(PARCList, parcList_ToArray)
{
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARCList);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
