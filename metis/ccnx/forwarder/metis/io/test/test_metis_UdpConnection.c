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

#include "../metis_UdpConnection.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/forwarder/metis/io/metis_UdpListener.h>
#include <ccnx/forwarder/metis/config/metis_Configuration.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

// for inet_pton
#include <arpa/inet.h>

#define ALICE_PORT 49018
#define BOB_PORT 49019

// ---- Used to monitor Missive messages so we know when a connection is up
typedef struct test_notifier_data {
    MetisMissiveType type;
    unsigned connectionid;
} TestNotifierData;

static void
testNotifier(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    struct test_notifier_data *data = metisMessengerRecipient_GetRecipientContext(recipient);
    data->type = metisMissive_GetType(missive);
    data->connectionid = metisMissive_GetConnectionId(missive);
    metisMissive_Release(&missive);
}

// ----

typedef struct test_tap_data {
    unsigned onReceiveCount;
    MetisMessage *message;
} TestTapData;

static bool
testTap_IsTapOnReceive(const MetisTap *tap)
{
    return true;
}

static void
testTap_TapOnReceive(MetisTap *tap, const MetisMessage *message)
{
    TestTapData *mytap = (TestTapData *) tap->context;
    mytap->onReceiveCount++;
    if (mytap->message) {
        metisMessage_Release(&mytap->message);
    }

    mytap->message = metisMessage_Acquire(message);
}

static MetisTap testTapTemplate = {
    .context        = NULL,
    .isTapOnReceive = &testTap_IsTapOnReceive,
    .isTapOnSend    = NULL,
    .isTapOnDrop    = NULL,
    .tapOnReceive   = &testTap_TapOnReceive,
    .tapOnSend      = NULL,
    .tapOnDrop      = NULL
};

// --- Used to inspect packets received


typedef struct test_data {
    int remoteSocket;

#define ALICE 0
#define BOB 1

    MetisForwarder *metis[2];
    MetisListenerOps *listener[2];
    MetisMessengerRecipient *recipient[2];
    TestNotifierData notifierData[2];
    MetisTap taps[2];
    TestTapData tapData[2];
} TestData;



static void
_crankHandle(TestData *data)
{
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis[ALICE]), &((struct timeval) {0, 10000}));
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis[BOB]), &((struct timeval) {0, 10000}));
}

static void
_setup(TestData *data, int side, uint16_t port)
{
    data->metis[side] = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(data->metis[side]), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));

    data->listener[side] = metisUdpListener_CreateInet(data->metis[side], addr);

    // snoop events
    data->recipient[side] = metisMessengerRecipient_Create(&data->notifierData[side], testNotifier);
    MetisMessenger *messenger = metisForwarder_GetMessenger(data->metis[side]);
    metisMessenger_Register(messenger, data->recipient[side]);

    // snoop packets
    memcpy(&data->taps[side], &testTapTemplate, sizeof(testTapTemplate));
    data->taps[side].context = &data->tapData[side];
    metisForwarder_AddTap(data->metis[side], &data->taps[side]);

    // save in Metis
    metisListenerSet_Add(metisForwarder_GetListenerSet(data->metis[side]), data->listener[side]);
}

/*
 * Create a UDP socket pair
 */
static void
_commonSetup(const LongBowTestCase *testCase)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));

    _setup(data, ALICE, ALICE_PORT);
    _setup(data, BOB, BOB_PORT);

    _crankHandle(data);

    longBowTestCase_SetClipBoardData(testCase, data);
}

static void
_commonTeardown(const LongBowTestCase *testCase)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // the listeners are stored in the respectie Metis, so we don't need to
    // destroy those separately.

    metisMessengerRecipient_Destroy(&data->recipient[ALICE]);
    metisMessengerRecipient_Destroy(&data->recipient[BOB]);

    if (data->tapData[ALICE].message) {
        metisMessage_Release(&data->tapData[ALICE].message);
    }

    if (data->tapData[BOB].message) {
        metisMessage_Release(&data->tapData[BOB].message);
    }

    metisForwarder_Destroy(&data->metis[ALICE]);
    metisForwarder_Destroy(&data->metis[BOB]);

    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(metis_UdpConnection)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_UdpConnection)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_UdpConnection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    //XXX: this test does not work anymore because we do not create the connection in metis
    //LONGBOW_RUN_TEST_CASE(Global, metisUdpConnection_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _commonSetup(testCase);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * Create connection from ALICE to BOB
 */
