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

/*
 *
 */
#include <config.h>
#include <LongBow/unit-test.h>

#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_KeyValue.c"


LONGBOW_TEST_RUNNER(parc_KeyValue)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(PARCKeyValueAsPARCObject);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_KeyValue)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_KeyValue)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(PARCKeyValueAsPARCObject)
{
    LONGBOW_RUN_TEST_CASE(PARCKeyValueAsPARCObject, parcObject_Conformance);
}

LONGBOW_TEST_FIXTURE_SETUP(PARCKeyValueAsPARCObject)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(PARCKeyValueAsPARCObject)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(PARCKeyValueAsPARCObject, parcObject_Conformance)
{
    PARCBuffer *key = parcBuffer_WrapCString("Key_1");
    PARCBuffer *value = parcBuffer_WrapCString("Value");
    PARCKeyValue *inst1 = parcKeyValue_Create(key, value);
    PARCKeyValue *inst2 = parcKeyValue_Create(key, value);
    PARCKeyValue *inst3 = parcKeyValue_Create(key, value);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    key = parcBuffer_WrapCString("Key_0");
    value = parcBuffer_WrapCString("Value");
    PARCKeyValue *lesser = parcKeyValue_Create(key, value);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    key = parcBuffer_WrapCString("Key_2");
    value = parcBuffer_WrapCString("Value");
    PARCKeyValue *greater = parcKeyValue_Create(key, value);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);

    parcObjectTesting_AssertObjectConformance(inst1, inst2, inst3, lesser, greater);

    parcKeyValue_Release(&inst1);
    parcKeyValue_Release(&inst2);
    parcKeyValue_Release(&inst3);
    parcKeyValue_Release(&lesser);
    parcKeyValue_Release(&greater);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_GetKey);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_GetValue);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_EqualKeys);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_SetKey);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyValue_SetValue);
}

typedef struct {
    PARCKeyValue *testKV1;
    PARCKeyValue *testKV2;
    PARCKeyValue *nullValue;
    PARCBuffer *key1;
    PARCBuffer *value1;
    PARCBuffer *key2;
    PARCBuffer *value2;
} TestData;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    data->key1 = parcBuffer_WrapCString("This is key 1");
    data->value1 = parcBuffer_WrapCString("This is value 1");
    data->key2 = parcBuffer_WrapCString("This is key 2");
    data->value2 = parcBuffer_WrapCString("This is value 2");

    data->testKV1 = parcKeyValue_Create(data->key1, data->value1);
    data->testKV2 = parcKeyValue_Create(data->key2, data->value2);

    PARCBuffer *nullKey = parcBuffer_WrapCString("NULL KEY");
    data->nullValue = parcKeyValue_Create(nullKey, NULL);
    parcBuffer_Release(&nullKey);

    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    parcBuffer_Release(&data->key1);
    parcBuffer_Release(&data->value1);
    parcBuffer_Release(&data->key2);
    parcBuffer_Release(&data->value2);

    parcKeyValue_Release(&data->testKV1);
    parcKeyValue_Release(&data->testKV2);
    parcKeyValue_Release(&data->nullValue);

    parcMemory_Deallocate((void **) &data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcKeyValue_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    assertNotNull(data->testKV1, "Expect a non-NULL key value");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_Acquire)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    assertNotNull(data->testKV1, "Expect a non-NULL key value");

    PARCKeyValue *kv = parcKeyValue_Acquire(data->testKV1);
    parcKeyValue_Release(&kv);
}

LONGBOW_TEST_CASE(Global, parcKeyValue_GetKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    assertTrue(parcBuffer_Equals(parcKeyValue_GetKey(data->testKV1), data->key1),
               "The key returned is not the key provided");

    assertNotNull(parcKeyValue_GetKey(data->nullValue), "Expect Non-NULL key from NULL value kv");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_GetValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    assertTrue(parcBuffer_Equals(parcKeyValue_GetValue(data->testKV1), data->value1),
               "The key returned is not the key provided");

    assertNull(parcKeyValue_GetValue(data->nullValue), "Expect NULL from GetValue");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_Equals)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    bool isEqual = parcKeyValue_Equals(data->testKV1, data->testKV2);
    assertFalse(isEqual, "Expect test key-values to not be equal");

    PARCKeyValue *kv = parcKeyValue_Create(data->key1, data->value2);
    isEqual = parcKeyValue_Equals(kv, data->testKV1) || parcKeyValue_Equals(kv, data->testKV2);
    parcKeyValue_Release(&kv);
    assertFalse(isEqual, "Expect test key-values to not be equal");

    kv = parcKeyValue_Create(data->key1, data->value1);
    isEqual = parcKeyValue_Equals(kv, data->testKV1) && !parcKeyValue_Equals(kv, data->testKV2);
    parcKeyValue_Release(&kv);

    assertTrue(isEqual, "Expect test key-values to be equal");

    // NULL values
    isEqual = parcKeyValue_Equals(data->testKV1, data->nullValue);
    assertFalse(isEqual, "Expect NULL key-valuet to not be equal");

    kv = parcKeyValue_Copy(data->nullValue);
    isEqual = parcKeyValue_Equals(kv, data->nullValue);
    assertTrue(isEqual, "Expect NULL key-valuet to not be equal");
    parcKeyValue_Release(&kv);
}

