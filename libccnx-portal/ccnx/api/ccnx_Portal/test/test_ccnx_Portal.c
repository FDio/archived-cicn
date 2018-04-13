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

#include <config.h>
#include "../ccnx_Portal.c"

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <stdio.h>
#include <sys/errno.h>

#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalAPI.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

#include <parc/developer/parc_Stopwatch.h>

#include <ccnx/transport/test_tools/bent_pipe.h>

#include <parc/security/parc_IdentityFile.h>
#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>

//#define USE_APILOOPBACK

#ifdef USE_APILOOPBACK
#  define TEST_STACK ccnxPortalAPI_LoopBack
#else
#  define TEST_STACK ccnxPortalRTA_LoopBack
#endif

LONGBOW_TEST_RUNNER(test_ccnx_Portal /*, .requires="FeatureLongBowSubProcess"*/)
{
    LONGBOW_RUN_TEST_FIXTURE(Performance);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

#define perTestCase

LONGBOW_TEST_RUNNER_SETUP(test_ccnx_Portal)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(test_ccnx_Portal)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Open);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Open_NonBlocking);

    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Send);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_GetStatus);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_GetError);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_GetFileId);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Listen);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Ignore);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_GetKeyId);

    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_IsEOF);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_IsError);

//    LONGBOW_RUN_TEST_CASE(Global, Hello);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Receive_NeverTimeout);
//    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Receive_NeverTimeout_Hang);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Receive_ImmediateTimeout);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Receive_ImmediateTimeout_NoData);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Receive_5SecondTimeout);

    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Send_NeverTimeout);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Send_ImmediateTimeout);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortal_Send_ImmediateTimeout_WouldBlock);
}

static uint32_t InitialMemoryOutstanding = 0;

