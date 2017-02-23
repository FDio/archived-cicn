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

#include <ccnx/forwarder/metis/io/metis_IoOperations.h>

#include <LongBow/runtime.h>

void *
metisIoOperations_GetClosure(const MetisIoOperations *ops)
{
    assertNotNull(ops, "Parameter ops must be non-null");
    return ops->closure;
}

bool
metisIoOperations_Send(MetisIoOperations *ops, const CPIAddress *nexthop, MetisMessage *message)
{
    return ops->send(ops, nexthop, message);
}

const CPIAddress *
metisIoOperations_GetRemoteAddress(const MetisIoOperations *ops)
{
    return ops->getRemoteAddress(ops);
}

const MetisAddressPair *
metisIoOperations_GetAddressPair(const MetisIoOperations *ops)
{
    return ops->getAddressPair(ops);
}

bool
metisIoOperations_IsUp(const MetisIoOperations *ops)
{
    return ops->isUp(ops);
}

bool
metisIoOperations_IsLocal(const MetisIoOperations *ops)
{
    return ops->isLocal(ops);
}

unsigned
metisIoOperations_GetConnectionId(const MetisIoOperations *ops)
{
    return ops->getConnectionId(ops);
}

void
metisIoOperations_Release(MetisIoOperations **opsPtr)
{
    MetisIoOperations *ops = *opsPtr;
    ops->destroy(opsPtr);
}

const void *
metisIoOperations_Class(const MetisIoOperations *ops)
{
    return ops->class(ops);
}

CPIConnectionType
metisIoOperations_GetConnectionType(const MetisIoOperations *ops)
{
    return ops->getConnectionType(ops);
}

MetisTicks 
metisIoOperations_SendProbe(MetisIoOperations *ops, unsigned probeType)
{
    return ops->sendProbe(ops, probeType);
}

