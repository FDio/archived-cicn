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


#include <config.h>
#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include "../parc_Dictionary.c"

static int *
_keyNewInt(int key)
{
    int *newKey = parcMemory_Allocate(sizeof(int));
    assertNotNull(newKey, "parcMemory_Allocate(%zu) returned NULL",
                  sizeof(int));
    *newKey = key;
    return newKey;
}

static int *
_valueNewInt(int value)
{
    int *newValue = parcMemory_Allocate(sizeof(int));
    assertNotNull(newValue, "parcMemory_Allocate(%zu) returned NULL", sizeof(int));
    *newValue = value;
    return newValue;
}

static bool
_valueEquals(const void *value1, const void *value2)
{
    return *(int *) value1 == *(int *) value2;
}

static int
_intKeyComp(const void *key1, const void *key2)
{
    if (*(int *) key1 < *(int *) key2) {
        return -1;
    }
    if (*(int *) key1 == *(int *) key2) {
        return 0;
    }
    return 1;
}

static uint32_t
_intKeyHash(const void *key1)
{
    return *(int *) key1;
}


static void
_keyFree(void **value)
{
    parcMemory_Deallocate((void **) value);
    *value = NULL;
}

static void
_valueFree(void **key)
{
    parcMemory_Deallocate((void **) key);
    *key = NULL;
}




LONGBOW_TEST_RUNNER(PARC_Dictionary)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

