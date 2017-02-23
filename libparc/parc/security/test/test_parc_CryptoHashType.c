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
#include "../parc_CryptoHashType.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_CryptoHashType)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_CryptoHashType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_CryptoHashType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHashType_FromString);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHashType_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcCryptoHashType_FromString)
{
    assertTrue(PARCCryptoHashType_SHA256 == parcCryptoHashType_FromString("PARCCryptoHashType_SHA256"), "Expected true");

    assertTrue(PARCCryptoHashType_SHA512 == parcCryptoHashType_FromString("PARCCryptoHashType_SHA512"), "Expected true");

    assertTrue(PARCCryptoHashType_CRC32C == parcCryptoHashType_FromString("PARCCryptoHashType_CRC32C"), "Expected true");

    assertTrue(PARCCryptoHashType_NULL == parcCryptoHashType_FromString("NULL"), "Expected true");
}

LONGBOW_TEST_CASE(Global, parcCryptoHashType_ToString)
{
    const char *string1 = parcCryptoHashType_ToString(PARCCryptoHashType_SHA256);
    assertNotNull(string1, "Expected non-null result.");

    const char *string2 = parcCryptoHashType_ToString(PARCCryptoHashType_SHA512);
    assertNotNull(string2, "Expected non-null result.");

    const char *string3 = parcCryptoHashType_ToString(PARCCryptoHashType_CRC32C);
    assertNotNull(string3, "Expected non-null result.");

    const char *string4 = parcCryptoHashType_ToString(PARCCryptoHashType_NULL);
    assertNull(string4, "Expected to be null");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_CryptoHashType);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
