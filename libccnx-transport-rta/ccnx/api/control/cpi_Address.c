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

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <errno.h>

#include <ccnx/api/control/cpi_Address.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Base64.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Network.h>

#include <LongBow/runtime.h>

const char *cpiAddressType = "ADDRESSTYPE";
const char *cpiAddrData = "DATA";

struct cpi_address {
    CPIAddressType addressType;
    PARCBuffer *blob;
};

static struct cpi_address_type_str {
    CPIAddressType type;
    const char *str;
} cpiAddressTypeString[] = {
    { .type = cpiAddressType_INET,  .str = "INET"  },
    { .type = cpiAddressType_INET6, .str = "INET6" },
    { .type = cpiAddressType_LINK,  .str = "LINK"  },
    { .type = cpiAddressType_IFACE, .str = "IFACE" },
    { .type = cpiAddressType_UNIX,  .str = "UNIX"  },
    { .type = 0,                    .str = NULL    }
};

void
cpiAddress_Destroy(CPIAddress **addressPtr)
{
    assertNotNull(addressPtr, "Parameter must be non-null double pointer");
    assertNotNull(*addressPtr, "Parameter must dereference to non-null pointer");

    CPIAddress *address = *addressPtr;
    parcBuffer_Release(&address->blob);
    parcMemory_Deallocate((void **) &address);
    *addressPtr = NULL;
}

void
cpiAddress_AssertValid(const CPIAddress *address)
{
    assertNotNull(address, "Parameter must be non-null CPIAddress *");
}

const char *
cpiAddress_TypeToString(CPIAddressType type)
{
    for (int i = 0; cpiAddressTypeString[i].str != NULL; i++) {
        if (cpiAddressTypeString[i].type == type) {
            return cpiAddressTypeString[i].str;
        }
    }
    trapIllegalValue(type, "Unknown value: %d", type);
}

CPIAddressType
cpiAddress_StringToType(const char *str)
{
    for (int i = 0; cpiAddressTypeString[i].str != NULL; i++) {
        if (strcasecmp(cpiAddressTypeString[i].str, str) == 0) {
            return cpiAddressTypeString[i].type;
        }
    }
    trapIllegalValue(str, "Unknown type '%s'", str);
}

static CPIAddress *
_cpiAddress_Create(CPIAddressType addressType, PARCBuffer *buffer)
{
    CPIAddress *result = parcMemory_AllocateAndClear(sizeof(CPIAddress));

    assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIAddress));
    if (result != NULL) {
        result->addressType = addressType;
        result->blob = buffer;
    }
    return result;
}

CPIAddress *
cpiAddress_CreateFromInet(struct sockaddr_in *addr_in)
{
    assertNotNull(addr_in, "Parameter must be non-null");

    addr_in->sin_family = AF_INET;

    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(struct sockaddr_in));
    parcBuffer_PutArray(buffer, sizeof(struct sockaddr_in), (uint8_t *) addr_in);
    parcBuffer_Flip(buffer);

    CPIAddress *result = _cpiAddress_Create(cpiAddressType_INET, buffer);

    return result;
}

CPIAddress *
cpiAddress_CreateFromInet6(struct sockaddr_in6 *addr_in6)
{
    assertNotNull(addr_in6, "Parameter must be non-null");

    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(struct sockaddr_in6));
    parcBuffer_PutArray(buffer, sizeof(struct sockaddr_in6), (uint8_t *) addr_in6);
    parcBuffer_Flip(buffer);

    CPIAddress *result = _cpiAddress_Create(cpiAddressType_INET6, buffer);

    return result;
}

CPIAddress *
cpiAddress_CreateFromLink(const uint8_t *linkaddr, size_t length)
{
    assertNotNull(linkaddr, "Parameter must be non-null");

    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(struct sockaddr_in6));
    parcBuffer_PutArray(buffer, length, linkaddr);
    parcBuffer_Flip(buffer);

    CPIAddress *result = _cpiAddress_Create(cpiAddressType_LINK, buffer);
    return result;
}

