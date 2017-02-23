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

typedef enum {
    CPI_IFACE_LOOPBACK = 1,
    CPI_IFACE_ETHERNET = 2,
    CPI_IFACE_LOCALAPP = 3,
    CPI_IFACE_TUNNEL = 4,
    CPI_IFACE_GROUP = 5
} CPIInterfaceType;

typedef enum {
    CPI_IFACE_UNKNOWN = 0,
    CPI_IFACE_UP = 1,
    CPI_IFACE_DOWN = 2
} CPIInterfaceStateType;

struct cpi_interface_types_string {
    CPIInterfaceType type;
    const char *str;
} interfaceTypesArray[] = {
    { .type = CPI_IFACE_LOOPBACK, .str = "LOOPBACK" },
    { .type = CPI_IFACE_ETHERNET, .str = "ETHERNET" },
    { .type = CPI_IFACE_LOCALAPP, .str = "LOCALAPP" },
    { .type = CPI_IFACE_TUNNEL,   .str = "TUNNEL"   },
    { .type = CPI_IFACE_GROUP,    .str = "GROUP"    },
    { .type = 0,                  .str = NULL       }
};

struct cpi_interface_state_types_string {
    CPIInterfaceStateType type;
    const char *str;
} interfaceStateTypesArray[] = {
    { .type = CPI_IFACE_UNKNOWN, .str = "UNKNOWN" },
    { .type = CPI_IFACE_UP,      .str = "UP"      },
    { .type = CPI_IFACE_DOWN,    .str = "DOWN"    },
    { .type = 0,                 .str = NULL      }
};

const char *
cpiInterfaceType_ToString(CPIInterfaceType type)
{
    for (int i = 0; interfaceTypesArray[i].str != NULL; i++) {
        if (interfaceTypesArray[i].type == type) {
            return interfaceTypesArray[i].str;
        }
    }
    trapIllegalValue(type, "Unknown type: %d", type);
}

CPIInterfaceType
cpiInterfaceType_FromString(const char *str)
{
    for (int i = 0; interfaceTypesArray[i].str != NULL; i++) {
        if (strcasecmp(interfaceTypesArray[i].str, str) == 0) {
            return interfaceTypesArray[i].type;
        }
    }
    // use a LongBow trap here, instead of an assertion followed by abort().
    assertTrue(0, "Unknown stirng: %s", str);
    abort();
}

const char *
cpiInterfaceStateType_ToString(CPIInterfaceStateType type)
{
    for (int i = 0; interfaceStateTypesArray[i].str != NULL; i++) {
        if (interfaceStateTypesArray[i].type == type) {
            return interfaceStateTypesArray[i].str;
        }
    }
    trapIllegalValue(type, "Unknown type: %d", type);
}

CPIInterfaceStateType
cpiInterfaceStateType_FromString(const char *str)
{
    for (int i = 0; interfaceStateTypesArray[i].str != NULL; i++) {
        if (strcasecmp(interfaceStateTypesArray[i].str, str) == 0) {
            return interfaceStateTypesArray[i].type;
        }
    }
    assertTrue(0, "Unknown stirng: %s", str);
    abort();
}
