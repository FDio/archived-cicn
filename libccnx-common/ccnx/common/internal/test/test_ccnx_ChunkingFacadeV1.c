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
#include "../ccnx_ChunkingFacadeV1.c"
#include <parc/algol/parc_SafeMemory.h>

#include <LongBow/unit-test.h>
#include <inttypes.h>

#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/ccnx_Interest.h>
#include <ccnx/common/ccnx_PayloadType.h>

#include <ccnx/common/internal/ccnx_ContentObjectInterface.h>

typedef struct test_data {
    CCNxName *name;
    CCNxTlvDictionary *contentObjectV1;
    CCNxTlvDictionary *contentObjectVFF;

    CCNxTlvDictionary *interest;
} TestData;

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    data->name = ccnxName_CreateFromCString("lci:/foo/bar/v1");

    data->contentObjectV1 =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   data->name,
                                                   CCNxPayloadType_DATA, NULL);

    data->contentObjectVFF = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);

    ccnxTlvDictionary_SetMessageType_ContentObject(data->contentObjectVFF, 0xFF);

    data->interest =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    data->name, 5000, NULL, NULL, 100);

    return data;
}

static void
_commonTeardown(TestData *data)
{
    ccnxName_Release(&data->name);
    ccnxTlvDictionary_Release(&data->contentObjectV1);
    ccnxTlvDictionary_Release(&data->contentObjectVFF);
    ccnxTlvDictionary_Release(&data->interest);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(ccnx_ChunkingFacade)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_ChunkingFacade)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_ChunkingFacade)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_GetEndChunkNumber_NotContentObject);

    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_GetEndChunkNumber_V1_With);
    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_GetEndChunkNumber_V1_Without);
    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_GetEndChunkNumber_InvalidVersion);

    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_HasEndChunkNumber_NotContentObject);
    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_HasEndChunkNumber_V1_With);
    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_HasEndChunkNumber_V1_Without);
    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_HasEndChunkNumber_InvalidVersion);

    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_SetEndChunkNumber_NotContentObject);
    LONGBOW_RUN_TEST_CASE(Global, ccnxChunkingFacade_SetEndChunkNumber_InvalidVersion);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _commonTeardown(data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// ======================================================================================

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxChunkingFacade_GetEndChunkNumber_NotContentObject, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxChunkingFacadeV1_GetEndChunkNumber(data->interest);
}

LONGBOW_TEST_CASE(Global, ccnxChunkingFacade_GetEndChunkNumber_V1_With)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint64_t endChunkNumber = 5;

    ccnxChunkingFacadeV1_SetEndChunkNumber(data->contentObjectV1, endChunkNumber);
    uint64_t test = ccnxChunkingFacadeV1_GetEndChunkNumber(data->contentObjectV1);
    assertTrue(test == endChunkNumber, "wrong value, got %" PRIu64 " expected %" PRIu64 "", test, endChunkNumber)
    {
        ccnxTlvDictionary_Display(data->contentObjectV1, 3);
    }
}

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxChunkingFacade_GetEndChunkNumber_V1_Without, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxChunkingFacadeV1_GetEndChunkNumber(data->contentObjectV1);
}

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxChunkingFacade_GetEndChunkNumber_InvalidVersion, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxChunkingFacadeV1_GetEndChunkNumber(data->contentObjectVFF);
}

// ======================================================================================


LONGBOW_TEST_CASE(Global, ccnxChunkingFacade_HasEndChunkNumber_NotContentObject)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool success = ccnxChunkingFacadeV1_HasEndChunkNumber(data->interest);
    assertFalse(success, "An Interest should always return FALSE for EndChunkNumber");
}

LONGBOW_TEST_CASE(Global, ccnxChunkingFacade_HasEndChunkNumber_V1_With)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    ccnxChunkingFacadeV1_SetEndChunkNumber(data->contentObjectV1, 5);
    bool success = ccnxChunkingFacadeV1_HasEndChunkNumber(data->contentObjectV1);
    assertTrue(success, "Content Object with EndChunkNumber returned false")
    {
        ccnxTlvDictionary_Display(data->contentObjectV1, 3);
    }
}

LONGBOW_TEST_CASE(Global, ccnxChunkingFacade_HasEndChunkNumber_V1_Without)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    bool success = ccnxChunkingFacadeV1_HasEndChunkNumber(data->contentObjectV1);
    assertFalse(success, "Content Object without EndChunkNumber returned true")
    {
        ccnxTlvDictionary_Display(data->contentObjectV1, 3);
    }
}

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxChunkingFacade_HasEndChunkNumber_InvalidVersion, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxChunkingFacadeV1_HasEndChunkNumber(data->contentObjectVFF);
}

// ======================================================================================

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxChunkingFacade_SetEndChunkNumber_NotContentObject, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint64_t endChunkNumber = 7;
    ccnxChunkingFacadeV1_SetEndChunkNumber(data->interest, endChunkNumber);
}


LONGBOW_TEST_CASE(Global, ccnxChunkingFacade_SetEndChunkNumber_V1)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint64_t endChunkNumber = 7;
    bool success = ccnxChunkingFacadeV1_SetEndChunkNumber(data->contentObjectV1, endChunkNumber);

    assertTrue(success, "Setting EndChunkNumber failed")
    {
        ccnxTlvDictionary_Display(data->contentObjectV1, 3);
    }
}

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxChunkingFacade_SetEndChunkNumber_InvalidVersion, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxChunkingFacadeV1_SetEndChunkNumber(data->contentObjectVFF, 7);
}

// ======================================================================================

LONGBOW_TEST_FIXTURE(Local)
{
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

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_ChunkingFacade);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
