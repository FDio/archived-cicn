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
 * @file metis_TcpTunnel.h
 * @brief Creates a TCP Tunnel to a remote address
 *
 * The connection will be established in "connecting" state and once the remote accepts, it will
 * be promoted to "up" state.
 *
 */

#ifndef Metis_metis_TcpTunnel_h
#define Metis_metis_TcpTunnel_h

#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/io/metis_IoOperations.h>

/**
 * @function metisTcpTunnel_Create
 * @abstract Creates a TCP tunnel to a remote system.
 * @discussion
 *   The two addresses must be the same type (i.e. both INET or INET6) and cannot point to the same system.
 *
 *   The tunnel will look just like an in-bound connection after its built.  It exposes the standard
 *   MetisIoOperations so it can be put in the MetisConnectionTable.
 *
 *   The connection will go in the table immediately, but will be in the "down" state until the
 *   connection is established.
 *
 *
 * @param <#param1#>
 * @return The I/O ops for the tunnel.
 */
MetisIoOperations *metisTcpTunnel_Create(MetisForwarder *metis, const CPIAddress *localAddress, const CPIAddress *remoteAddress);
#endif // Metis_metis_TcpTunnel_h
