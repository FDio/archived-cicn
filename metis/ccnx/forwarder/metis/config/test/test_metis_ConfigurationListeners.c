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
#include "../metis_ConfigurationListeners.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <signal.h>
#include <net/ethernet.h>

#define TEST_PORT 9697
static const CPIAddress *
getFirstAddressOfType(CPIInterfaceSet *set, CPIAddressType type)
{
    for (int i = 0; i < cpiInterfaceSet_Length(set); i++) {
        CPIInterface *iface = cpiInterfaceSet_GetByOrdinalIndex(set, i);
        const CPIAddressList *list = cpiInterface_GetAddresses(iface);
        for (int j = 0; j < cpiAddressList_Length(list); j++) {
            const CPIAddress *address = cpiAddressList_GetItem(list, j);
            if (cpiAddress_GetType(address) == type) {
                return address;
            }
        }
    }
    return NULL;
}

struct sigaction save_sigchld;
struct sigaction save_sigpipe;

static void
blockSigChild()
{
    struct sigaction ignore_action;
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    ignore_action.sa_flags = 0;

    sigaction(SIGCHLD, NULL, &save_sigchld);
    sigaction(SIGPIPE, NULL, &save_sigpipe);

    sigaction(SIGCHLD, &ignore_action, NULL);
    sigaction(SIGPIPE, &ignore_action, NULL);
}

static void
unblockSigChild()
{
    sigaction(SIGCHLD, &save_sigchld, NULL);
    sigaction(SIGPIPE, &save_sigpipe, NULL);
}

static bool
verifyInNetstat(const char *addressString, int port)
{
    // now verify that we are listening
    // tcp4       0      0  127.0.0.1.49009        *.*                    LISTEN

    FILE *fp = popen("netstat -an", "r");
    assertNotNull(fp, "Got null opening netstat for reading");

    char buffer[4][1024];

    sprintf(buffer[0], "%s.%d", addressString, port);
    sprintf(buffer[1], "%s:%d", addressString, port);
    sprintf(buffer[2], "%s%%lo0.%d", addressString, port);
    sprintf(buffer[3], "%s%%lo0:%d", addressString, port);

    char str[1035];
    bool found = false;
    while (!found && (fgets(str, sizeof(str) - 1, fp) != NULL)) {
        for (int i = 0; i < 4; i++) {
            if (strstr(str, buffer[i]) != NULL) {
                found = true;
            }
        }
    }

    blockSigChild();
    pclose(fp);
    unblockSigChild();

    return found;
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

// =========================================================================

LONGBOW_TEST_RUNNER(metis_Configuration)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Configuration)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Configuration)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==============================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisConfigurationListeners_SetupAll);
    LONGBOW_RUN_TEST_CASE(Global, metisConfigurationListeners_Add_Ether);
    LONGBOW_RUN_TEST_CASE(Global, metisConfigurationListeners_Add_IP_UDP4);
    LONGBOW_RUN_TEST_CASE(Global, metisConfigurationListeners_Add_IP_UDP6);
    LONGBOW_RUN_TEST_CASE(Global, metisConfigurationListeners_Add_IP_TCP4);
    LONGBOW_RUN_TEST_CASE(Global, metisConfigurationListeners_Add_IP_TCP6);
    LONGBOW_RUN_TEST_CASE(Global, metisConfigurationListeners_Remove);
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

