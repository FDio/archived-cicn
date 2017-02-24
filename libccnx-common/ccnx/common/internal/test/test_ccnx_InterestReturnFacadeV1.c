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
#include "../ccnx_InterestReturnFacadeV1.c"
#include "../ccnx_InterestFacadeV1.h"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <ccnx/common/ccnx_Interest.h>
#include <ccnx/common/ccnx_InterestReturn.h>
#include <ccnx/common/ccnx_PayloadType.h>

typedef struct test_data {
    CCNxTlvDictionary *interest;

    CCNxName *name;
    PARCBuffer *keyid;
    PARCBuffer *contentObjectHash;
    PARCBuffer *payload;

    // allocated data
    uint8_t keyidArray[32];
    uint8_t contentObjectHashArray[32];
    uint8_t payloadArray[128];

    uint32_t lifetime;
    uint32_t hoplimit;
    CCNxPayloadType payloadType;
} TestData;


LONGBOW_TEST_RUNNER(ccnx_InterestReturnFacadeV1)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_InterestReturnFacadeV1)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_InterestReturnFacadeV1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ========================================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestReturnFacadeV1_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestReturnFacadeV1_GetReturnCode);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%lu) returned NULL", sizeof(TestData));
    data->name = ccnxName_CreateFromCString("lci:/once/upon/a/time");

    for (int i = 0; i < 32; i++) {
        data->keyidArray[i] = i * 7;
        data->contentObjectHashArray[i] = i * 11;
    }

    for (int i = 0; i < 128; i++) {
        data->payloadArray[i] = i * 13;
    }

    data->keyid = parcBuffer_Wrap(data->keyidArray, 32, 0, 32);
    data->contentObjectHash = parcBuffer_Wrap(data->contentObjectHashArray, 32, 0, 32);
    data->payloadType = CCNxPayloadType_DATA;
    data->payload = parcBuffer_Wrap(data->payloadArray, 128, 0, 128);

    data->lifetime = 900;
    data->hoplimit = 77;

    data->interest = ccnxInterest_Create(data->name,
                                         data->lifetime,
                                         data->keyid,
                                         data->contentObjectHash);

    ccnxInterest_SetPayload(data->interest, data->payload);

    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxName_Release(&data->name);
    parcBuffer_Release(&data->keyid);
    parcBuffer_Release(&data->contentObjectHash);
    parcBuffer_Release(&data->payload);
    ccnxTlvDictionary_Release(&data->interest);

    parcMemory_Deallocate((void **) &data);

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_TEARDOWN_FAILED;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxInterestReturnFacadeV1_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxTlvDictionary *interestReturn =
        _ccnxInterestReturnFacadeV1_Create(data->interest, CCNxInterestReturn_ReturnCode_NoRoute);
    assertNotNull(interestReturn, "Expect non-NULL interestReturn");
    _ccnxInterestReturnFacadeV1_AssertValid(interestReturn);

    CCNxInterestReturn_ReturnCode code = _ccnxInterestReturnFacadeV1_GetReturnCode(interestReturn);
    assertTrue((CCNxInterestReturn_ReturnCode_NoRoute == code), "InterestReturn wrong Return Code");
    ccnxTlvDictionary_Release(&interestReturn);
}

LONGBOW_TEST_CASE(Global, ccnxInterestReturnFacadeV1_GetReturnCode)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxTlvDictionary *interestReturn =
        _ccnxInterestReturnFacadeV1_Create(data->interest, CCNxInterestReturn_ReturnCode_NoRoute);
    _ccnxInterestReturnFacadeV1_AssertValid(interestReturn);

    CCNxInterestReturn_ReturnCode code =
        _ccnxInterestReturnFacadeV1_GetReturnCode(interestReturn);

    assertTrue((CCNxInterestReturn_ReturnCode_NoRoute == code), "InterestReturn wrong Return Code");
    ccnxTlvDictionary_Release(&interestReturn);
}


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_InterestReturnFacadeV1);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
