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
 *
 * This test will setup a server socket so the Metis connector can connect to it.  We can
 * then see the packets the connector thinks it is sending to Metis.
 */

#define DEBUG 1
#include "../connector_Forwarder_Metis.c"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_Security.h>

#include <ccnx/api/control/cpi_ControlFacade.h>
#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_Forwarding.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_Commands.c>
#include <ccnx/transport/transport_rta/core/rta_Framework_private.h>
#include <ccnx/transport/transport_rta/config/config_All.h>

#include <ccnx/common/codec/ccnxCodec_TlvPacket.h>
#include <ccnx/transport/test_tools/traffic_tools.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_nameA.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_crc32c.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_cpi_add_route_crc32c.h>

#include <ccnx/common/ccnx_WireFormatMessage.h>

// inet_pton
#include <arpa/inet.h>

#include <LongBow/unit-test.h>

static char keystorename[1024];
static const char keystorepass[] = "2398472983479234";

#ifndef INPORT_ANY
#define INPORT_ANY 0
#endif

typedef struct test_data {
    PARCRingBuffer1x1 *commandRingBuffer;
    PARCNotifier *commandNotifier;

    // we will bind to a random port, this is what we end up binding to
    // Its in host byte order
    uint16_t metis_port;

    // server_socket is a socket we listen to like the Metis forwarder, so
    // we can see all the traffic that comes out the bottom of the connector.
    int server_socket;

    // when we accept a client on the server socket, this is his socket
    int client_socket;

    RtaFramework *framework;
    CCNxTransportConfig *params;

    char keystoreName[1024];
    char keystorePassword[1024];
} TestData;

/*
 * @function setup_server
 * @abstract Bind to 127.0.0.1 on a random port, returns the socket and port
 * @discussion
 *   <#Discussion#>
 *
 * @param portOutput is the port bound to in host byte order
 * @return <#return#>
 */
static int
_setup_server(uint16_t *portOutput)
{
    struct sockaddr_in address;

    /* listen on 127.0.0.1 random port */
    address.sin_family = PF_INET;
    address.sin_port = INPORT_ANY;
    inet_pton(AF_INET, "127.0.0.1", &(address.sin_addr));

    int fd = socket(PF_INET, SOCK_STREAM, 0);
    assertFalse(fd < 0, "error on bind: (%d) %s", errno, strerror(errno));

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    failure = bind(fd, (struct sockaddr *) &address, sizeof(struct sockaddr_in));
    assertFalse(failure, "error on bind: (%d) %s", errno, strerror(errno));

    failure = listen(fd, 16);
    assertFalse(failure, "error on listen: (%d) %s", errno, strerror(errno));

    socklen_t x = sizeof(address);
    failure = getsockname(fd, (struct sockaddr *) &address, &x);
    assertFalse(failure, "error on getsockname: (%d) %s", errno, strerror(errno));

    *portOutput = htons(address.sin_port);

    printf("test server setup on port %d\n", *portOutput);
    return fd;
}

static int
_accept_client(int server_socket)
{
    socklen_t addrlen;
    struct sockaddr_in address;
    int client_socket;

    addrlen = sizeof(struct sockaddr_in);
    client_socket = accept(server_socket, (struct sockaddr *) &address, &addrlen);
    assertFalse(client_socket < 0, "accept error: %s", strerror(errno));

    printf("%s accepted client on socket %d\n", __func__, client_socket);
    return client_socket;
}

static RtaConnection *
_openConnection(TestData *data, int stack_id, int fds[2])
{
    RtaCommandOpenConnection *openConnection = rtaCommandOpenConnection_Create(stack_id, fds[0], fds[1],
                                                                               ccnxConnectionConfig_GetJson(ccnxTransportConfig_GetConnectionConfig(data->params)));
    _rtaFramework_ExecuteOpenConnection(data->framework, openConnection);
    rtaCommandOpenConnection_Release(&openConnection);

    return rtaConnectionTable_GetByApiFd(data->framework->connectionTable, fds[0]);
}

static void
_createStack(TestData *data, int stack_id)
{
    RtaCommandCreateProtocolStack *createStack =
        rtaCommandCreateProtocolStack_Create(stack_id, ccnxTransportConfig_GetStackConfig(data->params));
    _rtaFramework_ExecuteCreateStack(data->framework, createStack);
    rtaCommandCreateProtocolStack_Release(&createStack);
}

static CCNxTransportConfig *
_createParams(int port, const char *keystore_name, const char *keystore_passwd)
{
    CCNxStackConfig *stackConfig;
    CCNxConnectionConfig *connConfig;

    assertNotNull(keystore_name, "Got null keystore name\n");
    assertNotNull(keystore_passwd, "Got null keystore passwd\n");

    stackConfig = apiConnector_ProtocolStackConfig(
        testingUpper_ProtocolStackConfig(
            metisForwarder_ProtocolStackConfig(
                protocolStack_ComponentsConfigArgs(ccnxStackConfig_Create(),
                                                   apiConnector_GetName(),
                                                   testingUpper_GetName(),
                                                   metisForwarder_GetName(), NULL))));

    connConfig = apiConnector_ConnectionConfig(
        testingUpper_ConnectionConfig(
            metisForwarder_ConnectionConfig(
                tlvCodec_ConnectionConfig(
                    ccnxConnectionConfig_Create()), port)));

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
    memset(data, 0, sizeof(TestData));

    data->server_socket = _setup_server(&data->metis_port);

    //    printf("%s listening on port %u\n", __func__, data->metis_port);

    sprintf(data->keystoreName, "%s", keystorename);
    sprintf(data->keystorePassword, keystorepass);

    data->commandRingBuffer = parcRingBuffer1x1_Create(128, NULL);
    data->commandNotifier = parcNotifier_Create();
    data->framework = rtaFramework_Create(data->commandRingBuffer, data->commandNotifier);

    data->params = _createParams(data->metis_port, data->keystoreName, keystorepass);
    // we will always create stack #1 as the default stack
    _createStack(data, 1);
    return data;
}

