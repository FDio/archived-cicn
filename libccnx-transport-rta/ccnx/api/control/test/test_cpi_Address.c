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
 * Each type test (inet, inet6, etc.) should test:
 * - CreateFromX
 * - GetX
 * - GetType = X
 * - Y = cpiAddress_CreateFromJson( cpiAddress_ToJson(X) ) == X
 * - Equals(Y, X)
 * - Equals(Copy(X), X)
 */

// for inet_pton
#include <config.h>
#include <arpa/inet.h>
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../cpi_Address.c"

LONGBOW_TEST_RUNNER(cpi_Address)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_Address)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_Address)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_Copy);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_Equals_ReallyEqual);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_Equals_SamePointer);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_Equals_NotEqual);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_CreateFromInet);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_CreateFromInet6);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_CreateFromInterface);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_CreateFromLink);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_CreateFromUnix);

    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_ToString_INET);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_ToString_INET6);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_ToString_LINK);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_ToString_IFACE);
    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_ToString_UNIX);

    LONGBOW_RUN_TEST_CASE(Global, cpiAddress_BuildString);
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

LONGBOW_TEST_CASE(Global, cpiAddress_Copy)
{
    CPIAddress *a = cpiAddress_CreateFromInterface(1);
    CPIAddress *b = cpiAddress_Copy(a);

    assertTrue(cpiAddress_Equals(a, b), "Copy did not compare as equal: %s and %s", cpiAddress_ToString(a), cpiAddress_ToString(b));

    cpiAddress_Destroy(&a);
    cpiAddress_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, cpiAddress_Equals_ReallyEqual)
{
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(struct sockaddr_in));

    addr_in.sin_addr.s_addr = 0x01020304;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = 0x0A0B;

    CPIAddress *a = cpiAddress_CreateFromInet(&addr_in);
    CPIAddress *b = cpiAddress_CreateFromInet(&addr_in);

    assertTrue(cpiAddress_Equals(a, b), "Equals did not compare two equal addresses: %s and %s", cpiAddress_ToString(a), cpiAddress_ToString(b));

    cpiAddress_Destroy(&a);
    cpiAddress_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, cpiAddress_Equals_SamePointer)
{
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(struct sockaddr_in));

    addr_in.sin_addr.s_addr = 0x01020304;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = 0x0A0B;

    CPIAddress *a = cpiAddress_CreateFromInet(&addr_in);

    assertTrue(cpiAddress_Equals(a, a), "Equals did not compare two equal addresses: %s and %s", cpiAddress_ToString(a), cpiAddress_ToString(a));

    cpiAddress_Destroy(&a);
}

LONGBOW_TEST_CASE(Global, cpiAddress_Equals_NotEqual)
{
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(struct sockaddr_in));

    addr_in.sin_addr.s_addr = 0x01020304;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = 0x0A0B;

    CPIAddress *a = cpiAddress_CreateFromInet(&addr_in);
    CPIAddress *b = cpiAddress_CreateFromInterface(1);

    assertFalse(cpiAddress_Equals(a, b), "Equals failed on different addresses: %s and %s", cpiAddress_ToString(a), cpiAddress_ToString(b));

    cpiAddress_Destroy(&a);
    cpiAddress_Destroy(&b);
}


LONGBOW_TEST_CASE(Global, cpiAddress_CreateFromInet)
{
    struct sockaddr_in addr_in;
    struct sockaddr_in addr_test;
    memset(&addr_in, 0, sizeof(struct sockaddr_in));

    addr_in.sin_addr.s_addr = 0x01020304;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = 0x0A0B;

    CPIAddress *address = cpiAddress_CreateFromInet(&addr_in);

    bool success = cpiAddress_GetInet(address, &addr_test);
    assertTrue(success, "Got false converting back address");

    assertTrue(memcmp(&addr_in, &addr_test, sizeof(struct sockaddr_in)) == 0, "Got mismatch addressed");

    assertTrue(cpiAddress_GetType(address) == cpiAddressType_INET,
               "Got wrong address type, expected %d, got %d", cpiAddressType_INET, cpiAddress_GetType(address));

    PARCJSON *json = cpiAddress_ToJson(address);
    CPIAddress *fromjson = cpiAddress_CreateFromJson(json);

    assertTrue(cpiAddress_GetType(address) == cpiAddress_GetType(fromjson), "fromjson type does not equal known");
    assertTrue(parcBuffer_Equals(address->blob, fromjson->blob), "fromjson blob does not equal known address");
    assertTrue(cpiAddress_Equals(address, fromjson), "cpiAddress_Equals broken for INET type");

    // This test does too much.  Case 1032
    CPIAddress *copy = cpiAddress_Copy(address);
    assertTrue(cpiAddress_Equals(copy, address), "Copy and address not equal for INET");

    cpiAddress_Destroy(&copy);
    cpiAddress_Destroy(&fromjson);

    parcJSON_Release(&json);

    cpiAddress_Destroy(&address);
    return;
}

