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
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/security/parc_ContainerEncoding.h>

static struct {
    PARCContainerEncoding type;
    char *name;
} _certificateType_ToString[] = {
    { PARCContainerEncoding_PEM,    "PARCContainerEncoding_PEM"    },
    { PARCContainerEncoding_DER,    "PARCContainerEncoding_DER"    },
    { PARCContainerEncoding_PKCS12, "PARCContainerEncoding_PKCS12" },
    { 0,                            NULL                           }
};

const char *
parcContainerEncoding_ToString(PARCContainerEncoding type)
{
    for (int i = 0; _certificateType_ToString[i].name != NULL; i++) {
        if (_certificateType_ToString[i].type == type) {
            return _certificateType_ToString[i].name;
        }
    }
    return NULL;
}

PARCContainerEncoding
parcContainerEncoding_FromString(const char *name)
{
    for (int i = 0; _certificateType_ToString[i].name != NULL; i++) {
        if (strcmp(_certificateType_ToString[i].name, name) == 0) {
            return _certificateType_ToString[i].type;
        }
    }
    return PARCContainerEncoding_Invalid;
}
