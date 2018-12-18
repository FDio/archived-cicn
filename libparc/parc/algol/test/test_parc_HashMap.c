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
#include "../parc_HashMap.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_StdlibMemory.h>

#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(parc_HashMap)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(ObjectContract);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_HashMap)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_HashMap)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateCapacity0);
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateCapacityNominal);
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
    PARCHashMap *instance = parcHashMap_Create();
    assertNotNull(instance, "Expeced non-null result from parcHashMap_Create();");
    parcObjectTesting_AssertAcquireReleaseContract(parcHashMap_Acquire, instance);

    parcHashMap_Release(&instance);
    assertNull(instance, "Expeced null result from parcHashMap_Release();");
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateCapacity0)
{
    const size_t CAPACITY = 0;
    PARCHashMap *instance = parcHashMap_CreateCapacity(CAPACITY);
    assertNotNull(instance, "Expeced non-null result from parcHashMap_Create();");
    parcObjectTesting_AssertAcquireReleaseContract(parcHashMap_Acquire, instance);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateCapacityNominal)
{
    const size_t CAPACITY = 10000;
    PARCHashMap *instance = parcHashMap_CreateCapacity(CAPACITY);
    assertNotNull(instance, "Expeced non-null result from parcHashMap_Create();");
    parcObjectTesting_AssertAcquireReleaseContract(parcHashMap_Acquire, instance);
    assertTrue(instance->capacity == CAPACITY, "Expect capacity to be %zu", CAPACITY);
    assertTrue(instance->size == 0, "Expect size to be 0");

    //Make sure all the buckets exist
    for (size_t i = 0; i < CAPACITY; ++i) {
        assertNull(instance->buckets[i], "Expect the hashmap to be clear");
    }

    parcHashMap_Release(&instance);
    assertNull(instance, "Expeced null result from parcHashMap_Release();");
}

