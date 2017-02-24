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


#include "../cpi_Connection.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>


LONGBOW_TEST_RUNNER(cpi_Connection)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_Connection)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_Connection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiConnection_Copy);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnection_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnection_GetAddresses);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnection_GetIndex);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnection_GetState);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnection_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnection_FromJSON);
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

LONGBOW_TEST_CASE(Global, cpiConnection_Copy)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIConnection *iptun = cpiConnection_Create(1, src, dst, cpiConnection_TCP);

    CPIConnection *copy = cpiConnection_Copy(iptun);

    assertTrue(cpiConnection_GetIndex(copy) == cpiConnection_GetIndex(iptun),
               "ifidx did not match, expected %u got %u",
               cpiConnection_GetIndex(iptun),
               cpiConnection_GetIndex(copy));

    assertTrue(cpiConnection_GetState(copy) == cpiConnection_GetState(iptun),
               "states did not match, expected %d got %d",
               cpiConnection_GetState(iptun),
               cpiConnection_GetState(copy));

    assertTrue(cpiAddress_Equals(cpiConnection_GetSourceAddress(copy), cpiConnection_GetSourceAddress(iptun)),
               "did not get same source address");
    assertTrue(cpiAddress_Equals(cpiConnection_GetDestinationAddress(copy), cpiConnection_GetDestinationAddress(iptun)),
               "did not get same destination address");

    assertTrue(cpiConnection_GetConnectionType(copy) == cpiConnection_GetConnectionType(iptun),
               "did not get same connection types!");

    cpiConnection_Release(&copy);
    cpiConnection_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiConnection_Create_Destroy)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIConnection *iptun = cpiConnection_Create(1, src, dst, cpiConnection_GRE);
    cpiConnection_Release(&iptun);

    assertTrue(parcMemory_Outstanding() == 0, "Imbalance after destroying");
}

LONGBOW_TEST_CASE(Global, cpiConnection_GetAddresses)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIConnection *iptun = cpiConnection_Create(1, src, dst, cpiConnection_TCP);

    const CPIAddress *test;

    test = cpiConnection_GetSourceAddress(iptun);
    assertTrue(cpiAddress_Equals(src, test), "Address lists did not match");

    test = cpiConnection_GetDestinationAddress(iptun);
    assertTrue(cpiAddress_Equals(dst, test), "Address lists did not match");

    cpiConnection_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiConnection_GetIndex)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIConnection *iptun = cpiConnection_Create(1, src, dst, cpiConnection_TCP);

    assertTrue(cpiConnection_GetIndex(iptun) == 1, "ifidx did not match");

    cpiConnection_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiConnection_GetState)
{
    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIConnection *iptun = cpiConnection_Create(1, src, dst, cpiConnection_TCP);

    assertTrue(cpiConnection_GetState(iptun) == CPI_IFACE_UNKNOWN, "state did not match");

    cpiConnection_SetState(iptun, CPI_IFACE_UP);
    assertTrue(cpiConnection_GetState(iptun) == CPI_IFACE_UP, "state did not match");

    cpiConnection_SetState(iptun, CPI_IFACE_DOWN);
    assertTrue(cpiConnection_GetState(iptun) == CPI_IFACE_DOWN, "state did not match");

    cpiConnection_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiConnection_ToJSON)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char *expected = "{\"Connection\":{\"IFIDX\":1,\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAgHBgUAAAAAAAAAAA==\"}}}";
#elif defined(__linux__)
    char *expected = "{\"Connection\":{\"IFIDX\":1,\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAgHBgUAAAAAAAAAAA==\"}}}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIConnection *iptun = cpiConnection_Create(1, src, dst, cpiConnection_TCP);

    PARCJSON *test_json = cpiConnection_ToJson(iptun);

    char *actual = parcJSON_ToCompactString(test_json);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSON_Release(&test_json);
    cpiConnection_Release(&iptun);
}

LONGBOW_TEST_CASE(Global, cpiConnection_FromJSON)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char *input = "{\"Connection\":{\"IFIDX\":1,\"STATE\":\"UP\",\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAgHBgUAAAAAAAAAAA==\"}}}";
#elif defined(__linux__)
    char *input = "{\"Connection\":{\"IFIDX\":1,\"STATE\":\"UP\",\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAgHBgUAAAAAAAAAAA==\"}}}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    CPIAddress *src = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 }));
    CPIAddress *dst = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_addr.s_addr = 0x05060708 }));
    CPIConnection *expected = cpiConnection_Create(1, src, dst, cpiConnection_TCP);
    cpiConnection_SetState(expected, CPI_IFACE_UP);

    PARCJSON *json = parcJSON_ParseString(input);

    CPIConnection *actual = cpiConnection_CreateFromJson(json);
    assertTrue(cpiConnection_Equals(expected, actual), "Connection interfaces do not match");

    parcJSON_Release(&json);
    cpiConnection_Release(&expected);
    cpiConnection_Release(&actual);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_Connection);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