LONGBOW_TEST_CASE(Global, metisUdpConnection_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);
    int fd = data->listener[ALICE]->getSocket(data->listener[ALICE]);
    MetisIoOperations *ops = metisUdpConnection_Create(data->metis[ALICE], fd, pair, false);
    metisAddressPair_Release(&pair);

    _crankHandle(data);

    // send a data packet to bring it up on BOB
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV1_Interest_NameA_Crc32c, sizeof(metisTestDataV1_Interest_NameA_Crc32c), 2, 3, metisForwarder_GetLogger(data->metis[ALICE]));

    ops->send(ops, NULL, message);

    metisMessage_Release(&message);

    // wait until we indicate that the connection is up on Bob's side
    while (data->notifierData[BOB].type == MetisMissiveType_ConnectionDestroyed) {
        _crankHandle(data);
    }

    // and verify that the message was sent in to the message processor on Bob's side
    assertTrue(data->tapData[BOB].onReceiveCount == 1, "Wrong receive count, expected 1 got %u", data->tapData[BOB].onReceiveCount);

    ops->destroy(&ops);
}

// ===========================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _saveSockaddr_INET);
    LONGBOW_RUN_TEST_CASE(Local, _saveSockaddr_INET6);
    LONGBOW_RUN_TEST_CASE(Local, _send);
    LONGBOW_RUN_TEST_CASE(Local, _getRemoteAddress);
    LONGBOW_RUN_TEST_CASE(Local, _getAddressPair);
    LONGBOW_RUN_TEST_CASE(Local, _getConnectionId);
    LONGBOW_RUN_TEST_CASE(Local, _isUp);
    LONGBOW_RUN_TEST_CASE(Local, _isLocal_True);
    LONGBOW_RUN_TEST_CASE(Local, _setConnectionState);
    LONGBOW_RUN_TEST_CASE(Local, _getConnectionType);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    _commonSetup(testCase);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    _commonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _saveSockaddr_INET)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    _MetisUdpState *udpConnState = parcMemory_AllocateAndClear(sizeof(_MetisUdpState));
    assertNotNull(udpConnState, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisUdpState));

    udpConnState->metis = data->metis[ALICE];
    udpConnState->logger = metisLogger_Acquire(metisForwarder_GetLogger(udpConnState->metis));

    struct sockaddr_in sin1, sin2;

    memset(&sin1, 0, sizeof(sin1));
    sin1.sin_family = AF_INET;
    sin1.sin_port = htons(ALICE_PORT);
    inet_pton(AF_INET, "127.0.0.1", &(sin1.sin_addr));

    memset(&sin2, 0, sizeof(sin2));
    sin2.sin_family = AF_INET;
    sin2.sin_port = htons(BOB_PORT);
    inet_pton(AF_INET, "127.0.0.1", &(sin2.sin_addr));

    CPIAddress *aliceAddress = cpiAddress_CreateFromInet(&sin1);
    CPIAddress *bobAddress = cpiAddress_CreateFromInet(&sin2);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);

    bool saved = _saveSockaddr(udpConnState, pair);
    assertTrue(saved, "Failed to save address");
    assertTrue(udpConnState->peerAddressLength == sizeof(sin1), "Wrong length, expected %zu got %u", sizeof(sin1), udpConnState->peerAddressLength);

    cpiAddress_Destroy(&aliceAddress);
    cpiAddress_Destroy(&bobAddress);

    metisAddressPair_Release(&pair);
    metisLogger_Release(&udpConnState->logger);
    parcMemory_Deallocate((void **) &udpConnState->peerAddress);
    parcMemory_Deallocate((void **) &udpConnState);
}

