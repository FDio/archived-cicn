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

/*
 * Some of these tests might not execute on certain systems, as they
 * depend on having INET and INET6 addresses available.  If you system
 * does not have one or both of those, the corresponding tests will not
 * execute.
 */

// We need to specifically include the Ethernet mockup and set the proper define so
// we do not need an actual Ethernet listener

#define METIS_MOCK_ETHERNET 1
#include "../../io/test/testrig_GenericEther.c"

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_Configuration.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <signal.h>

// so we can mock up an interface
#include "../../core/test/testrig_MetisIoOperations.h"

// so we can test content store size
#include "../../core/metis_Forwarder.c"
#include "../../processor/metis_MessageProcessor.c"
#include "../../content_store/metis_ContentStoreInterface.h"

struct sigaction save_sigchld;
struct sigaction save_sigpipe;

/**
 * Add a connection to the connection table to mock the "ingress" port of a control message
 *
 * You must release the return value
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval non-null A mockup of a connection
 * @retval null An error
 *
 * Example:
 * @code
 * {
 *      unsigned mockConnectionId = 7;
 *      MetisIoOperations *ops = _addIngressMockConnection(metis, mockConnectionId);
 *      MockIoOperationsData *data = ops->context;
 *      mockIoOperationsData_Destroy(&ops);
 * }
 * @endcode
 */
static MetisIoOperations *
_addIngressMockConnection(MetisForwarder *metis, unsigned mockup_id)
{
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, mockup_id, true, true, true);

    MetisConnection *conn = metisConnection_Create(ops);
    MetisConnectionTable *connTable = metisForwarder_GetConnectionTable(metis);
    metisConnectionTable_Add(connTable, conn);
    return ops;
}

// returns a strdup() of the interface name, use free(3)
static char *
_pickInterfaceName(MetisForwarder *metis)
{
    char *ifname = NULL;

    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    size_t length = cpiInterfaceSet_Length(set);
    assertTrue(length > 0, "metisSystem_Interfaces returned no interfaces");

    for (size_t i = 0; i < length; i++) {
        CPIInterface *iface = cpiInterfaceSet_GetByOrdinalIndex(set, i);
        const CPIAddressList *addressList = cpiInterface_GetAddresses(iface);

        size_t length = cpiAddressList_Length(addressList);
        for (size_t i = 0; i < length && !ifname; i++) {
            const CPIAddress *a = cpiAddressList_GetItem(addressList, i);
            if (cpiAddress_GetType(a) == cpiAddressType_LINK) {
                ifname = strdup(cpiInterface_GetName(iface));
            }
        }
    }

    cpiInterfaceSet_Destroy(&set);
    return ifname;
}

/**
 * Adds a mock ethernet connection to the given peer address with a symbolic name.
 * You must have previously added an Ethernet listener.
 *
 * @return true Added
 * @return false An error
 */
static bool
_addEthernetConnection(MetisForwarder *metis, unsigned connid, const char *symbolicName, MetisListenerOps *listener, uint8_t peerEther[6])
{
    // Create a CPIConnectionEthernet Add control message
    char *ifname = _pickInterfaceName(metis);

    uint16_t etherType = 0x0801;

    CPIAddress *peerAddress = cpiAddress_CreateFromLink(peerEther, 6);
    CPIConnectionEthernet *etherConn = cpiConnectionEthernet_Create(ifname, peerAddress, etherType, symbolicName);
    bool success = _metisConfiguration_AddConnectionEthernet(metisForwarder_GetConfiguration(metis), etherConn, peerAddress, listener);


    cpiAddress_Destroy(&peerAddress);
    free(ifname);
    cpiConnectionEthernet_Release(&etherConn);

    return success;
}

// =========================================================================

LONGBOW_TEST_RUNNER(metis_Configuration)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    printf("line 140\n");
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Configuration)
{
    printf("line 148\n");
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Configuration)
{
    printf("line 156\n");
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==============================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisConfiguration_SetupAllListeners);
    LONGBOW_RUN_TEST_CASE(Global, metisConfiguration_Receive);
    LONGBOW_RUN_TEST_CASE(Global, metisConfiguration_SetObjectStoreSize);
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

