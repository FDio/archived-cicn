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
 * These are end-to-end system tests.  They nail up two Ethernet connections, setup a FIB entry, and send
 * and interest then a content object back.
 *
 * To Metis, it looks like it has 2 ethernet interfaces, "fake0" and "fake1".  We send an Interest
 * in "fake0" and it should come out "fake1", then send a content object back the other way.
 *
 * Uses the mock GenericEthernet object so there's no actual network required.
 *
 */

#include <config.h>
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>

// include the mock Ethernet implementation
#include "../io/test/testrig_GenericEther.c"
#include <ccnx/forwarder/metis/io/metis_EtherListener.c>

#include <ccnx/api/control/cpi_RouteEntry.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

#define ETHERTYPE 0x0801

LONGBOW_TEST_RUNNER(sys_EtherEndToEnd)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(sys_EtherEndToEnd)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(sys_EtherEndToEnd)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * Used to monitor Missive messages so we know when
 * a connection is setup
 */
static struct test_notifier_data {
    MetisMissiveType type;
    unsigned connectionid;
} testNotifierData;

static void
_testNotifier(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    struct test_notifier_data *data = metisMessengerRecipient_GetRecipientContext(recipient);
    data->type = metisMissive_GetType(missive);
    data->connectionid = metisMissive_GetConnectionId(missive);
    metisMissive_Release(&missive);
}

// ---- Utility functions to setup Ethernet endpoints

static MetisListenerOps *
_setupEtherListener(MetisForwarder *metis, const char *devName, uint16_t ethertype)
{
    MetisListenerSet *listeners = metisForwarder_GetListenerSet(metis);
    MetisListenerOps *ops = metisEtherListener_Create(metis, devName, ethertype);
    metisListenerSet_Add(listeners, ops);

    // crank the libevent handle
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));
    return ops;
}

// ---- Clipboard state used by tests

typedef struct test_data {
    MetisForwarder *metis;
    MetisMessengerRecipient *recipient;

#define CLIENTSIDE 0
#define SERVERSIDE 1

    MetisListenerOps *etherListener[2];
    MetisGenericEther *ether[2];

    // These are the peer addresses off ether0 and ether1 (i.e. the "remote" systems
    // that are sending the frames to metis).
    uint8_t ether_peerAddress[ETHER_ADDR_LEN][2];
} TestData;

static void
_sendPing(TestData *data, int side)
{
    PARCBuffer *ether0_mac = metisGenericEther_GetMacAddress(data->ether[side]);
    PARCBuffer *frame = mockGenericEther_createFrame(sizeof(metisTestDataV0_EncodedInterest), metisTestDataV0_EncodedInterest,  parcBuffer_Overlay(ether0_mac, 0), data->ether_peerAddress[side], ETHERTYPE);

    mockGenericEther_QueueFrame(data->ether[side], frame);
    mockGenericEther_Notify(data->ether[side]);

    parcBuffer_Release(&frame);
}

static void
_bringUpListener(TestData *data, int side, const char *devName)
{
    data->etherListener[side] = _setupEtherListener(data->metis, devName, ETHERTYPE);
    data->ether[side] = ((_MetisEtherListener *) data->etherListener[side]->context)->genericEther;

    // send a ping to Metis to bring up a connection
    _sendPing(data, side);

    // crank the dispatcher handle
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis), &((struct timeval) {0, 10000}));
    printf("side %d connection id = %u\n", side, testNotifierData.connectionid);
}

static void
_commonSetup(const LongBowTestCase *testCase)
{
    TestData *data = parcMemory_Allocate(sizeof(TestData));

    data->metis = metisForwarder_Create(NULL);

    // setup a messenger recpient so we get a notification when the connection is up
    data->recipient = metisMessengerRecipient_Create(&testNotifierData, _testNotifier);

    // setup the peer addresses (need to do this before bringing up the listeners)
    memset(data->ether_peerAddress[CLIENTSIDE], 0xAA, ETHER_ADDR_LEN);
    memset(data->ether_peerAddress[SERVERSIDE], 0xBB, ETHER_ADDR_LEN);

    // register a messenger callback so we know when the connections get setup
    MetisMessenger *messenger = metisForwarder_GetMessenger(data->metis);
    metisMessenger_Register(messenger, data->recipient);

    _bringUpListener(data, CLIENTSIDE, "fake0");

    _bringUpListener(data, SERVERSIDE, "fake1");

    // Add a FIB entry out the receiver connection (testNotifierData.connectionid will be set to the "fake1" connection id
    // because it was the last thing to get a missive sent)
    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, testNotifierData.connectionid, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    metisForwarder_AddOrUpdateRoute(data->metis, routeAdd);
    cpiRouteEntry_Destroy(&routeAdd);

    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis), &((struct timeval) { 0, 10000 }));

    longBowTestCase_SetClipBoardData(testCase, data);
}

static void
_commonTeardown(const LongBowTestCase *testCase)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    metisForwarder_Destroy(&data->metis);
    metisMessengerRecipient_Destroy(&data->recipient);
    parcMemory_Deallocate((void **) &data);
}

// ==========================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, passInterest_Unicast);
    LONGBOW_RUN_TEST_CASE(Global, returnContentObject);
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

/*
 * Send the Interest to the client socket (ether0).
 * Returns the frame sent (with Ethernet header)
 */
