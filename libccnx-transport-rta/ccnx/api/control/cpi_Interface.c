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
#include <stdlib.h>
#include <string.h>

#include <ccnx/api/control/cpi_AddressList.h>
#include <ccnx/api/control/cpi_Interface.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Object.h>

#include <LongBow/runtime.h>

struct cpi_interface {
    char *name;
    unsigned interfaceIndex;
    bool loopback;
    bool supportMulticast;
    unsigned mtu;

    CPIAddressList *addressList;
};

char *
cpiInterface_ToString(const CPIInterface *interface)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    parcBufferComposer_Format(composer, "%3u %10s %1s%1s %8u ",
                              interface->interfaceIndex,
                              interface->name,
                              interface->loopback ? "l" : " ",
                              interface->supportMulticast ? "m" : " ",
                              interface->mtu);

    for (size_t i = 0; i < cpiAddressList_Length(interface->addressList); i++) {
        cpiAddress_BuildString(cpiAddressList_GetItem(interface->addressList, i), composer);
        if (i < (cpiAddressList_Length(interface->addressList) - 1)) {
            parcBufferComposer_PutStrings(composer, "\n", NULL);
        }
    }

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);
    return result;
}

CPIInterface *
cpiInterface_Create(const char *name, unsigned interfaceIndex, bool loopback, bool supportMulticast, unsigned mtu)
{
    CPIInterface *iface = parcMemory_AllocateAndClear(sizeof(CPIInterface));

    assertNotNull(iface, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterface));
    iface->name = parcMemory_StringDuplicate(name, 64);
    iface->interfaceIndex = interfaceIndex;
    iface->loopback = loopback;
    iface->supportMulticast = supportMulticast;
    iface->mtu = mtu;
    iface->addressList = cpiAddressList_Create();

    return iface;
}

void
cpiInterface_Destroy(CPIInterface **interfacePtr)
{
    assertNotNull(interfacePtr, "Parameter must be non-null double pointer");
    assertNotNull(*interfacePtr, "Parameter must dereference to non-null pointer");

    CPIInterface *iface = *interfacePtr;
    parcMemory_Deallocate((void **) &iface->name);
    cpiAddressList_Destroy(&iface->addressList);
    parcMemory_Deallocate((void **) &iface);
    interfacePtr = NULL;
}

void
cpiInterface_AddAddress(CPIInterface *iface, CPIAddress *address)
{
    assertNotNull(iface, "Parameter iface must be non-null");

    size_t length = cpiAddressList_Length(iface->addressList);
    for (size_t i = 0; i < length; i++) {
        const CPIAddress *a = cpiAddressList_GetItem(iface->addressList, i);
        if (cpiAddress_Equals(a, address)) {
            return;
        }
    }

    cpiAddressList_Append(iface->addressList, address);
}

const CPIAddressList *
cpiInterface_GetAddresses(const CPIInterface *iface)
{
    assertNotNull(iface, "Parameter iface must be non-null");
    return iface->addressList;
}

unsigned
cpiInterface_GetInterfaceIndex(const CPIInterface *iface)
{
    assertNotNull(iface, "Parameter iface must be non-null");
    return iface->interfaceIndex;
}

bool
cpiInterface_NameEquals(const CPIInterface *iface, const char *name)
{
    assertNotNull(iface, "Parameter iface must be non-null");

    if (strcasecmp(iface->name, name) == 0) {
        return true;
    }
    return false;
}

