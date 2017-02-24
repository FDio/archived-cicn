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


#include "../cpi_Forwarding.c"

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <inttypes.h>



LONGBOW_TEST_RUNNER(cpi_Forwarding)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_Forwarding)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_Forwarding)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_AddRoute_1);
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_AddRoute_2);
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_AddRoute_3);

    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_AddRouteJsonTag);
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_AddRouteToSelf);
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_RemoveRoute);
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_RemoveRouteJsonTag);
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_RemoveRouteToSelf);

    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_RouteFromControlMessage);

    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_CreateRouteListRequest);
    LONGBOW_RUN_TEST_CASE(Global, cpiForwarding_RouteListFromControlMessage);
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

/**
 * Add route with all options
 */
LONGBOW_TEST_CASE(Global, cpiForwarding_AddRoute_1)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"REGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200,\"LIFETIME\":[3600,0]}}}";
#elif defined(__linux__)
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"REGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200,\"LIFETIME\":[3600,0]}}}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    char truth[1024];

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    CCNxControl *control = ccnxControl_CreateAddRouteRequest(route);

    // get its sequence number
    uint64_t seqnum = cpi_GetSequenceNumber(control);
    sprintf(truth, truth_format, seqnum);

    PARCJSON *test_json = ccnxControl_GetJson(control);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Expected '%s', actual '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    ccnxControl_Release(&control);
    cpiRouteEntry_Destroy(&route);
    cpiAddress_Destroy(&nexthop);
}

/**
 * Add route without lifeitme
 */
LONGBOW_TEST_CASE(Global, cpiForwarding_AddRoute_2)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"REGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}}}";
#elif defined(__linux__)
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"REGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}}}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    char truth[1024];

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);
    CCNxControl *control = ccnxControl_CreateAddRouteRequest(route);

    // get its sequence number
    uint64_t seqnum = cpi_GetSequenceNumber(control);
    sprintf(truth, truth_format, seqnum);

    PARCJSON *test_json = ccnxControl_GetJson(control);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Expected '%s', actual '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    ccnxControl_Release(&control);
    cpiRouteEntry_Destroy(&route);
    cpiAddress_Destroy(&nexthop);
}

/**
 * Add route without lifeitme or nexthop
 */
LONGBOW_TEST_CASE(Global, cpiForwarding_AddRoute_3)
{
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"REGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}}}";
    char truth[1024];

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);
    CCNxControl *control = ccnxControl_CreateAddRouteRequest(route);

    // get its sequence number
    uint64_t seqnum = cpi_GetSequenceNumber(control);
    sprintf(truth, truth_format, seqnum);

    PARCJSON *test_json = ccnxControl_GetJson(control);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Control message json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    ccnxControl_Release(&control);
    cpiRouteEntry_Destroy(&route);
}


LONGBOW_TEST_CASE(Global, cpiForwarding_AddRouteJsonTag)
{
    const char *tag = cpiForwarding_AddRouteJsonTag();
    assertTrue(strcmp(tag, cpiRegister) == 0, "cpiForwarding_AddRouteJsonTag not using defined value %s", cpiRegister);
}

LONGBOW_TEST_CASE(Global, cpiForwarding_AddRouteToSelf)
{
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"REGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":2147483647,\"FLAGS\":0,\"PROTOCOL\":\"LOCAL\",\"ROUTETYPE\":\"LONGEST\",\"COST\":0}}}";
    char truth[1024];

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    CCNxControl *control = ccnxControl_CreateAddRouteToSelfRequest(prefix);
    uint64_t seqnum = cpi_GetSequenceNumber(control);
    sprintf(truth, truth_format, seqnum);

    PARCJSON *test_json = ccnxControl_GetJson(control);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Control message json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    ccnxControl_Release(&control);
    ccnxName_Release(&prefix);
}

LONGBOW_TEST_CASE(Global, cpiForwarding_RemoveRoute)
{
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"UNREGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}}}";
    char truth[1024];

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    unsigned cost = 200;

    CPIRouteEntry *route =
        cpiRouteEntry_Create(prefix, ifidx, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);

    CCNxControl *control = ccnxControl_CreateRemoveRouteRequest(route);

    // get its sequence number
    uint64_t seqnum = cpi_GetSequenceNumber(control);
    sprintf(truth, truth_format, seqnum);

    PARCJSON *test_json = ccnxControl_GetJson(control);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Control message json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    ccnxControl_Release(&control);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiForwarding_RemoveRouteJsonTag)
{
    const char *tag = cpiForwarding_RemoveRouteJsonTag();
    assertTrue(strcmp(tag, cpiUnregister) == 0, "cpiForwarding_AddRouteJsonTag not using defined value %s", cpiUnregister);
}

LONGBOW_TEST_CASE(Global, cpiForwarding_RemoveRouteToSelf)
{
    char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"UNREGISTER\":{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":2147483647,\"FLAGS\":0,\"PROTOCOL\":\"LOCAL\",\"ROUTETYPE\":\"LONGEST\",\"COST\":0}}}";
    char truth[1024];

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    CCNxControl *control = ccnxControl_CreateRemoveRouteToSelfRequest(prefix);

    uint64_t seqnum = cpi_GetSequenceNumber(control);
    sprintf(truth, truth_format, seqnum);

    PARCJSON *test_json = ccnxControl_GetJson(control);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Control message json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    ccnxControl_Release(&control);
    ccnxName_Release(&prefix);
}

LONGBOW_TEST_CASE(Global, cpiForwarding_RouteFromControlMessage)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);
    CCNxControl *control = ccnxControl_CreateAddRouteRequest(route);

    CPIRouteEntry *test_route = cpiForwarding_RouteFromControlMessage(control);

    assertTrue(cpiRouteEntry_Equals(route, test_route),
               "messages not equa: expected %s got %s",
               parcJSON_ToCompactString(cpiRouteEntry_ToJson(route)),
               parcJSON_ToCompactString(cpiRouteEntry_ToJson(test_route)));

    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
    cpiRouteEntry_Destroy(&test_route);
    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, cpiForwarding_CreateRouteListRequest)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();

    assertTrue(ccnxControl_IsCPI(control), "Control message not a CPI message");
    assertTrue(cpi_GetMessageOperation(control) == CPI_PREFIX_REGISTRATION_LIST, "Message not a prefix regisration list");

    ccnxControl_Release(&control);
}

LONGBOW_TEST_CASE(Global, cpiForwarding_RouteListFromControlMessage)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();

    CPIRouteEntryList *routeList = cpiRouteEntryList_Create();
    PARCJSON *json = cpiRouteEntryList_ToJson(routeList);
    CCNxControl *response = cpi_CreateResponse(control, json);
    parcJSON_Release(&json);
    CPIRouteEntryList *test = cpiForwarding_RouteListFromControlMessage(response);
    assertTrue(cpiRouteEntryList_Equals(routeList, test), "Route lists not equal");

    ccnxControl_Release(&control);
    ccnxControl_Release(&response);
    cpiRouteEntryList_Destroy(&routeList);
    cpiRouteEntryList_Destroy(&test);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_Forwarding);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
