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

#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_JSON.h>

#include <ccnx/api/control/cpi_InterfaceIPTunnel.h>
#include <ccnx/api/control/cpi_InterfaceGeneric.h>

#define SOURCE_INDEX 0
#define DESTINATION_INDEX 1

const static char cpiIFIDX[] = "IFIDX";
const static char cpiSRCADDR[] = "SRC";
const static char cpiDSTADDR[] = "DST";
const static char cpiTUNTYPE[] = "TUNTYPE";
const static char cpiSTATE[] = "STATE";
const static char cpiSYMBOLIC[] = "SYMBOLIC";

struct cpi_interface_iptun {
    CPIInterfaceGeneric *generic;
    CPIInterfaceIPTunnelType tunnelType;
    char *symbolic;
};

struct iptunnel_type_string_s {
    CPIInterfaceIPTunnelType type;
    const char *str;
} iptunnelTypeStrings[] = {
    { .type = IPTUN_UDP, .str = "UDP" },
    { .type = IPTUN_TCP, .str = "TCP" },
    { .type = IPTUN_GRE, .str = "GRE" },
    { .type = 0,         .str = NULL  },
};


static void
_cpiInterfaceIPTunnel_Destroy(CPIInterfaceIPTunnel **iptunPtr)
{
    assertNotNull(iptunPtr, "Parameter must be non-null double pointer");
    assertNotNull(*iptunPtr, "Parameter must dereference to non-null pointer");

    CPIInterfaceIPTunnel *iptun = *iptunPtr;
    cpiInterfaceGeneric_Destroy(&iptun->generic);
    parcMemory_Deallocate((void **) &iptun->symbolic);
}

parcObject_ExtendPARCObject(CPIInterfaceIPTunnel, _cpiInterfaceIPTunnel_Destroy, cpiInterfaceIPTunnel_Copy, NULL, cpiInterfaceIPTunnel_Equals, NULL, NULL, cpiInterfaceIPTunnel_ToJson);

parcObject_ImplementRelease(cpiInterfaceIPTunnel, CPIInterfaceIPTunnel);

parcObject_ImplementAcquire(cpiInterfaceIPTunnel, CPIInterfaceIPTunnel);

const char *
cpiInterfaceIPTunnel_TypeToString(CPIInterfaceIPTunnelType type)
{
    for (int i = 0; iptunnelTypeStrings[i].str != NULL; i++) {
        if (iptunnelTypeStrings[i].type == type) {
            return iptunnelTypeStrings[i].str;
        }
    }
    assertTrue(0, "Unknown type: %d", type);
    abort();
}

CPIInterfaceIPTunnelType
cpiInterfaceIPTunnel_TypeFromString(const char *str)
{
    for (int i = 0; iptunnelTypeStrings[i].str != NULL; i++) {
        if (strcasecmp(iptunnelTypeStrings[i].str, str) == 0) {
            return iptunnelTypeStrings[i].type;
        }
    }
    assertTrue(0, "Unknown stirng: %s", str);
    abort();
}

CPIInterfaceIPTunnel *
cpiInterfaceIPTunnel_Create(unsigned ifidx, CPIAddress *source, CPIAddress *destination, CPIInterfaceIPTunnelType tunnelType, const char *symbolic)
{
    assertNotNull(source, "Parameter source must be non-null");
    assertNotNull(destination, "Parameter destination must be non-null");

    assertTrue(cpiAddress_GetType(source) == cpiAddressType_INET || cpiAddress_GetType(source) == cpiAddressType_INET6,
               "source address unsupported type: %d",
               cpiAddress_GetType(source));

    assertTrue(cpiAddress_GetType(destination) == cpiAddressType_INET || cpiAddress_GetType(destination) == cpiAddressType_INET6,
               "destination address unsupported type: %d",
               cpiAddress_GetType(destination));

    CPIInterfaceIPTunnel *iptun = parcObject_CreateInstance(CPIInterfaceIPTunnel);
    assertNotNull(iptun, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceIPTunnel));

    CPIAddressList *addrlist = cpiAddressList_Create();
    cpiAddressList_Append(addrlist, source);
    cpiAddressList_Append(addrlist, destination);

    iptun->generic = cpiInterfaceGeneric_Create(ifidx, addrlist);
    iptun->tunnelType = tunnelType;
    iptun->symbolic = parcMemory_StringDuplicate(symbolic, strlen(symbolic));

    return iptun;
}

