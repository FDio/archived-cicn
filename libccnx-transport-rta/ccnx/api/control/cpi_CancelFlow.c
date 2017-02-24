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
#include <string.h>

#include <LongBow/runtime.h>

#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_CancelFlow.h>
#include <parc/algol/parc_Memory.h>

static const char *cpiCancelFlow = "CPI_CANCEL_FLOW";
static const char *cpiFlowName = "FLOW_NAME";

PARCJSON *
cpiCancelFlow_CreateRequest(const CCNxName *name)
{
    PARCJSON *operation = parcJSON_Create();

    char *uri = ccnxName_ToString(name);
    parcJSON_AddString(operation, cpiFlowName, uri);
    parcMemory_Deallocate((void **) &uri);

    PARCJSON *result = cpi_CreateRequest(cpiCancelFlow, operation);
    parcJSON_Release(&operation);

    return result;
}

PARCJSON *
cpiCancelFlow_Create(const CCNxName *name)
{
    PARCJSON *operation = parcJSON_Create();

    char *uri = ccnxName_ToString(name);
    parcJSON_AddString(operation, cpiFlowName, uri);
    parcMemory_Deallocate((void **) &uri);

    PARCJSON *result = cpi_CreateRequest(cpiCancelFlow, operation);
    parcJSON_Release(&operation);

    return result;
}

CCNxName *
cpiCancelFlow_GetFlowName(const PARCJSON *controlMessage)
{
    assertNotNull(controlMessage, "Parameter controlMessage must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(controlMessage, cpiRequest_GetJsonTag());
    assertNotNull(value, "only support getting the name from a Request at the moment, not from an ack/nack.");
    PARCJSON *inner_json = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(inner_json, cpiCancelFlow_CancelFlowJsonTag());
    assertNotNull(value, "Missing JSON tag in control message: %s", cpiCancelFlow_CancelFlowJsonTag());
    inner_json = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(inner_json, cpiFlowName);
    assertNotNull(value, "Missing JSON tag in control message: %s", cpiFlowName);
    PARCBuffer *sBuf = parcJSONValue_GetString(value);
    const char *uri = parcBuffer_Overlay(sBuf, 0);

    CCNxName *name = ccnxName_CreateFromCString(uri);

    return name;
}

CCNxName *
cpiCancelFlow_NameFromControlMessage(CCNxControl *control)
{
    assertNotNull(control, "Parameter control must be non-null");
    return cpiCancelFlow_GetFlowName(ccnxControl_GetJson(control));
}

bool
cpiCancelFlow_SuccessFromResponse(CCNxControl *control)
{
    trapNotImplemented("cpiCancelFlow_SuccessFromResponse");
}

const char *
cpiCancelFlow_CancelFlowJsonTag(void)
{
    return cpiCancelFlow;
}
