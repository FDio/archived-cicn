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
#include <time.h>
#include <stdlib.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_HashMap.h>

#include <ccnx/forwarder/metis/strategies/strategy_Rnd.h>


static void             _strategyRnd_ReceiveObject(MetisStrategyImpl *strategy, const MetisNumberSet *egressId, const MetisMessage *objectMessage, MetisTicks rtt);
static void             _strategyRnd_OnTimeout(MetisStrategyImpl *strategy, const MetisNumberSet *egressId);
static MetisNumberSet *_strategyRnd_LookupNexthop(MetisStrategyImpl *strategy, const MetisMessage *interestMessage);
static MetisNumberSet *_strategyRnd_ReturnNexthops(MetisStrategyImpl *strategy);
static unsigned         _strategyRnd_CountNexthops(MetisStrategyImpl *strategy);
static void             _strategyRnd_AddNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route);
static void             _strategyRnd_RemoveNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route);
static void             _strategyRnd_ImplDestroy(MetisStrategyImpl **strategyPtr);
static const char *_strategyRnd_GetStrategy(MetisStrategyImpl *strategy);

static MetisStrategyImpl _template = {
    .context        = NULL,
    .receiveObject  = &_strategyRnd_ReceiveObject,
    .onTimeout      = &_strategyRnd_OnTimeout,
    .lookupNexthop  = &_strategyRnd_LookupNexthop,
    .returnNexthops = &_strategyRnd_ReturnNexthops,
    .countNexthops  = &_strategyRnd_CountNexthops,
    .addNexthop     = &_strategyRnd_AddNexthop,
    .removeNexthop  = &_strategyRnd_RemoveNexthop,
    .destroy        = &_strategyRnd_ImplDestroy,
    .getStrategy    = &_strategyRnd_GetStrategy,
};

struct strategy_rnd;
typedef struct strategy_rnd StrategyRnd;


struct strategy_rnd {
    MetisNumberSet *nexthops;
};

MetisStrategyImpl *
strategyRnd_Create()
{
    StrategyRnd *strategy = parcMemory_AllocateAndClear(sizeof(StrategyRnd));
    assertNotNull(strategy, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(StrategyRnd));

    strategy->nexthops = metisNumberSet_Create();
    srand(time(NULL));

    MetisStrategyImpl *impl = parcMemory_AllocateAndClear(sizeof(MetisStrategyImpl));
    assertNotNull(impl, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisStrategyImpl));
    memcpy(impl, &_template, sizeof(MetisStrategyImpl));
    impl->context = strategy;

    return impl;
}

// =======================================================
// Dispatch API

static const char*
_strategyRnd_GetStrategy(MetisStrategyImpl *strategy)
{
    return FWD_STRATEGY_RANDOM;
}


static int
_select_Nexthop(StrategyRnd *strategy)
{
    unsigned len = metisNumberSet_Length(strategy->nexthops);
    if (len == 0) {
        return -1;
    }

    int rnd = (rand() % len);
    return metisNumberSet_GetItem(strategy->nexthops, rnd);
}


static void
_strategyRnd_ReceiveObject(MetisStrategyImpl *strategy, const MetisNumberSet *egressId, const MetisMessage *objectMessage, MetisTicks rtt)
{
}

static void
_strategyRnd_OnTimeout(MetisStrategyImpl *strategy, const MetisNumberSet *egressId)
{
}

//ATTENTION!! This interface force us to create a MetisNumberSet which need to be delited somewhere (maybe in the FIB where we call this function)
//The specification in the interface requires that this function never returns NULL. in case we have no output face we need to return an empty MetisNumberSet
static MetisNumberSet *
_strategyRnd_LookupNexthop(MetisStrategyImpl *strategy, const MetisMessage *interestMessage)
{
    StrategyRnd *srnd = (StrategyRnd *) strategy->context;

    unsigned in_connection = metisMessage_GetIngressConnectionId(interestMessage);
    unsigned nexthopSize = metisNumberSet_Length(srnd->nexthops);

    MetisNumberSet *out = metisNumberSet_Create();
    if ((nexthopSize == 0) || ((nexthopSize == 1) && metisNumberSet_Contains(srnd->nexthops, in_connection))) {
        //there are no output faces or the input face is also the only output face. return null to avoid loops
        return out;
    }

    unsigned out_connection;
    do {
        out_connection = _select_Nexthop(srnd);
    } while (out_connection == in_connection);

    if (out_connection == -1) {
        return out;
    }

    metisNumberSet_Add(out, out_connection);
    return out;
}

static MetisNumberSet *
_strategyRnd_ReturnNexthops(MetisStrategyImpl *strategy)
{
    StrategyRnd *srnd = (StrategyRnd *) strategy->context;
    return srnd->nexthops;
}

unsigned
_strategyRnd_CountNexthops(MetisStrategyImpl *strategy)
{
    StrategyRnd *srnd = (StrategyRnd *) strategy->context;
    return metisNumberSet_Length(srnd->nexthops);
}

static void
_strategyRnd_AddNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route)
{
    unsigned connectionId = cpiRouteEntry_GetInterfaceIndex(route); //this returns what in all the rest of the ccnx code is called connection id!

    StrategyRnd *srnd = (StrategyRnd *) strategy->context;
    if (!metisNumberSet_Contains(srnd->nexthops, connectionId)) {
        metisNumberSet_Add(srnd->nexthops, connectionId);
    }
}

static void
_strategyRnd_RemoveNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route)
{
    unsigned connectionId = cpiRouteEntry_GetInterfaceIndex(route);
    StrategyRnd *srnd = (StrategyRnd *) strategy->context;

    if (metisNumberSet_Contains(srnd->nexthops, connectionId)) {
        metisNumberSet_Remove(srnd->nexthops, connectionId);
    }
}

static void
_strategyRnd_ImplDestroy(MetisStrategyImpl **strategyPtr)
{
    assertNotNull(strategyPtr, "Parameter must be non-null double pointer");
    assertNotNull(*strategyPtr, "Parameter must dereference to non-null pointer");

    MetisStrategyImpl *impl = *strategyPtr;
    StrategyRnd *strategy = (StrategyRnd *) impl->context;

    metisNumberSet_Release(&(strategy->nexthops));

    parcMemory_Deallocate((void **) &strategy);
    parcMemory_Deallocate((void **) &impl);
    *strategyPtr = NULL;
}
