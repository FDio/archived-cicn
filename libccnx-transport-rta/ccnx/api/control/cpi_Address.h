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
 * @file cpi_Address.h
 * @brief Represents an endpoint address.
 *
 * Represents an endpoint address.  May be INET, INET6, or a multi-byte LINK,
 * or an Interface Index.
 *
 * INET and INET6 must contain the .sa_addr member, and other members as needed
 * by the use of the address.
 *
 * The Interface Index address is essentially a pointer to a device.
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#ifndef libccnx_cpi_Address_h
#define libccnx_cpi_Address_h

#include <netinet/in.h>
#include <sys/un.h>
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BufferComposer.h>

typedef enum {
    cpiAddressType_INET = 1,
    cpiAddressType_INET6 = 2,
    cpiAddressType_LINK = 3,
    cpiAddressType_IFACE = 4,
    cpiAddressType_UNIX = 5    /* PF_UNIX */
} CPIAddressType;

/**
 * Return a string representation of the given `CPIAddressType`
 *
 * @param [in] type A valid CPIAddressType value.
 *
 * @return NULL An error occurred
 * @return non-NULL A pointer to a static string representation of the `CPIAddressType`.
 *
 * Example:
 * @code
 * {
 *     const char *typeAsString = cpiAddress_TypeToString(cpiAddressType_INET);
 * }
 * @endcode
 *
 * @see cpiAddress_StringToType
 */
const char *cpiAddress_TypeToString(CPIAddressType type);

/**
 * Return a `CPIAddressType` from the given nul-terminated C string.
 *
 * This induces a LongBow trap for an illegal value.
 *
 * @param [in] typeAsString A nul-terminated, C string representation of a `CPIAddressType`.
 *
 * @return A CPIAddressType
 *
 * Example:
 * @code
 * {
 *     CPIAddressType type = cpiAddress_TypeToString("INET");
 * }
 * @endcode
 *
 * @see cpiAddress_TypeToString
 */
CPIAddressType cpiAddress_StringToType(const char *typeAsString);

struct cpi_address;
typedef struct cpi_address CPIAddress;

/**
 * Create a new `CPIAddress` instance from an IPv4 IP address, the port is optional.
 *
 * The sockaddr_in should be filled in network byte order. The newly created instance must
 * eventually be destroyed by calling {@link cpiAddress_Destroy}().
 *
 * @param [in] addr_in The `sockaddr_in` representing the IPv4 IP address with which to initialize the new `CPIAddress` instance.
 * @return A new instance of `CPIAddress` that must eventually be destroyed by calling {@link cpiAddress_Destroy}().
 *
 * Example:
 * @code
 * {
 *     CPIAddress *dest = cpiAddress_CreateFromInet(
 *                                                  &(struct sockaddr_in) {
 *                                                      .sa_addr = inet_addr("foo.bar.com"),
 *                                                      .sa_port = htons(9695) } );
 *     cpiAddress_Destroy(&dest);
 * }
 * @endcode
 * @see cpiAddress_Destroy
 */
CPIAddress *cpiAddress_CreateFromInet(struct sockaddr_in *addr_in);

/**
 * Create a new `CPIAddress` instance from an IPv6 IP address, the port is optional.
 *
 *
 * The sockaddr_in should be filled in network byte order. The newly created instance must
 * eventually be destroyed by calling {@link cpiAddress_Destroy}().
 *
 * @param [in] addr_in6 A `sockaddr_in6` from which to initialize a new instance of CPIAddress
 * @return A new instance of `CPIAddress` that must eventually be destroyed by calling {@link cpiAddress_Destroy}()
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_in6 addr_in6;
 *     memset(&addr_in6, 0, sizeof(struct sockaddr_in6));
 *
 *     inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
 *     addr_in6.sin6_family = AF_INET6;
 *     addr_in6.sin6_port = 0x0A0B;
 *     addr_in6.sin6_flowinfo = 0x01020304;
 *
 *     CPIAddress *address = cpiAddress_CreateFromInet6(&addr_in6);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see cpiAddress_Destroy
 */
CPIAddress *cpiAddress_CreateFromInet6(struct sockaddr_in6 *addr_in6);

/**
 * Create a new `CPIAddress` instance, initialized from a Link address.
 *
 * User must know the link address format (i.e. token ring vs ethernet) and have the address in a byte array.
 * The array is encoded in left-to-right order. The newly created instance must eventually be destroyed by
 * calling {@link cpiAddress_Destroy}().
 *
 * @param [in] linkaddr A byte array containing the link address
 * @param [in] length The length of the link address byte array
 * @return A new instance of `CPIAddress` that must eventually be destroyed by calling {@link cpiAddress_Destroy}()
 *
 * Example:
 * @code
 * {
 *     uint8_t mac[] = { 0x14, 0x10, 0x9f, 0xd7, 0x0b, 0x89 };
 *     CPIAddress *address = cpiAddress_CreateFromLink(mac, sizeof(mac));
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see cpiAddress_Destroy
 */
