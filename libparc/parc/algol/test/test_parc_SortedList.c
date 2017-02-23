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
#include "../parc_SortedList.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_SortedList)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_SortedList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_SortedList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateCompare);
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
    PARCSortedList *instance = parcSortedList_Create();
    assertNotNull(instance, "Expected non-null result from parcSortedList_Create();");

    parcObjectTesting_AssertAcquire(instance);

    parcSortedList_Release(&instance);
    assertNull(instance, "Expected null result from parcSortedList_Release();");
}

// A signum function to compare two PARCBuffers by length of buffer.
static int
_compareTwoBuffersByLength(const PARCObject *buf1, const PARCObject *buf2)
{
    size_t size1 = parcBuffer_Limit((PARCBuffer *) buf1);
    size_t size2 = parcBuffer_Limit((PARCBuffer *) buf2);

    if (size1 > size2) {
        return 1;
    } else if (size2 > size1) {
        return -1;
    }
    return 0;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateCompare)
{
    PARCSortedList *instance = parcSortedList_CreateCompare(_compareTwoBuffersByLength);

    PARCBuffer *buf1 = parcBuffer_WrapCString("medium long");
    PARCBuffer *buf2 = parcBuffer_WrapCString("somewhat longer");
    PARCBuffer *buf3 = parcBuffer_WrapCString("short");

    parcSortedList_Add(instance, buf1);
    parcSortedList_Add(instance, buf2);
    parcSortedList_Add(instance, buf3);

    PARCBuffer *test = parcSortedList_GetAtIndex(instance, 0);
    assertTrue(test == buf3, "Expected the shortes buffer first");

    test = parcSortedList_GetAtIndex(instance, 1);
    assertTrue(test == buf1, "Expected the medium length buffer second");

    test = parcSortedList_GetAtIndex(instance, 2);
    assertTrue(test == buf2, "Expected the longest buffer last");

    parcBuffer_Release(&buf1);
    parcBuffer_Release(&buf2);
    parcBuffer_Release(&buf3);

    parcSortedList_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSortedList_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcSortedList_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcSortedList_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcSortedList_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcSortedList_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, parcSortedList_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, parcSortedList_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcSortedList_Copy)
{
    PARCSortedList *instance = parcSortedList_Create();
    PARCSortedList *copy = parcSortedList_Copy(instance);
    assertTrue(parcSortedList_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcSortedList_Release(&instance);
    parcSortedList_Release(&copy);
}

LONGBOW_TEST_CASE(Global, parcSortedList_Display)
{
    PARCSortedList *instance = parcSortedList_Create();
    parcSortedList_Display(instance, 0);
    parcSortedList_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcSortedList_Equals)
{
    PARCSortedList *x = parcSortedList_Create();
    PARCSortedList *y = parcSortedList_Create();
    PARCSortedList *z = parcSortedList_Create();

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcSortedList_Release(&x);
    parcSortedList_Release(&y);
    parcSortedList_Release(&z);
}

LONGBOW_TEST_CASE(Global, parcSortedList_HashCode)
{
    PARCSortedList *x = parcSortedList_Create();
    PARCSortedList *y = parcSortedList_Create();

    parcObjectTesting_AssertHashCode(x, y);

    parcSortedList_Release(&x);
    parcSortedList_Release(&y);
}

LONGBOW_TEST_CASE(Global, parcSortedList_IsValid)
{
    PARCSortedList *instance = parcSortedList_Create();
    assertTrue(parcSortedList_IsValid(instance), "Expected parcSortedList_Create to result in a valid instance.");

    parcSortedList_Release(&instance);
    assertFalse(parcSortedList_IsValid(instance), "Expected parcSortedList_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcSortedList_ToJSON)
{
    PARCSortedList *instance = parcSortedList_Create();

    PARCJSON *json = parcSortedList_ToJSON(instance);

    parcJSON_Release(&json);

    parcSortedList_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcSortedList_ToString)
{
    PARCSortedList *instance = parcSortedList_Create();

    char *string = parcSortedList_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcSortedList_ToString");

    parcMemory_Deallocate((void **) &string);
    parcSortedList_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_Add);
    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_Remove);
    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_GetAtIndex);
    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_GetFirst);
    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_GetLast);

    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_RemoveFirst);
    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_RemoveFirst_SingleElement);
    LONGBOW_RUN_TEST_CASE(Specialization, parcSortedList_RemoveLast);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

