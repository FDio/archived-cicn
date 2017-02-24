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
#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>

#include <ccnx/api/control/cpi_InterfaceIPTunnelList.h>
#include <LongBow/runtime.h>

struct cpi_interface_iptunnel_list {
    PARCArrayList *listOfTunnels;
};

/**
 * PARCArrayList entry destroyer
 */
static void
_arrayDestroyer(void **voidPtr)
{
    CPIInterfaceIPTunnel **entryPtr = (CPIInterfaceIPTunnel **) voidPtr;
    cpiInterfaceIPTunnel_Release(entryPtr);
}

CPIInterfaceIPTunnelList *
cpiInterfaceIPTunnelList_Create(void)
{
    CPIInterfaceIPTunnelList *list = parcMemory_AllocateAndClear(sizeof(CPIInterfaceIPTunnelList));
    assertNotNull(list, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceIPTunnelList));
    list->listOfTunnels = parcArrayList_Create(_arrayDestroyer);
    return list;
}

void
cpiInterfaceIPTunnelList_Destroy(CPIInterfaceIPTunnelList **listPtr)
{
    assertNotNull(listPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listPtr, "Parameter must dereference to non-null pointer");
    CPIInterfaceIPTunnelList *list = *listPtr;
    parcArrayList_Destroy(&list->listOfTunnels);
    parcMemory_Deallocate((void **) &list);
    *listPtr = NULL;
}

void
cpiInterfaceIPTunnelList_Append(CPIInterfaceIPTunnelList *list, CPIInterfaceIPTunnel *entry)
{
    assertNotNull(list, "Parameter list must be non-null");
    assertNotNull(entry, "Parameter entry must be non-null");

    parcArrayList_Add(list->listOfTunnels, (PARCObject *) entry);
}

size_t
cpiInterfaceIPTunnelList_Length(const CPIInterfaceIPTunnelList *list)
{
    assertNotNull(list, "Parameter list must be non-null");
    return parcArrayList_Size(list->listOfTunnels);
}

CPIInterfaceIPTunnel *
cpiInterfaceIPTunnelList_Get(CPIInterfaceIPTunnelList *list, size_t index)
{
    assertNotNull(list, "Parameter list must be non-null");
    CPIInterfaceIPTunnel *original = (CPIInterfaceIPTunnel *) parcArrayList_Get(list->listOfTunnels, index);
    return cpiInterfaceIPTunnel_Copy(original);
}

bool
cpiInterfaceIPTunnelList_Equals(const CPIInterfaceIPTunnelList *a, const CPIInterfaceIPTunnelList *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }

    if (parcArrayList_Size(a->listOfTunnels) == parcArrayList_Size(b->listOfTunnels)) {
        size_t length = parcArrayList_Size(a->listOfTunnels);
        for (size_t i = 0; i < length; i++) {
            CPIInterfaceIPTunnel *tunnel_a = (CPIInterfaceIPTunnel *) parcArrayList_Get(a->listOfTunnels, i);
            CPIInterfaceIPTunnel *tunnel_b = (CPIInterfaceIPTunnel *) parcArrayList_Get(b->listOfTunnels, i);
            if (!cpiInterfaceIPTunnel_Equals(tunnel_a, tunnel_b)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

const char cpi_InterfaceIPTunnelList[] = "TunnelList";

PARCJSON *
cpiInterfaceIPTunnelList_ToJson(const CPIInterfaceIPTunnelList *list)
{
    assertNotNull(list, "Parameter must be non-null");

    PARCJSONArray *tunnelList = parcJSONArray_Create();

    size_t length = parcArrayList_Size(list->listOfTunnels);
    for (size_t i = 0; i < length; i++) {
        CPIInterfaceIPTunnel *tunnel = (CPIInterfaceIPTunnel *) parcArrayList_Get(list->listOfTunnels, i);
        PARCJSON *json = cpiInterfaceIPTunnel_ToJson(tunnel);
        PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
        parcJSON_Release(&json);
        parcJSONArray_AddValue(tunnelList, value);
        parcJSONValue_Release(&value);
    }

    PARCJSON *result = parcJSON_Create();
    parcJSON_AddArray(result, cpi_InterfaceIPTunnelList, tunnelList);
    parcJSONArray_Release(&tunnelList);

    return result;
}

CPIInterfaceIPTunnelList *
cpiInterfaceIPTunnelList_FromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(json, cpi_InterfaceIPTunnelList);
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpi_InterfaceIPTunnelList,
                  parcJSON_ToString(json));
    PARCJSONArray *tunnelListJson = parcJSONValue_GetArray(value);

    CPIInterfaceIPTunnelList *list = cpiInterfaceIPTunnelList_Create();

    size_t length = parcJSONArray_GetLength(tunnelListJson);
    for (size_t i = 0; i < length; i++) {
        value = parcJSONArray_GetValue(tunnelListJson, i);
        CPIInterfaceIPTunnel *tunnel =
            cpiInterfaceIPTunnel_CreateFromJson(parcJSONValue_GetJSON(value));

        cpiInterfaceIPTunnelList_Append(list, tunnel);
    }

    return list;
}
