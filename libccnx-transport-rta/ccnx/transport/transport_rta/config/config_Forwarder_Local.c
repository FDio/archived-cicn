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
#include "config_Forwarder_Local.h"

#include <ccnx/transport/transport_rta/core/components.h>
#include <LongBow/runtime.h>

static const char param_FWD_LOCAL_NAME[] = "LOCAL_NAME";

CCNxStackConfig *
localForwarder_ProtocolStackConfig(CCNxStackConfig *stackConfig)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    CCNxStackConfig *result = ccnxStackConfig_Add(stackConfig, localForwarder_GetName(), value);
    parcJSONValue_Release(&value);
    return result;
}

/**
 * Generates:
 *
 * { "FWD_LOCAL" : { "path" : pipePath } }
 */
CCNxConnectionConfig *
localForwarder_ConnectionConfig(CCNxConnectionConfig *connConfig, const char *pipePath)
{
    PARCJSON *json = parcJSON_Create();
    parcJSON_AddString(json, param_FWD_LOCAL_NAME, pipePath);
    PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
    parcJSON_Release(&json);
    CCNxConnectionConfig *result = ccnxConnectionConfig_Add(connConfig, localForwarder_GetName(), value);
    parcJSONValue_Release(&value);
    return result;
}

const char *
localForwarder_GetName()
{
    return RtaComponentNames[FWD_LOCAL];
}

const char *
localForwarder_GetPath(PARCJSON *json)
{
    PARCJSONValue *value = parcJSON_GetValueByName(json, localForwarder_GetName());
    assertNotNull(value, "Got null for %s json", localForwarder_GetName());
    PARCJSON *localJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(localJson, param_FWD_LOCAL_NAME);
    assertNotNull(value, "Must specify a path for the PF_UNIX pipe for local forwarder");
    assertTrue(parcJSONValue_IsString(value), "JSON key %s must be type STRING", localForwarder_GetName());

    PARCBuffer *sBuf = parcJSONValue_GetString(value);
    const char *path = parcBuffer_Overlay(sBuf, 0);

    return path;
}