CPIAddress *
cpiAddress_CreateFromInterface(unsigned interfaceIndex)
{
    unsigned netbyteorder = htonl(interfaceIndex);

    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(netbyteorder));
    parcBuffer_PutArray(buffer, sizeof(netbyteorder), (uint8_t *) &netbyteorder);
    parcBuffer_Flip(buffer);

    CPIAddress *result = _cpiAddress_Create(cpiAddressType_IFACE, buffer);
    return result;
}

CPIAddress *
cpiAddress_CreateFromUnix(struct sockaddr_un *addr_un)
{
    assertNotNull(addr_un, "Parameter must be non-null");

    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(struct sockaddr_un));
    parcBuffer_PutArray(buffer, sizeof(struct sockaddr_un), (uint8_t *) addr_un);
    parcBuffer_Flip(buffer);

    CPIAddress *result = _cpiAddress_Create(cpiAddressType_UNIX, buffer);
    return result;
}

CPIAddress *
cpiAddress_Copy(const CPIAddress *original)
{
    cpiAddress_AssertValid(original);

    CPIAddress *result = _cpiAddress_Create(original->addressType, parcBuffer_Copy(original->blob));
    return result;
}

bool
cpiAddress_Equals(const CPIAddress *a, const CPIAddress *b)
{
    if (a == b) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    if (a->addressType == b->addressType) {
        if (parcBuffer_Equals(a->blob, b->blob)) {
            return true;
        }
    }

    return false;
}

PARCJSON *
cpiAddress_ToJson(const CPIAddress *address)
{
    cpiAddress_AssertValid(address);

    PARCJSON *json = parcJSON_Create();
    PARCBufferComposer *encoded = parcBase64_Encode(parcBufferComposer_Create(), address->blob);

    // we need a NULL at the end of the string.
    parcBufferComposer_PutUint8(encoded, 0);
    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(encoded);
    char *str = parcBuffer_Overlay(buffer, 0);

    parcJSON_AddString(json, cpiAddressType, cpiAddress_TypeToString(address->addressType));
    parcJSON_AddString(json, cpiAddrData, str);

    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&encoded);

    return json;
}

CPIAddress *
cpiAddress_CreateFromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter must be non-null");

    PARCJSONValue *addrFamilyValue = parcJSON_GetValueByName(json, cpiAddressType);

    assertNotNull(addrFamilyValue, "json is not valid, missing %s: %s", cpiAddressType, parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsString(addrFamilyValue),
               "%s key is not a number: %s", cpiAddressType, parcJSON_ToString(json));

    PARCJSONValue *addrDataValue = parcJSON_GetValueByName(json, cpiAddrData);

    assertNotNull(addrDataValue, "json is not valid, missing %s: %s", cpiAddrData, parcJSON_ToString(json));

    PARCBufferComposer *composer =
        parcBase64_Decode(parcBufferComposer_Create(), parcJSONValue_GetString(addrDataValue));
    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    parcBufferComposer_Release(&composer);

    PARCBuffer *sBuf = parcJSONValue_GetString(addrFamilyValue);

    CPIAddress *result =
        _cpiAddress_Create(cpiAddress_StringToType(parcBuffer_Overlay(sBuf, 0)), buffer);

    return result;
}

CPIAddressType
cpiAddress_GetType(const CPIAddress *address)
{
    cpiAddress_AssertValid(address);

    return address->addressType;
}

// The Get functions need better names, what they do (Get from what? Put to what?)
// is not clear from their names.  Case 1028
bool
cpiAddress_GetInet(const CPIAddress *address, struct sockaddr_in *addr_in)
{
    cpiAddress_AssertValid(address);
    assertNotNull(addr_in, "Parameter addr_in must be non-null");

    if (address->addressType == cpiAddressType_INET) {
        assertTrue(parcBuffer_Remaining(address->blob) == sizeof(struct sockaddr_in),
                   "CPIAddress corrupted. Expected length %zu, actual length %zu",
                   sizeof(struct sockaddr_in),
                   parcBuffer_Remaining(address->blob));

        memcpy(addr_in, parcBuffer_Overlay(address->blob, 0), sizeof(struct sockaddr_in));
        return true;
    }
    return false;
}

