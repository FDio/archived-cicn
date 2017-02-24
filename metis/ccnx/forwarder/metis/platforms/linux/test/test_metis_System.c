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
#include "../metis_System.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Memory.h>

// Include the generic tests of MetisSystem
#include "../../test/testrig_metis_System.c"

LONGBOW_TEST_RUNNER(linux_Interface)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);

    // these are defined in testrig_metis_System.c
    LONGBOW_RUN_TEST_FIXTURE(PublicApi);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(linux_Interface)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(linux_Interface)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisSystem_Interfaces);
    LONGBOW_RUN_TEST_CASE(Global, metisSystem_InterfaceMTU);
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

LONGBOW_TEST_CASE(Global, metisSystem_Interfaces)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    assertNotNull(set, "metisSystem_Interfaces return null set");

    // XXX we need some sort of validation test.  e.g. open a socket, then ioctl to
    // XXX get the interface name, then verify its in the list.

    size_t length = cpiInterfaceSet_Length(set);
    assertTrue(length > 0, "metisSystem_Interfaces returned no interfaces");

    for (size_t i = 0; i < length; i++) {
        CPIInterface *iface = cpiInterfaceSet_GetByOrdinalIndex(set, i);
        printf("Interface Index %u\n", cpiInterface_GetInterfaceIndex(iface));
        const CPIAddressList *list = cpiInterface_GetAddresses(iface);
        PARCJSONArray *json = cpiAddressList_ToJson(list);
        char *str = parcJSONArray_ToString(json);
        printf("%s\n", str);
        parcMemory_Deallocate((void **) &str);
        parcJSONArray_Release(&json);
    }

    cpiInterfaceSet_Destroy(&set);
    metisForwarder_Destroy(&metis);
}

// returns a strdup() of the interface name, use free(3)
static char *
_pickInterfaceName(MetisForwarder *metis)
{
    char *ifname = NULL;

    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    size_t length = cpiInterfaceSet_Length(set);
    assertTrue(length > 0, "metisSystem_Interfaces returned no interfaces");

    for (size_t i = 0; i < length; i++) {
        CPIInterface *iface = cpiInterfaceSet_GetByOrdinalIndex(set, i);
        const CPIAddressList *addressList = cpiInterface_GetAddresses(iface);

        size_t length = cpiAddressList_Length(addressList);
        for (size_t i = 0; i < length && !ifname; i++) {
            const CPIAddress *a = cpiAddressList_GetItem(addressList, i);
            if (cpiAddress_GetType(a) == cpiAddressType_LINK) {
                ifname = strdup(cpiInterface_GetName(iface));
            }
        }
    }

    cpiInterfaceSet_Destroy(&set);
    return ifname;
}

LONGBOW_TEST_CASE(Global, metisSystem_InterfaceMTU)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);

    char *deviceName = _pickInterfaceName(metis);
    unsigned mtu = metisSystem_InterfaceMtu(metis, deviceName);

    assertTrue(mtu > 0, "Did not get mtu for interface %s", deviceName);
    free(deviceName);
    metisForwarder_Destroy(&metis);
}

// ==================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(linux_Interface);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
