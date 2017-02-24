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
 * Tests the public API for the SystemInterfaces calls.
 *
 */

// ==================================================================

LONGBOW_TEST_FIXTURE(PublicApi)
{
    LONGBOW_RUN_TEST_CASE(PublicApi, metisSystem_Interfaces);
    LONGBOW_RUN_TEST_CASE(PublicApi, metisSystem_GetMacAddressByName);
}

LONGBOW_TEST_FIXTURE_SETUP(PublicApi)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(PublicApi)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(PublicApi, metisSystem_Interfaces)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    CPIInterfaceSet *set = metisSystem_Interfaces(metis);
    assertNotNull(set, "metisSystem_Interfaces return null set");

    size_t length = cpiInterfaceSet_Length(set);
    assertTrue(length > 0, "metisSystem_Interfaces returned no interfaces");

    for (size_t i = 0; i < length; i++) {
        CPIInterface *iface = cpiInterfaceSet_GetByOrdinalIndex(set, i);
        printf("Interface Index %u\n", cpiInterface_GetInterfaceIndex(iface));
        const CPIAddressList *list = cpiInterface_GetAddresses(iface);
        PARCJSONArray *jsonArray = cpiAddressList_ToJson(list);
        char *str = parcJSONArray_ToString(jsonArray);
        printf("%s\n", str);
        parcMemory_Deallocate((void **) &str);
        parcJSONArray_Release(&jsonArray);
    }

    cpiInterfaceSet_Destroy(&set);
    metisForwarder_Destroy(&metis);
}

// returns a strdup() of the interface name, use free(3)
char *
pickInterfaceName(MetisForwarder *metis)
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

LONGBOW_TEST_CASE(PublicApi, metisSystem_GetMacAddressByName)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    char *ifname = pickInterfaceName(metis);

    CPIAddress *a = metisSystem_GetMacAddressByName(metis, ifname);
    assertNotNull(a, "Got null mac address for interface %s", ifname);
    cpiAddress_Destroy(&a);
    metisForwarder_Destroy(&metis);
    free(ifname);
}