typedef struct test_data {
    BentPipeState *bentpipe;
    CCNxPortalFactory *factory;
} TestData;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    InitialMemoryOutstanding = parcMemory_Outstanding();
    if (InitialMemoryOutstanding != 0) {
        printf("Global fixture setup has outstanding allocations %u\n", InitialMemoryOutstanding);
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    }

    TestData *data = parcMemory_Allocate(sizeof(TestData));

    char bent_pipe_name[1024];
    static const char bent_pipe_format[] = "/tmp/test_ccnx_Portal%d.sock";
    sprintf(bent_pipe_name, bent_pipe_format, getpid());
    unlink(bent_pipe_name);
    setenv("BENT_PIPE_NAME", bent_pipe_name, 1);

    data->bentpipe = bentpipe_Create(bent_pipe_name);
    bentpipe_Start(data->bentpipe);

    unsigned int keyLength = 1024;
    unsigned int validityDays = 30;
    char *subjectName = "test_ccnx_Portal";

    parcSecurity_Init();

    bool success = parcPkcs12KeyStore_CreateFile("my_keystore", "my_keystore_password", subjectName, PARCSigningAlgorithm_RSA, keyLength, validityDays);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('my_keystore', 'my_keystore_password') failed.");

    PARCIdentityFile *identityFile = parcIdentityFile_Create("my_keystore", "my_keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    parcIdentityFile_Release(&identityFile);

    data->factory = ccnxPortalFactory_Create(identity, PARCCryptoSuite_RSA_SHA256);
    parcIdentity_Release(&identity);

    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    sleep(2);
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxPortalFactory_Release(&data->factory);

    bentpipe_Stop(data->bentpipe);
    bentpipe_Destroy(&data->bentpipe);

    parcMemory_Deallocate((void **) &data);
    unsetenv("BENT_PIPE_NAME");
    parcSecurity_Fini();

    if (parcMemory_Outstanding() != InitialMemoryOutstanding) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("('%s' leaks memory by %d\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - InitialMemoryOutstanding);
//        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

char *keyStoreFileName = "/tmp/test_ccnx_Portal.keystore";
char *keyStorePassWord = "password";

LONGBOW_TEST_CASE(Global, ccnxPortal_Open)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    parcObjectTesting_AssertAcquire(portal);

    ccnxPortal_Release(&portal);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Open_NonBlocking)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);


    parcObjectTesting_AssertAcquire(portal);

    ccnxPortal_Release(&portal);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Send)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);
    bool actual = ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxPortal_Flush(portal, CCNxStackTimeout_Never);

    ccnxMetaMessage_Release(&message);
    ccnxInterest_Release(&interest);

    ccnxPortal_Release(&portal);

    assertTrue(actual, "Expected ccnxPortal_Send to be successful.");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_GetStatus)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);

    ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxPortal_Flush(portal, CCNxStackTimeout_Never);

    const CCNxPortalStatus *status = ccnxPortal_GetStatus(portal);

    ccnxMetaMessage_Release(&message);
    ccnxInterest_Release(&interest);

    ccnxPortal_Release(&portal);

    assertNotNull(status, "Expected non-null result from ccnxPortal_GetStatus");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_GetError)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);

    ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxPortal_Flush(portal, CCNxStackTimeout_Never);

    const int error = ccnxPortal_GetError(portal);

    ccnxMetaMessage_Release(&message);
    ccnxInterest_Release(&interest);

    ccnxPortal_Release(&portal);

    assertTrue(error == 0, "Expected 0 result from ccnxPortal_GetError");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_GetFileId)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);
    ccnxInterest_Release(&interest);

    ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxMetaMessage_Release(&message);
    ccnxPortal_Flush(portal, CCNxStackTimeout_Never);
    int fileId = ccnxPortal_GetFileId(portal);
    assertTrue(fileId != -1, "Expected ccnxPortal_GetFileId to not return -1");
    ccnxPortal_Release(&portal);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_IsEOF)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);

    ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxPortal_Flush(portal, CCNxStackTimeout_Never);

    bool actual = ccnxPortal_IsEOF(portal);

    ccnxInterest_Release(&interest);
    ccnxMetaMessage_Release(&message);

    ccnxPortal_Release(&portal);

    assertFalse(actual, "Expected to not be at EOF");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_IsError)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);

    ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);
    ccnxPortal_Flush(portal, CCNxStackTimeout_Never);

    bool actual = ccnxPortal_IsError(portal);

    ccnxMetaMessage_Release(&message);
    ccnxInterest_Release(&interest);

    ccnxPortal_Release(&portal);

    assertFalse(actual, "Expected not to have an error status");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Listen)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    bool actual = ccnxPortal_Listen(portal, name, 60, CCNxStackTimeout_Never);

    ccnxName_Release(&name);

    ccnxPortal_Release(&portal);

    assertTrue(actual, "Expected ccnxPortal_Listen to return true");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Ignore)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    bool actual = ccnxPortal_Ignore(portal, name, CCNxStackTimeout_Never);
    ccnxName_Release(&name);

    ccnxPortal_Release(&portal);

    assertTrue(actual, "Expected ccnxPortal_Ignore to return true");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_GetKeyId)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    const PARCKeyId *actual = ccnxPortal_GetKeyId(portal);
    const PARCKeyId *expected = ccnxPortalFactory_GetKeyId(data->factory);

    ccnxPortal_Release(&portal);

    assertTrue(parcKeyId_Equals(actual, expected), "Expected the PARCKeyId instances to be equal.");
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Send_NeverTimeout)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portalOut = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);
    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *interestMessage = ccnxMetaMessage_CreateFromInterest(interest);
    ccnxInterest_Release(&interest);

    if (ccnxPortal_Send(portalOut, interestMessage, CCNxStackTimeout_Never)) {
        ccnxMetaMessage_Release(&interestMessage);
        CCNxMetaMessage *message = ccnxPortal_Receive(portalIn, CCNxStackTimeout_Never);
        ccnxMetaMessage_Release(&message);
    }

    ccnxPortal_Release(&portalIn);
    ccnxPortal_Release(&portalOut);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Send_ImmediateTimeout)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portalOut = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);
    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *interestMessage = ccnxMetaMessage_CreateFromInterest(interest);
    ccnxInterest_Release(&interest);

    if (ccnxPortal_Send(portalOut, interestMessage, CCNxStackTimeout_Immediate)) {
        ccnxMetaMessage_Release(&interestMessage);
        CCNxMetaMessage *message = ccnxPortal_Receive(portalIn, CCNxStackTimeout_Never);
        ccnxMetaMessage_Release(&message);
    }

    ccnxPortal_Release(&portalIn);
    ccnxPortal_Release(&portalOut);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Send_ImmediateTimeout_WouldBlock)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portalOut = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *interestMessage = ccnxMetaMessage_CreateFromInterest(interest);

    for (int count = 0; count < 10000; count++) {
        if (ccnxPortal_Send(portalOut, interestMessage, CCNxStackTimeout_Immediate) == false) {
            break;
        }
        count++;
    }

    assertFalse(ccnxPortal_Send(portalOut, interestMessage, CCNxStackTimeout_Immediate),
                "Expected send to fail due to blocking");

    ccnxMetaMessage_Release(&interestMessage);
    ccnxInterest_Release(&interest);
    ccnxPortal_Release(&portalOut);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Receive_NeverTimeout)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portalOut = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);
    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *interestMessage = ccnxMetaMessage_CreateFromInterest(interest);
    ccnxInterest_Release(&interest);

    if (ccnxPortal_Send(portalOut, interestMessage, CCNxStackTimeout_Never)) {
        ccnxMetaMessage_Release(&interestMessage);
        CCNxMetaMessage *message = ccnxPortal_Receive(portalIn, CCNxStackTimeout_Never);
        ccnxMetaMessage_Release(&message);
    }

    ccnxPortal_Release(&portalIn);
    ccnxPortal_Release(&portalOut);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Receive_NeverTimeout_Hang)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxMetaMessage *message = ccnxPortal_Receive(portalIn, CCNxStackTimeout_Never);
    ccnxMetaMessage_Release(&message);
    ccnxPortal_Release(&portalIn);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Receive_ImmediateTimeout)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portalOut = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);
    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *interestMessage = ccnxMetaMessage_CreateFromInterest(interest);

    if (ccnxPortal_Send(portalOut, interestMessage, CCNxStackTimeout_Never)) {
        sleep(2);
        ccnxMetaMessage_Release(&interestMessage);
        CCNxMetaMessage *message = ccnxPortal_Receive(portalIn, CCNxStackTimeout_Immediate);

        assertTrue(ccnxInterest_Equals(interest, ccnxMetaMessage_GetInterest(message)), "Expected Interest to be received.");
        ccnxMetaMessage_Release(&message);
    }

    ccnxInterest_Release(&interest);
    ccnxPortal_Release(&portalIn);
    ccnxPortal_Release(&portalOut);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Receive_ImmediateTimeout_NoData)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    ccnxPortal_Receive(portalIn, CCNxStackTimeout_Immediate);
    assertTrue(errno == ENOMSG, "Expected errno to be set to ENOMSG, actual %s", strerror(errno));

    ccnxPortal_Release(&portalIn);
}