LONGBOW_TEST_FIXTURE(ObjectContract)
{
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_Copy);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_Display);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_Equals);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_HashCode_Empty);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_HashCode_NonEmpty);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_IsValid);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_AssertValid);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_ToJSON);
    LONGBOW_RUN_TEST_CASE(ObjectContract, parcHashMap_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(ObjectContract)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(ObjectContract)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);

        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_Copy)
{
    PARCHashMap *instance = parcHashMap_Create();
    PARCHashMap *copy = parcHashMap_Copy(instance);

    assertTrue(parcHashMap_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcHashMap_Release(&instance);
    parcHashMap_Release(&copy);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_Display)
{
    PARCHashMap *x = parcHashMap_Create();
    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");
    parcHashMap_Put(x, key, value);
    parcHashMap_Display(x, 0);

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&x);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_Equals)
{
    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    PARCHashMap *x = parcHashMap_Create();
    parcHashMap_Put(x, key, value);
    PARCHashMap *y = parcHashMap_Create();
    parcHashMap_Put(y, key, value);
    PARCHashMap *z = parcHashMap_Create();
    parcHashMap_Put(z, key, value);

    PARCHashMap *u1 = parcHashMap_Create();

    PARCHashMap *u2 = parcHashMap_Create();
    parcHashMap_Put(u2, key, value);

    parcObjectTesting_AssertEquals(x, y, z, u1, NULL);

    parcHashMap_Release(&x);
    parcHashMap_Release(&y);
    parcHashMap_Release(&z);
    parcHashMap_Release(&u1);
    parcHashMap_Release(&u2);

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_HashCode_Empty)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCHashCode code = parcHashMap_HashCode(instance);

    assertTrue(code == 0, "Expected 0, actual %" PRIPARCHashCode, code);
    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_HashCode_NonEmpty)
{
    PARCHashMap *instance = parcHashMap_Create();
    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");
    parcHashMap_Put(instance, key, value);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    PARCHashCode code = parcHashMap_HashCode(instance);
    assertTrue(code != 0, "Expected a non-zero hash code, actual %" PRIPARCHashCode, code);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_IsValid)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");
    parcHashMap_Put(instance, key, value);
    assertTrue(parcHashMap_IsValid(instance), "Expected parcHashMap_Create to result in a valid instance.");

    parcHashMap_Release(&instance);
    assertFalse(parcHashMap_IsValid(instance), "Expected parcHashMap_Create to result in an invalid instance.");

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_AssertValid)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");
    parcHashMap_Put(instance, key, value);
    parcHashMap_AssertValid(instance);

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);
    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_ToJSON)
{
    PARCHashMap *instance = parcHashMap_Create();
    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");
    parcHashMap_Put(instance, key, value);

    PARCJSON *json = parcHashMap_ToJSON(instance);

    parcJSON_Release(&json);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(ObjectContract, parcHashMap_ToString)
{
    PARCHashMap *instance = parcHashMap_Create();
    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");
    parcHashMap_Put(instance, key, value);

    char *string = parcHashMap_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcHashMap_ToString");

    parcMemory_Deallocate(&string);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);
    parcHashMap_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Put);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_PutN);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Put_Replace);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Get_NoValue);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Contains_True);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Contains_False);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Remove);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Remove_False);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_Resize);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_GetClusteringNumber);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_CreateValueIterator);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_CreateValueIterator_HasNext);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_CreateValueIterator_Next);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_KeyIterator);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_KeyIterator_HasNext);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_KeyIterator_Next);
    LONGBOW_RUN_TEST_CASE(Global, parcHashMap_KeyIterator_Remove);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcHashMap_Put)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    //size_t keyReferences = parcObject_GetReferenceCount(key);
    size_t valueReferences = parcObject_GetReferenceCount(value);

    parcHashMap_Put(instance, key, value);
    //M.S. Put() now results in a copy of the key.
    //assertTrue(keyReferences + 1 == parcObject_GetReferenceCount(key), "Expected key reference to be incremented by 1.");
    assertTrue(valueReferences + 1 == parcObject_GetReferenceCount(value), "Expected value reference to be incremented by 1.");

    PARCBuffer *actual = (PARCBuffer *) parcHashMap_Get(instance, key);

    assertTrue(parcBuffer_Equals(value, actual), "Expected value was not returned from Get");

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

typedef struct {
    int64_t number;
} _Int;

static char *
_int_toString(const _Int *anInt)
{
    char *result = parcMemory_AllocateAndClear(22);
    sprintf(result, "%" PRIi64 "", anInt->number);
    return result;
}

static PARCHashCode
_int_hashCode(const _Int *anInt)
{
    PARCHashCode result = anInt->number;

    return result;
}

parcObject_ExtendPARCObject(_Int, NULL, NULL, _int_toString, NULL, NULL, _int_hashCode, NULL);

parcObject_ImplementRelease(_int, _Int);

static _Int *
_int_Create(int64_t anInt)
{
    _Int *_int = parcObject_CreateInstance(_Int);

    if (_int != NULL) {
        _int->number = anInt;
    }

    return _int;
}

LONGBOW_TEST_CASE(Global, parcHashMap_GetClusteringNumber)
{
    size_t minimumSize = 100;
    PARCHashMap *instance = parcHashMap_CreateCapacity(minimumSize);

    double maxLoadFactor = instance->maxLoadFactor;

    // Load a hash map up to its load-factor
    size_t testRunSize = minimumSize * maxLoadFactor - 20;
    srand(time(NULL));
    for (int i = 0; i < testRunSize; ++i) {
        _Int *key = _int_Create(rand());
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 1000 + i);
        parcHashMap_Put(instance, key, value);
        parcBuffer_Release(&value);
        _int_Release(&key);
    }

    double currentClusteringNumber = parcHashMap_GetClusteringNumber(instance);

    if (currentClusteringNumber < 0.5) {
        testWarn("Oddly low clustering number detected.");
    }

    if (currentClusteringNumber > 1.5) {
        testWarn("Oddly high clustering number detected.");
    }

    // This will load up one bucket
    for (int i = 0; i < 20; ++i) {
        _Int *key = _int_Create(1 + (100 * i));
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 10 + i);
        parcHashMap_Put(instance, key, value);
        parcBuffer_Release(&value);
        _int_Release(&key);
    }

    currentClusteringNumber = parcHashMap_GetClusteringNumber(instance);

    parcHashMap_Release(&instance);

    if (currentClusteringNumber < 2.9) {
        testWarn("Oddly low clustering number detected.");
    }
}

