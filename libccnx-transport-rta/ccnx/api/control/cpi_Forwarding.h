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
 * @file cpi_Forwarding.h
 * @brief CPI Forwarding
 *
 */
#ifndef libccnx_cpi_ManageForwarding_h
#define libccnx_cpi_ManageForwarding_h

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/api/control/cpi_ControlMessage.h>
#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_NameRouteType.h>
#include <ccnx/api/control/cpi_RouteEntry.h>
#include <ccnx/api/control/cpi_RouteEntryList.h>
#include <ccnx/api/control/cpi_ForwardingStrategy.h>

/**
 * Generate a request for a list all routes
 *
 *   The transport should resond with a CPI Response message.
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *cpiForwarding_CreateRouteListRequest();

/**
 * Parse a control message into a list of interfaces
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
CPIRouteEntryList *cpiForwarding_RouteListFromControlMessage(CCNxControl *response);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 * @see <#references#>
 */
PARCJSON *cpiForwarding_CreateAddRouteRequest(const CPIRouteEntry *route);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 * @see <#references#>
 */
PARCJSON *cpiForwarding_CreateRemoveRouteRequest(const CPIRouteEntry *route);

/**
 * Simplified form of <code>cpiForwarding_AddRoute</code> to add a route to the current transport
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
PARCJSON *cpiForwarding_AddRouteToSelf(const CCNxName *prefix);

/**
 * Simplified form of <code>cpiForwarding_RemoveRoute</code> to remove a route to the current transport
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
PARCJSON *cpiForwarding_RemoveRouteToSelf(const CCNxName *prefix);

/**
 * Creates a control message representing the route
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
PARCJSON *cpiForwarding_AddRoute(const CPIRouteEntry *route);

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
PARCJSON *cpiForwarding_RemoveRoute(const CPIRouteEntry *route);

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
CPIRouteEntry *cpiForwarding_RouteFromControlMessage(CCNxControl *control);

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
const char *cpiForwarding_AddRouteJsonTag();

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
const char *cpiForwarding_RemoveRouteJsonTag();

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
const char *cpiForwarding_RouteListJsonTag();

PARCJSON *cpiForwarding_CreateSetStrategyRequest();
const char *cpiForwarding_SetStrategyJsonTag();
CPIForwardingStrategy *cpiForwarding_ForwardingStrategyFromControlMessage(CCNxControl *control);
#endif // libccnx_cpi_ManageForwarding_h
