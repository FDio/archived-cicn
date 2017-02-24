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

#include <ccnx/api/control/cpi_InterfaceGeneric.h>
#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <string.h>

struct cpi_interface_generic {
    unsigned ifidx;
    CPIInterfaceStateType state;
    CPIAddressList *addresses;
};

CPIInterfaceGeneric *
cpiInterfaceGeneric_Create(unsigned ifidx, CPIAddressList *addresses)
{
    assertNotNull(addresses, "Parameter addresses must be non-null");

    CPIInterfaceGeneric *generic = parcMemory_AllocateAndClear(sizeof(CPIInterfaceGeneric));
    assertNotNull(generic, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceGeneric));
    generic->ifidx = ifidx;
    generic->state = CPI_IFACE_UNKNOWN;
    generic->addresses = addresses;
    return generic;
}

CPIInterfaceGeneric *
cpiInterfaceGeneric_Copy(const CPIInterfaceGeneric *original)
{
    assertNotNull(original, "Parameter original must be non-null");

    CPIInterfaceGeneric *generic = parcMemory_AllocateAndClear(sizeof(CPIInterfaceGeneric));
    assertNotNull(generic, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CPIInterfaceGeneric));
    generic->ifidx = original->ifidx;
    generic->state = original->state;
    generic->addresses = cpiAddressList_Copy(original->addresses);
    return generic;
}

void
cpiInterfaceGeneric_Destroy(CPIInterfaceGeneric **genericPtr)
{
    assertNotNull(genericPtr, "Parameter must be non-null double pointer");
    assertNotNull(*genericPtr, "Parameter must dereference to non-null pointer");

    CPIInterfaceGeneric *generic = *genericPtr;
    cpiAddressList_Destroy(&generic->addresses);
    parcMemory_Deallocate((void **) &generic);
    *genericPtr = NULL;
}

void
cpiInterfaceGeneric_SetState(CPIInterfaceGeneric *generic, CPIInterfaceStateType state)
{
    assertNotNull(generic, "Parameter must be non-null pointer");
    generic->state = state;
}

unsigned
cpiInterfaceGeneric_GetIndex(const CPIInterfaceGeneric *generic)
{
    assertNotNull(generic, "Parameter must be non-null pointer");
    return generic->ifidx;
}

const CPIAddressList *
cpiInterfaceGeneric_GetAddresses(const CPIInterfaceGeneric *generic)
{
    assertNotNull(generic, "Parameter must be non-null pointer");
    return generic->addresses;
}

CPIInterfaceStateType
cpiInterfaceGeneric_GetState(const CPIInterfaceGeneric *generic)
{
    assertNotNull(generic, "Parameter must be non-null pointer");
    return generic->state;
}

bool
cpiInterfaceGeneric_Equals(const CPIInterfaceGeneric *a, const CPIInterfaceGeneric *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");

    if (a == b) {
        return true;
    }

    if (a->ifidx == b->ifidx) {
        if (a->state == b->state) {
            if (cpiAddressList_Equals(a->addresses, b->addresses)) {
                return true;
            }
        }
    }
    return false;
}

PARCBufferComposer *
cpiInterfaceGeneric_BuildString(const CPIInterfaceGeneric *interface, PARCBufferComposer *composer)
{
    char *addressString = cpiAddressList_ToString(interface->addresses);
    parcBufferComposer_Format(composer, "%5d %4s %s",
                              interface->ifidx,
                              cpiInterfaceStateType_ToString(interface->state),
                              addressString
                              );
    parcMemory_Deallocate((void **) &addressString);
    return composer;
}
