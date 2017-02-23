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
 * @file cpi_RouteEntry.h
 * @brief A representation of a route entry.
 *
 * A CCNx route consists of the tuple (prefix, interfaceIndex, [nextHop], routingProtocol, routeType, [lifetime], cost).
 *
 * The "prefix" is the CCNx name in question.  If the "routeType" is Exact Match then the prefix must exactly match an Interest Name.
 * If the routeType is Longest Prefix (a normal CCNx route), then it will match any equal or longer Interest name.  If the routeType
 * is Default, then it will match any equal or longer name if no other route matched.
 *
 * The interfaceIndex (a.k.a Connection ID) is the entry in the forwarder's connection table to use to forward the Interest.
 * Newer commands use a symblic name instead of a connection id. A symbolic name is an alpha followed by alphanums.  It is specified
 * when creating a tunnel or connection.  Auto-added connections inside the forwarder will only have a connection id.
 *
 * The optional NextHop specifies a link-specific nexthop identifier on the outbound interfaceIndex.   This could be used, for example, with
 * an Ethernet link.  The Connection table entry could be the CCNx Group address entry (i.e. any packet sent to it will go out on the
 * CCNx Ethernet group address) and by specifying the optional NextHop give a specific unicast MAC address.
 *
 * routingProtocol identifies the protocol that created the route entry.
 *
 * routeType, as described above, specifies how the prefix matches an Interest name.
 *
 * lifetime specified how long the router will keep the forwarding entry active.  The routing protocol must refresh the entry
 * to keep it alive.
 *
 * cost reflects the route cost.  Some forwarding strategies might use the cost information to make a decision, but it is not
 * used by the normal unicast or multicast strategies.
 *
 */
#ifndef libccnx_cpi_RouteEntry_h
#define libccnx_cpi_RouteEntry_h

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_NameRouteProtocolType.h>
#include <ccnx/api/control/cpi_NameRouteType.h>

#include <parc/algol/parc_JSON.h>

struct cpi_route_entry;
/**
 * @typedef CPIRouteEntry
 * @brief A representation of a route entry.
 */
typedef struct cpi_route_entry CPIRouteEntry;

/**
 * Creates a route entry, takes ownership of the name object
 *
 * @param [in] prefix for the route, takes ownership of this memory.
 * @param [in] optionalNexthop may be NULL, represents where the FIB points.
 * @param [in] routingProtocol represents the algorithm used to create the route entry.
 * @param [in] routeType selects how the FIB matches the prefix.
 * @param [in] optionalLifetime is how long the FIB entry stays valid unless refreshed.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a valid CPIRouteEntry instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 * }
 * @endcode
 *
 * @see cpiRouteEntry_Destroy
 */
CPIRouteEntry *cpiRouteEntry_Create(CCNxName *prefix, unsigned interfaceIndex, const CPIAddress *optionalNexthop,
                                    CPINameRouteProtocolType routingProtocol, CPINameRouteType routeType,
                                    const struct timeval *optionalLifetime, unsigned cost);


/**
 * Creates a route entry, takes ownership of the name object
 *
 * @param [in] prefix for the route, takes ownership of this memory.
 * @param [in] symbolicName The symbolic name of the connection or tunnel to use.
 * @param [in] routingProtocol represents the algorithm used to create the route entry.
 * @param [in] routeType selects how the FIB matches the prefix.
 * @param [in] optionalLifetime is how long the FIB entry stays valid unless refreshed.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a valid CPIRouteEntry instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_CreateSymbolic(prefix, "tun0", cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 * }
 * @endcode
 *
 * @see cpiRouteEntry_Destroy
 */
CPIRouteEntry *cpiRouteEntry_CreateSymbolic(CCNxName *prefix, const char *symbolicName,
                                            CPINameRouteProtocolType routingProtocol, CPINameRouteType routeType,
                                            const struct timeval *optionalLifetime, unsigned cost);

