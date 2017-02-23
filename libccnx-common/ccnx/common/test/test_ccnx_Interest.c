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
#include <config.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include <ccnx/common/ccnx_Interest.c>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(ccnx_Interest)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(EmptyImpl);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_Interest)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_Interest)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_CreateSimple);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_Release);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_AssertValid);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_Equals_Same);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_Equals);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_SetLifetime);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_GetLifetime);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_GetName);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_SetGetPayload);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_SetPayloadWithId);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_SetPayloadAndId);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_SetGetHopLimit);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_SetKeyIdRestriction);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_GetKeyIdRestriction);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_GetContentObjectHashRestriction);
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_SetContentObjectHashRestriction);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_ToString);

    LONGBOW_RUN_TEST_CASE(Global, ccnxInterest_Display);
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

LONGBOW_TEST_CASE(Global, ccnxInterest_Create)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *key = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(key, 1234L);

    CCNxInterest *interest = ccnxInterest_Create(name,
                                                 15 * 1000,     /* lifetime, 15 seconds in milliseconds */
                                                 key,           /* KeyId */
                                                 NULL           /* ContentObjectHash */
                                                 );
    ccnxName_Release(&name);
    parcBuffer_Release(&key);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_CreateSimple)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_Release)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    CCNxInterest *reference = ccnxInterest_Acquire(interest);

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
    ccnxInterest_Release(&reference);

    assertNull(interest, "Expected ccnxInterest_Release to null the pointer.");
    assertNull(reference, "Expected ccnxInterest_Release to null the pointer.");
}

LONGBOW_TEST_CASE(Global, ccnxInterest_AssertValid)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    ccnxInterest_AssertValid(interest);

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_Equals_Same)
{
    CCNxName *nameA = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *key = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(key, 1234L);

    CCNxInterest *interestA = ccnxInterest_Create(nameA,
                                                  CCNxInterestDefault_LifetimeMilliseconds,     /* lifetime */
                                                  key,                                         /* KeyId */
                                                  NULL                                         /* ContentObjectHash */
                                                  );

    assertTrue(ccnxInterest_Equals(interestA, interestA), "Expected the same interest to be equal.");

    assertFalse(ccnxInterest_Equals(interestA, NULL), "Did not expect NULL to equal an Interest");


    ccnxName_Release(&nameA);
    parcBuffer_Release(&key);
    ccnxInterest_Release(&interestA);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_Equals)
{
    CCNxName *nameA = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *keyA = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(keyA, 1234L);

    CCNxInterest *interestA = ccnxInterest_Create(nameA,
                                                  1000, /* lifetime */
                                                  keyA, /* KeyId */
                                                  NULL  /* ContentObjectHash */
                                                  );

    CCNxName *nameB = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *keyB = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(keyB, 1234L);
    CCNxInterest *interestB = ccnxInterest_Create(nameB,
                                                  1000, /* lifetime */
                                                  keyB, /* KeyId */
                                                  NULL  /* ContentObjectHash */
                                                  );

    assertTrue(ccnxInterest_Equals(interestA, interestB), "Expected equivalent interests to be equal.");

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
    parcBuffer_Release(&keyA);
    parcBuffer_Release(&keyB);
    ccnxInterest_Release(&interestA);
    ccnxInterest_Release(&interestB);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_SetLifetime)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *key = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(key, 1234L);

    uint32_t lifetime = 5000; // 5 seconds, in milliseconds

    CCNxInterest *interest = ccnxInterest_Create(name,
                                                 lifetime, /* lifetime */
                                                 key,      /* KeyId */
                                                 NULL      /* ContentObjectHash */
                                                 );

    uint32_t actual = ccnxInterest_GetLifetime(interest);

    assertTrue(actual == lifetime, "Expected the retrieved lifetime to be equal to the assigned one.");

    lifetime = 2000;
    ccnxInterest_SetLifetime(interest, lifetime);
    actual = ccnxInterest_GetLifetime(interest);

    assertTrue(actual == lifetime, "Expected the retrieved lifetime to be equal to the assigned one.");

    ccnxName_Release(&name);
    parcBuffer_Release(&key);
    ccnxInterest_Release(&interest);
}


