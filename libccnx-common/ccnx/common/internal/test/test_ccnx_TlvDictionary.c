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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnx_TlvDictionary.c"
#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/common/internal/ccnx_ContentObjectInterface.h>
#include <ccnx/common/internal/ccnx_InterestInterface.h>

#include <LongBow/unit-test.h>

typedef struct test_data {
    CCNxTlvDictionary *dictionary;
    size_t fastArraySize;
    size_t listSize;
} TestData;

typedef enum {
    SchemaFree = 0,
    SchemaBuffer = 1,
    SchemaInteger = 2,
    SchemaIoVec = 3,
    SchemaJson = 4,
    SchemaName = 5,
    SchemaEnd = 6,
} TestSchema;

static CCNxCodecNetworkBufferIoVec *
createIoVec(void)
{
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
    ccnxCodecNetworkBuffer_Release(&netbuff);
    return vec;
}

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->fastArraySize = SchemaEnd + 2;
    data->listSize = SchemaEnd + 10;
    data->dictionary = ccnxTlvDictionary_Create(data->fastArraySize, data->listSize);

    // populate the know test vectors
    PARCBuffer *buffer = parcBuffer_Allocate(5);
    ccnxTlvDictionary_PutBuffer(data->dictionary, SchemaBuffer, buffer);
    parcBuffer_Release(&buffer);

    ccnxTlvDictionary_PutInteger(data->dictionary, SchemaInteger, 42);

    CCNxCodecNetworkBufferIoVec *vec = createIoVec();
    ccnxTlvDictionary_PutIoVec(data->dictionary, SchemaIoVec, vec);
    ccnxCodecNetworkBufferIoVec_Release(&vec);

    PARCJSON *json = parcJSON_ParseString("{\"KEY\": \"VALUE\"}");
    ccnxTlvDictionary_PutJson(data->dictionary, SchemaJson, json);
    parcJSON_Release(&json);

    CCNxName *name = ccnxName_CreateFromCString("lci:/great/gatsby");
    ccnxTlvDictionary_PutName(data->dictionary, SchemaName, name);
    ccnxName_Release(&name);

    return data;
}

static void
_commonTeardown(TestData *data)
{
    ccnxTlvDictionary_Release(&data->dictionary);
    parcMemory_Deallocate((void **) &data);
}

// =============================================================