LONGBOW_TEST_CASE(Global, ccnxPortal_Receive_5SecondTimeout)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    ccnxPortal_Receive(portalIn, CCNxStackTimeout_MicroSeconds(5000000));
    assertTrue(errno == ENOMSG, "Expected errno to be set to ENOMSG, actual %s", strerror(errno));

    ccnxPortal_Release(&portalIn);
}

LONGBOW_TEST_CASE(Global, Hello)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);
    CCNxPortal *portalIn = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    assertNotNull(portal, "Expected a non-null CCNxPortal pointer.");

    CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    CCNxMetaMessage *interestMessage = ccnxMetaMessage_CreateFromInterest(interest);

    if (ccnxPortal_Send(portal, interestMessage, CCNxStackTimeout_Never)) {
        for (int responses = 0; responses == 0; ) {
            CCNxMetaMessage *message = ccnxPortal_Receive(portalIn, CCNxStackTimeout_Never);
            if (message != NULL) {
                if (ccnxMetaMessage_IsContentObject(message)) {
                    CCNxContentObject *contentObject = ccnxMetaMessage_GetContentObject(message);

                    PARCBuffer *payload = ccnxContentObject_GetPayload(contentObject);
                    if (parcBuffer_HasRemaining(payload) == false) {
                        fprintf(stderr, "**************** Content object has arrived WITH EMPTY CONTENT\n");
                    } else {
                        char *string = parcBuffer_ToString(payload);
                        fprintf(stderr, "**************** Content object has arrived: %s\n", string);
                        parcMemory_Deallocate((void **) &string);
                    }
                    responses++;
                }
                ccnxMetaMessage_Release(&message);
            }
        }
    }

    ccnxMetaMessage_Release(&interestMessage);
    ccnxPortal_Release(&portal);
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, ccnxPortal_SendReceive);
    LONGBOW_RUN_TEST_CASE(Performance, ccnxPortalFactory_CreatePortal);
    LONGBOW_RUN_TEST_CASE(Performance, ccnxPortal_Send);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    InitialMemoryOutstanding = parcMemory_Outstanding();

    TestData *data = parcMemory_Allocate(sizeof(TestData));

    char bent_pipe_name[1024];
    static const char bent_pipe_format[] = "/tmp/test_ccnx_Portal%d.sock";
    sprintf(bent_pipe_name, bent_pipe_format, getpid());
    setenv("BENT_PIPE_NAME", bent_pipe_name, 1);

    data->bentpipe = bentpipe_Create(bent_pipe_name);
    bentpipe_Start(data->bentpipe);

    longBowTestRunner_SetClipBoardData(testRunner, data->bentpipe);

    unsigned int keyLength = 1024;
    unsigned int validityDays = 30;
    char *subjectName = "test_ccnx_Comm";

    parcSecurity_Init();

    bool success = parcPkcs12KeyStore_CreateFile("my_keystore", "my_keystore_password", subjectName, PARCSigningAlgorithm_RSA, keyLength, validityDays);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('my_keystore', 'my_keystore_password') failed.");

    PARCIdentityFile *identityFile = parcIdentityFile_Create("my_keystore", "my_keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    parcIdentityFile_Release(&identityFile);

    data->factory = ccnxPortalFactory_Create(identity, PARCCryptoSuite_RSA_SHA256);
    parcIdentity_Release(&identity);

    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxPortalFactory_Release(&data->factory);
    bentpipe_Stop(data->bentpipe);
    bentpipe_Destroy(&data->bentpipe);
    parcMemory_Deallocate((void **) &data);
    unsetenv("BENT_PIPE_NAME");
    parcSecurity_Fini();

    if (parcMemory_Outstanding() != InitialMemoryOutstanding) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("('%s' leaks memory by %d\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - InitialMemoryOutstanding);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Performance, ccnxPortalFactory_CreatePortal)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    for (int i = 0; i < 1000; i++) {
        CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);
        ccnxPortal_Release(&portal);
    }
}

