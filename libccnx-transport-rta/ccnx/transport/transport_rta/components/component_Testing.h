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
 * @file component_Testing.h
 * @brief A component that takes no actions, not even reads.
 *
 * This is useful to put in the stack around a component under test to
 * isolate it from the system and then intercept the queues.
 *
 * This component may be used as both TESTING_UPPER and TESTING_LOWER
 * to surround another component:
 *
 * { SYSTEM : COMPONENTS : [TESTING_UPPER, component under test, TESTING_LOWER] }
 *
 * In your test code, you would then have something like this to operate in
 * the "down" direction:
 *    PARCEventQueue *upper = rtaProtocolStack_GetPutQueue(stack, TESTING_UPPER, RTA_DOWN);
 *    PARCEventQueue *rea   = rtaProtocolStack_GetPutQueue(stack, component under test, RTA_UP);
 *    PARCEventQueue *lower = rtaProtocolStack_GetPutQueue(stack, TESTING_LOWER, RTA_UP);
 *
 */
#ifndef Libccnx_component_Testing_h
#define Libccnx_component_Testing_h

#include <ccnx/transport/transport_rta/core/rta_Component.h>

extern RtaComponentOperations testing_null_ops;
#endif // Libccnx_component_Testing_h
