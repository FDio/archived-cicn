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
#include <LongBow/runtime.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <ccnx/transport/common/ccnx_StackConfig.h>

struct CCNxStackConfig_ {
    PARCJSON *stackjson;
};

static void
_ccnxStackConfig_Finalize(CCNxStackConfig **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a CCNxStackConfig pointer.");

    CCNxStackConfig *instance = *instancePtr;
    ccnxStackConfig_OptionalAssertValid(instance);

    parcJSON_Release(&instance->stackjson);
}

parcObject_ImplementAcquire(ccnxStackConfig, CCNxStackConfig);

parcObject_ImplementRelease(ccnxStackConfig, CCNxStackConfig);

parcObject_ExtendPARCObject(CCNxStackConfig, _ccnxStackConfig_Finalize, ccnxStackConfig_Copy, ccnxStackConfig_ToString, ccnxStackConfig_Equals, NULL, ccnxStackConfig_HashCode, ccnxStackConfig_ToJSON);

void
ccnxStackConfig_AssertValid(const CCNxStackConfig *instance)
{
    assertTrue(ccnxStackConfig_IsValid(instance),
               "CCNxStackConfig is not valid.");
}

CCNxStackConfig *
ccnxStackConfig_Create(void)
{
    CCNxStackConfig *result = parcObject_CreateInstance(CCNxStackConfig);
    if (result != NULL) {
        result->stackjson = parcJSON_Create();
    }

    return result;
}

CCNxStackConfig *
ccnxStackConfig_Copy(const CCNxStackConfig *original)
{
    ccnxStackConfig_OptionalAssertValid(original);

    CCNxStackConfig *result = parcObject_CreateInstance(CCNxStackConfig);

    result->stackjson = parcJSON_Copy(original->stackjson);

    return result;
}

void
ccnxStackConfig_Display(const CCNxStackConfig *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "CCNxStackConfig@%p {", instance);
    PARCJSON *json = ccnxStackConfig_GetJson(instance);

    parcJSON_Display(json, indentation + 1);
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
ccnxStackConfig_Equals(const CCNxStackConfig *x, const CCNxStackConfig *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        result = parcJSON_Equals(x->stackjson, y->stackjson);
    }

    return result;
}

bool
ccnxStackConfig_IsValid(const CCNxStackConfig *instance)
{
    bool result = false;
    if (instance != NULL) {
        result = true;
    }
    return result;
}

PARCJSON *
ccnxStackConfig_ToJSON(const CCNxStackConfig *instance)
{
    ccnxStackConfig_OptionalAssertValid(instance);

    return instance->stackjson;
}

char *
ccnxStackConfig_ToString(const CCNxStackConfig *instance)
{
    PARCJSON *json = ccnxStackConfig_ToJSON(instance);

    char *result = parcJSON_ToString(json);

    return result;
}

PARCJSONValue *
ccnxStackConfig_Get(const CCNxStackConfig *config, const char *componentKey)
{
    ccnxStackConfig_OptionalAssertValid(config);
    PARCJSONValue *value = parcJSON_GetValueByName(config->stackjson, componentKey);
    return value;
}

PARCHashCode
ccnxStackConfig_HashCode(const CCNxStackConfig *config)
{
    ccnxStackConfig_OptionalAssertValid(config);
    return parcJSON_HashCode(config->stackjson);
}

CCNxStackConfig *
ccnxStackConfig_Add(CCNxStackConfig *config, const char *componentKey, PARCJSONValue *jsonObject)
{
    ccnxStackConfig_OptionalAssertValid(config);

    parcJSON_AddValue(config->stackjson, componentKey, jsonObject);
    return config;
}

PARCJSON *
ccnxStackConfig_GetJson(const CCNxStackConfig *config)
{
    ccnxStackConfig_OptionalAssertValid(config);

    return (config->stackjson);
}
