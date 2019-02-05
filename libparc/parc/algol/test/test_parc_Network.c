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
#include "../parc_Network.c"

#include <LongBow/unit-test.h>

#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

#include <sys/un.h>

LONGBOW_TEST_RUNNER(parc_Networking)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Networking)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Networking)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_SockInet4AddressAny);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_SockInet4Address_BuildString);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_SockInet6Address_BuildString);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_BuildString_dashes);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_BuildString_colons);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_Inet4Equals);

    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_dashes);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_colons);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_dots);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_ParseLinkAddress_BadScheme);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_BadLink);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_BadMixOfDashesAndDots);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_BadMixOfDotsAndDashes);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_BadSpecification);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_ParseInet4Address);

    LONGBOW_RUN_TEST_CASE(Global, parseMAC48Address);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_ParseMAC48Address_Colons);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_ParseMAC48Address_Colons_TooShort);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_ParseMAC48Address_Colons_Garbage);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_ParseMAC48Address);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_ParseMAC48Address_TooShort);

    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_SockAddress_ipv4);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_SockAddress_ipv6);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_SockAddress_hostname);

    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_LOCAL);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_INET4);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_INET6);
    LONGBOW_RUN_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_IPX);
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

LONGBOW_TEST_CASE(Global, parcNetwork_SockInet4AddressAny)
{
    struct sockaddr_in *test_sock = parcNetwork_SockInet4AddressAny();

    assertNotNull(test_sock, "Expected a not null pointer\n");
    assertTrue(test_sock->sin_family == AF_INET, "Expecting sin_family to be AF_INET\n");
    assertTrue(test_sock->sin_addr.s_addr == INADDR_ANY, "Expecting sin_addr.s_addr to be set to INADDR_ANY\n");
#if defined(SIN6_LEN)
    assertTrue(test_sock->sin_len == sizeof(struct sockaddr_in), "Expecting sockaddr.sin_len to be %zu not %hhu\n",
               sizeof(struct sockaddr_in), test_sock->sin_len);
#endif

    parcMemory_Deallocate((void **) &test_sock);
}

LONGBOW_TEST_CASE(Global, parcNetwork_SockInet4Address_BuildString)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    struct sockaddr_in *address = parcNetwork_SockInet4Address("127.0.0.1", 1234);
#if defined(SIN6_LEN)
    assertTrue(address->sin_len == sizeof(struct sockaddr_in), "Expecting sockaddr.sin_len to be %zu not %hhu\n",
               sizeof(struct sockaddr_in), address->sin_len);
#endif
    parcNetwork_SockInet4Address_BuildString(address, composer);

    char *expected = "inet4://127.0.0.1:1234";

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcMemory_Deallocate((void **) &address);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_SockInet6Address_BuildString)
{
    struct sockaddr_in6 *address = parcNetwork_SockInet6Address("2001:720:1500:1::a100", 1234, 0, 1);
#if defined(SIN6_LEN)
    assertTrue(address->sin6_len == sizeof(struct sockaddr_in6), "Expecting sockaddr.sin6_len to be %zu not %hhu\n",
               sizeof(struct sockaddr_in6), address->sin6_len);
#endif

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcNetwork_SockInet6Address_BuildString(address, composer);

    char *expected = "inet6://[2001:720:1500:1::a100%1]:1234";

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcMemory_Deallocate((void **) &address);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_LinkAddress_BuildString_dashes)
{
    char *expected = "link://01-23-45-67-89-ab";

    PARCBuffer *address = parcNetwork_ParseLinkAddress(expected);

    PARCBufferComposer *composer = parcBufferComposer_Create();

    parcNetwork_LinkAddress_BuildString((unsigned char *) parcBuffer_Overlay(address, 0), parcBuffer_Remaining(address), composer);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcBufferComposer_Release(&composer);
    parcBuffer_Release(&address);
}

