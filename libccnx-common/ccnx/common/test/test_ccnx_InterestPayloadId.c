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
#include <config.h>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <stdio.h>

#include "../ccnx_InterestPayloadId.c"

static const PARCMemoryInterface *_originalMemoryProvider;

LONGBOW_TEST_RUNNER(ccnx_InterestPayloadId)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Error);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_InterestPayloadId)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_InterestPayloadId)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_CreateWithAppDefinedType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_CreateAsCryptoHash);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_CreateFromSegmentInName);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_CreateFromSegmentInName_NotFound);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_GetNameSegment);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_GetValue);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_GetType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_ToString);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_Copy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_NotEquals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_Compare);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestPayloadId_HashCode);
}

typedef struct {
    uint8_t type;
    PARCBuffer *value;
} TestData;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->value = parcBuffer_WrapCString("123456789abcdef");
    data->type = 42 + CCNxInterestPayloadId_TypeCode_App;
    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    parcBuffer_Release(&data->value);
    parcMemory_Deallocate((void **) &data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    parcMemory_SetInterface(_originalMemoryProvider);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_CreateWithAppDefinedType)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    assertNotNull(ipId, "Expect a non-NULL result from Create");
    ccnxInterestPayloadId_AssertValid(ipId);

    parcBuffer_Release(&value);

    assertTrue(ccnxInterestPayloadId_IsValid(ipId), "Expected a valid CCNxInteresPayloadId.");
    ccnxInterestPayloadId_Release(&ipId);
}


LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_CreateAsCryptoHash)
{
    PARCBuffer *value = parcBuffer_WrapCString("123456789abcdef");
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_CreateAsSHA256Hash(value);
    assertNotNull(ipId, "Expect a non-NULL result from CreateByHash");
    ccnxInterestPayloadId_AssertValid(ipId);

    parcBuffer_Release(&value);

    assertTrue(ccnxInterestPayloadId_IsValid(ipId), "Expected a valid CCNxInteresPayloadId.");
    ccnxInterestPayloadId_Release(&ipId);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_CreateFromSegmentInName)
{
    PARCBuffer *value = parcBuffer_WrapCString("123456789abcdef");
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_CreateAsSHA256Hash(value);
    parcBuffer_Release(&value);
    assertNotNull(ipId, "Expect a non-NULL result from CreateByHash");
    ccnxInterestPayloadId_AssertValid(ipId);
    assertTrue(ccnxInterestPayloadId_IsValid(ipId), "Expected a valid CCNxInteresPayloadId.");

    CCNxName *name = ccnxName_CreateFromCString("lci:/segment1/segment2/segment3");
    ccnxName_Append(name, ccnxInterestPayloadId_GetNameSegment(ipId));

    CCNxInterestPayloadId *result = ccnxInterestPayloadId_CreateFromSegmentInName(name);
    ccnxName_Release(&name);

    assertNotNull(result, "Should have found a payload ID");
    ccnxInterestPayloadId_AssertValid(result);

    assertTrue(ccnxInterestPayloadId_Equals(result, ipId),
               "Expected source and result Interest Payload IDs to be equal");

    ccnxInterestPayloadId_Release(&ipId);
    ccnxInterestPayloadId_Release(&result);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_CreateFromSegmentInName_NotFound)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/segment1/segment2/segment3");
    CCNxInterestPayloadId *result = ccnxInterestPayloadId_CreateFromSegmentInName(name);
    ccnxName_Release(&name);

    assertNull(result, "Should have not found a payload ID");
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_Acquire)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    parcBuffer_Release(&value);
    ccnxInterestPayloadId_AssertValid(ipId);

    CCNxInterestPayloadId *ipIdAcq = ccnxInterestPayloadId_Acquire(ipId);
    ccnxInterestPayloadId_AssertValid(ipIdAcq);

    ccnxInterestPayloadId_Release(&ipId);
    ccnxInterestPayloadId_Release(&ipIdAcq);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_GetValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    ccnxInterestPayloadId_AssertValid(ipId);

    assertTrue(parcBuffer_Equals(ccnxInterestPayloadId_GetValue(ipId), value),
               "Expect GetId to produce the correct result");

    parcBuffer_Release(&value);
    ccnxInterestPayloadId_Release(&ipId);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_GetType)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    ccnxInterestPayloadId_AssertValid(ipId);

    assertTrue(ccnxInterestPayloadId_GetType(ipId) == type,
               "Expect GetId to produce the correct result");

    parcBuffer_Release(&value);
    ccnxInterestPayloadId_Release(&ipId);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_GetType_App)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    ccnxInterestPayloadId_AssertValid(ipId);

    assertTrue(parcBuffer_Equals(ccnxInterestPayloadId_GetValue(ipId), value),
               "Expect GetId to produce the correct result");

    parcBuffer_Release(&value);
    ccnxInterestPayloadId_Release(&ipId);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_HashCode)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId1 = ccnxInterestPayloadId_Create(value, type);
    uint32_t hashCode1 = ccnxInterestPayloadId_HashCode(ipId1);
    CCNxInterestPayloadId *ipId2 = ccnxInterestPayloadId_Create(value, type);
    uint32_t hashCode2 = ccnxInterestPayloadId_HashCode(ipId2);
    assertTrue(hashCode1 == hashCode2, "Expect hash codes of equal objects to be equal");
    parcBuffer_Release(&value);
    ccnxInterestPayloadId_Release(&ipId1);
    ccnxInterestPayloadId_Release(&ipId2);
}


LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_Equals)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_WrapCString("123456789abcdef");
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    ccnxInterestPayloadId_AssertValid(ipId);

    PARCBuffer *value2 = parcBuffer_WrapCString("123456789abcdef");
    CCNxInterestPayloadId *ipId2 = ccnxInterestPayloadId_Create(value2, type);
    ccnxInterestPayloadId_AssertValid(ipId2);

    assertTrue(ccnxInterestPayloadId_Equals(ipId, ipId2),
               "Expect InterestPayloadIds to be equal");

    CCNxInterestPayloadId *ipId3 = ccnxInterestPayloadId_Create(value, type);
    ccnxInterestPayloadId_AssertValid(ipId3);

    assertTrue(ccnxInterestPayloadId_Equals(ipId, ipId3),
               "Expect InterestPayloadIds to be equal");

    parcBuffer_Release(&value);
    parcBuffer_Release(&value2);
    ccnxInterestPayloadId_Release(&ipId);
    ccnxInterestPayloadId_Release(&ipId2);
    ccnxInterestPayloadId_Release(&ipId3);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_NotEquals)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_WrapCString("123456789abcdef");
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    ccnxInterestPayloadId_AssertValid(ipId);

    PARCBuffer *value2 = parcBuffer_WrapCString("123456789abcdex");
    CCNxInterestPayloadId *ipId2 = ccnxInterestPayloadId_Create(value2, type);
    ccnxInterestPayloadId_AssertValid(ipId2);

    assertTrue(!ccnxInterestPayloadId_Equals(ipId, ipId2),
               "Expect InterestPayloadIds to be equal");

    parcBuffer_Release(&value);
    parcBuffer_Release(&value2);
    ccnxInterestPayloadId_Release(&ipId);
    ccnxInterestPayloadId_Release(&ipId2);
}


LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_Compare)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value1 = parcBuffer_WrapCString("123456789abcdef");
    CCNxInterestPayloadId *ipId1 = ccnxInterestPayloadId_Create(value1, type);
    ccnxInterestPayloadId_AssertValid(ipId1);

    assertTrue(ccnxInterestPayloadId_Compare(ipId1, ipId1) == 0,
               "Expect compare result of 0 when comparing InterestPayloadId to itself");

    PARCBuffer *value1p = parcBuffer_WrapCString("123456789abcdef");
    CCNxInterestPayloadId *ipId1p = ccnxInterestPayloadId_Create(value1p, type);
    ccnxInterestPayloadId_AssertValid(ipId1p);

    assertTrue(ccnxInterestPayloadId_Compare(ipId1, ipId1p) == 0,
               "Expect compare result of 0 when comparing InterestPayloadIds with the same content");

    PARCBuffer *value2 = parcBuffer_WrapCString("123456789abcdex");
    CCNxInterestPayloadId *ipId2 = ccnxInterestPayloadId_Create(value2, type);
    ccnxInterestPayloadId_AssertValid(ipId2);

    assertTrue(ccnxInterestPayloadId_Compare(ipId2, ipId1) > 0,
               "Expect compare result > 0 when comparing InterestPayloadId2 to InterestPayloadId1");

    parcBuffer_Release(&value1);
    parcBuffer_Release(&value1p);
    parcBuffer_Release(&value2);

    ccnxInterestPayloadId_Release(&ipId1);
    ccnxInterestPayloadId_Release(&ipId1p);
    ccnxInterestPayloadId_Release(&ipId2);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_Copy)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value1 = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId1 = ccnxInterestPayloadId_Create(value1, type);
    parcBuffer_Release(&value1);
    ccnxInterestPayloadId_AssertValid(ipId1);

    CCNxInterestPayloadId *ipIdCopy = ccnxInterestPayloadId_Copy(ipId1);
    ccnxInterestPayloadId_AssertValid(ipIdCopy);
    assertTrue(ccnxInterestPayloadId_Equals(ipId1, ipIdCopy), "Expect original and copy InterestPayloadId to be equal");

    ccnxInterestPayloadId_Release(&ipId1);
    ccnxInterestPayloadId_Release(&ipIdCopy);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_IsValid)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    parcBuffer_Release(&value);

    assertTrue(ccnxInterestPayloadId_IsValid(ipId), "Expected a valid CCNxInteresPayloadId.");
    ccnxInterestPayloadId_Release(&ipId);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_ToString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    char *test = "123456789abcdef";
    PARCBuffer *value = parcBuffer_WrapCString(test);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);
    parcBuffer_Release(&value);

    char *result = ccnxInterestPayloadId_ToString(ipId);
    assertNotNull(result, "Expect a non-NULL result from ToString");
    ccnxInterestPayloadId_Release(&ipId);

    PARCBufferComposer *composer = parcBufferComposer_Allocate(10);
    parcBufferComposer_PutString(composer, CCNxNameLabel_InterestPayloadId);
    parcBufferComposer_PutChar(composer, '=');

    PARCBufferComposer *uriComposer = parcBufferComposer_Allocate(10);
    parcBufferComposer_PutChar(uriComposer, type);
    parcBufferComposer_PutString(uriComposer, test);
    PARCBuffer *producedBuffer = parcBufferComposer_ProduceBuffer(uriComposer);
    PARCURISegment *uriSegment = parcURISegment_CreateFromBuffer(producedBuffer);
    parcBuffer_Release(&producedBuffer);
    parcBufferComposer_Release(&uriComposer);
    parcURISegment_BuildString(uriSegment, composer);
    parcURISegment_Release(&uriSegment);
    char *expect = parcBufferComposer_ToString(composer);
    parcBufferComposer_Release(&composer);

    assertTrue(strcmp(expect, result) == 0, "Expect test and result strings to be the same.");

    parcMemory_Deallocate((void **) &result);
    parcMemory_Deallocate((void **) &expect);
}

LONGBOW_TEST_CASE(Global, ccnxInterestPayloadId_GetNameSegment)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t type = data->type;
    PARCBuffer *value = parcBuffer_Acquire(data->value);
    CCNxInterestPayloadId *ipId = ccnxInterestPayloadId_Create(value, type);

    CCNxNameSegment *segment = ccnxNameSegment_Acquire(ccnxInterestPayloadId_GetNameSegment(ipId));
    ccnxInterestPayloadId_Release(&ipId);

    PARCBufferComposer *composer = parcBufferComposer_Allocate(parcBuffer_Capacity(value) + 1);
    parcBufferComposer_PutUint8(composer, type);
    parcBufferComposer_PutBuffer(composer, value);
    PARCBuffer *testValue = parcBufferComposer_ProduceBuffer(composer);
    parcBufferComposer_Release(&composer);
    parcBuffer_Release(&value);
    CCNxNameSegment *testSegment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_PAYLOADID, testValue);
    parcBuffer_Release(&testValue);
    assertTrue(ccnxNameSegment_Equals(segment, testSegment), "Expect GetAsNameSegment result to match test NameSegment");

    ccnxNameSegment_Release(&segment);
    ccnxNameSegment_Release(&testSegment);
}

LONGBOW_TEST_FIXTURE(Error)
{
    LONGBOW_RUN_TEST_CASE(Error, ccnxInterestPayloadId__CreateFromNameSegment_NotFound);
}

typedef struct {
    CCNxName *name;
} TestDataError;

LONGBOW_TEST_FIXTURE_SETUP(Error)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);


    TestDataError *data = parcMemory_AllocateAndClear(sizeof(TestDataError));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataError));
    data->name = ccnxName_CreateFromCString("lci:/segment1/segment2/segment3");
    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Error)
{
    TestDataError *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxName_Release(&data->name);
    parcMemory_Deallocate((void **) &data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    parcMemory_SetInterface(_originalMemoryProvider);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Error, ccnxInterestPayloadId__CreateFromNameSegment_NotFound, .event = &LongBowAssertEvent)
{
    TestDataError *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxNameSegment *nameSegment = ccnxName_GetSegment(data->name, 0);
    CCNxInterestPayloadId *result =
        _ccnxInterestPayloadId_CreateFromNameSegment(nameSegment);
    assertNull(result, "Expect an assert event or NULL");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_InterestPayloadId);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
