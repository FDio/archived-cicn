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
#include <stdio.h>

#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnelList.h>
#include <ccnx/api/control/cpi_ManageLinks.h>
#include <LongBow/runtime.h>

#include "cpi_private.h"

static const char *cpiInterfaceList = "INTERFACE_LIST";
static const char *cpiCreateTunnel = "CREATE_TUNNEL";
static const char *cpiRemoveTunnel = "REMOVE_TUNNEL";
static const char *cpiConnectionList = "CONNECTION_LIST";
static const char *cpiSetWldr = "SET_WLDR";

PARCJSON *
cpiLinks_CreateInterfaceListRequest(void)
{
    PARCJSON *json = parcJSON_Create();
    PARCJSON *result = cpi_CreateRequest(cpiInterfaceList, json);
    parcJSON_Release(&json);

    return result;
}

bool cpiLinks_IsInterfaceListResponse(CCNxControl *control);

CPIInterfaceSet *
cpiLinks_InterfacesFromControlMessage(CCNxControl *response)
{
    PARCJSON *json = ccnxControl_GetJson(response);
    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiResponse_GetJsonTag());
    PARCJSON *inner_json = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(inner_json, cpiLinks_InterfaceListJsonTag());
    PARCJSON *operation = parcJSONValue_GetJSON(value);

    return cpiInterfaceSet_FromJson(operation);
}

CPIInterfaceIPTunnel *
cpiLinks_CreateIPTunnelFromControlMessage(CCNxControl *response)
{
    PARCJSON *json = ccnxControl_GetJson(response);
    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiRequest_GetJsonTag());
    if (value == NULL) {
        value = parcJSON_GetValueByName(json, cpiResponse_GetJsonTag());
    }
    PARCJSON *inner_json = parcJSONValue_GetJSON(value);
    value = parcJSON_GetValueByName(inner_json, cpiLinks_CreateTunnelJsonTag());
    if (value == NULL) {
        value = parcJSON_GetValueByName(inner_json, cpiLinks_RemoveTunnelJsonTag());
    }
    PARCJSON *operation = parcJSONValue_GetJSON(value);
    return cpiInterfaceIPTunnel_CreateFromJson(operation);
}

PARCJSON *
cpiLinks_CreateConnectionListRequest()
{
    PARCJSON *json = parcJSON_Create();
    PARCJSON *result = cpi_CreateRequest(cpiConnectionList, json);
    parcJSON_Release(&json);

    return result;
}


CPIConnectionList *
cpiLinks_ConnectionListFromControlMessage(CCNxControl *response)
{
    PARCJSON *json = ccnxControl_GetJson(response);
    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiRequest_GetJsonTag());
    if (value == NULL) {
        value = parcJSON_GetValueByName(json, cpiResponse_GetJsonTag());
    }
    PARCJSON *inner_json = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(inner_json, cpiLinks_ConnectionListJsonTag());
    PARCJSON *operation = parcJSONValue_GetJSON(value);
    return cpiConnectionList_FromJson(operation);
}

PARCJSON *
cpiLinks_CreateIPTunnel(const CPIInterfaceIPTunnel *iptun)
{
    PARCJSON *tunnelJson = cpiInterfaceIPTunnel_ToJson(iptun);
    PARCJSON *result = cpi_CreateRequest(cpiCreateTunnel, tunnelJson);
    parcJSON_Release(&tunnelJson);

    return result;
}

PARCJSON *
cpiLinks_RemoveIPTunnel(const CPIInterfaceIPTunnel *iptun)
{
    PARCJSON *tunnelJson = cpiInterfaceIPTunnel_ToJson(iptun);
    PARCJSON *result = cpi_CreateRequest(cpiRemoveTunnel, tunnelJson);
    parcJSON_Release(&tunnelJson);

    return result;
}

CCNxControl *
cpiLinks_SetInterfaceState(unsigned ifidx, CPIInterfaceStateType state)
{
    return NULL;
}

CCNxControl *
cpiLinks_RemoveInterface(unsigned ifidx)
{
    return NULL;
}

const char *
cpiLinks_InterfaceListJsonTag()
{
    return cpiInterfaceList;
}

const char *
cpiLinks_CreateTunnelJsonTag()
{
    return cpiCreateTunnel;
}

const char *
cpiLinks_RemoveTunnelJsonTag()
{
    return cpiRemoveTunnel;
}

const char *
cpiLinks_ConnectionListJsonTag()
{
    return cpiConnectionList;
}

PARCJSON *
cpiLinks_CreateSetWldrRequest(const CPIManageWldr *cpiWldr)
{
    PARCJSON *json = cpiManageWldr_ToJson(cpiWldr);
    PARCJSON *result = cpi_CreateRequest(cpiSetWldr, json);
    parcJSON_Release(&json);

    return result;
}

CPIManageWldr *
cpiLinks_ManageWldrFromControlMessage(CCNxControl *control)
{
    assertNotNull(control, "Parameter control must be non-null");
    PARCJSON *json = ccnxControl_GetJson(control);

    PARCJSONPair *cpiWldrOpPair = cpi_ParseRequest(json);

    PARCJSON *cpiWldrJson = parcJSONValue_GetJSON(parcJSONPair_GetValue(cpiWldrOpPair));
    CPIManageWldr *cpiWldr = cpiManageWldr_FromJson(cpiWldrJson);

    return cpiWldr;


    return cpiWldr;
}

const char *
cpiLinks_SetWldrJsonTag()
{
    return cpiSetWldr;
}

