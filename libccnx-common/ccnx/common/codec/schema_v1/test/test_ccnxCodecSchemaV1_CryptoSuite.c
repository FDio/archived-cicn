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
#include "../ccnxCodecSchemaV1_CryptoSuite.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <stdio.h>

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_CryptoSuite)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_CryptoSuite)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_CryptoSuite)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_ParcToTlv);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_TlvToParc);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv);
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

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_ParcToTlv)
{
    struct test_vector {
        PARCCryptoSuite input;
        CCNxCodecSchemaV1TlvDictionary_CryptoSuite output;
        bool success;
        bool sentinel;
    } vectors[] = {
        { .input = PARCCryptoSuite_RSA_SHA256,  .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256,  .success = true,  .sentinel = false },
        { .input = PARCCryptoSuite_HMAC_SHA256, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256, .success = true,  .sentinel = false },
        { .input = PARCCryptoSuite_NULL_CRC32C, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C,     .success = true,  .sentinel = false },
        { .input = PARCCryptoSuite_DSA_SHA256,  .output = 0,                                                     .success = false, .sentinel = false },
        { .input = PARCCryptoSuite_RSA_SHA512,  .output = 0,                                                     .success = false, .sentinel = false },
        { .input = PARCCryptoSuite_HMAC_SHA512, .output = 0,                                                     .success = false, .sentinel = false },
        { .input = 13579,                       .output = 0,                                                     .success = false, .sentinel = false },
        { .input = 0,                           .output = 0,                                                     .success = false, .sentinel = true  },
    };

    for (int i = 0; !vectors[i].sentinel; i++) {
        unsigned output;
        bool success = ccnxCodecSchemaV1CryptoSuite_ParcToTlv(vectors[i].input, &output);
        assertTrue(success == vectors[i].success, "Wrong return value, index %d expected %d got %d", i, vectors[i].success, success);
        if (success) {
            assertTrue(output == vectors[i].output, "Wrong output, index %d expected %u got %u", i, vectors[i].output, output);
        }
    }
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_TlvToParc)
{
    struct test_vector {
        PARCCryptoSuite output;
        CCNxCodecSchemaV1TlvDictionary_CryptoSuite input;
        bool success;
        bool sentinel;
    } vectors[] = {
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256,   .output = PARCCryptoSuite_RSA_SHA256,  .success = true,  .sentinel = false },
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256,  .output = PARCCryptoSuite_HMAC_SHA256, .success = true,  .sentinel = false },
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C,      .output = PARCCryptoSuite_NULL_CRC32C, .success = true,  .sentinel = false },
        { .input = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_EcSecp256K1, .output = 0,                           .success = false, .sentinel = false },
        { .input = 13579,                                                  .output = 0,                           .success = false, .sentinel = false },
        { .input = 0,                                                      .output = 0,                           .success = false, .sentinel = true  },
    };

    for (int i = 0; !vectors[i].sentinel; i++) {
        unsigned output;
        bool success = ccnxCodecSchemaV1CryptoSuite_TlvToParc(vectors[i].input, &output);
        assertTrue(success == vectors[i].success, "Wrong return value, index %d expected %d got %d", i, vectors[i].success, success);
        if (success) {
            assertTrue(output == vectors[i].output, "Wrong output, index %d expected %u got %u", i, vectors[i].output, output);
        }
    }
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv)
{
    struct test_vector {
        PARCSigningAlgorithm signAlg;
        PARCCryptoHashType hashType;
        CCNxCodecSchemaV1TlvDictionary_CryptoSuite output;
        bool success;
        bool sentinel;
    } vectors[] = {
        { .signAlg = PARCSigningAlgorithm_RSA,  .hashType = PARCCryptoHashType_SHA256, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256,  .success = true,  .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_HMAC, .hashType = PARCCryptoHashType_SHA256, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256, .success = true,  .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_NULL, .hashType = PARCCryptoHashType_CRC32C, .output = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C,     .success = true,  .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_RSA,  .hashType = 12345,                     .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_HMAC, .hashType = 12345,                     .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = PARCSigningAlgorithm_NULL, .hashType = 12345,                     .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = 12345,                     .hashType = 12345,                     .output = 0,                                                     .success = false, .sentinel = false },
        { .signAlg = 0,                         .hashType = 0,                         .output = 0,                                                     .success = false, .sentinel = true  },
    };

    for (int i = 0; !vectors[i].sentinel; i++) {
        unsigned output;
        bool success = ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv(vectors[i].signAlg, vectors[i].hashType, &output);
        assertTrue(success == vectors[i].success, "Wrong return value, index %d expected %d got %d", i, vectors[i].success, success);
        if (success) {
            assertTrue(output == vectors[i].output, "Wrong output, index %d expected %u got %u", i, vectors[i].output, output);
        }
    }
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_CryptoSuite);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
