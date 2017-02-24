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
 * @file cpi_Listener.h
 * @brief Represents a protocol listener
 *
 * A protocol listener is the tuple (protocol, local address), where protocol is one
 * of TCP, UDP, Ether, etc., local address is a CPI address.  For IP protocols,
 * local address is an (ip address, port) pair.  For Ethernet, it is a (mac address, ethertype) pair.
 *
 */

#ifndef CCNx_Control_API_cpi_Listener_h
#define CCNx_Control_API_cpi_Listener_h

struct cpi_listener;
typedef struct cpi_listener CPIListener;

#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_ControlMessage.h>

/**
 * Creates a CPIListener object
 *
 * The symbolic name represents this listener and may be used by other commands.  It must be
 * unique, otherwise the command will fail when sent to the forwarder.
 *
 * @param [in] interfaceName The name of the local interface
 * @param [in] ethertype The ethertype to use (host byte order)
 * @param [in] symbolic The user-defined symbolic name
 *
 * @return non-null An Allocated object
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CPIListener *listener = cpiListener_CreateEther("eth0", 0x0801, "puppy");
 *     cpiListener_Release(&listener);
 * }
 * @endcode
 */
CPIListener *cpiListener_CreateEther(const char *interfaceName, uint16_t ethertype, const char *symbolic);

/**
 * Creates a CPIListener object
 *
 * The symbolic name represents this connection and may be used by other commands.  It must be
 * unique, otherwise the command will fail when sent to the forwarder.  IPv4 and IPv6 are differentiated
 * based on the address.
 *
 * @param [in] type The local address encapsulation type
 * @param [in] localAddress The local address to bind to
 * @param [in] symbolic The user-defined symbolic name
 *
 * @return non-null An Allocated object
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_in sin;
 *     memset(&sin, 0, sizeof(sin));
 *     sin.sin_family = AF_INET;
 *     sin.sin_port = htons(port);
 *     inet_aton(addressString, &sin.sin_addr);
 *     CPIAddress *address = cpiAddress_CreateFromInet(&sin);
 *     CPIListener *listener = cpiListener_CreateIP(IPTUN_UDP, address, "fido");
 *
 *     cpiAddress_Destroy(&address);
 *     cpiListener_Release(&listener);
 * }
 * @endcode
 */
CPIListener *cpiListener_CreateIP(CPIInterfaceIPTunnelType type, CPIAddress *localAddress, const char *symbolic);

/**
 * Releases a reference count to the object
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in,out] etherConnPtr A pointer to an etherConn object, will be null'd.
 *
 * Example:
 * @code
 * {
 *     CPIListener *listener = cpiListener_CreateEther("eth0", 0x0801, "puppy");
 *     cpiListener_Release(&listener);
 * }
 * @endcode
 */
void cpiListener_Release(CPIListener **etherConnPtr);

/**
 * Determine if two CPIListener instances are equal.
 *
 * Two CPIListener instances are equal if, and only if,
 * they are either both null or both non-null and compare
 * as equal field-for-field.
 *
 * The interface name is case sensitive, so "ETH0" is not the same as "eth0".
 *
 *
 * The following equivalence relations on non-null `CPIListener` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIListener_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiListener_Equals(x, y)` must return true if and only if
 *        `cpiListener_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiListener_Equals(x, y)` returns true and
 *        `cpiListener_Equals(y, z)` returns true,
 *        then  `cpiListener_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiListener_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiListener_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIListener` instance.
 * @param b A pointer to a `CPIListener` instance.
 * @return true if the two `CPIListener` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIListener *a = cpiListener_Create();
 *    CPIListener *b = cpiListener_Create();
 *
 *    if (cpiListener_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */

bool cpiListener_Equals(const CPIListener *a, const CPIListener *b);

/**
 * Creates a control message to add the listener
 *
 * An add message indicates to the forwarder that it should add the listener.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null a CPI control message
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CPIListener *listener = cpiListener_CreateEther("eth0", 0x0801, "puppy");
 *     CCNxControl *control = cpiListener_CreateAddMessage(listener);
 *     cpiListener_Release(&listener);
 *
 *     ccnxPortal_Send(portal, control, CCNxStackTimeout_Never);
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 */
CCNxControl *cpiListener_CreateAddMessage(const CPIListener *etherConn);

/**
 * Creates a control message to remove the connection
 *
 * A remove message indicates to the forwarder that it should remove the listener.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null a CPI control message
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CPIListener *listener = cpiListener_CreateEther("eth0", 0x0801, "puppy");
 *     CCNxControl *control = cpiListener_CreateRemoveMessage(listener);
 *     cpiListener_Release(&listener);
 *
 *     ccnxPortal_Send(portal, control, CCNxStackTimeout_Never);
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 */
CCNxControl *cpiListener_CreateRemoveMessage(const CPIListener *etherConn);