LONGBOW_TEST_CASE(Global, metisConfigurationListeners_SetupAll)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    metisConfigurationListeners_SetupAll(metisForwarder_GetConfiguration(metis), TEST_PORT, NULL);

    MetisListenerSet *set = metisForwarder_GetListenerSet(metis);
    size_t len = metisListenerSet_Length(set);
    assertTrue(len > 0, "Bad listener set size, expected positive, got %zu", len);

    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Global, metisConfigurationListeners_Add_Ether)
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
    free(ifname);

    ccnxControl_Release(&control);
    cpiListener_Release(&cpiListener);

    MetisListenerSet *set = metisForwarder_GetListenerSet(metis);
    size_t len = metisListenerSet_Length(set);
    assertTrue(len == 1, "Bad listener set size, expected 1, got %zu", len);

    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Global, metisConfigurationListeners_Add_IP_UDP4)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 77;

    // create the listener
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(TEST_PORT);
    int result = inet_aton("127.0.0.1", &sin.sin_addr);
    assertTrue(result == 1, "failed inet_aton: (%d) %s", errno, strerror(errno));

    CPIAddress *address = cpiAddress_CreateFromInet(&sin);
    CPIListener *listener = cpiListener_CreateIP(IPTUN_UDP, address, "conn1");
    cpiAddress_Destroy(&address);


    CCNxControl *control = cpiListener_CreateAddMessage(listener);
    bool listenerOk = metisConfigurationListeners_Add(metisForwarder_GetConfiguration(metis), control, mockup_id);
    assertTrue(listenerOk, "Failed to setup ether listener.") {
        int res;
        res = system("netstat -an -p udp");
        assertTrue(res != -1, "Error on system call");
        res = system("ps -el");
        assertTrue(res != -1, "Error on system call");
    }

    ccnxControl_Release(&control);
    cpiListener_Release(&listener);

    MetisListenerSet *set = metisForwarder_GetListenerSet(metis);
    size_t len = metisListenerSet_Length(set);
    assertTrue(len == 1, "Bad listener set size, expected 1, got %zu", len);

    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Global, metisConfigurationListeners_Add_IP_UDP6)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 77;

    // create the listener
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(TEST_PORT);
    int result = inet_pton(AF_INET6, "::1", &(sin6.sin6_addr));
    if (result == 1) {
        CPIAddress *address = cpiAddress_CreateFromInet6(&sin6);
        CPIListener *listener = cpiListener_CreateIP(IPTUN_UDP, address, "conn1");
        cpiAddress_Destroy(&address);


        CCNxControl *control = cpiListener_CreateAddMessage(listener);
        bool listenerOk = metisConfigurationListeners_Add(metisForwarder_GetConfiguration(metis), control, mockup_id);
        assertTrue(listenerOk, "Failed to setup ether listener.") {
            int res;
            res = system("netstat -an -p udp");
            assertTrue(res != -1, "Error on system call");
            res = system("ps -el");
            assertTrue(res != -1, "Error on system call");
        }

        ccnxControl_Release(&control);
        cpiListener_Release(&listener);

        MetisListenerSet *set = metisForwarder_GetListenerSet(metis);
        size_t len = metisListenerSet_Length(set);
        assertTrue(len == 1, "Bad listener set size, expected 1, got %zu", len);
        metisForwarder_Destroy(&metis);
    } else {
        metisForwarder_Destroy(&metis);
        testSkip("IPv6 not supported");
    }
}

LONGBOW_TEST_CASE(Global, metisConfigurationListeners_Add_IP_TCP4)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 77;

    // create the listener
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(TEST_PORT);
    int result = inet_aton("127.0.0.1", &sin.sin_addr);
    assertTrue(result == 1, "failed inet_aton: (%d) %s", errno, strerror(errno));

    CPIAddress *address = cpiAddress_CreateFromInet(&sin);
    CPIListener *listener = cpiListener_CreateIP(IPTUN_TCP, address, "conn1");
    cpiAddress_Destroy(&address);


    CCNxControl *control = cpiListener_CreateAddMessage(listener);
    bool listenerOk = metisConfigurationListeners_Add(metisForwarder_GetConfiguration(metis), control, mockup_id);
    assertTrue(listenerOk, "Failed to setup ether listener.") {
        int res;
        res = system("netstat -an -p tcp");
        assertTrue(res != -1, "Error on system call");
        res = system("ps -el");
        assertTrue(res != -1, "Error on system call");
    }

    ccnxControl_Release(&control);
    cpiListener_Release(&listener);

    MetisListenerSet *set = metisForwarder_GetListenerSet(metis);
    size_t len = metisListenerSet_Length(set);
    assertTrue(len == 1, "Bad listener set size, expected 1, got %zu", len);

    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Global, metisConfigurationListeners_Add_IP_TCP6)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    // Create a mock up of an interface so we can see the response
    unsigned mockup_id = 77;

    // create the listener
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(TEST_PORT);
    int result = inet_pton(AF_INET6, "::1", &(sin6.sin6_addr));
    if (result == 1) {
        CPIAddress *address = cpiAddress_CreateFromInet6(&sin6);
        CPIListener *listener = cpiListener_CreateIP(IPTUN_TCP, address, "conn1");
        cpiAddress_Destroy(&address);


        CCNxControl *control = cpiListener_CreateAddMessage(listener);
        bool listenerOk = metisConfigurationListeners_Add(metisForwarder_GetConfiguration(metis), control, mockup_id);
        assertTrue(listenerOk, "Failed to setup ether listener.") {
            int res;
            res = system("netstat -an -p tcp");
            assertTrue(res != -1, "Error on system call");
            res = system("ps -el");
            assertTrue(res != -1, "Error on system call");
        }

        ccnxControl_Release(&control);
        cpiListener_Release(&listener);

        MetisListenerSet *set = metisForwarder_GetListenerSet(metis);
        size_t len = metisListenerSet_Length(set);
        assertTrue(len == 1, "Bad listener set size, expected 1, got %zu", len);
        metisForwarder_Destroy(&metis);
    } else {
        metisForwarder_Destroy(&metis);
        testSkip("IPv6 not supported");
    }
}

