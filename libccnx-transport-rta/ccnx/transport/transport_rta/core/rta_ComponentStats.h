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
 * @file rta_ComponentStats.h
 * @brief <#Brief Description#>
 *
 * Statistics are PER CONNECTION PER COMPONENT.  Therefore, a component would call
 * rtaConnection_GetStats(conn, component) to access its stats.  Each component must
 * create its stats counter in _Open and free it in _Close.
 *
 * Each ProtocolStack has a PER STACK PER COMPONENT set of statistics too.  When a
 * component creates its stats in _Open, it passes a pointer to its stack, so when
 * _Increment is called, it will increment both the component's stats and the stack's
 * stats.
 *
 * For example:
 *
 *    protocolStack_Init() creates stack-wide stats for each component type.
 *    componentX_Open(stack) creates per-connection stats for that component with
 *      a reference to stack using stats_Create(stack, component_type)
 *    componentX_Y(conn) performs some per-connection activity.  It would call
 *      stats_Increment(rtaConnection_GetStats(conn), component_type, stat_type).
 *      That would increment the per-connection per-component stat and if the stack
 *      was not null, would increment the identical component_type, stat_type
 *      stat in the per-stack per-component counters.
 *
 *
 *
 */
#ifndef Libccnx_rta_ComponentStats
#define Libccnx_rta_ComponentStats

#include <ccnx/transport/transport_rta/core/components.h>

struct protocol_stack;

struct rta_component_stats;
/**
 *
 * @see stats_Create
 */
typedef struct rta_component_stats RtaComponentStats;

typedef enum {
    STATS_OPENS,
    STATS_CLOSES,
    STATS_UPCALL_IN,
    STATS_UPCALL_OUT,
    STATS_DOWNCALL_IN,
    STATS_DOWNCALL_OUT,
    STATS_LAST              // must be last
} RtaComponentStatType;

/**
 * Create a stats component
 *
 * If the optional stack is specified, its statistics will be incremented whenever this
 * stats object is incremented.  Otherwise, it may be NULL.
 *
 * @param [in] stack Optional protocol stack
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
RtaComponentStats *rtaComponentStats_Create(struct protocol_stack *stack, RtaComponents componentType);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
char *rtaComponentStatType_ToString(RtaComponentStatType statType);

/**
 * Increment and return incremented value
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
uint64_t rtaComponentStats_Increment(RtaComponentStats *stats, RtaComponentStatType statType);

/**
 * Return value
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
uint64_t rtaComponentStats_Get(RtaComponentStats *stats, RtaComponentStatType statType);

/**
 * dump the stats to the given output
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void rtaComponentStats_Dump(RtaComponentStats *stats, FILE *output);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void rtaComponentStats_Destroy(RtaComponentStats **statsPtr);
#endif
