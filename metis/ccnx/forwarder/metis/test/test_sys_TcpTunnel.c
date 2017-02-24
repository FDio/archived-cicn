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
 * Run two instances of metis.
 * Client_1 - Metis_A - Metis_B - Client_2
 *
 * Steps
 * 1) run two instances of Metis
 * 2) Create TCP listeners on 127.0.0.1:10001 and 127.0.0.1:10002
 * 3) create a tunnel from A->B.
 * 4) setup route to /foo from a to b
 * 5) Connect client 1 to A
 * 6) Connect client 2 to B
 * 7) Setup route to /foo from metis B to client 2.
 * 8) Sent interest from #1 to #2
 * 9) Send object back from #2 to #1
 *
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
#include <ccnx/forwarder/metis/io/metis_TcpTunnel.h>
#include <ccnx/forwarder/metis/config/metis_Configuration.h>
#include <ccnx/forwarder/metis/config/metis_ConfigurationListeners.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

LONGBOW_TEST_RUNNER(test_sys_TcpTunnel)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_sys_TcpTunnel)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_sys_TcpTunnel)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, tcpTunnel);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

typedef struct notify_receiver {
    MetisMissive *lastMessage;
} NotifyReceiver;

static void
missiveNotify(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    NotifyReceiver *receiver = (NotifyReceiver *) metisMessengerRecipient_GetRecipientContext(recipient);
    if (receiver->lastMessage != NULL) {
        metisMissive_Release(&receiver->lastMessage);
    }
    receiver->lastMessage = missive;
}

