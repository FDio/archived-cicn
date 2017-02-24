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
// This permits internal static functions to be visible to this Test Runner.
#include "../cpi_ControlMessage.c"

#include <stdio.h>

#include <arpa/inet.h>
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnel.h>

LONGBOW_TEST_RUNNER(cpi_ControlMessage)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_ControlMessage)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_ControlMessage)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateAddRouteRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateAddRouteToSelfRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateCPIRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateCancelFlowRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateConnectionListRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateIPTunnelRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateInterfaceListRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreatePauseInputRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateRemoveRouteRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateRemoveRouteToSelfRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_CreateRouteListRequest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_Display);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_GetAckOriginalSequenceNumber);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_GetJson);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_GetNotifyStatus);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_IsACK);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_IsCPI);
    LONGBOW_RUN_TEST_CASE(Global, ccnxControl_IsNotification);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxControl_AcquireRelease)
{
    CCNxControl *control = ccnxControl_CreateInterfaceListRequest();
    CCNxControl *reference = ccnxControl_Acquire(control);

    ccnxControl_Release(&control);

    assertNull(control, "Expected control to be null");
    assertNotNull(reference, "Expected acquired reference to be non null");
    ccnxControl_Release(&reference);
    assertNull(reference, "Expected reference to be null");
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateAddRouteRequest)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(name);
    CCNxControl *control = ccnxControl_CreateAddRouteRequest(route);

    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_REGISTER_PREFIX,
               "Expected operation %d got %d", CPI_REGISTER_PREFIX, cpi_getCPIOperation2(json));
    ccnxControl_Release(&control);
    ccnxName_Release(&name);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateAddRouteToSelfRequest)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
    CCNxControl *control = ccnxControl_CreateAddRouteToSelfRequest(name);

    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_REGISTER_PREFIX,
               "Expected operation %d got %d", CPI_REGISTER_PREFIX, cpi_getCPIOperation2(json));

    ccnxControl_Release(&control);
    ccnxName_Release(&name);
}


LONGBOW_TEST_CASE(Global, ccnxControl_CreateCancelFlowRequest)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
    CCNxControl *control = ccnxControl_CreateCancelFlowRequest(name);
    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_CANCEL_FLOW,
               "Expected operation %d got %d", CPI_CANCEL_FLOW, cpi_getCPIOperation2(json));

    ccnxControl_Release(&control);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreatePauseInputRequest)
{
    CCNxControl *control = ccnxControl_CreatePauseInputRequest();
    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_PAUSE,
               "Expected operation %d got %d", CPI_PAUSE, cpi_getCPIOperation2(json));
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateConnectionListRequest)
{
    CCNxControl *control = ccnxControl_CreateConnectionListRequest();
    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_CONNECTION_LIST,
               "Expected operation %d got %d", CPI_CONNECTION_LIST, cpi_getCPIOperation2(json));
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateIPTunnelRequest)
{
    struct sockaddr_in sockaddr_any;
    memset(&sockaddr_any, 0, sizeof(sockaddr_any));
    sockaddr_any.sin_family = PF_INET;
    sockaddr_any.sin_addr.s_addr = INADDR_ANY;

    CPIAddress *source = cpiAddress_CreateFromInet(&sockaddr_any);

    struct sockaddr_in sockaddr_dst;
    memset(&sockaddr_dst, 0, sizeof(sockaddr_dst));
    sockaddr_dst.sin_family = PF_INET;
    sockaddr_dst.sin_port = htons(9999);
    inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));

    CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);

    CPIInterfaceIPTunnel *tunnel = cpiInterfaceIPTunnel_Create(0, source, destination, IPTUN_TCP, "tun0");
    CCNxControl *control = ccnxControl_CreateIPTunnelRequest(tunnel);

    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_CREATE_TUNNEL,
               "Expected operation %d got %d", CPI_CREATE_TUNNEL, cpi_getCPIOperation2(json));

    ccnxControl_Release(&control);
    cpiInterfaceIPTunnel_Release(&tunnel);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateInterfaceListRequest)
{
    CCNxControl *control = ccnxControl_CreateInterfaceListRequest();
    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_INTERFACE_LIST,
               "Expected operation %d got %d", CPI_INTERFACE_LIST, cpi_getCPIOperation2(json));
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateRemoveRouteRequest)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(name);
    CCNxControl *control = ccnxControl_CreateRemoveRouteRequest(route);

    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_UNREGISTER_PREFIX,
               "Expected operation %d got %d", CPI_UNREGISTER_PREFIX, cpi_getCPIOperation2(json));

    ccnxControl_Release(&control);
    ccnxName_Release(&name);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateRemoveRouteToSelfRequest)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
    CCNxControl *control = ccnxControl_CreateRemoveRouteToSelfRequest(name);

    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_UNREGISTER_PREFIX,
               "Expected operation %d got %d", CPI_UNREGISTER_PREFIX, cpi_getCPIOperation2(json));

    ccnxControl_Release(&control);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateRouteListRequest)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();
    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    assertTrue(cpi_getCPIOperation2(json) == CPI_PREFIX_REGISTRATION_LIST,
               "Expected operation %d got %d", CPI_PREFIX_REGISTRATION_LIST, cpi_getCPIOperation2(json));

    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, ccnxControl_CreateCPIRequest)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
    PARCJSON *cpiRequest = cpiCancelFlow_CreateRequest(name);

    CCNxControl *control = ccnxControl_CreateCPIRequest(cpiRequest);

    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    parcJSON_Release(&cpiRequest);
    ccnxControl_Release(&control);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxControl_Display)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();
    ccnxControl_Display(control, 4);
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, ccnxControl_GetAckOriginalSequenceNumber)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();
    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    PARCJSON *jsonAck = cpiAcks_CreateAck(json);

    CCNxControl *response = ccnxControl_CreateCPIRequest(jsonAck);

    // Calling GetAckOriginalSequenceNumber() to make sure the path works. We don't care
    // about the value.
    /*uint64_t originalSequenceNumber = */ ccnxControl_GetAckOriginalSequenceNumber(response);

    ccnxControl_Release(&control);
    ccnxControl_Release(&response);
    parcJSON_Release(&jsonAck);
}

