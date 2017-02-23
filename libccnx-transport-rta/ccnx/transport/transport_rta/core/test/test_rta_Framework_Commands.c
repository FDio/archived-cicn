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
 * Creates a bentpipe forwarder and then creates and runs in non-threaded Transport in
 * the commonSetup() function.  The function commonTeardown() undoes all that.
 *
 */

#include "../rta_Framework_Commands.c"
#include <sys/param.h>

#include <LongBow/unit-test.h>

#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_Security.h>

#include <ccnx/api/control/cpi_ControlMessage.h>

#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/transport/transport_rta/config/config_All.h>
#include <ccnx/transport/transport_rta/rta_Transport.h>
#include <ccnx/transport/common/transport_private.h>
#include <ccnx/transport/test_tools/traffic_tools.h>

#include <ccnx/transport/test_tools/bent_pipe.h>

// ==============================================
typedef struct test_data {
    PARCRingBuffer1x1 *commandRingBuffer;
    PARCNotifier *commandNotifier;
    RtaFramework *framework;

    char bentpipe_Directory[MAXPATHLEN];
    char bentpipe_LocalName[MAXPATHLEN];
    BentPipeState *bentpipe;
    char keystoreName[MAXPATHLEN];
    char keystorePassword[MAXPATHLEN];
} TestData;

static CCNxTransportConfig *
_createParams(const char *local_name, const char *keystore_name, const char *keystore_passwd)
{
    assertNotNull(local_name, "Got null local name\n");
    assertNotNull(keystore_name, "Got null keystore name\n");
    assertNotNull(keystore_passwd, "Got null keystore passwd\n");

    CCNxStackConfig *stackConfig = apiConnector_ProtocolStackConfig(
        tlvCodec_ProtocolStackConfig(
            localForwarder_ProtocolStackConfig(
                protocolStack_ComponentsConfigArgs(ccnxStackConfig_Create(),
                                                   apiConnector_GetName(),
                                                   tlvCodec_GetName(),
                                                   localForwarder_GetName(),
                                                   NULL))));

    CCNxConnectionConfig *connConfig = apiConnector_ConnectionConfig(
        localForwarder_ConnectionConfig(ccnxConnectionConfig_Create(), local_name));

    connConfig = tlvCodec_ConnectionConfig(connConfig);

    publicKeySigner_ConnectionConfig(connConfig, keystore_name, keystore_passwd);

    CCNxTransportConfig *result = ccnxTransportConfig_Create(stackConfig, connConfig);
    ccnxStackConfig_Release(&stackConfig);
    return result;
}

static void
_runNonThreaded(TestData *data)
{
    rtaFramework_NonThreadedStepTimed(data->framework, &((struct timeval) { 0, 100000 }));
}

static void
_stopThreaded(TestData *data)
{
    printf("Beginning shutdown pid %d\n", getpid());
    // blocks until done
    rtaFramework_Shutdown(data->framework);
    printf("Finished shutdown pid %d\n", getpid());
}

static void
_stopNonThreaded(TestData *data)
{
    printf("Beginning shutdown pid %d\n", getpid());
    rtaFramework_Teardown(data->framework);
    printf("Finished shutdown pid %d\n", getpid());
}

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    snprintf(data->bentpipe_Directory, MAXPATHLEN, "/tmp/bentpipe_XXXXXX");
    char *p = mkdtemp(data->bentpipe_Directory);
    assertNotNull(p, "Got null from mkdtemp(%s)", data->bentpipe_Directory);
    snprintf(data->bentpipe_LocalName, MAXPATHLEN, "%s/bentpipe.sock", data->bentpipe_Directory);

    data->bentpipe = bentpipe_Create(data->bentpipe_LocalName);
    bentpipe_SetChattyOutput(data->bentpipe, false);

    printf("Staring bent pipe pid %d\n", getpid());
    bentpipe_Start(data->bentpipe);
    printf("Started bent pipe\n");

    snprintf(data->keystoreName, MAXPATHLEN, "/tmp/keystore_p12_XXXXXX");
    int fd = mkstemp(data->keystoreName);
    assertTrue(fd != -1, "Error from mkstemp(%s)", data->keystoreName);

    sprintf(data->keystorePassword, "23439429");

    bool success = parcPkcs12KeyStore_CreateFile(data->keystoreName, data->keystorePassword, "user", 1024, 30);
    assertTrue(success, "parcPublicKeySignerPkcs12Store_CreateFile() failed.");
    close(fd);

    data->commandRingBuffer = parcRingBuffer1x1_Create(128, NULL);
    data->commandNotifier = parcNotifier_Create();
    data->framework = rtaFramework_Create(data->commandRingBuffer, data->commandNotifier);
    return data;
}