static void
_commonTeardown(TestData *data)
{
    if (data != NULL) {
        if (data->server_socket > 0) {
            close(data->server_socket);
        }

        if (data->client_socket > 0) {
            close(data->client_socket);
        }

        ccnxTransportConfig_Destroy(&data->params);
        rtaFramework_Teardown(data->framework);

        parcRingBuffer1x1_Release(&data->commandRingBuffer);
        parcNotifier_Release(&data->commandNotifier);
        rtaFramework_Destroy(&data->framework);
        parcMemory_Deallocate((void **) &data);
    }
    parcSecurity_Fini();
}

// ======================================================
// helper functions

/**
 * Wait for a READ event on the specifid socket.  Has a 1 second timeout.
 *
 * @return true if READ event
 * @return false otherwise
 */
static bool
_waitForSelect(int fd)
{
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(fd, &readset);
    int result = select(fd + 1, &readset, NULL, NULL, &(struct timeval) { 1, 0 });
    assertFalse(result < 0, "Error on select: (%d) %s", errno, strerror(errno));
    assertFalse(result == 0, "Timeout waiting for connection attempt");
    assertTrue(FD_ISSET(fd, &readset), "server_socket was not set by select");

    return true;
}



static size_t
_sendPacketToConnectorV1(int fd, size_t payloadLength)
{
    // Setup the header
    uint8_t headerLength = 13;
    uint16_t packetLength = payloadLength + headerLength;
    uint8_t packetType = CCNxCodecSchemaV1Types_PacketType_Interest;

    CCNxCodecSchemaV1FixedHeader hdr = { .version = 1, .packetType = packetType, .packetLength = htons(packetLength), .headerLength = headerLength };

    // put header in packet and write the packet
    uint8_t packet[1024];
    memcpy(packet, &hdr, sizeof(hdr));

    // write out exactly the number of bytes we need
    size_t writeSize = packetLength;

    ssize_t nwritten = write(fd, packet, writeSize);
    assertTrue(nwritten == writeSize, "Wrong write size, expected %zu got %zd", writeSize, nwritten);
    return writeSize;
}

static RtaConnection *
setupConnectionAndClientSocket(TestData *data, int *apiSocketOuptut, int *clientSocketOutput)
{
    // Open a listener and accept the forwarders connection
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);
    RtaConnection *conn = _openConnection(data, 1, fds);
    assertNotNull(conn, "Got null connection opening on stack 1");

    rtaFramework_NonThreadedStepCount(data->framework, 2);

    // we should now see a connection request
    _waitForSelect(data->server_socket);

    // accept the client and set a 1 second read timeout on the socket
    int client_fd = _accept_client(data->server_socket);
    struct timeval readTimeout = { 1, 0 };
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &readTimeout, sizeof(readTimeout));

    *apiSocketOuptut = fds[0];
    *clientSocketOutput = client_fd;
    return conn;
}

// throw away the first control message
static void
_throwAwayControlMessage(PARCEventQueue *out)
{
    TransportMessage *control_tm = rtaComponent_GetMessage(out);
    assertNotNull(control_tm, "Did not receive a transport message out of the top of the connector");
    assertTrue(transportMessage_IsControl(control_tm),
               "transport message is not a control message")
    {
        ccnxTlvDictionary_Display(transportMessage_GetDictionary(control_tm), 0);
    }
    transportMessage_Destroy(&control_tm);
}

static void
_testReadPacketV1(size_t extraBytes)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);

    // this replaces "_openSocket"
    fwd_state->fd = fds[STACK];

    _setupSocket(fwd_state);

    // Setup the header
    uint16_t packetLength = 24;
    uint8_t headerLength = 13;
    uint8_t packetType = CCNxCodecSchemaV1Types_PacketType_Interest;

    CCNxCodecSchemaV1FixedHeader hdr = { .version = 1, .packetType = packetType, .packetLength = htons(packetLength), .headerLength = headerLength };

    // put header in packet and write the packet
    uint8_t packet[1024];
    memcpy(packet, &hdr, sizeof(hdr));

    // write out exactly the number of bytes we need
    size_t firstWrite = packetLength;

    ssize_t nwritten = write(fds[REMOTE], packet, firstWrite + extraBytes);
    assertTrue(nwritten == firstWrite + extraBytes, "Wrong write size, expected %zu got %zd",
               firstWrite + extraBytes, nwritten);

    ReadReturnCode readCode = _readPacket(fwd_state);

    assertTrue(readCode == ReadReturnCode_Finished, "readCode should be %d got %d", ReadReturnCode_Finished, readCode);

    // should indicate there's nothing left to read of the header
    assertTrue(fwd_state->nextMessage.remainingReadLength == 0, "Remaining length should be 0 got %zu", fwd_state->nextMessage.remainingReadLength);

    // we should be at position "firstWrite" in the packet buffer
    assertNotNull(fwd_state->nextMessage.packet, "Packet buffer is null");
    assertTrue(parcBuffer_Position(fwd_state->nextMessage.packet) == firstWrite,
               "Wrong position, expected %zu got %zu", firstWrite, parcBuffer_Position(fwd_state->nextMessage.packet));

    // cleanup
    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
    close(fds[REMOTE]);
}