LONGBOW_TEST_CASE(Global, parcNetwork_LinkAddress_BuildString_colons)
{
    char *expected = "link://01-23-45-67-89-ab";

    PARCBuffer *address = parcNetwork_ParseLinkAddress("link://01:23:45:67:89:ab");

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcNetwork_LinkAddress_BuildString((unsigned char *) parcBuffer_Overlay(address, 0), parcBuffer_Remaining(address), composer);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcBufferComposer_Release(&composer);
    parcBuffer_Release(&address);
}

LONGBOW_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_dashes)
{
    char *expected = "link://01-23-45-67-89-ab";
    PARCBuffer *address = parcNetwork_ParseLinkAddress(expected);
    parcBuffer_Flip(address);

    PARCBuffer *e = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    parcBuffer_SetPosition(address, 0);
    parcBuffer_SetLimit(address, 6);

    parcBuffer_SetPosition(e, 0);
    parcBuffer_SetLimit(e, 6);

    assertTrue(parcBuffer_Equals(address, e),
               "Expected result failed.");

    parcBuffer_Release(&e);
    parcBuffer_Release(&address);
}

LONGBOW_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_colons)
{
    char *expected = "link://01:23:45:67:89:ab";
    PARCBuffer *address = parcNetwork_ParseLinkAddress(expected);

    PARCBuffer *e = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    parcBuffer_SetPosition(address, 0);
    parcBuffer_SetPosition(e, 0);
    parcBuffer_SetLimit(address, 6);
    parcBuffer_SetLimit(e, 6);

    assertTrue(parcBuffer_Equals(address, e),
               "Expected result failed.");

    parcBuffer_Release(&e);
    parcBuffer_Release(&address);
}

LONGBOW_TEST_CASE(Global, parcNetwork_LinkAddress_Parse_dots)
{
    char *expected = "link://0123.4567.89ab";
    PARCBuffer *address = parcNetwork_ParseLinkAddress(expected);

    PARCBuffer *e = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    parcBuffer_SetPosition(address, 0);
    parcBuffer_SetPosition(e, 0);
    parcBuffer_SetLimit(address, 6);
    parcBuffer_SetLimit(e, 6);

    assertTrue(parcBuffer_Equals(address, e),
               "Expected result failed.");

    parcBuffer_Release(&e);
    parcBuffer_Release(&address);
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcNetwork_ParseLinkAddress_BadScheme, .event = &LongBowTrapIllegalValue)
{
    char *expected = "asdf://";
    parcNetwork_ParseLinkAddress(expected);
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcNetwork_LinkAddress_Parse_BadLink, .event = &LongBowTrapIllegalValue)
{
    char *expected = "link://";
    parcNetwork_ParseLinkAddress(expected);
    printf("Hello\n");
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcNetwork_LinkAddress_Parse_BadSpecification, .event = &LongBowTrapIllegalValue)
{
    char *expected = "link://a";
    parcNetwork_ParseLinkAddress(expected);
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcNetwork_LinkAddress_Parse_BadMixOfDashesAndDots, .event = &LongBowTrapIllegalValue)
{
    char *expected = "link://01-23-45.6789ab";
    parcNetwork_ParseLinkAddress(expected);
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcNetwork_LinkAddress_Parse_BadMixOfDotsAndDashes, .event = &LongBowTrapIllegalValue)
{
    char *expected = "link://012345.67-89-ab";
    parcNetwork_ParseLinkAddress(expected);
}

