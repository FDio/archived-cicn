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
 * @header <#Headline Name#>
 * <#Abstract#>
 *
 *     <#Discussion#>
 *
 * @author Marc Mosko
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

/**
 * These comments describe the implementation of the protocol described in the header file.
 *
 * A Control Plane Information (CPI) message is a JSON object of this form:
 *  {
 *     "CPI_REQUEST" | "CPI_RESPONSE" :
 *        {  "SEQUENCE" : <sequence number>,
 *           <operation> : <contents>
 *        }
 *     ["AUTHENTICATOR" : <TBD proof based on request/response, e.g. a crypto signature>]
 *  }
 *
 *  {
 *     "CPI_ACK" :
 *        {  "SEQUENCE" : <sequence number>,
 *           "RETURN"   : "ACK" or "NACK",
 *           "REQUEST"  : <original request JSON>
 *           [, "MESSAGE" : <optional message> ]
 *        }
 *     ["AUTHENTICATOR" : <TBD proof based on request/response, e.g. a crypto signature>]
 *  }
 *
 *
 *  { "REGISTER" :
 *     {  "PREFIX"    : <name URI string>,
 *        "INTERFACE" : <integer>,
 *        "FLAGS"     : <integer>
 *        [, "LIFETIME" : [seconds, micro_seconds] ]
 *     }
 *  }
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

#include <config.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include <LongBow/runtime.h>

#include "controlPlaneInterface.h"
#include "cpi_private.h"
#include "cpi_NameRouteProtocolType.h"
#include "cpi_Acks.h"
#include <ccnx/api/control/cpi_ConnectionEthernet.h>

static const char *cpiRequest = "CPI_REQUEST";
static const char *cpiResponse = "CPI_RESPONSE";
static const char *cpiPause = "CPI_PAUSE";
static const char *cpiFlush = "CPI_FLUSH";

// This is the unique sequence number used by all messages and its thread locks
static pthread_mutex_t cpiNextSequenceNumberMutex = PTHREAD_MUTEX_INITIALIZER;
static uint64_t cpiNextSequenceNumber = 1;

const char *
cpiRequest_GetJsonTag()
{
    return cpiRequest;
}

const char *
cpiResponse_GetJsonTag()
{
    return cpiResponse;
}

const char *
cpiSequence_GetJSONTag()
{
    return cpiSeqnum;
}

uint64_t
cpi_GetNextSequenceNumber(void)
{
    uint64_t seqnum;

    int result = pthread_mutex_lock(&cpiNextSequenceNumberMutex);
    assertTrue(result == 0, "Got error from pthread_mutex_lock: %d", result);

    seqnum = cpiNextSequenceNumber++;

    result = pthread_mutex_unlock(&cpiNextSequenceNumberMutex);
    assertTrue(result == 0, "Got error from pthread_mutex_unlock: %d", result);

    return seqnum;
}

CpiOperation
cpi_getCPIOperation2(const PARCJSON *json)
{
    PARCJSONValue *cpi_value = parcJSON_GetValueByName(json, cpiRequest);

    if (cpi_value == NULL) {
        cpi_value = parcJSON_GetValueByName(json, cpiResponse);
    }
    assertNotNull(cpi_value, "Could not get Request or response");

    PARCJSON *cpi_json = parcJSONValue_GetJSON(cpi_value);

    /*
     * The JSON is defined as { REQUEST : { SEQUENCE: xxx, <OPERATION>: xxx } }
     * so we want to get the key of the 2nd item (index 1) of the array of objects
     * under the request
     */

    PARCJSONPair *item1Pair = parcJSON_GetPairByIndex(cpi_json, 1);
    PARCBuffer *name = parcJSONPair_GetName(item1Pair);
    const char *p = parcBuffer_Overlay(name, 0);

    if (strncasecmp(p, cpiForwarding_AddRouteJsonTag(), strlen(cpiForwarding_AddRouteJsonTag())) == 0) {
        return CPI_REGISTER_PREFIX;
    }

    if (strncasecmp(p, cpiForwarding_RemoveRouteJsonTag(), strlen(cpiForwarding_RemoveRouteJsonTag())) == 0) {
        return CPI_UNREGISTER_PREFIX;
    }

    if (strncasecmp(p, cpiPause, strlen(cpiPause)) == 0) {
        return CPI_PAUSE;
    }

    if (strncasecmp(p, cpiFlush, strlen(cpiFlush)) == 0) {
        return CPI_FLUSH;
    }

    if (strncasecmp(p, cpiCancelFlow_CancelFlowJsonTag(), strlen(cpiCancelFlow_CancelFlowJsonTag())) == 0) {
        return CPI_CANCEL_FLOW;
    }

    if (strncasecmp(p, cpiLinks_InterfaceListJsonTag(), strlen(cpiLinks_InterfaceListJsonTag())) == 0) {
        return CPI_INTERFACE_LIST;
    }

    if (strncasecmp(p, cpiForwarding_RouteListJsonTag(), strlen(cpiForwarding_RouteListJsonTag())) == 0) {
        return CPI_PREFIX_REGISTRATION_LIST;
    }

    if (strncasecmp(p, cpiLinks_CreateTunnelJsonTag(), strlen(cpiLinks_CreateTunnelJsonTag())) == 0) {
        return CPI_CREATE_TUNNEL;
    }

    if (strncasecmp(p, cpiLinks_RemoveTunnelJsonTag(), strlen(cpiLinks_RemoveTunnelJsonTag())) == 0) {
        return CPI_REMOVE_TUNNEL;
    }

    if (strncasecmp(p, cpiLinks_ConnectionListJsonTag(), strlen(cpiLinks_ConnectionListJsonTag())) == 0) {
        return CPI_CONNECTION_LIST;
    }

    if (strncasecmp(p, cpiLinks_AddEtherConnectionJasonTag(), strlen(cpiLinks_AddEtherConnectionJasonTag())) == 0) {
        return (CPI_ADD_ETHER_CONNECTION);
    }

    if (strncasecmp(p, cpiManageChaces_CacheStoreOnJsonTag(), strlen(cpiManageChaces_CacheStoreOnJsonTag())) == 0) {
        return CPI_CACHE_STORE_ON;
    }

    if (strncasecmp(p, cpiManageChaces_CacheStoreOffJsonTag(), strlen(cpiManageChaces_CacheStoreOffJsonTag())) == 0) {
        return CPI_CACHE_STORE_OFF;
    }

    if (strncasecmp(p, cpiManageChaces_CacheServeOnJsonTag(), strlen(cpiManageChaces_CacheServeOnJsonTag())) == 0) {
        return CPI_CACHE_SERVE_ON;
    }

    if (strncasecmp(p, cpiManageChaces_CacheServeOffJsonTag(), strlen(cpiManageChaces_CacheServeOffJsonTag())) == 0) {
        return CPI_CACHE_SERVE_OFF;
    }

    if (strncasecmp(p, cpiManageChaces_CacheClearJsonTag(), strlen(cpiManageChaces_CacheClearJsonTag())) == 0) {
        return CPI_CACHE_CLEAR;
    }

    if (strncasecmp(p, cpiForwarding_SetStrategyJsonTag(), strlen(cpiForwarding_SetStrategyJsonTag())) == 0) {
        return CPI_SET_FORWARDING_STRATEGY;
    }

    if (strncasecmp(p, cpiLinks_SetWldrJsonTag(), strlen(cpiLinks_SetWldrJsonTag())) == 0) {
        return CPI_SET_WLDR;
    }

    if (strncasecmp(p, "AddConnEther", strlen("AddConnEther")) == 0) {
        return CPI_ADD_CONNECTION_ETHERNET;
    }

    if (strncasecmp(p, "RemoveConnEther", strlen("RemoveConnEther")) == 0) {
        return CPI_REMOVE_CONNECTION_ETHERNET;
    }

    if (strncasecmp(p, "AddListener", strlen("AddListener")) == 0) {
        return CPI_ADD_LISTENER;
    }

    if (strncasecmp(p, "RemoveListener", strlen("RemoveListener")) == 0) {
        return CPI_REMOVE_LISTENER;
    }

    trapIllegalValue(json, "Could not parse: %s\n", parcJSON_ToString(json));
}