LONGBOW_TEST_RUNNER(rta_TlvDictionary)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(KnownKeys);
    LONGBOW_RUN_TEST_FIXTURE(UnknownKeys);
    LONGBOW_RUN_TEST_FIXTURE(Local);

    // type specific tests
    LONGBOW_RUN_TEST_FIXTURE(Buffer);
    LONGBOW_RUN_TEST_FIXTURE(Integer);
    LONGBOW_RUN_TEST_FIXTURE(IoVec);
    LONGBOW_RUN_TEST_FIXTURE(Json);
    LONGBOW_RUN_TEST_FIXTURE(Name);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_TlvDictionary)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_TlvDictionary)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// =============================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_Performance);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_Release);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_ContentObject);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_Interest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_Control);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_InterestReturn);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_SetGetMessageTypeImplementation);


    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvDictionary_ShallowCopy);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_Performance)
{
    int reps = 100000;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    for (int i = 0; i < reps; i++) {
        CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(10, 20);
        ccnxTlvDictionary_Release(&dictionary);
    }
    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &t1);
    double seconds = t1.tv_sec + t1.tv_usec * 1E-6;

    printf("time %.6f seconds, tps = %.2f\n", seconds, (double) reps / seconds);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_Acquire)
{
    CCNxTlvDictionary *first = ccnxTlvDictionary_Create(10, 20);
    CCNxTlvDictionary *second = ccnxTlvDictionary_Acquire(first);

    assertTrue(parcObject_GetReferenceCount(first) == 2, "Wrong ref count, got %" PRIu64 " expected %u", parcObject_GetReferenceCount(first), 2);

    ccnxTlvDictionary_Release(&second);
    ccnxTlvDictionary_Release(&first);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_Create)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(20, 30);
    assertNotNull(dictionary, "Got null dictionary from Create");
    assertNotNull(dictionary->directArray, "DirectArray is null");
    assertNotNull(dictionary->fixedListHeads, "fixedListHeads is null");

    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_Release)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(1, 1);
    ccnxTlvDictionary_Release(&dictionary);
    assertNull(dictionary, "Release did not null argument");
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_ContentObject)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(1, 1);
    ccnxTlvDictionary_SetMessageType_ContentObject(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    assertTrue(ccnxTlvDictionary_IsContentObject(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsControl(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsInterest(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsInterestReturn(dictionary), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(dictionary) == CCNxTlvDictionary_SchemaVersion_V1, "Wrong schema type");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_Interest)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(1, 1);
    ccnxTlvDictionary_SetMessageType_Interest(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    assertFalse(ccnxTlvDictionary_IsContentObject(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsControl(dictionary), "Wrong message type");
    assertTrue(ccnxTlvDictionary_IsInterest(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsInterestReturn(dictionary), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(dictionary) == CCNxTlvDictionary_SchemaVersion_V1, "Wrong schema type");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_Control)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(1, 1);
    ccnxTlvDictionary_SetMessageType_Control(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    assertFalse(ccnxTlvDictionary_IsContentObject(dictionary), "Wrong message type");
    assertTrue(ccnxTlvDictionary_IsControl(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsInterest(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsInterestReturn(dictionary), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(dictionary) == CCNxTlvDictionary_SchemaVersion_V1, "Wrong schema type");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_SetMessageType_InterestReturn)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(1, 1);
    ccnxTlvDictionary_SetMessageType_InterestReturn(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    assertFalse(ccnxTlvDictionary_IsContentObject(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsControl(dictionary), "Wrong message type");
    assertFalse(ccnxTlvDictionary_IsInterest(dictionary), "Wrong message type");
    assertTrue(ccnxTlvDictionary_IsInterestReturn(dictionary), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(dictionary) == CCNxTlvDictionary_SchemaVersion_V1, "Wrong schema type");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_SetGetMessageTypeImplementation)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(1, 1);
    void *impl = ccnxTlvDictionary_GetMessageInterface(dictionary);

    assertNull(impl, "Expected a NULL implementation by default");

    ccnxTlvDictionary_SetMessageInterface(dictionary, &CCNxContentObjectFacadeV1_Implementation);
    assertTrue(ccnxTlvDictionary_GetMessageInterface(dictionary) == &CCNxContentObjectFacadeV1_Implementation,
               "Expected CCNxContentObjectFacadeV1_Implementation");

    ccnxTlvDictionary_SetMessageInterface(dictionary, &CCNxContentObjectFacadeV1_Implementation);
    assertTrue(ccnxTlvDictionary_GetMessageInterface(dictionary) == &CCNxContentObjectFacadeV1_Implementation,
               "Expected CCNxContentObjectFacadeV1_Implementation");

    ccnxTlvDictionary_SetMessageInterface(dictionary, &CCNxInterestFacadeV1_Implementation);
    assertTrue(ccnxTlvDictionary_GetMessageInterface(dictionary) == &CCNxInterestFacadeV1_Implementation,
               "Expected CCNxInterestFacadeV1_Implementation");

    ccnxTlvDictionary_SetMessageInterface(dictionary, &CCNxInterestFacadeV1_Implementation);
    assertTrue(ccnxTlvDictionary_GetMessageInterface(dictionary) == &CCNxInterestFacadeV1_Implementation,
               "Expected CCNxInterestFacadeV1_Implementation");

    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_Equals)
{
    CCNxTlvDictionary *a = ccnxTlvDictionary_Create(1, 1);
    CCNxTlvDictionary *b = ccnxTlvDictionary_Create(1, 1);
    CCNxTlvDictionary *c = ccnxTlvDictionary_Create(1, 1);

    ccnxTlvDictionary_SetMessageType_Interest(a, CCNxTlvDictionary_SchemaVersion_V1);
    ccnxTlvDictionary_SetMessageType_Interest(b, CCNxTlvDictionary_SchemaVersion_V1);
    ccnxTlvDictionary_SetMessageType_Interest(c, CCNxTlvDictionary_SchemaVersion_V1);

    CCNxTlvDictionary *diffArraySize = ccnxTlvDictionary_Create(2, 1);
    CCNxTlvDictionary *diffListSize = ccnxTlvDictionary_Create(1, 2);
    CCNxTlvDictionary *diffType = ccnxTlvDictionary_Create(1, 1);
    ccnxTlvDictionary_SetMessageType_Control(diffType, CCNxTlvDictionary_SchemaVersion_V1);

    CCNxTlvDictionary *diffType2 = ccnxTlvDictionary_Create(1, 1);
    ccnxTlvDictionary_SetMessageType_ContentObject(diffType2, CCNxTlvDictionary_SchemaVersion_V1);

    assertEqualsContract(ccnxTlvDictionary_Equals, a, b, c, diffArraySize, diffListSize, diffType, diffType2, NULL);

    ccnxTlvDictionary_Release(&diffType2);
    ccnxTlvDictionary_Release(&diffType);
    ccnxTlvDictionary_Release(&diffListSize);
    ccnxTlvDictionary_Release(&diffArraySize);
    ccnxTlvDictionary_Release(&c);
    ccnxTlvDictionary_Release(&b);
    ccnxTlvDictionary_Release(&a);
}

LONGBOW_TEST_CASE(Global, ccnxTlvDictionary_ShallowCopy)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxTlvDictionary *a = data->dictionary;

    PARCBuffer *buffer = parcBuffer_WrapCString("Some Stuff");
    ccnxTlvDictionary_PutListBuffer(a, SchemaEnd, 23, buffer);
    parcBuffer_Release(&buffer);

    CCNxTlvDictionary *b = ccnxTlvDictionary_ShallowCopy(a);

    assertTrue(ccnxTlvDictionary_Equals(a, b), "Expected Dictionaries to be Equal after ShallowCopy");

    ccnxTlvDictionary_Release(&b);
}

// ================================================================

LONGBOW_TEST_FIXTURE(KnownKeys)
{
    LONGBOW_RUN_TEST_CASE(KnownKeys, ccnxTlvDictionary_Get_Exists);
    LONGBOW_RUN_TEST_CASE(KnownKeys, ccnxTlvDictionary_Get_NotExists);
    LONGBOW_RUN_TEST_CASE(KnownKeys, ccnxTlvDictionary_Put_Unique);
    LONGBOW_RUN_TEST_CASE(KnownKeys, ccnxTlvDictionary_Put_Duplicate);

    LONGBOW_RUN_TEST_CASE(KnownKeys, ccnxTlvDictionary_PutList_Unique);
    LONGBOW_RUN_TEST_CASE(KnownKeys, ccnxTlvDictionary_PutList_Duplicate);
}

LONGBOW_TEST_FIXTURE_SETUP(KnownKeys)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(KnownKeys)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(KnownKeys, ccnxTlvDictionary_Get_Exists)
{
    uint32_t key = SchemaEnd;

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);

    ccnxTlvDictionary_PutBuffer(data->dictionary, key, buffer);
    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(data->dictionary, key);
    assertTrue(test == buffer, "Get value wrong, got %p expected %p", (void *) test, (void *) buffer);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(KnownKeys, ccnxTlvDictionary_Get_NotExists)
{
    uint32_t key = SchemaEnd;
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCBuffer *buffer = parcBuffer_Allocate(1);
    ccnxTlvDictionary_PutBuffer(data->dictionary, key, buffer);
    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(data->dictionary, key + 1);
    assertNull(test, "Get for missing key should return null");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(KnownKeys, ccnxTlvDictionary_Put_Unique)
{
    uint32_t key = SchemaEnd;
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    bool success = ccnxTlvDictionary_PutBuffer(data->dictionary, key, buffer);
    assertTrue(success, "Put returned false adding a unique key");
    parcBuffer_Release(&buffer);

    // one extra test particular to it being in the fast array
    assertTrue(data->dictionary->directArray[key].entryType == ENTRY_BUFFER, "Not buffer type, got %d", data->dictionary->directArray[key].entryType);
    assertNotNull(data->dictionary->directArray[key]._entry.buffer, "They fast array entry for key is null");
}

LONGBOW_TEST_CASE(KnownKeys, ccnxTlvDictionary_Put_Duplicate)
{
    uint32_t key = SchemaEnd;

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    ccnxTlvDictionary_PutBuffer(data->dictionary, key, buffer);
    bool success = ccnxTlvDictionary_PutBuffer(data->dictionary, key, buffer);
    assertFalse(success, "Put returned true adding a duplicate key");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(KnownKeys, ccnxTlvDictionary_PutList_Unique)
{
    uint32_t listKey = SchemaEnd;
    uint32_t bufferKey = 1000;

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    bool success = ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, bufferKey, buffer);
    assertTrue(success, "Put returned false adding a unique key");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(KnownKeys, ccnxTlvDictionary_PutList_Duplicate)
{
    uint32_t listKey = SchemaEnd;
    uint32_t bufferKey = 1000;

    // its ok to have duplicates of the custom keys
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, bufferKey, buffer);
    bool success = ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, bufferKey, buffer);
    assertTrue(success, "Put returned false adding a duplicate key to list");
    parcBuffer_Release(&buffer);
}

// ================================================================

LONGBOW_TEST_FIXTURE(UnknownKeys)
{
    LONGBOW_RUN_TEST_CASE(UnknownKeys, ccnxTlvDictionary_PutList_Unique);
    LONGBOW_RUN_TEST_CASE(UnknownKeys, ccnxTlvDictionary_PutList_Duplicate);

    LONGBOW_RUN_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListGetByPosition);
    LONGBOW_RUN_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListGetByType);
    LONGBOW_RUN_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListSize);
    LONGBOW_RUN_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListEquals);
}

LONGBOW_TEST_FIXTURE_SETUP(UnknownKeys)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(UnknownKeys)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(UnknownKeys, ccnxTlvDictionary_PutList_Unique)
{
    uint32_t listKey = FIXED_LIST_LENGTH + 1;
    uint32_t bufferKey = 1000;

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    bool success = ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, bufferKey, buffer);
    assertTrue(success, "Put returned false adding a unique key");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(UnknownKeys, ccnxTlvDictionary_PutList_Duplicate)
{
    uint32_t listKey = FIXED_LIST_LENGTH + 1;
    uint32_t bufferKey = 1000;

    // its ok to have duplicates of the custom keys
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, bufferKey, buffer);
    bool success = ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, bufferKey, buffer);
    assertTrue(success, "Put returned false adding a duplicate key to list");
    parcBuffer_Release(&buffer);
}

