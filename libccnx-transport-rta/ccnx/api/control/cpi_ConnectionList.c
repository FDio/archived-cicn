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

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>

#include <ccnx/api/control/cpi_ConnectionList.h>
#include <LongBow/runtime.h>

struct cpi_connection_list {
    PARCArrayList *listOfConnections;
};

/**
 * PARCArrayList entry destroyer
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
_cpiConnectionList_ArrayDestroyer(void **voidPtr)
{
    CPIConnection **entryPtr = (CPIConnection **) voidPtr;
    cpiConnection_Release(entryPtr);
}

CPIConnectionList *
cpiConnectionList_Create()
{
    CPIConnectionList *list = parcMemory_AllocateAndClear(sizeof(CPIConnectionList));
    assertNotNull(list, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIConnectionList));
    list->listOfConnections = parcArrayList_Create(_cpiConnectionList_ArrayDestroyer);
    return list;
}

void
cpiConnectionList_Destroy(CPIConnectionList **listPtr)
{
    assertNotNull(listPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listPtr, "Parameter must dereference to non-null pointer");
    CPIConnectionList *list = *listPtr;
    parcArrayList_Destroy(&list->listOfConnections);
    parcMemory_Deallocate((void **) &list);
    *listPtr = NULL;
}

void
cpiConnectionList_Append(CPIConnectionList *list, CPIConnection *entry)
{
    assertNotNull(list, "Parameter list must be non-null");
    assertNotNull(entry, "Parameter entry must be non-null");

    parcArrayList_Add(list->listOfConnections, entry);
}

size_t
cpiConnectionList_Length(const CPIConnectionList *list)
{
    assertNotNull(list, "Parameter list must be non-null");
    return parcArrayList_Size(list->listOfConnections);
}

CPIConnection *
cpiConnectionList_Get(CPIConnectionList *list, size_t index)
{
    assertNotNull(list, "Parameter list must be non-null");
    CPIConnection *original = (CPIConnection *) parcArrayList_Get(list->listOfConnections, index);
    return cpiConnection_Copy(original);
}

bool
cpiConnectionList_Equals(const CPIConnectionList *a, const CPIConnectionList *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }

    if (parcArrayList_Size(a->listOfConnections) == parcArrayList_Size(b->listOfConnections)) {
        size_t length = parcArrayList_Size(a->listOfConnections);
        for (size_t i = 0; i < length; i++) {
            CPIConnection *tunnel_a = (CPIConnection *) parcArrayList_Get(a->listOfConnections, i);
            CPIConnection *tunnel_b = (CPIConnection *) parcArrayList_Get(b->listOfConnections, i);
            if (!cpiConnection_Equals(tunnel_a, tunnel_b)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

const char cpi_ConnectionList[] = "ConnectionList";

PARCJSON *
cpiConnectionList_ToJson(const CPIConnectionList *list)
{
    assertNotNull(list, "Parameter must be non-null");

    PARCJSONArray *inner_json = parcJSONArray_Create();

    size_t length = parcArrayList_Size(list->listOfConnections);
    for (size_t i = 0; i < length; i++) {
        CPIConnection *tunnel = (CPIConnection *) parcArrayList_Get(list->listOfConnections, i);
        PARCJSON *json = cpiConnection_ToJson(tunnel);
        PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
        parcJSON_Release(&json);
        parcJSONArray_AddValue(inner_json, value);
        parcJSONValue_Release(&value);
    }

    PARCJSON *outter_json = parcJSON_Create();
    parcJSON_AddArray(outter_json, cpi_ConnectionList, inner_json);
    parcJSONArray_Release(&inner_json);
    return outter_json;
}

CPIConnectionList *
cpiConnectionList_FromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(json, cpi_ConnectionList);
    assertNotNull(value,
                  "JSON key not found %s: %s",
                  cpi_ConnectionList,
                  parcJSON_ToString(json));
    PARCJSONArray *tunnelListJson = parcJSONValue_GetArray(value);

    CPIConnectionList *list = cpiConnectionList_Create();

    size_t length = parcJSONArray_GetLength(tunnelListJson);
    for (size_t i = 0; i < length; i++) {
        value = parcJSONArray_GetValue(tunnelListJson, i);
        PARCJSON *tunnelJson = parcJSONValue_GetJSON(value);
        CPIConnection *tunnel = cpiConnection_CreateFromJson(tunnelJson);
        cpiConnectionList_Append(list, tunnel);
    }

    return list;
}
