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
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <sys/socket.h>
#include <ctype.h>
#include <sys/types.h>
#include <netdb.h>

#include <parc/algol/parc_Network.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_URI.h>

struct sockaddr *
parcNetwork_SockAddress(const char *address, in_port_t port)
{
    // this is the final return value from the function
    struct sockaddr *addr = NULL;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_ADDRCONFIG;

    // getaddrinfo allocates this pointer
    struct addrinfo *ai;
    int failure = getaddrinfo(address, NULL, &hints, &ai);
    if (failure == 0) {
        switch (ai->ai_family) {
            case PF_INET: {
                struct sockaddr_in *result = parcMemory_AllocateAndClear(sizeof(struct sockaddr_in));
                assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct sockaddr_in));
                if (result != NULL) {
                    assertTrue(ai->ai_addrlen == sizeof(struct sockaddr_in),
                               "Sockaddr wrong length, expected %zu got %u", sizeof(struct sockaddr_in), ai->ai_addrlen);
                    memcpy(result, ai->ai_addr, ai->ai_addrlen);
                    result->sin_port = htons(port);
                    addr = (struct sockaddr *) result;
                }
                break;
            }

            case PF_INET6: {
                struct sockaddr_in6 *result = parcMemory_AllocateAndClear(sizeof(struct sockaddr_in6));
                assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct sockaddr_in6));
                if (result != NULL) {
                    assertTrue(ai->ai_addrlen == sizeof(struct sockaddr_in6),
                               "Sockaddr wrong length, expected %zu got %u", sizeof(struct sockaddr_in6), ai->ai_addrlen);

                    memcpy(result, ai->ai_addr, ai->ai_addrlen);
                    result->sin6_port = htons(port);
                    result->sin6_flowinfo = 0;
                    result->sin6_scope_id = 0;
                    addr = (struct sockaddr *) result;
                }
                break;
            }

            default: {
                // unsupported protocol
                addr = NULL;
                break;
            }
        }

        freeaddrinfo(ai);
    }

    return addr;
}

struct sockaddr_in *
parcNetwork_SockInet4Address(const char *address, in_port_t port)
{
    struct sockaddr_in *result = parcMemory_AllocateAndClear(sizeof(struct sockaddr_in));
    assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct sockaddr_in));
    if (result != NULL) {
        result->sin_family = AF_INET;
        result->sin_port = htons(port);
#if defined(SIN6_LEN)
        result->sin_len = sizeof(struct sockaddr_in);
#endif
        if (inet_pton(AF_INET, address, &(result->sin_addr)) == 1) {
            return result;
        }
        parcMemory_Deallocate((void **) &result);
    }

    return NULL;
}

struct sockaddr_in6 *
parcNetwork_SockInet6Address(const char *address, in_port_t port, uint32_t flowInfo, uint32_t scopeId)
{
    struct sockaddr_in6 *result = parcMemory_AllocateAndClear(sizeof(struct sockaddr_in6));
    assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct sockaddr_in6));
    if (result != NULL) {
        result->sin6_family = AF_INET6;
        result->sin6_port = htons(port);
        result->sin6_flowinfo = flowInfo;
        result->sin6_scope_id = scopeId;
#if defined(SIN6_LEN)
        result->sin6_len = sizeof(struct sockaddr_in6);
#endif

        if (inet_pton(AF_INET6, address, &(result->sin6_addr)) == 1) {
            return result;
        }
        parcMemory_Deallocate((void **) &result);
    }

    return NULL;
}

struct sockaddr_in *
parcNetwork_SockInet4AddressAny()
{
    struct sockaddr_in *result = parcMemory_AllocateAndClear(sizeof(struct sockaddr_in));
    assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct sockaddr_in));
    if (result != NULL) {
        result->sin_family = AF_INET;
        result->sin_addr.s_addr = INADDR_ANY;
#if defined(SIN6_LEN)
        result->sin_len = sizeof(struct sockaddr_in);
#endif
        return result;
    }
    return NULL;
}

PARCBufferComposer *
parcNetwork_SockInet4Address_BuildString(const struct sockaddr_in *address, PARCBufferComposer *composer)
{
    assertNotNull(address, "Parameter must be a non-null pointer to a struct sockaddr_in.");

    if (address->sin_family != AF_INET) {
        trapIllegalValue(address->sin_family, "Expected an AF_INET configured address, not %d", address->sin_family);
    }

    char buffer[INET_ADDRSTRLEN];
    inet_ntop(address->sin_family, &address->sin_addr, buffer, INET_ADDRSTRLEN);
    parcBufferComposer_Format(composer, "inet4://%s:%u", buffer, ntohs(address->sin_port));

    return composer;
}

PARCBufferComposer *
parcNetwork_SockInet6Address_BuildString(const struct sockaddr_in6 *address, PARCBufferComposer *composer)
{
    assertNotNull(address, "Parameter must be a non-null pointer to a struct sockaddr_in.");

    if (address->sin6_family != AF_INET6) {
        trapIllegalValue(address->sin_family, "Expected an AF_INET6 configured address, not %d", address->sin6_family);
    }

    char buffer[INET6_ADDRSTRLEN];
    inet_ntop(address->sin6_family, &address->sin6_addr, buffer, INET6_ADDRSTRLEN);

    parcBufferComposer_Format(composer, "inet6://[%s%%%u]:%u",
                              buffer,
                              address->sin6_scope_id,
                              ntohs(address->sin6_port));

    return composer;
}

