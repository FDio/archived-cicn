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
#include "../ccnxCodecSchemaV1_HashCodec.c"

#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <ccnx/common/codec/ccnxCodec_Error.h>

LONGBOW_TEST_RUNNER(ccnxTlvCodec_Hash)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxTlvCodec_Hash)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxTlvCodec_Hash)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidHash);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidLength_SHA256);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidLength_SHA512);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidLength_App);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1HashCodec_Encode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1HashCodec_Encode_InvalidLength);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue)
{
    // -- 32-byte hash
    uint8_t encoded[] = {
        0x00, CCNxCodecSchemaV1Types_HashType_SHA256, 0x00, 0x20,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
    };

    PARCBuffer *tlvBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *payloadBuffer = parcBuffer_Wrap(encoded + 4, sizeof(encoded) - 4, 0, sizeof(encoded) - 4);

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(tlvBuffer);
    PARCCryptoHash *hash = ccnxCodecSchemaV1HashCodec_DecodeValue(decoder, sizeof(encoded));
    assertNotNull(hash, "got non-NULL hash when it should have been an error (null)");

    assertTrue(parcCryptoHash_GetDigestType(hash) == PARCCryptoHashType_SHA256, "Expected to decode the correct hash type.");
    assertTrue(parcBuffer_Equals(payloadBuffer, parcCryptoHash_GetDigest(hash)), "Expected the digest to match.");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNull(error, "Got null error when it should have been set");

    parcCryptoHash_Release(&hash);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&tlvBuffer);
    parcBuffer_Release(&payloadBuffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidHash)
{
    // -- 32-byte hash
    uint8_t encoded[] = {
        0x00, 0xFF, 0x00, 0x20,
        0x00, 0x00, 0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,0x00,  0x00, 0x00, 0x00,
    };

    PARCBuffer *tlvBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(tlvBuffer);
    PARCCryptoHash *hash = ccnxCodecSchemaV1HashCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(hash, "Should not have decoded an incorrect hash digest");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&tlvBuffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidLength_SHA256)
{
    // -- 32-byte hash
    uint8_t encoded[] = {
        0x00, CCNxCodecSchemaV1Types_HashType_SHA256, 0x00, 0x18,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
    };

    PARCBuffer *tlvBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(tlvBuffer);
    PARCCryptoHash *hash = ccnxCodecSchemaV1HashCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(hash, "Should not have decoded a SHA256 hash digest with an incorrect length");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&tlvBuffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidLength_SHA512)
{
    // -- 32-byte hash
    uint8_t encoded[] = {
        0x00, CCNxCodecSchemaV1Types_HashType_SHA512, 0x00, 0x18,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
    };

    PARCBuffer *tlvBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(tlvBuffer);
    PARCCryptoHash *hash = ccnxCodecSchemaV1HashCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(hash, "Should not have decoded a SHA512 hash digest with an incorrect length");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&tlvBuffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecHash_DecodeValue_InvalidLength_App)
{
    // -- 32-byte hash
    uint8_t encoded[] = {
        0x00, CCNxCodecSchemaV1Types_HashType_App, 0x00, 0x08,
        0x00, 0x00,                                0x00, 0x00,0x00,0x00, 0x00, 0x00
    };

    PARCBuffer *tlvBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(tlvBuffer);
    PARCCryptoHash *hash = ccnxCodecSchemaV1HashCodec_DecodeValue(decoder, sizeof(encoded));
    assertNotNull(hash, "Should have decoded an application hash digest with an arbitrary length");

    parcCryptoHash_Release(&hash);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&tlvBuffer);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1HashCodec_Encode)
{
    uint8_t encoded[] = {
        0x00, CCNxCodecSchemaV1Types_HashType_SHA256, 0x00, 0x20,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
        0x00, 0x00,                                   0x00, 0x00,0x00,  0x00, 0x00, 0x00,
    };

    PARCBuffer *trueEncoding = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    // Create the hash
    PARCBuffer *payloadBuffer = parcBuffer_Allocate(0x20);
    PARCCryptoHash *expectedHash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, payloadBuffer);

    // Encode it
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1HashCodec_Encode(encoder, expectedHash);
    assertFalse(length < 0, "Got error on encode: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zd got %zd", sizeof(encoded), length);

    // Check for equality
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *testEncoding = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(trueEncoding, testEncoding), "The hash was encoded incorrectly.")
    {
        printf("Expected\n");
        parcBuffer_Display(trueEncoding, 3);
        printf("Got\n");
        parcBuffer_Display(testEncoding, 3);
    }

    parcBuffer_Release(&testEncoding);
    parcBuffer_Release(&trueEncoding);

    parcCryptoHash_Release(&expectedHash);
    parcBuffer_Release(&payloadBuffer);

    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1HashCodec_Encode_InvalidLength)
{
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxTlvCodec_Hash);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