LONGBOW_TEST_CASE(Global, tcpTunnel)
{
    uint16_t metisA_port = 10001;
    uint16_t metisB_port = 10002;

    // these will get filled in with the most recent message
    NotifyReceiver receiver_a = { NULL };
    NotifyReceiver receiver_b = { NULL };

    MetisMessengerRecipient *recipient_a = metisMessengerRecipient_Create(&receiver_a, missiveNotify);
    MetisMessengerRecipient *recipient_b = metisMessengerRecipient_Create(&receiver_b, missiveNotify);

    // in between each step, run the dispatchers for 1 msec to let things settle.

    // ===============================================
    /* 1) run two instances of Metis */
    MetisForwarder *metis_a = metisForwarder_Create(NULL);
    MetisForwarder *metis_b = metisForwarder_Create(NULL);

    MetisDispatcher *dispatcher_a = metisForwarder_GetDispatcher(metis_a);
    MetisDispatcher *dispatcher_b = metisForwarder_GetDispatcher(metis_b);

    // register to receive notifications
    metisMessenger_Register(metisForwarder_GetMessenger(metis_a), recipient_a);
    metisMessenger_Register(metisForwarder_GetMessenger(metis_b), recipient_b);

    // ===============================================
    /* 2) Create TCP listeners on 127.0.0.1:10001 and 10002 */

    metisConfigurationListeners_SetupAll(metisForwarder_GetConfiguration(metis_a), metisA_port, NULL);
    metisConfigurationListeners_SetupAll(metisForwarder_GetConfiguration(metis_b), metisB_port, NULL);

    // ---- run
    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));
    // ----

    // ===============================================
    /* 3) create a tunnel from A->B. */

    // connect from any address
    struct sockaddr_in metisA_AnyIpAddress;
    memset(&metisA_AnyIpAddress, 0, sizeof(metisA_AnyIpAddress));
    metisA_AnyIpAddress.sin_family = PF_INET;
    metisA_AnyIpAddress.sin_addr.s_addr = INADDR_ANY;

    // connect to 127.0.0.1:10002
    struct sockaddr_in metisB_LoopbackAddress;
    memset(&metisB_LoopbackAddress, 0, sizeof(metisB_LoopbackAddress));
    metisB_LoopbackAddress.sin_family = PF_INET;
    metisB_LoopbackAddress.sin_port = htons(metisB_port);
    inet_pton(AF_INET, "127.0.0.1", &(metisB_LoopbackAddress.sin_addr));

    CPIAddress *metisA_localCpiAddress = cpiAddress_CreateFromInet(&metisA_AnyIpAddress);
    CPIAddress *metisA_remoteCpiAddress = cpiAddress_CreateFromInet(&metisB_LoopbackAddress);

    MetisIoOperations *ops = metisTcpTunnel_Create(metis_a, metisA_localCpiAddress, metisA_remoteCpiAddress);
    MetisConnection *conn = metisConnection_Create(ops);
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis_a), conn);

    cpiAddress_Destroy(&metisA_localCpiAddress);
    cpiAddress_Destroy(&metisA_remoteCpiAddress);

    // ---- run
    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));
    // ----

    // ===============================================
    /* 4) setup route to /foo from a to b */

    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/2=hello");
    CPIRouteEntry *route = cpiRouteEntry_Create(ccnxName, ops->getConnectionId(ops), NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    bool success = metisForwarder_AddOrUpdateRoute(metis_a, route);
    cpiRouteEntry_Destroy(&route);
    assertTrue(success, "error adding route from A to B");

    // ---- run
    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));
    // ----

    // ===============================================
    /* 5) Connect client 1 to A */

    struct sockaddr_in metisA_LoopbackAddress;
    memset(&metisA_LoopbackAddress, 0, sizeof(metisA_LoopbackAddress));
    metisA_LoopbackAddress.sin_family = PF_INET;
    metisA_LoopbackAddress.sin_port = htons(metisA_port);
    inet_pton(AF_INET, "127.0.0.1", &(metisA_LoopbackAddress.sin_addr));

    int client1_Socket = socket(PF_INET, SOCK_STREAM, 0);
    assertFalse(client1_Socket < 0, "Error creating socket: (%d) %s", errno, strerror(errno));

    int failure = connect(client1_Socket, (struct sockaddr *) &metisA_LoopbackAddress, sizeof(metisA_LoopbackAddress));
    assertFalse(failure, "Error connect: (%d) %s", errno, strerror(errno));

    // ---- run
    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));
    // ----

    // ===============================================
    /* 6) Connect client 2 to B */

    // We need to sniff connections on metis b to learn the connection ID of the client

    int client2_Socket = socket(PF_INET, SOCK_STREAM, 0);
    assertFalse(client2_Socket < 0, "Error creating socket: (%d) %s", errno, strerror(errno));

    failure = connect(client2_Socket, (struct sockaddr *) &metisB_LoopbackAddress, sizeof(metisB_LoopbackAddress));
    assertFalse(failure, "Error connect: (%d) %s", errno, strerror(errno));

    // ---- run
    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));
    // ----

    unsigned client2_ConnectionId = metisMissive_GetConnectionId(receiver_b.lastMessage);
    printf("client 2 connection id is %u\n", client2_ConnectionId);

    // ===============================================
    /* 7) Setup route to /foo from metis B to client 2. */

    ccnxName = ccnxName_CreateFromCString("lci:/2=hello");
    route = cpiRouteEntry_Create(ccnxName, client2_ConnectionId, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    success = metisForwarder_AddOrUpdateRoute(metis_b, route);
    cpiRouteEntry_Destroy(&route);
    assertTrue(success, "error adding route from B to #2");

    // ---- run
    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));
    // ----

    // ===============================================
    /* 8) Sent interest from #1 to #2 */

    ssize_t interest_write_length = write(client1_Socket, metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName));
    assertTrue(interest_write_length == sizeof(metisTestDataV0_InterestWithName),
               "Wrong write length, expected %zu got %zu",
               sizeof(metisTestDataV0_EncodedInterest),
               interest_write_length);

    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));

    // wait to receive it
    uint8_t readBuffer[1024];
    ssize_t interest_read_length = read(client2_Socket, readBuffer, 1024);
    assertTrue(interest_read_length == sizeof(metisTestDataV0_InterestWithName),
               "Wrong write length, expected %zu got %zu",
               sizeof(metisTestDataV0_InterestWithName),
               interest_read_length);

    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));

    // ===============================================
    /* 9) Send object back from #2 to #1 */

    ssize_t object_write_length = write(client2_Socket, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject));
    assertTrue(object_write_length == sizeof(metisTestDataV0_EncodedObject),
               "Wrong write length, expected %zu got %zu",
               sizeof(metisTestDataV0_EncodedInterest),
               object_write_length);

    // very important: run b first, then a
    metisDispatcher_RunDuration(dispatcher_b, &((struct timeval) { 0, 1000 }));
    metisDispatcher_RunDuration(dispatcher_a, &((struct timeval) { 0, 1000 }));

    // wait to receive it
    ssize_t object_read_length = read(client1_Socket, readBuffer, 1024);
    assertTrue(object_read_length == sizeof(metisTestDataV0_EncodedObject),
               "Wrong write length, expected %zu got %zu",
               sizeof(metisTestDataV0_EncodedObject),
               object_read_length);


    // ===============================================
    // cleanup
    metisMissive_Release(&receiver_a.lastMessage);
    metisMissive_Release(&receiver_b.lastMessage);
    metisMessengerRecipient_Destroy(&recipient_a);
    metisMessengerRecipient_Destroy(&recipient_b);
    metisForwarder_Destroy(&metis_b);
    metisForwarder_Destroy(&metis_a);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_sys_TcpTunnel);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