/*
 * Add 3 items to list then make sure we can retrieve the 2nd
 */
LONGBOW_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListGetByPosition)
{
    uint32_t listKey = SchemaEnd;
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *a = parcBuffer_Allocate(1);
    PARCBuffer *b = parcBuffer_Allocate(1);
    PARCBuffer *c = parcBuffer_Allocate(1);

    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1000, a);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1001, b);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1002, c);

    PARCBuffer *test = NULL;
    uint32_t testkey = 0;
    ccnxTlvDictionary_ListGetByPosition(data->dictionary, listKey, 1, &test, &testkey);

    assertTrue(testkey == 1001, "Wrong key, expected %u got %u", 1001, testkey);
    assertTrue(test == b, "Wrong buffer, expected %p got %p", (void *) b, (void *) test);

    parcBuffer_Release(&a);
    parcBuffer_Release(&b);
    parcBuffer_Release(&c);
}

/*
 * Add 3 items to list then make sure we can retrieve the 2nd
 */
LONGBOW_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListGetByType)
{
    uint32_t listKey = SchemaEnd;
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *a = parcBuffer_Allocate(1);
    PARCBuffer *b = parcBuffer_Allocate(1);
    PARCBuffer *c = parcBuffer_Allocate(1);

    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1000, a);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1001, b);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1002, c);

    PARCBuffer *test = ccnxTlvDictionary_ListGetByType(data->dictionary, listKey, 1001);
    assertTrue(test == b, "Wrong buffer, expected %p got %p", (void *) b, (void *) test);

    parcBuffer_Release(&a);
    parcBuffer_Release(&b);
    parcBuffer_Release(&c);
}