/**
 * Create a CPIRouteEntry instance that represents a route to this node.
 *
 * @param [in] name A CCNxName instance representing the name to route to this node.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to an allocated CPIRouteEntry instance that must be deallocated via cpiRouteEntry_Destroy.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_FromCString("lci:/a/b");
 *     CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(prefix);
 *     ccnxName_Release(&prefix);
 * }
 * @endcode
 *
 * @see cpiRouteEntry_Destroy
 */
CPIRouteEntry *cpiRouteEntry_CreateRouteToSelf(const CCNxName *name);

/**
 * Deallocate and destroy a CPIRouteEntry instance.
 *
 * @param [in] routeEntryPtr A pointer to a pointer to a valid CPIRouteEntry instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_FromCString("lci:/a/b");
 *     CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(prefix);
 *     ccnxName_Release(&prefix);
 *
 *     cpiRouteEntry_Destroy(&route);
 * }
 * @endcode
 */
void cpiRouteEntry_Destroy(CPIRouteEntry **routeEntryPtr);

/**
 * Copy a CPIRouteEntry instance.
 *
 * Creates a deep copy of the Route Entry.
 *
 * @param [in] routeEntry A pointer to a valid CPIRouteEntry instance.
 *
 * @return NULL An error occurred
 * @return non-NULL A pointer to a valid CPIRouteEntry instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_FromCString("lci:/a/b");
 *     CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(prefix);
 *     ccnxName_Release(&prefix);
 *
 *     CPIRouteEntry *copy = cpiRouteEntry_Copy(route);
 *     cpiRouteEntry_Destroy(&route);
 *     cpiRouteEntry_Destroy(&copy);
 * }
 * @endcode
 */
CPIRouteEntry *cpiRouteEntry_Copy(const CPIRouteEntry *routeEntry);

/**
 * Determine if two `CPIRouteEntry` instances are equal.
 *
 * The following equivalence relations on non-null `CPIRouteEntry` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `cpiRouteEntry_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `cpiRouteEntry_Equals(x, y)` must return true if and only if
 *        `cpiRouteEntry_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiRouteEntry_Equals(x, y)` returns true and
 *        `cpiRouteEntry_Equals(y, z)` returns true,
 *        then `cpiRouteEntry_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `cpiRouteEntry_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `cpiRouteEntry_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] a A pointer to a `CPIRouteEntry` instance.
 * @param [in] b A pointer to a `CPIRouteEntry` instance.
 *
 * @return true `CPIRouteEntry` x and y are equal.
 * @return false `CPIRouteEntry` x and y are not equal.
 *
 * Example:
 * @code
 * {
 *     CPIRouteEntry *bufferA = parcBuffer_Allocate(10);
 *     CPIRouteEntry *bufferB = parcBuffer_Allocate(10);
 *
 *     CCNxName *prefixA = ccnxName_FromCString("lci:/a/b");
 *     CPIRouteEntry *routeA = cpiRouteEntry_CreateRouteToSelf(prefixA);
 *     ccnxName_Release(&prefixA);
 *
 *     CCNxName *prefixB = ccnxName_FromCString("lci:/a/b");
 *     CPIRouteEntry *routeB = cpiRouteEntry_CreateRouteToSelf(prefixB);
 *     ccnxName_Release(&prefixB);
 *
 *     if (cpiRouteEntry_Equals(routeA, routeB)) {
 *         printf("Routes are equal.\n");
 *     } else {
 *         printf("Routes are  NOT equal.\n");
 *     }
 *     cpiRouteEntry_Destroy(&bufferA);
 *     cpiRouteEntry_Destroy(&bufferB);
 * }
 * @endcode
 */
bool cpiRouteEntry_Equals(const CPIRouteEntry *a, const CPIRouteEntry *b);

