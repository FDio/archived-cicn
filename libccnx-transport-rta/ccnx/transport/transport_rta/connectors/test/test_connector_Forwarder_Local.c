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


#define DEBUG_OUTPUT 1
#include "../connector_Forwarder_Local.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>

#include <ccnx/api/control/cpi_ControlMessage.h>
#include <ccnx/api/control/controlPlaneInterface.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_Commands.c>
#include <ccnx/transport/transport_rta/core/rta_Framework_private.h>
#include <ccnx/transport/transport_rta/config/config_All.h>
#include <ccnx/transport/test_tools/bent_pipe.h>

typedef struct test_data {
    PARCRingBuffer1x1 *commandRingBuffer;
    PARCNotifier *commandNotifier;
    RtaFramework *framework;

    int api_fds[2];
    int listen_fd;
    int rnd_fd;

    int stackId;
    RtaConnection *connectionUnderTest;

    char bentpipe_LocalName[1024];
    BentPipeState *bentpipe;
    char keystoreName[1024];
    char keystorePassword[1024];
} TestData;

static CCNxTransportConfig *
_createParams(const char *local_name, const char *keystore_name, const char *keystore_passwd)
{
    assertNotNull(local_name, "Got null keystore name\n");
    assertNotNull(keystore_name, "Got null keystore name\n");
    assertNotNull(keystore_passwd, "Got null keystore passwd\n");

    CCNxStackConfig *stackConfig = apiConnector_ProtocolStackConfig(
        testingUpper_ProtocolStackConfig(
            localForwarder_ProtocolStackConfig(
                protocolStack_ComponentsConfigArgs(ccnxStackConfig_Create(),
                                                   apiConnector_GetName(),
                                                   testingUpper_GetName(),
                                                   localForwarder_GetName(), NULL))));

    CCNxConnectionConfig *connConfig = apiConnector_ConnectionConfig(
        testingUpper_ConnectionConfig(
            tlvCodec_ConnectionConfig(
                localForwarder_ConnectionConfig(
                    ccnxConnectionConfig_Create(), local_name))));

    publicKeySigner_ConnectionConfig(connConfig, keystore_name, keystore_passwd);

    CCNxTransportConfig *result = ccnxTransportConfig_Create(stackConfig, connConfig);
    ccnxStackConfig_Release(&stackConfig);
    return result;
}

static TestData *
_commonSetup(void)
{
    parcSecurity_Init();
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    sprintf(data->bentpipe_LocalName, "/tmp/bentpipe_%d.sock", getpid());
    data->bentpipe = bentpipe_Create(data->bentpipe_LocalName);
    bentpipe_Start(data->bentpipe);

    sprintf(data->keystoreName, "/tmp/keystore_%d.p12", getpid());
    sprintf(data->keystorePassword, "23439429");

    unlink(data->keystoreName);

    bool success = parcPkcs12KeyStore_CreateFile(data->keystoreName, data->keystorePassword, "user", PARCSigningAlgorithm_RSA, 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile() failed.");

    data->commandRingBuffer = parcRingBuffer1x1_Create(128, NULL);
    data->commandNotifier = parcNotifier_Create();
    data->framework = rtaFramework_Create(data->commandRingBuffer, data->commandNotifier);

    // Create a protocol stack and a connection to use
    CCNxTransportConfig *params = _createParams(data->bentpipe_LocalName, data->keystoreName, data->keystorePassword);

    data->stackId = 1;

    RtaCommandCreateProtocolStack *createStack =
        rtaCommandCreateProtocolStack_Create(data->stackId, ccnxTransportConfig_GetStackConfig(params));

    _rtaFramework_ExecuteCreateStack(data->framework, createStack);
    rtaCommandCreateProtocolStack_Release(&createStack);

    socketpair(PF_LOCAL, SOCK_STREAM, 0, data->api_fds);
    RtaCommandOpenConnection *openConnection = rtaCommandOpenConnection_Create(data->stackId, data->api_fds[0], data->api_fds[1],
                                                                               ccnxConnectionConfig_GetJson(ccnxTransportConfig_GetConnectionConfig(params)));
    _rtaFramework_ExecuteOpenConnection(data->framework, openConnection);
    rtaCommandOpenConnection_Release(&openConnection);

    ccnxTransportConfig_Destroy(&params);

    // now poke in to the connection table to get the connection to test
    data->connectionUnderTest = rtaConnectionTable_GetByApiFd(data->framework->connectionTable, data->api_fds[0]);

    return data;
}

static void
_commonTeardown(TestData *data)
{
    rtaFramework_Teardown(data->framework);

    parcRingBuffer1x1_Release(&data->commandRingBuffer);
    parcNotifier_Release(&data->commandNotifier);
    rtaFramework_Destroy(&data->framework);

    bentpipe_Stop(data->bentpipe);
    bentpipe_Destroy(&data->bentpipe);
    unlink(data->keystoreName);
    parcMemory_Deallocate((void **) &data);
    parcSecurity_Fini();
}

// =============================================================

LONGBOW_TEST_RUNNER(connector_Forwarder_Local)
{
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(connector_Forwarder_Local)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(connector_Forwarder_Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


// ======================================================
LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, connector_Fwd_Local_Init_Release);
    LONGBOW_RUN_TEST_CASE(Local, connector_Fwd_Local_Cpi_Pause);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// ====================================================================

LONGBOW_TEST_CASE(Local, connector_Fwd_Local_Init_Release)
{
    // nothing to do, just checking that memory is in balance in teardown
}

/**
 * Send a PAUSE CPI message to the forwarder.  It should reflect
 * back a CPI ACK
 */
LONGBOW_TEST_CASE(Local, connector_Fwd_Local_Cpi_Pause)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCJSON *controlPause = cpi_CreatePauseInputRequest();

    CCNxTlvDictionary *controlDictionary = ccnxControlFacade_CreateCPI(controlPause);
    TransportMessage *tm_in = transportMessage_CreateFromDictionary(controlDictionary);

    uint64_t pause_seqnum = controlPlaneInterface_GetSequenceNumber(controlPause);
    parcJSON_Release(&controlPause);
    ccnxTlvDictionary_Release(&controlDictionary);

    transportMessage_SetInfo(tm_in, rtaConnection_Copy(data->connectionUnderTest), (void (*)(void **))rtaConnection_Destroy);

    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(rtaConnection_GetStack(data->connectionUnderTest), TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(rtaConnection_GetStack(data->connectionUnderTest), TESTING_UPPER, RTA_DOWN);

    rtaComponent_PutMessage(in, tm_in);

    // this will crank it though the forwarder and reflect back up to us
    rtaFramework_NonThreadedStepCount(data->framework, 4);

    // The first message out should be a CONNECTION_OPEN
    TransportMessage *tm_out = rtaComponent_GetMessage(out);
    transportMessage_Destroy(&tm_out);

    tm_out = rtaComponent_GetMessage(out);

    assertTrue(transportMessage_IsControl(tm_out), "got wrong type, not a control message");

    CCNxControl *control = ccnxMetaMessage_GetControl(transportMessage_GetDictionary(tm_out));

    assertTrue(ccnxControl_IsACK(control), "Expected ccnxControl_IsACK to be true.");

    uint64_t _ack_original_seqnum = ccnxControl_GetAckOriginalSequenceNumber(control);

    assertTrue(_ack_original_seqnum == pause_seqnum,
               "Got wrong original message seqnum, expected %" PRIu64 " got %" PRIu64, pause_seqnum, _ack_original_seqnum);

    transportMessage_Destroy(&tm_out);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(connector_Forwarder_Local);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