static void
_testReadFromMetisFromArray(TestData *data, size_t length, uint8_t buffer[length])
{
    // create our connection.  This will become part of the RTA framework, so will be
    // cleaned up in the teardown
    int api_fd;
    int client_fd;
    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);

    ssize_t nwritten = write(client_fd, buffer, length);
    assertTrue(nwritten == length, "Wrong write size, expected %zu got %zd", length, nwritten);

    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);;

    _readFromMetis(fwd_state, conn);

    // now crank the handle to pop those messages up the stack
    rtaFramework_NonThreadedStepCount(data->framework, 5);

    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(rtaConnection_GetStack(conn), TESTING_UPPER, RTA_DOWN);
    _throwAwayControlMessage(out);

    // verify the wire format is what we wrote
    TransportMessage *test_tm = rtaComponent_GetMessage(out);
    assertNotNull(test_tm, "Did not receive a transport message out of the top of the connector");

    CCNxTlvDictionary *testDictionary = transportMessage_GetDictionary(test_tm);
    PARCBuffer *writeFormat = ccnxWireFormatMessage_GetWireFormatBuffer(testDictionary);
    assertNotNull(writeFormat,
                  "transport message does not have a wire format");

    PARCBuffer *truth = parcBuffer_Wrap(buffer, length, 0, length);
    assertTrue(parcBuffer_Equals(truth, writeFormat), "Wire format does not match expected")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Received:\n");
        parcBuffer_Display(writeFormat, 3);
    }

    parcBuffer_Release(&truth);
    transportMessage_Destroy(&test_tm);
}


// ======================================================

LONGBOW_TEST_RUNNER(connector_Forwarder_Metis)
{
    LONGBOW_RUN_TEST_FIXTURE(Local);

    LONGBOW_RUN_TEST_FIXTURE(UpDirectionV1);
    LONGBOW_RUN_TEST_FIXTURE(DownDirectionV1);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(connector_Forwarder_Metis)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    snprintf(keystorename, 1024, "/tmp/keystore_%d.p12", getpid());

    // init + fini here so there's no memory imbalance
    parcSecurity_Init();
    parcPkcs12KeyStore_CreateFile(keystorename, keystorepass, "ccnxuser", PARCSigningAlgorithm_RSA, 1024, 365);
    parcSecurity_Fini();

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(connector_Forwarder_Metis)
{
    unlink(keystorename);
    return LONGBOW_STATUS_SUCCEEDED;
}

// ======================================================
LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, connector_Fwd_Metis_Init_Release);
    LONGBOW_RUN_TEST_CASE(Local, connector_Fwd_Metis_Opener_GoodPort);
    LONGBOW_RUN_TEST_CASE(Local, _fwdMetisState_Release);
    LONGBOW_RUN_TEST_CASE(Local, _readInEnvironmentConnectionSpecification);
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

LONGBOW_TEST_CASE(Local, connector_Fwd_Metis_Init_Release)
{
    // nothing to do, just checking that memory is in balance in teardown
}

/**
 * Call the opener with the right port.  We should see a connection attempt on
 * the server socket and be able to accept it.
 */
LONGBOW_TEST_CASE(Local, connector_Fwd_Metis_Opener_GoodPort)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);
    RtaConnection *conn = _openConnection(data, 1, fds);
    assertNotNull(conn, "Got null connection opening on stack 1");

    rtaFramework_NonThreadedStepCount(data->framework, 2);

    // we should now see a connection request
    _waitForSelect(data->server_socket);

    close(fds[1]);
}

/**
 * Make sure everything is released and file descriptor is closed
 */
LONGBOW_TEST_CASE(Local, _fwdMetisState_Release)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);

    // this replaces "_openSocket"
    fwd_state->fd = fds[STACK];

    _setupSocket(fwd_state);


    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);

    // ensure that fds[STACK] is closed by _fwdMetisState_Release
    uint8_t buffer[16];
    ssize_t nread = recv(fds[STACK], buffer, 16, 0);
    assertTrue(nread == -1 && errno == EBADF,
               "read from closed socket %d should be EBADF, got return %zd and errno (%d) %s",
               fds[STACK], nread, errno, strerror(errno));

    close(fds[REMOTE]);
}

LONGBOW_TEST_CASE(Local, _readInEnvironmentConnectionSpecification)
{
    char *oldEnv = getenv(FORWARDER_CONNECTION_ENV);
    setenv(FORWARDER_CONNECTION_ENV, "tcp://127.0.0.1:9999", 1);
    struct sockaddr_in addr_in;
    _readInEnvironmentConnectionSpecification(&addr_in);
    assertTrue(addr_in.sin_port == htons(9999), "Port specification incorrectly parsed");
    assertTrue(addr_in.sin_addr.s_addr == inet_addr("127.0.0.1"), "Address specification incorrectly parsed");;
    if (oldEnv) {
        setenv(FORWARDER_CONNECTION_ENV, oldEnv, 1);
    } else {
        unsetenv(FORWARDER_CONNECTION_ENV);
    }
}



