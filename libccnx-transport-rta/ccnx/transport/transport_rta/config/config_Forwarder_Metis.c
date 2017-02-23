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

#include <stdio.h>
#include "config_Forwarder_Metis.h"
#include <ccnx/transport/transport_rta/core/components.h>

static const char param_METIS_PORT[] = METIS_PORT_ENV;          // integer, e.g. 9695
static const short default_port = 9695;

/**
 * Generates:
 *
 * { "FWD_METIS" : { "port" : port } }
 */
CCNxStackConfig *
metisForwarder_ProtocolStackConfig(CCNxStackConfig *stackConfig)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    CCNxStackConfig *result = ccnxStackConfig_Add(stackConfig, metisForwarder_GetName(), value);
    parcJSONValue_Release(&value);

    return result;
}

/**
 * The metis forwarder port may be set per connection in the stack
 *
 * { "FWD_METIS" : { "port" : port } }
 */
CCNxConnectionConfig *
metisForwarder_ConnectionConfig(CCNxConnectionConfig *connConfig, uint16_t port)
{
    PARCJSON *json = parcJSON_Create();
    parcJSON_AddInteger(json, param_METIS_PORT, port);

    PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
    parcJSON_Release(&json);
    CCNxConnectionConfig *result = ccnxConnectionConfig_Add(connConfig, metisForwarder_GetName(), value);
    parcJSONValue_Release(&value);

    return result;
}

uint16_t
metisForwarder_GetDefaultPort()
{
    return default_port;
}

const char *
metisForwarder_GetName()
{
    return RtaComponentNames[FWD_METIS];
}

uint16_t
metisForwarder_GetPortFromConfig(PARCJSON *json)
{
    PARCJSONValue *value = parcJSON_GetValueByName(json, metisForwarder_GetName());
    assertNotNull(value, "Got null for %s json", metisForwarder_GetName());
    PARCJSON *metisJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(metisJson, param_METIS_PORT);
    return (uint16_t) parcJSONValue_GetInteger(value);
}