LONGBOW_TEST_CASE(Global, cpiAddress_CreateFromInet6)
{
    struct sockaddr_in6 addr_in6;
    memset(&addr_in6, 0, sizeof(struct sockaddr_in6));

    inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
    addr_in6.sin6_family = AF_INET6;
    addr_in6.sin6_port = 0x0A0B;
    addr_in6.sin6_flowinfo = 0x01020304;

    CPIAddress *address = cpiAddress_CreateFromInet6(&addr_in6);

    struct sockaddr_in6 addr_test;
    bool success = cpiAddress_GetInet6(address, &addr_test);
    assertTrue(success, "Got false converting back address");

    assertTrue(memcmp(&addr_in6, &addr_test, sizeof(struct sockaddr_in6)) == 0, "Got mismatch addressed");

    assertTrue(cpiAddress_GetType(address) == cpiAddressType_INET6,
               "Got wrong address type, expected %d, got %d", cpiAddressType_INET6, cpiAddress_GetType(address));

    PARCJSON *json = cpiAddress_ToJson(address);
    CPIAddress *fromjson = cpiAddress_CreateFromJson(json);

    assertTrue(parcBuffer_Equals(address->blob, fromjson->blob), "fromjson blob does not equal known address");
    assertTrue(cpiAddress_Equals(address, fromjson), "cpiAddress_Equals broken for INET6 type");

    CPIAddress *copy = cpiAddress_Copy(address);
    assertTrue(cpiAddress_Equals(copy, address), "Copy and address not equal for INET6");

    parcJSON_Release(&json);
    cpiAddress_Destroy(&address);
    cpiAddress_Destroy(&copy);
    cpiAddress_Destroy(&fromjson);
}

LONGBOW_TEST_CASE(Global, cpiAddress_CreateFromUnix)
{
    struct sockaddr_un addr_un;
    struct sockaddr_un addr_test;
    memset(&addr_un, 0, sizeof(struct sockaddr_un));
    char path[] = "/Hello/Cruel/World";
    strcpy(addr_un.sun_path, path);
    addr_un.sun_family = AF_UNIX;

    CPIAddress *address = cpiAddress_CreateFromUnix(&addr_un);

    bool success = cpiAddress_GetUnix(address, &addr_test);
    assertTrue(success, "Got false converting back address");

    assertTrue(memcmp(&addr_un, &addr_test, sizeof(struct sockaddr_un)) == 0, "Got mismatch addressed");

    assertTrue(cpiAddress_GetType(address) == cpiAddressType_UNIX,
               "Got wrong address type, expected %d, got %d", cpiAddressType_UNIX, cpiAddress_GetType(address));

    PARCJSON *json = cpiAddress_ToJson(address);
    CPIAddress *fromjson = cpiAddress_CreateFromJson(json);

    assertTrue(parcBuffer_Equals(address->blob, fromjson->blob), "fromjson blob does not equal known address");
    assertTrue(cpiAddress_Equals(address, fromjson), "cpiAddress_Equals broken for UNIX type");

    CPIAddress *copy = cpiAddress_Copy(address);
    assertTrue(cpiAddress_Equals(copy, address), "Copy and address not equal for UNIX");

    parcJSON_Release(&json);
    cpiAddress_Destroy(&address);
    cpiAddress_Destroy(&copy);
    cpiAddress_Destroy(&fromjson);
}

LONGBOW_TEST_CASE(Global, cpiAddress_CreateFromInterface)
{
    uint32_t ifidx = 0x01020304;
    uint32_t test;

    CPIAddress *address = cpiAddress_CreateFromInterface(ifidx);

    bool success = cpiAddress_GetInterfaceIndex(address, &test);
    assertTrue(success, "Got false converting back address");

    assertTrue(ifidx == test, "Got mismatch addressed");

    assertTrue(cpiAddress_GetType(address) == cpiAddressType_IFACE,
               "Got wrong address type, expected %d, got %d", cpiAddressType_IFACE, cpiAddress_GetType(address));

    PARCJSON *json = cpiAddress_ToJson(address);
    CPIAddress *fromjson = cpiAddress_CreateFromJson(json);

    assertTrue(parcBuffer_Equals(address->blob, fromjson->blob), "fromjson blob does not equal known address");
    assertTrue(cpiAddress_Equals(address, fromjson), "cpiAddress_Equals broken for IFACE type");

    CPIAddress *copy = cpiAddress_Copy(address);
    assertTrue(cpiAddress_Equals(copy, address), "Copy and address not equal for IFACE");

    parcJSON_Release(&json);
    cpiAddress_Destroy(&address);
    cpiAddress_Destroy(&copy);
    cpiAddress_Destroy(&fromjson);
}