LONGBOW_TEST_CASE(Global, ccnxInterest_GetLifetime)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *key = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(key, 1234L);

    uint32_t lifetime = 5000; // 5 seconds, in milliseconds

    CCNxInterest *interest = ccnxInterest_Create(name,
                                                 lifetime, /* lifetime */
                                                 key,      /* KeyId */
                                                 NULL      /* ContentObjectHash */
                                                 );

    uint32_t actual = ccnxInterest_GetLifetime(interest);

    assertTrue(actual == lifetime, "Expected the retrieved lifetime to be equal to the assigned one.");

    ccnxName_Release(&name);
    parcBuffer_Release(&key);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_GetName)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    CCNxName *actual = ccnxInterest_GetName(interest);
    assertTrue(ccnxName_Equals(name, actual), "Expected the same name.");

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_SetKeyIdRestriction)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *key = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(key, 1234L);

    CCNxInterest *interest = ccnxInterest_Create(name, 3000, NULL, NULL);
    ccnxInterest_SetKeyIdRestriction(interest, key);
    PARCBuffer *actual = ccnxInterest_GetKeyIdRestriction(interest);

    actual = ccnxInterest_GetKeyIdRestriction(interest);
    assertTrue(actual == key, "Expected retrieved key to be the same as assigned");

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
    parcBuffer_Release(&key);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_GetKeyIdRestriction)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *key = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(key, 1234L);

    CCNxInterest *interest = ccnxInterest_Create(name, 3000, key, NULL);

    PARCBuffer *actual = ccnxInterest_GetKeyIdRestriction(interest);

    actual = ccnxInterest_GetKeyIdRestriction(interest);
    assertTrue(actual == key, "Expected retrieved key to be the same as assigned");

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
    parcBuffer_Release(&key);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_SetContentObjectHashRestriction)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *coh = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(coh, 77573L);

    CCNxInterest *interest = ccnxInterest_Create(name, CCNxInterestDefault_LifetimeMilliseconds, NULL, NULL);

    PARCBuffer *actual = ccnxInterest_GetContentObjectHashRestriction(interest);
    assertNull(actual, "Expected retrieved ContentObjectHash to be initially NULL");

    ccnxInterest_SetContentObjectHashRestriction(interest, coh);
    actual = ccnxInterest_GetContentObjectHashRestriction(interest);

    assertTrue(actual == coh, "Expected retrieved ContentObjectHash to be the same as assigned");

    // Re-setting is not yet supported. At the moment, you can only put the COHR once.
    // Now change it, and validate.
    //PARCBuffer *coh2 = parcBuffer_Allocate(8);
    //parcBuffer_PutUint64(coh2, 3262L);
    //ccnxInterest_SetContentObjectHashRestriction(interest, coh2);
    //actual = ccnxInterest_GetContentObjectHashRestriction(interest);
    //assertTrue(actual == coh2, "Expected retrieved ContentObjectHash to be the same as assigned");

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
    parcBuffer_Release(&coh);
    //parcBuffer_Release(&coh2);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_GetContentObjectHashRestriction)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    PARCBuffer *coh = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(coh, 1234L);

    CCNxInterest *interest = ccnxInterest_Create(name, CCNxInterestDefault_LifetimeMilliseconds, NULL, coh);

    PARCBuffer *actual = ccnxInterest_GetContentObjectHashRestriction(interest);

    assertTrue(actual == coh, "Expected retrieved ContentObjectHash to be the same as assigned");

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
    parcBuffer_Release(&coh);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_SetGetPayload)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    CCNxName *origNameCopy = ccnxName_Copy(name);

    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    if (impl->getPayload) {
        assertNull(ccnxInterest_GetPayload(interest), "Expected a NULL payload");
    }

    if (impl->getPayload && impl->setPayload) {
        PARCBuffer *payload = parcBuffer_WrapCString("impls have pimples");
        ccnxInterest_SetPayload(interest, payload);

        PARCBuffer *payloadOut = ccnxInterest_GetPayload(interest);

        assertTrue(parcBuffer_Equals(payload, payloadOut), "Expected an equal buffer");

        CCNxName *nameAfterPayload = ccnxInterest_GetName(interest);
        assertTrue(ccnxName_Equals(nameAfterPayload, origNameCopy), "Expected an unmodified name");

        parcBuffer_Release(&payload);
    }
    ccnxName_Release(&name);
    ccnxName_Release(&origNameCopy);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_SetPayloadAndId)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    if (impl->getPayload) {
        assertNull(ccnxInterest_GetPayload(interest), "Expected a NULL payload");
    }

    if (impl->getPayload && impl->setPayload) {
        PARCBuffer *payload = parcBuffer_WrapCString("impls have pimples");

        ccnxInterest_SetPayloadAndId(interest, payload);

        PARCBuffer *payloadOut = ccnxInterest_GetPayload(interest);

        assertTrue(parcBuffer_Equals(payload, payloadOut), "Expected an equal buffer");

        CCNxName *nameAfterPayload = ccnxInterest_GetName(interest);
        CCNxNameSegment *segment = ccnxName_GetSegment(nameAfterPayload, ccnxName_GetSegmentCount(nameAfterPayload) - 1);

        assertTrue(ccnxNameSegment_GetType(segment) == CCNxNameLabelType_PAYLOADID, "Expected to find a payload ID appended to the name");

        parcBuffer_Release(&payload);
    }
    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_SetPayloadWithId)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    CCNxName *origNameCopy = ccnxName_Copy(name);

    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    if (impl->getPayload) {
        assertNull(ccnxInterest_GetPayload(interest), "Expected a NULL payload");
    }

    if (impl->getPayload && impl->setPayload) {
        PARCBuffer *payload = parcBuffer_WrapCString("impls have pimples");
        PARCBuffer *payloadIdBuffer = parcBuffer_WrapCString("payload Id buffer");

        CCNxInterestPayloadId *payloadId = ccnxInterestPayloadId_Create(payloadIdBuffer,
                                                                        CCNxInterestPayloadId_TypeCode_App + 2);

        ccnxInterest_SetPayloadWithId(interest, payload, payloadId);

        PARCBuffer *payloadOut = ccnxInterest_GetPayload(interest);

        assertTrue(parcBuffer_Equals(payload, payloadOut), "Expected an equal buffer");

        CCNxName *nameAfterPayload = ccnxInterest_GetName(interest);
        CCNxNameSegment *segment = ccnxName_GetSegment(nameAfterPayload, ccnxName_GetSegmentCount(nameAfterPayload) - 1);

        assertTrue(ccnxNameSegment_GetType(segment) == CCNxNameLabelType_PAYLOADID, "Expected to find a payload ID appended to the name");

        CCNxInterestPayloadId *outId = ccnxInterestPayloadId_CreateFromSegmentInName(nameAfterPayload);
        assertTrue(ccnxInterestPayloadId_Equals(outId, payloadId), "expected to see the same payload Id after setting the payload");

        ccnxInterestPayloadId_Release(&payloadId);
        ccnxInterestPayloadId_Release(&outId);

        parcBuffer_Release(&payload);
        parcBuffer_Release(&payloadIdBuffer);
    }
    ccnxName_Release(&name);
    ccnxName_Release(&origNameCopy);
    ccnxInterest_Release(&interest);
}


