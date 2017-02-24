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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnxCodecSchemaV1_ValidationDecoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_keyid1_rsasha256.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_all_fields.h>

#include "testrig_packetwrapper.c"

// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_ValidationDecoder)
{
    LONGBOW_RUN_TEST_FIXTURE(DecodeAlg);
    LONGBOW_RUN_TEST_FIXTURE(DecodePayload);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_ValidationDecoder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_ValidationDecoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// =========================================================================

LONGBOW_TEST_FIXTURE(DecodeAlg)
{
    LONGBOW_RUN_TEST_CASE(DecodeAlg, CRC32C);
    LONGBOW_RUN_TEST_CASE(DecodeAlg, HMAC_SHA256);
    LONGBOW_RUN_TEST_CASE(DecodeAlg, RSA_SHA256);

    LONGBOW_RUN_TEST_CASE(DecodeAlg, Cert);
    LONGBOW_RUN_TEST_CASE(DecodeAlg, PublicKey);
    LONGBOW_RUN_TEST_CASE(DecodeAlg, KeyId);
    LONGBOW_RUN_TEST_CASE(DecodeAlg, KeyName);
    LONGBOW_RUN_TEST_CASE(DecodeAlg, SigTime);

    LONGBOW_RUN_TEST_CASE(DecodeAlg, KeyName_Invalid);
}

LONGBOW_TEST_FIXTURE_SETUP(DecodeAlg)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(DecodeAlg)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(DecodeAlg, CRC32C)
{
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvSuite = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C;
    PARCCryptoSuite parcSuite = PARCCryptoSuite_NULL_CRC32C;

    uint8_t encoded[] = {
        0x00, tlvSuite, 0x00, 0,
        0x00, 0x00,     0x00, 0x00
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    PARCCryptoSuite test = (PARCCryptoSuite) ccnxTlvDictionary_GetInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE);
    assertTrue(test == parcSuite, "Got wrong suite, expected %d got %d", parcSuite, test);

    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, HMAC_SHA256)
{
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvSuite = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256;
    PARCCryptoSuite parcSuite = PARCCryptoSuite_HMAC_SHA256;

    uint8_t encoded[] = {
        0x00, tlvSuite, 0x00, 0,
        0x00, 0x00,     0x00, 0x00
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    PARCCryptoSuite test = (PARCCryptoSuite) ccnxTlvDictionary_GetInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE);
    assertTrue(test == parcSuite, "Got wrong suite, expected %d got %d", parcSuite, test);

    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, RSA_SHA256)
{
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvSuite = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256;
    PARCCryptoSuite parcSuite = PARCCryptoSuite_RSA_SHA256;

    uint8_t encoded[] = {
        0x00, tlvSuite, 0x00, 0,
        0x00, 0x00,     0x00, 0x00
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    PARCCryptoSuite test = (PARCCryptoSuite) ccnxTlvDictionary_GetInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE);
    assertTrue(test == parcSuite, "Got wrong suite, expected %d got %d", parcSuite, test);

    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, Cert)
{
    uint8_t encoded[] = {
        0x00, 0x06, 0x00, 10,
        0x00, 0x0C, 0x00, 6,
        0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 8, sizeof(encoded));
    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CERT);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, PublicKey)
{
    uint8_t encoded[] = {
        0x00, 0x06, 0x00, 10,
        0x00, 0x0B, 0x00, 6,
        0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 8, sizeof(encoded));
    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEY);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, KeyId)
{
    uint8_t encoded[] = {
        0x00, 0x06, 0x00, 10,
        0x00, 0x09, 0x00, 6,
        0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 8, sizeof(encoded));
    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYID);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, KeyName)
{
    uint8_t encoded[] = {
        0x00, 0x06, 0x00, 44,
        0x00, 0x0E, 0x00, 40,
        // --- name
        0x00, 0x00, 0x00, 16,
        0x00, 0x03, 0x00, 5,
        'a',  'p',  'p',  'l',
        'e',
        0x00, 0x03, 0x00, 3,
        'p',  'i',  'e',
        // --- keyid
        0x00, 0x01, 0x00, 4,
        0xa1, 0xa2, 0xa3, 0xa4,
        // --- hash
        0x00, 0x02, 0x00, 8,
        0xb1, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxName *name = ccnxName_CreateFromCString("lci:/3=apple/3=pie");
    PARCBuffer *keyid = parcBuffer_Wrap(encoded, sizeof(encoded), 32, 36);
    PARCBuffer *hash = parcBuffer_Wrap(encoded, sizeof(encoded), 40, 48);

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    // now test the 3 decoded fields
    CCNxName *testName = ccnxTlvDictionary_GetName(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_NAME);
    assertTrue(ccnxName_Equals(testName, name), "keynames do not match")
    {
        printf("Expected\n");
        ccnxName_Display(name, 3);
        printf("Got\n");
        ccnxName_Display(testName, 3);
    }

    PARCBuffer *testKeyid = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_KEYID);
    assertTrue(parcBuffer_Equals(testKeyid, keyid), "keyid do not match")
    {
        printf("Expected\n");
        parcBuffer_Display(keyid, 3);
        printf("Got\n");
        parcBuffer_Display(testKeyid, 3);
    }

    PARCBuffer *testHash = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_OBJHASH);
    assertTrue(parcBuffer_Equals(testHash, hash), "keyid do not match")
    {
        printf("Expected\n");
        parcBuffer_Display(hash, 3);
        printf("Got\n");
        parcBuffer_Display(testHash, 3);
    }

    parcBuffer_Release(&keyid);
    parcBuffer_Release(&hash);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, KeyName_Invalid)
{
    // link is missing the Name
    uint8_t encoded[] = {
        0x00, 0x06, 0x00, 24,
        0x00, 0x0E, 0x00, 20,
        // --- keyid
        0x00, 0x01, 0x00, 4,
        0xa1, 0xa2, 0xa3, 0xa4,
        // --- hash
        0x00, 0x02, 0x00, 8,
        0xb1, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertFalse(success, "Should have failed decode as keyname is invalid");

    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodeAlg, SigTime)
{
    uint64_t sigtime = 0x1122334455667788ULL;
    uint8_t encoded[] = {
        0x00, 0x06, 0x00, 12,
        0x00, 0x0F, 0x00, 8,
        0x11, 0x22, 0x33, 0x44,
        0x55, 0x66, 0x77, 0x88
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, dictionary);
    assertTrue(success, "Failed ccnxCodecSchemaV1ValidationDecoder_DecodeAlg: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    uint64_t test = ccnxTlvDictionary_GetInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_SIGNTIME);
    assertTrue(test == sigtime, "Wrong sig time, expected %" PRIx64 ", got %" PRIx64, sigtime, test);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}


// =========================================================================

LONGBOW_TEST_FIXTURE(DecodePayload)
{
    LONGBOW_RUN_TEST_CASE(DecodePayload, payload);
    LONGBOW_RUN_TEST_CASE(DecodePayload, payload_zero);
}

LONGBOW_TEST_FIXTURE_SETUP(DecodePayload)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(DecodePayload)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(DecodePayload, payload)
{
    uint8_t encoded[] = {
        0x00, 0x04, 0x00, 8,
        0x11, 0x22, 0x33, 0x44,
        0x55, 0x66, 0x77, 0x88
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);

    // the caller has already parsed the T and L, so we point to just payload
    ccnxCodecTlvDecoder_Advance(decoder, 4);

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodePayload(decoder, dictionary);
    assertTrue(success, "Failed to decode a valid payload: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 4, 12);
    PARCBuffer *test = ccnxTlvDictionary_GetBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_PAYLOAD);
    assertTrue(parcBuffer_Equals(truth, test), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(DecodePayload, payload_zero)
{
    uint8_t encoded[] = {
        0x00, 0x04, 0x00, 0,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);

    // the caller has already parsed the T and L, so we point to just payload
    ccnxCodecTlvDecoder_Advance(decoder, 4);

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    bool success = ccnxCodecSchemaV1ValidationDecoder_DecodePayload(decoder, dictionary);
    assertFalse(success, "Should have failed on 0-length payload");

    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

// =========================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_ValidationDecoder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