CPIAddress *cpiAddress_CreateFromLink(const uint8_t *linkaddr, size_t length);

/**
 * Create a new `CPIAddress` instance from a network interface index.
 *
 * The interfaceIndex should be in host byte order. The newly created instance must eventually be destroyed by
 * calling {@link cpiAddress_Destroy}().
 *
 * @param [in] interfaceIndex The index of the interface to encode
 * @return A new instance of `CPIAddress` that must eventually be destroyed by calling {@link cpiAddress_Destroy}()
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(2);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see cpiAddress_Destroy
 */
CPIAddress *cpiAddress_CreateFromInterface(uint32_t interfaceIndex);

/**
 * Create a new CPIAddress instance from a PF_UNIX address domain.
 *
 * The newly created instance must eventually be destroyed by calling {@link cpiAddress_Destroy}().
 *
 * @param [in] addr_un The `struct sockaddr_un` specifying the local PF_UNIX socket address
 * @return A new instance of `CPIAddress` that must eventually be destroyed by calling {@link cpiAddress_Destroy}()
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_un addr_unix;
 *     memset(&addr_unix, 0, sizeof(struct sockaddr_un));
 *     char path[] = "/Hello/Cruel/World";
 *     strcpy(addr_un.sun_path, path);
 *     addr_un.sun_family = AF_UNIX;
 *
 *     CPIAddress *address = cpiAddress_CreateFromUnix(&addr_un);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see cpiAddress_Destroy
 */
CPIAddress *cpiAddress_CreateFromUnix(struct sockaddr_un *addr_un);

/**
 * Create a deep copy of an instance of a `CPIAddress`. A completely new, indedependent instance is created.
 *
 * The newly created instance must eventually be destroyed by calling {@link cpiAddress_Destroy}().
 *
 * @param [in] original A pointer to a `CPIAddress` instance to be copied.
 * @return A new instance of a CPIAddress, deep copied from the `original` instance.
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(2);
 *
 *     CPIAddress *copy = cpiAddress_Copy(address);
 *
 *     cpiAddress_Destroy(&address);
 *     cpiAddress_Destroy(&copy);
 * }
 * @endcode
 * @see cpiAddress_Destroy
 */
CPIAddress *cpiAddress_Copy(const CPIAddress *original);

/**
 * Deallocate an instance of a CPIAddress.
 *
 * The CPIAddress instance is deallocated, and any referenced data is also deallocated.
 * The referenced pointer is set to NULL upon return.
 *
 * @param [in] addressPtr A pointer to a pointer to an instance of CPIAddress.
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(2);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 */
void cpiAddress_Destroy(CPIAddress **addressPtr);

/**
 * Create a new PARCJSON instance representing the specified `CPIAddress` instance.
 *
 * The newly created PARCJSON instance must eventually be destroyed by calling {@link parcJSON_Release}().
 *
 * @param [in] address A pointer to a CPIAddress instance.
 * @return A new PARCJSON instance representing the specified `address`.
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_in6 addr_in6;
 *     memset(&addr_in6, 0, sizeof(struct sockaddr_in6));
 *
 *     inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
 *     addr_in6.sin6_family = AF_INET6;
 *     addr_in6.sin6_port = 0x0A0B;
 *     addr_in6.sin6_flowinfo = 0x01020304;
 *
 *     CPIAddress *address = cpiAddress_CreateFromInet6(&addr_in6);
 *
 *     PARCJSON *json = cpiAddress_ToJson(address);
 *
 *     CPIAddress *address2 = cpiAddress_CreateFromJson(json);
 *
 *     cpiAddress_Destroy(&address);
 *     cpiAddress_Destroy(&address2);
 *     parcJSON_Release(&json);
 * }
 * @endcode
 *
 * @see <#references#>
 */
PARCJSON *cpiAddress_ToJson(const CPIAddress *address);