LONGBOW_TEST_CASE(Global, cpiAddress_CreateFromLink)
{
    uint8_t mac[] = { 0x01, 0x02, 0x03, 0x04, 0xFF, 0x8F };
    PARCBuffer *macbuffer = parcBuffer_Flip(parcBuffer_CreateFromArray(mac, sizeof(mac)));

    CPIAddress *address = cpiAddress_CreateFromLink(mac, sizeof(mac));

    // Do not release test, it is the same reference as address->blob
    PARCBuffer *test = cpiAddress_GetLinkAddress(address);
    assertNotNull(test, "Got null link address buffer");
    assertTrue(parcBuffer_Equals(test, address->blob), "Returned buffer from cpiAddress_GetLinkAddress not equal to address");

    assertTrue(cpiAddress_GetType(address) == cpiAddressType_LINK,
               "Got wrong address type, expected %d, got %d", cpiAddressType_LINK, cpiAddress_GetType(address));

    PARCJSON *json = cpiAddress_ToJson(address);
    CPIAddress *fromjson = cpiAddress_CreateFromJson(json);

    assertTrue(cpiAddress_GetType(address) == cpiAddress_GetType(fromjson),
               "fromjson type does not equal known");
    assertTrue(parcBuffer_Equals(address->blob, fromjson->blob),
               "fromjson blob does not equal known address");
    assertTrue(cpiAddress_Equals(address, fromjson),
               "cpiAddress_Equals broken for LINK type");

    CPIAddress *copy = cpiAddress_Copy(address);
    assertTrue(cpiAddress_Equals(copy, address),
               "Copy and address not equal for LINK");

    parcJSON_Release(&json);
    cpiAddress_Destroy(&address);
    cpiAddress_Destroy(&copy);
    cpiAddress_Destroy(&fromjson);
    parcBuffer_Release(&macbuffer);
}