/**
 * Checks if the control message is an Add command
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] control An allocated CCNxControl message
 *
 * @return true Message is an Add command for a Listener
 * @return false Message is not an Add command for a Listener
 *
 * Example:
 * @code
 * {
 *     CCNxMetaMessage *msg = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);
 *     if (ccnxMetaMessage_IsControl(msg)) {
 *        CCNxControl *control = ccnxMetaMessage_GetControl(msg);
 *        if (cpiListener_IsAddMessage(control)) {
 *           // process an add listener request
 *        }
 *     }
 *     ccnxMetaMessage_Release(&msg);
 * }
 * @endcode
 */
bool cpiListener_IsAddMessage(const CCNxControl *control);

/**
 * Checks if the message is a Remove command
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] control A CCNx Control message
 *
 * @return true Message is an Remove command for a Listener
 * @return false Message is not Remove Add command for a Listener
 *
 * Example:
 * @code
 * {
 *     CCNxMetaMessage *msg = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);
 *     if (ccnxMetaMessage_IsControl(msg)) {
 *        CCNxControl *control = ccnxMetaMessage_GetControl(msg);
 *        if (cpiListener_IsRemoveMessage(control)) {
 *           // process a remove listener request
 *        }
 *     }
 *     ccnxMetaMessage_Release(&msg);
 * }
 * @endcode
 */
bool cpiListener_IsRemoveMessage(const CCNxControl *control);

/**
 * Creates an object from the control message
 *
 * The object does not carry any sense of Add or Remove, that is only part of the
 * Control message.  You must release the object when done.
 *
 * @param [in] control A CCNx Control message
 *
 * @return non-null An Allocated object
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CCNxMetaMessage *msg = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);
 *     if (ccnxMetaMessage_IsControl(msg)) {
 *        CCNxControl *control = ccnxMetaMessage_GetControl(msg);
 *        if (cpiListener_IsRemoveMessage(control)) {
 *           // process a remove listener request
 *           CPIListener *listener = cpiListener_FromControl(control);
 *           ...
 *           cpiListener_Release(&listener);
 *        }
 *     }
 *     ccnxMetaMessage_Release(&msg);
 * }
 * @endcode
 */
CPIListener *cpiListener_FromControl(const CCNxControl *control);

/**
 * Determines if the encapsulation is an Ethernet protocol
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] listener An allocated CPIListener
 *
 * @retval true It's Ethernet based
 * @retval false It's not
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool cpiListener_IsEtherEncap(const CPIListener *listener);

/**
 * Determines if the encapsulation is an IP-based protocol
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] listener An allocated CPIListener
 *
 * @retval true It's IP based
 * @retval false It's not
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool cpiListener_IsIPEncap(const CPIListener *listener);

/**
 * Returns the Ethertype for an Ethernet encapsulation
 *
 * The returned value is in host byte order
 *
 * @param [in] listener An allocated CPIListener
 *
 * @retval 0 Not Ethernet encapsulation
 * @retval positive The ethertype
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint16_t cpiListener_GetEtherType(const CPIListener *listener);

/**
 * Returns the interface name
 *
 * The caller should duplicate the string if it will be stored.
 *
 * @param [in] etherConn An allocated CPIListener
 *
 * @return non-null The interface name.
 * @return null An error (or not Ethernet encapsulation)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiListener_GetInterfaceName(const CPIListener *etherConn);

/**
 * Returns the symbolic name
 *
 * The caller should duplicate the string if it will be stored.
 *
 * @param [in] listener An allocated CPIListener
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiListener_GetSymbolicName(const CPIListener *listener);

/**
 * Returns the address (LINK mac address, or INET or INET6 ip address)
 *
 * Returns the local address to use for the listener.  The address type is
 * as appropriate for the encapsulation.
 *
 * @param [in] listener An allocated CPIListener
 *
 * @return non-null The peer's link address
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIAddress *cpiListener_GetAddress(const CPIListener *listener);

/**
 * For IP encapsulation, tests if the IP protocol is UDP
 *
 * Tests if the IP protocol is UDP.  If the protocol is not UDP or the encapsulation
 * is not IP, returns false.
 *
 * @param [in] listener An allocated CPIListener
 *
 * @retval true IP protocol is UDP
 * @retval false Not IP or not IP and UDP
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool cpiListener_IsProtocolUdp(const CPIListener *listener);

/**
 * For IP encapsulation, tests if the IP protocol is TCP
 *
 * Tests if the IP protocol is TCP.  If the protocol is not TCP or the encapsulation
 * is not IP, returns false.
 *
 * @param [in] listener An allocated CPIListener
 *
 * @retval true IP protocol is TCP
 * @retval false Not IP or not IP and TCP
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool cpiListener_IsProtocolTcp(const CPIListener *listener);

#endif // CCNx_Control_API_cpi_Listener_h