bool
cpiAddress_GetInet6(const CPIAddress *address, struct sockaddr_in6 *addr_in6)
{
    cpiAddress_AssertValid(address);
    assertNotNull(addr_in6, "Parameter addr_in6 must be non-null");

    if (address->addressType == cpiAddressType_INET6) {
        assertTrue(parcBuffer_Remaining(address->blob) == sizeof(struct sockaddr_in6),
                   "CPIAddress corrupted. Expected length %zu, actual length %zu",
                   sizeof(struct sockaddr_in6),
                   parcBuffer_Remaining(address->blob));

        memcpy(addr_in6, parcBuffer_Overlay(address->blob, 0), sizeof(struct sockaddr_in6));
        return true;
    }
    return false;
}

bool
cpiAddress_GetUnix(const CPIAddress *address, struct sockaddr_un *addr_un)
{
    cpiAddress_AssertValid(address);
    assertNotNull(addr_un, "Parameter addr_in6 must be non-null");

    if (address->addressType == cpiAddressType_UNIX) {
        assertTrue(parcBuffer_Remaining(address->blob) == sizeof(struct sockaddr_un),
                   "CPIAddress corrupted. Expected length %zu, actual length %zu",
                   sizeof(struct sockaddr_un),
                   parcBuffer_Remaining(address->blob));

        memcpy(addr_un, parcBuffer_Overlay(address->blob, 0), sizeof(struct sockaddr_un));
        return true;
    }
    return false;
}

bool
cpiAddress_GetInterfaceIndex(const CPIAddress *address, uint32_t *ifidx)
{
    cpiAddress_AssertValid(address);
    assertNotNull(ifidx, "Parameter ifidx must be non-null");

    if (address->addressType == cpiAddressType_IFACE) {
        assertTrue(parcBuffer_Remaining(address->blob) == sizeof(uint32_t),
                   "CPIAddress corrupted. Expected length %zu, actual length %zu",
                   sizeof(uint32_t),
                   parcBuffer_Remaining(address->blob));

        uint32_t netbyteorder;
        memcpy(&netbyteorder, parcBuffer_Overlay(address->blob, 0), sizeof(uint32_t));
        *ifidx = ntohl(netbyteorder);
        return true;
    }
    return false;
}

PARCBuffer *
cpiAddress_GetLinkAddress(const CPIAddress *address)
{
    cpiAddress_AssertValid(address);
    if (address->addressType == cpiAddressType_LINK) {
        return address->blob;
    }
    return NULL;
}

static PARCBufferComposer *
_Inet_BuildString(const CPIAddress *address, PARCBufferComposer *composer)
{
    cpiAddress_AssertValid(address);

    struct sockaddr_in *saddr = (struct sockaddr_in *) parcBuffer_Overlay(address->blob, 0);
    return parcNetwork_SockInet4Address_BuildString(saddr, composer);
}

static PARCBufferComposer *
_Inet6_BuildString(const CPIAddress *address, PARCBufferComposer *composer)
{
    cpiAddress_AssertValid(address);

    struct sockaddr_in6 *saddr = (struct sockaddr_in6 *) parcBuffer_Overlay(address->blob, 0);
    return parcNetwork_SockInet6Address_BuildString(saddr, composer);
}

static PARCBufferComposer *
_Link_BuildString(const CPIAddress *address, PARCBufferComposer *composer)
{
    cpiAddress_AssertValid(address);

    const unsigned char *addr = parcBuffer_Overlay(address->blob, 0);

    size_t length = parcBuffer_Remaining(address->blob);

    return parcNetwork_LinkAddress_BuildString(addr, length, composer);
}

