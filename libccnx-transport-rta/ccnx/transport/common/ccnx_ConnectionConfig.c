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
 * These are subsystems instantiated within components
 * They define per-connection behavior, not stack structure.
 *
 */
#include <config.h>
#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <ccnx/transport/common/ccnx_TransportConfig.h>

struct ccnx_connection_config {
    PARCJSON *connjson;
};

bool
ccnxConnectionConfig_IsValid(const CCNxConnectionConfig *config)
{
    bool result = false;
    if (config != NULL) {
        result = true;
    }
    return result;
}

void
ccnxConnectionConfig_AssertValid(const CCNxConnectionConfig *config)
{
    assertTrue(ccnxConnectionConfig_IsValid(config), "CCNxConnectionConfig instance is invalid.");
}

CCNxConnectionConfig *
ccnxConnectionConfig_Create(void)
{
    CCNxConnectionConfig *config = parcMemory_AllocateAndClear(sizeof(CCNxConnectionConfig));
    assertNotNull(config, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CCNxConnectionConfig));
    config->connjson = parcJSON_Create();
    return config;
}

void
ccnxConnectionConfig_Destroy(CCNxConnectionConfig **connectionConfigPtr)
{
    assertNotNull(connectionConfigPtr, "Parameter must be non-null double pointer");

    CCNxConnectionConfig *config = *connectionConfigPtr;
    ccnxConnectionConfig_OptionalAssertValid(config);

    parcJSON_Release(&config->connjson);
    parcMemory_Deallocate((void **) &config);
    *connectionConfigPtr = NULL;
}

PARCJSON *
ccnxConnectionConfig_GetJson(const CCNxConnectionConfig *config)
{
    ccnxConnectionConfig_OptionalAssertValid(config);

    return config->connjson;
}

CCNxConnectionConfig *
ccnxConnectionConfig_Add(CCNxConnectionConfig *config, const char *key, PARCJSONValue *componentJson)
{
    ccnxConnectionConfig_OptionalAssertValid(config);

    parcJSON_AddValue(config->connjson, key, componentJson);
    return config;
}

CCNxConnectionConfig *
ccnxConnectionConfig_Copy(const CCNxConnectionConfig *original)
{
    ccnxConnectionConfig_OptionalAssertValid(original);

    CCNxConnectionConfig *copy = parcMemory_AllocateAndClear(sizeof(CCNxConnectionConfig));
    assertNotNull(copy, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CCNxConnectionConfig));
    copy->connjson = parcJSON_Copy(original->connjson);
    return copy;
}

bool
ccnxConnectionConfig_Equals(const CCNxConnectionConfig *x, const CCNxConnectionConfig *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        result = parcJSON_Equals(x->connjson, y->connjson);
    }

    return result;
}

void
ccnxConnectionConfig_Display(const CCNxConnectionConfig *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "ConnectionConfig@%p {", instance);
    PARCJSON *json = ccnxConnectionConfig_GetJson(instance);

    parcJSON_Display(json, indentation + 1);
    parcDisplayIndented_PrintLine(indentation, "}");
}
