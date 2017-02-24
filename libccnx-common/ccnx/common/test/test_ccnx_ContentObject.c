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
#include "../ccnx_ContentObject.c"

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <inttypes.h>
#include <stdio.h>

typedef struct test_data {
    CCNxContentObjectInterface impl;
    CCNxName *name;
    CCNxContentObject *contentObject;
    CCNxContentObject *namelessContentObject;
} TestData;

LONGBOW_TEST_RUNNER(ccnx_ContentObject)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(EmptyImpl);
}

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));

    CCNxName *name = ccnxName_CreateFromCString("ccnx:/default/testData/content");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    data->impl = CCNxContentObjectFacadeV1_Implementation;
    data->name = name;
    data->contentObject = ccnxContentObject_CreateWithImplAndPayload(&data->impl, name, CCNxPayloadType_DATA, payload);
    data->namelessContentObject = ccnxContentObject_CreateWithImplAndPayload(&data->impl, NULL, CCNxPayloadType_DATA, payload);

    parcBuffer_Release(&payload);
    return data;
}

static void
_commonTeardown(TestData *data)
{
    if (data->contentObject) {
        ccnxContentObject_Release(&data->contentObject);
    }
    if (data->namelessContentObject) {
        ccnxContentObject_Release(&data->namelessContentObject);
    }
    if (data->name) {
        ccnxName_Release(&data->name);
    }

    parcMemory_Deallocate((void **) &data);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_ContentObject)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_ContentObject)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_SetSignature);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_GetKeyId);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_CreateWithNameAndPayload);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_CreateWithPayload);

    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_HasFinalChunkNumber);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_GetSetFinalChunkNumber);

    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_GetName);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_GetNameWithNameless);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_GetPayload);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_GetPayloadType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_AcquireRelease);

    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_HasExpiryTime);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_SetGetExpiryTime);

    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxContentObject_Display);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_CASE(Global, ccnxContentObject_CreateWithNameAndPayload)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/foo/bar");
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    ccnxContentObject_AssertValid(contentObject);

    ccnxName_Release(&name);
    parcBuffer_Release(&payload);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_CreateWithPayload)
{
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithPayload(payload);
    ccnxContentObject_AssertValid(contentObject);

    parcBuffer_Release(&payload);
    ccnxContentObject_Release(&contentObject);
}


LONGBOW_TEST_CASE(Global, ccnxContentObject_Equals)
{
    CCNxName *nameA = ccnxName_CreateFromCString("ccnx:/foo/bar/A");
    PARCBuffer *payloadA = parcBuffer_Allocate(100);

    CCNxContentObject *objectA = ccnxContentObject_CreateWithNameAndPayload(nameA, payloadA);
    ccnxContentObject_AssertValid(objectA);

    assertTrue(ccnxContentObject_Equals(objectA, objectA), "Expected same instance to be equal");

    CCNxContentObject *objectA2 = ccnxContentObject_CreateWithNameAndPayload(nameA, payloadA);
    ccnxContentObject_AssertValid(objectA2);

    assertTrue(ccnxContentObject_Equals(objectA, objectA2), "Expected ContentObject with same payload and name to be equal");

    CCNxName *nameB = ccnxName_CreateFromCString("ccnx:/foo/bar/B");
    CCNxContentObject *objectB = ccnxContentObject_CreateWithNameAndPayload(nameB, payloadA);
    ccnxContentObject_AssertValid(objectB);

    assertFalse(ccnxContentObject_Equals(objectA, objectB), "Expected ContentObject with same payload and different name");

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
    parcBuffer_Release(&payloadA);

    ccnxContentObject_Release(&objectA);
    ccnxContentObject_Release(&objectA2);
    ccnxContentObject_Release(&objectB);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_AcquireRelease)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/foo/bar");
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    ccnxContentObject_AssertValid(contentObject);

    CCNxContentObject *reference = ccnxContentObject_Acquire(contentObject);
    assertTrue(reference == contentObject, "Expected acquired reference to be equal to original");

    ccnxName_Release(&name);
    parcBuffer_Release(&payload);

    ccnxContentObject_AssertValid(contentObject);
    ccnxContentObject_AssertValid(reference);

    ccnxContentObject_Release(&contentObject);

    assertTrue(contentObject == NULL, "Expected contentObject pointer to be null");
    ccnxContentObject_AssertValid(reference);

    ccnxContentObject_Release(&reference);

    assertTrue(reference == NULL, "Expected contentObject pointer to be null");
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_HasFinalChunkNumber)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/foo/bar");
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    assertFalse(ccnxContentObject_HasFinalChunkNumber(contentObject), "Expected no final chunk number");

    ccnxContentObject_SetFinalChunkNumber(contentObject, 100);
    ccnxContentObject_AssertValid(contentObject);
    assertTrue(ccnxContentObject_HasFinalChunkNumber(contentObject), "Expected HasFinalChunkNumber to return true");
    assertTrue(ccnxContentObject_GetFinalChunkNumber(contentObject) == 100, "Expected final chunk number to be 100");

    ccnxName_Release(&name);
    parcBuffer_Release(&payload);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_GetSetFinalChunkNumber)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/foo/bar");
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);

    ccnxContentObject_SetFinalChunkNumber(contentObject, 100);
    ccnxContentObject_AssertValid(contentObject);
    assertTrue(ccnxContentObject_GetFinalChunkNumber(contentObject) == 100, "Expected final chunk number to be 100");

    ccnxContentObject_SetFinalChunkNumber(contentObject, 20010);
    ccnxContentObject_AssertValid(contentObject);
    assertTrue(ccnxContentObject_GetFinalChunkNumber(contentObject) == 20010, "Expected final chunk number to be 20010");

    ccnxName_Release(&name);
    parcBuffer_Release(&payload);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_GetName)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/foo/bar/baz");
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    ccnxContentObject_AssertValid(contentObject);

    CCNxName *actual = ccnxContentObject_GetName(contentObject);

    assertTrue(actual == name, "Expected GetName() to return the original CCNxName");

    ccnxName_Release(&name);
    parcBuffer_Release(&payload);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_GetNameWithNameless)
{
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithPayload(payload);
    ccnxContentObject_AssertValid(contentObject);

    CCNxName *actual = ccnxContentObject_GetName(contentObject);

    assertNull(actual, "Nameless CCNxContentObjects have no name and must therefore be null.");

    parcBuffer_Release(&payload);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_GetPayload)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/foo/bar");
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    ccnxContentObject_AssertValid(contentObject);

    PARCBuffer *actual = ccnxContentObject_GetPayload(contentObject);

    assertTrue(actual == payload, "Expected GetPayload() to return the original PARCBuffer");

    ccnxName_Release(&name);
    parcBuffer_Release(&payload);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_GetPayloadType)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/name");
    PARCBuffer *payload = parcBuffer_Allocate(100);

    CCNxPayloadType types[] = {
        CCNxPayloadType_DATA,
        CCNxPayloadType_KEY,
        CCNxPayloadType_LINK,
        CCNxPayloadType_MANIFEST,
    };


    for (int i = 0; i < sizeof(types) / sizeof(CCNxPayloadType); i++) {
        CCNxPayloadType type = types[i];
        CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
        ccnxContentObject_SetPayload(contentObject, type, payload);

        assertTrue(ccnxContentObject_GetPayloadType(contentObject) == type, "Unexpected PayloadType");
        ccnxContentObject_Release(&contentObject);
    }

    parcBuffer_Release(&payload);
    ccnxName_Release(&name);
}