CPIInterfaceIPTunnel *
cpiInterfaceIPTunnel_Copy(const CPIInterfaceIPTunnel *original)
{
    assertNotNull(original, "Parameter original must be non-null");
    CPIInterfaceIPTunnel *iptun = parcObject_CreateInstance(CPIInterfaceIPTunnel);
    assertNotNull(iptun, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceIPTunnel));
    iptun->generic = cpiInterfaceGeneric_Copy(original->generic);
    iptun->tunnelType = original->tunnelType;
    iptun->symbolic = parcMemory_StringDuplicate(original->symbolic, strlen(original->symbolic));
    return iptun;
}

void
cpiInterfaceIPTunnel_SetState(CPIInterfaceIPTunnel *iptun, CPIInterfaceStateType state)
{
    assertNotNull(iptun, "Parameter must be non-null");
    cpiInterfaceGeneric_SetState(iptun->generic, state);
}

const char *
cpiInterfaceIPTunnel_GetSymbolicName(const CPIInterfaceIPTunnel *iptun)
{
    assertNotNull(iptun, "Parameter must be non-null");
    return iptun->symbolic;
}

unsigned
cpiInterfaceIPTunnel_GetIndex(const CPIInterfaceIPTunnel *iptun)
{
    assertNotNull(iptun, "Parameter must be non-null");
    return cpiInterfaceGeneric_GetIndex(iptun->generic);
}

const CPIAddress *
cpiInterfaceIPTunnel_GetSourceAddress(const CPIInterfaceIPTunnel *iptun)
{
    assertNotNull(iptun, "Parameter must be non-null");
    const CPIAddressList *addrs = cpiInterfaceGeneric_GetAddresses(iptun->generic);
    return cpiAddressList_GetItem(addrs, SOURCE_INDEX);
}

const CPIAddress *
cpiInterfaceIPTunnel_GetDestinationAddress(const CPIInterfaceIPTunnel *iptun)
{
    assertNotNull(iptun, "Parameter must be non-null");
    const CPIAddressList *addrs = cpiInterfaceGeneric_GetAddresses(iptun->generic);
    return cpiAddressList_GetItem(addrs, DESTINATION_INDEX);
}

CPIInterfaceIPTunnelType
cpiInterfaceIPTunnel_GetTunnelType(const CPIInterfaceIPTunnel *iptun)
{
    assertNotNull(iptun, "Parameter must be non-null");
    return iptun->tunnelType;
}

CPIInterfaceStateType
cpiInterfaceIPTunnel_GetState(const CPIInterfaceIPTunnel *iptun)
{
    assertNotNull(iptun, "Parameter must be non-null");
    return cpiInterfaceGeneric_GetState(iptun->generic);
}

bool
cpiInterfaceIPTunnel_Equals(const CPIInterfaceIPTunnel *a, const CPIInterfaceIPTunnel *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");

    if (a->tunnelType == b->tunnelType) {
        if (cpiInterfaceGeneric_Equals(a->generic, b->generic)) {
            if (strcasecmp(a->symbolic, b->symbolic) == 0) {
                return true;
            }
        }
    }
    return false;
}