LONGBOW_TEST_CASE(Global, ccnxInterest_SetGetHopLimit)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    if (impl->getHopLimit) {
        assertTrue(ccnxInterest_GetHopLimit(interest) == CCNxInterestDefault_HopLimit, "Expected the default hop limit");
    }

    if (impl->setHopLimit && impl->getHopLimit) {
        ccnxInterest_SetHopLimit(interest, 10);
        assertTrue(ccnxInterest_GetHopLimit(interest) == 10, "Expected a hopLimit of 10");
        ccnxInterest_SetHopLimit(interest, 20);
        assertTrue(ccnxInterest_GetHopLimit(interest) == 20, "Expected a hopLimit of 20");
    }

    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_ToString)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_Create(name,
                                                 CCNxInterestDefault_LifetimeMilliseconds, /* lifetime */
                                                 NULL,                                    /* KeyId */
                                                 NULL                                     /* ContentObjectHash */
                                                 );

    const char *expectedString = ccnxInterest_ToString(interest);
    assertNotNull(expectedString, "Expected non-null result from ccnxInterest_ToString.");

    parcMemory_Deallocate((void **) &expectedString);
    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
}

LONGBOW_TEST_CASE(Global, ccnxInterest_Display)
{
    PARCBuffer *coh = parcBuffer_Allocate(8);
    parcBuffer_PutUint64(coh, 7778L);

    CCNxName *name = ccnxName_CreateFromCString("lci:/name");
    CCNxInterest *interest = ccnxInterest_Create(name,
                                                 100, /* lifetime */
                                                 NULL,                                    /* KeyId */
                                                 coh);                                    /* ContentObjectHash */

    ccnxInterest_Display(interest, 2);
    ccnxName_Release(&name);
    ccnxInterest_Release(&interest);
    parcBuffer_Release(&coh);
}