LONGBOW_TEST_CASE(Global, ccnxContentObject_SetSignature)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/hello/dolly");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);

    PARCBuffer *keyId = parcBuffer_WrapCString("keyhash");
    PARCBuffer *sigbits = parcBuffer_WrapCString("siggybits");
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256, parcBuffer_Flip(sigbits));

    ccnxContentObject_SetSignature(contentObject, keyId, signature, NULL);

    parcBuffer_Release(&payload);
    parcBuffer_Release(&sigbits);
    parcBuffer_Release(&keyId);
    parcSignature_Release(&signature);
    ccnxName_Release(&name);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_GetKeyId)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/hello/dolly");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);

    assertNull(ccnxContentObject_GetKeyId(contentObject), "Expect NULL for KeyId here");

    PARCBuffer *testKeyId = parcBuffer_WrapCString("keyhash");
    PARCBuffer *sigbits = parcBuffer_WrapCString("siggybits");
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256, parcBuffer_Flip(sigbits));

    ccnxContentObject_SetSignature(contentObject, testKeyId, signature, NULL);

    PARCBuffer *keyId = ccnxContentObject_GetKeyId(contentObject);

    assertTrue(parcBuffer_Equals(keyId, testKeyId), "Expect key ids to match");

    parcBuffer_Release(&payload);
    parcBuffer_Release(&sigbits);
    parcBuffer_Release(&keyId);
    parcSignature_Release(&signature);
    ccnxName_Release(&name);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_HasExpiryTime)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/hello/dolly");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    // Use a V1 ContentObject, as V0 doesn't support ExpiryTime
    CCNxContentObject *contentObject =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_DATA, payload);


    assertFalse(ccnxContentObject_HasExpiryTime(contentObject), "Expected no expiration time by default");

    parcBuffer_Release(&payload);
    ccnxName_Release(&name);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_SetGetExpiryTime)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/hello/dolly");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    // Use a V1 ContentObject, as V0 doesn't support ExpiryTime
    CCNxContentObject *contentObject =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_DATA, payload);

    assertFalse(ccnxContentObject_HasExpiryTime(contentObject), "Expected no expiration time by default");

    uint64_t expiryTime = 1010101ULL;
    ccnxContentObject_SetExpiryTime(contentObject, expiryTime);

    assertTrue(ccnxContentObject_HasExpiryTime(contentObject), "Expected the expiryTime to be set");
    uint64_t retrievedTime = ccnxContentObject_GetExpiryTime(contentObject);
    assertTrue(expiryTime == retrievedTime, "Did not retrieve expected expiryTime from ContentObject");

    parcBuffer_Release(&payload);
    ccnxName_Release(&name);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxContentObject_GetExpiryTimeWithNoExpiryTime, .event = &LongBowTrapUnexpectedStateEvent)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/hello/dolly");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    // Use a V1 ContentObject, as V0 doesn't support ExpiryTime
    CCNxContentObject *contentObject =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_DATA, payload);

    // This should throw.
    uint64_t retrievedTime = ccnxContentObject_GetExpiryTime(contentObject);
    trapUnexpectedState("Expected to have thrown an exception when calling GetExpiryTime(), which returned %" PRIu64, retrievedTime);
    parcBuffer_Release(&payload);
    ccnxName_Release(&name);
    ccnxContentObject_Release(&contentObject);
}

