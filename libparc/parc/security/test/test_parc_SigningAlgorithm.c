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

/** *
 */
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_SigningAlgorithm.c"
#include <parc/security/parc_CryptoSuite.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

LONGBOW_TEST_RUNNER(test_parc_SigningAlgorithm)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_SigningAlgorithm)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_SigningAlgorithm)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSigningAlgorithm_ToFromString);
    LONGBOW_RUN_TEST_CASE(Global, parcSigningAlgorithm_ToFromString_NotFound);
    LONGBOW_RUN_TEST_CASE(Global, parcSigningAlgorithm_GetSigningAlgorithm);
    LONGBOW_RUN_TEST_CASE(Global, parcSigningAlgorithm_FromString_NotFound);
    LONGBOW_RUN_TEST_CASE(Global, parcSigningAlgorithm_GetSigningAlgorithm_BadAlgorithm);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcSigningAlgorithm_ToFromString)
{
    PARCSigningAlgorithm expected = PARCSigningAlgorithm_HMAC;

    const char *string = parcSigningAlgorithm_ToString(expected);

    PARCSigningAlgorithm actual = parcSigningAlgorithm_FromString(string);

    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcSigningAlgorithm_ToFromString_NotFound)
{
    PARCSigningAlgorithm expected = 123456;

    const char *string = parcSigningAlgorithm_ToString(expected);

    assertNull(string, "Expect parcSigningAlgorithm_ToString to return NULL");
}

LONGBOW_TEST_CASE(Global, parcSigningAlgorithm_FromString_NotFound)
{
    PARCSigningAlgorithm actual = parcSigningAlgorithm_FromString("garbage string of unknown stuff");

    assertTrue(actual == PARCSigningAlgorithm_UNKNOWN,
               "Expect parcSigningAlgorithm_FromString to return PARCSigningAlgorithm_UNKNOWN");
}

LONGBOW_TEST_CASE(Global, parcSigningAlgorithm_GetSigningAlgorithm)
{
    PARCSigningAlgorithm actual = parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite_DSA_SHA256);
    assertTrue(PARCSigningAlgorithm_DSA == actual, "Expected %d, actual %d", PARCSigningAlgorithm_DSA, actual);

    actual = parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite_RSA_SHA256);
    assertTrue(PARCSigningAlgorithm_RSA == actual, "Expected %d, actual %d", PARCSigningAlgorithm_RSA, actual);

    actual = parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite_RSA_SHA512);
    assertTrue(PARCSigningAlgorithm_RSA == actual, "Expected %d, actual %d", PARCSigningAlgorithm_RSA, actual);

    actual = parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite_HMAC_SHA256);
    assertTrue(PARCSigningAlgorithm_HMAC == actual, "Expected %d, actual %d", PARCSigningAlgorithm_HMAC, actual);

    actual = parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite_HMAC_SHA512);
    assertTrue(PARCSigningAlgorithm_HMAC == actual, "Expected %d, actual %d", PARCSigningAlgorithm_HMAC, actual);

    actual = parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite_NULL_CRC32C);
    assertTrue(PARCSigningAlgorithm_NULL == actual, "Expected %d, actual %d", PARCSigningAlgorithm_NULL, actual);
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcSigningAlgorithm_GetSigningAlgorithm_BadAlgorithm, .event = &LongBowTrapIllegalValue)
{
    parcCryptoSuite_GetSigningAlgorithm(-1);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_SigningAlgorithm);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