LONGBOW_TEST_CASE(Performance, ccnxPortal_Send)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    for (int i = 0; i < 100000; i++) {
        ccnxPortal_Flush(portal, CCNxStackTimeout_Never);
    }

    ccnxPortal_Release(&portal);
}

typedef struct parc_ewma {
    bool initialized;
    int64_t value;
    double coefficient;
} PARCEWMA;

PARCEWMA *
parcEWMA_Create(double coefficient)
{
    PARCEWMA *result = parcMemory_AllocateAndClear(sizeof(PARCEWMA));
    if (result != NULL) {
        result->initialized = false;
        result->value = 0;
        result->coefficient = coefficient;
    }

    return result;
}

void
parcEWMW_Destroy(PARCEWMA **ewma)
{
    parcMemory_Deallocate(ewma);
}

int64_t
parcEWMA_Update(PARCEWMA *ewma, int64_t value)
{
    if (ewma->initialized) {
        ewma->value = ((value + ewma->coefficient * ewma->value - ewma->value) / ewma->coefficient);
    } else {
        ewma->value = value;
        ewma->initialized = true;
    }
    return ewma->value;
}

int64_t
parcEWMA_GetValue(const PARCEWMA *ewma)
{
    return ewma->value;
}

static uint64_t
sendx(CCNxPortal *portalOut, uint32_t index, const CCNxName *name)
{
    PARCStopwatch *timer = parcStopwatch_Create();

    parcStopwatch_Start(timer);

    CCNxInterest *interest = ccnxInterest_CreateSimple(name);

    PARCBuffer *payload = parcBuffer_Allocate(sizeof(uint64_t) + sizeof(uint32_t));
    parcBuffer_PutUint32(payload, index);
    struct timeval tv;
    gettimeofday(&tv, 0);
    uint64_t theTime = tv.tv_sec * 1000000 + tv.tv_usec;
    parcBuffer_PutUint64(payload, theTime);
    parcBuffer_Flip(payload);

    ccnxInterest_SetPayload(interest, payload);

    CCNxMetaMessage *interestMessage = ccnxMetaMessage_CreateFromInterest(interest);

    ccnxPortal_Send(portalOut, interestMessage, CCNxStackTimeout_Never);

    parcBuffer_Release(&payload);

    ccnxMetaMessage_Release(&interestMessage);
    ccnxInterest_Release(&interest);

    uint64_t result = parcStopwatch_ElapsedTimeNanos(timer);
    parcStopwatch_Release(&timer);
    return result;
}

