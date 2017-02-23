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
#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/strategies/strategy_RndSegment.h>


static void             _strategyRndSegment_ReceiveObject(MetisStrategyImpl *strategy, const MetisNumberSet *egressId, const MetisMessage *objectMessage, MetisTicks rtt);
static void             _strategyRndSegment_OnTimeout(MetisStrategyImpl *strategy, const MetisNumberSet *egressId);
static MetisNumberSet *_strategyRndSegment_LookupNexthop(MetisStrategyImpl *strategy, const MetisMessage *interestMessage);
static MetisNumberSet *_strategyRndSegment_ReturnNexthops(MetisStrategyImpl *strategy);
static unsigned         _strategyRndSegment_CountNexthops(MetisStrategyImpl *strategy);
static void             _strategyRndSegment_AddNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route);
static void             _strategyRndSegment_RemoveNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route);
static void             _strategyRndSegment_ImplDestroy(MetisStrategyImpl **strategyPtr);
static const char *_strategyRndSegment_GetStrategy(MetisStrategyImpl *strategy);

static MetisStrategyImpl _template = {
    .context        = NULL,
    .receiveObject  = &_strategyRndSegment_ReceiveObject,
    .onTimeout      = &_strategyRndSegment_OnTimeout,
    .lookupNexthop  = &_strategyRndSegment_LookupNexthop,
    .returnNexthops = &_strategyRndSegment_ReturnNexthops,
    .countNexthops  = &_strategyRndSegment_CountNexthops,
    .addNexthop     = &_strategyRndSegment_AddNexthop,
    .removeNexthop  = &_strategyRndSegment_RemoveNexthop,
    .destroy        = &_strategyRndSegment_ImplDestroy,
    .getStrategy    = &_strategyRndSegment_GetStrategy,
};

struct strategy_rnd_segment;
typedef struct strategy_rnd_segment StrategyRndSegment;


struct strategy_rnd_segment {
    MetisNumberSet *nexthops;
    MetisTlvName *segmentName;
    int last_used_face;
};

MetisStrategyImpl *
strategyRndSegment_Create()
{
    StrategyRndSegment *strategy = parcMemory_AllocateAndClear(sizeof(StrategyRndSegment));
    assertNotNull(strategy, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(StrategyRndSegment));

    strategy->nexthops = metisNumberSet_Create();
    strategy->segmentName = NULL;
    strategy->last_used_face = 0;
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
_strategyRndSegment_GetStrategy(MetisStrategyImpl *strategy)
{
    return FWD_STRATEGY_RANDOM_PER_DASH_SEGMENT;
}


static int
_select_Nexthop(StrategyRndSegment *strategy)
{
    unsigned len = metisNumberSet_Length(strategy->nexthops);
    if (len == 0) {
        return -1;
    }

    int rnd = (rand() % len);
    return metisNumberSet_GetItem(strategy->nexthops, rnd);
}


static void
_strategyRndSegment_ReceiveObject(MetisStrategyImpl *strategy, const MetisNumberSet *egressId, const MetisMessage *objectMessage, MetisTicks rtt)
{
}

static void
_strategyRndSegment_OnTimeout(MetisStrategyImpl *strategy, const MetisNumberSet *egressId)
{
}

//ATTENTION!! This interface force us to create a MetisNumberSet which need to be delited somewhere (maybe in the FIB where we call this function)
//The specification in the interface requires that this function never returns NULL. in case we have no output face we need to return an empty MetisNumberSet
static MetisNumberSet *
_strategyRndSegment_LookupNexthop(MetisStrategyImpl *strategy, const MetisMessage *interestMessage)
{
    StrategyRndSegment *srnd = (StrategyRndSegment *) strategy->context;


    unsigned in_connection = metisMessage_GetIngressConnectionId(interestMessage);
    unsigned nexthopSize = metisNumberSet_Length(srnd->nexthops);

    MetisNumberSet *out = metisNumberSet_Create();
    if ((nexthopSize == 0) || ((nexthopSize == 1) && metisNumberSet_Contains(srnd->nexthops, in_connection))) {
        //there are no output faces or the input face is also the only output face. return null to avoid loops
        return out;
    }

    if (metisMessage_HasName(interestMessage)) {
        MetisTlvName *interestName = metisMessage_GetName(interestMessage);
        size_t sc = metisTlvName_SegmentCount(interestName);
        interestName = metisTlvName_Slice(interestName, (sc - 1));

        if (srnd->segmentName == NULL) {
            srnd->segmentName = interestName;
        } else if (!metisTlvName_Equals(srnd->segmentName, interestName)) {
            metisTlvName_Release(&srnd->segmentName);
            srnd->segmentName = interestName;
        } else  {
            //here we need to check if the output face still exists or if someone erase it
            if (metisNumberSet_Contains(srnd->nexthops, srnd->last_used_face)) {
                // face exists, so keep using it!
                metisTlvName_Release(&interestName);
                metisNumberSet_Add(out, srnd->last_used_face);
                return out;
            } else  {
                //the face does not exists anymore, try to find a new face but keep the name
                //of the dash segment
                metisTlvName_Release(&interestName);
            }
        }
    }

    int out_connection;
    do {
        out_connection = _select_Nexthop(srnd);
    } while (out_connection == in_connection);

    if (out_connection == -1) {
        return out;
    }

    srnd->last_used_face = out_connection;
    metisNumberSet_Add(out, out_connection);
    return out;
}

static MetisNumberSet *
_strategyRndSegment_ReturnNexthops(MetisStrategyImpl *strategy)
{
    StrategyRndSegment *srnd = (StrategyRndSegment *) strategy->context;
    return srnd->nexthops;
}

unsigned
_strategyRndSegment_CountNexthops(MetisStrategyImpl *strategy)
{
    StrategyRndSegment *srnd = (StrategyRndSegment *) strategy->context;
    return metisNumberSet_Length(srnd->nexthops);
}

static void
_strategyRndSegment_AddNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route)
{
    unsigned connectionId = cpiRouteEntry_GetInterfaceIndex(route); //this returns what in all the rest of the ccnx code is called connection id!

    StrategyRndSegment *srnd = (StrategyRndSegment *) strategy->context;
    if (!metisNumberSet_Contains(srnd->nexthops, connectionId)) {
        metisNumberSet_Add(srnd->nexthops, connectionId);
    }
}

static void
_strategyRndSegment_RemoveNexthop(MetisStrategyImpl *strategy, CPIRouteEntry *route)
{
    unsigned connectionId = cpiRouteEntry_GetInterfaceIndex(route);
    StrategyRndSegment *srnd = (StrategyRndSegment *) strategy->context;

    if (metisNumberSet_Contains(srnd->nexthops, connectionId)) {
        metisNumberSet_Remove(srnd->nexthops, connectionId);
    }
}

static void
_strategyRndSegment_ImplDestroy(MetisStrategyImpl **strategyPtr)
{
    assertNotNull(strategyPtr, "Parameter must be non-null double pointer");
    assertNotNull(*strategyPtr, "Parameter must dereference to non-null pointer");

    MetisStrategyImpl *impl = *strategyPtr;
    StrategyRndSegment *strategy = (StrategyRndSegment *) impl->context;

    metisNumberSet_Release(&(strategy->nexthops));
    if (strategy->segmentName != NULL) {
        metisTlvName_Release(&strategy->segmentName);
    }

    parcMemory_Deallocate((void **) &strategy);
    parcMemory_Deallocate((void **) &impl);
    *strategyPtr = NULL;
}
