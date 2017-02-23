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
 * @file cpi_InterfaceIPTunnel.h
 * @brief Represents a point-to-point tunnel over IP.
 *
 * The carries can be UDP, TCP, or GRE
 *
 * We use InterfaceGeneric to back this type.  We always use 2 addresses in the address list.
 * Address 0 is the source and address 1 is the destination.
 *
 */
#ifndef libccnx_cpi_InterfaceIPTunnel_h
#define libccnx_cpi_InterfaceIPTunnel_h

#include <ccnx/api/control/cpi_InterfaceType.h>
#include <ccnx/api/control/cpi_Address.h>

#include <parc/algol/parc_JSON.h>

struct cpi_interface_iptun;
/**
 *
 * @see cpiInterfaceIPTunnel_Create
 */
typedef struct cpi_interface_iptun CPIInterfaceIPTunnel;

typedef enum {
    IPTUN_UDP,
    IPTUN_TCP,
    IPTUN_GRE
} CPIInterfaceIPTunnelType;

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] type A CPIInterfaceIPTunnelType value
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
const char *cpiInterfaceIPTunnel_TypeToString(CPIInterfaceIPTunnelType type);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] str A nul-terminated C string.
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
CPIInterfaceIPTunnelType cpiInterfaceIPTunnel_TypeFromString(const char *str);

/**
 * Creates a representation of an IP tunnel
 *
 * The symblic name will be used in the future to refer to this tunnel.  It must be unique or the forwarder will reject the command.
 *
 * @param [in] ifidx The interface index of the tunnel (may be 0 if not known)
 * @param [in] source The local address and optional port
 * @param [in] destination The remote address and port
 * @param [in] tunnelType The encapsulation protocol
 * @param [in] symbolic The symbolic name to refer to this tunnel (e.g. 'tun2')
 *
 * @return non-null An allocated CPIInterfaceIPTunnel
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
CPIInterfaceIPTunnel *cpiInterfaceIPTunnel_Create(unsigned ifidx, CPIAddress *source, CPIAddress *destination, CPIInterfaceIPTunnelType tunnelType, const char *symbolic);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
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
CPIInterfaceIPTunnel *cpiInterfaceIPTunnel_Copy(const CPIInterfaceIPTunnel *ipTunnel);
CPIInterfaceIPTunnel *cpiInterfaceIPTunnel_Acquire(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
 * @param [in] state A CPIInterfaceStateType value.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void cpiInterfaceIPTunnel_SetState(CPIInterfaceIPTunnel *ipTunnel, CPIInterfaceStateType state);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnelPtr A pointer to a pointer to a valid CPIInterfaceIPTunnel instance.
 *
 * @see <#references#>
 */
void cpiInterfaceIPTunnel_Release(CPIInterfaceIPTunnel **ipTunnelPtr);

/**
 * Returns the symbolic name of the tunnel
 *
 * The caller should make a copy of the string if it will be stored.
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
 *
 * @return non-null The symbolic name
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
const char *cpiInterfaceIPTunnel_GetSymbolicName(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
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
unsigned cpiInterfaceIPTunnel_GetIndex(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
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
const CPIAddress *cpiInterfaceIPTunnel_GetSourceAddress(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
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
const CPIAddress *cpiInterfaceIPTunnel_GetDestinationAddress(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
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
CPIInterfaceIPTunnelType cpiInterfaceIPTunnel_GetTunnelType(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
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
CPIInterfaceStateType cpiInterfaceIPTunnel_GetState(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * Determine if two CPIInterfaceIPTunnel instances are equal.
 *
 *
 * The following equivalence relations on non-null `CPIInterfaceIPTunnel` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIInterfaceIPTunnel_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiInterfaceIPTunnel_Equals(x, y)` must return true if and only if
 *        `cpiInterfaceIPTunnel_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiInterfaceIPTunnel_Equals(x, y)` returns true and
 *        `cpiInterfaceIPTunnel_Equals(y, z)` returns true,
 *        then  `cpiInterfaceIPTunnel_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiInterfaceIPTunnel_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiInterfaceIPTunnel_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIInterfaceIPTunnel` instance.
 * @param b A pointer to a `CPIInterfaceIPTunnel` instance.
 * @return true if the two `CPIInterfaceIPTunnel` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIInterfaceIPTunnel *a = cpiInterfaceIPTunnel_Create();
 *    CPIInterfaceIPTunnel *b = cpiInterfaceIPTunnel_Create();
 *
 *    if (cpiInterfaceIPTunnel_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool cpiInterfaceIPTunnel_Equals(const CPIInterfaceIPTunnel *a, const CPIInterfaceIPTunnel *b);

/**
 * JSON representation
 *
 * <code>
 * { "TUNNEL" :
 *   { "IFIDX" : ifidx,
 *    ["STATE" : "UP" | "DOWN", ]
 *    "TYPE": "UDP" | "TCP" | "GRE",
 *    "SRC" : {srcaddr},
 *    "DST" : {dstaddr}
 *   }
 * }
 * </code>
 *
 * @param [in] ipTunnel A pointer to a valid CPIInterfaceIPTunnel
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *cpiInterfaceIPTunnel_ToJson(const CPIInterfaceIPTunnel *ipTunnel);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] json A pointer to a valid PARCJSON instance.
 *
 * @return non-NULL A pointer to a valid CPIInterfaceIPTunnel instance.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
CPIInterfaceIPTunnel *cpiInterfaceIPTunnel_CreateFromJson(PARCJSON *json);
#endif // libccnx_cpi_InterfaceIPTunnel_h