static void
dump(PARCSortedList *i)
{
    PARCIterator *iterator = parcSortedList_CreateIterator(i);
    while (parcIterator_HasNext(iterator)) {
        PARCBuffer *buffer = parcIterator_Next(iterator);
        parcBuffer_Display(buffer, 0);
    }

    parcIterator_Release(&iterator);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_Add)
{
    PARCSortedList *instance = parcSortedList_Create();
    PARCBuffer *element1 = parcBuffer_WrapCString("1");
    PARCBuffer *element2 = parcBuffer_WrapCString("2");
    PARCBuffer *element3 = parcBuffer_WrapCString("3");
    PARCBuffer *element4 = parcBuffer_WrapCString("4");
    PARCBuffer *element7 = parcBuffer_WrapCString("7");
    PARCBuffer *element6 = parcBuffer_WrapCString("6");
    PARCBuffer *element5 = parcBuffer_WrapCString("5");
    PARCBuffer *element8 = parcBuffer_WrapCString("8");

    parcSortedList_Add(instance, element2);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element8);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element3);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element4);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element7);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element6);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element5);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element1);
    parcSortedList_Display(instance, 0);
    parcSortedList_Add(instance, element6);
    parcSortedList_Display(instance, 0);

    dump(instance);

    parcBuffer_Release(&element1);
    parcBuffer_Release(&element2);
    parcBuffer_Release(&element3);
    parcBuffer_Release(&element4);
    parcBuffer_Release(&element5);
    parcBuffer_Release(&element6);
    parcBuffer_Release(&element7);
    parcBuffer_Release(&element8);
    parcSortedList_Release(&instance);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_Remove)
{
    PARCSortedList *instance = parcSortedList_Create();
    PARCBuffer *element1 = parcBuffer_WrapCString("1");
    PARCBuffer *element2 = parcBuffer_WrapCString("2");
    PARCBuffer *element3 = parcBuffer_WrapCString("3");
    PARCBuffer *element4 = parcBuffer_WrapCString("4");
    PARCBuffer *element7 = parcBuffer_WrapCString("7");
    PARCBuffer *element6 = parcBuffer_WrapCString("6");
    PARCBuffer *element5 = parcBuffer_WrapCString("5");
    PARCBuffer *element8 = parcBuffer_WrapCString("8");

    parcSortedList_Add(instance, element1);
    parcSortedList_Add(instance, element2);
    parcSortedList_Add(instance, element3);
    parcSortedList_Display(instance, 0);

    parcSortedList_Remove(instance, element2);

    assertTrue(parcSortedList_Size(instance) == 2, "Expected list to be 2 in size");

    parcBuffer_Release(&element1);
    parcBuffer_Release(&element2);
    parcBuffer_Release(&element3);
    parcBuffer_Release(&element4);
    parcBuffer_Release(&element5);
    parcBuffer_Release(&element6);
    parcBuffer_Release(&element7);
    parcBuffer_Release(&element8);
    parcSortedList_Release(&instance);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_GetAtIndex)
{
    PARCSortedList *instance = parcSortedList_Create();
    PARCBuffer *element1 = parcBuffer_WrapCString("1");
    PARCBuffer *element2 = parcBuffer_WrapCString("2");
    PARCBuffer *element3 = parcBuffer_WrapCString("3");
    PARCBuffer *element4 = parcBuffer_WrapCString("4");
    PARCBuffer *element7 = parcBuffer_WrapCString("7");
    PARCBuffer *element6 = parcBuffer_WrapCString("6");
    PARCBuffer *element5 = parcBuffer_WrapCString("5");
    PARCBuffer *element8 = parcBuffer_WrapCString("8");

    parcSortedList_Add(instance, element1);
    parcSortedList_Add(instance, element2);
    parcSortedList_Add(instance, element3);

    PARCBuffer *actual = (PARCBuffer *) parcSortedList_GetAtIndex(instance, 1);
    assertTrue(parcBuffer_Equals(element2, actual), "Got the wrong value at index 1");

    parcBuffer_Release(&element1);
    parcBuffer_Release(&element2);
    parcBuffer_Release(&element3);
    parcBuffer_Release(&element4);
    parcBuffer_Release(&element5);
    parcBuffer_Release(&element6);
    parcBuffer_Release(&element7);
    parcBuffer_Release(&element8);
    parcSortedList_Release(&instance);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_GetFirst)
{
    PARCSortedList *instance = parcSortedList_Create();
    PARCBuffer *element1 = parcBuffer_WrapCString("1");
    PARCBuffer *element2 = parcBuffer_WrapCString("2");
    PARCBuffer *element3 = parcBuffer_WrapCString("3");
    PARCBuffer *element4 = parcBuffer_WrapCString("4");
    PARCBuffer *element7 = parcBuffer_WrapCString("7");
    PARCBuffer *element6 = parcBuffer_WrapCString("6");
    PARCBuffer *element5 = parcBuffer_WrapCString("5");
    PARCBuffer *element8 = parcBuffer_WrapCString("8");

    parcSortedList_Add(instance, element1);
    parcSortedList_Add(instance, element2);
    parcSortedList_Add(instance, element3);

    PARCBuffer *actual = (PARCBuffer *) parcSortedList_GetFirst(instance);
    assertTrue(parcBuffer_Equals(element1, actual), "Got the wrong value.");

    parcBuffer_Release(&element1);
    parcBuffer_Release(&element2);
    parcBuffer_Release(&element3);
    parcBuffer_Release(&element4);
    parcBuffer_Release(&element5);
    parcBuffer_Release(&element6);
    parcBuffer_Release(&element7);
    parcBuffer_Release(&element8);
    parcSortedList_Release(&instance);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_GetLast)
{
    PARCSortedList *instance = parcSortedList_Create();
    PARCBuffer *element1 = parcBuffer_WrapCString("1");
    PARCBuffer *element2 = parcBuffer_WrapCString("2");
    PARCBuffer *element3 = parcBuffer_WrapCString("3");
    PARCBuffer *element4 = parcBuffer_WrapCString("4");
    PARCBuffer *element7 = parcBuffer_WrapCString("7");
    PARCBuffer *element6 = parcBuffer_WrapCString("6");
    PARCBuffer *element5 = parcBuffer_WrapCString("5");
    PARCBuffer *element8 = parcBuffer_WrapCString("8");

    parcSortedList_Add(instance, element1);
    parcSortedList_Add(instance, element2);
    parcSortedList_Add(instance, element3);

    PARCBuffer *actual = (PARCBuffer *) parcSortedList_GetLast(instance);
    assertTrue(parcBuffer_Equals(element3, actual), "Got the wrong value at index 1");

    parcBuffer_Release(&element1);
    parcBuffer_Release(&element2);
    parcBuffer_Release(&element3);
    parcBuffer_Release(&element4);
    parcBuffer_Release(&element5);
    parcBuffer_Release(&element6);
    parcBuffer_Release(&element7);
    parcBuffer_Release(&element8);
    parcSortedList_Release(&instance);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_RemoveFirst)
{
    PARCSortedList *deque = parcSortedList_Create();

    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    parcSortedList_Add(deque, object1);
    parcSortedList_Add(deque, object2);
    parcSortedList_Add(deque, object3);

    PARCBuffer *peek = parcSortedList_RemoveFirst(deque);
    assertTrue(parcBuffer_Equals(object1, peek), "Objects out of order");

    parcBuffer_Release(&peek);
    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcSortedList_Release(&deque);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_RemoveFirst_SingleElement)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCSortedList *deque = parcSortedList_Create();
    parcSortedList_Add(deque, object1);

    PARCBuffer *peek = parcSortedList_RemoveFirst(deque);
    assertTrue(parcBuffer_Equals(object1, peek),
               "Objects out of order.");

    parcBuffer_Release(&peek);
    parcBuffer_Release(&object1);
    parcSortedList_Release(&deque);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_RemoveLast)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");
    PARCBuffer *object2 = parcBuffer_WrapCString("2");
    PARCBuffer *object3 = parcBuffer_WrapCString("3");

    PARCSortedList *deque = parcSortedList_Create();
    parcSortedList_Add(deque, object1);
    parcSortedList_Add(deque, object2);
    parcSortedList_Add(deque, object3);

    PARCBuffer *peek = parcSortedList_RemoveLast(deque);
    assertTrue(parcBuffer_Equals(object3, peek),
               "Objects out of order.");

    parcBuffer_Release(&peek);

    parcBuffer_Release(&object1);
    parcBuffer_Release(&object2);
    parcBuffer_Release(&object3);
    parcSortedList_Release(&deque);
}

LONGBOW_TEST_CASE(Specialization, parcSortedList_RemoveLast_SingleElement)
{
    PARCBuffer *object1 = parcBuffer_WrapCString("1");

    PARCSortedList *deque = parcSortedList_Create();
    parcSortedList_Add(deque, object1);

    PARCBuffer *peek = parcSortedList_RemoveLast(deque);
    assertTrue(parcBuffer_Equals(object1, peek),
               "Objects out of order.");

    parcBuffer_Release(&object1);
    parcSortedList_Release(&deque);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_SortedList);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
