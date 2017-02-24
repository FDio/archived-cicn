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

#include <config.h>

#include <LongBow/runtime.h>

#include <ccnx/api/control/cpi_ControlMessage.h>

#include <ccnx/api/control/cpi_ControlFacade.h>

#include <ccnx/api/notify/notify_Status.h>

#include <ccnx/api/control/controlPlaneInterface.h>

PARCJSON *
ccnxControl_GetJson(const CCNxControl *control)
{
    return ccnxControlFacade_GetJson(control);
}

void
ccnxControl_Display(const CCNxControl *control, int indentation)
{
    ccnxControlFacade_Display(control, indentation);
}

void
ccnxControl_Release(CCNxControl **controlP)
{
    ccnxTlvDictionary_Release(controlP);
}

CCNxControl *
ccnxControl_Acquire(const CCNxControl *control)
{
    return ccnxTlvDictionary_Acquire(control);
}

bool
ccnxControl_IsACK(const CCNxControl *control)
{
    if (cpi_GetMessageType(control) == CPI_ACK) {
        PARCJSON *json = ccnxControlFacade_GetJson(control);
        return cpiAcks_IsAck(json);
    }
    return false;
}

bool
ccnxControl_IsNACK(const CCNxControl *control)
{
    if (cpi_GetMessageType(control) == CPI_ACK) {
        PARCJSON *json = ccnxControlFacade_GetJson(control);
        return !cpiAcks_IsAck(json);
    }
    return false;
}

uint64_t
ccnxControl_GetAckOriginalSequenceNumber(const CCNxControl *control)
{
    PARCJSON *json = ccnxControlFacade_GetJson(control);
    return cpiAcks_GetAckOriginalSequenceNumber(json);
}

bool
ccnxControl_IsNotification(const CCNxControl *control)
{
    return ccnxControlFacade_IsNotification(control);
}

NotifyStatus *
ccnxControl_GetNotifyStatus(const CCNxControl *control)
{
    return notifyStatus_ParseJSON(ccnxControl_GetJson(control));
}

CCNxControl *
ccnxControl_CreateCPIRequest(PARCJSON *json)
{
    return ccnxControlFacade_CreateCPI(json);
}

CCNxControl *
ccnxControl_CreateAddRouteRequest(const CPIRouteEntry *route)
{
    PARCJSON *cpiRequest = cpiForwarding_CreateAddRouteRequest(route);
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateRemoveRouteRequest(const CPIRouteEntry *route)
{
    PARCJSON *cpiRequest = cpiForwarding_CreateRemoveRouteRequest(route);
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateSetStrategyRequest(const CPIForwardingStrategy *fwdStrategy)
{
    PARCJSON *cpiRequest = cpiForwarding_CreateSetStrategyRequest(fwdStrategy);
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateSetWldrRequest(const CPIManageWldr *cpiWldr)
{
    PARCJSON *cpiRequest = cpiLinks_CreateSetWldrRequest(cpiWldr);
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateRouteListRequest()
{
    PARCJSON *cpiRequest = cpiForwarding_CreateRouteListRequest();
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateConnectionListRequest()
{
    PARCJSON *cpiRequest = cpiLinks_CreateConnectionListRequest();
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateInterfaceListRequest()
{
    PARCJSON *cpiRequest = cpiLinks_CreateInterfaceListRequest();
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateAddRouteToSelfRequest(const CCNxName *name)
{
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(name);
    CCNxControl *result = ccnxControl_CreateAddRouteRequest(route);
    cpiRouteEntry_Destroy(&route);
    return result;
}

CCNxControl *
ccnxControl_CreateRemoveRouteToSelfRequest(const CCNxName *name)
{
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(name);
    CCNxControl *result = ccnxControl_CreateRemoveRouteRequest(route);
    cpiRouteEntry_Destroy(&route);
    return result;
}

CCNxControl *
ccnxControl_CreatePauseInputRequest()
{
    PARCJSON *cpiRequest = cpi_CreatePauseInputRequest();
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateFlushRequest(void)
{
    PARCJSON *cpiRequest = cpi_CreateFlushRequest();
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

bool
ccnxControl_IsCPI(const CCNxControl *controlMsg)
{
    return ccnxControlFacade_IsCPI((CCNxTlvDictionary *) controlMsg);
}

CCNxControl *
ccnxControl_CreateIPTunnelRequest(const CPIInterfaceIPTunnel *tunnel)
{
    PARCJSON *request = cpiLinks_CreateIPTunnel(tunnel);
    CCNxControl *result = ccnxControl_CreateCPIRequest(request);
    parcJSON_Release(&request);
    return result;
}

CCNxControl *
ccnxControl_CreateCancelFlowRequest(const CCNxName *name)
{
    PARCJSON *request = cpiCancelFlow_CreateRequest(name);
    CCNxControl *result = ccnxControl_CreateCPIRequest(request);
    parcJSON_Release(&request);
    return result;
}

CCNxControl *
ccnxControl_CreateCacheStoreRequest(bool activate)
{
    PARCJSON *cpiRequest = cpiManageChaces_CreateCacheStoreRequest(activate);
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateCacheServeRequest(bool activate)
{
    PARCJSON *cpiRequest = cpiManageChaces_CreateCacheServeRequest(activate);
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}

CCNxControl *
ccnxControl_CreateCacheClearRequest()
{
    PARCJSON *cpiRequest = cpiManageChaces_CreateCacheClearRequest();
    CCNxControl *result = ccnxControl_CreateCPIRequest(cpiRequest);
    parcJSON_Release(&cpiRequest);
    return result;
}
