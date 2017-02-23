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
 * @file cpi_NameRouteProtocolType.h
 * @brief Specifies the reason for or creator of a route (i.e. the protocol that created the route)
 *
 * A LOCAL route points to an application running on the localhost.
 *
 * A CONNECTED route exists because the described destination is directly connected to the localhost.  For
 * example, a route to a link local network name would be CONNECTED.
 *
 * A STATIC route is administratively created, such as via the "metis_control" program or via the
 * configuration file.
 *
 * An ACORN route is dynamically created by the ACRON routing protocol.
 *
 */
#ifndef libccnx_cpi_NameRouteProtocolType_h
#define libccnx_cpi_NameRouteProtocolType_h



/**
 * @typedef CPINameRouteProtocolType
 * @abstract Enumerates the protocol that created a route
 * @constant cpiNameRouteProtocolType_LOCAL An application running on the localhost
 * @constant cpiNameRouteProtocolType_CONNECTED A directly connected destination
 * @constant cpiNameRouteProtocolType_STATIC Administratively created
 * @constant cpiNameRouteProtocolType_ACORN The ACORN routing protocol
 */
typedef enum {
    cpiNameRouteProtocolType_LOCAL = 0,      // local face to app
    cpiNameRouteProtocolType_CONNECTED = 1,  // directly connected network
    cpiNameRouteProtocolType_STATIC = 2,     // administrative static route
    cpiNameRouteProtocolType_ACORN = 20
} CPINameRouteProtocolType;

/**
 * Return the string representation of the specified `CPINameRouteProtocolType`.
 *
 * The returned string does not need to be freed.
 *
 * @param [in] type The type to represent as a string.
 *
 * @return The string representation of the specified 'CPINameRouteProtocolType'.
 *
 * Example:
 * @code
 * {
 *     CPINameRouteProtocolType type = ROUTE_PROTO_CONNECTED;
 *
 *     char *name = cpiNameRouteProtocolType_ToString(type);
 *
 *     printf("NameRouteProtocolType is %s\n", name);
 * }
 * @endcode
 *
 * @see cpiNameRouteProtocolType_FromString
 */
const char *cpiNameRouteProtocolType_ToString(CPINameRouteProtocolType type);

/**
 * Given a string describing a `CPINameRouteProtocolType`, return the matching `CPINameRouteProtocolType`.
 *
 * If an invalid string is specified, the program will terminate with an IllegalValue exception.
 * Possible values are: "LOCAL", "CONNECTED", "STATIC", and "ACORN".
 *
 * @param [in] str A pointer to a string representation of the desired `CPINameRouteProtocolType`.
 *
 * @return The `CPINameRouteProtocolType` matching the specified string.
 *
 * Example:
 * @code
 * {
 *     CPINameRouteProtocolType type = cpiNameRouteProtocolType_FromString("STATIC");
 * }
 * @endcode
 *
 * @see cpiNameRouteProtocolType_ToString
 */
CPINameRouteProtocolType cpiNameRouteProtocolType_FromString(const char *str);
#endif // libccnx_cpi_NameRouteProtocolType_h
