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
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
/*
 *  {
 *      "CPI_ACK" : {
 *          "SEQUENCE" : <sequence number>,
 *          "RETURN"   : "ACK" or "NACK",
 *          "REQUEST"  : <original request JSON>
 *          [, "MESSAGE" : <optional message> ]
 *        }
 *     ["AUTHENTICATOR" : <TBD proof based on request/response, e.g. a crypto signature>]
 *  }
 */

#include <config.h>
#include <stdio.h>
#include <strings.h>

#include <LongBow/runtime.h>

#include "controlPlaneInterface.h"
#include "cpi_private.h"
#include "cpi_Acks.h"

struct control_plane_ack {
    ControlPlaneInformation cpi_ack;
    CpiAckType ack_type;
    ControlPlaneInformation cpi_original;
};

static const char *cpiReturn = "RETURN";
static const char *cpiReturnAck = "ACK";
static const char *cpiReturnNack = "NACK";
static const char *cpiOriginal = "REQUEST";
static const char *cpiRequest = "CPI_REQUEST";

PARCJSON *
cpiAcks_CreateAck(const PARCJSON *originalRequest)
{
    uint64_t seqnum = cpi_GetNextSequenceNumber();
    PARCJSON *body = parcJSON_Create();

    parcJSON_AddInteger(body, cpiSeqnum, (int) seqnum);
    parcJSON_AddString(body, cpiReturn, cpiReturnAck);

    PARCJSON *copy = parcJSON_Copy(originalRequest);
    parcJSON_AddObject(body, cpiOriginal, copy);
    parcJSON_Release(&copy);

    PARCJSON *json = parcJSON_Create();

    parcJSON_AddObject(json, cpiAck, body);
    parcJSON_Release(&body);

    return json;
}

PARCJSON *
cpiAcks_CreateNack(const PARCJSON *request)
{
    uint64_t seqnum = cpi_GetNextSequenceNumber();
    PARCJSON *body = parcJSON_Create();
    parcJSON_AddInteger(body, cpiSeqnum, (int) seqnum);
    parcJSON_AddString(body, cpiReturn, cpiReturnNack);

    PARCJSON *copy = parcJSON_Copy(request);
    parcJSON_AddObject(body, cpiOriginal, copy);
    parcJSON_Release(&copy);

    PARCJSON *json = parcJSON_Create();
    parcJSON_AddObject(json, cpiAck, body);
    parcJSON_Release(&body);

    return json;
}

bool
cpiAcks_IsAck(const PARCJSON *json)
{
    PARCJSONValue *ack_value = parcJSON_GetValueByName(json, cpiAck);
    if (ack_value != NULL) {
        PARCJSON *ack_json = parcJSONValue_GetJSON(ack_value);
        PARCJSONValue *return_value = parcJSON_GetValueByName(ack_json, cpiReturn);
        PARCBuffer *sBuf = parcJSONValue_GetString(return_value);
        const char *returnStr = parcBuffer_Overlay(sBuf, 0);
        return strcasecmp(returnStr, cpiReturnAck) == 0;
    }
    return false;
}

uint64_t
cpiAcks_GetAckOriginalSequenceNumber(const PARCJSON *json)
{
    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiAck);
    assertNotNull(value, "got null ack json: %s", parcJSON_ToString(json));

    PARCJSON *tempJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(tempJson, cpiOriginal);
    assertNotNull(value, "got null original json from the ack: %s", parcJSON_ToString(tempJson));

    tempJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(tempJson, cpiRequest);
    assertNotNull(value, "got null request json from the ack: %s", parcJSON_ToString(tempJson));

    tempJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(tempJson, cpiSeqnum);
    assertNotNull(value, "got null seqnum inside the request: %s", parcJSON_ToString(tempJson));

    return parcJSONValue_GetInteger(value);
}
