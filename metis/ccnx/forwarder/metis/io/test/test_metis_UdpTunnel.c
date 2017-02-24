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
 * Need a solution to avoid hard-coding port numbers
 *
 */

#include "../metis_UdpTunnel.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/forwarder/metis/io/metis_UdpListener.h>
#include <ccnx/forwarder/metis/config/metis_Configuration.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

// for inet_pton
#include <arpa/inet.h>

#define ALICE_PORT 49028
#define BOB_PORT 49029

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
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis[ALICE]),  &((struct timeval) {0, 10000}));
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis[BOB]),  &((struct timeval) {0, 10000}));
}

static void
_setup(TestData *data, int side, uint16_t port)
{
    data->metis[side] = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(data->metis[side]), MetisLoggerFacility_IO, PARCLogLevel_Error);

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

// ==================================

LONGBOW_TEST_RUNNER(metis_UdpTunnel)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_UdpTunnel)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_UdpTunnel)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisUdpTunnel_CreateOnListener);
    //XXX: this test does not work anymore beacuase we don't create the connection in metis
    //LONGBOW_RUN_TEST_CASE(Global, metisUdpTunnel_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisUdpTunnel_Create_MismatchedTypes);
    LONGBOW_RUN_TEST_CASE(Global, metisUdpTunnel_Create_NotFound);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _commonSetup(testCase);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(testCase);
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_CASE(Global, metisUdpTunnel_CreateOnListener)
{
}

/*
 * Create from Alice to Bob
 */
LONGBOW_TEST_CASE(Global, metisUdpTunnel_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);
    const CPIAddress *bobAddress = data->listener[BOB]->getListenAddress(data->listener[BOB]);

    MetisIoOperations *ops = metisUdpTunnel_Create(data->metis[ALICE], aliceAddress, bobAddress);

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

/*
 * sockets not same address family
 */
LONGBOW_TEST_CASE(Global, metisUdpTunnel_Create_MismatchedTypes)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    const CPIAddress *aliceAddress = data->listener[ALICE]->getListenAddress(data->listener[ALICE]);

    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    CPIAddress *bobAddress = cpiAddress_CreateFromInet6(&sin6);

    MetisIoOperations *ops = metisUdpTunnel_Create(data->metis[ALICE], aliceAddress, bobAddress);
    assertNull(ops, "Should have gotten null return for mismatched address types");

    cpiAddress_Destroy(&bobAddress);
}

/*
 * Listener not found
 */
LONGBOW_TEST_CASE(Global, metisUdpTunnel_Create_NotFound)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set Bob's notifier to a known bad state
    data->notifierData[BOB].type = MetisMissiveType_ConnectionDestroyed;

    // Create a connection from Alice to Bob
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    CPIAddress *aliceAddress = cpiAddress_CreateFromInet(&sin);
    CPIAddress *bobAddress = cpiAddress_CreateFromInet(&sin);

    MetisIoOperations *ops = metisUdpTunnel_Create(data->metis[ALICE], aliceAddress, bobAddress);
    assertNull(ops, "Should have gotten null return for mismatched address types");

    cpiAddress_Destroy(&aliceAddress);
    cpiAddress_Destroy(&bobAddress);
}



// ==================================


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_UdpTunnel);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
