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


#include "../cpi_InterfaceEthernet.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>



LONGBOW_TEST_RUNNER(cpi_InterfaceEthernet)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_InterfaceEthernet)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_InterfaceEthernet)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceEthernet_Copy);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceEthernet_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceEthernet_GetAddresses);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceEthernet_GetIndex);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceEthernet_GetState);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceEthernet_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceEthernet_FromJSON);
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

LONGBOW_TEST_CASE(Global, cpiInterfaceEthernet_Copy)
{
    CPIAddress *addr = cpiAddress_CreateFromInterface(5);
    CPIAddressList *list = cpiAddressList_Append(cpiAddressList_Create(), addr);
    CPIInterfaceEthernet *ethernet = cpiInterfaceEthernet_Create(1, list);

    CPIInterfaceEthernet *copy = cpiInterfaceEthernet_Copy(ethernet);

    assertTrue(cpiInterfaceEthernet_GetIndex(copy) == cpiInterfaceEthernet_GetIndex(ethernet),
               "ifidx did not match, expected %u got %u",
               cpiInterfaceEthernet_GetIndex(ethernet),
               cpiInterfaceEthernet_GetIndex(copy));

    assertTrue(cpiInterfaceEthernet_GetState(copy) == cpiInterfaceEthernet_GetState(ethernet),
               "states did not match, expected %d got %d",
               cpiInterfaceEthernet_GetState(ethernet),
               cpiInterfaceEthernet_GetState(copy));

    assertTrue(cpiAddressList_Equals(cpiInterfaceEthernet_GetAddresses(copy), cpiInterfaceEthernet_GetAddresses(ethernet)), "did not get same addresses");

    cpiInterfaceEthernet_Destroy(&copy);
    cpiInterfaceEthernet_Destroy(&ethernet);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceEthernet_Create_Destroy)
{
    CPIInterfaceEthernet *ethernet = cpiInterfaceEthernet_Create(1, cpiAddressList_Create());
    cpiInterfaceEthernet_Destroy(&ethernet);

    assertTrue(parcMemory_Outstanding() == 0, "Imbalance after destroying");
}

LONGBOW_TEST_CASE(Global, cpiInterfaceEthernet_GetAddresses)
{
    CPIAddress *addr = cpiAddress_CreateFromInterface(5);
    CPIAddressList *list = cpiAddressList_Append(cpiAddressList_Create(), addr);
    CPIInterfaceEthernet *ethernet = cpiInterfaceEthernet_Create(1, list);

    const CPIAddressList *test = cpiInterfaceEthernet_GetAddresses(ethernet);
    assertTrue(cpiAddressList_Equals(list, test), "Address lists did not match");

    cpiInterfaceEthernet_Destroy(&ethernet);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceEthernet_GetIndex)
{
    CPIAddress *addr = cpiAddress_CreateFromInterface(5);
    CPIAddressList *list = cpiAddressList_Append(cpiAddressList_Create(), addr);
    CPIInterfaceEthernet *ethernet = cpiInterfaceEthernet_Create(1, list);

    assertTrue(cpiInterfaceEthernet_GetIndex(ethernet) == 1, "ifidx did not match");

    cpiInterfaceEthernet_Destroy(&ethernet);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceEthernet_GetState)
{
    CPIAddress *addr = cpiAddress_CreateFromInterface(5);
    CPIAddressList *list = cpiAddressList_Append(cpiAddressList_Create(), addr);
    CPIInterfaceEthernet *ethernet = cpiInterfaceEthernet_Create(1, list);

    assertTrue(cpiInterfaceEthernet_GetState(ethernet) == CPI_IFACE_UNKNOWN, "state did not match");

    cpiInterfaceEthernet_SetState(ethernet, CPI_IFACE_UP);
    assertTrue(cpiInterfaceEthernet_GetState(ethernet) == CPI_IFACE_UP, "state did not match");

    cpiInterfaceEthernet_SetState(ethernet, CPI_IFACE_DOWN);
    assertTrue(cpiInterfaceEthernet_GetState(ethernet) == CPI_IFACE_DOWN, "state did not match");

    cpiInterfaceEthernet_Destroy(&ethernet);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceEthernet_ToJSON)
{
    char truth_json_str[] = "{\"ETHERNET\":{\"IFIDX\":1,\"ADDRS\":[{\"ADDRESSTYPE\":\"IFACE\",\"DATA\":\"AAAABQ==\"},{\"ADDRESSTYPE\":\"IFACE\",\"DATA\":\"AAAADw==\"}]}}";

    CPIAddress *addr5 = cpiAddress_CreateFromInterface(5);
    CPIAddress *addr15 = cpiAddress_CreateFromInterface(15);
    CPIAddressList *list = cpiAddressList_Append(cpiAddressList_Append(cpiAddressList_Create(), addr5), addr15);

    CPIInterfaceEthernet *ethernet = cpiInterfaceEthernet_Create(1, list);

    PARCJSON *test_json = cpiInterfaceEthernet_ToJson(ethernet);
    char *test_json_str = parcJSON_ToCompactString(test_json);
    assertTrue(strcmp(truth_json_str, test_json_str) == 0, "JSON strings do not match");

    parcMemory_Deallocate((void **) &test_json_str);
    parcJSON_Release(&test_json);
    cpiInterfaceEthernet_Destroy(&ethernet);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceEthernet_FromJSON)
{
    char truth_json_str[] = "{\"ETHERNET\":{\"IFIDX\":1,\"STATE\":\"UP\",\"ADDRS\":[{\"ADDRESSTYPE\":\"IFACE\",\"DATA\":\"AAAABQ==\"},{\"ADDRESSTYPE\":\"IFACE\",\"DATA\":\"AAAADw==\"}]}}";

    CPIAddress *addr5 = cpiAddress_CreateFromInterface(5);
    CPIAddress *addr15 = cpiAddress_CreateFromInterface(15);
    CPIAddressList *list = cpiAddressList_Append(cpiAddressList_Append(cpiAddressList_Create(), addr5), addr15);

    CPIInterfaceEthernet *truth = cpiInterfaceEthernet_Create(1, list);
    cpiInterfaceEthernet_SetState(truth, CPI_IFACE_UP);

    PARCJSON *json = parcJSON_ParseString(truth_json_str);

    CPIInterfaceEthernet *test = cpiInterfaceEthernet_CreateFromJson(json);
    assertTrue(cpiInterfaceEthernet_Equals(truth, test), "Ethernet interfaces do not match");

    parcJSON_Release(&json);
    cpiInterfaceEthernet_Destroy(&truth);
    cpiInterfaceEthernet_Destroy(&test);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_InterfaceEthernet);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