/**
 * Create a new PARCJSON instance from a JSON description of an address.
 *
 * The JSON passed in should look like `{ "LABEL" : { "ADDRESSTYPE" : integer, "DATA" : base_64_data } }`.
 * The newly created PARCJSON instance must eventually be destroyed by calling {@link parcJSON_Release}().
 *
 * The value of "LABEL" does not matter, but the inner structure must be as specified.
 *
 * The ADDRESSTYPE is one of {@link CPIAddressType}.
 *
 * @param [in] json A pointer to a PARCJSON instance describing an address
 * @return A newly created CPIAddress instance
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_in6 addr_in6;
 *     memset(&addr_in6, 0, sizeof(struct sockaddr_in6));
 *
 *     inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
 *     addr_in6.sin6_family = AF_INET6;
 *     addr_in6.sin6_port = 0x0A0B;
 *     addr_in6.sin6_flowinfo = 0x01020304;
 *
 *     CPIAddress *address = cpiAddress_CreateFromInet6(&addr_in6);
 *
 *     PARCJSON *json = cpiAddress_ToJson(address);
 *
 *     CPIAddress *address2 = cpiAddress_CreateFromJson(json);
 *
 *     cpiAddress_Destroy(&address);
 *     cpiAddress_Destroy(&address2);
 *     parcJSON_Release(&json);
 * }
 * @endcode
 */
CPIAddress *cpiAddress_CreateFromJson(PARCJSON *json);

/**
 * Determine if two CPIAddress instances are equal.
 *
 *
 * The following equivalence relations on non-null `CPIAddress` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `cpiAddress_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiAddress_Equals(x, y)` must return true if and only if
 *        `cpiAddress_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiAddress_Equals(x, y)` returns true and
 *        `cpiAddress_Equals(y, z)` returns true,
 *        then  `cpiAddress_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiAddress_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiAddress_Equals(x, NULL)` must
 *      return false.
 *
 * If one address specifies more information than other,
 * e.g. a is INET with a port and b is not, they are not equal.
 *
 * `a` and `b` may be NULL, and NULL == NULL.
 *
 * @param a A pointer to a CPIAddress instance
 * @param b A pointer to a CPIAddress instance
 * @return true if the two instances are equal
 * @return false if the two instances are not equal
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(2);
 *     CPIAddress *copy = cpiAddress_Copy(address);
 *
 *     if (cpiAddress_Equals(address, copy)) {
 *         // true
 *     }  else {
 *         // false
 *     }
 *
 *     cpiAddress_Destroy(&address);
 *     cpiAddress_Destroy(&copy);
 * }
 * @endcode
 */
bool cpiAddress_Equals(const CPIAddress *a, const CPIAddress *b);

/**
 * Return the {@link CPIAddressType} from a specified CPIAddress.
 *
 * @param [in] A pointer to a CPIAddress instance
 *
 * @return the {@link CPIAddressType} of the specified CPIAddress instance
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(2);
 *
 *     CPIAddressType type = cpiAddress_GetType(address);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 *
 * @see CPIAddressType
 */
CPIAddressType cpiAddress_GetType(const CPIAddress *address);

/**
 * Fills in the output parameter with an INET address.
 *
 * @param addr_in must be non-NULL
 * @return true if INET address and output filled in, false otherwise.
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_in6 addr_in6;
 *     memset(&addr_in6, 0, sizeof(struct sockaddr_in6));
 *
 *     inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
 *     addr_in6.sin6_family = AF_INET6;
 *     addr_in6.sin6_port = 0x0A0B;
 *     addr_in6.sin6_flowinfo = 0x01020304;
 *
 *     CPIAddress *address = cpiAddress_CreateFromInet6(&addr_in6);
 *
 *     struct sockaddr_in6 addr_test;
 *     bool success = cpiAddress_GetInet6(address, &addr_test);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 */
bool cpiAddress_GetInet(const CPIAddress *address, struct sockaddr_in *addr_in);

/**
 * Retrieve the INET6 address associated with a `CPIAddress` instance.
 *
 * If the specified CPIAddress instance is of type {@link cpiAddressType_INET6}, then
 * populate the supplied `struct sockaddr_in6` from the CPIAddress and return true. If the
 * CPIAddress is not of type `cpiAddressType_INET6`, this function returns false.
 *
 * @param [in] address A pointer to a `CPIAddress` instance of type {@link cpiAddressType_INET6}.
 * @param [in] addr_in6 A pointer to a `struct sockaddr_in6`. Must be non-NULL.
 * @return true If the CPIAddress instance is of type `cpiAddressType_INET6` and `addr_in6` was filled in
 * @return false If the CPIAddress instance was not of type `cpiAddressType_INET6` or `addr_in6` could not be filled in.
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_in6 addr_in6;
 *     memset(&addr_in6, 0, sizeof(struct sockaddr_in6));
 *
 *     inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
 *     addr_in6.sin6_family = AF_INET6;
 *     addr_in6.sin6_port = 0x0A0B;
 *     addr_in6.sin6_flowinfo = 0x01020304;
 *
 *     CPIAddress *address = cpiAddress_CreateFromInet6(&addr_in6);
 *
 *     struct sockaddr_in6 addr_test;
 *     bool success = cpiAddress_GetInet6(address, &addr_test);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see cpiAddress_GetType
 */
