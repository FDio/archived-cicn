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

#include <ccnx/api/control/cpi_InterfaceEthernet.h>
#include <ccnx/api/control/cpi_InterfaceGeneric.h>
#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <string.h>

const static char cpiIFIDX[] = "IFIDX";
const static char cpiADDRS[] = "ADDRS";
const static char cpiSTATE[] = "STATE";

static const char *cpiAddEtherConnection = "AddConnEther";

struct cpi_interface_ethernet {
    CPIInterfaceGeneric *generic;
};

CPIInterfaceEthernet *
cpiInterfaceEthernet_Create(unsigned ifidx, CPIAddressList *addresses)
{
    assertNotNull(addresses, "Parameter addresses must be non-null");

    CPIInterfaceEthernet *ethernet = parcMemory_AllocateAndClear(sizeof(CPIInterfaceEthernet));
    assertNotNull(ethernet, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceEthernet));
    ethernet->generic = cpiInterfaceGeneric_Create(ifidx, addresses);
    return ethernet;
}

CPIInterfaceEthernet *
cpiInterfaceEthernet_Copy(const CPIInterfaceEthernet *original)
{
    assertNotNull(original, "Parameter original must be non-null");

    CPIInterfaceEthernet *ethernet = parcMemory_AllocateAndClear(sizeof(CPIInterfaceEthernet));
    assertNotNull(ethernet, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceEthernet));
    ethernet->generic = cpiInterfaceGeneric_Copy(original->generic);
    return ethernet;
}

void
cpiInterfaceEthernet_Destroy(CPIInterfaceEthernet **ethernetPtr)
{
    assertNotNull(ethernetPtr, "Parameter must be non-null double pointer");
    assertNotNull(*ethernetPtr, "Parameter must dereference to non-null pointer");

    CPIInterfaceEthernet *ethernet = *ethernetPtr;
    cpiInterfaceGeneric_Destroy(&ethernet->generic);
    parcMemory_Deallocate((void **) &ethernet);
    *ethernetPtr = NULL;
}

void
cpiInterfaceEthernet_SetState(CPIInterfaceEthernet *ethernet, CPIInterfaceStateType state)
{
    assertNotNull(ethernet, "Parameter must be non-null pointer");
    cpiInterfaceGeneric_SetState(ethernet->generic, state);
}

unsigned
cpiInterfaceEthernet_GetIndex(const CPIInterfaceEthernet *ethernet)
{
    assertNotNull(ethernet, "Parameter must be non-null pointer");
    return cpiInterfaceGeneric_GetIndex(ethernet->generic);
}

const CPIAddressList *
cpiInterfaceEthernet_GetAddresses(const CPIInterfaceEthernet *ethernet)
{
    assertNotNull(ethernet, "Parameter must be non-null pointer");
    return cpiInterfaceGeneric_GetAddresses(ethernet->generic);
}

CPIInterfaceStateType
cpiInterfaceEthernet_GetState(const CPIInterfaceEthernet *ethernet)
{
    assertNotNull(ethernet, "Parameter must be non-null pointer");
    return cpiInterfaceGeneric_GetState(ethernet->generic);
}

PARCJSON *
cpiInterfaceEthernet_ToJson(const CPIInterfaceEthernet *ethernet)
{
    assertNotNull(ethernet, "Parameter must be non-null");

    PARCJSON *innerJson = parcJSON_Create();

    parcJSON_AddInteger(innerJson, cpiIFIDX, cpiInterfaceEthernet_GetIndex(ethernet));

    if (cpiInterfaceEthernet_GetState(ethernet) != CPI_IFACE_UNKNOWN) {
        parcJSON_AddString(innerJson,
                           cpiSTATE,
                           cpiInterfaceStateType_ToString(cpiInterfaceEthernet_GetState(ethernet)));
    }

    PARCJSONArray *addrsArray = cpiAddressList_ToJson(cpiInterfaceEthernet_GetAddresses(ethernet));
    parcJSON_AddArray(innerJson, cpiADDRS, addrsArray);
    parcJSONArray_Release(&addrsArray);

    PARCJSON *result = parcJSON_Create();
    parcJSON_AddObject(result, cpiInterfaceType_ToString(CPI_IFACE_ETHERNET), innerJson);
    parcJSON_Release(&innerJson);

    return result;
}

CPIInterfaceEthernet *
cpiInterfaceEthernet_CreateFromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiInterfaceType_ToString(CPI_IFACE_ETHERNET));
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpiInterfaceType_ToString(CPI_IFACE_ETHERNET),
                  parcJSON_ToString(json));
    PARCJSON *etherJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(etherJson, cpiIFIDX);
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpiIFIDX,
                  parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsNumber(value),
               "%s is not a number: %s",
               cpiIFIDX,
               parcJSON_ToString(json));
    unsigned ifidx = (unsigned) parcJSONValue_GetInteger(value);

    value = parcJSON_GetValueByName(etherJson, cpiADDRS);
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpiADDRS,
                  parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsArray(value),
               "%s is not a number: %s",
               cpiADDRS,
               parcJSON_ToString(json));
    PARCJSONArray *addrsJson = parcJSONValue_GetArray(value);

    CPIAddressList *addrs = cpiAddressList_CreateFromJson(addrsJson);
    CPIInterfaceEthernet *ethernet = cpiInterfaceEthernet_Create(ifidx, addrs);

    value = parcJSON_GetValueByName(etherJson, cpiSTATE);
    if (value != NULL) {
        PARCBuffer *sBuf = parcJSONValue_GetString(value);
        char *state = parcBuffer_Overlay(sBuf, 0);
        cpiInterfaceEthernet_SetState(ethernet, cpiInterfaceStateType_FromString(state));
    }

    return ethernet;
}

bool
cpiInterfaceEthernet_Equals(const CPIInterfaceEthernet *a, const CPIInterfaceEthernet *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");

    return cpiInterfaceGeneric_Equals(a->generic, b->generic);
}

const char *
cpiLinks_AddEtherConnectionJasonTag()
{
    return cpiAddEtherConnection;
}