/**
 * Return the relevant operation from a REQUEST or a REPSONSE.
 * Do not call on an ACK
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CpiOperation
cpi_GetMessageOperation(CCNxControl *control)
{
    if (cpiConnectionEthernet_IsAddMessage(control)) {
        return CPI_ADD_CONNECTION_ETHERNET;
    }

    if (cpiConnectionEthernet_IsRemoveMessage(control)) {
        return CPI_REMOVE_CONNECTION_ETHERNET;
    }

    PARCJSON *json = ccnxControl_GetJson(control);

    CpiOperation result = cpi_getCPIOperation2(json);
    return result;
}

CpiMessageType
controlPlaneInterface_GetCPIMessageType(PARCJSON *json)
{
    assertNotNull(json, "Invalid state, got NULL json from control message");

    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiResponse);
    if (value != NULL) {
        return CPI_RESPONSE;
    }

    value = parcJSON_GetValueByName(json, cpiRequest);
    if (value != NULL) {
        return CPI_REQUEST;
    }

    value = parcJSON_GetValueByName(json, cpiAck);
    if (value != NULL) {
        return CPI_ACK;
    }

    trapIllegalValue(json, "Expected CpiMessageType, actual %s", parcJSON_ToString(json));
}

/**
 * You should verify that it's a CPI message with cpi_IsCpiMessage() before using this.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CpiMessageType
cpi_GetMessageType(const CCNxControl *control)
{
    PARCJSON *json = ccnxControl_GetJson(control);
    CpiMessageType result = controlPlaneInterface_GetCPIMessageType(json);
    return result;
}

/**
 * Returns the inner operation JSON from the request.
 *
 *   INPUT: "{ CPI_REQUEST: { SEQUENCE:number key: { operation } }}"
 *   OUTPUT: "{ key : { operation } }"
 *
 *   Example return: "{ operation }"
 * @param <#param1#>
 * @return The inner json, do not destroy it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSONPair *
cpi_ParseRequest(PARCJSON *request)
{
    PARCJSONValue *value = parcJSON_GetValueByName(request, cpiRequest);
    assertNotNull(value, "Could not find JSON key %s in %s", cpiRequest, parcJSON_ToString(request));
    assertTrue(parcJSONValue_IsJSON(value), "cpiRequest is unexpected type");

    PARCJSON *requestJson = parcJSONValue_GetJSON(value);
    PARCJSONPair *result = parcJSON_GetPairByIndex(requestJson, 1);

    return result;
}

CCNxControl *
cpi_ForwarderVersion()
{
    return NULL;
}

uint64_t
controlPlaneInterface_GetSequenceNumber(const PARCJSON *controlPlaneMessage)
{
    assertNotNull(controlPlaneMessage, "Invalid state, got NULL json from control message");

    PARCJSONValue *value = parcJSON_GetValueByName(controlPlaneMessage, cpiRequest);
    if (value == NULL) {
        value = parcJSON_GetValueByName(controlPlaneMessage, cpiResponse);
    }
    if (value == NULL) {
        value = parcJSON_GetValueByName(controlPlaneMessage, cpiAck);
    }

    assertNotNull(value, "Could not get request or response");

    PARCJSON *json = parcJSONValue_GetJSON(value);
    value = parcJSON_GetValueByName(json, cpiSeqnum);
    assertNotNull(value, "Could not retrieve key %s from CPI section", cpiSeqnum);

    return parcJSONValue_GetInteger(value);
}

/**
 * All CPI messages carry a sequence number.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint64_t
cpi_GetSequenceNumber(CCNxControl *control)
{
    PARCJSON *json = ccnxControl_GetJson(control);

    return controlPlaneInterface_GetSequenceNumber(json);
}

PARCJSON *
cpi_CreatePauseInputRequest(void)
{
    PARCJSON *operation = parcJSON_Create();
    PARCJSON *result = cpi_CreateRequest(cpiPause, operation);
    parcJSON_Release(&operation);

    return result;
}

PARCJSON *
cpi_CreateFlushRequest(void)
{
    PARCJSON *operation = parcJSON_Create();
    PARCJSON *result = cpi_CreateRequest(cpiFlush, operation);
    parcJSON_Release(&operation);

    return result;
}

/**
 * Given the inner operation member, wrap it in a Request with a sequence number
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *
cpi_CreateRequest(const char *key, PARCJSON *operation)
{
    PARCJSON *result = parcJSON_Create();
    PARCJSON *request = parcJSON_Create();

    uint64_t seqnum = cpi_GetNextSequenceNumber();

    parcJSON_AddInteger(request, cpiSeqnum, (int) seqnum);
    parcJSON_AddObject(request, key, operation);
    parcJSON_AddObject(result, cpiRequest, request);
    parcJSON_Release(&request);

    return result;
}

CCNxControl *
cpi_CreateResponse(CCNxControl *request, PARCJSON *operation)
{
    PARCJSON *requestJson = ccnxControl_GetJson(request);

    // use the same key as the request
    uint64_t seqnum = controlPlaneInterface_GetSequenceNumber(requestJson);

    PARCJSONValue *value = parcJSON_GetValueByName(requestJson, cpiRequest);
    assertNotNull(value, "Could not get request or response");
    assertTrue(parcJSONValue_IsJSON(value), "cpiRequest should be a JSON object");

    PARCJSON *operationJson = parcJSONValue_GetJSON(value);
    PARCJSONPair *pair = parcJSON_GetPairByIndex(operationJson, 1);
    const PARCBuffer *opKeyBuf = parcJSONPair_GetName(pair);
    const char *opKey = parcBuffer_ToString(opKeyBuf);

    PARCJSON *response = parcJSON_Create();
    parcJSON_AddInteger(response, cpiSeqnum, (int) seqnum);
    parcJSON_AddObject(response, opKey, operation);
    parcMemory_Deallocate(&opKey);

    PARCJSON *responseJson = parcJSON_Create();
    parcJSON_AddObject(responseJson, cpiResponse, response);
    parcJSON_Release(&response);

    CCNxControl *result = ccnxControl_CreateCPIRequest(responseJson);

    parcJSON_Release(&responseJson);

    return result;
}