// ====================================================================


LONGBOW_TEST_FIXTURE(UpDirectionV1)
{
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacketHeader_ExactFit);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacketHeader_TwoReads);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _setupNextPacket);

    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacket_PartialMessage);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacket_ExactlyOneMessage);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacket_MoreThanOneMessage);

    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readFromMetis_ThreeMessages);

    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readFromMetis_InterestV1);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readFromMetis_ContentObjectV1);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readFromMetis_ControlV1);

    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacketHeader_Error);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacketBody_Error);

    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacketHeader_Closed);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readPacketBody_Closed);
    LONGBOW_RUN_TEST_CASE(UpDirectionV1, _readFromMetis_Closed);
}

LONGBOW_TEST_FIXTURE_SETUP(UpDirectionV1)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(UpDirectionV1)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * Put in exactly 8 bytes.
 * This should return NULL, but will set nextMessageLength to be the right thing.
 * Does not drain the buffer
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacketHeader_ExactFit)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);

    // this replaces "_openSocket"
    fwd_state->fd = fds[STACK];

    _setupSocket(fwd_state);

    // Setup the header
    uint16_t packetLength = 24;
    uint8_t headerLength = 13;
    uint8_t packetType = CCNxCodecSchemaV1Types_PacketType_Interest;

    CCNxCodecSchemaV1FixedHeader hdr = { .version = 1, .packetType = packetType, .packetLength = htons(packetLength), .headerLength = headerLength };

    // put header in packet and write the packet
    uint8_t packet[1024];
    size_t bufferReadLength = sizeof(hdr);
    memcpy(packet, &hdr, bufferReadLength);

    // write out exactly the number of bytes we need
    ssize_t nwritten = write(fds[REMOTE], packet, sizeof(CCNxCodecSchemaV1FixedHeader));
    assertTrue(nwritten == sizeof(CCNxCodecSchemaV1FixedHeader), "Wrong write size, expected %zu got %zd",
               sizeof(CCNxCodecSchemaV1FixedHeader), nwritten);

    // test the function
    ReadReturnCode readCode = _readPacketHeader(fwd_state);
    assertTrue(readCode == ReadReturnCode_Finished, "readCode should be %d got %d", ReadReturnCode_Finished, readCode);
    assertTrue(fwd_state->nextMessage.remainingReadLength == 0, "Remaining length should be 0 got %zu", fwd_state->nextMessage.remainingReadLength);

    // other properties are tested as part of _setupNextPacket

    // cleanup
    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
    close(fds[REMOTE]);
}

