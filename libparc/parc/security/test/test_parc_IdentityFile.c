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
#include "../parc_IdentityFile.c"
#include "../parc_Security.h"

#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_IdentityFile)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_IdentityFile)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_IdentityFile)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_GetFileName);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_GetPassWord);
    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_CreateSigner);
//    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_Display);
//    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_Exists_True);
//    LONGBOW_RUN_TEST_CASE(Global, parcIdentityFile_Exists_False);
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

LONGBOW_TEST_CASE(Global, parcIdentityFile_Acquire)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    assertNotNull(identityFile, "Expected non-null");

    parcObjectTesting_AssertAcquireReleaseContract(parcIdentityFile_Acquire, identityFile);

    parcIdentityFile_Release(&identityFile);
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_Create)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    assertNotNull(identityFile, "Expected non-null");

    parcIdentityFile_Release(&identityFile);
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_GetFileName)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    assertNotNull(identityFile, "Expected non-null");

    assertEqualStrings(keystoreName, parcIdentityFile_GetFileName(identityFile));

    parcIdentityFile_Release(&identityFile);
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_GetPassWord)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    assertNotNull(identityFile, "Expected non-null");

    assertEqualStrings(keystorePassword, parcIdentityFile_GetPassWord(identityFile));

    parcIdentityFile_Release(&identityFile);
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_Exists_True)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    bool actual = parcIdentityFile_Exists(identityFile);

    assertTrue(actual, "Expected %s to exist.", parcIdentityFile_GetFileName(identityFile));

    parcIdentityFile_Release(&identityFile);
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_Exists_False)
{
    const char *keystoreName = "/dev/notgoingtoexist";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    bool actual = parcIdentityFile_Exists(identityFile);

    assertFalse(actual, "Expected %s to not exist.", parcIdentityFile_GetFileName(identityFile));

    parcIdentityFile_Release(&identityFile);
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_CreateSigner)
{
    parcSecurity_Init();

    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    assertNotNull(identityFile, "Expected non-null");

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(stdout, "Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
    PARCSigner *signer = parcIdentityFile_CreateSigner(identityFile);

    assertNotNull(signer, "Expected non-null");

    parcIdentityFile_Release(&identityFile);
    parcSigner_Release(&signer);

    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_Release)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);

    assertNotNull(identityFile, "Expected non-null");

    parcIdentityFile_Release(&identityFile);
    assertNull(identityFile, "Identity File was not nulled out after Release()");
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_Equals)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *x = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentityFile *y = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentityFile *z = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentityFile *u1 = parcIdentityFile_Create("foo", keystorePassword);
    PARCIdentityFile *u2 = parcIdentityFile_Create(keystoreName, "bar");

    parcObjectTesting_AssertEqualsFunction(parcIdentityFile_Equals, x, y, z, u1, u2);

    parcIdentityFile_Release(&x);
    parcIdentityFile_Release(&y);
    parcIdentityFile_Release(&z);
    parcIdentityFile_Release(&u1);
    parcIdentityFile_Release(&u2);
}

LONGBOW_TEST_CASE(Global, parcIdentityFile_Display)
{
    const char *keystoreName = "test_rsa.p12";
    const char *keystorePassword = "blueberry";

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    assertNotNull(identityFile, "Expected non-null");

    parcIdentityFile_Display(identityFile, 0);

    parcIdentityFile_Release(&identityFile);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_IdentityFile);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
