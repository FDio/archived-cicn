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
 * @file metis_StrategyImpl.h
 * @brief Defines the function structure for a Strategy implementation
 *
 * <#Detailed Description#>
 *
 */

/**
 * A dispatch structure for a concrete implementation of a forwarding strategy.
 */

#ifndef Metis_metis_StrategyImpl_h
#define Metis_metis_StrategyImpl_h

#include <ccnx/forwarder/metis/core/metis_NumberSet.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/api/control/cpi_RouteEntry.h>

struct metis_strategy_impl;
typedef struct metis_strategy_impl MetisStrategyImpl;

extern const char *FWD_STRATEGY_LOADBALANCER;
extern const char *FWD_STRATEGY_RANDOM;
extern const char *FWD_STRATEGY_RANDOM_PER_DASH_SEGMENT;
extern const char *FWD_STRATEGY_LOADBALANCER_WITH_DELAY;

/**
 * @typedef MetisStrategyImpl
 * @abstract Forwarding strategy implementation
 * @constant receiveObject is called when we receive an object and have a measured round trip time.  This
 *           allows a strategy to update its performance data.
 * @constant lookupNexthop Find the set of nexthops to use for the Interest.
 *           May be empty, should not be NULL.  Must be destroyed.
 * @constant addNexthop Add a nexthop to the list of available nexthops with a routing protocol-specific cost.
 * @constant destroy cleans up the strategy, freeing all memory and state.  A strategy is reference counted,
 *           so the final destruction only happens after the last reference is released.
 * @discussion <#Discussion#>
 */
struct metis_strategy_impl {
    void *context;
    void (*receiveObject)(MetisStrategyImpl *strategy, const MetisNumberSet *egressId, const MetisMessage *objectMessage, MetisTicks rtt);
    void (*onTimeout)(MetisStrategyImpl *strategy, const MetisNumberSet *egressId);
    MetisNumberSet * (*lookupNexthop)(MetisStrategyImpl * strategy, const MetisMessage * interestMessage);
    MetisNumberSet * (*returnNexthops)(MetisStrategyImpl * strategy);
    unsigned (*countNexthops)(MetisStrategyImpl *strategy);
    void (*addNexthop)(MetisStrategyImpl *strategy, CPIRouteEntry *route);
    void (*removeNexthop)(MetisStrategyImpl *strategy, CPIRouteEntry *route);
    void (*destroy)(MetisStrategyImpl **strategyPtr);
    const char * (*getStrategy)(MetisStrategyImpl * strategy);
};
#endif // Metis_metis_StrategyImpl_h
