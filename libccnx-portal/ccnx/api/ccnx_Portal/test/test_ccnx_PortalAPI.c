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
 */
#include "../ccnx_Portal.c"

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <stdio.h>
#include <unistd.h>

#include <ccnx/api/ccnx_Portal/ccnx_PortalAPI.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/security/parc_IdentityFile.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>

LONGBOW_TEST_RUNNER(test_ccnx_PortalAPI /*, .requires="FeatureLongBowSubProcess"*/)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(test_ccnx_PortalAPI)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(test_ccnx_PortalAPI)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalAPI_CreateRelease);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalAPI_SendReceive);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalAPI_GetFileId);
}

static size_t InitialMemoryOutstanding = 0;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    InitialMemoryOutstanding = parcMemory_Outstanding();

    unsigned int keyLength = 1024;
    unsigned int validityDays = 30;
    char *subjectName = "test_ccnx_Comm";

    parcSecurity_Init();

    bool success = parcPkcs12KeyStore_CreateFile("my_keystore", "my_keystore_password", subjectName, keyLength, validityDays);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('my_keystore', 'my_keystore_password') failed.");

    PARCIdentityFile *identityFile = parcIdentityFile_Create("my_keystore", "my_keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);

    CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);
    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);

    longBowTestCase_SetClipBoardData(testCase, factory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    CCNxPortalFactory *factory = longBowTestCase_GetClipBoardData(testCase);
    ccnxPortalFactory_Release(&factory);

    parcSecurity_Fini();

    if (parcMemory_Outstanding() != InitialMemoryOutstanding) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("('%s' leaks memory by %zd\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - InitialMemoryOutstanding);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxPortalAPI_CreateRelease)
{
    CCNxPortalFactory *factory = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalAPI_LoopBack);
    assertNotNull(portal, "Expected a portal");

    ccnxPortal_Release(&portal);
    assertNull(portal, "Expected a null portal");
}

LONGBOW_TEST_CASE(Global, ccnxPortalAPI_SendReceive)
{
    CCNxPortalFactory *factory = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalAPI_LoopBack);

    // Send Hello
    CCNxName *name1 = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *sentInterest1 = ccnxInterest_CreateSimple(name1);
    ccnxName_Release(&name1);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(sentInterest1);
    ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxMetaMessage_Release(&message);

    // Send Goodbye. We want to make sure these arrive in that order.
    CCNxName *name2 = ccnxName_CreateFromCString("lci:/Goodbye/World");
    CCNxInterest *sentInterest2 = ccnxInterest_CreateSimple(name2);
    ccnxName_Release(&name2);

    message = ccnxMetaMessage_CreateFromInterest(sentInterest2);
    ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxMetaMessage_Release(&message);

    // Now verify that they arrive in Hello, Goodbye order.

    CCNxMetaMessage *receivedMessage = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);

    CCNxInterest *receivedInterest1 = ccnxMetaMessage_GetInterest(receivedMessage);
    assertTrue(ccnxInterest_Equals(sentInterest1, receivedInterest1), "Expected the Hello interest");
    ccnxMetaMessage_Release(&receivedMessage);
    ccnxInterest_Release(&sentInterest1);

    receivedMessage = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);
    CCNxInterest *receivedInterest2 = ccnxMetaMessage_GetInterest(receivedMessage);
    assertTrue(ccnxInterest_Equals(sentInterest2, receivedInterest2), "Expected the Goodbye interest");
    ccnxMetaMessage_Release(&receivedMessage);
    ccnxInterest_Release(&sentInterest2);

    ccnxPortal_Release(&portal);
}

LONGBOW_TEST_CASE(Global, ccnxPortalAPI_GetFileId)
{
    CCNxPortalFactory *factory = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalAPI_LoopBack);

    int fileId = ccnxPortal_GetFileId(portal);

    assertTrue(fileId != -1, "Expected file-id to not be -1");

    ccnxPortal_Release(&portal);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_ccnx_PortalAPI);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