LONGBOW_TEST_CASE(Global, cpiAddress_ToString_INET)
{
    struct sockaddr_in *addr_in = parcNetwork_SockInet4Address("1.2.3.4", 12345);

    char expected[] = "inet4://1.2.3.4:12345";

    CPIAddress *cpiaddr = cpiAddress_CreateFromInet(addr_in);

    char *actual = cpiAddress_ToString(cpiaddr);

    assertTrue(strcmp(actual, expected) == 0, "Bad string, expected '%s' got '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    cpiAddress_Destroy(&cpiaddr);
    parcMemory_Deallocate((void **) &addr_in);
}

LONGBOW_TEST_CASE(Global, cpiAddress_ToString_UNIX)
{
    struct sockaddr_un addr_un;
    char path[] = "/Hello/Cruel/World";
    memset(&addr_un, 0, sizeof(struct sockaddr_un));
    strcpy(addr_un.sun_path, path);
    addr_un.sun_family = AF_UNIX;

    char truth_str[] = "{ .type=UNIX, .data={ .path=/Hello/Cruel/World, .len=18 } }";

    CPIAddress *cpiaddr = cpiAddress_CreateFromUnix(&addr_un);

    char *output = cpiAddress_ToString(cpiaddr);

    assertTrue(strcmp(output, truth_str) == 0, "Bad string, expected %s got %s", truth_str, output);

    parcMemory_Deallocate((void **) &output);
    cpiAddress_Destroy(&cpiaddr);
}

LONGBOW_TEST_CASE(Global, cpiAddress_ToString_INET6)
{
    struct sockaddr_in6 addr_in6;
    memset(&addr_in6, 0, sizeof(struct sockaddr_in6));

    inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
    addr_in6.sin6_family = AF_INET6;
    addr_in6.sin6_port = htons(43215);

    char *expected = "inet6://[2001:720:1500:1::a100%0]:43215";

    CPIAddress *cpiaddr = cpiAddress_CreateFromInet6(&addr_in6);
    char *actual = cpiAddress_ToString(cpiaddr);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    cpiAddress_Destroy(&cpiaddr);
}

LONGBOW_TEST_CASE(Global, cpiAddress_ToString_LINK)
{
    uint8_t addr[6] = { 0x01, 0x02, 0x03, 0xF4, 0xF5, 0xF6 };

    char truth_str[] = "link://01-02-03-f4-f5-f6";

    CPIAddress *cpiaddr = cpiAddress_CreateFromLink(addr, sizeof(addr));
    char *output = cpiAddress_ToString(cpiaddr);

    assertTrue(strcmp(output, truth_str) == 0,
               "Bad string, expected %s got %s", truth_str, output);

    parcMemory_Deallocate((void **) &output);
    cpiAddress_Destroy(&cpiaddr);
}

LONGBOW_TEST_CASE(Global, cpiAddress_ToString_IFACE)
{
    char truth_str[] = "{ .type=IFACE, .data={ .ifidx=55 } }";

    CPIAddress *cpiaddr = cpiAddress_CreateFromInterface(55);
    char *output = cpiAddress_ToString(cpiaddr);

    assertTrue(strcmp(output, truth_str) == 0, "Bad string, expected %s got %s", truth_str, output);

    parcMemory_Deallocate((void **) &output);
    cpiAddress_Destroy(&cpiaddr);
}

LONGBOW_TEST_CASE(Global, cpiAddress_BuildString)
{
    CPIAddress *address = cpiAddress_CreateFromInterface(1);
    uint32_t beforeBalance = parcMemory_Outstanding();
    PARCBufferComposer *composer = cpiAddress_BuildString(address, parcBufferComposer_Create());
    parcBufferComposer_Release(&composer);
    uint32_t afterBalance = parcMemory_Outstanding();

    cpiAddress_Destroy(&address);
    assertTrue(beforeBalance == afterBalance, "Memory leak off by %d allocations", (int) (afterBalance - beforeBalance));
}

// ===============================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _Inet_BuildString);
    LONGBOW_RUN_TEST_CASE(Local, _Inet6_BuildString);
    LONGBOW_RUN_TEST_CASE(Local, _LinkToString);
    LONGBOW_RUN_TEST_CASE(Local, _IfaceToString);
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

LONGBOW_TEST_CASE(Local, _Inet_BuildString)
{
    struct sockaddr_in addr_in;
    addr_in.sin_addr.s_addr = 0x04030201;
    addr_in.sin_port = htons(12345);

    char *expected = "inet4://1.2.3.4:12345";

    CPIAddress *cpiaddr = cpiAddress_CreateFromInet(&addr_in);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    _Inet_BuildString(cpiaddr, composer);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    cpiAddress_Destroy(&cpiaddr);
}

LONGBOW_TEST_CASE(Local, _Inet6_BuildString)
{
    struct sockaddr_in6 addr_in6;
    memset(&addr_in6, 0, sizeof(struct sockaddr_in6));

    inet_pton(AF_INET6, "2001:720:1500:1::a100", &(addr_in6.sin6_addr));
    addr_in6.sin6_family = AF_INET6;
    addr_in6.sin6_port = htons(43215);

    char *expected = "inet6://[2001:720:1500:1::a100%0]:43215";

    CPIAddress *cpiaddr = cpiAddress_CreateFromInet6(&addr_in6);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    _Inet6_BuildString(cpiaddr, composer);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    cpiAddress_Destroy(&cpiaddr);
}

LONGBOW_TEST_CASE(Local, _LinkToString)
{
    uint8_t addr[6] = { 0x01, 0x02, 0x03, 0xF4, 0xF5, 0xF6 };

    char *expected = "link://01-02-03-f4-f5-f6";

    CPIAddress *cpiaddr = cpiAddress_CreateFromLink(addr, sizeof(addr));

    PARCBufferComposer *composer = parcBufferComposer_Create();
    _Link_BuildString(cpiaddr, composer);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    cpiAddress_Destroy(&cpiaddr);
}

LONGBOW_TEST_CASE(Local, _IfaceToString)
{
    char truth_str[] = "{ .ifidx=55 }";

    CPIAddress *cpiaddr = cpiAddress_CreateFromInterface(55);

    char output[1024];
    ssize_t output_length = _IfaceToString(output, 1024, cpiaddr->blob);
    assertTrue(strcmp(output, truth_str) == 0, "Bad string, expected %s got %s", truth_str, output);
    assertTrue(strlen(truth_str) == output_length, "Got wrong output size, expected %zd got %zd", strlen(truth_str), output_length);

    cpiAddress_Destroy(&cpiaddr);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_Address);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
