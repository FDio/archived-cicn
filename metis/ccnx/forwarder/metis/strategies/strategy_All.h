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
 * Forward to all nexthops
 * THIS STRATEGY IS DEPRECATED
 */

#ifndef Metis_strategy_All_h
#define Metis_strategy_All_h

#include <ccnx/forwarder/metis/strategies/metis_Strategy.h>

MetisStrategyImpl *metisStrategyAll_Create();
#endif // Metis_strategy_All_h
