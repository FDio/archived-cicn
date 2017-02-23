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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../config_ProtocolStack.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include "testrig_RtaConfigCommon.c"

#include <ccnx/transport/common/ccnx_TransportConfig.h>

LONGBOW_TEST_RUNNER(config_ProtocolStack)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(config_ProtocolStack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(config_ProtocolStack)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, protocolStack_ComponentsConfigArgs);
    LONGBOW_RUN_TEST_CASE(Global, protocolStack_ComponentsConfigArrayList);
    LONGBOW_RUN_TEST_CASE(Global, protocolStack_GetComponentNameArray);
    LONGBOW_RUN_TEST_CASE(Global, protocolStack_GetName);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, protocolStack_ComponentsConfigArgs)
{
    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();

    const char truth[] = "{\"STACK\":{\"COMPONENTS\":[\"Apple\",\"Bananna\",\"Cherry\"]}}";

    protocolStack_ComponentsConfigArgs(stackConfig, "Apple", "Bananna", "Cherry", NULL);
    PARCJSON *json = ccnxStackConfig_GetJson(stackConfig);
    char *str = parcJSON_ToCompactString(json);
    assertTrue(strcmp(truth, str) == 0, "Got wrong config, got %s expected %s", str, truth);
    parcMemory_Deallocate((void **) &str);
    ccnxStackConfig_Release(&stackConfig);
}

LONGBOW_TEST_CASE(Global, protocolStack_ComponentsConfigArrayList)
{
    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();
    PARCArrayList *names = parcArrayList_Create(NULL);
    parcArrayList_Add(names, "Apple");
    parcArrayList_Add(names, "Bananna");
    parcArrayList_Add(names, "Cherry");

    const char truth[] = "{\"STACK\":{\"COMPONENTS\":[\"Apple\",\"Bananna\",\"Cherry\"]}}";

    protocolStack_ComponentsConfigArrayList(stackConfig, names);
    PARCJSON *json = ccnxStackConfig_GetJson(stackConfig);
    char *str = parcJSON_ToCompactString(json);
    assertTrue(strcmp(truth, str) == 0, "Got wrong config, got %s expected %s", str, truth);

    parcMemory_Deallocate((void **) &str);
    ccnxStackConfig_Release(&stackConfig);
    parcArrayList_Destroy(&names);
}

LONGBOW_TEST_CASE(Global, protocolStack_GetComponentNameArray)
{
    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();
    PARCArrayList *names = parcArrayList_Create(NULL);
    parcArrayList_Add(names, "Apple");
    parcArrayList_Add(names, "Bananna");
    parcArrayList_Add(names, "Cherry");

    protocolStack_ComponentsConfigArrayList(stackConfig, names);

    char truth[] = "{\"STACK\":{\"COMPONENTS\":[\"Apple\",\"Bananna\",\"Cherry\"]}}";
    PARCJSON *json = parcJSON_ParseString(truth);

    PARCArrayList *test = protocolStack_GetComponentNameArray(json);

    assertTrue(parcArrayList_Size(test) == parcArrayList_Size(names),
               "wrong array list size, got %zu expected %zu",
               parcArrayList_Size(test), parcArrayList_Size(names));
    for (int i = 0; i < parcArrayList_Size(test); i++) {
        char *a = parcArrayList_Get(test, i);
        char *b = parcArrayList_Get(names, i);
        assertTrue(strcmp(a, b) == 0, "mismatch elements %d, got %s expected %s", i, a, b);
    }

    ccnxStackConfig_Release(&stackConfig);
    parcArrayList_Destroy(&names);
    parcJSON_Release(&json);
    parcArrayList_Destroy(&test);
}

LONGBOW_TEST_CASE(Global, protocolStack_GetName)
{
    const char *name = protocolStack_GetName();
    assertTrue(strcmp(name, param_STACK) == 0, "Got wrong name, got %s expected %s", name, param_STACK);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(config_ProtocolStack);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
