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
#include "../parc_CryptoSuite.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_CryptoSuite)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_CryptoSuite)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_CryptoSuite)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoSuite_GetCryptoHash);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoSuite_GetCryptoHash_IllegalValue);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcCryptoSuite_GetCryptoHash)
{
    assertTrue(PARCCryptoHashType_SHA256 == parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_RSA_SHA256), "Expected to be true");
    assertTrue(PARCCryptoHashType_SHA256 == parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_DSA_SHA256), "Expected to be true");
    assertTrue(PARCCryptoHashType_SHA512 == parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_RSA_SHA512), "Expected to be true");
    assertTrue(PARCCryptoHashType_SHA256 == parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_HMAC_SHA256), "Expected to be true");
    assertTrue(PARCCryptoHashType_SHA512 == parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_HMAC_SHA512), "Expected to be true");
    assertTrue(PARCCryptoHashType_CRC32C == parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_NULL_CRC32C), "Expected to be true");
    assertTrue(PARCCryptoHashType_SHA256 == parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_EC_SECP_256K1), "Expected to be true");
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcCryptoSuite_GetCryptoHash_IllegalValue, .event = &LongBowTrapIllegalValue)
{
    parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_UNKNOWN);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_CryptoSuite);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