LONGBOW_TEST_CASE(Global, metisConfiguration_SetupAllListeners)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisConfiguration_Receive)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    // Add a connection to apply the route to
    unsigned mockConnectionId = 7000;
    MetisIoOperations *ops = _addIngressMockConnection(metis, mockConnectionId);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/foo");
    CPIRouteEntry *routeEntry = cpiRouteEntry_Create(prefix, mockConnectionId, NULL,
                                                     cpiNameRouteProtocolType_STATIC,
                                                     cpiNameRouteType_LONGEST_MATCH, NULL, 4);
    CCNxControl *request = ccnxControl_CreateAddRouteRequest(routeEntry);
    cpiRouteEntry_Destroy(&routeEntry);

    PARCBuffer *buffer = metisTlv_EncodeControlPlaneInformation(request);

    MetisMessage *message = metisMessage_CreateFromArray(parcBuffer_Overlay(buffer, 0), parcBuffer_Limit(buffer), mockConnectionId, 2, metisForwarder_GetLogger(metis));
    parcBuffer_Release(&buffer);

    // this takes ownership of message and disposes of it
    metisConfiguration_Receive(metisForwarder_GetConfiguration(metis), message);

    // crank the handle to lets the ACKs or NACKs move
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    assertTrue(data->sendCount == 1, "Did not send a response message, expected 1 got %u", data->sendCount);
    CCNxControl *response = metisMessage_CreateControlMessage(data->lastMessage);

    assertTrue(cpi_GetMessageType(response) == CPI_ACK,
               "CPI message not a response: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    metisForwarder_Destroy(&metis);

    mockIoOperationsData_Destroy(&ops);
}

LONGBOW_TEST_CASE(Global, metisConfiguration_SetObjectStoreSize)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    MetisContentStoreInterface *store = metis->processor->contentStore;
    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);
    size_t current_capacity = metisContentStoreInterface_GetObjectCapacity(store);
    size_t new_capacity = current_capacity + 5;

    metisConfiguration_SetObjectStoreSize(config, new_capacity);

    // Get the store pointer again, as it may have changed.
    store = metis->processor->contentStore;
    assertTrue(new_capacity == metisContentStoreInterface_GetObjectCapacity(store),
               "Object Store is wrong capacity, got %zu expected %zu",
               metisContentStoreInterface_GetObjectCapacity(store), new_capacity);

    metisForwarder_Destroy(&metis);
}

