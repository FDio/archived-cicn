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



#include "../cpi_ManageLinks.c"
#include <LongBow/testing.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Network.h>
#include <inttypes.h>



static const short testCpiManageLinks_MetisPort = 9695;

LONGBOW_TEST_RUNNER(cpi_ManageLinks)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_ManageLinks)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_ManageLinks)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiLinks_CreateIPTunnel);
    LONGBOW_RUN_TEST_CASE(Global, cpiLinks_CreateInterfaceListRequest);
    LONGBOW_RUN_TEST_CASE(Global, cpiLinks_InterfacesFromControlMessage);
    LONGBOW_RUN_TEST_CASE(Global, cpiLinks_InterfaceIPTunnelFromControlMessage);

    LONGBOW_RUN_TEST_CASE(Global, cpiLinks_CreateConnectionListRequest);
    LONGBOW_RUN_TEST_CASE(Global, cpiLinks_ConnectionListFromControlMessage);
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

LONGBOW_TEST_CASE(Global, cpiLinks_CreateIPTunnel)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform. Note that the port number is encoded in the JSON,
    // so if you change the port the test will fail.
#if defined(__APPLE__)
    char *truth_format = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"CREATE_TUNNEL\":{\"TUNNEL\":{\"IFIDX\":0,\"SYMBOLIC\":\"tun0\",\"TUNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAAAAAAAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIl338AAAEAAAAAAAAAAA==\"}}}}}";
#elif defined(__linux__)
    char *truth_format = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"CREATE_TUNNEL\":{\"TUNNEL\":{\"IFIDX\":0,\"SYMBOLIC\":\"tun0\",\"TUNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAAAAAAAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAl338AAAEAAAAAAAAAAA==\"}}}}}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

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
    sockaddr_dst.sin_port = htons(testCpiManageLinks_MetisPort);
    inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));

    CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);

    // ---------------------------

    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(0, source, destination, IPTUN_TCP, "tun0");
    CCNxControl *control = ccnxControl_CreateIPTunnelRequest(iptun);

    char buffer[1024];
    sprintf(buffer, truth_format, cpi_GetSequenceNumber(control));

    PARCJSON *json = ccnxControl_GetJson(control);
    char *test_string = parcJSON_ToCompactString(json);
    assertTrue(strcmp(buffer, test_string) == 0, "JSON strings did not match.\nexpected %s\ngot %s\n", buffer, test_string);
    parcMemory_Deallocate((void **) &test_string);

    ccnxControl_Release(&control);
    cpiInterfaceIPTunnel_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiLinks_CreateInterfaceListRequest)
{
    char template[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%llu,\"INTERFACE_LIST\":{}}}";
    char truth[1024];

    CCNxControl *control = ccnxControl_CreateInterfaceListRequest();

    uint64_t seqnum = cpi_GetSequenceNumber(control);

    sprintf(truth, template, seqnum);

    PARCJSON *json = ccnxControl_GetJson(control);
    char *str = parcJSON_ToCompactString(json);
    assertTrue(strcmp(truth, str) == 0, "Did not get right json, expected '%s' got '%s'", truth, str);
    parcMemory_Deallocate((void **) &str);

    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, cpiLinks_InterfacesFromControlMessage)
{
    CCNxControl *control = ccnxControl_CreateInterfaceListRequest();

    CPIInterfaceSet *truth = cpiInterfaceSet_Create();
    CPIInterface *iface = cpiInterface_Create("eth0", 11, false, true, 1500);
    cpiInterfaceSet_Add(truth, iface);

    PARCJSON *json = cpiInterfaceSet_ToJson(truth);
    CCNxControl *response = cpi_CreateResponse(control, json);
    parcJSON_Release(&json);
    CPIInterfaceSet *test = cpiLinks_InterfacesFromControlMessage(response);

    assertTrue(cpiInterfaceSet_Equals(test, truth), "Interface sets not equal");

    ccnxControl_Release(&response);
    cpiInterfaceSet_Destroy(&truth);
    cpiInterfaceSet_Destroy(&test);
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, cpiLinks_InterfaceIPTunnelFromControlMessage)
{
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
    sockaddr_dst.sin_port = htons(testCpiManageLinks_MetisPort);
    inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));

    CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);

    // ---------------------------

    CPIInterfaceIPTunnel *truth = cpiInterfaceIPTunnel_Create(0, source, destination, IPTUN_TCP, "tun0");
    CCNxControl *control = ccnxControl_CreateIPTunnelRequest(truth);

    CPIInterfaceIPTunnel *test = cpiLinks_CreateIPTunnelFromControlMessage(control);

    assertTrue(cpiInterfaceIPTunnel_Equals(truth, test), "InterfaceIPTunnels do not match");

    ccnxControl_Release(&control);
    cpiInterfaceIPTunnel_Release(&test);
    cpiInterfaceIPTunnel_Release(&truth);
}

LONGBOW_TEST_CASE(Global, cpiLinks_CreateConnectionListRequest)
{
    char template[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%llu,\"CONNECTION_LIST\":{}}}";
    char truth[1024];

    CCNxControl *control = ccnxControl_CreateConnectionListRequest();
    uint64_t seqnum = cpi_GetSequenceNumber(control);

    sprintf(truth, template, seqnum);

    PARCJSON *json = ccnxControl_GetJson(control);
    char *str = parcJSON_ToCompactString(json);

    assertTrue(strcmp(truth, str) == 0, "Did not get right json, expected '%s' got '%s'", truth, str);
    parcMemory_Deallocate((void **) &str);

    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, cpiLinks_ConnectionListFromControlMessage)
{
    // The request we'll create a response to
    CCNxControl *request = ccnxControl_CreateConnectionListRequest();

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
    sockaddr_dst.sin_port = htons(testCpiManageLinks_MetisPort);
    inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));

    CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);

    // ---------------------------

    CPIConnectionList *truth_list = cpiConnectionList_Create();
    cpiConnectionList_Append(truth_list, cpiConnection_Create(0, source, destination, cpiConnection_TCP));

    PARCJSON *json = cpiConnectionList_ToJson(truth_list);
    CCNxControl *response = cpi_CreateResponse(request, json);
    parcJSON_Release(&json);
    CPIConnectionList *test = cpiLinks_ConnectionListFromControlMessage(response);

    assertTrue(cpiConnectionList_Equals(truth_list, test), "InterfaceIPTunnels do not match");

    ccnxControl_Release(&response);
    ccnxControl_Release(&request);
    cpiConnectionList_Destroy(&test);
    cpiConnectionList_Destroy(&truth_list);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_ManageLinks);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