/*
 * Write the fixed header in two 4 byte writes
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacketHeader_TwoReads)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);

    // this replaces "_openSocket"
    fwd_state->fd = fds[STACK];

    _setupSocket(fwd_state);

    // Setup the header
    uint16_t packetLength = 24;
    uint8_t headerLength = 13;
    uint8_t packetType = CCNxCodecSchemaV1Types_PacketType_Interest;

    CCNxCodecSchemaV1FixedHeader hdr = {
        .version      = 1,
        .packetType   = packetType,
        .packetLength = htons(packetLength),
        .headerLength = headerLength
    };

    // put header in packet and write the packet
    uint8_t packet[1024];
    size_t bufferReadLength = sizeof(hdr);
    memcpy(packet, &hdr, bufferReadLength);

    // write out exactly the number of bytes we need
    size_t firstWrite = 4;
    size_t secondWrite = sizeof(CCNxCodecSchemaV1FixedHeader) - firstWrite;

    ssize_t nwritten = write(fds[REMOTE], packet, firstWrite);
    assertTrue(nwritten == firstWrite, "Wrong write size, expected %zu got %zd", firstWrite, nwritten);

    ReadReturnCode readCode = _readPacketHeader(fwd_state);
    assertTrue(readCode == ReadReturnCode_PartialRead, "readCode should be %d got %d", ReadReturnCode_PartialRead, readCode);

    nwritten = write(fds[REMOTE], packet + firstWrite, secondWrite);
    assertTrue(nwritten == secondWrite, "Wrong write size, expected %zu got %zd", secondWrite, nwritten);

    readCode = _readPacketHeader(fwd_state);
    assertTrue(readCode == ReadReturnCode_Finished, "readCode should be %d got %d", ReadReturnCode_Finished, readCode);

    assertTrue(fwd_state->nextMessage.remainingReadLength == 0, "Remaining length should be 0 got %zu", fwd_state->nextMessage.remainingReadLength);

    // other properties are tested as part of _setupNextPacket

    // cleanup
    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
    close(fds[REMOTE]);
}

LONGBOW_TEST_CASE(UpDirectionV1, _setupNextPacket)
{
    uint16_t packetLength = 24;
    uint8_t headerLength = 13;
    uint8_t packetType = CCNxCodecSchemaV1Types_PacketType_Interest;
    uint8_t version = 1;
    CCNxCodecSchemaV1FixedHeader hdr = { .version = version, .packetType = packetType, .packetLength = htons(packetLength), .headerLength = headerLength };

    // setup fwd_state->nextMessage like we just read a header
    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);
    fwd_state->nextMessage.remainingReadLength = 0;
    memcpy(&fwd_state->nextMessage.fixedHeader, &hdr, sizeof(hdr));

    // this is the truth we will test against
    size_t nextMessageLength = packetLength;

    _setupNextPacket(fwd_state);

    size_t allocatedLength = parcBuffer_Capacity(fwd_state->nextMessage.packet);
    size_t position = parcBuffer_Position(fwd_state->nextMessage.packet);
    parcBuffer_Flip(fwd_state->nextMessage.packet);
    void *buffer = parcBuffer_Overlay(fwd_state->nextMessage.packet, 0);

    assertTrue(fwd_state->nextMessage.length == nextMessageLength, "Wrong packet length, expected %zu got %zu", nextMessageLength, fwd_state->nextMessage.length);
    assertTrue(fwd_state->nextMessage.packetType == packetType, "Wrong packetType, expected %u got %u", packetType, fwd_state->nextMessage.packetType);
    assertTrue(fwd_state->nextMessage.version == version, "Wrong version, expected %u got %u", version, fwd_state->nextMessage.version);
    assertTrue(allocatedLength == nextMessageLength, "Wrong packet buffer length, expected %zu got %zu", nextMessageLength, allocatedLength);

    // and make sure the beginning of the buffer is the fixed header
    assertTrue(position == sizeof(hdr), "Wrong write position, expected %zu got %zu", sizeof(hdr), position);
    assertTrue(memcmp(buffer, &hdr, sizeof(hdr)) == 0, "Beginning of buffer not the fixed header");

    // TODO: Finish me
    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
}

/**
 * Write the fixed header plus part of the message body.
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacket_PartialMessage)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);

    // this replaces "_openSocket"
    fwd_state->fd = fds[STACK];

    _setupSocket(fwd_state);

    // Setup the header
    uint16_t packetLength = 160;
    uint8_t headerLength = 13;
    uint8_t packetType = CCNxCodecSchemaV1Types_PacketType_Interest;
    uint8_t version = 1;
    CCNxCodecSchemaV1FixedHeader hdr = { .version = version, .packetType = packetType, .packetLength = htons(packetLength), .headerLength = headerLength };

    // put header in packet and write the packet
    uint8_t packet[1024];
    memcpy(packet, &hdr, sizeof(hdr));

    // write out exactly the number of bytes we need
    size_t firstWrite = 100;

    ssize_t nwritten = write(fds[REMOTE], packet, firstWrite);
    assertTrue(nwritten == firstWrite, "Wrong write size, expected %zu got %zd", firstWrite, nwritten);

    ReadReturnCode readCode = _readPacket(fwd_state);

    assertTrue(readCode == ReadReturnCode_PartialRead, "return value should be %d got %d", ReadReturnCode_PartialRead, readCode);

    // should indicate there's nothing left to read of the header
    assertTrue(fwd_state->nextMessage.remainingReadLength == 0, "Remaining length should be 0 got %zu", fwd_state->nextMessage.remainingReadLength);

    // we should be at position "firstWrite" in the packet buffer
    assertNotNull(fwd_state->nextMessage.packet, "Packet buffer is null");
    assertTrue(parcBuffer_Position(fwd_state->nextMessage.packet) == firstWrite,
               "Wrong position, expected %zu got %zu", firstWrite, parcBuffer_Position(fwd_state->nextMessage.packet));

    // cleanup
    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
    close(fds[REMOTE]);
}

/**
 * Write exactly one message
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacket_ExactlyOneMessage)
{
    _testReadPacketV1(0);
}

/**
 * Write more than one message.
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacket_MoreThanOneMessage)
{
    _testReadPacketV1(100);
}

/**
 * Make 3 messages pending on the read socket and make sure _readFromMetis delivers all
 * 3 up the stack.  _readFromMetis requires an RtaConnection, so we need a mock framework.
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readFromMetis_ThreeMessages)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // create our connection.  This will become part of the RTA framework, so will be
    // cleaned up in the teardown
    int api_fd;
    int client_fd;
    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);

    // Write three wire format packets up the bottom of the connector
    const int loopCount = 3;
    size_t writeSizes[loopCount];

    for (int i = 0; i < loopCount; i++) {
        writeSizes[i] = _sendPacketToConnectorV1(client_fd, (i + 1) * 100);
    }

    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);;

    _readFromMetis(fwd_state, conn);

    // now crank the handle to pop those messages up the stack
    rtaFramework_NonThreadedStepCount(data->framework, 5);

    // now read the message out of the test component
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(rtaConnection_GetStack(conn), TESTING_UPPER, RTA_DOWN);

    // throw away the first control message
    _throwAwayControlMessage(out);

    // Now read the actual messages we want to test
    for (int i = 0; i < loopCount; i++) {
        TransportMessage *test_tm = rtaComponent_GetMessage(out);
        assertNotNull(test_tm, "Did not receive a transport message %d out of %d out of the top of the connector", i + 1, loopCount);

        assertTrue(transportMessage_IsInterest(test_tm),
                   "second transport message is not an interest")
        {
            ccnxTlvDictionary_Display(transportMessage_GetDictionary(test_tm), 0);
        }

        // Make sure the transport message has the right properties
        CCNxTlvDictionary *testDictionary = transportMessage_GetDictionary(test_tm);
        PARCBuffer *writeFormat = ccnxWireFormatMessage_GetWireFormatBuffer(testDictionary);
        assertNotNull(writeFormat,
                      "transport message does not have a wire format");

        assertTrue(parcBuffer_Remaining(writeFormat) == writeSizes[i],
                   "Raw format message wrong length, expected %zu got %zu",
                   writeSizes[i],
                   parcBuffer_Remaining(writeFormat));

        // cleanup
        transportMessage_Destroy(&test_tm);
    }

    // no extra cleanup, done in teardown
}

LONGBOW_TEST_CASE(UpDirectionV1, _readFromMetis_InterestV1)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _testReadFromMetisFromArray(data, sizeof(v1_interest_nameA), v1_interest_nameA);
}

LONGBOW_TEST_CASE(UpDirectionV1, _readFromMetis_ContentObjectV1)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _testReadFromMetisFromArray(data, sizeof(v1_content_nameA_crc32c), v1_content_nameA_crc32c);
}

LONGBOW_TEST_CASE(UpDirectionV1, _readFromMetis_ControlV1)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _testReadFromMetisFromArray(data, sizeof(v1_cpi_add_route_crc32c), v1_cpi_add_route_crc32c);
}

/*
 * read from a closed socket
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacketHeader_Closed)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);
    fwd_state->fd = fds[STACK];
    _setupSocket(fwd_state);

    // close remote side then try to write to it
    close(fds[REMOTE]);

    ReadReturnCode readCode = _readPacketHeader(fwd_state);

    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);

    assertTrue(readCode == ReadReturnCode_Closed, "Wrong return code, expected %d got %d", ReadReturnCode_Closed, readCode);
}

LONGBOW_TEST_CASE(UpDirectionV1, _readPacketBody_Closed)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);
    fwd_state->fd = fds[STACK];
    _setupSocket(fwd_state);

    ssize_t nwritten = write(fds[REMOTE], v1_interest_nameA, 8);
    assertTrue(nwritten == 8, "Wrong write size, expected 8 got %zd", nwritten);

    // read the header to setup the read of the body
    ReadReturnCode readCode;

    readCode = _readPacketHeader(fwd_state);
    assertTrue(readCode == ReadReturnCode_Finished, "Did not read entire header");

    // close remote side then try to write to it
    close(fds[REMOTE]);

    // now try 2nd read
    readCode = _readPacketBody(fwd_state);

    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);

    assertTrue(readCode == ReadReturnCode_Closed, "Wrong return code, expected %d got %d", ReadReturnCode_Closed, readCode);
}

/*
 * Set the socket to -1 to cause and error
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacketHeader_Error)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);
    fwd_state->fd = fds[STACK];
    _setupSocket(fwd_state);

    fwd_state->fd = -1;

    // close remote side then try to write to it

    ReadReturnCode readCode = _readPacketHeader(fwd_state);

    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
    close(fds[STACK]);
    close(fds[REMOTE]);

    assertTrue(readCode == ReadReturnCode_Error, "Wrong return code, expected %d got %d", ReadReturnCode_Error, readCode);
}

/*
 * Set the socket to -1 to cause and error
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readPacketBody_Error)
{
    const int REMOTE = 0;
    const int STACK = 1;
    int fds[2];
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fds);

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);
    fwd_state->fd = fds[STACK];
    _setupSocket(fwd_state);

    ssize_t nwritten = write(fds[REMOTE], v1_interest_nameA, 8);
    assertTrue(nwritten == 8, "Wrong write size, expected 8 got %zd", nwritten);

    // read the header to setup the read of the body
    ReadReturnCode readCode;

    readCode = _readPacketHeader(fwd_state);
    assertTrue(readCode == ReadReturnCode_Finished, "Did not read entire header");

    // invalidate to cause an error
    fwd_state->fd = -1;

    // now try 2nd read
    readCode = _readPacketBody(fwd_state);

    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
    close(fds[STACK]);
    close(fds[REMOTE]);

    assertTrue(readCode == ReadReturnCode_Error, "Wrong return code, expected %d got %d", ReadReturnCode_Error, readCode);
}

/*
 * read from a closed socket.
 * This should generate a Notify message that the connection is closed
 */
