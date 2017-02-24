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
#include "../parc_Verifier.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_Verifier)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Verifier)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Verifier)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcVerifier_AddKey);
    LONGBOW_RUN_TEST_CASE(Global, parcVerifier_AllowedCryptoSuite);
    LONGBOW_RUN_TEST_CASE(Global, parcVerifier_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcVerifier_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parcVerifier_GetCryptoHasher);
    LONGBOW_RUN_TEST_CASE(Global, parcVerifier_RemoveKeyId);
    LONGBOW_RUN_TEST_CASE(Global, parcVerifier_VerifySignature);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcVerifier_AddKey)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcVerifier_AllowedCryptoSuite)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcVerifier_Create)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcVerifier_Destroy)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcVerifier_GetCryptoHasher)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcVerifier_RemoveKeyId)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcVerifier_VerifySignature)
{
    testUnimplemented("");
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Verifier);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