LONGBOW_TEST_CASE(Local, _saveSockaddr_INET6)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    _MetisUdpState *udpConnState = parcMemory_AllocateAndClear(sizeof(_MetisUdpState));
    assertNotNull(udpConnState, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisUdpState));

    udpConnState->metis = data->metis[ALICE];
    udpConnState->logger = metisLogger_Acquire(metisForwarder_GetLogger(udpConnState->metis));

    struct sockaddr_in6 sin1, sin2;

    memset(&sin1, 0, sizeof(sin1));
    sin1.sin6_family = AF_INET6;
    sin1.sin6_port = htons(ALICE_PORT);
    int ok = inet_pton(AF_INET6, "::1", &(sin1.sin6_addr));

    if (ok) {
        memset(&sin2, 0, sizeof(sin2));
        sin2.sin6_family = AF_INET6;
        sin2.sin6_port = htons(BOB_PORT);
        inet_pton(AF_INET6, "::1", &(sin2.sin6_addr));

        CPIAddress *aliceAddress = cpiAddress_CreateFromInet6(&sin1);
        CPIAddress *bobAddress = cpiAddress_CreateFromInet6(&sin2);

        MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);

        bool saved = _saveSockaddr(udpConnState, pair);
        assertTrue(saved, "Failed to save address");
        assertTrue(udpConnState->peerAddressLength == sizeof(sin1), "Wrong length, expected %zu got %u", sizeof(sin1), udpConnState->peerAddressLength);

        cpiAddress_Destroy(&aliceAddress);
        cpiAddress_Destroy(&bobAddress);

        metisAddressPair_Release(&pair);
    } else {
        testSkip("Skipping inet6 test");
    }

    metisLogger_Release(&udpConnState->logger);
    parcMemory_Deallocate((void **) &udpConnState->peerAddress);
    parcMemory_Deallocate((void **) &udpConnState);
}

LONGBOW_TEST_CASE(Local, _send)
{
}

LONGBOW_TEST_CASE(Local, _getRemoteAddress)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);
    int fd = data->listener[ALICE]->getSocket(data->listener[ALICE]);
    MetisIoOperations *ops = metisUdpConnection_Create(data->metis[ALICE], fd, pair, false);
    metisAddressPair_Release(&pair);

    // now run test
    const CPIAddress *test = _getRemoteAddress(ops);
    assertNotNull(test, "Got null remote address");
    assertTrue(cpiAddress_Equals(test, bobAddress), "Addresses do not match");
    ops->destroy(&ops);
}

LONGBOW_TEST_CASE(Local, _getAddressPair)
{
}

LONGBOW_TEST_CASE(Local, _getConnectionId)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);
    int fd = data->listener[ALICE]->getSocket(data->listener[ALICE]);
    MetisIoOperations *ops = metisUdpConnection_Create(data->metis[ALICE], fd, pair, false);
    metisAddressPair_Release(&pair);

    // now run test
    unsigned connid = _getConnectionId(ops);

    assertTrue(connid > 0, "Expected positive connid, got 0");
    ops->destroy(&ops);
}

LONGBOW_TEST_CASE(Local, _isUp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);
    int fd = data->listener[ALICE]->getSocket(data->listener[ALICE]);
    MetisIoOperations *ops = metisUdpConnection_Create(data->metis[ALICE], fd, pair, false);
    metisAddressPair_Release(&pair);

    // now run test
    bool isup = _isUp(ops);

    assertTrue(isup, "Expected connection to be up");
    ops->destroy(&ops);
}

LONGBOW_TEST_CASE(Local, _isLocal_True)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);
    int fd = data->listener[ALICE]->getSocket(data->listener[ALICE]);
    MetisIoOperations *ops = metisUdpConnection_Create(data->metis[ALICE], fd, pair, true);
    metisAddressPair_Release(&pair);

    // now run test
    bool islocal = _isLocal(ops);

    assertTrue(islocal, "Expected connection to be local");
    ops->destroy(&ops);
}

LONGBOW_TEST_CASE(Local, _setConnectionState)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);
    int fd = data->listener[ALICE]->getSocket(data->listener[ALICE]);
    MetisIoOperations *ops = metisUdpConnection_Create(data->metis[ALICE], fd, pair, true);
    metisAddressPair_Release(&pair);

    // now run test
    _MetisUdpState *udpConnState = (_MetisUdpState *) metisIoOperations_GetClosure(ops);

    _setConnectionState(udpConnState, false);
    bool isup = _isUp(ops);

    assertFalse(isup, "Expected connection to be down");
    ops->destroy(&ops);
}

LONGBOW_TEST_CASE(Local, _getConnectionType)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisAddressPair *pair = metisAddressPair_Create(aliceAddress, bobAddress);
    int fd = data->listener[ALICE]->getSocket(data->listener[ALICE]);
    MetisIoOperations *ops = metisUdpConnection_Create(data->metis[ALICE], fd, pair, false);
    metisAddressPair_Release(&pair);

    // now run test
    CPIConnectionType connType = _getConnectionType(ops);

    assertTrue(connType == cpiConnection_UDP, "Expected connection to be %d got %d", cpiConnection_UDP, connType);
    ops->destroy(&ops);
}


// ===========================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_UdpConnection);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
