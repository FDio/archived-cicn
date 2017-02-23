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
 * @header metis_System.h
 * @abstract System-level properties
 * @discussion
 *     <#Discussion#>
 *
 */

#ifndef Metis_metis_System_h
#define Metis_metis_System_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/api/control/cpi_InterfaceSet.h>

/**
 * @function metisSystem_Interfaces
 * @abstract The system network interfaces
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
CPIInterfaceSet *metisSystem_Interfaces(MetisForwarder *metis);

/**
 * Returns the MTU of the named interface
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] metis An allocated forwarder
 * @param [in] interfaceName The system interface name, e.g. "eth0"
 *
 * @return 0 Interface does not exist
 * @return positive the MTU the kernel reports
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
unsigned metisSystem_InterfaceMtu(MetisForwarder *metis, const char *interfaceName);

/**
 * Returns the LINK address of the specified interface
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] metis An allocated forwarder
 * @param [in] interfaceName The system interface name, e.g. "eth0"
 *
 * @retval non-null The MAC address of the interface
 * @retval null The interface does not exist
 *
 * Example:
 * @code
 * {
 *     CPIAddress *linkAddress = metisSystem_GetMacAddressByName(metis, "en0");
 * }
 * @endcode
 */
CPIAddress *metisSystem_GetMacAddressByName(MetisForwarder *metis, const char *interfaceName);
#endif
