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
 * Setter tests use ground truth by examining the dictionary.
 * Getter tests use the setters to set values.
 *
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnx_ValidationFacadeV1.c"

#include <stdio.h>
#include <inttypes.h>

#include <parc/algol/parc_SafeMemory.h>

#include <LongBow/unit-test.h>

//#include "../validation/test/testrig_validation.c"

LONGBOW_TEST_RUNNER(ccnx_ValidationFacadeV1)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Setters);
    LONGBOW_RUN_TEST_FIXTURE(Getters);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_ValidationFacadeV1)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_ValidationFacadeV1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Setters)
{
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_SetKeyId);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_SetKeyName);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_SetPublicKey);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_SetCertificate);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_SetPayload);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_SetCryptoSuite);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_SetSigningTime);
}

LONGBOW_TEST_FIXTURE_SETUP(Setters)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Setters)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_SetKeyId)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *keyid = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    bool success = ccnxValidationFacadeV1_SetKeyId(dictionary, keyid);
    assertTrue(success, "Failed to set keyid");

    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYID);
    assertTrue(parcBuffer_Equals(test, keyid), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(keyid, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&keyid);
    ccnxTlvDictionary_Release(&dictionary);
}


LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_SetKeyName)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *keyid = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    PARCBuffer *hash = parcBuffer_Wrap((uint8_t []) { 11, 12, 13, 14 }, 4, 0, 4);
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo");
    CCNxLink *link = ccnxLink_Create(name, keyid, hash);

    bool success = ccnxValidationFacadeV1_SetKeyName(dictionary, link);
    assertTrue(success, "Failed to set keyname");

    CCNxName *testName = ccnxTlvDictionary_GetName(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_NAME);
    assertTrue(ccnxName_Equals(testName, name), "Wrong name")
    {
        printf("Expected\n");
        ccnxName_Display(name, 3);
        printf("Got\n");
        ccnxName_Display(testName, 3);
    }

    PARCBuffer *testKeyid = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_KEYID);
    assertTrue(parcBuffer_Equals(testKeyid, keyid), "Wrong keyid")
    {
        printf("Expected\n");
        parcBuffer_Display(keyid, 3);
        printf("Got\n");
        parcBuffer_Display(testKeyid, 3);
    }

    PARCBuffer *testHash = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_OBJHASH);
    assertTrue(parcBuffer_Equals(testHash, hash), "Wrong hash")
    {
        printf("Expected\n");
        parcBuffer_Display(hash, 3);
        printf("Got\n");
        parcBuffer_Display(testHash, 3);
    }

    parcBuffer_Release(&hash);
    parcBuffer_Release(&keyid);
    ccnxName_Release(&name);
    ccnxLink_Release(&link);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_SetPublicKey)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *key = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    bool success = ccnxValidationFacadeV1_SetPublicKey(dictionary, key);
    assertTrue(success, "Failed to set keyid");

    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEY);
    assertTrue(parcBuffer_Equals(test, key), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(key, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&key);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_SetCertificate)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *cert = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    bool success = ccnxValidationFacadeV1_SetCertificate(dictionary, cert);
    assertTrue(success, "Failed to set keyid");

    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CERT);
    assertTrue(parcBuffer_Equals(test, cert), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(cert, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&cert);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_SetPayload)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *payload = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    bool success = ccnxValidationFacadeV1_SetPayload(dictionary, payload);
    assertTrue(success, "Failed to set keyid");

    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_PAYLOAD);
    assertTrue(parcBuffer_Equals(test, payload), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(payload, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&payload);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_SetCryptoSuite)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCCryptoSuite suite = PARCCryptoSuite_RSA_SHA256;
    bool success = ccnxValidationFacadeV1_SetCryptoSuite(dictionary, suite);
    assertTrue(success, "Failed to set keyid");

    bool hasCryptoSuite = ccnxTlvDictionary_IsValueInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE);
    assertTrue(hasCryptoSuite, "Dictionary does not have a crypto suite value in it");

    PARCCryptoSuite test = (PARCCryptoSuite) ccnxTlvDictionary_GetInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE);
    assertTrue(test == suite, "Wrong suite, expected %d got %d", suite, test);

    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_SetSigningTime)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    uint64_t signingTime = 0x0102030405060708ULL;

    bool success = ccnxValidationFacadeV1_SetSigningTime(dictionary, signingTime);
    assertTrue(success, "Failed to set signing time");

    bool hasSigningTime = ccnxTlvDictionary_IsValueInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_SIGNTIME);
    assertTrue(hasSigningTime, "Dictionary does not have a signing time value in it");

    uint64_t test = ccnxTlvDictionary_GetInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_SIGNTIME);
    assertTrue(test == signingTime, "Wrong signing time, expected %" PRIx64 " got %" PRIx64, signingTime, test);

    ccnxTlvDictionary_Release(&dictionary);
}

