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

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <sys/queue.h>

#include "rta_Framework.h"
#include "rta_Framework_private.h"
#include "rta_Framework_Services.h"

ticks
rtaFramework_GetTicks(RtaFramework *framework)
{
    assertNotNull(framework, "Parameter framework cannot be null");
    return framework->clock_ticks;
}

uint64_t
rtaFramework_TicksToUsec(ticks tick)
{
    return FC_USEC_PER_TICK * tick;
}

ticks
rtaFramework_UsecToTicks(unsigned usec)
{
    return MSEC_TO_TICKS(usec / 1000);
}
