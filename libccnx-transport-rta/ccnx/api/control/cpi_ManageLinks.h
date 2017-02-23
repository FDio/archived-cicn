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
 * @file cpi_ManageLinks.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef libccnx_cpi_ManageLinks_h
#define libccnx_cpi_ManageLinks_h

#include <ccnx/api/control/cpi_ConnectionList.h>
#include <ccnx/api/control/cpi_ControlMessage.h>
#include <ccnx/api/control/cpi_ManageWldr.h>
#include <ccnx/api/control/cpi_InterfaceType.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnel.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnelList.h>
#include <ccnx/api/control/cpi_InterfaceSet.h>


/**
 * Create a control message that asks the forwarder to return a list of connections
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
PARCJSON *cpiLinks_CreateConnectionListRequest();

/**
 * Returns a native object from a control message of connections
 *
 *   The decoder of the response to <code>cpiLinks_CreateConnectionListRequest()</code>
 *
 * @param <#param1#>
 * @return An allocated object, you must destroy it.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnectionList *cpiLinks_ConnectionListFromControlMessage(CCNxControl *response);

/**
 * Generate a request for a list of all interfaces
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
PARCJSON *cpiLinks_CreateInterfaceListRequest(void);

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
CPIInterfaceSet *cpiLinks_InterfacesFromControlMessage(CCNxControl *response);

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
PARCJSON *cpiLinks_CreateIPTunnel(const CPIInterfaceIPTunnel *iptun);

PARCJSON *cpiLinks_RemoveIPTunnel(const CPIInterfaceIPTunnel *iptun);

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
CPIInterfaceIPTunnel *cpiLinks_CreateIPTunnelFromControlMessage(CCNxControl *response);

/**
 * Set an interface to UP or DOWN
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
CCNxControl *cpiLinks_SetInterfaceState(unsigned ifidx, CPIInterfaceStateType state);

/**
 * Removes an interface
 *
 *   If it is a virtual interface created through the ControlPlaneInterface, it
 *   is complete removed.
 *
 *   Trying to remove a physical interface will result in it going down, but it
 *   might not be removed from the system.
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxControl *cpiLinks_RemoveInterface(unsigned ifidx);

/**
 * The key name for an InterfaceList branch
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return Do not free it.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiLinks_InterfaceListJsonTag();

/**
 * The string tag used in JSON for a Create Tunnel request
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return Do not free it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiLinks_CreateTunnelJsonTag();

/**
 * The string tag used in JSON for a Remove Tunnel request
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return Do not free it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiLinks_RemoveTunnelJsonTag();

/**
 * The string tag used in JSON for a Connection List request
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return Do not free it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiLinks_ConnectionListJsonTag();

/**
 * The string tag used in JSON to add an Ethernet connection
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return Do not free it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiLinks_AddEtherConnectionJasonTag();

PARCJSON *cpiLinks_CreateSetWldrRequest(const CPIManageWldr *cpiWldr);

CPIManageWldr *cpiLinks_ManageWldrFromControlMessage(CCNxControl *control);

const char *cpiLinks_SetWldrJsonTag();

#endif // libccnx_cpi_ManageLinks_h
