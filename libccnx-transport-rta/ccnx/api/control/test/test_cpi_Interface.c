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


#include "../cpi_Interface.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>


LONGBOW_TEST_RUNNER(cpi_Interface)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_Interface)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_Interface)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_AddAddress);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_GetAddresses);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_GetMtu);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_GetInterfaceIndex);

    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_NameEquals_IsEqual);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_NameEquals_IsNotEqual);

    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_ToJson);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_FromJson);

    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_IsEqual);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_BothNull);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_OneNull);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_UnequalName);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_UnequalIndex);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_UnequalLoopback);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_UnequalMulticast);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_UnequalMTU);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_Equals_UnequalAddresses);

    LONGBOW_RUN_TEST_CASE(Global, cpiInterface_ToString);
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

LONGBOW_TEST_CASE(Global, cpiInterface_Create_Destroy)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_Destroy(&iface);

    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Memory imbalance on create/destroy");
}

LONGBOW_TEST_CASE(Global, cpiInterface_AddAddress)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);

    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));
    assertTrue(cpiAddressList_Length(iface->addressList) == 1,
               "Incorrect address list length, expected %u got %zu",
               1,
               cpiAddressList_Length(iface->addressList));

    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(2));
    assertTrue(cpiAddressList_Length(iface->addressList) == 2,
               "Incorrect address list length, expected %u got %zu",
               2,
               cpiAddressList_Length(iface->addressList));

    cpiInterface_Destroy(&iface);
}

LONGBOW_TEST_CASE(Global, cpiInterface_GetAddresses)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);

    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(2));

    const CPIAddressList *list = cpiInterface_GetAddresses(iface);
    assertTrue(cpiAddressList_Length(list) == 2, "Incorrect list size, expected %u got %zu", 2, cpiAddressList_Length(list));
    cpiInterface_Destroy(&iface);
}

LONGBOW_TEST_CASE(Global, cpiInterface_GetMtu)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);

    unsigned test = cpiInterface_GetMTU(iface);
    assertTrue(test == 1500, "Wrong MTU expected 1500 got %u", test);
    cpiInterface_Destroy(&iface);
}

LONGBOW_TEST_CASE(Global, cpiInterface_GetInterfaceIndex)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);

    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(2));

    unsigned testvalue = cpiInterface_GetInterfaceIndex(iface);

    assertTrue(testvalue == 1, "Incorrect interfaceIndex, expected %u got %u", 1, testvalue);
    cpiInterface_Destroy(&iface);
}

LONGBOW_TEST_CASE(Global, cpiInterface_NameEquals_IsEqual)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));
    assertTrue(cpiInterface_NameEquals(iface, "eth0"), "name did not compare as equal");
    cpiInterface_Destroy(&iface);
}

LONGBOW_TEST_CASE(Global, cpiInterface_NameEquals_IsNotEqual)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));
    assertFalse(cpiInterface_NameEquals(iface, "eth2"), "Unequal names compare as equal");
    cpiInterface_Destroy(&iface);
}


LONGBOW_TEST_CASE(Global, cpiInterface_ToJson)
{
    char truth[] = "{\"Interface\":{\"Name\":\"eth0\",\"Index\":1,\"Loopback\":\"true\",\"Multicast\":\"false\",\"MTU\":1500,\"Addrs\":[{\"ADDRESSTYPE\":\"IFACE\",\"DATA\":\"AAAAAQ==\"},{\"ADDRESSTYPE\":\"IFACE\",\"DATA\":\"AAAAAg==\"}]}}";

    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(2));

    PARCJSON *json = cpiInterface_ToJson(iface);

    char *str = parcJSON_ToCompactString(json);
    assertTrue(strcmp(str, truth) == 0, "JSON mismatch, expected '%s' got '%s'", truth, str);
    parcMemory_Deallocate((void **) &str);
    parcJSON_Release(&json);
    cpiInterface_Destroy(&iface);
}

LONGBOW_TEST_CASE(Global, cpiInterface_FromJson)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(2));

    PARCJSON *json = cpiInterface_ToJson(iface);

    CPIInterface *test_iface = cpiInterface_FromJson(json);

    assertTrue(cpiInterface_Equals(iface, test_iface), "Interface from json not equal to truth");

    cpiInterface_Destroy(&test_iface);
    cpiInterface_Destroy(&iface);
    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_IsEqual)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    CPIInterface *iface_b = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(2));

    assertTrue(cpiInterface_Equals(iface_a, iface_b), "Two equal interfaces did not compare equal");

    cpiInterface_Destroy(&iface_b);
    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_BothNull)
{
    assertTrue(cpiInterface_Equals(NULL, NULL), "Two NULL interfaces did not compare equal");
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_OneNull)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    assertFalse(cpiInterface_Equals(iface_a, NULL), "One null one non-null interfaces compare equal");

    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_UnequalName)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    CPIInterface *iface_b = cpiInterface_Create("eth1", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(2));

    assertFalse(cpiInterface_Equals(iface_a, iface_b), "Two unequal interfaces compare equal");

    cpiInterface_Destroy(&iface_b);
    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_UnequalIndex)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    CPIInterface *iface_b = cpiInterface_Create("eth0", 2, true, false, 1500);
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(2));

    assertFalse(cpiInterface_Equals(iface_a, iface_b), "Two unequal interfaces compare equal");

    cpiInterface_Destroy(&iface_b);
    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_UnequalLoopback)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    CPIInterface *iface_b = cpiInterface_Create("eth0", 1, false, false, 1500);
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(2));

    assertFalse(cpiInterface_Equals(iface_a, iface_b), "Two unequal interfaces compare equal");

    cpiInterface_Destroy(&iface_b);
    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_UnequalMulticast)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    CPIInterface *iface_b = cpiInterface_Create("eth0", 1, true, true, 1500);
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(2));

    assertFalse(cpiInterface_Equals(iface_a, iface_b), "Two unequal interfaces compare equal");

    cpiInterface_Destroy(&iface_b);
    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_UnequalMTU)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    CPIInterface *iface_b = cpiInterface_Create("eth0", 1, true, false, 9000);
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(2));

    assertFalse(cpiInterface_Equals(iface_a, iface_b), "Two unequal interfaces compare equal");

    cpiInterface_Destroy(&iface_b);
    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_Equals_UnequalAddresses)
{
    CPIInterface *iface_a = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(1));
    cpiInterface_AddAddress(iface_a, cpiAddress_CreateFromInterface(2));

    CPIInterface *iface_b = cpiInterface_Create("eth0", 1, true, false, 1500);
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(3));
    cpiInterface_AddAddress(iface_b, cpiAddress_CreateFromInterface(2));

    assertFalse(cpiInterface_Equals(iface_a, iface_b), "Two unequal interfaces compare equal");

    cpiInterface_Destroy(&iface_b);
    cpiInterface_Destroy(&iface_a);
}

LONGBOW_TEST_CASE(Global, cpiInterface_ToString)
{
    CPIInterface *iface = cpiInterface_Create("eth0", 1, false, true, 1500);
    cpiInterface_AddAddress(iface, cpiAddress_CreateFromInterface(1));

    uint32_t beforeBalance = parcMemory_Outstanding();
    char *string = cpiInterface_ToString(iface);
    parcMemory_Deallocate((void **) &string);
    uint32_t afterBalance = parcMemory_Outstanding();
    cpiInterface_Destroy(&iface);

    assertTrue(beforeBalance == afterBalance, "Memory leak: off by %d allocations", (int) (afterBalance - beforeBalance));
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_Interface);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
