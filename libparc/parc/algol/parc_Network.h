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
 * @file parc_Network.h
 * @ingroup networking
 * @brief Basic Networking Support
 *
 */
#ifndef libparc_parc_Networking_h
#define libparc_parc_Networking_h

#include <arpa/inet.h>
#include <netinet/in.h>

#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Buffer.h>

#ifndef INPORT_ANY
#define INPORT_ANY 0
#endif

#ifdef __ANDROID__
typedef uint16_t in_port_t;
#endif

/**
 * Parses any string in to an address, if possible
 *
 * The string may be an IPv6, IPv6 or hostname.  If the string does not match an IPv4 or IPv6 nominal format,
 * it will try to resolve the string as a hostname.  If that fails, the function will return NULL.
 *
 * IMPORTANT: the returned pointer is allocated with <code>parcMemory_Allocate()</code> and you must use <code>parcMemory_Deallocate()</code>.
 *
 * @param [in] address An IPv4, IPv6, or hostname
 * @param [in] port the port address
 *
 * @return NULL Could not parse the address or resolve as a hostname
 * @return non-NULL a valid sockaddr.  You shoule examine `sockaddr->sa_family` to determine IPv4 or IPv6.
 *
 * Example:
 * @code
 * {
 *    struct sockaddr *addr;
 *    addr = parcNetwork_SockAddress("1.2.3.4", 555);
 *    assertTrue(addr && addr->sa_family == AF_INET, "Addr not IPv4 for a dotted quad.");
 *    struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;
 *    // ...
 *    parcMemory_Deallocate((void **)&addr);
 *
 *    addr = parcNetwork_SockAddress("fe80::aa20:66ff:fe00:314a", 555);
 *    assertTrue(addr && addr->sa_family == AF_INET6, "Addr not IPv6.");
 *    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *) addr;
 *    // ...
 *    parcMemory_Deallocate((void **)&addr);
 *
 *    addr = parcNetwork_SockAddress("alpha.parc.com", 555);
 *    assertTrue(addr && addr->sa_family == AF_INET, "Addr not IPv4 hostname with only ipv4 address.");
 *    // ...
 *    parcMemory_Deallocate((void **)&addr);
 *
 *    addr = parcNetwork_SockAddress("Over the rainbow, way up high", 555);
 *    assertNull(addr, "Addr no null for bogus name.");
 *
 * }
 * @endcode
 */
struct sockaddr *parcNetwork_SockAddress(const char *address, in_port_t port);

/**
 * Compose an allocated sockaddr_in structure.
 *
 *
 * @param [in] address An IPv4, IPv6, or hostname
 * @param [in] port the port address
 * @return A pointer to an allocated struct sockaddr_in which must be freed by <code>parcMemory_Deallocate()</code>, or NULL on error.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
struct sockaddr_in *parcNetwork_SockInet4Address(const char *address, in_port_t port);

/**
 * Compose an allocated sockaddr_in6 structure.
 *
 *
 * @param [in] address An IPv4, IPv6, or hostname
 * @param [in] port the port address
 * @param flowInfo
 * @param scopeId
 * @return A pointer to an allocated sockaddr_in6 structure that must be freed via parcMemory_Deallocate(), or NULL on error.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
struct sockaddr_in6 *parcNetwork_SockInet6Address(const char *address, in_port_t port, uint32_t flowInfo, uint32_t scopeId);

/**
 * Allocate a struct sockaddr_in
 *
 * @return A pointer to an allocated struct sockaddr_in which must be freed by {@link parcMemory_Deallocate()}, or NULL on error.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
struct sockaddr_in *parcNetwork_SockInet4AddressAny(void);

/**
 * Append the string representation of the `struct sockaddr_in` to a {@link PARCBufferComposer}.
 *
 * The position of the `PARCBufferComposer` is incremented by the number characters necessary to represent the `struct sockaddr_in`
 *
 * @param [in] address An IPv4, IPv6, or hostname
 * @param [in,out] composer  The instance of `PARCBufferComposer` to be modified.
 * @return return
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBufferComposer *parcNetwork_SockInet4Address_BuildString(const struct sockaddr_in *address, PARCBufferComposer *composer);

/**
 * Append the string representation of the `struct sockaddr_in6` to a `PARCBufferComposer`.
 *
 * The position of the `PARCBufferComposer` is incremented by the number characters necessary to represent the `struct sockaddr_in6`
 *
 *
 * @param [in] address A pointer to the spckaddr_in6 to append.
 * @param [in,out] composer a POinter to the `PARCBufferComposer` to modify.
 * @return A pointer to the @p composer.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBufferComposer *parcNetwork_SockInet6Address_BuildString(const struct sockaddr_in6 *address, PARCBufferComposer *composer);

/**
 * The standard (IEEE 802) format for printing MAC-48 addresses in human-friendly form is six groups of two hexadecimal digits,
 * separated by hyphens (-) or colons (:), in transmission order (e.g. `01-23-45-67-89-ab` or `01:23:45:67:89:ab`).
 * This form is also commonly used for EUI-64.
 * Another convention used by networking equipment uses three groups of four hexadecimal digits separated by dots (.)
 * (e.g. `0123.4567.89ab` ), again in transmission order.
 *
 * @param [in] address A pointer to the adress of the string.
 * @param [in] length  The size of the buffer
 * @param [in] composer A pointer to the buffer.
 * @return A pointer to the `PARCBufferComposer` containing the built string.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBufferComposer *parcNetwork_LinkAddress_BuildString(const unsigned char *address, size_t length, PARCBufferComposer *composer);

/**
 * Parse a link address, expressed as a nul-terminated C string of the form
 * `link://` followed by hexadecimal numbers representing single byte values,
 * each value separated by either a ':' or '-' character.
 *
 * The standard (IEEE 802) format for printing MAC-48 addresses in human-friendly form is six groups of two hexadecimal digits,
 * separated by hyphens (-) or colons (:), in transmission order (e.g. `01-23-45-67-89-ab` or `01:23:45:67:89:ab`).
 * This form is also commonly used for EUI-64.
 *
 * Another convention used by networking equipment uses three groups of four hexadecimal digits separated by dots (.)
 * (e.g. `0123.4567.89ab` ), again in transmission order.
 *
 * @param address A null-terminated C string of the form `link://` followed single byte values separated by ':' or '-'.
 * @return A PARCBuffer instance containing the parsed bytes of the link address.
 *
 * Example:
 * @code
 * {
 *
 *     PARCBuffer *address = parcNetwork_ParseLinkAddress("link://32-00-14-06-30-60");
 *
 * }
 * @endcode
 *
 * @see parcNetwork_ParseMAC48Address
 */