static void
_commonTeardown(TestData *data)
{
    if (rtaFramework_GetStatus(data->framework) == FRAMEWORK_RUNNING) {
        _stopThreaded(data);
    } else {
        _stopNonThreaded(data);
    }

    parcRingBuffer1x1_Release(&data->commandRingBuffer);
    parcNotifier_Release(&data->commandNotifier);

    printf("Destroying framework pid %d\n", getpid());
    rtaFramework_Destroy(&data->framework);

    bentpipe_Stop(data->bentpipe);
    bentpipe_Destroy(&data->bentpipe);
    unlink(data->keystoreName);
    unlink(data->bentpipe_LocalName);
    rmdir(data->bentpipe_Directory);

    parcMemory_Deallocate((void **) &data);
}


/**
 * @function assertConnectionOpen
 * @abstract Block on reading the 1st message out of the socket.  It's the connection ready message.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
static void
_assertConnectionOpen(int fd)
{
    CCNxMetaMessage *firstMessage;

    rtaTransport_Recv(NULL, fd, &firstMessage, CCNxStackTimeout_Never);

    assertTrue(ccnxMetaMessage_IsControl(firstMessage), "not a control message");

    CCNxControl *control = ccnxMetaMessage_GetControl(firstMessage);

    NotifyStatus *status = notifyStatus_ParseJSON(ccnxControl_GetJson(control));
    ccnxMetaMessage_Release(&firstMessage);

    assertTrue(notifyStatus_IsConnectionOpen(status), "Expected notifyStatus_IsConnectionOpen to be true");
    notifyStatus_Release(&status);
}


/**
 * @function openConnection
 * @abstract Opens a connection and fills in the socket pair
 * @discussion
 *   uses rtaFramework_ExecuteOpen to directly create, does not go over the command pair
 *
 * @param <#param1#>
 * @return <#return#>
 */
static void
_openConnection(RtaFramework *framework, CCNxTransportConfig *transportConfig, int stack_id, int socketPairOutput[])
{
    socketpair(PF_LOCAL, SOCK_STREAM, 0, socketPairOutput);

    struct timeval timeout = { .tv_sec = 10, .tv_usec = 0 };

    setsockopt(socketPairOutput[0], SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(socketPairOutput[0], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(socketPairOutput[1], SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(socketPairOutput[1], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    RtaCommandOpenConnection *openConnection = rtaCommandOpenConnection_Create(stack_id, socketPairOutput[1], socketPairOutput[0],
                                                                               ccnxConnectionConfig_GetJson(ccnxTransportConfig_GetConnectionConfig(transportConfig)));

    _rtaFramework_ExecuteOpenConnection(framework, openConnection);
    rtaCommandOpenConnection_Release(&openConnection);

    rtaFramework_NonThreadedStepCount(framework, 10);
    _assertConnectionOpen(socketPairOutput[1]);
}

static bool
_readAndCompareName(int fd, CCNxName *truthName)
{
    CCNxMetaMessage *test_msg;

    int res = rtaTransport_Recv(NULL, fd, &test_msg, CCNxStackTimeout_Never);
    assertTrue(res == 0, "Got error receiving on bob's socket: %s (%d)", strerror(errno), errno);

    assertNotNull(test_msg, "Got null message from Bob");

    assertTrue(ccnxMetaMessage_IsInterest(test_msg), "Got wrong type, expected Interest but got other");

    CCNxInterest *interest = ccnxMetaMessage_GetInterest(test_msg);

    assertTrue(ccnxName_Compare(truthName, ccnxInterest_GetName(interest)) == 0, "Names did not compare")
    {
        ccnxName_Display(ccnxInterest_GetName(interest), 3);
        ccnxName_Display(truthName, 3);
    }

    ccnxMetaMessage_Release(&test_msg);

    return true;
}

// ==========================

LONGBOW_TEST_RUNNER(rta_Framework_Commands)
{
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_Framework_Commands)
{
    printf("\n********\n%s starting\n\n", __func__);

    srandom((int) time(NULL));
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_Framework_Commands)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _rtaFramework_ExecuteCloseConnection);
    LONGBOW_RUN_TEST_CASE(Local, _rtaFramework_ExecuteCreateStack);
    LONGBOW_RUN_TEST_CASE(Local, _rtaFramework_ExecuteOpenConnection);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    parcSecurity_Init();

#if __APPLE__
    pthread_setname_np(longBowTestCase_GetName(testCase));
#else
    pthread_setname_np(pthread_self(), longBowTestCase_GetName(testCase));
#endif

    TestData *data = _commonSetup();
    _runNonThreaded(data);

    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _commonTeardown(data);
    parcSecurity_Fini();

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _rtaFramework_ExecuteCloseConnection)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    int stack_id = 5;

    CCNxTransportConfig *params = _createParams(data->bentpipe_LocalName, data->keystoreName, data->keystorePassword);

    RtaCommandCreateProtocolStack *createStack =
        rtaCommandCreateProtocolStack_Create(stack_id, ccnxTransportConfig_GetStackConfig(params));

    _rtaFramework_ExecuteCreateStack(data->framework, createStack);
    rtaCommandCreateProtocolStack_Release(&createStack);

    // now use three connections, then close 1 and make sure other 2 still ok
    {
        int alice_pair[2], bob_pair[2], charlie_pair[2];

        _openConnection(data->framework, params, stack_id, alice_pair);
        _openConnection(data->framework, params, stack_id, bob_pair);
        _openConnection(data->framework, params, stack_id, charlie_pair);

        CCNxInterest *firstInterest = trafficTools_CreateInterest();

        // send will consume the message, so copy out the name
        CCNxName *truth_name = ccnxName_Copy(ccnxInterest_GetName(firstInterest));

        CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(firstInterest);
        bool success = rtaTransport_Send(NULL, alice_pair[1], message, CCNxStackTimeout_Never);
        assertTrue(success, "Got error sending on alice's socket: %s (%d)", strerror(errno), errno);
        ccnxMetaMessage_Release(&message);

        // *** Read bob
        rtaFramework_NonThreadedStepCount(data->framework, 10);
        _readAndCompareName(bob_pair[1], truth_name);

        // *** Read Charlie
        rtaFramework_NonThreadedStepCount(data->framework, 10);
        _readAndCompareName(charlie_pair[1], truth_name);

        // Close charlie and make sure alice + bob still happy
        RtaCommandCloseConnection *closeConnection = rtaCommandCloseConnection_Create(charlie_pair[1]);
        _rtaFramework_ExecuteCloseConnection(data->framework, closeConnection);
        rtaCommandCloseConnection_Release(&closeConnection);
        rtaFramework_NonThreadedStepCount(data->framework, 10);

        // send another interest
        CCNxInterest *secondInterest = trafficTools_CreateInterest();
        message = ccnxMetaMessage_CreateFromInterest(secondInterest);

        success = rtaTransport_Send(NULL, alice_pair[1], message, CCNxStackTimeout_Never);
        assertTrue(success, "Got error sending on alice's socket: %s (%d)", strerror(errno), errno);
        ccnxMetaMessage_Release(&message);

        // make sure bob gets it
        rtaFramework_NonThreadedStepCount(data->framework, 10);
        _readAndCompareName(bob_pair[1], truth_name);

        ccnxName_Release(&truth_name);
        ccnxInterest_Release(&firstInterest);
        ccnxInterest_Release(&secondInterest);
    }

    ccnxTransportConfig_Destroy(&params);
}

LONGBOW_TEST_CASE(Local, _rtaFramework_ExecuteCreateStack)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int stack_id = 4;
    CCNxTransportConfig *params = _createParams(data->bentpipe_LocalName, data->keystoreName, data->keystorePassword);
    RtaCommandCreateProtocolStack *createStack =
        rtaCommandCreateProtocolStack_Create(stack_id, ccnxTransportConfig_GetStackConfig(params));


    // this call skirts around threading
    _rtaFramework_ExecuteCreateStack(data->framework, createStack);

    FrameworkProtocolHolder *holder;
    holder = rtaFramework_GetProtocolStackByStackId(data->framework, stack_id);
    assertNotNull(holder, "There is no protocol holder for this stack, not created?");

    ccnxTransportConfig_Destroy(&params);
    rtaCommandCreateProtocolStack_Release(&createStack);
}