/**
 * Set the interface index for the given `CPIRouteEntry`
 *
 * @param [in] route A pointer to a `CPIRouteEntry` instance.
 * @param [in] interfaceIndex The interface index value.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = -1; // unknown
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval *lifetimePtr = NULL;
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetimePtr, cost);
 *
 *     // once we know an interface index, set it
 *     cpiRouteEntry_SetInterfaceIndex(route, 55);
 * }
 * @endcode
 */
void cpiRouteEntry_SetInterfaceIndex(CPIRouteEntry *route, unsigned interfaceIndex);

/**
 * Get the name of the routing prefix in the given `CPIRouteEntry` instance.
 *
 * @param [in] route A pointer to a `CPIRouteEntry` instance.
 *
 * @return A pointer to the name of the routing prefix in the given `CPIRouteEntry` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = -1; // unknown
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval *lifetimePtr = NULL;
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetimePtr, cost);
 *
 *     CCNxName *testPrefix = cpiRouteEntry_GetPrefix(route);
 *     assertTrue(ccnxName_Equals(prefix, testPrefix), "The prefix of the route should be equal to what was set.");
 * }
 * @endcode
 */
const CCNxName *cpiRouteEntry_GetPrefix(const CPIRouteEntry *route);

/**
 * Get the interface index in the given `CPIRouteEntry`
 *
 * @param [in] route A pointer to a `CPIRouteEntry` instance.
 *
 * @return The interface index in the given `CPIRouteEntry`
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval *lifetimePtr = NULL;
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetimePtr, cost);
 *
 *     assertTrue(cpiRouteEntry_GetInterfaceIndex(route), ifidx, "Route interface should be equal to what was set.");
 * }
 * @endcode
 */
unsigned cpiRouteEntry_GetInterfaceIndex(const CPIRouteEntry *route);

/**
 * Get the CPIAddress of the next hop for the given CPIRouteEntry instance.
 *
 * The nexthop may be used for certain types of routes to override the destination address.  This might be used
 * with some Ethernet routes, but is not necessary if one creates a cpiConnectionEthernet to the specific destination.
 *
 * @param [in] route A pointer to a `CPIRouteEntry` instance.
 *
 * @return non-null A pointer to a valid CPIAddress.
 * @return null No nexthop was specified when the route was created.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = NULL;
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *
 *     assertNull(cpiRouteEntry_GetNexthop(route), "Route has no nexthop");
 * }
 * @endcode
 */
const CPIAddress *cpiRouteEntry_GetNexthop(const CPIRouteEntry *route);

/**
 * Determines if the Route Entry has a lifetime
 *
 * Returns true if a lifetime is specified for the route
 *
 * @param [in] route A non-null pointer to a CPIRouteEntry instance.
 *
 * @return true if the given CPIRouteEntry has a lifetime.
 * @return false if it does not have a lifetime
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval *lifetimePtr = NULL;
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetimePtr, cost);
 *
 *     assertFalse(cpiRouteEntry_HasLifetime(route), "NULL lifetimePtr should return false");
 * }
 * @endcode
 *
 * @see cpiRouteEntry_GetLifetime()
 */
bool cpiRouteEntry_HasLifetime(const CPIRouteEntry *route);

/**
 * Returns the lifetime associated with a route
 *
 * The return value is undefined is the route does not have a lifetime.  See cpiRouteEntry_HasLifetime().
 *
 * @param [in] route A non-null pointer to a CPIRouteEntry instance.
 *
 * @return The route lifetime
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *     struct timeval testLifetime = cpiRouteEntry_GetLifetime(route);
 *     assertTrue(timercmp(&lifetime, &testLifetime, ==), "Lifetimes should be equal");
 * }
 * @endcode
 */
struct timeval cpiRouteEntry_GetLifetime(const CPIRouteEntry *route);