LONGBOW_TEST_CASE(Global, parcKeyValue_Compare)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcKeyValue_Compare(NULL, NULL) == 0, "Expect 0 from comparing NULLs");
    assertTrue(parcKeyValue_Compare(data->testKV1, NULL) > 0, "Expect result > 0 from comparing non-NULL to NULL");
    assertTrue(parcKeyValue_Compare(NULL, data->testKV1) < 0, "Expect result < 0 from comparing NULL to non-NULL");

    int result = parcKeyValue_Compare(data->testKV1, data->testKV2);
    assertTrue(result < 0, "Expect compareison to be < 0");

    result = parcKeyValue_Compare(data->testKV2, data->testKV1);
    assertTrue(result > 0, "Expect compareison to be > 0");

    // Mixed keys & values
    PARCKeyValue *kv = parcKeyValue_Create(data->key1, data->value2);
    result = parcKeyValue_Compare(kv, data->testKV1);
    assertTrue(result == 0, "Expect comparison to be 0");

    result = parcKeyValue_Compare(kv, data->testKV2);
    assertTrue(result < 0, "Expect comparison to be < 0");

    parcKeyValue_Release(&kv);

    // NULL value
    result = parcKeyValue_Compare(data->testKV1, data->nullValue);
    assertTrue(result > 0, "Expect NULL key-value be > 0");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_HashCode)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCHashCode hash1 = parcKeyValue_HashCode(data->testKV1);
    PARCHashCode hash2 = parcKeyValue_HashCode(data->testKV2);
    assertFalse(hash1 == hash2, "Expect hash codes to be different");

    PARCKeyValue *kv = parcKeyValue_Create(data->key1, data->value1);
    hash2 = parcKeyValue_HashCode(kv);
    assertTrue(hash1 == hash2, "Expect hash codes to be equal");
    parcKeyValue_Release(&kv);

    // Mixed keys & values
    kv = parcKeyValue_Create(data->key1, data->value2);
    hash2 = parcKeyValue_HashCode(kv);
    assertTrue(hash1 == hash2, "Expect hash codes to be equal");
    parcKeyValue_Release(&kv);

    // NULL value
    PARCHashCode hash = parcKeyValue_HashCode(data->nullValue);
    assertTrue(hash != 0, "Expect NULL key-value hash to != 0");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_EqualKeys)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    assertFalse(parcKeyValue_EqualKeys(data->testKV1, data->testKV2), "Expect keys to be different");

    PARCKeyValue *kv = parcKeyValue_Create(data->key1, data->value2);

    assertTrue(parcKeyValue_EqualKeys(data->testKV1, kv), "Expect keys to be equal");

    parcKeyValue_Release(&kv);

    // NULL value
    assertFalse(parcKeyValue_EqualKeys(data->nullValue, data->testKV1), "Expect NULL key-value hash to != 0");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_SetKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    parcKeyValue_SetKey(data->testKV2, data->key1);

    assertTrue(parcKeyValue_EqualKeys(data->testKV1, data->testKV2),
               "Expect kv keys to be equal after SetKey");

    // NULL value
    parcKeyValue_SetKey(data->nullValue, data->key1);
    assertTrue(parcKeyValue_EqualKeys(data->testKV1, data->nullValue),
               "Expect kv keys to be equal after SetKey");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_SetValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    parcKeyValue_SetValue(data->testKV2, data->value1);

    assertTrue(parcBuffer_Equals(parcKeyValue_GetValue(data->testKV1),
                                 parcKeyValue_GetValue(data->testKV2)),
               "Expect kv values to be equal after SetValue");

    // NULL value
    parcKeyValue_SetValue(data->testKV2, NULL);
    assertNull(parcKeyValue_GetValue(data->testKV2),
               "Expect NULL for testKV2 after SetValue");

    parcKeyValue_SetValue(data->nullValue, data->value1);
    assertTrue(parcBuffer_Equals(parcKeyValue_GetValue(data->testKV1),
                                 parcKeyValue_GetValue(data->nullValue)),
               "Expect kv values to be equal after SetValue");
}

LONGBOW_TEST_CASE(Global, parcKeyValue_Copy)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCKeyValue *kv = parcKeyValue_Copy(data->testKV1);
    assertTrue(parcKeyValue_Equals(kv, data->testKV1),
               "Expect key-value copy to be equal to original key-value");

    parcKeyValue_Release(&kv);

    // NULL value
    kv = parcKeyValue_Copy(data->nullValue);
    assertTrue(parcKeyValue_Equals(kv, data->nullValue),
               "Expect key-value copy to be equal to original key-value");

    parcKeyValue_Release(&kv);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_KeyValue);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
