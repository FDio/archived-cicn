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
#include "../metis_StreamConnection.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

// inet_pton
#include <arpa/inet.h>

#include <fcntl.h>

#include <stdio.h>

#ifndef INPORT_ANY
#define INPORT_ANY 0
#endif

// we hand-code some packets in the unit tests
typedef struct __attribute__ ((__packed__)) metis_tlv_fixed_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t payloadLength;
    uint16_t reserved;
    uint16_t headerLength;
} _MetisTlvFixedHeaderV0;


LONGBOW_TEST_RUNNER(metis_StreamConnection)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);

    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_StreamConnection)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_StreamConnection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ================================================================================
LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisStreamConnection_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamConnection_OpenConnection);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisStreamConnection_Create)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fd, pair, false);

    ops->destroy(&ops);
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    metisForwarder_Destroy(&metis);
    close(fd);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
    assertTrue(parcSafeMemory_Outstanding() == 0, "Got memory imbalance: %u", parcSafeMemory_Outstanding());
}

static int
listenToInet(struct sockaddr_in *server)
{
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    assertFalse(fd < 0, "error on bind: (%d) %s", errno, strerror(errno));

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    failure = bind(fd, (struct sockaddr *) server, sizeof(struct sockaddr_in));
    assertFalse(failure, "error on bind: (%d) %s", errno, strerror(errno));

    failure = listen(fd, 16);
    assertFalse(failure, "error on listen: (%d) %s", errno, strerror(errno));

    return fd;
}

LONGBOW_TEST_CASE(Global, metisStreamConnection_OpenConnection)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = INPORT_ANY;
    inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr));

    int serverSocket = listenToInet(&serverAddr);
    socklen_t x = sizeof(serverAddr);
    int failure = getsockname(serverSocket, (struct sockaddr *) &serverAddr, &x);
    assertFalse(failure, "error on getsockname: (%d) %s", errno, strerror(errno));

    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = PF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = INPORT_ANY;

    CPIAddress *local = cpiAddress_CreateFromInet(&localAddr);

    // change from 0.0.0.0 to 127.0.0.1
    inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr));

    CPIAddress *remote = cpiAddress_CreateFromInet(&serverAddr);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);

    MetisIoOperations *ops = metisStreamConnection_OpenConnection(metis, pair, false);
    assertNotNull(ops, "Got null ops from metisStreamConnection_OpenConnection");
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
}

// =======================================================================

// ==================================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, conn_eventcb_Connected);
    LONGBOW_RUN_TEST_CASE(Local, conn_eventcb_EOF);
    LONGBOW_RUN_TEST_CASE(Local, conn_eventcb_ERROR);

    LONGBOW_RUN_TEST_CASE(Local, conn_readcb);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_Equals);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_GetAddress);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_GetAddressPair);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_GetConnectionId);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_HashCode);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_IsUp);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_Send);
    LONGBOW_RUN_TEST_CASE(Local, metisStreamConnection_GetConnectionType);
    LONGBOW_RUN_TEST_CASE(Local, printConnection);
    LONGBOW_RUN_TEST_CASE(Local, readMessage);
    LONGBOW_RUN_TEST_CASE(Local, setConnectionState);
    LONGBOW_RUN_TEST_CASE(Local, single_read_ZeroNextMessageLength);
    LONGBOW_RUN_TEST_CASE(Local, single_read_PartialRead);
    LONGBOW_RUN_TEST_CASE(Local, single_read_FullRead);
    LONGBOW_RUN_TEST_CASE(Local, startNewMessage);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, conn_eventcb_Connected)
{
    int fds[2];
    int failure = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    assertFalse(failure, "Error socketpair: (%d) %s", errno, strerror(errno));

    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fds[0], pair, false);
    _MetisStreamState *stream = (_MetisStreamState *) metisIoOperations_GetClosure(ops);

    stream->isUp = false;

    // ---- the actual test
    _conn_eventcb(stream->bufferEventVector, PARCEventQueueEventType_Connected, ops);
    assertTrue(stream->isUp, "PARCEventQueueEventType_Connected did not trigger stream to up state");
    // ----

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    close(fds[0]);
    close(fds[1]);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
}

