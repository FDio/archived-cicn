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
#include <stdlib.h>
#include <string.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <ccnx/transport/transport_rta/core/rta_ComponentStats.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>

struct rta_component_stats {
    RtaProtocolStack *stack;
    RtaComponents type;
    uint64_t stats[STATS_LAST];
};

char *
rtaComponentStatType_ToString(RtaComponentStatType statsType)
{
    switch (statsType) {
        case STATS_OPENS:
            return "opens";

        case STATS_CLOSES:
            return "closes";

        case STATS_UPCALL_IN:
            return "upcall_in";

        case STATS_UPCALL_OUT:
            return "upcall_out";

        case STATS_DOWNCALL_IN:
            return "downcall_in";

        case STATS_DOWNCALL_OUT:
            return "downcall_out";

        default:
            trapIllegalValue(statsType, "Unknown RtaComponentStatType %d", statsType);
    }
}

/**
 * Its ok to call with null stack.  that just means when we increment, we won't
 * also increment stack-wide stats
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaComponentStats *
rtaComponentStats_Create(RtaProtocolStack *stack, RtaComponents componentType)
{
    RtaComponentStats *stats = parcMemory_AllocateAndClear(sizeof(RtaComponentStats));
    assertNotNull(stats, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(RtaComponentStats));
    assertTrue(componentType < LAST_COMPONENT, "invalid type %d\n", componentType);

    stats->stack = stack;
    stats->type = componentType;
    return stats;
}

/* Increment and return incremented value */
uint64_t
rtaComponentStats_Increment(RtaComponentStats *stats, RtaComponentStatType statsType)
{
    assertNotNull(stats, "%s dereferenced a null stats pointer\n", __func__);
    assertFalse(statsType >= STATS_LAST, "%s incorrect stat type %d\n", __func__, statsType);
    stats->stats[statsType]++;

    if (stats->stack != NULL) {
        RtaComponentStats *stack_stats = rtaProtocolStack_GetStats(stats->stack, stats->type);
        // if stack is not null, then we must get stats from it
        assertNotNull(stack_stats, "%s got null stack stats\n", __func__);
        stack_stats->stats[statsType]++;
    }

    return stats->stats[statsType];
}

/* Return value */
uint64_t
rtaComponentStats_Get(RtaComponentStats *stats, RtaComponentStatType statsType)
{
    assertNotNull(stats, "dereferenced a null stats pointer\n");
    assertFalse(statsType >= STATS_LAST, "incorrect stat statsType %d\n", statsType);
    return stats->stats[statsType];
}

/* dump the stats to the given output */
void
rtaComponentStats_Dump(RtaComponentStats *stats, FILE *output)
{
}

void
rtaComponentStats_Destroy(RtaComponentStats **statsPtr)
{
    RtaComponentStats *stats;
    assertNotNull(statsPtr, "%s got null stats pointer\n", __func__);

    stats = *statsPtr;
    assertNotNull(stats, "%s dereferenced a null stats pointer\n", __func__);

    memset(stats, 0, sizeof(RtaComponentStats));
    parcMemory_Deallocate((void **) &stats);
}