LONGBOW_TEST_CASE(Global, parcHashMap_Resize)
{
    size_t initialSize = 8;
    PARCHashMap *instance = parcHashMap_CreateCapacity(initialSize);

    PARCBuffer *key = parcBuffer_Allocate(sizeof(uint32_t));
    PARCBuffer *value42 = parcBuffer_WrapCString("value42");
    double maxLoadFactor = instance->maxLoadFactor;

    // Load a hash map up to its load-factor
    size_t testRunSize = initialSize * maxLoadFactor;
    for (uint32_t i = 0; i < testRunSize; ++i) {
        parcBuffer_PutUint32(key, i);
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 1000 + i);
        parcHashMap_Put(instance, parcBuffer_Flip(key), value);
        parcBuffer_Release(&value);
    }
    assertTrue(parcHashMap_Size(instance) == testRunSize, "Expect the size to be %zu", testRunSize);
    assertTrue(instance->capacity == initialSize, "Expect to have the original capacity");

    // Test for expected values
    for (uint32_t i = 0; i < testRunSize; ++i) {
        parcBuffer_PutUint32(key, i);
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 1000 + i);
        const PARCBuffer *storedValue = parcHashMap_Get(instance, parcBuffer_Flip(key));
        assertTrue(parcBuffer_Equals(value, storedValue), "Expect looked up values to match");
        parcBuffer_Release(&value);
    }

    // Add one more item to the the hash map, this should trigger an expansion
    parcBuffer_PutUint32(key, 42);
    parcHashMap_Put(instance, parcBuffer_Flip(key), value42);
    assertTrue(parcHashMap_Size(instance) == testRunSize + 1, "Expect the size to be %zu", testRunSize);
    assertTrue(instance->capacity == 2 * initialSize, "Expect to have the original capacity");

    // Re-test value look ups to make sure the new hash map still maps correctly
    for (uint32_t i = 0; i < testRunSize; ++i) {
        parcBuffer_PutUint32(key, i);
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 1000 + i);
        const PARCBuffer *storedValue = parcHashMap_Get(instance, parcBuffer_Flip(key));
        assertTrue(parcBuffer_Equals(value, storedValue), "Expect looked up values to match");
        parcBuffer_Release(&value);
    }
    double averageBucketSize = parcHashMap_GetClusteringNumber(instance);
    parcBuffer_PutUint32(key, 42);
    const PARCBuffer *storedValue = parcHashMap_Get(instance, parcBuffer_Flip(key));
    assertTrue(parcBuffer_Equals(value42, storedValue), "Expect to get back value42");
    parcBuffer_Release(&value42);
    assertTrue(parcHashMap_GetClusteringNumber(instance) <= averageBucketSize,
               "Expect the average bucket size to be less then it was");

    // Now test multiple expansions to make sure they happened are result in a valid hash map
    size_t testCapacity = 1024;
    // If we load up to (maxLoadFactor * testCapacity) + 1, the capacity should expand to 2 * testCapacity
    testRunSize = (testCapacity * maxLoadFactor) + 1;
    for (uint32_t i = 0; i < testRunSize; ++i) {
        parcBuffer_PutUint32(key, i);
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 1000 + i);
        parcHashMap_Put(instance, parcBuffer_Flip(key), value);
        parcBuffer_Release(&value);
        if (i == (testRunSize - 2)) {
            averageBucketSize = parcHashMap_GetClusteringNumber(instance);
        }
    }
    assertTrue(instance->capacity == (2 * testCapacity),
               "Expect capacity to be %zu got %zu", (2 * testCapacity), instance->capacity);
    assertTrue(parcHashMap_GetClusteringNumber(instance) < averageBucketSize,
               "Expect the average bucket size to be less then it was");

    // Now test multiple contractions.
    // If we remove all elements from index "smallSize" (eg. 8) up we will be left with a map of size smallSize,
    // the map capacity should be contracting and, because the minimum load factor is 0.25 and the contraction
    // is a divide by 2, the last contraction should be from capacity of "smallSize * 4" (32) to one
    // of "smallSize * 2" (16) when size goes from "smallSize +1" (9) to "smallSize" (8).
    //
    size_t smallSize = 8;
    for (uint32_t i = smallSize; i < testRunSize; ++i) {
        parcBuffer_PutUint32(key, i);
        parcBuffer_Flip(key);
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 1000 + i);
        const PARCBuffer *storedValue = parcHashMap_Get(instance, key);
        assertTrue(parcBuffer_Equals(value, storedValue), "Expect looked up values to match");
        parcBuffer_Release(&value);

        assertTrue(parcHashMap_Remove(instance, key), "Expect Remove to succeed");
    }
    assertTrue(instance->size == smallSize,
               "Expect the hash map to have size %zu, got %zu", smallSize, instance->size)
    assertTrue(instance->capacity == (smallSize * 2),
               "Expect capacity to be %zu, got %zu", (smallSize * 2), instance->capacity);

    // Re-test value look ups to make sure the new hash map still maps correctly
    for (uint32_t i = 0; i < smallSize; ++i) {
        parcBuffer_PutUint32(key, i);
        PARCBuffer *value = parcBuffer_Allocate(sizeof(uint32_t));
        parcBuffer_PutUint32(value, 1000 + i);
        const PARCBuffer *storedValue = parcHashMap_Get(instance, parcBuffer_Flip(key));
        assertTrue(parcBuffer_Equals(value, storedValue), "Expect looked up values to match");
        parcBuffer_Release(&value);
    }

    parcBuffer_Release(&key);
    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_PutN)
{
    size_t testRunSize = 100;

    PARCHashMap *instance = parcHashMap_CreateCapacity(testRunSize);

    PARCBuffer *key = parcBuffer_Allocate(sizeof(uint32_t));
    PARCBuffer *value = parcBuffer_WrapCString("value1");
    PARCBuffer *value42 = parcBuffer_WrapCString("value42");
    for (uint32_t i = 0; i < testRunSize * 2; ++i) {
        parcBuffer_PutUint32(key, i);
        parcHashMap_Put(instance, parcBuffer_Flip(key), value);
        if (i == 42) {
            parcHashMap_Put(instance, key, value42);
        }
    }

    parcBuffer_PutUint32(key, 42);
    PARCBuffer *actual = (PARCBuffer *) parcHashMap_Get(instance, parcBuffer_Flip(key));
    assertTrue(parcBuffer_Equals(value42, actual), "Expect to get back value42");

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);
    parcBuffer_Release(&value42);
    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_Put_Replace)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value1 = parcBuffer_WrapCString("value1");
    PARCBuffer *value2 = parcBuffer_WrapCString("value2");

    parcHashMap_Put(instance, key, value1);

    parcHashMap_Put(instance, key, value2);

    PARCBuffer *actual = (PARCBuffer *) parcHashMap_Get(instance, key);

    assertTrue(parcBuffer_Equals(value2, actual), "Expected value was not returned from Get");

    parcBuffer_Release(&key);
    parcBuffer_Release(&value1);
    parcBuffer_Release(&value2);
    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_Get_NoValue)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");

    PARCBuffer *actual = (PARCBuffer *) parcHashMap_Get(instance, key);

    assertNull(actual, "Expected parcHashMap_Get to return NULL for non-existent key.");

    parcBuffer_Release(&key);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_Contains_True)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    parcHashMap_Put(instance, key, value);

    bool actual = parcHashMap_Contains(instance, key);

    assertTrue(actual, "Expected parcHashMap_Contains to return true");

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_Contains_False)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");

    bool actual = parcHashMap_Contains(instance, key);

    assertFalse(actual, "Expected parcHashMap_Contains to return NULL for non-existent key.");

    parcBuffer_Release(&key);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_Remove)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    parcHashMap_Put(instance, key, value);

    bool actual = parcHashMap_Remove(instance, key);

    assertTrue(actual, "Expected parcHashMap_Remove to return true.");

    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_Remove_False)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *key2 = parcBuffer_WrapCString("key2");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    parcHashMap_Put(instance, key, value);

    bool actual = parcHashMap_Remove(instance, key2);

    assertFalse(actual, "Expected parcHashMap_Remove to return false.");

    parcBuffer_Release(&key);
    parcBuffer_Release(&key2);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_CreateValueIterator)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    parcHashMap_Put(instance, key, value);

    PARCIterator *iterator = parcHashMap_CreateValueIterator(instance);

    assertNotNull(iterator, "Expected parcHashMap_ValueIterator to return non-null result");

    parcIterator_Release(&iterator);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_CreateValueIterator_HasNext)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    parcHashMap_Put(instance, key, value);

    PARCIterator *iterator = parcHashMap_CreateValueIterator(instance);

    assertTrue(parcIterator_HasNext(iterator), "Expected parcIterator_HasNext to return true");

    parcIterator_Release(&iterator);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_CreateValueIterator_Next)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key1 = parcBuffer_WrapCString("key1");
    PARCBuffer *value1 = parcBuffer_WrapCString("1");
    PARCBuffer *key2 = parcBuffer_WrapCString("key2");
    PARCBuffer *value2 = parcBuffer_WrapCString("2");
    PARCBuffer *key3 = parcBuffer_WrapCString("key3");
    PARCBuffer *value3 = parcBuffer_WrapCString("3");
    PARCBuffer *key4 = parcBuffer_WrapCString("key4");
    PARCBuffer *value4 = parcBuffer_WrapCString("4");

    parcHashMap_Put(instance, key1, value1);
    parcHashMap_Put(instance, key2, value2);
    parcHashMap_Put(instance, key3, value3);
    parcHashMap_Put(instance, key4, value4);

    PARCIterator *iterator = parcHashMap_CreateValueIterator(instance);

    while (parcIterator_HasNext(iterator)) {
        PARCBuffer *actual = parcIterator_Next(iterator);
        assertNotNull(actual, "Expected parcIterator_Next to return non-null");
        assertTrue(parcBuffer_Remaining(actual) > 0, "The same value appeared more than once in the iteration");
        parcBuffer_SetPosition(actual, parcBuffer_Limit(actual));
    }
    parcIterator_Release(&iterator);

    parcBuffer_Release(&key1);
    parcBuffer_Release(&value1);
    parcBuffer_Release(&key2);
    parcBuffer_Release(&value2);
    parcBuffer_Release(&key3);
    parcBuffer_Release(&value3);
    parcBuffer_Release(&key4);
    parcBuffer_Release(&value4);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_KeyIterator)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    parcHashMap_Put(instance, key, value);

    PARCIterator *iterator = parcHashMap_CreateKeyIterator(instance);

    assertNotNull(iterator, "Expected parcHashMap_KeyIterator to return non-null result");

    parcIterator_Release(&iterator);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_KeyIterator_HasNext)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    parcHashMap_Put(instance, key, value);

    PARCIterator *iterator = parcHashMap_CreateKeyIterator(instance);

    assertTrue(parcIterator_HasNext(iterator), "Expected parcIterator_HasNext to return true");

    parcIterator_Release(&iterator);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_KeyIterator_Next)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key1 = parcBuffer_WrapCString("key1");
    PARCBuffer *value1 = parcBuffer_WrapCString("1");
    PARCBuffer *key2 = parcBuffer_WrapCString("key2");
    PARCBuffer *value2 = parcBuffer_WrapCString("2");
    PARCBuffer *key3 = parcBuffer_WrapCString("key3");
    PARCBuffer *value3 = parcBuffer_WrapCString("3");
    PARCBuffer *key4 = parcBuffer_WrapCString("key4");
    PARCBuffer *value4 = parcBuffer_WrapCString("4");

    parcHashMap_Put(instance, key1, value1);
    parcHashMap_Put(instance, key2, value2);
    parcHashMap_Put(instance, key3, value3);
    parcHashMap_Put(instance, key4, value4);

    PARCIterator *iterator = parcHashMap_CreateKeyIterator(instance);

    while (parcIterator_HasNext(iterator)) {
        PARCBuffer *actual = parcIterator_Next(iterator);
        assertNotNull(actual, "Expected parcIterator_Next to return non-null");
        assertTrue(parcBuffer_Remaining(actual) > 0, "The same value appeared more than once in the iteration");
        parcBuffer_SetPosition(actual, parcBuffer_Limit(actual));
    }
    parcIterator_Release(&iterator);

    parcBuffer_Release(&key1);
    parcBuffer_Release(&value1);
    parcBuffer_Release(&key2);
    parcBuffer_Release(&value2);
    parcBuffer_Release(&key3);
    parcBuffer_Release(&value3);
    parcBuffer_Release(&key4);
    parcBuffer_Release(&value4);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcHashMap_KeyIterator_Remove)
{
    PARCHashMap *instance = parcHashMap_Create();

    PARCBuffer *key1 = parcBuffer_WrapCString("key1");
    PARCBuffer *value1 = parcBuffer_WrapCString("1");
    PARCBuffer *key2 = parcBuffer_WrapCString("key2");
    PARCBuffer *value2 = parcBuffer_WrapCString("2");
    PARCBuffer *key3 = parcBuffer_WrapCString("key3");
    PARCBuffer *value3 = parcBuffer_WrapCString("3");
    PARCBuffer *key4 = parcBuffer_WrapCString("key4");
    PARCBuffer *value4 = parcBuffer_WrapCString("4");

    parcHashMap_Put(instance, key1, value1);
    parcHashMap_Put(instance, key2, value2);
    parcHashMap_Put(instance, key3, value3);
    parcHashMap_Put(instance, key4, value4);

    assertTrue(parcHashMap_Size(instance) == 4, "Expected 4, actual %zd", parcHashMap_Size(instance));
    PARCIterator *iterator = parcHashMap_CreateKeyIterator(instance);

    while (parcIterator_HasNext(iterator)) {
        PARCBuffer *key = parcBuffer_Acquire(parcIterator_Next(iterator));
        parcIterator_Remove(iterator);
        assertNull(parcHashMap_Get(instance, key), "Expected deleted entry to not be gettable.");
        parcBuffer_Release(&key);
    }
    parcIterator_Release(&iterator);

    assertTrue(parcHashMap_Size(instance) == 0, "Expected 0, actual %zd", parcHashMap_Size(instance));
    parcBuffer_Release(&key1);
    parcBuffer_Release(&value1);
    parcBuffer_Release(&key2);
    parcBuffer_Release(&value2);
    parcBuffer_Release(&key3);
    parcBuffer_Release(&value3);
    parcBuffer_Release(&key4);
    parcBuffer_Release(&value4);

    parcHashMap_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, parcHashMapEntry);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Static, parcHashMapEntry)
{
    PARCBuffer *key = parcBuffer_WrapCString("key1");
    PARCBuffer *value = parcBuffer_WrapCString("value1");

    _PARCHashMapEntry *instance = _parcHashMapEntry_Create(key, value);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    _parcHashMapEntry_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_HashMap);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
