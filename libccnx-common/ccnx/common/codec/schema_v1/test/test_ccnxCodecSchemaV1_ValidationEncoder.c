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
#include "../ccnxCodecSchemaV1_ValidationEncoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_keyid1_rsasha256.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_all_fields.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_nameA_crc32c.h>

#include "testrig_packetwrapper.c"

#include <ccnx/common/internal/ccnx_ValidationFacadeV1.h>

#include <ccnx/common/validation/ccnxValidation_CRC32C.h>
#include <ccnx/common/validation/ccnxValidation_HmacSha256.h>

// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_ValidationDecoder)
{
    LONGBOW_RUN_TEST_FIXTURE(EncodeAlg);
    LONGBOW_RUN_TEST_FIXTURE(EncodePayload);
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

LONGBOW_TEST_FIXTURE(EncodeAlg)
{
    LONGBOW_RUN_TEST_CASE(EncodeAlg, CRC32C);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, HMAC_SHA256);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, RSA_SHA256);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, DeduceFromSigner);

    LONGBOW_RUN_TEST_CASE(EncodeAlg, _encodeCertificate);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, _encodePublicKey);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, _encodeKeyId);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, _encodeKeyName);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, _encodeSignatureTime_Specified);
    LONGBOW_RUN_TEST_CASE(EncodeAlg, _encodeSignatureTime_Generated);
}

LONGBOW_TEST_FIXTURE_SETUP(EncodeAlg)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(EncodeAlg)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(EncodeAlg, CRC32C)
{
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvSuite = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C;
    PARCCryptoSuite suite = PARCCryptoSuite_NULL_CRC32C;

    uint8_t encoded[] = {
        0x00, tlvSuite, 0x00, 0,
    };

    PARCBuffer *trueEncoded = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxTlvDictionary_PutInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE, suite);

    ssize_t length = ccnxCodecSchemaV1ValidationEncoder_EncodeAlg(encoder, dictionary);
    assertFalse(length < 0, "Error on encoding: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(trueEncoded, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(trueEncoded, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&trueEncoded);
}

LONGBOW_TEST_CASE(EncodeAlg, HMAC_SHA256)
{
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvSuite = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256;
    PARCCryptoSuite suite = PARCCryptoSuite_HMAC_SHA256;

    uint8_t encoded[] = {
        0x00, tlvSuite, 0x00, 0,
    };

    PARCBuffer *trueEncoded = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxTlvDictionary_PutInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE, suite);

    ssize_t length = ccnxCodecSchemaV1ValidationEncoder_EncodeAlg(encoder, dictionary);
    assertFalse(length < 0, "Error on encoding: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(trueEncoded, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(trueEncoded, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&trueEncoded);
}

LONGBOW_TEST_CASE(EncodeAlg, RSA_SHA256)
{
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvSuite = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256;
    PARCCryptoSuite suite = PARCCryptoSuite_RSA_SHA256;

    uint8_t encoded[] = {
        0x00, tlvSuite, 0x00, 0,
    };

    PARCBuffer *trueEncoded = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxTlvDictionary_PutInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE, suite);

    ssize_t length = ccnxCodecSchemaV1ValidationEncoder_EncodeAlg(encoder, dictionary);
    assertFalse(length < 0, "Error on encoding: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(trueEncoded, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(trueEncoded, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&trueEncoded);
}

