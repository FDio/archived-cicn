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
 * A forwarding strategy for a namespace
 */

#ifndef Metis_metis_Strategy_h
#define Metis_metis_Strategy_h

#include <ccnx/forwarder/metis/strategies/metis_StrategyImpl.h>

struct metis_strategy;
typedef struct metis_strategy MetisStrategy;

MetisStrategy *metisStrategy_Create(MetisStrategyImpl *impl);

MetisNumberSet *metisStrategy_LookupNexthops(const CCNxName *name, uint32_t ingressInterface);
#endif // Metis_metis_Strategy_h