PARCBufferComposer *
parcNetwork_LinkAddress_BuildString(const unsigned char *address, size_t length, PARCBufferComposer *composer)
{
    parcBufferComposer_PutString(composer, "link://");
    for (size_t i = 0; i < length; i++) {
        if (i > 0) {
            parcBufferComposer_PutString(composer, "-");
        }
        parcBufferComposer_Format(composer, "%02x", address[i]);
    }
    return composer;
}

struct sockaddr_in *
parcNetwork_ParseInet4Address(const char *addressURI)
{
    struct sockaddr_in *result = NULL;

    PARCURI *uri = parcURI_Parse(addressURI);
    if (strcmp("inet4", parcURI_GetScheme(uri)) == 0) {
    }
    parcURI_Release(&uri);

    return result;
}

static PARCBuffer *
_parseMAC48AddressDashOrColon(const char *address, PARCBuffer *result)
{
    char *nextP = NULL;

    const char *p = address;

    int i = 0;
    for (i = 0; i < 6; i++) {
        long value = strtol(p, &nextP, 16);
        if (nextP == p || (*nextP != ':' && *nextP != '-' && *nextP != 0)) {
            result = NULL;
            break;
        }
        parcBuffer_PutUint8(result, (uint8_t) value);

        p = nextP + 1;
    }

    if (i != 6) {
        result = NULL;
    }

    return result;
}

static PARCBuffer *
_parseMAC48AddressDot(const char *address, PARCBuffer *result)
{
    char *nextP = NULL;

    const char *p = address;

    int i = 0;
    for (i = 0; i < 3; i++) {
        long value = strtol(p, &nextP, 16);
        if (nextP == p || (*nextP != '.' && *nextP != 0)) {
            result = NULL;
            break;
        }
        parcBuffer_PutUint16(result, (uint16_t) value);

        p = nextP + 1;
    }

    if (i != 3) {
        result = NULL;
    }

    return result;
}

bool
parcNetwork_ParseMAC48Address(const char *address, PARCBuffer *buffer)
{
    bool result = false;

    size_t originalPosition = parcBuffer_Position(buffer);

    if (strchr(address, '-') != NULL || strchr(address, ':') != NULL) {
        if (_parseMAC48AddressDashOrColon(address, buffer) != NULL) {
            result = true;
        }
    }

    if (strchr(address, '.') != NULL) {
        if (_parseMAC48AddressDot(address, buffer) != NULL) {
            result = true;
        }
        ;
    }

    if (result == false) {
        parcBuffer_SetPosition(buffer, originalPosition);
    }

    return result;
}

PARCBuffer *
parcNetwork_ParseLinkAddress(const char *address)
{
    if (strncmp("link://", address, 7) == 0) {
        PARCBuffer *result = parcBuffer_Allocate(7);

        if (parcNetwork_ParseMAC48Address(&address[7], result) == false) {
            parcBuffer_Release(&result);
            trapIllegalValue(address, "Syntax error '%s'", address);
        }

        return parcBuffer_Flip(result);
    }

    trapIllegalValue(address, "Bad scheme '%s'", address);
}

bool
parcNetwork_Inet4Equals(const struct sockaddr_in *x, const struct sockaddr_in *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }
    if (x->sin_family == y->sin_family) {
        if (x->sin_addr.s_addr == y->sin_addr.s_addr) {
            if (x->sin_port == y->sin_port) {
                return true;
            }
        }
    }

    return false;
}

/*
 * Returns true for ::1 address, false otherwise
 *
 * The address is in network byte order
 *
 * @return true The address is local
 * @return false The address is not local
 */
static bool
_isInet6Loopback(struct sockaddr_in6 *sin6)
{
    if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr)) {
        return true;
    }
    return false;
}

/**
 * Returns true if the address is on the 127.0.0.0/8 network
 *
 * The address is expected to be in network byte order
 *
 * @param [in] sin4 A pointer to a sockaddr_in containing an IPv4 address.
 *
 * @return true Address on 127.0.0.0/8
 * @return false Not a 127.0.0.0/8
 */
static bool
_isInet4Loopback(struct sockaddr_in *sin4)
{
    uint32_t hostorder = htonl(sin4->sin_addr.s_addr);
    if ((hostorder & 0xFF000000) == 0x7F000000) {
        return true;
    }
    return false;
}

bool
parcNetwork_IsSocketLocal(struct sockaddr *sock)
{
    assertNotNull(sock, "Parameter sock must be non-null");
    bool isLocal = false;

    switch (sock->sa_family) {
        case PF_LOCAL:
            isLocal = true;
            break;

        case PF_INET:
            isLocal = _isInet4Loopback((struct sockaddr_in *) sock);
            break;

        case PF_INET6:
            isLocal = _isInet6Loopback((struct sockaddr_in6 *) sock);
            break;

        default:
            break;
    }

    return isLocal;
}