LONGBOW_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListSize)
{
    uint32_t listKey = SchemaEnd;
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *a = parcBuffer_Allocate(1);
    PARCBuffer *b = parcBuffer_Allocate(1);
    PARCBuffer *c = parcBuffer_Allocate(1);

    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1000, a);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1001, b);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, listKey, 1002, c);

    size_t length = ccnxTlvDictionary_ListSize(data->dictionary, listKey);
    assertTrue(length == 3, "Wrong length, expected %u got %zu", 3, length);

    parcBuffer_Release(&a);
    parcBuffer_Release(&b);
    parcBuffer_Release(&c);
}

LONGBOW_TEST_CASE(UnknownKeys, ccnxTlvDictionary_ListEquals)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *a = parcBuffer_Allocate(1);
    PARCBuffer *b = parcBuffer_Allocate(1);
    PARCBuffer *c = parcBuffer_Allocate(1);

    ccnxTlvDictionary_PutListBuffer(data->dictionary, 6, 1000, a);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, 6, 1001, b);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, 6, 1002, c);

    ccnxTlvDictionary_PutListBuffer(data->dictionary, 7, 1000, a);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, 7, 1001, b);
    ccnxTlvDictionary_PutListBuffer(data->dictionary, 7, 1002, c);

    bool equals = _ccnxTlvDictionary_ListEquals(_getListHead(data->dictionary, 6), _getListHead(data->dictionary, 7));
    assertTrue(equals, "Lists should be equal");

    parcBuffer_Release(&a);
    parcBuffer_Release(&b);
    parcBuffer_Release(&c);
}