LONGBOW_TEST_CASE(UpDirectionV1, _readFromMetis_Closed)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // create our connection.  This will become part of the RTA framework, so will be
    // cleaned up in the teardown
    int api_fd;
    int client_fd;
    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);
    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);;

    rtaFramework_NonThreadedStepCount(data->framework, 5);

    close(client_fd);

    _readFromMetis(fwd_state, conn);

    // now crank the handle to pop those messages up the stack
    rtaFramework_NonThreadedStepCount(data->framework, 5);

    // now read the message out of the test component
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(rtaConnection_GetStack(conn), TESTING_UPPER, RTA_DOWN);

    // throw away the first control message
    _throwAwayControlMessage(out);

    TransportMessage *test_tm = rtaComponent_GetMessage(out);
    assertNotNull(test_tm, "Did not receive a transport message out of the top of the connector");

    assertTrue(transportMessage_IsControl(test_tm),
               "second transport message is not a control")
    {
        ccnxTlvDictionary_Display(transportMessage_GetDictionary(test_tm), 0);
    }

    // Make sure the transport message has the right properties
    CCNxTlvDictionary *testDictionary = transportMessage_GetDictionary(test_tm);
    assertTrue(ccnxControlFacade_IsNotification(testDictionary), "Control message is not Notification")
    {
        ccnxTlvDictionary_Display(testDictionary, 3);
    }

    PARCJSON *json = ccnxControlFacade_GetJson(testDictionary);
    NotifyStatus *notify = notifyStatus_ParseJSON(json);
    assertTrue(notifyStatus_GetStatusCode(notify) == notifyStatusCode_CONNECTION_CLOSED,
               "Wrong code, expected %d got %d",
               notifyStatusCode_CONNECTION_CLOSED,
               notifyStatus_GetStatusCode(notify));
    notifyStatus_Release(&notify);

    // verify other properties
    assertFalse(fwd_state->isConnected, "Forwarder state should show connection closed");

    // cleanup
    transportMessage_Destroy(&test_tm);

    // no extra cleanup, done in teardown
}