LONGBOW_TEST_CASE(Local, conn_eventcb_EOF)
{
    int fds[2];
    int failure = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    assertFalse(failure, "Error socketpair: (%d) %s", errno, strerror(errno));

    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fds[0], pair, false);
    _MetisStreamState *stream = (_MetisStreamState *) metisIoOperations_GetClosure(ops);

    stream->isUp = true;

    // ---- the actual test
    _conn_eventcb(stream->bufferEventVector, PARCEventQueueEventType_EOF, ops);
    assertFalse(stream->isUp, "PARCEventQueueEventType_EOF did not trigger stream to down state");
    // ----

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    close(fds[0]);
    close(fds[1]);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
}

LONGBOW_TEST_CASE(Local, conn_eventcb_ERROR)
{
    int fds[2];
    int failure = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    assertFalse(failure, "Error socketpair: (%d) %s", errno, strerror(errno));

    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fds[0], pair, false);
    _MetisStreamState *stream = (_MetisStreamState *) metisIoOperations_GetClosure(ops);

    stream->isUp = true;

    // ---- the actual test
    _conn_eventcb(stream->bufferEventVector, PARCEventQueueEventType_Error, ops);
    assertFalse(stream->isUp, "PARCEventQueueEventType_Error did not trigger stream to down state");
    // ----

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    close(fds[0]);
    close(fds[1]);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
}

LONGBOW_TEST_CASE(Local, conn_readcb)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_Equals)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_GetAddress)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fd, pair, false);

    const CPIAddress *test_addr = ops->getRemoteAddress(ops);

    assertTrue(cpiAddress_Equals(remote, test_addr), "ops->getAddress incorrect");

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);

    close(fd);
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_GetAddressPair)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fd, pair, false);

    const MetisAddressPair *test_pair = ops->getAddressPair(ops);

    assertTrue(metisAddressPair_Equals(pair, test_pair), "ops->getRemoteAddress incorrect");

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);

    close(fd);
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_GetConnectionId)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    unsigned truth_connid = metisForwarder_GetNextConnectionId(metis) + 1;

    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fd, pair, false);

    assertTrue(ops->getConnectionId(ops) == truth_connid, "Got wrong connection id, expected %u got %u", truth_connid, ops->getConnectionId(ops));

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);

    close(fd);
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_HashCode)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_IsUp)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fd, pair, false);

    assertTrue(ops->isUp(ops), "isUp incorrect, expected true, got false");

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
    close(fd);
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_Send)
{
    // StreamConnection_Create needs a socket and address to represent the peer
    // we use a socket pair so we can actaully read from it and verify what is sent.

    int fds[2];
    int failure = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    assertFalse(failure, "Error socketpair: (%d) %s", errno, strerror(errno));

    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);
    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fds[0], pair, false);

    // ----------
    // Create a fake message.  Send does not care what the message is, it just writes it out.
    // We include a real header, but it is not needed.

    char message_str[] = "\x00Once upon a jiffie, in a stack far away, a dangling pointer found its way to the top of the heap.";
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) message_str;
    hdr->payloadLength = htons(92);
    hdr->headerLength = htons(0);

    MetisMessage *sendmessage = metisMessage_CreateFromArray((uint8_t *) message_str, sizeof(message_str), 1, 2, metisForwarder_GetLogger(metis));

    // ----------
    // actually send it
    ops->send(ops, NULL, sendmessage);
    metisMessage_Release(&sendmessage);

    // ----------
    // turn the handleto crank
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    // ----------
    // Now read the result from our end of the socket pair.

    uint8_t read_buffer[1024];

    // read it and verify
    ssize_t read_length = read(fds[1], read_buffer, 1024);
    assertTrue(read_length == sizeof(message_str),
               "Incorrect read length, expected %zu got %zd: (%d) %s",
               sizeof(message_str), read_length, errno, strerror(errno));

    assertTrue(memcmp(read_buffer, message_str, sizeof(message_str)) == 0, "read_buffer does not match message_str");

    // ----------
    // hurray, no messages where harmed in this experiment

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    close(fds[0]);
    close(fds[1]);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
}

