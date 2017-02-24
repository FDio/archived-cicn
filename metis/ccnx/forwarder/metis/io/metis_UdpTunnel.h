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
 * @file metis_UdpTunnel.h
 * @brief Establish a tunnel to a remote system
 *
 * Creates a "udp tunnel" to a remote system.  There must already be a local UDP listener for the
 * local side of the connection.  Because UDP is connectionless and we do not have a link protocol,
 * the udp tunnel will go in the connection table immediately in the "up" state.
 *
 */

#ifndef Metis_metis_UdpTunnel_h
#define Metis_metis_UdpTunnel_h

#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/io/metis_Listener.h>
#include <ccnx/forwarder/metis/io/metis_IoOperations.h>

/**
 * Establishes a connection to a remote system over UDP
 *
 * The remoteAddress must be of the same type (i.e. v4 or v6) as the localListener.
 *
 * The connection will go in the table immediately, and will be in the "up" state.
 *
 *
 * @param [in] metis An allocated MetisForwarder
 * @param [in] localListener The local receiver for UDP messages
 * @param [in] remote Address the remote IP address for the connection, must include a destination port.
 *
 * @retval non-null An allocated Io Operations structure for the connection
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisIoOperations *metisUdpTunnel_CreateOnListener(MetisForwarder *metis, MetisListenerOps *localListener, const CPIAddress *remoteAddress);

/**
 * Establishes a connection to a remote system over UDP
 *
 * The remoteAddress must be of the same type (i.e. v4 or v6) as the localAddress.  There must be an existing UDP listener
 * on the local address.  If either of these are not true, will return NULL.
 *
 * The connection will go in the table immediately, and will be in the "up" state.
 *
 * This function will lookup the appropraite listener, then use metisUdpTunnel_CreateOnListener().
 *
 * @param [in] metis An allocated MetisForwarder
 * @param [in] localAddress The local IP address and port to use for the connection
 * @param [in] remote Address the remote IP address for the connection, must include a destination port.
 *
 * @retval non-null An allocated Io Operations structure for the connection
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisIoOperations *metisUdpTunnel_Create(MetisForwarder *metis, const CPIAddress *localAddress, const CPIAddress *remoteAddress);

#endif // Metis_metis_UdpTunnel_h