// =============================================================

LONGBOW_TEST_FIXTURE(Buffer)
{
    LONGBOW_RUN_TEST_CASE(Buffer, ccnxTlvDictionary_GetBuffer_Exists);
    LONGBOW_RUN_TEST_CASE(Buffer, ccnxTlvDictionary_GetBuffer_Missing);
    LONGBOW_RUN_TEST_CASE(Buffer, ccnxTlvDictionary_PutBuffer_OK);
    LONGBOW_RUN_TEST_CASE(Buffer, ccnxTlvDictionary_PutBuffer_Duplicate);
    LONGBOW_RUN_TEST_CASE(Buffer, ccnxTlvDictionary_IsValueBuffer_True);
    LONGBOW_RUN_TEST_CASE(Buffer, ccnxTlvDictionary_IsValueBuffer_False);
}

LONGBOW_TEST_FIXTURE_SETUP(Buffer)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Buffer)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Buffer, ccnxTlvDictionary_GetBuffer_Exists)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(data->dictionary, SchemaBuffer);
    assertNotNull(test, "Got null buffer from key that hsould be buffer");
}

LONGBOW_TEST_CASE(Buffer, ccnxTlvDictionary_GetBuffer_Missing)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxTlvDictionary_GetBuffer(data->dictionary, SchemaFree);
}

LONGBOW_TEST_CASE(Buffer, ccnxTlvDictionary_PutBuffer_OK)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    bool success = ccnxTlvDictionary_PutBuffer(data->dictionary, SchemaFree, buffer);
    assertTrue(success, "Did not put buffer in to available slot");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Buffer, ccnxTlvDictionary_PutBuffer_Duplicate)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    bool success = ccnxTlvDictionary_PutBuffer(data->dictionary, SchemaBuffer, buffer);
    assertFalse(success, "Should have failed putting duplicate");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Buffer, ccnxTlvDictionary_IsValueBuffer_True)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueBuffer(data->dictionary, SchemaBuffer);
    assertTrue(success, "Should have succeeded on a buffer key");
}

LONGBOW_TEST_CASE(Buffer, ccnxTlvDictionary_IsValueBuffer_False)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueBuffer(data->dictionary, SchemaInteger);
    assertFalse(success, "Should have failed on a non-buffer");
}

// =============================================================

LONGBOW_TEST_FIXTURE(Integer)
{
    LONGBOW_RUN_TEST_CASE(Integer, ccnxTlvDictionary_GetInteger_Exists);
    LONGBOW_RUN_TEST_CASE(Integer, ccnxTlvDictionary_GetInteger_Missing);
    LONGBOW_RUN_TEST_CASE(Integer, ccnxTlvDictionary_PutInteger_OK);
    LONGBOW_RUN_TEST_CASE(Integer, ccnxTlvDictionary_PutInteger_Duplicate);
    LONGBOW_RUN_TEST_CASE(Integer, ccnxTlvDictionary_PutInteger_OverBuffer);
    LONGBOW_RUN_TEST_CASE(Integer, ccnxTlvDictionary_IsValueInteger_True);
    LONGBOW_RUN_TEST_CASE(Integer, ccnxTlvDictionary_IsValueInteger_False);
}

LONGBOW_TEST_FIXTURE_SETUP(Integer)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Integer)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Integer, ccnxTlvDictionary_GetInteger_Exists)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint64_t test = ccnxTlvDictionary_GetInteger(data->dictionary, SchemaInteger);
    assertTrue(test == 42, "Got wrong integer, got %" PRIu64 " expected 42", test);
}

LONGBOW_TEST_CASE_EXPECTS(Integer, ccnxTlvDictionary_GetInteger_Missing, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxTlvDictionary_GetInteger(data->dictionary, SchemaBuffer);
}

LONGBOW_TEST_CASE(Integer, ccnxTlvDictionary_PutInteger_OK)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_PutInteger(data->dictionary, SchemaFree, 69);
    assertTrue(success, "Did not put integer in to available slot");
}

