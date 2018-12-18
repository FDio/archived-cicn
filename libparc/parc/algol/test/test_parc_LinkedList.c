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
#include "../parc_LinkedList.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_StdlibMemory.h>

#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(PARCLinkedList)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(AcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(PARCLinkedList)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(PARCLinkedList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(AcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcLinkedList_CreateRelease);
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcLinkedList_AcquireRelease);
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
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(AcquireRelease, parcLinkedList_CreateRelease)
{
    PARCLinkedList *deque = parcLinkedList_Create();
    assertNotNull(deque, "Expected non-null result from parcLinkedList_Create()");

    assertTrue(parcLinkedList_IsValid(deque), "Expected created PARCLinkedList to be valid.");

    parcLinkedList_Release(&deque);
    assertNull(deque, "Expected parcLinkedList_Release to null the pointer");
}

LONGBOW_TEST_CASE(AcquireRelease, parcLinkedList_AcquireRelease)
{
    PARCLinkedList *original = parcLinkedList_Create();
    assertNotNull(original, "Expected non-null result from parcLinkedList_Create()");

    parcObjectTesting_AssertAcquireReleaseContract(parcLinkedList_Acquire, original);

    PARCLinkedList *reference = parcLinkedList_Acquire(original);
    assertTrue(original == reference, "Expected the reference to be equal to the original.");

    parcLinkedList_Release(&original);
    assertNull(original, "Expected parcLinkedList_Release to null the pointer");

    PARCBuffer *object = parcBuffer_Allocate(11);
    parcLinkedList_Append(reference, object);
    parcBuffer_Release(&object);

    size_t expected = 1;
    size_t actual = parcLinkedList_Size(reference);
    assertTrue(expected == actual,
               "Expected size %zd, actual %zd", expected, actual);
    parcLinkedList_Release(&reference);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Append_One);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Append_Two);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_AppendAll);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_AppendAll_None);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_CreateDestroy);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_GetFirst);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_GetLast);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_SetAtIndex);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Prepend_One);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Prepend_Two);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Prepend_Three);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_IsEmpty);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_GetAtIndex);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Contains_True);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Contains_False);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_RemoveFirst);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_RemoveFirst_SingleElement);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_RemoveLast);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Remove);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_RemoveNotFound);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_RemoveAtIndex);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Size);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_InsertAtIndex_Head);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_InsertAtIndex_HeadEmptyList);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_InsertAtIndex_Tail);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_InsertAtIndex_Middle);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_Display_NULL);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_CreateIterator);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_CreateIterator_Remove);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_CreateIterator_RemoveHead);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_CreateIterator_RemoveMiddle);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_CreateIterator_RemoveTail);

    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_SetEquals_True);
    LONGBOW_RUN_TEST_CASE(Global, parcLinkedList_SetEquals_False);
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

LONGBOW_TEST_CASE(Global, parcLinkedList_AssertValid)
{
    PARCLinkedList *list = parcLinkedList_Create();

    parcLinkedList_AssertValid(list);
    parcLinkedList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Append_One)
{
    PARCLinkedList *list = parcLinkedList_Create();

    PARCBuffer *object = parcBuffer_Allocate(11);
    PARCLinkedList *actual = parcLinkedList_Append(list, object);
    parcBuffer_Release(&object);

    assertTrue(parcLinkedList_IsValid(list), "PARCLinkedList is invalid.");

    assertTrue(list == actual, "Expected parcLinkedList_Append to return its argument.");
    assertTrue(parcLinkedList_Size(list) == 1, "Expected size of 1, actual %zd", parcLinkedList_Size(list));

    parcLinkedList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Append_Two)
{
    PARCLinkedList *deque = parcLinkedList_Create();

    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");

    parcLinkedList_Append(deque, object1);
    PARCLinkedList *actual = parcLinkedList_Append(deque, object2);
    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);

    assertTrue(deque == actual, "Expected parcLinkedList_Append to return its argument.");
    assertTrue(parcLinkedList_Size(deque) == 2, "Expected size of 2, actual %zd", parcLinkedList_Size(deque));

    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_AppendAll)
{
    PARCLinkedList *other = parcLinkedList_Create();

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), i);
        parcLinkedList_Append(other, buffer);
        parcBuffer_Release(&buffer);
    }

    PARCLinkedList *list = parcLinkedList_Create();

    parcLinkedList_AppendAll(list, other);

    assertTrue(parcLinkedList_Equals(list, other), "Expected equal lists.");

    parcLinkedList_Release(&list);
    parcLinkedList_Release(&other);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_AppendAll_None)
{
    PARCLinkedList *other = parcLinkedList_Create();

    PARCLinkedList *list = parcLinkedList_Create();

    parcLinkedList_AppendAll(list, other);

    assertTrue(parcLinkedList_Equals(list, other), "Expected equal lists.");

    parcLinkedList_Release(&list);
    parcLinkedList_Release(&other);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_CreateDestroy)
{
    PARCLinkedList *deque = parcLinkedList_Create();
    assertNotNull(deque, "Expected non-null result from parcLinkedList_Create()");

    parcLinkedList_Release(&deque);
    assertNull(deque, "Expected parcLinkedList_Destroy to null the pointer");
}

