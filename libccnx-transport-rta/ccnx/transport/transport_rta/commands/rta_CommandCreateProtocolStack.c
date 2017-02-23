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
 * Implements the RtaCommandCreateProtocolStack object which signals to RTA Framework to open a new connection
 * with the given configuration.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <ccnx/transport/transport_rta/commands/rta_CommandCreateProtocolStack.h>

struct rta_command_createprotocolstack {
    int stackId;
    CCNxStackConfig *config;
};

static void
_rtaCommandCreateProtocolStack_Destroy(RtaCommandCreateProtocolStack **openConnectionPtr)
{
    RtaCommandCreateProtocolStack *openConnection = *openConnectionPtr;

    if (openConnection->config) {
        ccnxStackConfig_Release(&openConnection->config);
    }
}

parcObject_ExtendPARCObject(RtaCommandCreateProtocolStack, _rtaCommandCreateProtocolStack_Destroy,
                            NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(rtaCommandCreateProtocolStack, RtaCommandCreateProtocolStack);

parcObject_ImplementRelease(rtaCommandCreateProtocolStack, RtaCommandCreateProtocolStack);

RtaCommandCreateProtocolStack *
rtaCommandCreateProtocolStack_Create(int stackId, CCNxStackConfig *config)
{
    RtaCommandCreateProtocolStack *createStack = parcObject_CreateInstance(RtaCommandCreateProtocolStack);
    createStack->stackId = stackId;
    createStack->config = ccnxStackConfig_Copy(config);
    return createStack;
}

const char *
rtaCommandCreateProtocolStack_AssessValidity(const RtaCommandCreateProtocolStack *instance)
{
    char *result = NULL;

    if (instance != NULL) {
        if (ccnxStackConfig_IsValid(instance->config)) {
            result = NULL;
        } else {
            result = "CCNxStackConfig instance is invalid";
        }
    } else {
        result = "Instance cannot be NULL";
    }

    return result;
}

bool
rtaCommandCreateProtocolStack_IsValid(const RtaCommandCreateProtocolStack *instance)
{
    const char *assessment = rtaCommandCreateProtocolStack_AssessValidity(instance);
    return assessment == NULL;
}

void
rtaCommandCreateProtocolStack_AssertValid(const RtaCommandCreateProtocolStack *instance)
{
    const char *assessment = rtaCommandCreateProtocolStack_AssessValidity(instance);
    trapIllegalValueIf(assessment != NULL, "%s", assessment);
}

int
rtaCommandCreateProtocolStack_GetStackId(const RtaCommandCreateProtocolStack *createStack)
{
    assertNotNull(createStack, "Parameter createStack must be non-null");
    return createStack->stackId;
}

CCNxStackConfig *
rtaCommandCreateProtocolStack_GetStackConfig(const RtaCommandCreateProtocolStack *createStack)
{
    return createStack->config;
}

PARCJSON *
rtaCommandCreateProtocolStack_GetConfig(const RtaCommandCreateProtocolStack *createStack)
{
    assertNotNull(createStack, "Parameter createStack must be non-null");
    return ccnxStackConfig_GetJson(createStack->config);
}