/**
 * JSON representation
 *
 * <code>
 * { "TUNNEL" :
 * { "IFIDX" : ifidx,
 *  "SYMBOLIC" : "tun3",
 * ["STATE" : "UP" | "DOWN", ]
 * "TYPE": "UDP" | "TCP" | "GRE",
 * "SRC" : {srcaddr},
 * "DST" : {dstaddr}
 * }
 * }
 * </code>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *
cpiInterfaceIPTunnel_ToJson(const CPIInterfaceIPTunnel *iptun)
{
    assertNotNull(iptun, "Parameter must be non-null");

    PARCJSON *inner_json = parcJSON_Create();

    parcJSON_AddInteger(inner_json, cpiIFIDX, cpiInterfaceIPTunnel_GetIndex(iptun));
    parcJSON_AddString(inner_json, cpiSYMBOLIC, iptun->symbolic);

    if (cpiInterfaceIPTunnel_GetState(iptun) != CPI_IFACE_UNKNOWN) {
        parcJSON_AddString(inner_json, cpiSTATE, cpiInterfaceStateType_ToString(cpiInterfaceIPTunnel_GetState(iptun)));
    }
    parcJSON_AddString(inner_json, cpiTUNTYPE, cpiInterfaceIPTunnel_TypeToString(cpiInterfaceIPTunnel_GetTunnelType(iptun)));

    PARCJSON *json = cpiAddress_ToJson(cpiInterfaceIPTunnel_GetSourceAddress(iptun));
    parcJSON_AddObject(inner_json, cpiSRCADDR, json);
    parcJSON_Release(&json);

    json = cpiAddress_ToJson(cpiInterfaceIPTunnel_GetDestinationAddress(iptun));
    parcJSON_AddObject(inner_json, cpiDSTADDR, json);
    parcJSON_Release(&json);

    PARCJSON *outter_json = parcJSON_Create();
    parcJSON_AddObject(outter_json, cpiInterfaceType_ToString(CPI_IFACE_TUNNEL), inner_json);
    parcJSON_Release(&inner_json);

    return outter_json;
}

CPIInterfaceIPTunnel *
cpiInterfaceIPTunnel_CreateFromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(json, cpiInterfaceType_ToString(CPI_IFACE_TUNNEL));
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpiInterfaceType_ToString(CPI_IFACE_TUNNEL),
                  parcJSON_ToString(json));
    PARCObject *tunnelJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(tunnelJson, cpiIFIDX);
    assertNotNull(value, "Could not find key %s: %s", cpiIFIDX, parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsNumber(value),
               "%s is not a number: %s",
               cpiIFIDX,
               parcJSON_ToString(json));
    PARCJSONValue *ifidx_value = value;

    value = parcJSON_GetValueByName(tunnelJson, cpiSYMBOLIC);
    assertNotNull(value, "Could not find key %s: %s", cpiSYMBOLIC, parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsString(value),
               "%s is not a string: %s",
               cpiSYMBOLIC,
               parcJSON_ToString(json));
    PARCJSONValue *symbolic_value = value;

    value = parcJSON_GetValueByName(tunnelJson, cpiTUNTYPE);
    assertNotNull(value, "Could not find key %s: %s", cpiTUNTYPE, parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsString(value),
               "%s is not a number: %s",
               cpiTUNTYPE,
               parcJSON_ToString(json));
    PARCJSONValue *tuntype_value = value;

    value = parcJSON_GetValueByName(tunnelJson, cpiSRCADDR);
    assertNotNull(value, "Could not find key %s: %s", cpiSRCADDR, parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsJSON(value),
               "%s is not an array: %s",
               cpiSRCADDR,
               parcJSON_ToString(json));
    PARCJSONValue *srcaddr_value = value;

    value = parcJSON_GetValueByName(tunnelJson, cpiDSTADDR);
    assertNotNull(value, "Could not find key %s: %s", cpiDSTADDR, parcJSON_ToString(json));
    assertTrue(parcJSONValue_IsJSON(value),
               "%s is not an array: %s",
               cpiDSTADDR,
               parcJSON_ToString(json));
    PARCJSONValue *dstaddr_value = value;

    unsigned ifidx = (unsigned) parcJSONValue_GetInteger(ifidx_value);
    PARCBuffer *sBuf = parcJSONValue_GetString(symbolic_value);
    const char *symbolic = parcBuffer_Overlay(sBuf, 0);
    CPIAddress *srcaddr =
        cpiAddress_CreateFromJson(parcJSONValue_GetJSON(srcaddr_value));
    CPIAddress *dstaddr =
        cpiAddress_CreateFromJson(parcJSONValue_GetJSON(dstaddr_value));
    sBuf = parcJSONValue_GetString(tuntype_value);
    CPIInterfaceIPTunnelType tunnelType =
        cpiInterfaceIPTunnel_TypeFromString(parcBuffer_Overlay(sBuf, 0));

    CPIInterfaceIPTunnel *iptun =
        cpiInterfaceIPTunnel_Create(ifidx, srcaddr, dstaddr, tunnelType, symbolic);

    PARCJSONValue *state_value = parcJSON_GetValueByName(tunnelJson, cpiSTATE);
    if (state_value != NULL) {
        sBuf = parcJSONValue_GetString(state_value);
        cpiInterfaceIPTunnel_SetState(iptun, cpiInterfaceStateType_FromString(parcBuffer_Overlay(sBuf, 0)));
    }

    return iptun;
}