PARCBuffer *parcNetwork_ParseLinkAddress(const char *address);

/**
 * Parse a MAC-48 address, expressed as a nul-terminated C string of the standard (IEEE 802)
 * format for printing MAC-48 addresses in human-friendly form is six groups of two hexadecimal digits,
 * separated by hyphens (-) or colons (:), in transmission order (e.g. `01-23-45-67-89-ab` or `01:23:45:67:89:ab`).
 * This form is also commonly used for EUI-64.
 *
 * Another convention used by networking equipment uses three groups of four hexadecimal digits separated by dots (.)
 * (e.g. `0123.4567.89ab` ), again in transmission order.
 *
 * @param [in] address A null-terminated C string of the form `link://` followed single byte values separated by ':' or '-'.
 * @param [out] result A pointer to a `PARCBuffer` for the result.
 * @return `true` If the address was successfully parsed.
 * @return `false` if the address was not successfully parsed. The output PARCBuffer is unmodified.
 *
 * Example:
 * @code
 * {
 *
 *     PARCBuffer *address = parcNetwork_ParseMAC48Address("32-00-14-06-30-60");
 *
 * }
 * @endcode
 */
bool parcNetwork_ParseMAC48Address(const char *address, PARCBuffer *result);

/**
 * Return the address parsed from @p addressURI.
 *
 * @param [in] addressURI
 * @return A pointer to the `sockaddr_in`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
struct sockaddr_in *parcNetwork_ParseInet4Address(const char *addressURI);

/**
 * Determines if a socket is local
 *
 * A socket is local if: (a) PF_LOCAL/AF_UNIX, or (b) PF_INET and on the 127.0.0.0/8 network,
 * or (c) PF_INET6 and equal to the ::1 address.  Anything else is considered non-local.
 *
 * @param [in] sock A socket structure
 *
 * @return `true` The socket represents a local/loopback address
 * @return `false` The socket is not a local/loopback address
 *
 * Example:
 * @code
 * {
 *   struct sockaddr *s = parcNetwork_SockAddress("127.1.1.1", 5900);
 *   assertTrue(parcNetwork_IsSocketLocal(s), "This will not assert");
 *   parcMemory_Deallocate((void **)&s);
 * }
 * @endcode
 */
bool parcNetwork_IsSocketLocal(struct sockaddr *sock);

/**
 * Return true if two `sockaddr_in` instances are equal.
 *
 * The equality function that this evaluates must implement the following equivalence relations on non-null instances:
 *
 *   * It is reflexive: for any non-null reference value x, parcNetwork_Inet4Equals(x, x) must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, parcNetwork_Inet4Equals(x, y) must return true if and only if
 *        parcNetwork_Inet4Equals(y x) returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        parcNetwork_Inet4Equals(x, y) returns true and
 *        parcNetwork_Inet4Equals(y, z) returns true,
 *        then  parcNetwork_Inet4Equals(x, z) must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of parcNetwork_Inet4Equals(x, y)
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, equals(x, NULL)) must return false.
 *
 * @param [in] x A pointer to a struct sockaddr_in instance.
 * @param [in] y A pointer to a struct sockaddr_in instance.s
 * @return true the given sockaddr_in instances are equal.
 * @return false the given sockaddr_in instances are not equal.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcNetwork_Inet4Equals(const struct sockaddr_in *x, const struct sockaddr_in *y);
#endif // libparc_parc_Networking_h
