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

#include <ccnx/api/control/cpi_NameRouteType.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Base64.h>

#include "controlPlaneInterface.h"
#include "cpi_private.h"

struct name_route_type_s {
    const char *str;
    CPINameRouteType type;
} nameRouteTypeString[] = {
    { .str = "EXACT",   .type = cpiNameRouteType_EXACT_MATCH   },
    { .str = "LONGEST", .type = cpiNameRouteType_LONGEST_MATCH },
    { .str = "DEFAULT", .type = cpiNameRouteType_DEFAULT       },
    { .str = NULL,      .type = 0                              }
};

const char *
cpiNameRouteType_ToString(CPINameRouteType type)
{
    for (int i = 0; nameRouteTypeString[i].str != NULL; i++) {
        if (nameRouteTypeString[i].type == type) {
            return nameRouteTypeString[i].str;
        }
    }
    trapIllegalValue(type, "Unknown type: %d", type);
}

CPINameRouteType
cpiNameRouteType_FromString(const char *str)
{
    for (int i = 0; nameRouteTypeString[i].str != NULL; i++) {
        if (strcasecmp(nameRouteTypeString[i].str, str) == 0) {
            return nameRouteTypeString[i].type;
        }
    }
    trapIllegalValue(type, "Unknown type: %s", str);
}