static PARCBuffer *
_sendInterest(TestData *data)
{
    PARCBuffer *ether0_mac = metisGenericEther_GetMacAddress(data->ether[CLIENTSIDE]);
    PARCBuffer *frame = mockGenericEther_createFrame(sizeof(metisTestDataV0_InterestWithName_keyid), metisTestDataV0_InterestWithName_keyid,  parcBuffer_Overlay(ether0_mac, 0), data->ether_peerAddress[CLIENTSIDE], ETHERTYPE);

    mockGenericEther_QueueFrame(data->ether[CLIENTSIDE], frame);

    // wake up the client's side of metis to receive a frame
    mockGenericEther_Notify(data->ether[CLIENTSIDE]);

    return frame;
}

/*
 * Receives an Interest Ethernet frame on the server's side of Metis and verifies it matches what was sent
 * Has to recompute the hop-limit in the truthFrame, uses magic knowledge of V0 packets.
 */
static void
_receiveInterest(TestData *data, PARCBuffer *truthFrame)
{
    uint8_t receiveBuffer[1024];

    int serverSocket = mockGenericEther_GetTestDescriptor(data->ether[SERVERSIDE]);

    ssize_t read_length = read(serverSocket, receiveBuffer, 1024);
    assertTrue(read_length == parcBuffer_Remaining(truthFrame),
               "Incorrect read, expected %zd got %zd: (%d) %s",
               parcBuffer_Remaining(truthFrame), read_length, errno, strerror(errno));

    // skip over the ethernet header
    parcBuffer_SetPosition(truthFrame, sizeof(struct ether_header));

    // We decrement the hoplimit, so need a new truth value
    // TODO: There's a lot of magic knowledge that the hoplimit is at byte offset 12 and
    // originally had the value 32.
    parcBuffer_PutAtIndex(truthFrame, sizeof(struct ether_header) + 12, 31);

    PARCBuffer *testFrame = parcBuffer_Wrap(receiveBuffer, parcBuffer_Limit(truthFrame), sizeof(struct ether_header), parcBuffer_Limit(truthFrame));
    assertTrue(parcBuffer_Equals(truthFrame, testFrame), "Messages do not match")
    {
        parcBuffer_Display(truthFrame, 3);
        parcBuffer_Display(testFrame, 3);
    }

    parcBuffer_Release(&testFrame);
}

/*
 * Send the Interest to the client socket (ether0).
 * Returns the frame sent (with Ethernet header)
 */
static PARCBuffer *
_sendContentObject(TestData *data)
{
    // metisTestDataV0_EncodedObject has the same name and keyid as in the Interest

    PARCBuffer *ether0_mac = metisGenericEther_GetMacAddress(data->ether[SERVERSIDE]);
    PARCBuffer *frame = mockGenericEther_createFrame(sizeof(metisTestDataV0_EncodedObject), metisTestDataV0_EncodedObject,  parcBuffer_Overlay(ether0_mac, 0), data->ether_peerAddress[SERVERSIDE], ETHERTYPE);

    mockGenericEther_QueueFrame(data->ether[SERVERSIDE], frame);

    // wake up the server's side of metis to receive a frame
    mockGenericEther_Notify(data->ether[SERVERSIDE]);

    return frame;
}

/*
 * Receives a ContentObject Ethernet frame on the client's side of Metis and verifies it matches what was sent
 */
static void
_receiveContentObject(TestData *data, PARCBuffer *truthFrame)
{
    uint8_t receiveBuffer[1024];

    int serverSocket = mockGenericEther_GetTestDescriptor(data->ether[CLIENTSIDE]);

    ssize_t read_length = read(serverSocket, receiveBuffer, 1024);
    assertTrue(read_length == parcBuffer_Remaining(truthFrame),
               "Incorrect read, expected %zd got %zd: (%d) %s",
               parcBuffer_Remaining(truthFrame), read_length, errno, strerror(errno));

    // skip over the ethernet header
    parcBuffer_SetPosition(truthFrame, sizeof(struct ether_header));

    PARCBuffer *testFrame = parcBuffer_Wrap(receiveBuffer, parcBuffer_Limit(truthFrame), sizeof(struct ether_header), parcBuffer_Limit(truthFrame));
    assertTrue(parcBuffer_Equals(truthFrame, testFrame), "Messages do not match")
    {
        parcBuffer_Display(truthFrame, 3);
        parcBuffer_Display(testFrame, 3);
    }

    parcBuffer_Release(&testFrame);
}

LONGBOW_TEST_CASE(Global, passInterest_Unicast)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);


    PARCBuffer *truthFrame = _sendInterest(data);

    // run for a duration so libevent has time to read the message, pass it off to the handler, then send the message out
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis), &((struct timeval) { 0, 10000 }));

    _receiveInterest(data, truthFrame);

    parcBuffer_Release(&truthFrame);
}

LONGBOW_TEST_CASE(Global, returnContentObject)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // send the Interest so we have a PIT entry

    PARCBuffer *truthInterest = _sendInterest(data);
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis), &((struct timeval) { 0, 10000 }));
    _receiveInterest(data, truthInterest);
    parcBuffer_Release(&truthInterest);

    // send the content object back

    PARCBuffer *truthContentObject = _sendContentObject(data);
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis), &((struct timeval) { 0, 10000 }));
    _receiveContentObject(data, truthContentObject);
    parcBuffer_Release(&truthContentObject);
}


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(sys_EtherEndToEnd);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}