LONGBOW_TEST_CASE(Global, parcLinkedList_HashCode)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Append(deque, object1);
    parcLinkedList_Append(deque, object2);
    parcLinkedList_Append(deque, object3);

    parcLinkedList_HashCode(deque);

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_GetFirst)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Append(deque, object1);
    parcLinkedList_Append(deque, object2);
    parcLinkedList_Append(deque, object3);

    PARCBuffer *actual = parcLinkedList_GetFirst(deque);
    assertTrue(parcBuffer_Equals(object1, actual), "Order of objects in the list is wrong.");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_GetLast)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Append(deque, object1);
    parcLinkedList_Append(deque, object2);
    parcLinkedList_Append(deque, object3);

    PARCBuffer *actual = parcLinkedList_GetLast(deque);
    assertTrue(parcBuffer_Equals(object3, actual), "Order of objects in the list is wrong.");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Prepend_One)
{
    PARCLinkedList *deque = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCLinkedList *actual = parcLinkedList_Prepend(deque, object1);
    parcBuffer_Release(&object1);

    assertTrue(deque == actual, "Expected parcLinkedList_Append to return its argument.");
    assertTrue(parcLinkedList_Size(deque) == 1, "Expected size of 1, actual %zd", parcLinkedList_Size(deque));
    assertTrue(deque->head != NULL, "Expected head to be not null.");
    assertTrue(deque->head == deque->tail, "Expected head to be equal to the tail.");

    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Prepend_Two)
{
    PARCLinkedList *deque = parcLinkedList_Create();

    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCLinkedList *actual = parcLinkedList_Prepend(deque, object1);
    parcLinkedList_Prepend(deque, object1);
    parcBuffer_Release(&object1);

    assertTrue(deque == actual, "Expected parcLinkedList_Prepend to return its argument.");
    assertTrue(parcLinkedList_Size(deque) == 2, "Expected size of 2, actual %zd", parcLinkedList_Size(deque));

    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Prepend_Three)
{
    PARCLinkedList *deque = parcLinkedList_Create();

    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    parcLinkedList_Prepend(deque, object1);
    parcLinkedList_Prepend(deque, object2);
    PARCLinkedList *actual = parcLinkedList_Prepend(deque, object3);

    assertTrue(deque == actual, "Expected parcLinkedList_Prepend to return its argument.");
    assertTrue(parcLinkedList_Size(deque) == 3, "Expected size of 3, actual %zd", parcLinkedList_Size(deque));

    PARCBuffer *peek = parcLinkedList_GetFirst(deque);
    assertTrue(parcBuffer_Equals(object3, peek), "Order of objects failed");

    peek = parcLinkedList_GetLast(deque);
    assertTrue(parcBuffer_Equals(object1, peek), "Order of objects failed");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);

    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_RemoveFirst)
{
    PARCLinkedList *list = parcLinkedList_Create();

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), i));
        parcLinkedList_Append(list, buffer);
        parcBuffer_Release(&buffer);
    }

    PARCBuffer *peek = parcLinkedList_RemoveFirst(list);
    assertTrue(parcObject_GetReferenceCount(peek) == 1, "Expected reference count to be 1.");
    assertTrue(parcBuffer_GetUint32(peek) == 0, "Objects out of order.");

    parcBuffer_Release(&peek);
    parcLinkedList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_RemoveFirst_SingleElement)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Prepend(deque, object1);

    PARCBuffer *peek = parcLinkedList_RemoveFirst(deque);
    assertTrue(parcBuffer_Equals(object1, peek),
               "Objects out of order.");

    parcBuffer_Release(&peek);
    parcBuffer_Release(&object1);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_RemoveLast)
{
    PARCLinkedList *list = parcLinkedList_Create();

    for (int i = 0; i < 1000; i++) {
        PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_PutUint32(parcBuffer_Allocate(sizeof(int)), i));
        parcLinkedList_Append(list, buffer);
        parcBuffer_Release(&buffer);
    }

    PARCBuffer *peek = parcLinkedList_RemoveLast(list);
    assertTrue(parcObject_GetReferenceCount(peek) == 1, "Expected reference count to be 1.");
    assertTrue(parcBuffer_GetUint32(peek) == 999, "Objects out of order.");

    parcBuffer_Release(&peek);
    parcLinkedList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_RemoveLast_SingleElement)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");

    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Prepend(deque, object1);

    PARCBuffer *peek = parcLinkedList_RemoveLast(deque);
    assertTrue(parcBuffer_Equals(object1, peek),
               "Objects out of order.");

    parcBuffer_Release(&object1);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Remove)
{
    PARCLinkedList *deque = parcLinkedList_Create();

    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    parcLinkedList_Prepend(deque, object3);
    parcLinkedList_Prepend(deque, object2);
    parcLinkedList_Prepend(deque, object1);

    bool found = parcLinkedList_Remove(deque, object2);
    assertTrue(found, "Expected item to be found");
    assertTrue(parcLinkedList_Size(deque) == 2, "Expected size of 2, actual %zd", parcLinkedList_Size(deque));

    PARCBuffer *peek;
    peek = parcLinkedList_RemoveFirst(deque);
    assertTrue(parcBuffer_Equals(object1, peek), "Object1 was not first in list");
    parcBuffer_Release(&peek);

    peek = parcLinkedList_RemoveFirst(deque);
    assertTrue(parcBuffer_Equals(object3, peek), "Object3 was not second in list");
    parcBuffer_Release(&peek);

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_RemoveAtIndex)
{
    PARCLinkedList *list = parcLinkedList_Create();

    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    parcLinkedList_Prepend(list, object3);
    parcLinkedList_Prepend(list, object2);
    parcLinkedList_Prepend(list, object1);

    PARCBuffer *actual = parcLinkedList_RemoveAtIndex(list, 1);

    assertTrue(parcBuffer_Equals(object2, actual), "Wrong object returned from parcLinkedList_RemoveAtIndex");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcBuffer_Release(&actual);
    parcLinkedList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_RemoveNotFound)
{
    PARCLinkedList *deque = parcLinkedList_Create();

    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    PARCBuffer *object4 = parcBuffer_WrapCString("4");

    parcLinkedList_Prepend(deque, object3);
    parcLinkedList_Prepend(deque, object2);
    parcLinkedList_Prepend(deque, object1);

    bool found = parcLinkedList_Remove(deque, object4);
    assertFalse(found, "Expected item to be not found");
    assertTrue(parcLinkedList_Size(deque) == 3, "Expected size of 3, actual %zd", parcLinkedList_Size(deque));

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcBuffer_Release(&object4);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Size)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Prepend(deque, object1);
    parcLinkedList_Prepend(deque, object2);
    parcLinkedList_Prepend(deque, object3);

    assertTrue(parcLinkedList_Size(deque) == 3,
               "Expected 3, actual %zd", parcLinkedList_Size(deque));

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);

    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_IsEmpty)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");

    PARCLinkedList *deque = parcLinkedList_Create();

    assertTrue(parcLinkedList_IsEmpty(deque), "Expected true.");
    parcLinkedList_Prepend(deque, object1);
    assertFalse(parcLinkedList_IsEmpty(deque), "Expected false.");

    parcBuffer_Release(&object1);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_GetAtIndex)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Append(deque, object1);
    parcLinkedList_Append(deque, object2);
    parcLinkedList_Append(deque, object3);

    PARCBuffer *actual;
    actual = parcLinkedList_GetAtIndex(deque, 0);
    assertTrue(parcBuffer_Equals(actual, object1), "parcLinkedList_GetAtIndex failed");
    actual = parcLinkedList_GetAtIndex(deque, 1);
    assertTrue(parcBuffer_Equals(actual, object2), "parcLinkedList_GetAtIndex failed");
    actual = parcLinkedList_GetAtIndex(deque, 2);
    assertTrue(parcBuffer_Equals(actual, object3), "parcLinkedList_GetAtIndex failed");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_SetAtIndex)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *deque = parcLinkedList_Create();
    parcLinkedList_Append(deque, object1);
    parcLinkedList_Append(deque, object2);
    parcLinkedList_Append(deque, object3);

    PARCBuffer *newObject = parcBuffer_WrapCString("Hello");

    PARCBuffer *actual = parcLinkedList_SetAtIndex(deque, 0, newObject);
    assertTrue(parcBuffer_Equals(actual, object1), "parcLinkedList_SetAtIndex failed to return the old value.");
    parcBuffer_Release(&actual);

    actual = parcLinkedList_GetAtIndex(deque, 0);
    assertTrue(parcBuffer_Equals(actual, newObject), "parcLinkedList_SetAtIndex failed to set the new value.");

    parcBuffer_Release(&newObject);
    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&deque);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Contains_True)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *list = parcLinkedList_Create();
    parcLinkedList_Append(list, object1);
    parcLinkedList_Append(list, object2);
    parcLinkedList_Append(list, object3);

    bool actual = parcLinkedList_Contains(list, object2);
    assertTrue(actual, "Expected parcLinkedList_Contains to return true for object in the list");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Contains_False)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *list = parcLinkedList_Create();
    parcLinkedList_Append(list, object1);
    parcLinkedList_Append(list, object3);

    bool actual = parcLinkedList_Contains(list, object2);
    assertFalse(actual, "Expected parcLinkedList_Contains to return false for object not in the list");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&list);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Equals)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *x = parcLinkedList_Create();
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);
    PARCLinkedList *y = parcLinkedList_Create();
    parcLinkedList_Append(y, object1);
    parcLinkedList_Append(y, object2);
    PARCLinkedList *z = parcLinkedList_Create();
    parcLinkedList_Append(z, object1);
    parcLinkedList_Append(z, object2);
    PARCLinkedList *u1 = parcLinkedList_Create();
    parcLinkedList_Append(u1, object2);
    PARCLinkedList *u2 = parcLinkedList_Create();
    parcLinkedList_Append(u2, object2);
    parcLinkedList_Append(u2, object3);

    parcObjectTesting_AssertEqualsFunction(parcLinkedList_Equals, x, y, z, u1, u2, NULL);

    parcLinkedList_Release(&x);
    parcLinkedList_Release(&y);
    parcLinkedList_Release(&z);
    parcLinkedList_Release(&u1);
    parcLinkedList_Release(&u2);

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Copy)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);
    parcLinkedList_Append(x, object3);

    PARCLinkedList *y = parcLinkedList_Copy(x);

    assertTrue(parcLinkedList_Equals(x, y), "Expected the copy to be equal to the original.");

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);

    parcLinkedList_Release(&y);
    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_InsertAtIndex_Head)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    PARCBuffer *object4 = parcBuffer_WrapCString("4");
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);
    parcLinkedList_Append(x, object3);

    parcLinkedList_InsertAtIndex(x, 0, object4);

    PARCBuffer *actual = parcLinkedList_GetAtIndex(x, 0);

    assertTrue(actual == object4, "Unexpected object at index 0");

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcBuffer_Release(&object4);

    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_InsertAtIndex_HeadEmptyList)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    PARCBuffer *object4 = parcBuffer_WrapCString("4");

    parcLinkedList_InsertAtIndex(x, 0, object4);
    assertTrue(x->head->object == object4, "Malformed linked list node does not contain the proper object reference");
    assertTrue(x->head == x->tail, "Expected the list head and tail to be the same for a single element list.");
    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");

    PARCBuffer *actual = parcLinkedList_GetAtIndex(x, 0);

    assertTrue(actual == object4, "Unexpected object at index 0");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcBuffer_Release(&object4);

    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_InsertAtIndex_Tail)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    PARCBuffer *object4 = parcBuffer_WrapCString("4");
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);
    parcLinkedList_Append(x, object3);

    parcLinkedList_InsertAtIndex(x, 3, object4);
    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");

    PARCBuffer *actual = parcLinkedList_GetAtIndex(x, 3);

    assertTrue(actual == object4, "Unexpected object at index 3");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcBuffer_Release(&object4);

    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_InsertAtIndex_Middle)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    PARCBuffer *object4 = parcBuffer_WrapCString("4");
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);
    parcLinkedList_Append(x, object3);

    parcLinkedList_InsertAtIndex(x, 1, object4);
    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");

    assertTrue(parcLinkedList_GetAtIndex(x, 0) == object1, "Unexpected object at index 1");
    assertTrue(parcLinkedList_GetAtIndex(x, 1) == object4, "Unexpected object at index 1");
    assertTrue(parcLinkedList_GetAtIndex(x, 2) == object2, "Unexpected object at index 1");
    assertTrue(parcLinkedList_GetAtIndex(x, 3) == object3, "Unexpected object at index 1");

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcBuffer_Release(&object4);

    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Display)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);
    parcLinkedList_Append(x, object3);

    parcLinkedList_Display(x, 0);

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_Display_NULL)
{
    parcLinkedList_Display(NULL, 0);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_CreateIterator)
{
    PARCLinkedList *x = parcLinkedList_Create();

    uint32_t expectedCount = 10;
    for (uint32_t i = 0; i < expectedCount; i++) {
        PARCBuffer *object = parcBuffer_Allocate(sizeof(int));
        parcBuffer_PutUint32(object, i);
        parcBuffer_Flip(object);
        parcLinkedList_Append(x, object);
        parcBuffer_Release(&object);
    }

    PARCIterator *iterator = parcLinkedList_CreateIterator(x);
    uint32_t expected = 0;
    while (parcIterator_HasNext(iterator)) {
        PARCBuffer *buffer = (PARCBuffer *) parcIterator_Next(iterator);
        uint32_t actual = parcBuffer_GetUint32(buffer);
        assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
        expected++;
    }
    parcIterator_Release(&iterator);

    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_CreateIterator_Remove)
{
    PARCLinkedList *x = parcLinkedList_Create();
    for (size_t i = 0; i < 5; i++) {
        PARCBuffer *buffer = parcBuffer_Allocate(10);
        parcBuffer_PutUint64(buffer, i);
        parcBuffer_Flip(buffer);
        parcLinkedList_Append(x, buffer);
        parcBuffer_Release(&buffer);
    }

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");

    PARCIterator *iterator = parcLinkedList_CreateIterator(x);
    size_t expected = 0;
    while (parcIterator_HasNext(iterator)) {
        size_t actual = parcBuffer_GetUint64(parcIterator_Next(iterator));
        assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
        parcIterator_Remove(iterator);
        expected++;
    }
    parcIterator_Release(&iterator);

    iterator = parcLinkedList_CreateIterator(x);
    assertFalse(parcIterator_HasNext(iterator), "Expected an interator on an empty list to not HaveNext");
    parcIterator_Release(&iterator);

    assertTrue(parcLinkedList_Size(x) == 0, "List is not empty.");

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");
    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_CreateIterator_RemoveHead)
{
    size_t listSize = 5;

    PARCLinkedList *x = parcLinkedList_Create();
    for (size_t i = 0; i < listSize; i++) {
        PARCBuffer *buffer = parcBuffer_Allocate(10);
        parcBuffer_PutUint64(buffer, i);
        parcBuffer_Flip(buffer);
        parcLinkedList_Append(x, buffer);
        parcBuffer_Release(&buffer);
    }

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");
    parcLinkedList_Display(x, 0);

    PARCIterator *iterator = parcLinkedList_CreateIterator(x);
    if (parcIterator_HasNext(iterator)) {
        PARCBuffer *buffer = (PARCBuffer *) parcIterator_Next(iterator);
        size_t actual = parcBuffer_GetUint64(buffer);
        assertTrue(actual == 0, "Expected %d, actual %zd", 0, actual);
        parcIterator_Remove(iterator);
    }
    parcIterator_Release(&iterator);

    iterator = parcLinkedList_CreateIterator(x);
    assertTrue(parcIterator_HasNext(iterator), "Expected an interator on a non-empty list to HaveNext");
    parcIterator_Release(&iterator);

    assertTrue(parcLinkedList_Size(x) == listSize - 1, "Expected the list to be %zd, actual %zd", listSize - 1, parcLinkedList_Size(x));

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");
    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_CreateIterator_RemoveMiddle)
{
    size_t listSize = 5;

    PARCLinkedList *x = parcLinkedList_Create();
    for (size_t i = 0; i < listSize; i++) {
        PARCBuffer *buffer = parcBuffer_Allocate(10);
        parcBuffer_PutUint64(buffer, i);
        parcBuffer_Flip(buffer);
        parcLinkedList_Append(x, buffer);
        parcBuffer_Release(&buffer);
    }

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");
    assertTrue(parcLinkedList_Size(x) == listSize, "Expected the list to be %zd, actual %zd", listSize, parcLinkedList_Size(x));


    PARCIterator *iterator = parcLinkedList_CreateIterator(x);
    for (size_t i = 0; i <= listSize / 2; i++) {
        if (parcIterator_HasNext(iterator)) {
            parcIterator_Next(iterator);
        }
    }
    parcIterator_Remove(iterator);

    parcIterator_Release(&iterator);


    iterator = parcLinkedList_CreateIterator(x);
    size_t expected = 0;
    while (parcIterator_HasNext(iterator)) {
        if (expected != (listSize / 2)) {
            size_t actual = parcBuffer_GetUint64(parcIterator_Next(iterator));
            assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
        }
        expected++;
    }
    parcIterator_Release(&iterator);

    assertTrue(parcLinkedList_Size(x) == listSize - 1, "Expected the list to be %zd, actual %zd", listSize - 1, parcLinkedList_Size(x));

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");
    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_CreateIterator_RemoveTail)
{
    size_t listSize = 5;

    PARCLinkedList *x = parcLinkedList_Create();
    for (size_t i = 0; i < listSize; i++) {
        PARCBuffer *buffer = parcBuffer_Allocate(10);
        parcBuffer_PutUint64(buffer, i);
        parcBuffer_Flip(buffer);
        parcLinkedList_Append(x, buffer);
        parcBuffer_Release(&buffer);
    }

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");
    assertTrue(parcLinkedList_Size(x) == listSize, "Expected the list to be %zd, actual %zd", listSize, parcLinkedList_Size(x));

    PARCIterator *iterator = parcLinkedList_CreateIterator(x);
    for (size_t i = 0; i < listSize; i++) {
        if (parcIterator_HasNext(iterator)) {
            parcIterator_Next(iterator);
        }
    }
    parcIterator_Remove(iterator);

    parcIterator_Release(&iterator);

    assertTrue(parcLinkedList_Size(x) == listSize - 1, "Expected the list to be %zd, actual %zd", listSize - 1, parcLinkedList_Size(x));


    iterator = parcLinkedList_CreateIterator(x);
    size_t expected = 0;
    while (parcIterator_HasNext(iterator)) {
        size_t actual = parcBuffer_GetUint64(parcIterator_Next(iterator));
        assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
        expected++;
    }
    parcIterator_Release(&iterator);

    assertTrue(parcLinkedList_IsValid(x), "PARCLinkedList is invalid.");
    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_SetEquals_True)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *x = parcLinkedList_Create();
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);
    PARCLinkedList *y = parcLinkedList_Create();
    parcLinkedList_Append(y, object2);
    parcLinkedList_Append(y, object1);

    PARCLinkedList *u1 = parcLinkedList_Create();
    parcLinkedList_Append(u1, object2);

    PARCLinkedList *u2 = parcLinkedList_Create();
    parcLinkedList_Append(u2, object2);
    parcLinkedList_Append(u2, object3);

    assertTrue(parcLinkedList_SetEquals(x, y), "Expected to lists with the same elements to be equal regarless of order.");

    parcLinkedList_Release(&x);
    parcLinkedList_Release(&y);
    parcLinkedList_Release(&u1);
    parcLinkedList_Release(&u2);

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
}