LONGBOW_TEST_CASE(Integer, ccnxTlvDictionary_PutInteger_Duplicate)
{
    // we allow replacing integer
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_PutInteger(data->dictionary, SchemaInteger, 69);
    assertTrue(success, "Should have succeeded putting duplicate");
}

LONGBOW_TEST_CASE(Integer, ccnxTlvDictionary_PutInteger_OverBuffer)
{
    // This will fail, cannot change a buffer to an integer
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_PutInteger(data->dictionary, SchemaBuffer, 69);
    assertFalse(success, "Should not be able to change a buffer to an integer");
}

LONGBOW_TEST_CASE(Integer, ccnxTlvDictionary_IsValueInteger_True)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueInteger(data->dictionary, SchemaInteger);
    assertTrue(success, "Should have succeeded on a integer key");
}

LONGBOW_TEST_CASE(Integer, ccnxTlvDictionary_IsValueInteger_False)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueInteger(data->dictionary, SchemaBuffer);
    assertFalse(success, "Should have failed on a non-integer");
}

// =============================================================

LONGBOW_TEST_FIXTURE(IoVec)
{
    LONGBOW_RUN_TEST_CASE(IoVec, ccnxTlvDictionary_GetIoVec_Exists);
    LONGBOW_RUN_TEST_CASE(IoVec, ccnxTlvDictionary_GetIoVec_Missing);
    LONGBOW_RUN_TEST_CASE(IoVec, ccnxTlvDictionary_PutIoVec_OK);
    LONGBOW_RUN_TEST_CASE(IoVec, ccnxTlvDictionary_PutIoVec_Duplicate);
    LONGBOW_RUN_TEST_CASE(IoVec, ccnxTlvDictionary_IsValueIoVec_True);
    LONGBOW_RUN_TEST_CASE(IoVec, ccnxTlvDictionary_IsValueIoVec_False);
}

LONGBOW_TEST_FIXTURE_SETUP(IoVec)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(IoVec)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(IoVec, ccnxTlvDictionary_GetIoVec_Exists)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxCodecNetworkBufferIoVec *test = ccnxTlvDictionary_GetIoVec(data->dictionary, SchemaIoVec);
    assertNotNull(test, "Got null buffer from key that hsould be buffer");
}

LONGBOW_TEST_CASE(IoVec, ccnxTlvDictionary_GetIoVec_Missing)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxTlvDictionary_GetIoVec(data->dictionary, SchemaFree);
}

LONGBOW_TEST_CASE(IoVec, ccnxTlvDictionary_PutIoVec_OK)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxCodecNetworkBufferIoVec *vec = createIoVec();
    bool success = ccnxTlvDictionary_PutIoVec(data->dictionary, SchemaFree, vec);
    assertTrue(success, "Did not put vec in to available slot");
    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

LONGBOW_TEST_CASE(IoVec, ccnxTlvDictionary_PutIoVec_Duplicate)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxCodecNetworkBufferIoVec *vec = createIoVec();
    bool success = ccnxTlvDictionary_PutIoVec(data->dictionary, SchemaIoVec, vec);
    assertFalse(success, "Should have failed putting duplicate");
    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

LONGBOW_TEST_CASE(IoVec, ccnxTlvDictionary_IsValueIoVec_True)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueIoVec(data->dictionary, SchemaIoVec);
    assertTrue(success, "Should have succeeded on a vec key");
}

LONGBOW_TEST_CASE(IoVec, ccnxTlvDictionary_IsValueIoVec_False)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueIoVec(data->dictionary, SchemaInteger);
    assertFalse(success, "Should have failed on a non-vec");
}

// =============================================================

LONGBOW_TEST_FIXTURE(Json)
{
    LONGBOW_RUN_TEST_CASE(Json, ccnxTlvDictionary_GetJson_Exists);
    LONGBOW_RUN_TEST_CASE(Json, ccnxTlvDictionary_GetJson_Missing);
    LONGBOW_RUN_TEST_CASE(Json, ccnxTlvDictionary_PutJson_OK);
    LONGBOW_RUN_TEST_CASE(Json, ccnxTlvDictionary_PutJson_Duplicate);
    LONGBOW_RUN_TEST_CASE(Json, ccnxTlvDictionary_IsValueJson_True);
    LONGBOW_RUN_TEST_CASE(Json, ccnxTlvDictionary_IsValueJson_False);
}