/**
 * Returns the protocol identifer that created the route
 *
 * The ProtocolType identifies who created the route, such as a static route (administratively created) or
 * a routing protocol such as ACRON.
 *
 * @param [in] route A non-null pointer to a CPIRouteEntry instance.
 *
 * @return `CPINameRouteProtocolType` the protocol specified when the route was created
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *
 *     CPINameRouteProtocolType protocol = cpiRouteEntry_GetRouteProtocolType(route);
 *     assertTrue(protocol == cpiNameRouteProtocolType_STATIC, "Route is of STATIC protocol");
 * }
 * @endcode
 */
CPINameRouteProtocolType cpiRouteEntry_GetRouteProtocolType(const CPIRouteEntry *route);

/**
 * Returns the type of route
 *
 * The route type determines how an Interest name matches the route.
 *
 * @param [in] route An allocated route entry
 *
 * @return `CPINameRouteType` The route type specified when the route entry was created
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *
 *     CPINameRouteType type = cpiRouteEntry_GetRouteType(route);
 *     assertTrue(type == cpiNameRouteType_LONGEST_MATCH, "Route is of LONGEST_MATCH type");
 * }
 * @endcode
 */
CPINameRouteType cpiRouteEntry_GetRouteType(const CPIRouteEntry *route);

/**
 * Get the "cost" value of the given `CPIRouteEntry`
 *
 * The cost may be used by some forwarding strategies to pick between alternatives.
 *
 * @param [in] route A pointer to a valid `CPIRouteEntry`
 *
 * @return The "cost" value of the given `CPIRouteEntry`
 *
 * Example:
 * @code
 * {
 *     CCNxName * prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress * nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *
 *     unsigned testCost = cpiRouteEntry_GetCost(route);
 *     assertTrue(cost == testCost, "Route cost should be 200");
 * }
 * @endcode
 */
unsigned cpiRouteEntry_GetCost(const CPIRouteEntry *route);

/**
 * Create a `PARCJSON` representation of the given `CPIRouteEntry` instance.
 *
 * @param [in] route A pointer to a valid `PARCJSON` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to an allocated `CPIRouteEntry` instance that must be deallocated via `cpiRouteEntry_Destroy`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *
 *     PARCJSON *json = cpiRouteEntry_ToJson(route);
 *
 *     ...
 * }
 * @endcode
 *
 * @see cpiRouteEntry_FromJson
 */
PARCJSON *cpiRouteEntry_ToJson(const CPIRouteEntry *route);

/**
 * Create a new `CPIRouteEntry` instance from the given `PARCJSON` instance.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to an allocated `CPIRouteEntry` instance that must be deallocated via `cpiRouteEntry_Destroy`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *
 *     PARCJSON *json = cpiRouteEntry_ToJson(route);
 *
 *     ...
 *
 *     CPIRouteEntry *newRoute = cpiRouteEntry_FromJson(json);
 *     // route and newRoute will be equal
 * }
 * @endcode
 *
 * @see cpiRouteEntry_ToJson
 */
CPIRouteEntry *cpiRouteEntry_FromJson(PARCJSON *json);

/**
 * Produce a nul-terminated string representation of the specified instance.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] buffer A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, nul-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 *
 * Example:
 * @code
 * {
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
 *     unsigned ifidx = 55;
 *     CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
 *     struct timeval lifetime = { 3600, 0 };
 *     unsigned cost = 200;
 *
 *     CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
 *
 *     printf("Route: %s\n", cpiRouteEntry_ToString(route));
 * }
 * @endcode
 */
char *cpiRouteEntry_ToString(CPIRouteEntry *route);

/**
 * Returns the symblic name associated with the route entry, may be NULL
 *
 * A symbolic name is not always associated with a route entry.
 *
 * @param [in] route An allocted CPIRouteEntry
 *
 * @return non-null The symbolic name associated with the route entry
 * @return null No symbolic name is associated with the route entry.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const char *cpiRouteEntry_GetSymbolicName(const CPIRouteEntry *route);

#endif // libccnx_cpi_RouteEntry_h