LONGBOW_TEST_FIXTURE(DownDirectionV1)
{
    LONGBOW_RUN_TEST_CASE(DownDirectionV1, _queueMessageToMetis);
    LONGBOW_RUN_TEST_CASE(DownDirectionV1, _dequeueMessagesToMetis);
    LONGBOW_RUN_TEST_CASE(DownDirectionV1, _dequeueMessagesToMetis_TwoWrites);
    LONGBOW_RUN_TEST_CASE(DownDirectionV1, _dequeueMessagesToMetis_Closed);

    LONGBOW_RUN_TEST_CASE(DownDirectionV1, connector_Fwd_Metis_Downcall_Read_Interst);
    LONGBOW_RUN_TEST_CASE(DownDirectionV1, connector_Fwd_Metis_Downcall_Read_CPIRequest);
}

LONGBOW_TEST_FIXTURE_SETUP(DownDirectionV1)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(DownDirectionV1)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * _queueMessageToMetis postconditions:
 * - increases the reference count to the wireFormat
 * - adds the reference to fwd_output buffer
 * - increments the debugging counter fwd_metis_references_queued
 */
LONGBOW_TEST_CASE(DownDirectionV1, _queueMessageToMetis)
{
    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));
    size_t expectedRefCount = parcObject_GetReferenceCount(wireFormat);

    _queueBufferMessageToMetis(wireFormat, fwd_state->metisOutputQueue);

    assertTrue(parcObject_GetReferenceCount(wireFormat) == expectedRefCount,
               "Did not get right ref count for wire format, expected %zu got %" PRIu64, expectedRefCount, parcObject_GetReferenceCount(wireFormat));
    assertTrue(parcEventBuffer_GetLength(fwd_state->metisOutputQueue) == parcBuffer_Remaining(wireFormat),
               "Wrong output buffer length, expected %zu got %zu", parcBuffer_Remaining(wireFormat), parcEventBuffer_GetLength(fwd_state->metisOutputQueue));

    parcBuffer_Release(&wireFormat);
    parcEventBuffer_Destroy(&fwd_state->metisOutputQueue);
    _fwdMetisState_Release(&fwd_state);
    parcEventScheduler_Destroy(&scheduler);
}

/*
 * Dequeue a small message to metis, should all be written out.
 */
LONGBOW_TEST_CASE(DownDirectionV1, _dequeueMessagesToMetis)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int api_fd;
    int client_fd;
    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);

    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);;

    // Put data in the output queue
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));
    _queueBufferMessageToMetis(wireFormat, fwd_state->metisOutputQueue);

    // write it out
    _dequeueMessagesToMetis(fwd_state);
    rtaFramework_NonThreadedStepCount(data->framework, 5);

    // we should now be able to read it
    bool readReady = _waitForSelect(client_fd);
    assertTrue(readReady, "client socket %d not ready for read", client_fd);

    uint8_t testArray[sizeof(v1_interest_nameA) + 1];
    ssize_t nrecv = recv(client_fd, testArray, sizeof(testArray), 0);

    assertTrue(nrecv == sizeof(v1_interest_nameA), "Wrong read length, expected %zu got %zd", sizeof(v1_interest_nameA), nrecv);
    assertTrue(memcmp(testArray, v1_interest_nameA, sizeof(v1_interest_nameA)) == 0, "Read memory does not compare");
    assertTrue(parcEventBuffer_GetLength(fwd_state->metisOutputQueue) == 0, "Metis output buffer not zero length, got %zu", parcEventBuffer_GetLength(fwd_state->metisOutputQueue));
    parcEventBuffer_Destroy(&(fwd_state->metisOutputQueue));
    parcBuffer_Release(&wireFormat);
}

/*
 * Set the forwarder's send buffer small so it will take two writes to send the packet.
 * This will test that when _dequeueMessagesToMetis cannot write the whole thing it will enable the
 * write event and that metis will then trigger a second write when there's buffer space.
 */
LONGBOW_TEST_CASE(DownDirectionV1, _dequeueMessagesToMetis_TwoWrites)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int api_fd;
    int client_fd;
    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);

    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);;

    // set the send buffer
    {
        // make it slightly bigger than 1/2
        const int sendBufferSize = sizeof(v1_interest_nameA) / 2 + 1;
        int res = setsockopt(fwd_state->fd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(int));
        if (res < 0) {
            if (DEBUG_OUTPUT) {
                printf("%9c %s failed to set SO_SNDBUF to %d: (%d) %s\n",
                       ' ', __func__, sendBufferSize, errno, strerror(errno));
            }
            // This is a non-fatal error
        }
    }

    // Put data in the output queue
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));
    _queueBufferMessageToMetis(wireFormat, fwd_state->metisOutputQueue);

    // write it out
    _dequeueMessagesToMetis(fwd_state);
    rtaFramework_NonThreadedStepCount(data->framework, 5);

    // we should now be able to read it
    bool readReady = _waitForSelect(client_fd);
    assertTrue(readReady, "client socket %d not ready for read", client_fd);

    uint8_t testArray[sizeof(v1_interest_nameA) + 1];
    ssize_t nrecv = recv(client_fd, testArray, sizeof(testArray), 0);

    assertTrue(nrecv == sizeof(v1_interest_nameA), "Wrong read length, expected %zu got %zd", sizeof(v1_interest_nameA), nrecv);
    assertTrue(memcmp(testArray, v1_interest_nameA, sizeof(v1_interest_nameA)) == 0, "Read memory does not compare");
    assertTrue(parcEventBuffer_GetLength(fwd_state->metisOutputQueue) == 0, "Metis output buffer not zero length, got %zu", parcEventBuffer_GetLength(fwd_state->metisOutputQueue));
    parcEventBuffer_Destroy(&(fwd_state->metisOutputQueue));
    parcBuffer_Release(&wireFormat);
}