LONGBOW_TEST_FIXTURE_SETUP(Json)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Json)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Json, ccnxTlvDictionary_GetJson_Exists)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCJSON *test = ccnxTlvDictionary_GetJson(data->dictionary, SchemaJson);
    assertNotNull(test, "Got null json from key that should be json");
}

LONGBOW_TEST_CASE(Json, ccnxTlvDictionary_GetJson_Missing)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCJSON *test = ccnxTlvDictionary_GetJson(data->dictionary, SchemaFree);
    assertNull(test, "Should have gotten null for non-json key");
}

LONGBOW_TEST_CASE(Json, ccnxTlvDictionary_PutJson_OK)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCJSON *json = parcJSON_Create();
    bool success = ccnxTlvDictionary_PutJson(data->dictionary, SchemaFree, json);
    assertTrue(success, "Did not put buffer in to available slot");
    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(Json, ccnxTlvDictionary_PutJson_Duplicate)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCJSON *json = parcJSON_Create();
    bool success = ccnxTlvDictionary_PutJson(data->dictionary, SchemaJson, json);
    assertFalse(success, "Should have failed putting duplicate");
    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(Json, ccnxTlvDictionary_IsValueJson_True)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueJson(data->dictionary, SchemaJson);
    assertTrue(success, "Should have succeeded on a json key");
}

LONGBOW_TEST_CASE(Json, ccnxTlvDictionary_IsValueJson_False)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueJson(data->dictionary, SchemaInteger);
    assertFalse(success, "Should have failed on a non-json");
}

// =============================================================

LONGBOW_TEST_FIXTURE(Name)
{
    LONGBOW_RUN_TEST_CASE(Name, ccnxTlvDictionary_GetName_Exists);
    LONGBOW_RUN_TEST_CASE(Name, ccnxTlvDictionary_GetName_Missing);
    LONGBOW_RUN_TEST_CASE(Name, ccnxTlvDictionary_PutName_OK);
    LONGBOW_RUN_TEST_CASE(Name, ccnxTlvDictionary_PutName_Duplicate);
    LONGBOW_RUN_TEST_CASE(Name, ccnxTlvDictionary_IsValueName_True);
    LONGBOW_RUN_TEST_CASE(Name, ccnxTlvDictionary_IsValueName_False);
}

LONGBOW_TEST_FIXTURE_SETUP(Name)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Name)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Name, ccnxTlvDictionary_GetName_Exists)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxName *test = ccnxTlvDictionary_GetName(data->dictionary, SchemaName);
    assertNotNull(test, "Got null json from key that should be name");
}

LONGBOW_TEST_CASE(Name, ccnxTlvDictionary_GetName_Missing)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxName *test = ccnxTlvDictionary_GetName(data->dictionary, SchemaFree);
    assertNull(test, "Should have gotten null for non-name key");
}

LONGBOW_TEST_CASE(Name, ccnxTlvDictionary_PutName_OK)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxName *name = ccnxName_Create();
    bool success = ccnxTlvDictionary_PutName(data->dictionary, SchemaFree, name);
    assertTrue(success, "Did not put name in to available slot");
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Name, ccnxTlvDictionary_PutName_Duplicate)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxName *name = ccnxName_Create();
    bool success = ccnxTlvDictionary_PutName(data->dictionary, SchemaName, name);
    assertFalse(success, "Should have failed putting duplicate");
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Name, ccnxTlvDictionary_IsValueName_True)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueName(data->dictionary, SchemaName);
    assertTrue(success, "Should have succeeded on a json key");
}

LONGBOW_TEST_CASE(Name, ccnxTlvDictionary_IsValueName_False)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxTlvDictionary_IsValueName(data->dictionary, SchemaInteger);
    assertFalse(success, "Should have failed on a non-json");
}

// =============================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _rtaTlvEntry_Equals_Unset);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTlvEntry_Equals_Buffer);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTlvEntry_Equals_Integer);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTlvEntry_Equals_IoVec);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTlvEntry_Equals_Json);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTlvEntry_Equals_Name);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static void
assertRtaTlvEntryEquals(int length, _CCNxTlvDictionaryEntry array[length])
{
    assertTrue(length == 5, "Must provide 5 array elements");
    assertEqualsContract((bool (*)(void *, void *))_ccnxTlvDictionaryEntry_Equals, &array[0], &array[1], &array[2], &array[3], &array[4], NULL);
}