LONGBOW_TEST_CASE(Global, ccnxControl_GetJson)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();
    PARCJSON *json = ccnxControl_GetJson(control);
    assertNotNull(json, "Expected some JSON");
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, ccnxControl_IsACK)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();
    assertNotNull(control, "Expected control message to be non null");
    assertTrue(ccnxControl_IsCPI(control), "Expected control to be a CPI control message");

    PARCJSON *json = ccnxControl_GetJson(control);
    PARCJSON *jsonAck = cpiAcks_CreateAck(json);

    CCNxControl *response = ccnxControl_CreateCPIRequest(jsonAck);

    assertTrue(ccnxControl_IsACK(response), "Expected the message to be an Ack");

    ccnxControl_Release(&control);
    ccnxControl_Release(&response);
    parcJSON_Release(&jsonAck);
}

LONGBOW_TEST_CASE(Global, ccnxControl_IsCPI)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();
    assertTrue(ccnxControl_IsCPI(control), "Expected a CPI Message");
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, ccnxControl_IsNotification)
{
    PARCJSON *json = parcJSON_Create();
    CCNxMetaMessage *notification = ccnxControlFacade_CreateNotification(json);
    CCNxControl *control = ccnxMetaMessage_GetControl(notification);

    assertTrue(ccnxControl_IsNotification(control), "Expected a notification");
    assertFalse(ccnxControl_IsCPI(control), "Did not expect a CPI command");

    parcJSON_Release(&json);
    ccnxTlvDictionary_Release(&notification);
}

LONGBOW_TEST_CASE(Global, ccnxControl_GetNotifyStatus)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");

    NotifyStatus *expected = notifyStatus_Create(1, notifyStatusCode_CONNECTION_OPEN, name, "There's a spider behind you.");

    PARCJSON *json = notifyStatus_ToJSON(expected);
    CCNxMetaMessage *notification = ccnxControlFacade_CreateNotification(json);
    CCNxControl *control = ccnxMetaMessage_GetControl(notification);

    assertTrue(ccnxControl_IsNotification(control), "Expected a notification");
    NotifyStatus *status = ccnxControl_GetNotifyStatus(control);

    assertTrue(ccnxName_Equals(notifyStatus_GetName(expected), notifyStatus_GetName(status)), "Expected equal names");
    assertTrue(notifyStatus_GetStatusCode(expected) == notifyStatus_GetStatusCode(status), "Expected equal status codes");
    assertTrue(strcmp(notifyStatus_GetMessage(expected), notifyStatus_GetMessage(status)) == 0, "Expected equal messages");

    parcJSON_Release(&json);
    ccnxTlvDictionary_Release(&notification);
    ccnxName_Release(&name);
    notifyStatus_Release(&status);
    notifyStatus_Release(&expected);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_ControlMessage);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