/*
 * Dequeue a message to a closed socket
 */
LONGBOW_TEST_CASE(DownDirectionV1, _dequeueMessagesToMetis_Closed)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int api_fd;
    int client_fd;
    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);

    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);;
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));
    _queueBufferMessageToMetis(wireFormat, fwd_state->metisOutputQueue);

    // close remote side then try to write to it
    close(client_fd);

    _dequeueMessagesToMetis(fwd_state);
    rtaFramework_NonThreadedStepCount(data->framework, 5);

    parcEventBuffer_Destroy(&(fwd_state->metisOutputQueue));
    parcBuffer_Release(&wireFormat);
}

/**
 * Sends an Interest down the stack.  We need to create an Interest and encode its TLV wire format,
 * then send it down the stack and make sure we receive it on a client socket.  We don't actually
 * run Metis in this test.
 */
LONGBOW_TEST_CASE(DownDirectionV1, connector_Fwd_Metis_Downcall_Read_Interst)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // create our connection.  This will become part of the RTA framework, so will be
    // cleaned up in the teardown
    int api_fd;
    int client_fd;
    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);
    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);

    // Create the interest with wire format and send it down the stack
    TransportMessage *tm = trafficTools_CreateTransportMessageWithDictionaryInterest(conn, CCNxTlvDictionary_SchemaVersion_V1);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecSchemaV1PacketEncoder_DictionaryEncode(transportMessage_GetDictionary(tm), NULL);

    ccnxWireFormatMessage_PutIoVec(transportMessage_GetDictionary(tm), vec);
    ccnxCodecNetworkBufferIoVec_Release(&vec);

    // send it down the stack
    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(rtaConnection_GetStack(conn), TESTING_UPPER, RTA_DOWN);
    rtaComponent_PutMessage(in, tm);
    rtaFramework_NonThreadedStepCount(data->framework, 5);

    bool readReady = _waitForSelect(client_fd);
    assertTrue(readReady, "select did not indicate read ready");

    // now read it from out listener.  It has a read timeout so if we dont get it in a reasonable amount
    // of time, read will return an error about the timeout

    const size_t maxPacketLength = 1024;
    uint8_t packet[maxPacketLength];

    ssize_t readBytes = read(client_fd, packet, maxPacketLength);
    assertFalse(readBytes < 0, "Got error on read: (%d) %s", errno, strerror(errno));

    parcEventBuffer_Destroy(&(fwd_state->metisOutputQueue));
    close(client_fd);
}

/**
 * Send an AddRoute command down the stack.  It does not need a wire format in the transport message, its
 * the job of the forwarder to create the Metis-specific message.
 */
LONGBOW_TEST_CASE(DownDirectionV1, connector_Fwd_Metis_Downcall_Read_CPIRequest)
{
    testUnimplemented("No way to create a v1 CPI message yet");

//    TestData *data = longBowTestCase_GetClipBoardData(testCase);
//
//    // create our connection.  This will become part of the RTA framework, so will be
//    // cleaned up in the teardown
//    int api_fd;
//    int client_fd;
//    RtaConnection *conn = setupConnectionAndClientSocket(data, &api_fd, &client_fd);
//    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);
//
//    // now make the control message
//    TransportMessage *tm = trafficTools_CreateTransportMessageWithDictionaryControl(conn, CCNxTlvDictionary_SchemaVersion_V1);
//    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecSchemaV1PacketEncoder_DictionaryEncode(transportMessage_GetDictionary(tm), NULL);
//    ccnxWireFormatFacade_PutIoVec(transportMessage_GetDictionary(tm), vec);
//    ccnxCodecNetworkBufferIoVec_Release(&vec);
//
//    // send it down the stack
//    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(rtaConnection_GetStack(conn), TESTING_UPPER, RTA_DOWN);
//    rtaComponent_PutMessage(in, tm);
//    rtaFramework_NonThreadedStepCount(data->framework, 5);
//
//    // now read it from out listener.  It has a read timeout so if we dont get it in a reasonable amount
//    // of time, read will return an error about the timeout
//
//    const size_t maxPacketLength = 1024;
//    uint8_t packet[maxPacketLength];
//
//    ssize_t readBytes = read(client_fd, packet, maxPacketLength);
//    assertFalse(readBytes < 0, "Got error on read: (%d) %s", errno, strerror(errno));
//    parcEventBuffer_Destroy(&(fwd_state->metisOutputQueue));
}

// ====================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(connector_Forwarder_Metis);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
