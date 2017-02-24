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
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>

#include <LongBow/runtime.h>

#include <ccnx/api/control/cpi_NameRouteProtocolType.h>

struct name_route_protocol_type_s {
    const char *str;
    CPINameRouteProtocolType type;
} nameRouteProtocolTypeString[] = {
    { .str = "LOCAL",     .type = cpiNameRouteProtocolType_LOCAL     },
    { .str = "CONNECTED", .type = cpiNameRouteProtocolType_CONNECTED },
    { .str = "STATIC",    .type = cpiNameRouteProtocolType_STATIC    },
    { .str = "ACORN",     .type = cpiNameRouteProtocolType_ACORN     },
    { .str = NULL,        .type = 0                                  }
};

const char *
cpiNameRouteProtocolType_ToString(CPINameRouteProtocolType type)
{
    for (int i = 0; nameRouteProtocolTypeString[i].str != NULL; i++) {
        if (nameRouteProtocolTypeString[i].type == type) {
            return nameRouteProtocolTypeString[i].str;
        }
    }
    trapIllegalValue(type, "Unknown type: %d", type);
}

CPINameRouteProtocolType
cpiNameRouteProtocolType_FromString(const char *str)
{
    for (int i = 0; nameRouteProtocolTypeString[i].str != NULL; i++) {
        if (strcasecmp(nameRouteProtocolTypeString[i].str, str) == 0) {
            return nameRouteProtocolTypeString[i].type;
        }
    }
    trapIllegalValue(type, "Unknown type name: %s", str);
}
