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


#include "../cpi_InterfaceIPTunnel.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>


LONGBOW_TEST_RUNNER(cpi_InterfaceIPTunnel)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_InterfaceIPTunnel)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_InterfaceIPTunnel)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceIPTunnel_Copy);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceIPTunnel_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceIPTunnel_GetAddresses);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceIPTunnel_GetIndex);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceIPTunnel_GetState);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceIPTunnel_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, cpiInterfaceIPTunnel_FromJSON);
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

LONGBOW_TEST_CASE(Global, cpiInterfaceIPTunnel_Copy)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(1, src, dst, IPTUN_TCP, "tun0");

    CPIInterfaceIPTunnel *copy = cpiInterfaceIPTunnel_Copy(iptun);

    assertTrue(cpiInterfaceIPTunnel_GetIndex(copy) == cpiInterfaceIPTunnel_GetIndex(iptun),
               "ifidx did not match, expected %u got %u",
               cpiInterfaceIPTunnel_GetIndex(iptun),
               cpiInterfaceIPTunnel_GetIndex(copy));

    assertTrue(cpiInterfaceIPTunnel_GetState(copy) == cpiInterfaceIPTunnel_GetState(iptun),
               "states did not match, expected %d got %d",
               cpiInterfaceIPTunnel_GetState(iptun),
               cpiInterfaceIPTunnel_GetState(copy));

    assertTrue(cpiAddress_Equals(cpiInterfaceIPTunnel_GetSourceAddress(copy), cpiInterfaceIPTunnel_GetSourceAddress(iptun)),
               "did not get same source address");
    assertTrue(cpiAddress_Equals(cpiInterfaceIPTunnel_GetDestinationAddress(copy), cpiInterfaceIPTunnel_GetDestinationAddress(iptun)),
               "did not get same destination address");

    assertTrue(cpiInterfaceIPTunnel_GetTunnelType(copy) == cpiInterfaceIPTunnel_GetTunnelType(iptun),
               "did not get same tunnel types!");

    assertNotNull(copy->symbolic, "Copy has null symbolic name");
    assertTrue(strcmp(iptun->symbolic, copy->symbolic) == 0, "symbolics name wrong expected '%s' got '%s'",
               iptun->symbolic, copy->symbolic);

    cpiInterfaceIPTunnel_Release(&copy);
    cpiInterfaceIPTunnel_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceIPTunnel_Create_Destroy)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(1, src, dst, IPTUN_GRE, "tun0");
    cpiInterfaceIPTunnel_Release(&iptun);

    assertTrue(parcMemory_Outstanding() == 0, "Imbalance after destroying");
}

LONGBOW_TEST_CASE(Global, cpiInterfaceIPTunnel_GetAddresses)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(1, src, dst, IPTUN_TCP, "tun0");

    const CPIAddress *test;

    test = cpiInterfaceIPTunnel_GetSourceAddress(iptun);
    assertTrue(cpiAddress_Equals(src, test), "Address lists did not match");

    test = cpiInterfaceIPTunnel_GetDestinationAddress(iptun);
    assertTrue(cpiAddress_Equals(dst, test), "Address lists did not match");

    cpiInterfaceIPTunnel_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceIPTunnel_GetIndex)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(1, src, dst, IPTUN_TCP, "tun0");

    assertTrue(cpiInterfaceIPTunnel_GetIndex(iptun) == 1, "ifidx did not match");

    cpiInterfaceIPTunnel_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceIPTunnel_GetState)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(1, src, dst, IPTUN_TCP, "tun0");

    assertTrue(cpiInterfaceIPTunnel_GetState(iptun) == CPI_IFACE_UNKNOWN, "state did not match");

    cpiInterfaceIPTunnel_SetState(iptun, CPI_IFACE_UP);
    assertTrue(cpiInterfaceIPTunnel_GetState(iptun) == CPI_IFACE_UP, "state did not match");

    cpiInterfaceIPTunnel_SetState(iptun, CPI_IFACE_DOWN);
    assertTrue(cpiInterfaceIPTunnel_GetState(iptun) == CPI_IFACE_DOWN, "state did not match");

    cpiInterfaceIPTunnel_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceIPTunnel_ToJSON)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char *expected = "{\"TUNNEL\":{\"IFIDX\":1,\"SYMBOLIC\":\"tun0\",\"TUNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAgHBgUAAAAAAAAAAA==\"}}}";
#elif defined(__linux__)
    char *expected = "{\"TUNNEL\":{\"IFIDX\":1,\"SYMBOLIC\":\"tun0\",\"TUNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAgHBgUAAAAAAAAAAA==\"}}}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIInterfaceIPTunnel *iptun = cpiInterfaceIPTunnel_Create(1, src, dst, IPTUN_TCP, "tun0");

    PARCJSON *test_json = cpiInterfaceIPTunnel_ToJson(iptun);

    char *actual = parcJSON_ToCompactString(test_json);
    assertEqualStrings(expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSON_Release(&test_json);
    cpiInterfaceIPTunnel_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiInterfaceIPTunnel_FromJSON)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char truth_json_str[] = "{\"TUNNEL\":{\"IFIDX\":1,\"SYMBOLIC\":\"tun0\",\"STATE\":\"UP\",\"TUNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAgHBgUAAAAAAAAAAA==\"}}}";
#elif defined(__linux__)
    char truth_json_str[] = "{\"TUNNEL\":{\"IFIDX\":1,\"SYMBOLIC\":\"tun0\",\"STATE\":\"UP\",\"TUNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAgHBgUAAAAAAAAAAA==\"}}}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif


    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIInterfaceIPTunnel *truth = cpiInterfaceIPTunnel_Create(1, src, dst, IPTUN_TCP, "tun0");
    cpiInterfaceIPTunnel_SetState(truth, CPI_IFACE_UP);

    PARCJSON *json = parcJSON_ParseString(truth_json_str);

    CPIInterfaceIPTunnel *test = cpiInterfaceIPTunnel_CreateFromJson(json);
    assertTrue(cpiInterfaceIPTunnel_Equals(truth, test), "IPTunnel interfaces do not match");

    parcJSON_Release(&json);
    cpiInterfaceIPTunnel_Release(&truth);
    cpiInterfaceIPTunnel_Release(&test);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_InterfaceIPTunnel);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