LONGBOW_TEST_CASE(Global, ccnxContentObject_Display)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/hello/dolly");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);

    ccnxContentObject_Display(contentObject, 0);

    parcBuffer_Release(&payload);
    ccnxName_Release(&name);
    ccnxContentObject_Release(&contentObject);
}

///////////////////////////////////////////////////////////////////////////
// Empty Implementation Tests
///////////////////////////////////////////////////////////////////////////

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetPayloadType, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getPayloadType = NULL;

    CCNxPayloadType type = ccnxContentObject_GetPayloadType(data->contentObject);
    printf("We shouldn't get here. Payload = %d", type);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetPayload, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getPayload = NULL;

    PARCBuffer *payload = ccnxContentObject_GetPayload(data->contentObject);
    printf("We shouldn't get here. Payload = %p", (void *) payload);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetPayload, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setPayload = NULL;

    ccnxContentObject_SetPayload(data->contentObject, CCNxPayloadType_DATA, NULL);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetName, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getName = NULL;

    CCNxName *name = ccnxContentObject_GetName(data->contentObject);
    printf("We shouldn't get here. Name = %p", (void *) name);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetFinalChunkNumber, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setFinalChunkNumber = NULL;

    ccnxContentObject_SetFinalChunkNumber(data->contentObject, 100);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetFinalChunkNumber, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getFinalChunkNumber = NULL;

    ccnxContentObject_SetFinalChunkNumber(data->contentObject, 100);
    ccnxContentObject_GetFinalChunkNumber(data->contentObject);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetFinalChunkNumberNoHas, .event = &LongBowTrapUnexpectedStateEvent)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getFinalChunkNumber = NULL;

    ccnxContentObject_GetFinalChunkNumber(data->contentObject);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_HasFinalChunkNumber, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.hasFinalChunkNumber = NULL;

    if (ccnxContentObject_HasFinalChunkNumber(data->contentObject)) {
        printf("Shouldn't get here");
    }
}

LONGBOW_TEST_CASE(EmptyImpl, empty_HasExpiryTime)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.hasExpiryTime = NULL;

    assertFalse(ccnxContentObject_HasExpiryTime(data->contentObject), "If no expiry time implementation, return false.");
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetExpiryTime, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setExpiryTime = NULL;

    ccnxContentObject_SetExpiryTime(data->contentObject, 100);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetExpiryTime, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getExpiryTime = NULL;

    ccnxContentObject_SetExpiryTime(data->contentObject, 100);
    uint64_t expiryTime = ccnxContentObject_GetExpiryTime(data->contentObject);
    printf("We shouldn't get here, with expiryTime = %" PRIu64, expiryTime);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetExpiryTimeNoHas, .event = &LongBowTrapUnexpectedStateEvent)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getExpiryTime = NULL;

    uint64_t expiryTime = ccnxContentObject_GetExpiryTime(data->contentObject);
    printf("We shouldn't get here, with expiryTime = %" PRIu64, expiryTime);
}

LONGBOW_TEST_CASE(EmptyImpl, empty_Display)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.display = NULL;

    ccnxContentObject_Display(data->contentObject, 2);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_ToString, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.toString = NULL;

    const char *expectedString = ccnxContentObject_ToString(data->contentObject);
    if (expectedString != NULL) {
        parcMemory_Deallocate((void **) &expectedString);
    }
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_Equals, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.equals = NULL;

    if (ccnxContentObject_Equals(data->contentObject, data->contentObject)) {
        printf("Shouldn't get here");
    }
}

LONGBOW_TEST_FIXTURE(EmptyImpl)
{
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_Display);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_HasExpiryTime);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetExpiryTime);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetExpiryTimeNoHas);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetExpiryTime);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetFinalChunkNumber);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetFinalChunkNumber);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetFinalChunkNumberNoHas);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_HasFinalChunkNumber);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetPayload);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetPayloadType);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetPayload);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetName);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_ToString);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_Equals);
}

LONGBOW_TEST_FIXTURE_SETUP(EmptyImpl)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(EmptyImpl)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_ContentObject);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