LONGBOW_TEST_CASE(Global, parcLinkedList_SetEquals_False)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCLinkedList *x = parcLinkedList_Create();
    parcLinkedList_Append(x, object1);
    parcLinkedList_Append(x, object2);

    PARCLinkedList *u1 = parcLinkedList_Create();
    parcLinkedList_Append(u1, object2);

    PARCLinkedList *u2 = parcLinkedList_Create();
    parcLinkedList_Append(u2, object2);
    parcLinkedList_Append(u2, object3);

    assertFalse(parcLinkedList_SetEquals(x, u1), "Expected to lists without the same elements to be equal regarless of order.");

    parcLinkedList_Release(&x);
    parcLinkedList_Release(&u1);
    parcLinkedList_Release(&u2);

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _parcLinkedListNode_Create);
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

LONGBOW_TEST_CASE(Local, _parcLinkedListNode_Create)
{
    PARCBuffer *object = parcBuffer_Allocate(10);
    struct parc_linkedlist_node *previous = NULL;
    struct parc_linkedlist_node *next = NULL;

    struct parc_linkedlist_node *actual = _parcLinkedListNode_Create(object, previous, next);
    parcBuffer_Release(&object);
    _parcLinkedListNode_Destroy(NULL, &actual);
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcLinkedList_Append);
    LONGBOW_RUN_TEST_CASE(Performance, parcLinkedList_N2);
    LONGBOW_RUN_TEST_CASE(Performance, parcLinkedList_CreateIterator);
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

