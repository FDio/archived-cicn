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

#include <ccnx/forwarder/metis/processor/metis_FibEntry.h>
#include <ccnx/forwarder/metis/core/metis_NumberSet.h>

#include <ccnx/forwarder/metis/strategies/metis_StrategyImpl.h>
#include <ccnx/forwarder/metis/strategies/strategy_Rnd.h>
#include <ccnx/forwarder/metis/strategies/strategy_LoadBalancer.h>
#include <ccnx/forwarder/metis/strategies/strategy_RndSegment.h>
#include <ccnx/forwarder/metis/strategies/strategy_LoadBalancerWithPD.h>

#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

struct metis_fib_entry {
    MetisTlvName *name;
    unsigned refcount;
    MetisStrategyImpl *fwdStrategy;
};


MetisFibEntry *
metisFibEntry_Create(MetisTlvName *name, const char *fwdStrategy)
{
    MetisFibEntry *fibEntry = parcMemory_AllocateAndClear(sizeof(MetisFibEntry));
    assertNotNull(fibEntry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisFibEntry));
    fibEntry->name = metisTlvName_Acquire(name);
    CCNxName *ccnxName = metisTlvName_ToCCNxName(name);
    char *strname = ccnxName_ToString(ccnxName);
    if (strcmp(fwdStrategy, FWD_STRATEGY_LOADBALANCER) == 0) {
        printf("[Metis Forwarding Strategy] --- set \"laodbalancer\" for %s\n", strname);
        fibEntry->fwdStrategy = strategyLoadBalancer_Create();
    } else if (strcmp(fwdStrategy, FWD_STRATEGY_RANDOM_PER_DASH_SEGMENT) == 0) {
        printf("[Metis Forwarding Strategy] --- set \"random_per_dash_segment\" for %s\n", strname);
        fibEntry->fwdStrategy = strategyRndSegment_Create();
    } else if (strcmp(fwdStrategy, FWD_STRATEGY_LOADBALANCER_WITH_DELAY) == 0) {
        printf("[Metis Forwarding Strategy] --- set \"laodbalancer with dealy\" for %s\n", strname);
        fibEntry->fwdStrategy = strategyLoadBalancerWithPD_Create();
    } else {
        //random is the defualt strategy
        printf("[Metis Forwarding Strategy] --- set \"random\" for %s\n", strname);
        fibEntry->fwdStrategy = strategyRnd_Create(); //the Random strategy is the default one
                                                      //other strategies can be set using the appropiate function
    }

    ccnxName_Release(&ccnxName);
    parcMemory_Deallocate((void **) &strname);
    fibEntry->refcount = 1;
    return fibEntry;
}

MetisFibEntry *
metisFibEntry_Acquire(const MetisFibEntry *fibEntry)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    MetisFibEntry *copy = (MetisFibEntry *) fibEntry;
    copy->refcount++;
    return copy;
}

void
metisFibEntry_Release(MetisFibEntry **fibEntryPtr)
{
    MetisFibEntry *fibEntry = *fibEntryPtr;
    assertTrue(fibEntry->refcount > 0, "Illegal state: refcount is 0");
    fibEntry->refcount--;
    if (fibEntry->refcount == 0) {
        metisTlvName_Release(&fibEntry->name);
        fibEntry->fwdStrategy->destroy(&(fibEntry->fwdStrategy));
        parcMemory_Deallocate((void **) &fibEntry);
    }
    *fibEntryPtr = NULL;
}

