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
 * These are end-to-end system tests.  They nail up two UDP connections, setup a FIB entry, and send
 * and interest then a content object back.
 */

#include <config.h>
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

#include <ccnx/api/control/cpi_RouteEntry.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/io/metis_UdpListener.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

LONGBOW_TEST_RUNNER(sys_UdpEndToEnd)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(sys_UdpEndToEnd)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(sys_UdpEndToEnd)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ---- Used to monitor Missive messages so we know when
//      a connection is setup
static struct test_notifier_data {
    MetisMissiveType type;
    unsigned connectionid;
} testNotifierData;

static void
testNotifier(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    struct test_notifier_data *data = metisMessengerRecipient_GetRecipientContext(recipient);
    data->type = metisMissive_GetType(missive);
    data->connectionid = metisMissive_GetConnectionId(missive);
    metisMissive_Release(&missive);
}

// ---- Utility functions to setup TCP endpoints

static void
setupInetListener(MetisForwarder *metis, uint16_t port)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));

    MetisListenerSet *listeners = metisForwarder_GetListenerSet(metis);
    MetisListenerOps *ops = metisUdpListener_CreateInet(metis, addr);
    metisListenerSet_Add(listeners, ops);

    // crank the handle
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));
}

static int
setupInetClient(MetisForwarder *metis, uint16_t port)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));

    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    assertFalse(fd < 0, "Error on socket: (%d) %s", errno, strerror(errno));

    int failure = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
    assertFalse(failure, "Error on connect: (%d) %s", errno, strerror(errno));

    // crank the handle
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));
    return fd;
}

// ---- Global state used by tests

static struct global_state {
    MetisForwarder *metis;
    MetisMessengerRecipient *recipient;

    int fd_sender;
    int fd_receiver;
} globalState;

static void
setupUdp(void)
{
    globalState.metis = metisForwarder_Create(NULL);
    globalState.recipient = metisMessengerRecipient_Create(&testNotifierData, testNotifier);

    // register a messenger callback so we know when the connections get setup
    MetisMessenger *messenger = metisForwarder_GetMessenger(globalState.metis);
    metisMessenger_Register(messenger, globalState.recipient);

    setupInetListener(globalState.metis, 49996);
    setupInetListener(globalState.metis, 49997);

    // create two test connections and learn their connection IDs via
    // the messenger callback

    globalState.fd_sender = setupInetClient(globalState.metis, 49996);
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(globalState.metis), &((struct timeval) { 0, 10000 }));

    printf("sender   connection id = %u\n", testNotifierData.connectionid);

    globalState.fd_receiver = setupInetClient(globalState.metis, 49997);
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(globalState.metis), &((struct timeval) { 0, 10000 }));

    // send something to actually connect it
    ssize_t nwritten = write(globalState.fd_receiver, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject));
    assertTrue(nwritten == sizeof(metisTestDataV0_EncodedObject), "Error on write, expected %zu got %zd", sizeof(metisTestDataV0_EncodedObject), nwritten);

    // crank the handle
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(globalState.metis), &((struct timeval) { 0, 10000 }));

    unsigned receiverConnectionId = testNotifierData.connectionid;
    printf("receiver connection id = %u\n", testNotifierData.connectionid);

    // Add a FIB entry out the receiver connection
    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, receiverConnectionId, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    metisForwarder_AddOrUpdateRoute(globalState.metis, routeAdd);
    cpiRouteEntry_Destroy(&routeAdd);
}

static void
teardownUdp(void)
{
    metisForwarder_Destroy(&globalState.metis);
    metisMessengerRecipient_Destroy(&globalState.recipient);

    close(globalState.fd_sender);
    close(globalState.fd_receiver);
}

// ==========================================================

LONGBOW_TEST_FIXTURE(Global)
{
    //XXX: these tests do not work anymore because we do not create the UDP connection from the listener
    //LONGBOW_RUN_TEST_CASE(Global, passInterest);
    //LONGBOW_RUN_TEST_CASE(Global, returnContentObject);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    setupUdp();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    teardownUdp();
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, passInterest)
{
    uint8_t receiveBuffer[1024];

    // now send the interest on the sender and see if we get it on the receiver
    ssize_t write_length = write(globalState.fd_sender, metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName));
    assertTrue(write_length == sizeof(metisTestDataV0_InterestWithName), "Partial write, expected %zd got %zd: (%d) %s", sizeof(metisTestDataV0_InterestWithName), write_length, errno, strerror(errno));

    // run for a duration so there is time to read the message, pass it off to the handler, then send the message out
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(globalState.metis), &((struct timeval) { 0, 10000 }));

    ssize_t read_length = read(globalState.fd_receiver, receiveBuffer, 1024);
    assertTrue(read_length == sizeof(metisTestDataV0_InterestWithName), "Incorrect read, expected %zd got %zd: (%d) %s", sizeof(metisTestDataV0_InterestWithName), read_length, errno, strerror(errno));

    // We decrement the hoplimit, so need a new truth value
    uint8_t truth[1024];
    memcpy(truth, metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName));
    truth[12] = 31;

    assertTrue(memcmp(receiveBuffer, truth, read_length) == 0, "Messages do not match");
}

LONGBOW_TEST_CASE(Global, returnContentObject)
{
    uint8_t receiveBuffer[1024];

    // send the interest on the sender and see if we get it on the receiver
    ssize_t write_length = write(globalState.fd_sender, metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName));
    assertTrue(write_length == sizeof(metisTestDataV0_InterestWithName), "Partial write of interest, expected %zd got %zd: (%d) %s", sizeof(metisTestDataV0_InterestWithName), write_length, errno, strerror(errno));

    // run for a duration so there is time to read the message, pass it off to the handler, then send the message out
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(globalState.metis), &((struct timeval) { 0, 1000 }));

    ssize_t read_length = read(globalState.fd_receiver, receiveBuffer, 1024);
    assertTrue(read_length == sizeof(metisTestDataV0_InterestWithName), "Incorrect read of interest, expected %zd got %zd: (%d) %s", sizeof(metisTestDataV0_InterestWithName), read_length, errno, strerror(errno));

    // send content object back
    write_length = write(globalState.fd_receiver, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject));
    assertTrue(write_length == sizeof(metisTestDataV0_EncodedObject), "Partial write of object, expected %zd got %zd: (%d) %s", sizeof(metisTestDataV0_EncodedObject), write_length, errno, strerror(errno));

    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(globalState.metis), &((struct timeval) { 0, 1000 }));

    // now check that we got the object
    read_length = read(globalState.fd_sender, receiveBuffer, 1024);
    assertTrue(read_length == sizeof(metisTestDataV0_EncodedObject), "Incorrect read of object, expected %zd got %zd: (%d) %s", sizeof(metisTestDataV0_EncodedObject), read_length, errno, strerror(errno));

    assertTrue(memcmp(receiveBuffer, metisTestDataV0_EncodedObject, read_length) == 0, "Objects do not match");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(sys_UdpEndToEnd);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
