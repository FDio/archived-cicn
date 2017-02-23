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
#include "config_TestingComponent.h"

#include <ccnx/transport/transport_rta/core/components.h>

CCNxStackConfig *
testingUpper_ProtocolStackConfig(CCNxStackConfig *stackConfig)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    CCNxStackConfig *result = ccnxStackConfig_Add(stackConfig, testingUpper_GetName(), value);
    parcJSONValue_Release(&value);

    return result;
}

CCNxStackConfig *
testingLower_ProtocolStackConfig(CCNxStackConfig *stackConfig)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    CCNxStackConfig *result = ccnxStackConfig_Add(stackConfig, testingLower_GetName(), value);
    parcJSONValue_Release(&value);

    return result;
}

CCNxConnectionConfig *
testingUpper_ConnectionConfig(CCNxConnectionConfig *connConfig)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    CCNxConnectionConfig *result = ccnxConnectionConfig_Add(connConfig, testingUpper_GetName(), value);
    parcJSONValue_Release(&value);

    return result;
}

CCNxConnectionConfig *
testingLower_ConnectionConfig(CCNxConnectionConfig *connConfig)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    CCNxConnectionConfig *result = ccnxConnectionConfig_Add(connConfig, testingLower_GetName(), value);
    parcJSONValue_Release(&value);

    return result;
}

const char *
testingUpper_GetName()
{
    return RtaComponentNames[TESTING_UPPER];
}

const char *
testingLower_GetName()
{
    return RtaComponentNames[TESTING_LOWER];
}