LONGBOW_TEST_CASE(Global, metisConfigurationListeners_Remove)
{
    testUnimplemented("This test is unimplemented");
}

// ==============================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, setupEthernetListenerOnLink);
    LONGBOW_RUN_TEST_CASE(Local, setupEthernetListenerOnLink_SecondEthertype);
    LONGBOW_RUN_TEST_CASE(Local, setupIPMulticastListenerOnInet);
    LONGBOW_RUN_TEST_CASE(Local, setupListenersOnAddress);
    LONGBOW_RUN_TEST_CASE(Local, setupListenersOnInet);
    LONGBOW_RUN_TEST_CASE(Local, setupListenersOnInet6);
    LONGBOW_RUN_TEST_CASE(Local, setupListenersOnLink);
    LONGBOW_RUN_TEST_CASE(Local, setupLocalListener);
    LONGBOW_RUN_TEST_CASE(Local, setupTcpListenerOnInet);
    LONGBOW_RUN_TEST_CASE(Local, setupTcpListenerOnInet6);
    LONGBOW_RUN_TEST_CASE(Local, setupUdpListenerOnInet);
    LONGBOW_RUN_TEST_CASE(Local, setupUdpListenerOnInet6);
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

LONGBOW_TEST_CASE(Local, setupEthernetListenerOnLink)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    uint8_t addr[ETHER_ADDR_LEN] = { 1, 2, 3, 4, 5, 6 };
    CPIAddress *localAddress = cpiAddress_CreateFromLink(addr, ETHER_ADDR_LEN);

    char *ifname = _pickInterfaceName(metis);
    MetisListenerOps *listenerops = _setupEthernetListenerOnLink(metis, localAddress, ifname, 0x0801);
    assertNotNull(listenerops, "Got null result from _setupEthernetListenerOnLink");

    free(ifname);
    cpiAddress_Destroy(&localAddress);
    metisForwarder_Destroy(&metis);
}

/*
 * The current system does not allow multiple ether listeners on a single interface.
 * even if they are different ethertypes
 */
LONGBOW_TEST_CASE(Local, setupEthernetListenerOnLink_SecondEthertype)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    uint8_t addr[ETHER_ADDR_LEN] = { 1, 2, 3, 4, 5, 6 };
    CPIAddress *localAddress = cpiAddress_CreateFromLink(addr, ETHER_ADDR_LEN);

    char *ifname = _pickInterfaceName(metis);
    MetisListenerOps *listenerops = _setupEthernetListenerOnLink(metis, localAddress, ifname, 0x0801);
    assertNotNull(listenerops, "Got null result from _setupEthernetListenerOnLink");

    // now try to add again with different ethertype
    MetisListenerOps *second = _setupEthernetListenerOnLink(metis, localAddress, ifname, 0x0802);
    assertNull(second, "Should have gotten null for second listener");

    free(ifname);
    cpiAddress_Destroy(&localAddress);
    metisForwarder_Destroy(&metis);
}


