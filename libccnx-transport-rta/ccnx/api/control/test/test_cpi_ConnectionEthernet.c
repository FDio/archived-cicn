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
 */

#include "../cpi_ConnectionEthernet.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>


typedef struct test_data {
    CPIConnectionEthernet *etherConn;

    // the truth values of the connection
    uint8_t macArray[6];
    CPIAddress *macAddress;
    uint16_t ethertype;
    char ifname[16];
    char symbolic[16];
} TestData;

static CPIConnectionEthernet *
conjureObject(uint8_t mac[6], uint16_t ethertype, const char *ifname, const char *symbolic)
{
    CPIAddress *macAddress = cpiAddress_CreateFromLink(mac, 6);
    CPIConnectionEthernet *etherConn = cpiConnectionEthernet_Create(ifname, macAddress, ethertype, symbolic);
    cpiAddress_Destroy(&macAddress);
    return etherConn;
}

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    uint8_t mac[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    memcpy(data->macArray, mac, 6);
    data->macAddress = cpiAddress_CreateFromLink(data->macArray, 6);
    data->ethertype = 0x0801;
    sprintf(data->ifname, "em1");
    sprintf(data->symbolic, "conn0");

    data->etherConn = cpiConnectionEthernet_Create(data->ifname, data->macAddress, data->ethertype, data->symbolic);
    return data;
}

static void
_commonTeardown(TestData *data)
{
    cpiConnectionEthernet_Release(&data->etherConn);
    cpiAddress_Destroy(&data->macAddress);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(cpi_ConnectionEthernet)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_ConnectionEthernet)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_ConnectionEthernet)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_Create);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_CreateAddMessage);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_CreateRemoveMessage);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_FromControl);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_GetPeerLinkAddress);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_GetEthertype);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_GetEthertype);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_GetInterfaceName);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_IsAddMessage);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_IsRemoveMessage);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionEthernet_Equals);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    size_t beforeBalance = parcMemory_Outstanding();
    CPIConnectionEthernet *etherConn = cpiConnectionEthernet_Create(data->ifname, data->macAddress, data->ethertype, data->symbolic);
    cpiConnectionEthernet_Release(&etherConn);
    size_t afterBalance = parcMemory_Outstanding();

    assertTrue(afterBalance == beforeBalance,
               "Memory imbalance on create/destroy, before %zu afer %zu",
               beforeBalance, afterBalance);
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_Equals)
{
    uint8_t mac_a[6] = { 1, 2, 3, 4, 5, 6 };
    uint8_t mac_b[6] = { 9, 8, 7, 6, 5, 4 };

    CPIConnectionEthernet *x = conjureObject(mac_a, 0x0123, "happy", "puppy");
    CPIConnectionEthernet *y = conjureObject(mac_a, 0x0123, "happy", "puppy");
    CPIConnectionEthernet *z = conjureObject(mac_a, 0x0123, "happy", "puppy");

    CPIConnectionEthernet *u = conjureObject(mac_b, 0x0123, "happy", "puppy");
    CPIConnectionEthernet *v = conjureObject(mac_a, 0x7777, "happy", "puppy");
    CPIConnectionEthernet *w = conjureObject(mac_a, 0x0123, "sad", "kitten");

    assertEqualsContract(cpiConnectionEthernet_Equals, x, y, z, u, v, w);

    cpiConnectionEthernet_Release(&x);
    cpiConnectionEthernet_Release(&y);
    cpiConnectionEthernet_Release(&z);
    cpiConnectionEthernet_Release(&u);
    cpiConnectionEthernet_Release(&v);
    cpiConnectionEthernet_Release(&w);
}


LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_CreateAddMessage)
{
    const char *truthStringFormat = "{ \"CPI_REQUEST\" : { \"SEQUENCE\" : %d, \"%s\" : { \"IFNAME\" : \"em1\", \"SYMBOLIC\" : \"conn0\", \"PEER_ADDR\" : { \"ADDRESSTYPE\" : \"LINK\", \"DATA\" : \"AQIDBAUG\" }, \"ETHERTYPE\" : 2049 } } }";

    char buffer[1024];

    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxControl *control = cpiConnectionEthernet_CreateAddMessage(data->etherConn);

    if (ccnxControl_IsCPI(control)) {
        PARCJSON *testJson = ccnxControl_GetJson(control);
        assertNotNull(testJson, "Got null json from control message");
        uint64_t seqnum = controlPlaneInterface_GetSequenceNumber(testJson);
        sprintf(buffer, truthStringFormat, (int) seqnum, KEY_ADDETHER);

        PARCJSON *truthJson = parcJSON_ParseString(buffer);
        assertTrue(parcJSON_Equals(truthJson, testJson), "JSON not correct in Add Connection Ethernet")
        {
            char *a = parcJSON_ToString(testJson);
            printf("Got: \n%s\n", a);
            parcMemory_Deallocate((void **) &a);

            printf("Expected\n%s\n", buffer);
        }

        parcJSON_Release(&truthJson);
    }
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_CreateRemoveMessage)
{
    const char *truthStringFormat = "{ \"CPI_REQUEST\" : { \"SEQUENCE\" : %d, \"%s\" : { \"IFNAME\" : \"em1\", \"SYMBOLIC\" : \"conn0\", \"PEER_ADDR\" : { \"ADDRESSTYPE\" : \"LINK\", \"DATA\" : \"AQIDBAUG\" }, \"ETHERTYPE\" : 2049 } } }";

    char buffer[1024];

    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxControl *control = cpiConnectionEthernet_CreateRemoveMessage(data->etherConn);

    if (ccnxControl_IsCPI(control)) {
        PARCJSON *testJson = ccnxControl_GetJson(control);
        assertNotNull(testJson, "Got null json from control message");
        uint64_t seqnum = controlPlaneInterface_GetSequenceNumber(testJson);
        sprintf(buffer, truthStringFormat, (int) seqnum, KEY_REMOVEETHER);

        PARCJSON *truthJson = parcJSON_ParseString(buffer);
        assertTrue(parcJSON_Equals(truthJson, testJson), "JSON not correct in Remove Connection Ethernet")
        {
            char *a = parcJSON_ToString(testJson);
            printf("Got: \n%s\n", a);
            parcMemory_Deallocate((void **) &a);

            printf("Expected\n%s\n", buffer);
        }

        parcJSON_Release(&truthJson);
    }
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_FromControl)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxControl *addMessage = cpiConnectionEthernet_CreateAddMessage(data->etherConn);

    CPIConnectionEthernet *test = cpiConnectionEthernet_FromControl(addMessage);

    assertNotNull(test, "Got null object parsing json: %s\n", parcJSON_ToString(ccnxControl_GetJson(addMessage)));

    assertTrue(cpiConnectionEthernet_Equals(test, data->etherConn), "Object from control did not equal true value");

    cpiConnectionEthernet_Release(&test);
    ccnxControl_Release(&addMessage);
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_GetPeerLinkAddress)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CPIAddress *test = cpiConnectionEthernet_GetPeerLinkAddress(data->etherConn);
    assertTrue(cpiAddress_Equals(test, data->macAddress), "Wrong mac address");
}


LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_GetEthertype)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint16_t test = cpiConnectionEthernet_GetEthertype(data->etherConn);
    assertTrue(test == data->ethertype, "Wrong ethertype, got %04X expected %04X", test, data->ethertype);
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_GetInterfaceName)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    const char *test = cpiConnectionEthernet_GetInterfaceName(data->etherConn);
    assertTrue(strcmp(test, data->ifname) == 0, "Wrong interface name, got '%s' expected '%s'", test, data->ifname);
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_IsAddMessage)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxControl *control = cpiConnectionEthernet_CreateAddMessage(data->etherConn);
    bool isAdd = cpiConnectionEthernet_IsAddMessage(control);
    ccnxControl_Release(&control);

    assertTrue(isAdd, "Add Connection Ethernet message did not report as such a message.");
}

LONGBOW_TEST_CASE(Global, cpiConnectionEthernet_IsRemoveMessage)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxControl *control = cpiConnectionEthernet_CreateRemoveMessage(data->etherConn);
    bool isRemove = cpiConnectionEthernet_IsRemoveMessage(control);
    ccnxControl_Release(&control);

    assertTrue(isRemove, "Remove Connection Ethernet message did not report as such a message.");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_ConnectionEthernet);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