LONGBOW_TEST_CASE(Local, metisStreamConnection_GetConnectionType)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr_local;
    addr_local.sin_addr.s_addr = htonl(0x01020304);
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(56);

    struct sockaddr_in addr_remote;
    addr_remote.sin_addr.s_addr = htonl(0x0708090A);
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_port = htons(12);

    CPIAddress *local = cpiAddress_CreateFromInet(&addr_local);
    CPIAddress *remote = cpiAddress_CreateFromInet(&addr_remote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(metis, fd, pair, false);

    CPIConnectionType connType = _metisStreamConnection_GetConnectionType(ops);
    assertTrue(connType == cpiConnection_TCP, "Wrong connection type expected %d got %d", cpiConnection_TCP, connType);

    ops->destroy(&ops);
    metisForwarder_Destroy(&metis);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
    close(fd);
}

LONGBOW_TEST_CASE(Local, printConnection)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, readMessage)
{
    char message_str[] = "\x00Once upon a jiffie, in a stack far away, a dangling pointer found its way to the top of the heap.";
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) message_str;
    hdr->payloadLength = htons(92);
    hdr->headerLength = htons(0);

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    _MetisStreamState *stream = parcMemory_AllocateAndClear(sizeof(_MetisStreamState));
    assertNotNull(stream, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisStreamState));
    stream->nextMessageLength = parcEventBuffer_GetLength(buff);
    stream->id = 77;

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    stream->logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    MetisMessage *message = _readMessage(stream, 444, buff);

    assertNotNull(message, "Got null message from readMessage");
    assertTrue(parcEventBuffer_GetLength(buff) == 0, "Did not drain input buffer, expected 0 got %zu", parcEventBuffer_GetLength(buff));
    //assertTrue(metisMessage_Length(message) == sizeof(message_str),
    //"Message length wrong, expected %zu got %zu",
    //sizeof(message_str),
    //metisMessage_Length(message));

    metisMessage_Release(&message);
    parcEventBuffer_Destroy(&buff);
    metisLogger_Release(&stream->logger);
    parcMemory_Deallocate((void **) &stream);
}

LONGBOW_TEST_CASE(Local, setConnectionState)
{
    testUnimplemented("This test is unimplemented");
}

/**
 * Call like the beignning of a new packet, with stream->nextMessageLength set to 0
 */
LONGBOW_TEST_CASE(Local, single_read_ZeroNextMessageLength)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();

    // do it like a short read, only 12 bytes
    parcEventBuffer_Append(buff, metisTestDataV0_EncodedInterest, 12);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    _MetisStreamState *stream = parcMemory_AllocateAndClear(sizeof(_MetisStreamState));
    assertNotNull(stream, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisStreamState));
    stream->metis = metis;
    stream->nextMessageLength = 0;
    stream->id = 77;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    stream->logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = _single_read(buff, stream);

    assertNull(message, "message should be null, its a short read");
    assertTrue(parcEventBuffer_GetLength(buff) == 12, "Should not have drained buffer, expected %d got %zu", 12, parcEventBuffer_GetLength(buff));
    assertTrue(stream->nextMessageLength == sizeof(metisTestDataV0_EncodedInterest),
               "NextMessageLength not set correctly, expected %zu got %zu",
               sizeof(metisTestDataV0_EncodedInterest),
               stream->nextMessageLength);

    parcEventBuffer_Destroy(&buff);
    metisLogger_Release(&stream->logger);
    parcMemory_Deallocate((void **) &stream);
    metisForwarder_Destroy(&metis);
}