bool cpiAddress_GetInet6(const CPIAddress *address, struct sockaddr_in6 *addr_in6);

/**
 * Retrieve the interface index associated with a `CPIAddress` instance.
 *
 * If the specified `CPIAddress` instance is of type {@link cpiAddressType_IFACE}, then
 * populate the supplied `uint32_t` from the CPIAddress and return true. If the
 * `CPIAddress` is not of type `cpiAddressType_INET6`, this function returns false.
 *
 * @param [in] address A pointer to a `CPIAddress` instance of type {@link cpiAddressType_IFACE}.
 * @param [in] interfaceIndex A pointer to a `uint32_t` to fill in. Must be non-NULL.
 * @return true If the CPIAddress instance is of type `cpiAddressType_IFACE` and `interfaceIndex` was filled in.
 * @return false If the CPIAddress instance was not of type `cpiAddressType_IFACE` or `interfaceIndex` could not be filled in.
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(6);
 *
 *     uint32_t test;
 *     bool success = cpiAddress_GetInterfaceIndex(address, &test);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see cpiAddress_GetType
 */
bool cpiAddress_GetInterfaceIndex(const CPIAddress *address, uint32_t *interfaceIndex);

/**
 * Retrieve the link address associated with a `CPIAddress` instance.
 *
 * If the specified `CPIAddress` instance is of type {@link cpiAddressType_LINK}, then return a pointer
 * to the {@link PARCBuffer} containing the link address. If the `CPIAddress` is not of type {@link cpiAddressType_LINK},
 * then return NULL. The returned PARCBuffer pointer points to memory managed by the CPIAddress instance, and
 * does not need to be destroyed or released on its own.
 *
 * @param [in] address A pointer to a `CPIAddress` instance of type {@link cpiAddressType_LINK}.
 * @return A pointer to the {@link PARCBuffer} containing the link address.
 *
 * Example:
 * @code
 * {
 *     uint8_t mac[] = { 0x14, 0x10, 0x9f, 0xd7, 0x0b, 0x89 };
 *     CPIAddress *address = cpiAddress_CreateFromLink(mac, sizeof(mac));
 *
 *     PARCBuffer *macBuffer = cpiAddress_GetLinkAddress(address);
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see cpiAddress_GetType
 */
PARCBuffer *cpiAddress_GetLinkAddress(const CPIAddress *address);

/**
 * Append the string representation of a `CPIAddress` to a specified `PARCBufferComposer`.
 *
 * @param [in] address A pointer to a `CPIAddress` instance.
 * @param [in] composer A pointer to a `PARCBufferComposer` instance to which to append the string.
 *
 * @return The `PARCBufferComposer` instance that was passed in.
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(1);
 *     PARCBufferComposer *composer = cpiAddress_BuildString(address, parcBufferComposer_Create());
 *     parcBufferComposer_Release(&composer);
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 *
 * @see PARCBufferComposer
 */
PARCBufferComposer *cpiAddress_BuildString(const CPIAddress *address, PARCBufferComposer *composer);

/**
 * Produce a nil-terminated string representation of the specified instance.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] interest A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, nul-terminated C string that must be deallocated via {@link parcMemory_Deallocate}().
 *
 * Example:
 * @code
 * {
 *     CPIAddress *address = cpiAddress_CreateFromInterface(1);
 *
 *     char *string = cpiAddress_ToString(address);
 *
 *     if (string != NULL) {
 *         printf("CPIAddress looks like: %s\n", string);
 *         parcMemory_Deallocate(string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     cpiAddress_Destroy(&address);
 * }
 * @endcode
 * @see parcMemory_Deallocate
 * @see cpiAddress_BuildString
 */
char *cpiAddress_ToString(const CPIAddress *address);

/**
 * Return a non-cryptographic hash code consistent with Equals
 *
 * If cpiAddressA == cpiAddressB, then cpiAddress_HashCode(cpiAddressA) == cpiAddress_HashCode(cpiAddressB)
 *
 * @param [in] address A pointer to a CPIAddress instance.
 * @return A 32-bit hashcode for the specified CPIAddress instance.
 *
 * Example:
 * @code
 *     CPIAddress *address = cpiAddress_CreateFromInterface(1);
 *
 *     uint32_t hashCode = cpiAddress_HashCode(address);
 *
 *     cpiAddress_Destroy(&address);
 * @endcode
 */
PARCHashCode cpiAddress_HashCode(const CPIAddress *address);
#endif // libccnx_cpi_Address_h