LONGBOW_TEST_CASE(Performance, parcLinkedList_Append)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");

    for (size_t i = 0; i < 100000; i++) {
        parcLinkedList_Append(x, object1);
    }

    parcBuffer_Release(&object1);
    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Performance, parcLinkedList_N2)
{
    PARCLinkedList *x = parcLinkedList_Create();
    PARCBuffer *object1 = parcBuffer_WrapCString("1");

    for (size_t i = 0; i < 100000; i++) {
        parcLinkedList_Append(x, object1);
    }

    for (size_t expected = 0; expected < parcLinkedList_Size(x); expected++) {
        PARCBuffer *actual = (PARCBuffer *) parcLinkedList_GetAtIndex(x, expected);
        assertTrue(parcBuffer_Equals(object1, actual), "Mismatched value in the list.");
    }

    parcBuffer_Release(&object1);

    parcLinkedList_Release(&x);
}

LONGBOW_TEST_CASE(Performance, parcLinkedList_CreateIterator)
{
    PARCLinkedList *x = parcLinkedList_Create();

    uint32_t expectedCount = 100000;
    for (uint32_t i = 0; i < expectedCount; i++) {
        PARCBuffer *object = parcBuffer_Allocate(sizeof(int));
        parcBuffer_PutUint32(object, i);
        parcBuffer_Flip(object);
        parcLinkedList_Append(x, object);
        parcBuffer_Release(&object);
    }

    PARCIterator *iterator = parcLinkedList_CreateIterator(x);
    uint32_t expected = 0;
    while (parcIterator_HasNext(iterator)) {
        PARCBuffer *buffer = (PARCBuffer *) parcIterator_Next(iterator);
        uint32_t actual = parcBuffer_GetUint32(buffer);
        assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
        expected++;
    }
    parcIterator_Release(&iterator);

    parcLinkedList_Release(&x);
}


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARCLinkedList);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
