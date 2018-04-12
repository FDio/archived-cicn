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

#include <parc/security/parc_KeyType.h>

static struct {
    PARCKeyType type;
    char *name;
} _keyType_ToString[] = {
    { PARCKeyType_RSA, "PARCKeyType_RSA" },
    { PARCKeyType_EC, "PARCKeyType_EC" },
    { 0, NULL                            }
};

const char *
parcKeyType_ToString(PARCKeyType type)
{
    for (int i = 0; _keyType_ToString[i].name != NULL; i++) {
        if (_keyType_ToString[i].type == type) {
            return _keyType_ToString[i].name;
        }
    }
    return NULL;
}

PARCKeyType
parcKeyType_FromString(const char *name)
{
    for (int i = 0; _keyType_ToString[i].name != NULL; i++) {
        if (strcmp(_keyType_ToString[i].name, name) == 0) {
            return _keyType_ToString[i].type;
        }
    }
    return PARCKeyType_Invalid;
}
