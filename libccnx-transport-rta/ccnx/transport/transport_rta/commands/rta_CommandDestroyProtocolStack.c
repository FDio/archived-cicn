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
 * Implements the RtaCommandDestroyProtocolStack object which signals to RTA Framework to open a new connection
 * with the given configuration.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <ccnx/transport/transport_rta/commands/rta_CommandDestroyProtocolStack.h>

struct rta_command_destroyprotocolstack {
    int stackId;
};

parcObject_ExtendPARCObject(RtaCommandDestroyProtocolStack,
                            NULL, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(rtaCommandDestroyProtocolStack, RtaCommandDestroyProtocolStack);

parcObject_ImplementRelease(rtaCommandDestroyProtocolStack, RtaCommandDestroyProtocolStack);

// ======= Public API

RtaCommandDestroyProtocolStack *
rtaCommandDestroyProtocolStack_Create(int stackId)
{
    RtaCommandDestroyProtocolStack *createStack = parcObject_CreateInstance(RtaCommandDestroyProtocolStack);
    createStack->stackId = stackId;
    return createStack;
}


int
rtaCommandDestroyProtocolStack_GetStackId(const RtaCommandDestroyProtocolStack *destroyStack)
{
    assertNotNull(destroyStack, "Parameter destroyStack must be non-null");
    return destroyStack->stackId;
}
