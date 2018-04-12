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
 * @header <#Headline Name#>
 * @abstract <#Abstract#>
 * @discussion
 *     <#Discussion#>
 *
 */
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Identity.c"
#include "../parc_IdentityFile.h"
#include "../parc_Security.h"

#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(test_parc_Identity)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_Identity)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_Identity)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcIdentity_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentity_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentity_GetFileName);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentity_GetPassWord);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentity_CreateSigner);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentity_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentity_Display);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("Tests leak memory by %d allocations\n", outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcIdentity_Create)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    assertNotNull(identityFile, "Expected non-null");

    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    assertNotNull(identity, "Expected non-null");
    parcIdentityFile_Release(&identityFile);

    parcIdentity_Release(&identity);
}

LONGBOW_TEST_CASE(Global, parcIdentity_Acquire)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    assertNotNull(identityFile, "Expected non-null");

    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    assertNotNull(identity, "Expected non-null");

    parcObjectTesting_AssertAcquireReleaseContract(parcIdentity_Acquire, identity);

    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);
}

LONGBOW_TEST_CASE(Global, parcIdentity_GetFileName)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    assertNotNull(identityFile, "Expected non-null");

    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    assertNotNull(identity, "Expected non-null");

    assertEqualStrings(keystoreName, parcIdentity_GetFileName(identity));

    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);
}

LONGBOW_TEST_CASE(Global, parcIdentity_GetPassWord)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    assertNotNull(identityFile, "Expected non-null");

    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    assertNotNull(identity, "Expected non-null");

    assertEqualStrings(keystorePassword, parcIdentity_GetPassWord(identity));

    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);
}

LONGBOW_TEST_CASE(Global, parcIdentity_CreateSigner)
{
    parcSecurity_Init();

    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    assertNotNull(identityFile, "Expected non-null");

    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    assertNotNull(identity, "Expected non-null");

    assertEqualStrings(keystorePassword, parcIdentity_GetPassWord(identity));

    PARCSigner *signer = parcIdentity_CreateSigner(identity, PARCCryptoSuite_RSA_SHA256);

    assertNotNull(signer, "Expected non-null");

    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);
    parcSigner_Release(&signer);

    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(Global, parcIdentity_Equals)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFileX = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentity *x = parcIdentity_Create(identityFileX, PARCIdentityFileAsPARCIdentity);
    PARCIdentityFile *identityFileY = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentity *y = parcIdentity_Create(identityFileY, PARCIdentityFileAsPARCIdentity);
    PARCIdentityFile *identityFileZ = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentity *z = parcIdentity_Create(identityFileZ, PARCIdentityFileAsPARCIdentity);

    PARCIdentityFile *identityFile1 = parcIdentityFile_Create("foo", keystorePassword);
    PARCIdentityFile *identityFile2 = parcIdentityFile_Create(keystoreName, "bar");
    PARCIdentity *u1 = parcIdentity_Create(identityFile1, PARCIdentityFileAsPARCIdentity);
    PARCIdentity *u2 = parcIdentity_Create(identityFile2, PARCIdentityFileAsPARCIdentity);

    parcObjectTesting_AssertEqualsFunction(parcIdentity_Equals, x, y, z, u1, u2);

    parcIdentityFile_Release(&identityFileX);
    parcIdentityFile_Release(&identityFileY);
    parcIdentityFile_Release(&identityFileZ);
    parcIdentityFile_Release(&identityFile1);
    parcIdentityFile_Release(&identityFile2);

    parcIdentity_Release(&x);
    parcIdentity_Release(&y);
    parcIdentity_Release(&z);
    parcIdentity_Release(&u1);
    parcIdentity_Release(&u2);
}

LONGBOW_TEST_CASE(Global, parcIdentity_Display)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    assertNotNull(identityFile, "Expected non-null");

    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    assertNotNull(identity, "Expected non-null");

    parcIdentity_Display(identity, 0);

    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_Identity);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
