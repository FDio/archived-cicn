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
#include <ccnx/api/control/cpi_Forwarding.h>
#include <LongBow/runtime.h>

#include "cpi_private.h"

static const char *cpiRegister = "REGISTER";
static const char *cpiUnregister = "UNREGISTER";
static const char *cpiRouteList = "ROUTE_LIST";
static const char *cpiSetStrategy = "SET_STRATEGY";

PARCJSON *
cpiForwarding_CreateSetStrategyRequest(CPIForwardingStrategy *fwdStrategy)
{
    PARCJSON *json = cpiForwardingStrategy_ToJson(fwdStrategy);
    PARCJSON *result = cpi_CreateRequest(cpiSetStrategy, json);
    parcJSON_Release(&json);

    return result;
}

PARCJSON *
cpiForwarding_CreateAddRouteRequest(const CPIRouteEntry *route)
{
    PARCJSON *routeAsJSON = cpiRouteEntry_ToJson(route);
    PARCJSON *result = cpi_CreateRequest(cpiRegister, routeAsJSON);
    parcJSON_Release(&routeAsJSON);

    return result;
}

PARCJSON *
cpiForwarding_CreateRemoveRouteRequest(const CPIRouteEntry *route)
{
    PARCJSON *routeAsJSON = cpiRouteEntry_ToJson(route);
    PARCJSON *result = cpi_CreateRequest(cpiUnregister, routeAsJSON);
    parcJSON_Release(&routeAsJSON);

    return result;
}

PARCJSON *
cpiForwarding_AddRouteToSelf(const CCNxName *prefix)
{
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(prefix);
    PARCJSON *result = cpiForwarding_AddRoute(route);
    cpiRouteEntry_Destroy(&route);
    return result;
}

PARCJSON *
cpiForwarding_RemoveRouteToSelf(const CCNxName *prefix)
{
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(prefix);
    PARCJSON *result = cpiForwarding_RemoveRoute(route);
    cpiRouteEntry_Destroy(&route);
    return result;
}

PARCJSON *
cpiForwarding_AddRoute(const CPIRouteEntry *route)
{
    PARCJSON *operation = cpiRouteEntry_ToJson(route);
    PARCJSON *result = cpi_CreateRequest(cpiRegister, operation);
    parcJSON_Release(&operation);

    return result;
}

PARCJSON *
cpiForwarding_RemoveRoute(const CPIRouteEntry *route)
{
    PARCJSON *operation = cpiRouteEntry_ToJson(route);
    PARCJSON *result = cpi_CreateRequest(cpiUnregister, operation);
    parcJSON_Release(&operation);

    return result;
}

CPIRouteEntry *
cpiForwarding_RouteFromControlMessage(CCNxControl *control)
{
    assertNotNull(control, "Parameter control must be non-null");
    PARCJSON *json = ccnxControl_GetJson(control);

    PARCJSONPair *routeOpPair = cpi_ParseRequest(json);
    PARCJSON *routeJson = parcJSONValue_GetJSON(parcJSONPair_GetValue(routeOpPair));

    CPIRouteEntry *route = cpiRouteEntry_FromJson(routeJson);

    return route;
}

CPIForwardingStrategy *
cpiForwarding_ForwardingStrategyFromControlMessage(CCNxControl *control)
{
    assertNotNull(control, "Parameter control must be non-null");
    PARCJSON *json = ccnxControl_GetJson(control);

    PARCJSONPair *fwdStrOpPair = cpi_ParseRequest(json);
    PARCJSON *fwdStrategyJson = parcJSONValue_GetJSON(parcJSONPair_GetValue(fwdStrOpPair));
    CPIForwardingStrategy *fwdStrategy = cpiForwardingStrategy_FromJson(fwdStrategyJson);

    return fwdStrategy;
}

const char *
cpiForwarding_AddRouteJsonTag()
{
    return cpiRegister;
}

const char *
cpiForwarding_RemoveRouteJsonTag()
{
    return cpiUnregister;
}

const char *
cpiForwarding_RouteListJsonTag()
{
    return cpiRouteList;
}

const char *
cpiForwarding_SetStrategyJsonTag()
{
    return cpiSetStrategy;
}


PARCJSON *
cpiForwarding_CreateRouteListRequest()
{
    PARCJSON *json = parcJSON_Create();
    PARCJSON *result = cpi_CreateRequest(cpiRouteList, json);
    parcJSON_Release(&json);

    return result;
}

CPIRouteEntryList *
cpiForwarding_RouteListFromControlMessage(CCNxControl *control)
{
    PARCJSON *json = ccnxControl_GetJson(control);
    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiRequest_GetJsonTag());
    if (value == NULL) {
        value = parcJSON_GetValueByName(json, cpiResponse_GetJsonTag());
    }
    PARCJSON *innerJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(innerJson, cpiRouteList);
    PARCJSON *operation = parcJSONValue_GetJSON(value);
    return cpiRouteEntryList_FromJson(operation);
}
