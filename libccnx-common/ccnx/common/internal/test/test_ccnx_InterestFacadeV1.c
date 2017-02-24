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
#include "../ccnx_InterestFacadeV1.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <ccnx/common/ccnx_Interest.h>

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
} TestData;

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
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
    data->payload = parcBuffer_Wrap(data->payloadArray, 128, 0, 128);

    data->lifetime = 900;
    data->hoplimit = 77;

    data->interest = _ccnxInterestFacadeV1_Create(data->name,
                                                  data->lifetime,
                                                  data->keyid,
                                                  data->contentObjectHash,
                                                  data->hoplimit);

    _ccnxInterestFacadeV1_SetPayload(data->interest, data->payload);

    return data;
}

static void
_commonTeardown(TestData *data)
{
    ccnxName_Release(&data->name);
    parcBuffer_Release(&data->keyid);
    parcBuffer_Release(&data->contentObjectHash);
    parcBuffer_Release(&data->payload);
    ccnxTlvDictionary_Release(&data->interest);

    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(ccnx_InterestFacadeV1)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Performance);

    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_InterestFacadeV1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_InterestFacadeV1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ========================================================================================

LONGBOW_TEST_FIXTURE(Performance)
{
    LONGBOW_RUN_TEST_CASE(Performance, newfangled);
//    LONGBOW_RUN_TEST_CASE(Performance, oldtimey);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Performance, newfangled)
{
    uint8_t keyidArray[32];
    for (int i = 0; i < 32; i++) {
        keyidArray[i] = i;
    }

    PARCBuffer *keyid = parcBuffer_Wrap(keyidArray, 32, 0, 32);
    CCNxName *name = ccnxName_CreateFromCString("lci:/dark/and/stormy/bits");

    struct timeval t0, t1;
    unsigned trials = 10000;
    gettimeofday(&t0, NULL);
    for (int i = 0; i < trials; i++) {
        CCNxTlvDictionary *interest = _ccnxInterestFacadeV1_Create(name,
                                                                   CCNxInterestDefault_LifetimeMilliseconds,
                                                                   keyid,
                                                                   NULL,
                                                                   0x45);

        ccnxTlvDictionary_Release(&interest);
    }
    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &t1);
    double seconds = t1.tv_sec + t1.tv_usec * 1E-6;
    printf("\nNewFangled iterations %u seconds %.3f msg/sec %.3f\n", trials, seconds, trials / seconds);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyid);
}


// ========================================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_CreateSimple);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_GetContentObjectHash);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_GetHopLimit);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_GetInterestLifetime);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_GetName);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_GetPayload);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_GetPublisherPublicKeyDigest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_Display);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestFacadeV1_Equals);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_CreateSimple)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTlvDictionary *interest = _ccnxInterestFacadeV1_CreateSimple(data->name);

    CCNxName *test = _ccnxInterestFacadeV1_GetName(interest);
    assertTrue(ccnxName_Equals(test, data->name), "Names do not match");
    ccnxTlvDictionary_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_GetContentObjectHash)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *test = _ccnxInterestFacadeV1_GetContentObjectHashRestriction(data->interest);
    assertTrue(parcBuffer_Equals(test, data->contentObjectHash), "ContentObjectHashes do not match")
    {
        printf("\ngot     : \n"); parcBuffer_Display(test, 3);
        printf("\nexpected: \n"); parcBuffer_Display(data->contentObjectHash, 3);
    }
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_GetHopLimit)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    unsigned test = (unsigned) _ccnxInterestFacadeV1_GetHopLimit(data->interest);
    assertTrue(test == data->hoplimit, "Wrong hoplimit: got %u expected %u", test, data->hoplimit);
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_GetInterestLifetime)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    unsigned test = (unsigned) _ccnxInterestFacadeV1_GetLifetime(data->interest);
    assertTrue(test == data->lifetime, "Wrong lifetime: got %u expected %u", test, data->lifetime);
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_AssertValid)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _ccnxInterestFacadeV1_Display(data->interest, 4);
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_Display)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _ccnxInterestFacadeV1_AssertValid(data->interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_GetName)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxName *test = _ccnxInterestFacadeV1_GetName(data->interest);
    assertTrue(ccnxName_Equals(test, data->name), "Names do not match")
    {
        printf("\ngot     : \n"); ccnxName_Display(test, 3);
        printf("\nexpected: \n"); ccnxName_Display(data->name, 3);
    }
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_GetPublisherPublicKeyDigest)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *test = _ccnxInterestFacadeV1_GetKeyIdRestriction(data->interest);
    assertTrue(parcBuffer_Equals(test, data->keyid), "KeyIDs do not match")
    {
        printf("\ngot     : \n"); parcBuffer_Display(test, 3);
        printf("\nexpected: \n"); parcBuffer_Display(data->keyid, 3);
    }
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_GetPayload)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *test = _ccnxInterestFacadeV1_GetPayload(data->interest);
    assertTrue(parcBuffer_Equals(test, data->payload), "Payloads do not match")
    {
        printf("\ngot     : \n"); parcBuffer_Display(test, 3);
        printf("\nexpected: \n"); parcBuffer_Display(data->payload, 3);
    }
}

LONGBOW_TEST_CASE(Global, ccnxInterestFacadeV1_Equals)
{
    CCNxName *name1 = ccnxName_CreateFromCString("lci:/name/1");
    CCNxName *name2 = ccnxName_CreateFromCString("lci:/name/2");

    CCNxTlvDictionary *x = _ccnxInterestFacadeV1_CreateSimple(name1); // same
    CCNxTlvDictionary *y = _ccnxInterestFacadeV1_CreateSimple(name1); // same
    CCNxTlvDictionary *z = _ccnxInterestFacadeV1_CreateSimple(name1); // same

    CCNxTlvDictionary *diff = _ccnxInterestFacadeV1_CreateSimple(name2); // different

    assertEqualsContract(_ccnxInterestFacadeV1_Equals, x, y, z, diff);

    ccnxTlvDictionary_Release(&x);
    ccnxTlvDictionary_Release(&y);
    ccnxTlvDictionary_Release(&z);
    ccnxTlvDictionary_Release(&diff);

    ccnxName_Release(&name1);
    ccnxName_Release(&name2);
}


// ========================================================================================

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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_InterestFacadeV1);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