LONGBOW_TEST_CASE(Local, _rtaFramework_ExecuteOpenConnection)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int stack_id = 4;
    CCNxTransportConfig *params = _createParams(data->bentpipe_LocalName, data->keystoreName, data->keystorePassword);

    RtaCommandCreateProtocolStack *createStack =
        rtaCommandCreateProtocolStack_Create(stack_id, ccnxTransportConfig_GetStackConfig(params));
    _rtaFramework_ExecuteCreateStack(data->framework, createStack);
    rtaCommandCreateProtocolStack_Release(&createStack);

    // now create two connections and make sure they work
    {
        // now create
        int alice_pair[2], bob_pair[2];
        socketpair(PF_LOCAL, SOCK_STREAM, 0, alice_pair);
        socketpair(PF_LOCAL, SOCK_STREAM, 0, bob_pair);

        _openConnection(data->framework, params, stack_id, alice_pair);
        _openConnection(data->framework, params, stack_id, bob_pair);

        CCNxInterest *interest = trafficTools_CreateInterest();

        //ccnxInterest_Display(interest, 0);

        // send will consume the message, so copy out the name
        CCNxName *truth_name = ccnxName_Copy(ccnxInterest_GetName(interest));

        //ccnxName_Display(truth_name, 0);

        // now send it down the stack
        CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);
        bool success = rtaTransport_Send(NULL, alice_pair[1], message, CCNxStackTimeout_Never);
        assertTrue(success, "Got error sending on alice's socket: %s (%d)", strerror(errno), errno);
        ccnxMetaMessage_Release(&message);

        rtaFramework_NonThreadedStepCount(data->framework, 10);
        _readAndCompareName(bob_pair[1], truth_name);

        ccnxName_Release(&truth_name);
        ccnxInterest_Release(&interest);
    }

    ccnxTransportConfig_Destroy(&params);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_Framework_Commands);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