LONGBOW_TEST_CASE(Local, _rtaTlvEntry_Equals_Unset)
{
    _CCNxTlvDictionaryEntry array[5];
    memset(&array, 0, sizeof(array));

    array[3].entryType = ENTRY_JSON;
    array[4].entryType = ENTRY_BUFFER;
    assertRtaTlvEntryEquals(5, array);
}

LONGBOW_TEST_CASE(Local, _rtaTlvEntry_Equals_Buffer)
{
    char apple[] = "apple";
    char bananna[] = "bannana";

    _CCNxTlvDictionaryEntry array[5];
    memset(&array, 0, sizeof(array));

    for (int i = 0; i < 3; i++) {
        array[i].entryType = ENTRY_BUFFER;
        array[i]._entry.buffer = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(20), sizeof(apple), (uint8_t *) apple));
    }

    array[3].entryType = ENTRY_JSON;
    array[4].entryType = ENTRY_BUFFER;
    array[4]._entry.buffer = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(20), sizeof(bananna), (uint8_t *) bananna));
    assertRtaTlvEntryEquals(5, array);

    for (int i = 0; i < 5; i++) {
        if (array[i]._entry.buffer) {
            parcBuffer_Release(&array[i]._entry.buffer);
        }
    }
}

LONGBOW_TEST_CASE(Local, _rtaTlvEntry_Equals_Integer)
{
    _CCNxTlvDictionaryEntry array[5];
    memset(&array, 0, sizeof(array));

    for (int i = 0; i < 3; i++) {
        array[i].entryType = ENTRY_INTEGER;
        array[i]._entry.integer = 13;
    }

    array[3].entryType = ENTRY_JSON;
    array[4].entryType = ENTRY_INTEGER;
    array[4]._entry.integer = 99;

    assertRtaTlvEntryEquals(5, array);
}

LONGBOW_TEST_CASE(Local, _rtaTlvEntry_Equals_IoVec)
{
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
    ccnxCodecNetworkBuffer_PutUint8(netbuff, 0);
    CCNxCodecNetworkBufferIoVec *unequal = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
    ccnxCodecNetworkBuffer_Release(&netbuff);

    _CCNxTlvDictionaryEntry array[5];
    memset(&array, 0, sizeof(array));

    for (int i = 0; i < 3; i++) {
        array[i].entryType = ENTRY_IOVEC;
        array[i]._entry.vec = createIoVec();
    }

    array[3].entryType = ENTRY_JSON;
    array[4].entryType = ENTRY_IOVEC;
    array[4]._entry.vec = unequal;
    assertRtaTlvEntryEquals(5, array);

    for (int i = 0; i < 5; i++) {
        if (array[i]._entry.vec) {
            ccnxCodecNetworkBufferIoVec_Release(&array[i]._entry.vec);
        }
    }
}

LONGBOW_TEST_CASE(Local, _rtaTlvEntry_Equals_Json)
{
    char apple[] = "{\"apple\": 0}";
    char bananna[] = "{\"bannana\": 1}";

    _CCNxTlvDictionaryEntry array[5];
    memset(&array, 0, sizeof(array));

    for (int i = 0; i < 3; i++) {
        array[i].entryType = ENTRY_JSON;
        array[i]._entry.json = parcJSON_ParseString(apple);
    }

    array[3].entryType = ENTRY_JSON;
    array[4].entryType = ENTRY_JSON;
    array[4]._entry.json = parcJSON_ParseString(bananna);
    assertRtaTlvEntryEquals(5, array);

    for (int i = 0; i < 5; i++) {
        if (array[i]._entry.json) {
            parcJSON_Release(&array[i]._entry.json);
        }
    }
}

LONGBOW_TEST_CASE(Local, _rtaTlvEntry_Equals_Name)
{
    char apple[] = "lci:/apple";
    char bananna[] = "lci:/bannana";

    _CCNxTlvDictionaryEntry array[5];
    memset(&array, 0, sizeof(array));

    for (int i = 0; i < 3; i++) {
        array[i].entryType = ENTRY_NAME;
        array[i]._entry.name = ccnxName_CreateFromCString(apple);
    }

    array[3].entryType = ENTRY_JSON;
    array[4].entryType = ENTRY_NAME;
    array[4]._entry.name = ccnxName_CreateFromCString(bananna);
    assertRtaTlvEntryEquals(5, array);

    for (int i = 0; i < 5; i++) {
        if (array[i]._entry.name) {
            ccnxName_Release(&array[i]._entry.name);
        }
    }
}

// =============================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_TlvDictionary);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