void
metisFibEntry_SetStrategy(MetisFibEntry *fibEntry, const char *strategy)
{
    MetisStrategyImpl *fwdStrategyImpl;
    char *strname = ccnxName_ToString(metisTlvName_ToCCNxName(fibEntry->name));
    if (strcmp(strategy, FWD_STRATEGY_LOADBALANCER) == 0) {
        printf("[Metis Forwarding Strategy] --- change to \"laodbalancer\" for %s\n", strname);
        fwdStrategyImpl = strategyLoadBalancer_Create();
    } else if (strcmp(strategy, FWD_STRATEGY_RANDOM_PER_DASH_SEGMENT) == 0) {
        printf("[Metis Forwarding Strategy] --- change to \"random_per_dash_segment\" for %s\n", strname);
        fwdStrategyImpl = strategyRndSegment_Create();
    } else if (strcmp(strategy, FWD_STRATEGY_LOADBALANCER_WITH_DELAY) == 0) {
        printf("[Metis Forwarding Strategy] --- change to \"loadbalancer_with_delay\" for %s\n", strname);
        fwdStrategyImpl = strategyLoadBalancerWithPD_Create();
    } else {
        //random is the defualt strategy
        printf("[Metis Forwarding Strategy] --- change to \"random\" for %s\n", strname);
        fwdStrategyImpl = strategyRnd_Create(); //the Random strategy is the default one
        //other strategies can be set using the appropiate function
    }
    parcMemory_Deallocate((void **) &strname);

    const MetisNumberSet *nexthops = metisFibEntry_GetNexthops(fibEntry);
    unsigned size = metisFibEntry_NexthopCount(fibEntry);
    for (unsigned i = 0; i < size; i++) {
        CCNxName *ccnxName = metisTlvName_ToCCNxName(fibEntry->name);
        CPIRouteEntry *cpiRouteEntry = cpiRouteEntry_Create(ccnxName, metisNumberSet_GetItem(nexthops, i), NULL, 0, 0, NULL, 0);
        fwdStrategyImpl->addNexthop(fwdStrategyImpl, cpiRouteEntry);
        cpiRouteEntry_Destroy(&cpiRouteEntry);
    }
    fibEntry->fwdStrategy->destroy(&(fibEntry->fwdStrategy));
    fibEntry->fwdStrategy = fwdStrategyImpl;
}

void
metisFibEntry_AddNexthop(MetisFibEntry *fibEntry, CPIRouteEntry *route)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    fibEntry->fwdStrategy->addNexthop(fibEntry->fwdStrategy, route);
}

void
metisFibEntry_RemoveNexthopByRoute(MetisFibEntry *fibEntry, CPIRouteEntry *route)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    fibEntry->fwdStrategy->removeNexthop(fibEntry->fwdStrategy, route);
}

void
metisFibEntry_RemoveNexthopByConnectionId(MetisFibEntry *fibEntry, unsigned connectionId)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    CCNxName *ccnxName = metisTlvName_ToCCNxName(fibEntry->name);
    //this is a fake route, create only to deal with the strategyImpl interface
    CPIRouteEntry *cpiRouteEntry = cpiRouteEntry_Create(ccnxName, connectionId, NULL, 0, 0, NULL, 1);
    metisFibEntry_RemoveNexthopByRoute(fibEntry, cpiRouteEntry);
    cpiRouteEntry_Destroy(&cpiRouteEntry);
}


size_t
metisFibEntry_NexthopCount(const MetisFibEntry *fibEntry)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    return fibEntry->fwdStrategy->countNexthops(fibEntry->fwdStrategy);
}

const MetisNumberSet *
metisFibEntry_GetNexthops(const MetisFibEntry *fibEntry)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    return fibEntry->fwdStrategy->returnNexthops(fibEntry->fwdStrategy);
}

const MetisNumberSet *
metisFibEntry_GetNexthopsFromForwardingStrategy(const MetisFibEntry *fibEntry, const MetisMessage *interestMessage)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    return fibEntry->fwdStrategy->lookupNexthop(fibEntry->fwdStrategy, interestMessage);
}

void
metisFibEntry_ReceiveObjectMessage(const MetisFibEntry *fibEntry, const MetisNumberSet *egressId, const MetisMessage *objectMessage, MetisTicks rtt)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    fibEntry->fwdStrategy->receiveObject(fibEntry->fwdStrategy, egressId, objectMessage, rtt);
}

void
metisFibEntry_OnTimeout(const MetisFibEntry *fibEntry, const MetisNumberSet *egressId)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    fibEntry->fwdStrategy->onTimeout(fibEntry->fwdStrategy, egressId);
}

MetisTlvName *
metisFibEntry_GetPrefix(const MetisFibEntry *fibEntry)
{
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    return metisTlvName_Acquire(fibEntry->name);
}

const char *
metisFibEntry_GetFwdStrategyType(const MetisFibEntry *fibEntry)
{
    return fibEntry->fwdStrategy->getStrategy(fibEntry->fwdStrategy);
}

MetisStrategyImpl *
metisFibEntry_GetFwdStrategy(const MetisFibEntry *fibEntry)
{
    return fibEntry->fwdStrategy;
}