// ==============================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_Receive_AddConnectionEthernet);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_Receive_AddConnectionEthernet_Dup);

    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessUnregisterPrefix);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessRegisterPrefix);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessRegisterPrefix_Symbolic);

    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessInterfaceList);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessRegistrationList);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessCreateTunnel_Dup);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessCreateTunnel_TCP);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessCreateTunnel_UDP);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessConnectionList);

    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessAddConnectionEthernet);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_ProcessRemoveConnectionEthernet);

    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_Receive_AddConnectionEthernet);
    LONGBOW_RUN_TEST_CASE(Local, metisConfiguration_Receive_RemoveConnectionEthernet);
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

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessInterfaceList)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    CCNxControl *request = ccnxControl_CreateInterfaceListRequest();

    unsigned mockConnectionId = 7;

    CCNxControl *response = metisConfiguration_ProcessInterfaceList(metisForwarder_GetConfiguration(metis), request, mockConnectionId);

    assertNotNull(response, "Got null response");

    assertTrue(cpi_GetMessageType(response) == CPI_RESPONSE,
               "CPI message not a response: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    assertTrue(cpi_GetMessageOperation(response) == CPI_INTERFACE_LIST,
               "CPI message not an interface list: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessUnregisterPrefix)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessRegisterPrefix)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    // Add a connection to apply the route to
    unsigned mockConnectionId = 7000;

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/foo");
    CPIRouteEntry *routeEntry = cpiRouteEntry_Create(prefix, mockConnectionId, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 4);
    CCNxControl *request = ccnxControl_CreateAddRouteRequest(routeEntry);
    cpiRouteEntry_Destroy(&routeEntry);

    CCNxControl *response = metisConfiguration_ProcessRegisterPrefix(metisForwarder_GetConfiguration(metis), request, mockConnectionId);

    // crank the handle to lets the ACKs or NACKs move
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    assertNotNull(response, "LastMessage is not set in the test rig");

    assertTrue(cpi_GetMessageType(response) == CPI_ACK,
               "CPI message not a response: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessRegisterPrefix_Symbolic)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    // Add a connection to apply the route to
    unsigned mockConnectionId = 7000;

    // hack in the symbolic name because _addIngressMockConnection does not do that
    metisSymbolicNameTable_Add(metisForwarder_GetConfiguration(metis)->symbolicNameTable, "foo0", mockConnectionId);

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/foo");
    CPIRouteEntry *routeEntry = cpiRouteEntry_CreateSymbolic(prefix, "foo0", cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 4);
    CCNxControl *request = ccnxControl_CreateAddRouteRequest(routeEntry);
    cpiRouteEntry_Destroy(&routeEntry);

    CCNxControl *response = metisConfiguration_ProcessRegisterPrefix(metisForwarder_GetConfiguration(metis), request, mockConnectionId);

    // crank the handle to lets the ACKs or NACKs move
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    assertNotNull(response, "Response is NULL");

    assertTrue(cpi_GetMessageType(response) == CPI_ACK,
               "CPI message not a response: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    metisForwarder_Destroy(&metis);
}

/**
 * Add a route, then verify the route shows up in a list
 */
LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessRegistrationList)
{
    printf("\n%s starting\n", __func__);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 7;

    // Add a route to the forwarding table
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/pancakes/for/all");
    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, 3, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 2);
    metisForwarder_AddOrUpdateRoute(metis, route);
    cpiRouteEntry_Destroy(&route);

    // Create a request and send it in to MetisConfiguration.  The response will be
    // sent out the "mockup_id" interface

    CCNxControl *request = ccnxControl_CreateRouteListRequest();
    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);
    CCNxControl *response = metisConfiguration_ProcessRegistrationList(config, request, mockup_id);

    assertNotNull(response, "Got null response");

    assertTrue(cpi_GetMessageType(response) == CPI_RESPONSE,
               "CPI message not a response: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    assertTrue(cpi_GetMessageOperation(response) == CPI_PREFIX_REGISTRATION_LIST,
               "CPI message not an interface list: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessCreateTunnel_TCP)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 7;

    // -----
    // Issue a command to create a TCP tunnel.  We should be able to verify that it's in
    // the connection table and we'll see the ACK come back to our mock interface.

    // ---------------------------
    // Tunnel addresses
    struct sockaddr_in sockaddr_any;
    memset(&sockaddr_any, 0, sizeof(sockaddr_any));
    sockaddr_any.sin_family = PF_INET;
    sockaddr_any.sin_addr.s_addr = INADDR_ANY;

    CPIAddress *source = cpiAddress_CreateFromInet(&sockaddr_any);

    struct sockaddr_in sockaddr_dst;
    memset(&sockaddr_dst, 0, sizeof(sockaddr_dst));
    sockaddr_dst.sin_family = PF_INET;
    sockaddr_dst.sin_port = htons(PORT_NUMBER);
    inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));

    CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);

    // ---------------------------

    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(0, source, destination, IPTUN_TCP, "tun0");
    CCNxControl *request = ccnxControl_CreateIPTunnelRequest(iptun);

    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);

    CCNxControl *response = metisConfiguration_ProcessCreateTunnel(config, request, mockup_id);

    // crank the handle to lets the ACKs or NACKs move
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    // Validate the ACK
    assertNotNull(response, "Got null response");

    assertTrue(cpi_GetMessageType(response) == CPI_ACK,
               "CPI message not an ACK: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    cpiInterfaceIPTunnel_Release(&iptun);
    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessCreateTunnel_Dup)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 7000;
    MetisIoOperations *ops = _addIngressMockConnection(metis, mockup_id);

    // ---------------------------
    // Tunnel addresses
    struct sockaddr_in sockaddr_any;
    memset(&sockaddr_any, 0, sizeof(sockaddr_any));
    sockaddr_any.sin_family = PF_INET;
    sockaddr_any.sin_addr.s_addr = INADDR_ANY;

    CPIAddress *source = cpiAddress_CreateFromInet(&sockaddr_any);

    struct sockaddr_in sockaddr_dst;
    memset(&sockaddr_dst, 0, sizeof(sockaddr_dst));
    sockaddr_dst.sin_family = PF_INET;
    sockaddr_dst.sin_port = htons(PORT_NUMBER);
    inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));

    CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);

    // ---------------------------

    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(0, source, destination, IPTUN_TCP, "tun0");
    CCNxControl *request = ccnxControl_CreateIPTunnelRequest(iptun);

    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);

    CCNxControl *response_1 = metisConfiguration_ProcessCreateTunnel(config, request, mockup_id);
    assertNotNull(response_1, "got null response");
    assertTrue(ccnxControl_IsACK(response_1), "Did not get ACK response for first tunnel");
    ccnxControl_Release(&response_1);

    CCNxControl *response_2 = metisConfiguration_ProcessCreateTunnel(config, request, mockup_id);
    assertNotNull(response_2, "got null response");
    assertTrue(ccnxControl_IsNACK(response_2), "Did not get NACK response for second tunnel");

    ccnxControl_Release(&response_2);

    ccnxControl_Release(&request);
    cpiInterfaceIPTunnel_Release(&iptun);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);
}

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessCreateTunnel_UDP)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 7;

    // -----
    // Issue a command to create a UDP tunnel.  We should be able to verify that it's in
    // the connection table and we'll see the ACK come back to our mock interface.

    // ---------------------------
    // Tunnel addresses
    struct sockaddr_in sockaddr_any;
    memset(&sockaddr_any, 0, sizeof(sockaddr_any));
    sockaddr_any.sin_family = PF_INET;
    sockaddr_any.sin_addr.s_addr = INADDR_ANY;

    CPIAddress *source = cpiAddress_CreateFromInet(&sockaddr_any);

    struct sockaddr_in sockaddr_dst;
    memset(&sockaddr_dst, 0, sizeof(sockaddr_dst));
    sockaddr_dst.sin_family = PF_INET;
    sockaddr_dst.sin_port = htons(PORT_NUMBER);
    inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));

    CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);

    // ---------------------------

    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(0, source, destination, IPTUN_UDP, "conn0");
    CCNxControl *request = ccnxControl_CreateIPTunnelRequest(iptun);

    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);

    CCNxControl *response = metisConfiguration_ProcessCreateTunnel(config, request, mockup_id);

    // Validate the ACK
    assertNotNull(response, "Got null response");

    assertTrue(cpi_GetMessageType(response) == CPI_ACK,
               "CPI message not an ACK: %s",
               parcJSON_ToString(ccnxControl_GetJson(response)));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    cpiInterfaceIPTunnel_Release(&iptun);
    metisForwarder_Destroy(&metis);
}


LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessConnectionList)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 7;
    MetisIoOperations *ops = _addIngressMockConnection(metis, mockup_id);

    CCNxControl *request = ccnxControl_CreateConnectionListRequest();

    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);
    CCNxControl *response = metisConfiguration_ProcessConnectionList(config, request, mockup_id);

    // Validate the response
    assertNotNull(response, "Got null response");

    // Get the CPI response out of the test mock up
    CPIConnectionList *list = cpiLinks_ConnectionListFromControlMessage(response);
    assertTrue(cpiConnectionList_Length(list) == 1, "Wrong list size, expected %u got %zu", 1, cpiConnectionList_Length(list));

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    cpiConnectionList_Destroy(&list);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);
}


LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessAddConnectionEthernet)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 77;

    // create the listener
    char *ifname = _pickInterfaceName(metis);
    CPIListener *cpiListener = cpiListener_CreateEther(ifname, 0x0801, "fake0");
    CCNxControl *control = cpiListener_CreateAddMessage(cpiListener);
    bool listenerOk = metisConfigurationListeners_Add(metisForwarder_GetConfiguration(metis), control, mockup_id);
    assertTrue(listenerOk, "Failed to setup ether listener.");

    ccnxControl_Release(&control);
    cpiListener_Release(&cpiListener);

    // ========
    uint8_t peerEther[6] = { 0x02, 0x33, 0x44, 0x55, 0x66, 0x77 };
    CPIAddress *peerAddress = cpiAddress_CreateFromLink(peerEther, sizeof(peerEther));
    CPIConnectionEthernet *etherconn = cpiConnectionEthernet_Create(ifname, peerAddress, 0x0801, "conn3");
    CCNxControl *addRequest = cpiConnectionEthernet_CreateAddMessage(etherconn);

    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);

    CCNxControl *response = metisConfiguration_ProcessAddConnectionEthernet(config, addRequest, mockup_id);

    // crank the handle to lets the ACKs or NACKs move
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    // Get the CPI response out of the test mock up
    assertNotNull(response, "Got null response");

    assertTrue(ccnxControl_IsACK(response), "Response is not an ACK")
    {
        ccnxControl_Display(response, 3);
    }

    // we must manually destroy a Mock connection
    ccnxControl_Release(&response);
    free(ifname);
    cpiConnectionEthernet_Release(&etherconn);
    cpiAddress_Destroy(&peerAddress);
    ccnxControl_Release(&addRequest);
    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Local, metisConfiguration_ProcessRemoveConnectionEthernet)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, metisConfiguration_Receive_AddConnectionEthernet)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 7;
    MetisIoOperations *ops = _addIngressMockConnection(metis, mockup_id);

    // create the listener
    char *ifname = _pickInterfaceName(metis);
    CPIListener *cpiListener = cpiListener_CreateEther(ifname, 0x0801, "fake0");
    CCNxControl *control = cpiListener_CreateAddMessage(cpiListener);
    bool listenerOk = metisConfigurationListeners_Add(metisForwarder_GetConfiguration(metis), control, mockup_id);
    assertTrue(listenerOk, "Failed to setup ether listener.");

    ccnxControl_Release(&control);
    cpiListener_Release(&cpiListener);

    // create the connection
    uint8_t linkAddrArray[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
    uint16_t etherType = 0x0801;

    CPIAddress *peerAddress = cpiAddress_CreateFromLink(linkAddrArray, sizeof(linkAddrArray));
    CPIConnectionEthernet *etherconn = cpiConnectionEthernet_Create(ifname, peerAddress, etherType, "conn3");
    CCNxControl *addRequest = cpiConnectionEthernet_CreateAddMessage(etherconn);

    // Translate the control message to a MetisMessage
    PARCBuffer *buffer = metisTlv_EncodeControlPlaneInformation(addRequest);

    MetisMessage *message = metisMessage_CreateFromArray(parcBuffer_Overlay(buffer, 0), parcBuffer_Limit(buffer), mockup_id, 2, metisForwarder_GetLogger(metis));

    MetisConfiguration *config = metisForwarder_GetConfiguration(metis);

    // this will release the message
    metisConfiguration_Receive(config, message);

    // ==== Verify it's in the connection table

    MetisConnectionList *connList = metisConnectionTable_GetEntries(metisForwarder_GetConnectionTable(metis));
    assertNotNull(connList, "Got null connection list");

    bool found = false;
    for (size_t i = 0; i < metisConnectionList_Length(connList) && !found; i++) {
        MetisConnection *conn = metisConnectionList_Get(connList, i);
        const MetisAddressPair *pair = metisConnection_GetAddressPair(conn);
        const CPIAddress *remote = metisAddressPair_GetRemote(pair);
        if (cpiAddress_Equals(remote, peerAddress)) {
            found = true;
        }
    }

    assertTrue(found, "Could not find peer address in the connection table as a remote");

    // ==== Cleanup

    parcBuffer_Release(&buffer);
    ccnxControl_Release(&addRequest);
    cpiConnectionEthernet_Release(&etherconn);
    cpiAddress_Destroy(&peerAddress);
    free(ifname);
    metisConnectionList_Destroy(&connList);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);
}