LONGBOW_TEST_RUNNER_SETUP(PARC_Dictionary)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(PARC_Dictionary)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(-1);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Create);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_SetValue_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Size_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Size);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Size_AfterDelete);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Size_AfterOverwrite);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Get_EmptyTree);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Get_NonExistent);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Get_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Get);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Get_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Remove_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Remove);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Remove_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_RemoveAndDestroy_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_RemoveAndDestroy);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_RemoveAndDestroy_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Keys);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Values);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Equals_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Equals);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Equals_Not_Values);
    LONGBOW_RUN_TEST_CASE(Global, PARC_Dictionary_Equals_Not_Keys);
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

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Create)
{
    PARCDictionary *dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, NULL, NULL, NULL);

    parcDictionary_Destroy(&dictionary);

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_SetValue_Destroy)
{
    PARCDictionary *dictionary;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(1), (void *) _valueNewInt(11));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(2), (void *) _valueNewInt(12));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(3), (void *) _valueNewInt(13));

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Size_Empty)
{
    PARCDictionary *dictionary;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    assertTrue(0 == parcDictionary_Size(dictionary), "Wrong size of dictionary - empty, start");

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Size)
{
    PARCDictionary *dictionary;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(4), (void *) _valueNewInt(1004));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(3), (void *) _valueNewInt(1003));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(2), (void *) _valueNewInt(1002));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(8), (void *) _valueNewInt(1008));

    assertTrue(4 == parcDictionary_Size(dictionary), "Wrong size of dictionary after add 3");

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Size_AfterDelete)
{
    PARCDictionary *dictionary;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(4), (void *) _valueNewInt(1004));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(3), (void *) _valueNewInt(1003));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(2), (void *) _valueNewInt(1002));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(8), (void *) _valueNewInt(1008));

    int searchKey = 2;

    parcDictionary_RemoveAndDestroyValue(dictionary, &searchKey);

    size_t size = parcDictionary_Size(dictionary);

    assertTrue(3 == size, "Wrong size of dictionary after 1 delete (%zu instead of 3)", size);

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Size_AfterOverwrite)
{
    PARCDictionary *dictionary;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(4), (void *) _valueNewInt(1004));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(3), (void *) _valueNewInt(1003));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(2), (void *) _valueNewInt(1002));
    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(8), (void *) _valueNewInt(1008));

    parcDictionary_SetValue(dictionary, (void *) _keyNewInt(3), (void *) _valueNewInt(1010));

    size_t size = parcDictionary_Size(dictionary);

    assertTrue(4 == size, "Wrong size of dictionary after 1 delete (%zu instead of 4)", size);

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Get_EmptyTree)
{
    PARCDictionary *dictionary;

    int key = 100;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    void *value = parcDictionary_GetValue(dictionary, &key);

    assertTrue(NULL == value, "Object did not exist, must return NULL");

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Get_NonExistent)
{
    PARCDictionary *dictionary;
    int key = 100;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 1; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    void *value = parcDictionary_GetValue(dictionary, &key);

    assertTrue(NULL == value, "Object did not exist, must return NULL");

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Get_First)
{
    PARCDictionary *dictionary;
    int key = 1;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 1; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int *value = parcDictionary_GetValue(dictionary, &key);

    assertNotNull(value, "NULL value returned");
    assertTrue(*value == (1 << 8), "Wrong object returned or not found");

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Get)
{
    PARCDictionary *dictionary;
    int key = 5;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 1; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int *value = parcDictionary_GetValue(dictionary, &key);

    assertNotNull(value, "NULL value returned");
    assertTrue(*value == (5 << 8), "Wrong object returned or not found");

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Get_Last)
{
    PARCDictionary *dictionary;
    int key = 9;

    dictionary = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 1; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int *value = parcDictionary_GetValue(dictionary, &key);

    assertNotNull(value, "NULL value returned");
    assertTrue(*value == (9 << 8), "Wrong object returned or not found");

    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Remove_First)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 30; i < 40; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(1), (void *) _valueNewInt(1 << 8));
    for (int i = 2; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int searchKey = 1;

    void *data = parcDictionary_RemoveValue(dictionary1, &searchKey);

    _valueFree(&data);

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Trees dont match after remove");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Remove)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 31; i < 40; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(30), (void *) _valueNewInt(31 << 8));
    for (int i = 2; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int searchKey = 30;

    void *data = parcDictionary_RemoveValue(dictionary1, &searchKey);

    _valueFree(&data);

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Trees dont match after remove");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Remove_Last)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 30; i < 40; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(100), (void *) _valueNewInt(100 << 8));
    for (int i = 2; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int searchKey = 100;

    void *data = parcDictionary_RemoveValue(dictionary1, &searchKey);

    _valueFree(&data);

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Trees dont match after remove");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_RemoveAndDestroy_First)
{
    PARCDictionary *dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    PARCDictionary *dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 30; i < 40; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(1), (void *) _valueNewInt(1 << 8));
    for (int i = 2; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int searchKey = 1;

    parcDictionary_RemoveAndDestroyValue(dictionary1, &searchKey);

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Trees dont match after remove");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_RemoveAndDestroy)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 31; i < 40; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(30), (void *) _valueNewInt(31 << 8));
    for (int i = 2; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int searchKey = 30;

    parcDictionary_RemoveAndDestroyValue(dictionary1, &searchKey);

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Trees dont match after remove");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_RemoveAndDestroy_Last)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 30; i < 40; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(100), (void *) _valueNewInt(100 << 8));
    for (int i = 2; i < 10; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary1, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    int searchKey = 100;

    parcDictionary_RemoveAndDestroyValue(dictionary1, &searchKey);

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Trees dont match after remove");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Keys)
{
    PARCDictionary *dictionary =
        parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 0; i < 9; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    PARCArrayList *keys = parcDictionary_Keys(dictionary);

    assertNotNull(keys,
                  "parcDictionary_Keys returned NULL, expected non-NULL.");

    assertTrue(parcArrayList_Size(keys) == parcDictionary_Size(dictionary),
               "Expected parcDictionary_Keys size %zu, actual %zd, ",
               parcDictionary_Size(dictionary), parcArrayList_Size(keys));

    for (int i = 0; i < 9; i++) {
        bool found = false;
        int *keyToFind = _keyNewInt(i);
        for (int j = 0; j < parcArrayList_Size(keys); j++) {
            int *keyToTest = parcArrayList_Get(keys, j);
            if (*keyToTest == *keyToFind) {
                found = true;
                break;
            }
        }
        assertTrue(found, "Expected to find Key %d, not found", *keyToFind);
        parcMemory_Deallocate((void **) &keyToFind);
    }
    parcArrayList_Destroy(&keys);
    parcDictionary_Destroy(&dictionary);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Values)
{
    PARCDictionary *dictionary =
        parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 0; i < 9; i++) {
        // Add some elements to the dictionary
        parcDictionary_SetValue(dictionary, (void *) _keyNewInt(i), (void *) _valueNewInt(i << 8));
    }

    PARCArrayList *values = parcDictionary_Values(dictionary);

    assertNotNull(values,
                  "parcDictionary_Values returned NULL, expected not NULL");

    assertTrue(parcArrayList_Size(values) == parcDictionary_Size(dictionary),
               "parcDictionary_Values size %zd not equal not parcDictionary_Size, %zd",
               parcArrayList_Size(values), parcDictionary_Size(dictionary));

    for (int i = 0; i < 9; i++) {
        bool found = false;
        int *keyToFind = _keyNewInt(i);
        int *valueToFind = parcDictionary_GetValue(dictionary, keyToFind);
        for (int j = 0; j < parcArrayList_Size(values); j++) {
            int *valueToTest = parcArrayList_Get(values, j);
            if (valueToFind == valueToTest) {
                found = true;
                break;
            }
        }
        assertTrue(found,
                   "Expected to find value %d, not found", *valueToFind);
        parcMemory_Deallocate((void **) &keyToFind);
    }
    parcArrayList_Destroy(&values);
    parcDictionary_Destroy(&dictionary);
}


LONGBOW_TEST_CASE(Global, PARC_Dictionary_Equals_Empty)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Empty lists are not equal");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Equals_Not_Values)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    int compareSetValues = 100;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    printf("Testing dictionary equals...\n");

    for (int i = 1; i < compareSetValues; i++) {
        parcDictionary_SetValue(dictionary1,
                                (void *) _keyNewInt(i),
                                (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2,
                                (void *) _keyNewInt(compareSetValues - i),
                                (void *) _valueNewInt((compareSetValues + i) << 8));
    }

    assertFalse(parcDictionary_Equals(dictionary1, dictionary2), "Dictionaries are equal and they shouldn't be!");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Equals_Not_Keys)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    int compareSetValues = 100;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 1; i < compareSetValues; i++) {
        parcDictionary_SetValue(dictionary1,
                                (void *) _keyNewInt(i),
                                (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2,
                                (void *) _keyNewInt(compareSetValues + i),
                                (void *) _valueNewInt((compareSetValues - i) << 8));
    }

    assertFalse(parcDictionary_Equals(dictionary1, dictionary2), "Lists are equal");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
}

LONGBOW_TEST_CASE(Global, PARC_Dictionary_Equals)
{
    PARCDictionary *dictionary1;
    PARCDictionary *dictionary2;

    int compareSetValues = 100;

    dictionary1 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);
    dictionary2 = parcDictionary_Create(_intKeyComp, _intKeyHash, _keyFree, _valueEquals, _valueFree);

    for (int i = 1; i < compareSetValues; i++) {
        parcDictionary_SetValue(dictionary1,
                                (void *) _keyNewInt(i),
                                (void *) _valueNewInt(i << 8));
        parcDictionary_SetValue(dictionary2,
                                (void *) _keyNewInt(compareSetValues - i),
                                (void *) _valueNewInt((compareSetValues - i) << 8));
    }

    assertTrue(parcDictionary_Equals(dictionary1, dictionary2), "Dictionaries are not equal");

    parcDictionary_Destroy(&dictionary1);
    parcDictionary_Destroy(&dictionary2);
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

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARC_Dictionary);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
