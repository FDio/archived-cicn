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

#include <ccnx/api/control/cpi_InterfaceSet.h>

#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Memory.h>

#include <LongBow/runtime.h>

struct cpi_interface_set {
    PARCArrayList *listOfInterfaces;
};

static void
_destroyInterface(void **ifaceVoidPtr)
{
    cpiInterface_Destroy((CPIInterface **) ifaceVoidPtr);
}

CPIInterfaceSet *
cpiInterfaceSet_Create(void)
{
    CPIInterfaceSet *set = parcMemory_AllocateAndClear(sizeof(CPIInterfaceSet));
    assertNotNull(set, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceSet));
    set->listOfInterfaces = parcArrayList_Create(_destroyInterface);
    return set;
}

void
cpiInterfaceSet_Destroy(CPIInterfaceSet **setPtr)
{
    assertNotNull(setPtr, "Parameter must be non-null double pointer");
    assertNotNull(*setPtr, "Parameter must dereference to non-null pointer");

    CPIInterfaceSet *set = *setPtr;
    parcArrayList_Destroy(&set->listOfInterfaces);
    parcMemory_Deallocate((void **) &set);
    *setPtr = NULL;
}

bool
cpiInterfaceSet_Add(CPIInterfaceSet *set, CPIInterface *iface)
{
    assertNotNull(set, "Parameter set must be non-null");
    assertNotNull(iface, "Parameter iface must be non-null");

    unsigned ifaceIndex = cpiInterface_GetInterfaceIndex(iface);
    size_t length = parcArrayList_Size(set->listOfInterfaces);
    for (size_t i = 0; i < length; i++) {
        CPIInterface *listEntry = (CPIInterface *) parcArrayList_Get(set->listOfInterfaces, i);
        unsigned entryInterfaceIndex = cpiInterface_GetInterfaceIndex(listEntry);
        if (entryInterfaceIndex == ifaceIndex) {
            return false;
        }
    }

    parcArrayList_Add(set->listOfInterfaces, (PARCObject *) iface);
    return true;
}

size_t
cpiInterfaceSet_Length(const CPIInterfaceSet *set)
{
    assertNotNull(set, "Parameter set must be non-null");
    return parcArrayList_Size(set->listOfInterfaces);
}

CPIInterface *
cpiInterfaceSet_GetByOrdinalIndex(CPIInterfaceSet *set, size_t ordinalIndex)
{
    assertNotNull(set, "Parameter set must be non-null");
    return (CPIInterface *) parcArrayList_Get(set->listOfInterfaces, ordinalIndex);
}

CPIInterface *
cpiInterfaceSet_GetByInterfaceIndex(const CPIInterfaceSet *set, unsigned interfaceIndex)
{
    size_t length = parcArrayList_Size(set->listOfInterfaces);
    for (size_t i = 0; i < length; i++) {
        CPIInterface *listEntry = (CPIInterface *) parcArrayList_Get(set->listOfInterfaces, i);
        unsigned entryInterfaceIndex = cpiInterface_GetInterfaceIndex(listEntry);
        if (entryInterfaceIndex == interfaceIndex) {
            return listEntry;
        }
    }
    return NULL;
}

/**
 * Uses the system name (e.g. "en0")
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return NULL if not found
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIInterface *
cpiInterfaceSet_GetByName(CPIInterfaceSet *set, const char *name)
{
    size_t length = parcArrayList_Size(set->listOfInterfaces);
    for (size_t i = 0; i < length; i++) {
        CPIInterface *listEntry = (CPIInterface *) parcArrayList_Get(set->listOfInterfaces, i);
        if (cpiInterface_NameEquals(listEntry, name)) {
            return listEntry;
        }
    }
    return NULL;
}

bool
cpiInterfaceSet_Equals(const CPIInterfaceSet *a, const CPIInterfaceSet *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    size_t length_a = parcArrayList_Size(a->listOfInterfaces);
    size_t length_b = parcArrayList_Size(b->listOfInterfaces);

    if (length_a == length_b) {
        for (size_t i = 0; i < length_a; i++) {
            CPIInterface *iface_a = (CPIInterface *) parcArrayList_Get(a->listOfInterfaces, i);

            // the set is unique by interface id, so if it exists in set b, it
            // exists there by interface id
            CPIInterface *iface_b = cpiInterfaceSet_GetByInterfaceIndex(b, cpiInterface_GetInterfaceIndex(iface_a));
            if (!cpiInterface_Equals(iface_b, iface_b)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

const char cpi_InterfaceList[] = "Interfaces";

CPIInterfaceSet *
cpiInterfaceSet_FromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(json, cpi_InterfaceList);
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpi_InterfaceList,
                  parcJSON_ToString(json));
    PARCJSONArray *ifaceSetJson = parcJSONValue_GetArray(value);

    CPIInterfaceSet *set = cpiInterfaceSet_Create();

    size_t length = parcJSONArray_GetLength(ifaceSetJson);
    for (size_t i = 0; i < length; i++) {
        value = parcJSONArray_GetValue(ifaceSetJson, i);
        PARCJSON *ifaceJson = parcJSONValue_GetJSON(value);
        CPIInterface *iface = cpiInterface_FromJson(ifaceJson);
        cpiInterfaceSet_Add(set, iface);
    }

    return set;
}

PARCJSON *
cpiInterfaceSet_ToJson(CPIInterfaceSet *set)
{
    assertNotNull(set, "Parameter must be non-null");

    PARCJSONArray *interfaceList = parcJSONArray_Create();

    size_t length = parcArrayList_Size(set->listOfInterfaces);
    for (size_t i = 0; i < length; i++) {
        CPIInterface *iface = (CPIInterface *) parcArrayList_Get(set->listOfInterfaces, i);
        PARCJSON *json = cpiInterface_ToJson(iface);
        PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
        parcJSON_Release(&json);
        parcJSONArray_AddValue(interfaceList, value);
        parcJSONValue_Release(&value);
    }

    PARCJSON *result = parcJSON_Create();
    parcJSON_AddArray(result, cpi_InterfaceList, interfaceList);
    parcJSONArray_Release(&interfaceList);

    return result;
}