// =============================================================

LONGBOW_TEST_FIXTURE(Getters)
{
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_GetKeyId);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_GetKeyName);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_GetPublicKey);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_GetCertificate);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_GetPayload);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_HasCryptoSuite_True);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_HasCryptoSuite_False);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_GetCryptoSuite);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_HasSigningTime_True);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_HasSigningTime_False);
    LONGBOW_RUN_TEST_CASE(Setters, ccnxValidationFacadeV1_GetSigningTime);
}

LONGBOW_TEST_FIXTURE_SETUP(Getters)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Getters)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_GetKeyId)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *keyid = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    ccnxValidationFacadeV1_SetKeyId(dictionary, keyid);

    PARCBuffer *test = ccnxValidationFacadeV1_GetKeyId(dictionary);
    assertTrue(parcBuffer_Equals(test, keyid), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(keyid, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&keyid);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_GetKeyName)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *keyid = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    PARCBuffer *hash = parcBuffer_Wrap((uint8_t []) { 11, 12, 13, 14 }, 4, 0, 4);
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo");
    CCNxLink *link = ccnxLink_Create(name, keyid, hash);

    ccnxValidationFacadeV1_SetKeyName(dictionary, link);

    CCNxLink *testLink = ccnxValidationFacadeV1_GetKeyName(dictionary);
    assertTrue(ccnxLink_Equals(testLink, link), "Wrong link");
    // no ccnxLink_Dispay, so cannot easily display items

    ccnxLink_Release(&testLink);
    parcBuffer_Release(&hash);
    parcBuffer_Release(&keyid);
    ccnxName_Release(&name);
    ccnxLink_Release(&link);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_GetPublicKey)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *key = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    ccnxValidationFacadeV1_SetPublicKey(dictionary, key);

    PARCBuffer *test = ccnxValidationFacadeV1_GetPublicKey(dictionary);
    assertTrue(parcBuffer_Equals(test, key), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(key, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&key);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_GetCertificate)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *cert = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    ccnxValidationFacadeV1_SetCertificate(dictionary, cert);

    PARCBuffer *test = ccnxValidationFacadeV1_GetCertificate(dictionary);
    assertTrue(parcBuffer_Equals(test, cert), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(cert, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&cert);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_GetPayload)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCBuffer *payload = parcBuffer_Wrap((uint8_t []) { 1, 2, 3, 4, 5 }, 5, 0, 5);
    ccnxValidationFacadeV1_SetPayload(dictionary, payload);

    PARCBuffer *test = ccnxValidationFacadeV1_GetPayload(dictionary);
    assertTrue(parcBuffer_Equals(test, payload), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(payload, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&payload);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_HasCryptoSuite_True)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCCryptoSuite suite = PARCCryptoSuite_RSA_SHA256;
    ccnxValidationFacadeV1_SetCryptoSuite(dictionary, suite);

    bool hasCryptoSuite = ccnxValidationFacadeV1_HasCryptoSuite(dictionary);
    assertTrue(hasCryptoSuite, "Dictionary does not have a crypto suite value in it");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_HasCryptoSuite_False)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    bool hasCryptoSuite = ccnxValidationFacadeV1_HasCryptoSuite(dictionary);
    assertFalse(hasCryptoSuite, "Dictionary says it has a crypto suite when none was set");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_GetCryptoSuite)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    PARCCryptoSuite suite = PARCCryptoSuite_RSA_SHA256;
    ccnxValidationFacadeV1_SetCryptoSuite(dictionary, suite);

    PARCCryptoSuite test = ccnxValidationFacadeV1_GetCryptoSuite(dictionary);
    assertTrue(test == suite, "Wrong crypto suite, expected %d got %d", suite, test);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_HasSigningTime_True)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    uint64_t signingTime = 0x0102030405060708ULL;
    ccnxValidationFacadeV1_SetSigningTime(dictionary, signingTime);

    bool hasSigningTime = ccnxValidationFacadeV1_HasSigningTime(dictionary);
    assertTrue(hasSigningTime, "Dictionary does not have a signing time value in it");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_HasSigningTime_False)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    bool hasSigningTime = ccnxValidationFacadeV1_HasSigningTime(dictionary);
    assertFalse(hasSigningTime, "Dictionary says it has a signing time when none was set");
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Setters, ccnxValidationFacadeV1_GetSigningTime)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    uint64_t signingTime = 0x0102030405060708ULL;
    ccnxValidationFacadeV1_SetSigningTime(dictionary, signingTime);

    uint64_t test = ccnxValidationFacadeV1_GetSigningTime(dictionary);
    assertTrue(test == signingTime, "Wrong signing time, expected %" PRIx64 " got %" PRIx64, signingTime, test);
    ccnxTlvDictionary_Release(&dictionary);
}


// =============================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_ValidationFacadeV1);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