/**
 * Call with stream->nextMessageLength set correctly, but not enough bytes in the buffer
 */
LONGBOW_TEST_CASE(Local, single_read_PartialRead)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();

    // do it like a short read, only 12 bytes
    parcEventBuffer_Append(buff, metisTestDataV0_EncodedInterest, 12);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    _MetisStreamState *stream = parcMemory_AllocateAndClear(sizeof(_MetisStreamState));
    assertNotNull(stream, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisStreamState));
    stream->metis = metis;
    stream->nextMessageLength = sizeof(metisTestDataV0_EncodedInterest);
    stream->id = 77;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    stream->logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = _single_read(buff, stream);

    assertNull(message, "message should be null, its a short read");
    assertTrue(parcEventBuffer_GetLength(buff) == 12, "Should not have drained buffer, expected %d got %zu", 12, parcEventBuffer_GetLength(buff));
    assertTrue(stream->nextMessageLength == sizeof(metisTestDataV0_EncodedInterest),
               "NextMessageLength not set correctly, expected %zu got %zu",
               sizeof(metisTestDataV0_EncodedInterest),
               stream->nextMessageLength);

    parcEventBuffer_Destroy(&buff);
    metisLogger_Release(&stream->logger);
    parcMemory_Deallocate((void **) &stream);
    metisForwarder_Destroy(&metis);
}

/**
 * Call with enough bytes in the buffer to read the whole message
 */
LONGBOW_TEST_CASE(Local, single_read_FullRead)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();

    // do it like a full read
    parcEventBuffer_Append(buff, metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest));

    MetisForwarder *metis = metisForwarder_Create(NULL);
    _MetisStreamState *stream = parcMemory_AllocateAndClear(sizeof(_MetisStreamState));
    assertNotNull(stream, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisStreamState));
    stream->metis = metis;
    stream->nextMessageLength = sizeof(metisTestDataV0_EncodedInterest);
    stream->id = 77;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    stream->logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = _single_read(buff, stream);

    assertNotNull(message, "message should not be null, its a short read");
    assertTrue(parcEventBuffer_GetLength(buff) == 0, "Should have drained buffer, expected %d got %zu", 0, parcEventBuffer_GetLength(buff));

    // should reset the next message length after reading a whole packet
    assertTrue(stream->nextMessageLength == 0,
               "NextMessageLength not set correctly, expected %u got %zu",
               0,
               stream->nextMessageLength);

    metisMessage_Release(&message);
    parcEventBuffer_Destroy(&buff);
    metisLogger_Release(&stream->logger);
    parcMemory_Deallocate((void **) &stream);
    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Local, startNewMessage)
{
    _MetisStreamState *stream = parcMemory_AllocateAndClear(sizeof(_MetisStreamState));
    assertNotNull(stream, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisStreamState));

    // add data to the buffer to fake out having read from the network
    PARCEventBuffer *buff = parcEventBuffer_Create();
    uint8_t *truth_message = parcMemory_Allocate(100);
    assertNotNull(truth_message, "parcMemory_Allocate(%u) returned NULL", 100);

    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) truth_message;
    hdr->version = 0;
    hdr->payloadLength = htons(92);
    hdr->headerLength = htons(0);

    parcEventBuffer_Append(buff, truth_message, 100);

    stream->nextMessageLength = 0;

    _startNewMessage(stream, buff, 100);

    assertTrue(stream->nextMessageLength == 100, "nextMessageLength wrong, expected %d got %zu", 100, stream->nextMessageLength);

    parcEventBuffer_Destroy(&buff);
    parcMemory_Deallocate((void **) &stream);
    parcMemory_Deallocate((void **) &truth_message);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_StreamConnection);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
