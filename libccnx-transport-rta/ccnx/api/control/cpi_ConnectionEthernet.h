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
 * @file cpi_ConnectionEthernet.h
 * @brief Represents an ethernet connection
 *
 * An ethernet connection is a (local interface name, remote mac address, ethertype) tuple.  A unicast
 * connection, for example, could be ("em3", 3c:15:c2:e7:c5:ca, 0x0801).  The broadcast connection would
 * be ("em3", ff:ff:ff:ff:ff:ff, 0x0801).  You could also use group mac addresses.
 *
 * Creating an ethernet connetion in the forwarder sets up an entry in the connection table that
 * you an then attach routes to.  For example, you could add a route to /foo via the connection
 * ("em3", 3c:15:c2:e7:c5:ca, 0x0801), in which case an Interest would be unicast that way.  A route
 * to a broadcast or group address would broadcast the interest.
 *
 */

#ifndef CCNx_Control_API_cpi_ConnectionEthernet_h
#define CCNx_Control_API_cpi_ConnectionEthernet_h

struct cpi_connection_ethernet;
typedef struct cpi_connection_ethernet CPIConnectionEthernet;

#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_ControlMessage.h>

/**
 * Creates a CPIConnectionEthernet object
 *
 * The symbolic name represents this connection and may be used by other commands.  It must be
 * unique, otherwise the command will fail when sent to the forwarder.
 *
 * @param [in] interfaceName The name of the local interface
 * @param [in] peerLinkAddress The link layer address of the peer (stores a reference to it)
 * @param [in] ethertype The ethertype to use (host byte order)
 * @param [in] symbolic The user-defined symbolic name
 *
 * @return non-null An Allocated object
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnectionEthernet *cpiConnectionEthernet_Create(const char *interfaceName, CPIAddress *peerLinkAddress, uint16_t ethertype, const char *symbolic);

/**
 * Releases a reference count to the object
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in,out] etherConnPtr A pointer to an etherConn object, will be null'd.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void cpiConnectionEthernet_Release(CPIConnectionEthernet **etherConnPtr);

/**
 * Determine if two CPIConnectionEthernet instances are equal.
 *
 * Two CPIConnectionEthernet instances are equal if, and only if,
 * they are either both null or both non-null and compare
 * as equal field-for-field over (interfaceName, peerLinkAddress, ethertype, symbolic).
 *
 * The interface name is case sensitive, so "ETH0" is not the same as "eth0".
 *
 *
 * The following equivalence relations on non-null `CPIConnectionEthernet` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIConnectionEthernet_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiConnectionEthernet_Equals(x, y)` must return true if and only if
 *        `cpiConnectionEthernet_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiConnectionEthernet_Equals(x, y)` returns true and
 *        `cpiConnectionEthernet_Equals(y, z)` returns true,
 *        then  `cpiConnectionEthernet_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiConnectionEthernet_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiConnectionEthernet_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIConnectionEthernet` instance.
 * @param b A pointer to a `CPIConnectionEthernet` instance.
 * @return true if the two `CPIConnectionEthernet` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIConnectionEthernet *a = cpiConnectionEthernet_Create();
 *    CPIConnectionEthernet *b = cpiConnectionEthernet_Create();
 *
 *    if (cpiConnectionEthernet_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */

bool cpiConnectionEthernet_Equals(const CPIConnectionEthernet *a, const CPIConnectionEthernet *b);

/**
 * Creates a control message to add the connection
 *
 * An add message indicates to the forwarder that it should add the corresponding
 * Ethernet connection.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null a CPI control message
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxControl *cpiConnectionEthernet_CreateAddMessage(const CPIConnectionEthernet *etherConn);

/**
 * Creates a control message to remove the connection
 *
 * A remove message indicates to the forwarder that it should remove the corresponding
 * Ethernet connection.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null a CPI control message
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxControl *cpiConnectionEthernet_CreateRemoveMessage(const CPIConnectionEthernet *etherConn);

/**
 * Checks if the control message is an Add command
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return true Message is an Add command for a ConnectionEthernet
 * @return false Message is not an Add command for a ConnectionEthernet
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool cpiConnectionEthernet_IsAddMessage(const CCNxControl *control);

/**
 * Checks if the message is a Remove command
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] control A CCNx Control message
 *
 * @return true Message is an Remove command for a ConnectionEthernet
 * @return false Message is not Remove Add command for a ConnectionEthernet
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool cpiConnectionEthernet_IsRemoveMessage(const CCNxControl *control);

/**
 * Creates an object from the control message
 *
 * The object does not carry any sense of Add or Remove, that is only part of the
 * Control message.
 *
 * @param [in] control A CCNx Control message
 *
 * @return non-null An Allocated object
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnectionEthernet *cpiConnectionEthernet_FromControl(const CCNxControl *control);

/**
 * Returns the interface name
 *
 * The caller should duplicate the string if it will be stored.
 *
 * @param [in] etherConn An allocated CPIConnectionEthernet
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiConnectionEthernet_GetInterfaceName(const CPIConnectionEthernet *etherConn);

/**
 * Returns the symbolic name
 *
 * The caller should duplicate the string if it will be stored.
 *
 * @param [in] etherConn An allocated CPIConnectionEthernet
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiConnectionEthernet_GetSymbolicName(const CPIConnectionEthernet *etherConn);

/**
 * Returns the peer link address
 *
 * Returns the peer's link address (e.g. 48-bit MAC address).  The caller should
 * acquire its own reference if he address will be stored externally to the
 * CPIConnectionEthernet.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null The peer's link address
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIAddress *cpiConnectionEthernet_GetPeerLinkAddress(const CPIConnectionEthernet *etherConn);

/**
 * Returns the ethertype to use
 *
 * The ethertype will be in host byte order.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint16_t cpiConnectionEthernet_GetEthertype(const CPIConnectionEthernet *etherConn);
#endif // CCNx_Control_API_cpi_ConnectionEthernet_h