LONGBOW_TEST_CASE(EncodeAlg, DeduceFromSigner)
{
    PARCSigner *signer = ccnxValidationCRC32C_CreateSigner();
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvsuite = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C;

    uint8_t encoded[] = {
        0x00, tlvsuite, 0x00, 0,
    };

    PARCBuffer *trueEncoded = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_SetSigner(encoder, signer);

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();

    ssize_t length = ccnxCodecSchemaV1ValidationEncoder_EncodeAlg(encoder, dictionary);
    assertFalse(length < 0, "Error on encoding: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(trueEncoded, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(trueEncoded, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&trueEncoded);
    parcSigner_Release(&signer);
}

// =======

LONGBOW_TEST_CASE(EncodeAlg, _encodeCertificate)
{
    uint8_t encoded[] = {
        0x00, 0x0C, 0x00, 6,
        0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f
    };

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *cert = parcBuffer_Wrap(encoded, sizeof(encoded), 4, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxValidationFacadeV1_SetCertificate(dictionary, cert);

    ssize_t length = _encodeCertificate(encoder, dictionary);
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&cert);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}


LONGBOW_TEST_CASE(EncodeAlg, _encodePublicKey)
{
    uint8_t encoded[] = {
        0x00, 0x0B, 0x00, 6,
        0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f
    };

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *key = parcBuffer_Wrap(encoded, sizeof(encoded), 4, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxValidationFacadeV1_SetPublicKey(dictionary, key);

    ssize_t length = _encodePublicKey(encoder, dictionary);
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&key);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(EncodeAlg, _encodeKeyId)
{
    uint8_t encoded[] = {
        0x00, 0x09, 0x00, 6,
        0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f
    };

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *keyid = parcBuffer_Wrap(encoded, sizeof(encoded), 4, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxValidationFacadeV1_SetKeyId(dictionary, keyid);

    ssize_t length = _encodeKeyId(encoder, dictionary);
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&keyid);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(EncodeAlg, _encodeKeyName)
{
    uint8_t encoded[] = {
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

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *keyid = parcBuffer_Wrap(encoded, sizeof(encoded), 28, 32);
    PARCBuffer *hash = parcBuffer_Wrap(encoded, sizeof(encoded), 36, 44);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxName *name = ccnxName_CreateFromCString("lci:/3=apple/3=pie");
    CCNxLink *link = ccnxLink_Create(name, keyid, hash);

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxValidationFacadeV1_SetKeyName(dictionary, link);

    ssize_t length = _encodeKeyName(encoder, dictionary);
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&keyid);
    parcBuffer_Release(&hash);
    ccnxName_Release(&name);
    ccnxLink_Release(&link);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(EncodeAlg, _encodeSignatureTime_Specified)
{
    uint64_t sigtime = 0x1122334455667788ULL;
    uint8_t encoded[] = {
        0x00, 0x0F, 0x00, 8,
        0x11, 0x22, 0x33, 0x44,
        0x55, 0x66, 0x77, 0x88
    };

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxValidationFacadeV1_SetSigningTime(dictionary, sigtime);

    ssize_t length = _encodeSignatureTime(encoder, dictionary);
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    parcBuffer_Release(&truth);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

/*
 * Do not specify a signing time, but rather set a Signer and let the code
 * create the time on its own
 */
LONGBOW_TEST_CASE(EncodeAlg, _encodeSignatureTime_Generated)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    PARCBuffer *password = parcBuffer_Wrap("password", 8, 0, 8);
    PARCSigner *signer = ccnxValidationHmacSha256_CreateSigner(password);
    ccnxCodecTlvEncoder_SetSigner(encoder, signer);
    parcSigner_Release(&signer);

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();

    ssize_t length = _encodeSignatureTime(encoder, dictionary);
    assertTrue(length == 12, "Wrong length, expected 12, got %zd", length);

    parcBuffer_Release(&password);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(EncodePayload)
{
    LONGBOW_RUN_TEST_CASE(EncodePayload, payload_Specified);
    LONGBOW_RUN_TEST_CASE(EncodePayload, payload_Generated);
}

LONGBOW_TEST_FIXTURE_SETUP(EncodePayload)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(EncodePayload)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(EncodePayload, payload_Specified)
{
    uint8_t encoded[] = {
        0x11, 0x22, 0x33, 0x44,
        0x55, 0x66, 0x77, 0x88
    };

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxValidationFacadeV1_SetPayload(dictionary, truth);

    ssize_t length = ccnxCodecSchemaV1ValidationEncoder_EncodePayload(encoder, dictionary);
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    parcBuffer_Release(&truth);
    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

/*
 * Put the guts of v1_interest_nameA_crc32c in to the encoding buffer and mark it
 * as the signature block.  Generate the CRC and make sure we got the right thing.
 */
LONGBOW_TEST_CASE(EncodePayload, payload_Generated)
{
    TlvExtent interestExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_nameA_crc32c), V1_MANIFEST_INT_INTEREST);

    // This will test against the string (Interest, ValidationAlg, ValidationPayload)
    PARCBuffer *truth = parcBuffer_Wrap(v1_interest_nameA_crc32c,
                                        sizeof(v1_interest_nameA_crc32c),
                                        interestExtent.offset,
                                        sizeof(v1_interest_nameA_crc32c));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    PARCSigner *signer = ccnxValidationCRC32C_CreateSigner();
    ccnxCodecTlvEncoder_SetSigner(encoder, signer);
    parcSigner_Release(&signer);

    // This will append from the beginning of the Interest message up to the end of the ValidationAlg
    // This space is all marked as the "to-be-signed" section.
    ccnxCodecTlvEncoder_MarkSignatureStart(encoder);
    size_t signedInfoLenth = sizeof(v1_interest_nameA_crc32c) - 8 - interestExtent.offset;
    ccnxCodecTlvEncoder_AppendRawArray(encoder, signedInfoLenth, v1_interest_nameA_crc32c + interestExtent.offset);
    ccnxCodecTlvEncoder_MarkSignatureEnd(encoder);

    // add the validation payload container, then generate the signature
    ccnxCodecTlvEncoder_AppendContainer(encoder, CCNxCodecSchemaV1Types_MessageType_ValidationPayload, 4);

    // Do the actual encoding.  This will calculate the signature on the fly.
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ssize_t length = ccnxCodecSchemaV1ValidationEncoder_EncodePayload(encoder, dictionary);
    assertTrue(length == 4, "Wrong length, expected 4 got %zd", length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    // Tests that we got the right signature (CRC32c in this case)
    assertTrue(parcBuffer_Equals(truth, test), "Wrong buffer")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    ccnxTlvDictionary_Release(&dictionary);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
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
