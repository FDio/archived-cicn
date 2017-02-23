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
#include "../ccnx_PortalStack.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <ccnx/api/ccnx_Portal/ccnx_PortalFactory.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>

#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Memory.h>
#include <parc/security/parc_IdentityFile.h>

LONGBOW_TEST_RUNNER(test_ccnx_PortalStack)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_ccnx_PortalStack)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_ccnx_PortalStack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_FIXTURE(CreateRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateRelease, CreateRelease);
}

static uint32_t setupFixtureAllocations;

LONGBOW_TEST_FIXTURE_SETUP(CreateRelease)
{
    setupFixtureAllocations = parcMemory_Outstanding();
    parcSecurity_Init();

    const char *keystoreName = "test_ccnx_PortalImplementation_keystore";

    bool success = parcPkcs12KeyStore_CreateFile(keystoreName, "keystore_password", "consumer", 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('%s', 'keystore_password') failed.", keystoreName);

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, "keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);

    CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);
    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);

    longBowTestCase_SetClipBoardData(testCase, factory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateRelease)
{
    CCNxPortalFactory *factory = (CCNxPortalFactory *) longBowTestCase_GetClipBoardData(testCase);

    ccnxPortalFactory_Release(&factory);

    parcSecurity_Fini();
    if (!parcMemoryTesting_ExpectedOutstanding(setupFixtureAllocations, "%s", longBowTestCase_GetName(testCase))) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static void
_mockStart(void *privateData)
{
}

static void
_mockStop(void *privateData)
{
}

static CCNxMetaMessage *
_mockRead(void *privateData, const uint64_t *microSeconds)
{
    CCNxName *name = ccnxName_Create();
    PARCBuffer *payload = parcBuffer_Allocate(10);
    CCNxContentObject *object = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    ccnxName_Release(&name);

    parcBuffer_Release(&payload);

    CCNxMetaMessage *result = ccnxMetaMessage_CreateFromContentObject(object);
    ccnxContentObject_Release(&object);
    return result;
}

static bool
_mockSend(void *privateData, const CCNxMetaMessage *message, const uint64_t *microSeconds)
{
    return true;
}

static bool
_mockListen(void *privateData, const CCNxName *name, const uint64_t *microSeconds)
{
    return true;
}

static bool
_mockIgnore(void *privateData, const CCNxName *name, const uint64_t *microSeconds)
{
    return true;
}

static int
_mockGetFileId(void *privateData)
{
    return 2;
}

static CCNxPortalAttributes *
_mockGetAttributes(void *privateData)
{
    return NULL;
}

static bool
_mockSetAttributes(void *privateData, const CCNxPortalAttributes *attributes)
{
    return true;
}

LONGBOW_TEST_CASE(CreateRelease, CreateRelease)
{
    CCNxPortalFactory *factory = (CCNxPortalFactory *) longBowTestCase_GetClipBoardData(testCase);

    CCNxPortalAttributes *attributes = NULL;
    CCNxPortalStack *actual =
        ccnxPortalStack_Create(factory,
                               attributes,
                               _mockStart,
                               _mockStop,
                               _mockRead,
                               _mockSend,
                               _mockListen,
                               _mockIgnore,
                               _mockGetFileId,
                               _mockSetAttributes,
                               _mockGetAttributes,
                               parcMemory_Allocate(10),
                               parcMemory_DeallocateImpl);

    parcObjectTesting_AssertAcquire(actual);

    ccnxPortalStack_Release(&actual);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_GetFileId);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_GetKeyId);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_GetAttributes);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_SetAttributes);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_Listen);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_Ignore);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_Send);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_Receive);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_Start);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_Stop);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalStack_GetError);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    setupFixtureAllocations = parcMemory_Outstanding();
    parcSecurity_Init();

    const char *keystoreName = "test_ccnx_PortalImplementation_keystore";

    bool success = parcPkcs12KeyStore_CreateFile(keystoreName, "keystore_password", "consumer", 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile('%s', 'keystore_password') failed.", keystoreName);

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, "keystore_password");
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);

    CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);
    parcIdentityFile_Release(&identityFile);
    parcIdentity_Release(&identity);

    const CCNxPortalAttributes *attributes = &ccnxPortalAttributes_NonBlocking;

    CCNxPortalStack *stack = ccnxPortalStack_Create(factory,
                                                    attributes,
                                                    _mockStart,
                                                    _mockStop,
                                                    _mockRead,
                                                    _mockSend,
                                                    _mockListen,
                                                    _mockIgnore,
                                                    _mockGetFileId,
                                                    _mockSetAttributes,
                                                    _mockGetAttributes,
                                                    parcMemory_Allocate(10),
                                                    parcMemory_DeallocateImpl);
    ccnxPortalFactory_Release(&factory);
    longBowTestCase_SetClipBoardData(testCase, stack);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    ccnxPortalStack_Release(&stack);

    parcSecurity_Fini();
    if (!parcMemoryTesting_ExpectedOutstanding(setupFixtureAllocations, "%s", longBowTestCase_GetName(testCase))) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_GetError)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);
    extern int errno;
    errno = 0;
    int actual = ccnxPortalStack_GetErrorCode(stack);
    assertTrue(actual == 0, "Expected ccnxPortalStack_GetErrorCode to return 0, actual %d", actual);
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_Start)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    bool actual = ccnxPortalStack_Start(stack);
    assertTrue(actual, "Expected ccnxPortalStack_Start to return true");
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_Stop)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);
    bool actual = ccnxPortalStack_Stop(stack);

    assertTrue(actual, "Expected ccnxPortalStack_Stop to return true");
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_Receive)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    CCNxMetaMessage *result = ccnxPortalStack_Receive(stack, CCNxStackTimeout_Never);
    assertTrue(result, "Expected ccnxPortalStack_Listen to return true.");
    ccnxMetaMessage_Release(&result);
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_Send)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    CCNxName *name = ccnxName_Create();
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);
    ccnxInterest_Release(&interest);

    bool result = ccnxPortalStack_Send(stack, message, CCNxStackTimeout_Never);
    ccnxMetaMessage_Release(&message);
    assertTrue(result, "Expected ccnxPortalStack_Ignore to return true.");
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_Listen)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    CCNxName *name = ccnxName_Create();
    bool result = ccnxPortalStack_Listen(stack, name, CCNxStackTimeout_Never);
    ccnxName_Release(&name);
    assertTrue(result, "Expected ccnxPortalStack_Listen to return true.");
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_Ignore)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    CCNxName *name = ccnxName_Create();
    bool result = ccnxPortalStack_Ignore(stack, name, CCNxStackTimeout_Never);
    ccnxName_Release(&name);
    assertTrue(result, "Expected ccnxPortalStack_Ignore to return true.");
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_SetAttributes)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    const CCNxPortalAttributes *attributes = &ccnxPortalAttributes_NonBlocking;
    bool result = ccnxPortalStack_SetAttributes(stack, attributes);
    assertTrue(result, "Expected ccnxPortalStack_SetAttributes to return true.");
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_GetAttributes)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    ccnxPortalStack_GetAttributes(stack);
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_GetFileId)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    int fileId = ccnxPortalStack_GetFileId(stack);
    assertFalse(fileId == -1, "Expected ccnxPortalStack_GetFileId to not return -1.");
}

LONGBOW_TEST_CASE(Global, ccnxPortalStack_GetKeyId)
{
    CCNxPortalStack *stack = (CCNxPortalStack *) longBowTestCase_GetClipBoardData(testCase);

    const PARCKeyId *keyId = ccnxPortalStack_GetKeyId(stack);
    assertNotNull(keyId, "Expected non-NULL result from ccnxPortalStack_GetKeyId");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_ccnx_PortalStack);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