bool
cpiInterface_Equals(const CPIInterface *a, const CPIInterface *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    if (a->interfaceIndex == b->interfaceIndex) {
        if (a->loopback == b->loopback) {
            if (a->supportMulticast == b->supportMulticast) {
                if (a->mtu == b->mtu) {
                    if (strcasecmp(a->name, b->name) == 0) {
                        if (cpiAddressList_Equals(a->addressList, b->addressList)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

static const char cpi_Iface[] = "Interface";
static const char cpi_IfName[] = "Name";
static const char cpi_IFIDX[] = "Index";
static const char cpi_IsLoopback[] = "Loopback";
static const char cpi_Multicast[] = "Multicast";
static const char cpi_MTU[] = "MTU";

static const char cpi_True[] = "true";
static const char cpi_False[] = "false";
static const char cpi_Addrs[] = "Addrs";

PARCJSON *
cpiInterface_ToJson(CPIInterface *iface)
{
    assertNotNull(iface, "Parameter must be non-null");

    PARCJSON *inner_json = parcJSON_Create();

    parcJSON_AddString(inner_json, cpi_IfName, iface->name);
    parcJSON_AddInteger(inner_json, cpi_IFIDX, iface->interfaceIndex);
    parcJSON_AddString(inner_json, cpi_IsLoopback, iface->loopback ? cpi_True : cpi_False);
    parcJSON_AddString(inner_json, cpi_Multicast, iface->supportMulticast ? cpi_True : cpi_False);
    parcJSON_AddInteger(inner_json, cpi_MTU, iface->mtu);

    PARCJSONArray *addrsArray = cpiAddressList_ToJson(iface->addressList);
    parcJSON_AddArray(inner_json, cpi_Addrs, addrsArray);
    parcJSONArray_Release(&addrsArray);

    PARCJSON *outter_json = parcJSON_Create();
    parcJSON_AddObject(outter_json, cpi_Iface, inner_json);
    parcJSON_Release(&inner_json);

    return outter_json;
}

CPIInterface *
cpiInterface_FromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter must be non-null");


    PARCJSONValue *value = parcJSON_GetValueByName(json, cpi_Iface);
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpi_Iface,
                  parcJSON_ToString(json));
    PARCJSON *ifaceJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(ifaceJson, cpi_IfName);
    PARCBuffer *sBuf = parcJSONValue_GetString(value);
    const char *name = parcBuffer_Overlay(sBuf, 0);
    value = parcJSON_GetValueByName(ifaceJson, cpi_IFIDX);
    unsigned ifidx = (unsigned) parcJSONValue_GetInteger(value);
    value = parcJSON_GetValueByName(ifaceJson, cpi_IsLoopback);
    sBuf = parcJSONValue_GetString(value);
    const char *loopStr = parcBuffer_Overlay(sBuf, 0);
    value = parcJSON_GetValueByName(ifaceJson, cpi_Multicast);
    sBuf = parcJSONValue_GetString(value);
    const char *mcastStr = parcBuffer_Overlay(sBuf, 0);
    value = parcJSON_GetValueByName(ifaceJson, cpi_MTU);
    unsigned mtu = (unsigned) parcJSONValue_GetInteger(value);
    value = parcJSON_GetValueByName(ifaceJson, cpi_Addrs);
    PARCJSONArray *addrsJson = parcJSONValue_GetArray(value);

    bool isLoopback = (strcasecmp(loopStr, cpi_True) == 0);
    bool supportsMulticast = (strcasecmp(mcastStr, cpi_True) == 0);

    CPIInterface *iface = cpiInterface_Create(name, ifidx, isLoopback, supportsMulticast, mtu);

    CPIAddressList *addrs = cpiAddressList_CreateFromJson(addrsJson);
    for (size_t i = 0; i < cpiAddressList_Length(addrs); i++) {
        const CPIAddress *addr = cpiAddressList_GetItem(addrs, i);
        cpiInterface_AddAddress(iface, cpiAddress_Copy(addr));
    }

    cpiAddressList_Destroy(&addrs);
    return iface;
}

const char *
cpiInterface_GetName(const CPIInterface *iface)
{
    assertNotNull(iface, "Parameter iface must be non-null");
    return iface->name;
}

unsigned
cpiInterface_GetMTU(const CPIInterface *iface)
{
    assertNotNull(iface, "Parameter iface must be non-null");
    return iface->mtu;
}