static ssize_t
_UnixToString(char *output, size_t remaining_size, const PARCBuffer *addr)
{
    assertNotNull(output, "parameter output must be non-null");
    parcBuffer_AssertValid(addr);

    assertTrue(parcBuffer_Remaining(addr) == sizeof(struct sockaddr_un),
               "CPIAddress corrupted. Expected %zu actual %zu",
               sizeof(struct sockaddr_un), parcBuffer_Remaining(addr));

    // sockaddr length for the path, 16 for the ascii stuff, 3 for the length number
    struct sockaddr_un *saddr = (struct sockaddr_un *) parcBuffer_Overlay((PARCBuffer *) addr, 0);
    size_t min_remaining = strlen(saddr->sun_path) + 16 + 3;
    assertTrue(remaining_size >= min_remaining,
               "Remaining size too small, need at least %zu", min_remaining);

    ssize_t output_length = sprintf(output, "{ .path=%s, .len=%zu }", saddr->sun_path, strlen(saddr->sun_path));
    return output_length;
}

static ssize_t
_IfaceToString(char *output, size_t remaining_size, const PARCBuffer *addr)
{
    assertNotNull(output, "parameter output must be non-null");
    parcBuffer_AssertValid(addr);

    assertTrue(parcBuffer_Remaining(addr) == sizeof(uint32_t),
               "CPIAddress corrupted. Expected %zu actual %zu",
               sizeof(uint32_t), parcBuffer_Remaining(addr));

    uint32_t *ifidx = (uint32_t *) parcBuffer_Overlay((PARCBuffer *) addr, 0);

    ssize_t output_length = sprintf(output, "{ .ifidx=%u }", ntohl(*ifidx));

    return output_length;
}

PARCBufferComposer *
cpiAddress_BuildString(const CPIAddress *address, PARCBufferComposer *composer)
{
    if (address != NULL) {
        char *str = cpiAddress_ToString(address);
        parcBufferComposer_PutString(composer, str);
        parcMemory_Deallocate((void **) &str);
    }
    return composer;
}

char *
cpiAddress_ToString(const CPIAddress *address)
{
    cpiAddress_AssertValid(address);

    char addrstr[256];

    switch (address->addressType) {
        case cpiAddressType_INET: {
            PARCBufferComposer *composer = parcBufferComposer_Create();
            PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(_Inet_BuildString(address, composer));
            char *result = parcBuffer_ToString(tempBuffer);
            parcBuffer_Release(&tempBuffer);
            parcBufferComposer_Release(&composer);
            return result;
        }
        break;

        case cpiAddressType_INET6: {
            PARCBufferComposer *composer = parcBufferComposer_Create();

            PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(_Inet6_BuildString(address, composer));
            char *result = parcBuffer_ToString(tempBuffer);
            parcBuffer_Release(&tempBuffer);

            parcBufferComposer_Release(&composer);
            return result;
        }
        break;

        case cpiAddressType_UNIX:
            _UnixToString(addrstr, 256, address->blob);
            break;

        case cpiAddressType_LINK: {
            PARCBufferComposer *composer = parcBufferComposer_Create();

            PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(_Link_BuildString(address, composer));
            char *result = parcBuffer_ToString(tempBuffer);
            parcBuffer_Release(&tempBuffer);

            parcBufferComposer_Release(&composer);
            return result;
        }
        break;

        case cpiAddressType_IFACE:
            _IfaceToString(addrstr, 256, address->blob);
            break;

        default:
            sprintf(addrstr, "UNKNOWN type = %d", address->addressType);
            break;
    }

    ssize_t alloc_size = 1024;
    char *output = parcMemory_Allocate(alloc_size);
    assertNotNull(output, "parcMemory_Allocate(%zu) returned NULL", alloc_size);
    ssize_t output_length = snprintf(output, alloc_size, "{ .type=%s, .data=%s }", cpiAddress_TypeToString(address->addressType), addrstr);

    assertTrue(output_length < alloc_size, "allocated size too small, needed %zd", output_length);
    assertFalse(output_length < 0, "snprintf error: (%d) %s", errno, strerror(errno));

    return output;
}

PARCHashCode
cpiAddress_HashCode(const CPIAddress *address)
{
    cpiAddress_AssertValid(address);

    PARCHashCode hash = parcBuffer_HashCode(address->blob);
    hash = parcHashCode_HashImpl((uint8_t *) &address->addressType, sizeof(address->addressType), hash);

    return hash;
}