LONGBOW_TEST_CASE(Local, setupIPMulticastListenerOnInet)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, setupListenersOnAddress)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, setupListenersOnInet)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, setupListenersOnInet6)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, setupListenersOnLink)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, setupLocalListener)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, setupTcpListenerOnInet)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    const CPIAddress *address = getFirstAddressOfType(set, cpiAddressType_INET);
    if (address != NULL) {
        char valueToFind[1024];
        struct sockaddr_in sin;
        cpiAddress_GetInet(address, &sin);
        _setupTcpListenerOnInet(metis, address, PORT_NUMBER);
        bool found = verifyInNetstat(inet_ntoa(sin.sin_addr), PORT_NUMBER);
        if (!found) {
            // extra diagnostics
            int ret = system("netstat -an -p tcp");
            assertTrue(ret > -1, "Error on system call");
        }
        assertTrue(found, "Did not find value %s in netstat", valueToFind);
    }

    cpiInterfaceSet_Destroy(&set);
    metisForwarder_Destroy(&metis);

    if (address == NULL) {
        testSkip("No network interfaces of type INET found");
    }
}

LONGBOW_TEST_CASE(Local, setupTcpListenerOnInet6)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    const CPIAddress *address = getFirstAddressOfType(set, cpiAddressType_INET6);
    if (address != NULL) {
        char valueToFind[1024];
        char inet6str[INET6_ADDRSTRLEN];
        struct sockaddr_in6 sin6;
        cpiAddress_GetInet6(address, &sin6);
        inet_ntop(AF_INET6, &sin6.sin6_addr, inet6str, INET6_ADDRSTRLEN);
        _setupTcpListenerOnInet6(metis, address, PORT_NUMBER);
        bool found = verifyInNetstat(inet6str, PORT_NUMBER);
        if (!found) {
            // extra diagnostics
            int ret = system("netstat -an -p tcp");
            assertTrue(ret > -1, "Error on system call");
        }
        assertTrue(found, "Did not find value %s in netstat", valueToFind);
    }

    cpiInterfaceSet_Destroy(&set);
    metisForwarder_Destroy(&metis);

    if (address == NULL) {
        testSkip("No network interfaces of type INET found");
    }
}

LONGBOW_TEST_CASE(Local, setupUdpListenerOnInet)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    const CPIAddress *address = getFirstAddressOfType(set, cpiAddressType_INET);
    if (address != NULL) {
        char valueToFind[1024];
        struct sockaddr_in sin;
        cpiAddress_GetInet(address, &sin);
        _setupUdpListenerOnInet(metis, address, PORT_NUMBER);
        bool found = verifyInNetstat(inet_ntoa(sin.sin_addr), PORT_NUMBER);
        if (!found) {
            // extra diagnostics
            int ret = system("netstat -an -p udp");
            assertTrue(ret > -1, "Error on system call");
        }
        assertTrue(found, "Did not find value %s in netstat", valueToFind);
    }

    cpiInterfaceSet_Destroy(&set);
    metisForwarder_Destroy(&metis);

    if (address == NULL) {
        testSkip("No network interfaces of type INET found");
    }
}

LONGBOW_TEST_CASE(Local, setupUdpListenerOnInet6)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    const CPIAddress *address = getFirstAddressOfType(set, cpiAddressType_INET6);
    if (address != NULL) {
        char valueToFind[1024];
        char inet6str[INET6_ADDRSTRLEN];
        struct sockaddr_in6 sin6;
        cpiAddress_GetInet6(address, &sin6);
        inet_ntop(AF_INET6, &sin6.sin6_addr, inet6str, INET6_ADDRSTRLEN);
        _setupUdpListenerOnInet6(metis, address, PORT_NUMBER);
        bool found = verifyInNetstat(inet6str, PORT_NUMBER);
        if (!found) {
            // extra diagnostics
            int ret = system("netstat -an -p udp");
            assertTrue(ret > -1, "Error on system call");
        }
        assertTrue(found, "Did not find value %s in netstat", valueToFind);
    }

    cpiInterfaceSet_Destroy(&set);
    metisForwarder_Destroy(&metis);

    if (address == NULL) {
        testSkip("No network interfaces of type INET found");
    }
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
