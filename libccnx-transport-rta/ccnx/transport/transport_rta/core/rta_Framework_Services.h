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
 * @file rta_Framework_Services.h
 * @brief Miscellaneous services offered by the Framework for components and connectors
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_rta_Framework_Services_h
#define Libccnx_rta_Framework_Services_h

#include "rta_Framework.h"

#include <parc/algol/parc_EventScheduler.h>

// ===================================

typedef uint64_t ticks;
#define TICK_CMP(a, b) ((int64_t) a - (int64_t) b)

/**
 * <#One Line Description#>
 *
 * If a component wants to use the event scheduler to manage sockets, it
 * can get a reference to the event base to manage those things
 *
 * @param [in] framework <#description#>
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
PARCEventScheduler *rtaFramework_GetEventScheduler(RtaFramework *framework);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] framework <#description#>
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
unsigned rtaFramework_GetNextConnectionId(RtaFramework *framework);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] framework <#description#>
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
ticks rtaFramework_GetTicks(RtaFramework *framework);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] tick <#description#>
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
extern uint64_t rtaFramework_TicksToUsec(ticks tick);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] usec <#description#>
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
extern ticks rtaFramework_UsecToTicks(unsigned usec);
#endif // Libccnx_rta_Framework_Services_h
