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
 * Common test routines for the RTA component configuration functions
 *
 */

typedef struct test_data {
    CCNxConnectionConfig *connConfig;
    CCNxStackConfig *stackConfig;
} TestData;

TestData *
testRtaConfiguration_CommonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->connConfig = ccnxConnectionConfig_Create();
    data->stackConfig = ccnxStackConfig_Create();
    return data;
}

void
testRtaConfiguration_CommonTeardown(TestData *data)
{
    ccnxStackConfig_Release(&data->stackConfig);
    ccnxConnectionConfig_Destroy(&data->connConfig);
    parcMemory_Deallocate((void **) &data);
}

void
testRtaConfiguration_ComponentName(const char * (*getname)(void), const char *truth)
{
    const char *name = getname();
    assertTrue(strcmp(name, truth) == 0,
               "Got wrong name, got %s expected %s", name, truth);
}

void
testRtaConfiguration_ConnectionJsonKey(CCNxConnectionConfig *configToTest, const char *key)
{
    PARCJSON *json = ccnxConnectionConfig_GetJson(configToTest);
    PARCJSONValue *value = parcJSON_GetValueByName(json, key);
    assertNotNull(value, "Could not find key %s in configuration json", key);
}

void
testRtaConfiguration_ProtocolStackJsonKey(CCNxStackConfig *configToTest, const char *key)
{
    PARCJSON *json = ccnxStackConfig_GetJson(configToTest);
    PARCJSONValue *value = parcJSON_GetValueByName(json, key);
    assertNotNull(value, "Could not find key %s in configuration json", key);
}
