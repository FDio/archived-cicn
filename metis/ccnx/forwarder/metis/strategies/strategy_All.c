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
 * THIS STRATEGY IS DEPRECATED
 */

#include <config.h>
#include <stdio.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/strategies/strategy_All.h>

static void             _strategyAll_ReceiveObject(MetisStrategyImpl *strategy, const MetisNumberSet * egressId, const MetisMessage *objectMessage, MetisTicks rtt);
static MetisNumberSet *_strategyAll_LookupNexthop(MetisStrategyImpl *strategy, const MetisMessage *interestMessage);
static void             _strategyAll_AddNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route);
static void             _strategyAll_RemoveNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route);
static void             _strategyAll_ImplDestroy(MetisStrategyImpl **strategyPtr);

static MetisStrategyImpl _template = {
    .context       = NULL,
    .receiveObject = &_strategyAll_ReceiveObject,
    .lookupNexthop = &_strategyAll_LookupNexthop,
    .addNexthop    = &_strategyAll_AddNexthop,
    .removeNexthop = &_strategyAll_RemoveNexthop,
    .destroy       = &_strategyAll_ImplDestroy,
};

struct strategy_all;
typedef struct strategy_all StrategyAll;

struct strategy_all {
    int x;
};

MetisStrategyImpl *
strategyAll_Create()
{
    StrategyAll *strategy = parcMemory_AllocateAndClear(sizeof(StrategyAll));
    assertNotNull(strategy, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(StrategyAll));

    MetisStrategyImpl *impl = parcMemory_AllocateAndClear(sizeof(MetisStrategyImpl));
    assertNotNull(impl, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisStrategyImpl));
    memcpy(impl, &_template, sizeof(MetisStrategyImpl));
    impl->context = strategy;

    return impl;
}

// =======================================================
// Dispatch API

static void
_strategyAll_ReceiveObject(MetisStrategyImpl *strategy, const MetisNumberSet * egressId, const MetisMessage *objectMessage, MetisTicks rtt)
{
}

static MetisNumberSet *
_strategyAll_LookupNexthop(MetisStrategyImpl *strategy, const MetisMessage *interestMessage)
{
    return NULL;
}

static void
_strategyAll_AddNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route)
{
}

static void
_strategyAll_RemoveNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route)
{
}

static void
_strategyAll_ImplDestroy(MetisStrategyImpl **strategyPtr)
{
    assertNotNull(strategyPtr, "Parameter must be non-null double pointer");
    assertNotNull(*strategyPtr, "Parameter must dereference to non-null pointer");

    MetisStrategyImpl *impl = *strategyPtr;
    StrategyAll *strategy = (StrategyAll *) impl->context;

    parcMemory_Deallocate((void **) &strategy);
    parcMemory_Deallocate((void **) &impl);
    *strategyPtr = NULL;
}