/*
 * Try to add a second connection with same symbolic name
 */
LONGBOW_TEST_CASE(Local, metisConfiguration_Receive_AddConnectionEthernet_Dup)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    uint8_t peerEther[6] = { 7, 8, 9, 10, 11, 12 };

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 7000;
    MetisIoOperations *ops = _addIngressMockConnection(metis, mockup_id);

    char *ifname = _pickInterfaceName(metis);
    CPIListener *cpiListener = cpiListener_CreateEther(ifname, 0x0801, "fake0");
    CCNxControl *control = cpiListener_CreateAddMessage(cpiListener);
    metisConfigurationListeners_Add(metisForwarder_GetConfiguration(metis), control, mockup_id);
    ccnxControl_Release(&control);
    cpiListener_Release(&cpiListener);
    free(ifname);

    MetisListenerOps *listener = metisListenerSet_Get(metisForwarder_GetListenerSet(metis), 0);

    // Create a mock up of an interface so we can see the response
    bool success = _addEthernetConnection(metis, 1000, "conn3", listener, peerEther);
    assertTrue(success, "Failed to add first instance of connection");

    // now add again, should fail
    bool failure = _addEthernetConnection(metis, 1001, "conn3", listener, peerEther);
    assertFalse(failure, "Should have failed to add it a second time");

    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);
}

LONGBOW_TEST_CASE(Local, metisConfiguration_Receive_RemoveConnectionEthernet)
{
}

// ======================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Configuration);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