///////////////////////////////////////////////////////////////////////////////
// Empty Implementation tests
///////////////////////////////////////////////////////////////////////////////


typedef struct test_data {
    CCNxInterestInterface impl;
    CCNxName *name;
    CCNxInterest *interest;
} TestData;


static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));

    CCNxName *name = ccnxName_CreateFromCString("lci:/default/testData/content");

    data->impl = CCNxInterestFacadeV1_Implementation; // This copies the struct each time.
    data->name = name;
    data->interest = ccnxInterest_CreateWithImpl(&data->impl, name, 100, NULL, NULL, 10);

    return data;
}

static void
_commonTeardown(TestData *data)
{
    if (data->interest) {
        ccnxInterest_Release(&data->interest);
    }
    if (data->name) {
        ccnxName_Release(&data->name);
    }

    parcMemory_Deallocate((void **) &data);
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


LONGBOW_TEST_CASE(EmptyImpl, empty_Display)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.display = NULL;

    ccnxInterest_Display(data->interest, 2);
}

LONGBOW_TEST_CASE(EmptyImpl, empty_ToString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.toString = NULL;

    const char *expectedString = ccnxInterest_ToString(data->interest);
    if (expectedString != NULL) {
        parcMemory_Deallocate((void **) &expectedString);
    }
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetName, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getName = NULL;

    CCNxName *name = ccnxInterest_GetName(data->interest);
    printf("Shouldn't get here. name = %p\n", (void *) name);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetHopLimit, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setHopLimit = NULL;

    if (ccnxInterest_SetHopLimit(data->interest, 10)) {
        printf("Shouldn't get here");
    }
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetHopLimit, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getHopLimit = NULL;

    uint32_t hopLimit = ccnxInterest_GetHopLimit(data->interest);
    printf("Shouldn't get here. hopLimit = %u\n", hopLimit);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetLifetime, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setLifetime = NULL;

    if (ccnxInterest_SetLifetime(data->interest, 10)) {
        printf("Shouldn't get here");
    }
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetLifetime, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getLifetime = NULL;

    uint32_t lifetime = ccnxInterest_GetLifetime(data->interest);
    printf("Shouldn't get here. lifetime = %u\n", lifetime);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetKeyIdRestriction, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setKeyIdRestriction = NULL;

    if (ccnxInterest_SetKeyIdRestriction(data->interest, NULL)) {
        printf("Shouldn't get here");
    }
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetKeyIdRestriction, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getKeyIdRestriction = NULL;

    PARCBuffer *keyIdRestriction = ccnxInterest_GetKeyIdRestriction(data->interest);
    printf("Shouldn't get here. getKeyIdRestriction = %p\n", (void *) keyIdRestriction);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetContentObjectHashRestriction, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setContentObjectHashRestriction = NULL;

    if (ccnxInterest_SetContentObjectHashRestriction(data->interest, NULL)) {
        printf("Shouldn't get here");
    }
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetContentObjectHashRestriction, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getContentObjectHashRestriction = NULL;

    PARCBuffer *restriction = ccnxInterest_GetContentObjectHashRestriction(data->interest);
    printf("Shouldn't get here. restriction = %p\n", (void *) restriction);
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_SetPayload, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.setPayload = NULL;

    if (ccnxInterest_SetPayload(data->interest, NULL)) {
        printf("Shouldn't get here");
    }
}

LONGBOW_TEST_CASE_EXPECTS(EmptyImpl, empty_GetPayload, .event = &LongBowTrapNotImplemented)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->impl.getPayload = NULL;

    PARCBuffer *payload = ccnxInterest_GetPayload(data->interest);
    printf("Shouldn't get here. payload = %p\n", (void *) payload);
}

LONGBOW_TEST_FIXTURE(EmptyImpl)
{
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_Display);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_ToString);

    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetName);

    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetHopLimit);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetHopLimit);

    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetLifetime);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetLifetime);

    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetKeyIdRestriction);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetKeyIdRestriction);

    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetContentObjectHashRestriction);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetContentObjectHashRestriction);

    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_GetPayload);
    LONGBOW_RUN_TEST_CASE(EmptyImpl, empty_SetPayload);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_Interest);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
