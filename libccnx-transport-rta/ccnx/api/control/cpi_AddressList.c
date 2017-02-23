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

#include <LongBow/runtime.h>

#include <ccnx/api/control/cpi_AddressList.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Buffer.h>

struct cpi_addresslist {
    PARCArrayList *listOfCPIAddress;
};

static void
_cpiAddressList_FreeAddress(void **addressVoidPtr)
{
    CPIAddress **addressPtr = (CPIAddress **) addressVoidPtr;
    cpiAddress_Destroy(addressPtr);
}

CPIAddressList *
cpiAddressList_Create()
{
    CPIAddressList *list = parcMemory_AllocateAndClear(sizeof(CPIAddressList));
    assertNotNull(list, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIAddressList));
    list->listOfCPIAddress = parcArrayList_Create(_cpiAddressList_FreeAddress);
    assertNotNull(list->listOfCPIAddress, "Got null from parcArrayList_Create");

    return list;
}

void
cpiAddressList_Destroy(CPIAddressList **addressListPtr)
{
    assertNotNull(addressListPtr, "Parameter must be non-null double pointer");
    assertNotNull(*addressListPtr, "Parameter must dereference to non-null pointer");
    CPIAddressList *list = *addressListPtr;

    parcArrayList_Destroy(&list->listOfCPIAddress);
    parcMemory_Deallocate((void **) &list);
    *addressListPtr = NULL;
}

CPIAddressList *
cpiAddressList_Append(CPIAddressList *list, CPIAddress *address)
{
    assertNotNull(list, "Parameter list must be non-null");
    assertNotNull(address, "Parameter address must be non-null");

    parcArrayList_Add(list->listOfCPIAddress, (PARCObject *) address);
    return list;
}

CPIAddressList *
cpiAddressList_Copy(const CPIAddressList *original)
{
    assertNotNull(original, "Parameter must be non-null");

    CPIAddressList *copy = cpiAddressList_Create();
    for (int i = 0; i < parcArrayList_Size(original->listOfCPIAddress); i++) {
        CPIAddress *address = (CPIAddress *) parcArrayList_Get(original->listOfCPIAddress, i);
        parcArrayList_Add(copy->listOfCPIAddress, (PARCObject *) cpiAddress_Copy(address));
    }

    return copy;
}

bool
cpiAddressList_Equals(const CPIAddressList *a, const CPIAddressList *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");

    if (a == b) {
        return true;
    }

    if (parcArrayList_Size(a->listOfCPIAddress) != parcArrayList_Size(b->listOfCPIAddress)) {
        return false;
    }

    for (size_t i = 0; i < parcArrayList_Size(a->listOfCPIAddress); i++) {
        const CPIAddress *addr_a = (CPIAddress *) parcArrayList_Get(a->listOfCPIAddress, i);
        const CPIAddress *addr_b = (CPIAddress *) parcArrayList_Get(b->listOfCPIAddress, i);
        if (!cpiAddress_Equals(addr_a, addr_b)) {
            return false;
        }
    }
    return true;
}

size_t
cpiAddressList_Length(const CPIAddressList *list)
{
    assertNotNull(list, "Parameter must be non-null");
    return parcArrayList_Size(list->listOfCPIAddress);
}

const CPIAddress *
cpiAddressList_GetItem(const CPIAddressList *list, size_t item)
{
    assertNotNull(list, "Parameter must be non-null");
    assertTrue(item < cpiAddressList_Length(list), "Asked for item %zu beyond end of list %zu", item, cpiAddressList_Length(list));

    return (CPIAddress *) parcArrayList_Get(list->listOfCPIAddress, item);
}

/**
 * Returns a JSON array of the addresses
 *
 *   { [ {addr0}, {addr1}, ..., {addrN} ] }
 *
 * @param <#param1#>
 * @return A JSON array, even if array empty
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSONArray *
cpiAddressList_ToJson(const CPIAddressList *list)
{
    assertNotNull(list, "Parameter must be non-null");
    PARCJSONArray *array = parcJSONArray_Create();

    for (size_t i = 0; i < cpiAddressList_Length(list); i++) {
        const CPIAddress *addr = cpiAddressList_GetItem(list, i);
        PARCJSON *json = cpiAddress_ToJson(addr);
        PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
        parcJSON_Release(&json);
        parcJSONArray_AddValue(array, value);
        parcJSONValue_Release(&value);
    }

    return array;
}

/**
 * Creates an address list based on a JSON array
 *
 *   { [ {addr0}, {addr1}, ..., {addrN} ] }
 *
 * @param <#param1#>
 * @return An allocated address list.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

CPIAddressList *
cpiAddressList_CreateFromJson(PARCJSONArray *array)
{
    assertNotNull(array, "Parameter must be non-null");
    CPIAddressList *list = cpiAddressList_Create();

    for (size_t i = 0; i < parcJSONArray_GetLength(array); i++) {
        PARCJSONValue *value = parcJSONArray_GetValue(array, i);
        PARCJSON *addrjson = parcJSONValue_GetJSON(value);
        CPIAddress *addr = cpiAddress_CreateFromJson(addrjson);
        cpiAddressList_Append(list, addr);
    }

    return list;
}

char *
cpiAddressList_ToString(const CPIAddressList *list)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    for (size_t i = 0; i < cpiAddressList_Length(list); i++) {
        char *addressString = cpiAddress_ToString(cpiAddressList_GetItem(list, i));
        parcBufferComposer_PutString(composer, addressString);
        if (i < (cpiAddressList_Length(list) - 1)) {
            parcBufferComposer_PutString(composer, " ");
        }
        parcMemory_Deallocate((void **) &addressString);
    }

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(buffer);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);

    return result;
}
