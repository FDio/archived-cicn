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
#include "../ccnx_PortalFactory.c"

#include <stdio.h>
#include <unistd.h>

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Memory.h>
#include <parc/security/parc_IdentityFile.h>

#include <ccnx/transport/test_tools/bent_pipe.h>

#include <parc/security/parc_Pkcs12KeyStore.h>

LONGBOW_TEST_RUNNER(test_ccnx_PortalFactory /*, .requires="FeatureLongBowSubProcess"*/)
{
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

LONGBOW_TEST_RUNNER_SETUP(test_ccnx_PortalFactory)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(test_ccnx_PortalFactory)
{
//    LongBowSubProcess *metis = longBowTestRunner_GetClipBoardData(testRunner);
//    longBowSubProcess_Display(0, metis);
//    longBowSubProcess_Destroy(&metis);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, ccnxPortalFactory_Create);
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, ccnxPortalFactory_AcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    unlink("ccnxPortalFactory_keystore");

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, ccnxPortalFactory_Create)
{
    const char *keystoreName = "ccnxPortalFactory_keystore";

    parcSecurity_Init();

    bool success = parcPkcs12KeyStore_CreateFile(keystoreName, "keystore_password", "consumer", 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('%s', 'keystore_password') failed.", keystoreName);

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, "keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);

    CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);
    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);

    ccnxPortalFactory_Release(&factory);

    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(CreateAcquireRelease, ccnxPortalFactory_AcquireRelease)
{
    const char *keystoreName = "ccnxPortalFactory_keystore";

    parcSecurity_Init();

    bool success = parcPkcs12KeyStore_CreateFile(keystoreName, "keystore_password", "consumer", 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('%s', 'keystore_password') failed.", keystoreName);

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, "keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);

    CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);
    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);

    CCNxPortalFactory *reference = ccnxPortalFactory_Acquire(factory);
    assertTrue(factory == reference, "Expected Acquire to return its argument.");

    ccnxPortalFactory_Release(&factory);
    ccnxPortalFactory_Release(&reference);

    parcSecurity_Fini();
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalFactory_GetIdentity);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalFactory_GetKeyId);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    unlink("ccnxPortalFactory_keystore");

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxPortalFactory_GetIdentity)
{
    const char *keystoreName = "ccnxPortalFactory_keystore";

    parcSecurity_Init();
    bool success = parcPkcs12KeyStore_CreateFile(keystoreName, "keystore_password", "consumer", 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('%s', 'keystore_password') failed.", keystoreName);

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, "keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);

    CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);

    const PARCIdentity *actual = ccnxPortalFactory_GetIdentity(factory);

    assertTrue(identity == actual, "Expected the result to be the same as provided to the constructor");

    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);

    ccnxPortalFactory_Release(&factory);
    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(Global, ccnxPortalFactory_GetKeyId)
{
    const char *keystoreName = "ccnxPortalFactory_keystore";

    parcSecurity_Init();
    bool success = parcPkcs12KeyStore_CreateFile(keystoreName, "keystore_password", "consumer", 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('%s', 'keystore_password') failed.", keystoreName);

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, "keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);

    CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);

    const PARCKeyId *actual = ccnxPortalFactory_GetKeyId(factory);

    PARCSigner *signer = parcIdentity_CreateSigner(identity);
    PARCKeyId *expected = parcSigner_CreateKeyId(signer);
    parcSigner_Release(&signer);

    assertTrue(parcKeyId_Equals(expected, actual), "KeyIds not equal");

    parcKeyId_Release(&expected);

    ccnxPortalFactory_Release(&factory);

    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);

    parcSecurity_Fini();
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, ccnxPortalFactory_Create_NULL_Identity);
}

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    unlink("ccnxPortalFactory_keystore");

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Errors, ccnxPortalFactory_Create_NULL_Identity, .event = &LongBowTrapInvalidValue)
{
    CCNxPortalFactory *factory = ccnxPortalFactory_Create(NULL);

    ccnxPortalFactory_Release(&factory);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_ccnx_PortalFactory);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