static void *
sender(void *data)
{
    CCNxPortal *portalOut = data;

    PARCEWMA *ewma = parcEWMA_Create(0.75);
    CCNxName *name = ccnxName_CreateFormatString("lci:/local/trace");

    for (uint32_t i = 300; i != 0; i--) {
        uint64_t elapsedTime = sendx(portalOut, i, name);
        parcEWMA_Update(ewma, elapsedTime);
    }
    uint64_t elapsedTime = sendx(portalOut, 0, name);
    parcEWMA_Update(ewma, elapsedTime);

    printf("sender %9" PRId64 " us/message\n", parcEWMA_GetValue(ewma));

    parcEWMW_Destroy(&ewma);
    ccnxName_Release(&name);
    return 0;
}

static void *
receiver(void *data)
{
    CCNxPortal *portalIn = data;

    uint32_t index;
    PARCEWMA *ewma = parcEWMA_Create(0.75);
    PARCEWMA *roundTrip = parcEWMA_Create(0.75);

    PARCStopwatch *timer = parcStopwatch_Create();
    do {
        struct timeval tv;
        gettimeofday(&tv, 0);
        uint64_t theTime = tv.tv_sec * 1000000 + tv.tv_usec;

        parcStopwatch_Start(timer);
        CCNxMetaMessage *message = ccnxPortal_Receive(portalIn, CCNxStackTimeout_Never);

        PARCBuffer *payload = ccnxInterest_GetPayload(ccnxMetaMessage_GetInterest(message));

        index = parcBuffer_GetUint32(payload);

        parcEWMA_Update(roundTrip, theTime - parcBuffer_GetUint64(payload));

        parcEWMA_Update(ewma, parcStopwatch_ElapsedTimeNanos(timer));

        ccnxMetaMessage_Release(&message);
    } while (index != 0);

    printf("receiver %9" PRId64 " us/message %9" PRId64 " us\n", parcEWMA_GetValue(ewma), parcEWMA_GetValue(roundTrip));

    parcStopwatch_Release(&timer);
    parcEWMW_Destroy(&roundTrip);
    parcEWMW_Destroy(&ewma);

    return 0;
}

LONGBOW_TEST_CASE(Performance, ccnxPortal_SendReceive)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxPortal *portalSend = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);
    CCNxPortal *portalReceive = ccnxPortalFactory_CreatePortal(data->factory, TEST_STACK);

    pthread_t thread_receiver1;
    pthread_t thread_sender;
    pthread_create(&thread_receiver1, NULL, receiver, portalReceive);
    pthread_create(&thread_sender, NULL, sender, portalSend);

    pthread_join(thread_receiver1, NULL);

    ccnxPortal_Flush(portalSend, CCNxStackTimeout_Never);
    ccnxPortal_Flush(portalReceive, CCNxStackTimeout_Never);
    ccnxPortal_Release(&portalSend);
    ccnxPortal_Release(&portalReceive);
    sleep(2);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_ccnx_Portal);
    int exitStatus = longBowMain(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