LONGBOW_TEST_CASE(Global, parseMAC48Address)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    bool actual = parcNetwork_ParseMAC48Address("01-23-45-67-89-ab", buffer);
    assertTrue(actual, "Expected parcNetwork_ParseMAC48Address() to return true");

    parcBuffer_Flip(buffer);

    assertTrue(parcBuffer_Equals(expected, buffer), "Expected buffer contents failed.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_ParseMAC48Address_Colons)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    bool actual = parcNetwork_ParseMAC48Address("01:23:45:67:89:ab", buffer);
    assertTrue(actual, "Expected parcNetwork_ParseMAC48Address() to return true");
    parcBuffer_Flip(buffer);

    assertTrue(parcBuffer_Equals(expected, buffer), "Expected buffer contents failed.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_ParseMAC48Address_Colons_TooShort)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    bool actual = parcNetwork_ParseMAC48Address("01:23:45:67:89", buffer);
    assertFalse(actual, "Expected parcNetwork_ParseMAC48Address() to return false");
    assertTrue(parcBuffer_Position(buffer) == 0, "Expected buffer to be unmodified.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_ParseMAC48Address_Colons_Garbage)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    bool actual = parcNetwork_ParseMAC48Address("0x:23:45:67:89:ab", buffer);
    assertFalse(actual, "Expected parcNetwork_ParseMAC48Address() to return false");
    assertTrue(parcBuffer_Position(buffer) == 0, "Expected the PARCBuffer to be unchanged.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_ParseMAC48Address)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    bool actual = parcNetwork_ParseMAC48Address("0123.4567.89ab", buffer);
    assertTrue(actual, "Expected _parseLinkAddressDot() to return true");
    parcBuffer_Flip(buffer);

    assertTrue(parcBuffer_Equals(expected, buffer), "Expected buffer contents failed.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_ParseMAC48Address_TooShort)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    bool actual = parcNetwork_ParseMAC48Address("0123.4567", buffer);
    assertFalse(actual, "Expected parcNetwork_ParseMAC48Address() to return false");
    assertTrue(parcBuffer_Position(buffer) == 0, "Expected the PARCBuffer to be unchanged.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcNetwork_ParseInet4Address)
{
    struct sockaddr_in *address = parcNetwork_SockInet4Address("127.0.0.1", 1234);

    PARCBufferComposer *composer = parcNetwork_SockInet4Address_BuildString(address, parcBufferComposer_Create());

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *addressURI = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    struct sockaddr_in *actual = parcNetwork_ParseInet4Address(addressURI);

    assertTrue(parcNetwork_Inet4Equals(address, actual),
               "Expected Addresses are not equal");

    parcMemory_Deallocate((void **) &actual);
    parcMemory_Deallocate((void **) &addressURI);
    parcBufferComposer_Release(&composer);
    parcMemory_Deallocate((void **) &address);
}

LONGBOW_TEST_CASE(Global, parcNetwork_Inet4Equals)
{
    struct sockaddr_in *x = parcNetwork_SockInet4Address("127.0.0.1", 1234);
    struct sockaddr_in *y = parcNetwork_SockInet4Address("127.0.0.1", 1234);
    struct sockaddr_in *z = parcNetwork_SockInet4Address("127.0.0.1", 1234);
    struct sockaddr_in *u1 = parcNetwork_SockInet4Address("127.0.0.2", 1234);
    struct sockaddr_in *u2 = parcNetwork_SockInet4Address("127.0.0.1", 4567);

    parcObjectTesting_AssertEqualsFunction(parcNetwork_Inet4Equals, x, y, z, u1, u2);

    parcMemory_Deallocate((void **) &x);
    parcMemory_Deallocate((void **) &y);
    parcMemory_Deallocate((void **) &z);
    parcMemory_Deallocate((void **) &u1);
    parcMemory_Deallocate((void **) &u2);
}

LONGBOW_TEST_CASE(Global, parcNetwork_SockAddress_ipv4)
{
    const char *ipv4 = "1.1.1.1";
    unsigned short port = 5959;

    struct sockaddr_in truth = {
        .sin_family      = PF_INET,
        .sin_port        = htons(port),
        .sin_addr.s_addr = htonl(0x01010101)
    };

    struct sockaddr_in *test = (struct sockaddr_in *) parcNetwork_SockAddress(ipv4, port);
    assertNotNull(test, "Got null address for %s port %u", ipv4, port);
    assertTrue(truth.sin_family == test->sin_family, "wrong family, expected %d got %d", truth.sin_family, test->sin_family);
    assertTrue(truth.sin_port == test->sin_port, "wrong port, expected %u got %u", truth.sin_port, test->sin_port);

    assertTrue(memcmp(&truth.sin_addr, &test->sin_addr, sizeof(struct in_addr)) == 0, "struct in_addr did not compare");
    parcMemory_Deallocate((void **) &test);
}

LONGBOW_TEST_CASE(Global, parcNetwork_SockAddress_ipv6)
{
    const char *ipv6 = "fe80::aa20:66ff:fe00:314a";
    uint8_t truth_addr[16] = { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                               0xaa, 0x20, 0x66, 0xff, 0xfe, 0x00, 0x31, 0x4a };

    unsigned short port = 5959;

    struct sockaddr_in6 truth = {
        .sin6_family = PF_INET6,
        .sin6_port   = htons(port)
    };

    memcpy(&truth.sin6_addr, truth_addr, sizeof(truth.sin6_addr));


    struct sockaddr_in6 *test = (struct sockaddr_in6 *) parcNetwork_SockAddress(ipv6, port);
    if (test == NULL) {
        testSkip("IPv6 is not supported in the runtime environment.");
    }
    assertTrue(truth.sin6_family == test->sin6_family, "wrong family, expected %d got %d", truth.sin6_family, test->sin6_family);
    assertTrue(truth.sin6_port == test->sin6_port, "wrong port, expected %u got %u", truth.sin6_port, test->sin6_port);

    assertTrue(memcmp(&truth.sin6_addr, &test->sin6_addr, sizeof(struct in6_addr)) == 0, "struct in_addr did not compare");
    parcMemory_Deallocate((void **) &test);
}

LONGBOW_TEST_CASE(Global, parcNetwork_SockAddress_hostname)
{
    const char *name = "localhost";
    unsigned short port = 5959;

    struct sockaddr *test = parcNetwork_SockAddress(name, port);
    assertNotNull(test, "Got null looking up '%s'", name);
    parcMemory_Deallocate((void **) &test);
}

LONGBOW_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_LOCAL)
{
    struct sockaddr_un name;
    name.sun_family = PF_LOCAL;

    bool isLocal = parcNetwork_IsSocketLocal((struct sockaddr *) &name);
    assertTrue(isLocal, "PF_LOCAL address did not return as local");
}

LONGBOW_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_IPX)
{
    struct sockaddr_un name;
    name.sun_family = PF_IPX;

    bool isLocal = parcNetwork_IsSocketLocal((struct sockaddr *) &name);
    assertFalse(isLocal, "Expected parcNetwork_IsSocketLocal(PF_PUP) to return false");
}

LONGBOW_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_INET4)
{
    struct sockaddr *s = parcNetwork_SockAddress("127.1.1.1", 5900);
    bool isLoopback = parcNetwork_IsSocketLocal(s);
    assertTrue(isLoopback, "127.1.1.1 should be called loopback");
    parcMemory_Deallocate((void **) &s);
}

LONGBOW_TEST_CASE(Global, parcNetwork_IsSocketLocal_PF_INET6)
{
    struct sockaddr *s = parcNetwork_SockAddress("::1", 5900);
    bool isLoopback = parcNetwork_IsSocketLocal(s);
    assertTrue(isLoopback, "::1 should be called loopback");
    parcMemory_Deallocate((void **) &s);
}


// =======================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, parcNetwork_IsInet6Local_True);
    LONGBOW_RUN_TEST_CASE(Local, parcNetwork_IsInet6Local_False);
    LONGBOW_RUN_TEST_CASE(Local, parcNetwork_IsInet4Local_True);
    LONGBOW_RUN_TEST_CASE(Local, parcNetwork_IsInet4Local_False);

    LONGBOW_RUN_TEST_CASE(Local, _parseMAC48AddressDashOrColon);
    LONGBOW_RUN_TEST_CASE(Local, _parseMAC48AddressDashOrColon_Colons);
    LONGBOW_RUN_TEST_CASE(Local, _parseMAC48AddressDot);
    LONGBOW_RUN_TEST_CASE(Local, _parseMAC48AddressDot_TooShort);
    LONGBOW_RUN_TEST_CASE(Local, _parseMAC48AddressDashOrColon_Colons_TooShort);
    LONGBOW_RUN_TEST_CASE(Local, _parseMAC48AddressDashOrColon_Colons_Garbage);
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

LONGBOW_TEST_CASE(Local, parcNetwork_IsInet6Local_True)
{
    struct sockaddr *s = parcNetwork_SockAddress("::1", 5900);

    bool isLoopback = _isInet6Loopback((struct sockaddr_in6 *) s);
    assertTrue(isLoopback, "::1 should be called loopback");
    parcMemory_Deallocate((void **) &s);
}

LONGBOW_TEST_CASE(Local, parcNetwork_IsInet6Local_False)
{
    struct sockaddr *s = parcNetwork_SockAddress("fe80::aa20:66ff:fe00:1", 5900);

    bool isLoopback = _isInet6Loopback((struct sockaddr_in6 *) s);
    assertFalse(isLoopback, "fe80::aa20:66ff:fe00:1 should not be called loopback");
    parcMemory_Deallocate((void **) &s);
}

LONGBOW_TEST_CASE(Local, parcNetwork_IsInet4Local_True)
{
    struct sockaddr *s = parcNetwork_SockAddress("127.1.1.1", 5900);

    bool isLoopback = _isInet4Loopback((struct sockaddr_in *) s);
    assertTrue(isLoopback, "127.1.1.1 should be called loopback");
    parcMemory_Deallocate((void **) &s);
}

LONGBOW_TEST_CASE(Local, parcNetwork_IsInet4Local_False)
{
    struct sockaddr *s = parcNetwork_SockAddress("13.1.1.1", 5900);

    bool isLoopback = _isInet4Loopback((struct sockaddr_in *) s);
    assertFalse(isLoopback, "13.1.1.1 should not be called loopback");
    parcMemory_Deallocate((void **) &s);
}

LONGBOW_TEST_CASE(Local, _parseMAC48AddressDashOrColon)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    PARCBuffer *actual = _parseMAC48AddressDashOrColon("01-23-45-67-89-ab", buffer);
    assertNotNull(actual, "Expected _parseLinkAddressDashOrColon() to return non-NULL value");
    parcBuffer_Flip(actual);

    assertTrue(parcBuffer_Equals(expected, actual), "Expected buffer contents failed.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Local, _parseMAC48AddressDashOrColon_Colons)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    PARCBuffer *actual = _parseMAC48AddressDashOrColon("01:23:45:67:89:ab", buffer);
    assertNotNull(actual, "Expected _parseLinkAddressDashOrColon() to return non-NULL value");
    parcBuffer_Flip(actual);

    assertTrue(parcBuffer_Equals(expected, actual), "Expected buffer contents failed.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Local, _parseMAC48AddressDashOrColon_Colons_TooShort)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    PARCBuffer *actual = _parseMAC48AddressDashOrColon("01:23:45:67:89", buffer);
    assertNull(actual, "Expected _parseLinkAddressDashOrColon() to return NULL value");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Local, _parseMAC48AddressDashOrColon_Colons_Garbage)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    PARCBuffer *actual = _parseMAC48AddressDashOrColon("0x:23:45:67:89:ab", buffer);
    assertNull(actual, "Expected _parseLinkAddressDashOrColon() to return NULL value");
    assertTrue(parcBuffer_Position(buffer) == 0, "Expected the PARCBuffer to be unchanged.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Local, _parseMAC48AddressDot)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    PARCBuffer *actual = _parseMAC48AddressDot("0123.4567.89ab", buffer);
    assertNotNull(actual, "Expected _parseLinkAddressDot() to return non-NULL value");
    parcBuffer_Flip(actual);

    assertTrue(parcBuffer_Equals(expected, actual), "Expected buffer contents failed.");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Local, _parseMAC48AddressDot_TooShort)
{
    PARCBuffer *expected = parcBuffer_Wrap((uint8_t []) { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab }, 6, 0, 6);

    PARCBuffer *buffer = parcBuffer_Allocate(7);
    PARCBuffer *actual = _parseMAC48AddressDot("0123.4567", buffer);
    assertNull(actual, "Expected _parseLinkAddressDot() to return NULL value");

    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Networking);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
