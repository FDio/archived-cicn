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


#include <config.h>

#include <stdio.h>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../cpi_RouteEntry.c"

LONGBOW_TEST_RUNNER(cpi_RouteEntry)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Getters);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_RouteEntry)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_RouteEntry)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_Copy);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_Equals);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_CreateSymbolic);

    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_ToJson_1);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_ToJson_2);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_ToJson_3);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_ToJson_4);

    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_FromJson_1);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_FromJson_2);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_FromJson_3);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_FromJson_4);
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

LONGBOW_TEST_CASE(Global, cpiRouteEntry_Create_Destroy)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    unsigned cost = 4;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);
    cpiRouteEntry_Destroy(&route);

    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance on create/destroy: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_Copy)
{
    CCNxName *prefix_a = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *a = cpiRouteEntry_Create(prefix_a, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
    CPIRouteEntry *b = cpiRouteEntry_Copy(a);

    assertTrue(cpiRouteEntry_Equals(a, b), "Copy did not compare as equals");
    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&a);
    cpiRouteEntry_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_Equals)
{
    CCNxName *prefix_a = ccnxName_CreateFromCString("lci:/howdie/stranger");
    CCNxName *prefix_b = ccnxName_Copy(prefix_a);
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *a = cpiRouteEntry_Create(prefix_a, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
    CPIRouteEntry *b = cpiRouteEntry_Create(prefix_b, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    assertTrue(cpiRouteEntry_Equals(a, b), "Equals did not compare correctly");

    cpiRouteEntry_Destroy(&a);
    cpiRouteEntry_Destroy(&b);
    cpiAddress_Destroy(&nexthop);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_CreateSymbolic)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned cost = 4;

    CPIRouteEntry *route = cpiRouteEntry_CreateSymbolic(prefix, "tun0", cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);
    cpiRouteEntry_Destroy(&route);

    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance on create/destroy: %u", parcMemory_Outstanding());
}


/**
 * Add route with all options
 */
LONGBOW_TEST_CASE(Global, cpiRouteEntry_ToJson_1)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char truth[] = "{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200,\"LIFETIME\":[3600,0]}";
#elif defined(__linux__)
    char truth[] = "{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200,\"LIFETIME\":[3600,0]}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    PARCJSON *test_json = cpiRouteEntry_ToJson(route);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Route json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    cpiRouteEntry_Destroy(&route);
    cpiAddress_Destroy(&nexthop);
    parcJSON_Release(&test_json);
}

/**
 * Add route without lifeitme
 */
LONGBOW_TEST_CASE(Global, cpiRouteEntry_ToJson_2)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char truth[] = "{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}";
#elif defined(__linux__)
    char truth[] = "{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"NEXTHOP\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAAAAQDAgEAAAAAAAAAAA==\"},\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);
    PARCJSON *test_json = cpiRouteEntry_ToJson(route);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Route json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    cpiRouteEntry_Destroy(&route);
    cpiAddress_Destroy(&nexthop);
    parcJSON_Release(&test_json);
}

/**
 * Add route without lifeitme or nexthop
 */
LONGBOW_TEST_CASE(Global, cpiRouteEntry_ToJson_3)
{
    char truth[] = "{\"PREFIX\":\"ccnx:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}";

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);

    PARCJSON *test_json = cpiRouteEntry_ToJson(route);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Control message json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    cpiRouteEntry_Destroy(&route);
    parcJSON_Release(&test_json);
}

/**
 * Add route with symbolic name
 */
LONGBOW_TEST_CASE(Global, cpiRouteEntry_ToJson_4)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
    char truth[] = "{\"PREFIX\":\"ccnx:/howdie/stranger\",\"SYMBOLIC\":\"tun0\",\"INTERFACE\":55,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200,\"LIFETIME\":[3600,0]}";

    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_CreateSymbolic(prefix, "tun0", cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
    cpiRouteEntry_SetInterfaceIndex(route, ifidx);

    PARCJSON *test_json = cpiRouteEntry_ToJson(route);
    char *test = parcJSON_ToCompactString(test_json);
    assertTrue(strcasecmp(truth, test) == 0, "Route json does not match, expected '%s', got '%s'", truth, test);
    parcMemory_Deallocate((void **) &test);

    cpiRouteEntry_Destroy(&route);
    parcJSON_Release(&test_json);
}

/**
 * Add route with all options
 */
LONGBOW_TEST_CASE(Global, cpiRouteEntry_FromJson_1)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route_truth = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    PARCJSON *truth_json = cpiRouteEntry_ToJson(route_truth);

    CPIRouteEntry *route_test = cpiRouteEntry_FromJson(truth_json);
    assertTrue(cpiRouteEntry_Equals(route_truth, route_test), "FromJson does not match");


    cpiRouteEntry_Destroy(&route_truth);
    cpiRouteEntry_Destroy(&route_test);
    cpiAddress_Destroy(&nexthop);
    parcJSON_Release(&truth_json);
}

/**
 * Add route without lifeitme
 */
LONGBOW_TEST_CASE(Global, cpiRouteEntry_FromJson_2)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    unsigned cost = 200;

    CPIRouteEntry *route_truth = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);

    PARCJSON *truth_json = cpiRouteEntry_ToJson(route_truth);

    CPIRouteEntry *route_test = cpiRouteEntry_FromJson(truth_json);
    assertTrue(cpiRouteEntry_Equals(route_truth, route_test), "FromJson does not match");


    cpiRouteEntry_Destroy(&route_truth);
    cpiRouteEntry_Destroy(&route_test);
    cpiAddress_Destroy(&nexthop);
    parcJSON_Release(&truth_json);
}

/**
 * Add route without lifeitme or nexthop
 */
LONGBOW_TEST_CASE(Global, cpiRouteEntry_FromJson_3)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    unsigned cost = 200;

    CPIRouteEntry *route_truth = cpiRouteEntry_Create(prefix, ifidx, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, cost);

    PARCJSON *truth_json = cpiRouteEntry_ToJson(route_truth);

    CPIRouteEntry *route_test = cpiRouteEntry_FromJson(truth_json);
    assertTrue(cpiRouteEntry_Equals(route_truth, route_test), "FromJson does not match");


    cpiRouteEntry_Destroy(&route_truth);
    cpiRouteEntry_Destroy(&route_test);
    parcJSON_Release(&truth_json);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_FromJson_4)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route_truth = cpiRouteEntry_CreateSymbolic(prefix, "tun0", cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);
    cpiRouteEntry_SetInterfaceIndex(route_truth, ifidx);

    PARCJSON *truth_json = cpiRouteEntry_ToJson(route_truth);

    CPIRouteEntry *route_test = cpiRouteEntry_FromJson(truth_json);
    assertTrue(cpiRouteEntry_Equals(route_truth, route_test), "FromJson does not match");

    const char *symbolic = cpiRouteEntry_GetSymbolicName(route_test);
    assertTrue(strcmp(symbolic, "tun0") == 0, "wrong symbolic name expected 'tun0' got '%s'", symbolic);

    cpiRouteEntry_Destroy(&route_truth);
    cpiRouteEntry_Destroy(&route_test);
    parcJSON_Release(&truth_json);
}


// ====================================================

LONGBOW_TEST_FIXTURE(Getters)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetCost);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetInterfaceIndex);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetLifetime);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetNexthop);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetPrefix);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetRouteProtocolType);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetRouteType);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntry_GetSymbolicName);
}

LONGBOW_TEST_FIXTURE_SETUP(Getters)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Getters)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetCost)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    assertTrue(cpiRouteEntry_GetCost(route) == cost, "Got wrong cost, expected %u got %u", cost, cpiRouteEntry_GetCost(route));
    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetInterfaceIndex)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    assertTrue(cpiRouteEntry_GetInterfaceIndex(route) == ifidx, "Got wrong cost, expected %u got %u", ifidx, cpiRouteEntry_GetInterfaceIndex(route));
    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetLifetime)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    struct timeval test_time = cpiRouteEntry_GetLifetime(route);

    assertTrue(lifetime.tv_sec == test_time.tv_sec && lifetime.tv_usec == test_time.tv_usec,
               "Got wrong lifetime, expected %.6f got %.6f",
               lifetime.tv_sec + 1E-6 * lifetime.tv_usec,
               test_time.tv_sec + 1E-6 * test_time.tv_usec);
    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetNexthop)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    const CPIAddress *test = cpiRouteEntry_GetNexthop(route);

    assertTrue(cpiAddress_Equals(nexthop, test),
               "Got wrong nexthop, expected %s got %s",
               cpiAddress_ToString(nexthop),
               cpiAddress_ToString(test));

    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetPrefix)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    const CCNxName *test_prefix = cpiRouteEntry_GetPrefix(route);

    assertTrue(ccnxName_Equals(prefix, test_prefix),
               "Got wrong name, expected %s got %s",
               ccnxName_ToString(prefix),
               ccnxName_ToString(test_prefix));

    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetRouteProtocolType)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    assertTrue(cpiRouteEntry_GetRouteProtocolType(route) == cpiNameRouteProtocolType_STATIC,
               "Got wrong protocol, expected %d got %d",
               cpiNameRouteProtocolType_STATIC,
               cpiRouteEntry_GetRouteProtocolType(route));

    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetRouteType)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    unsigned ifidx = 55;
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;

    CPIRouteEntry *route = cpiRouteEntry_Create(prefix, ifidx, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    assertTrue(cpiRouteEntry_GetRouteType(route) == cpiNameRouteType_LONGEST_MATCH,
               "Got wrong route type, expected %d got %d",
               cpiNameRouteType_LONGEST_MATCH,
               cpiRouteEntry_GetRouteType(route));

    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntry_GetSymbolicName)
{
    CCNxName *prefix = ccnxName_CreateFromCString("lci:/howdie/stranger");
    CPIAddress *nexthop = cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_addr.s_addr = 0x01020304 });
    struct timeval lifetime = { 3600, 0 };
    unsigned cost = 200;
    const char *symbolicName = "tun0";

    CPIRouteEntry *route = cpiRouteEntry_CreateSymbolic(prefix, symbolicName, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, &lifetime, cost);

    const char *test = cpiRouteEntry_GetSymbolicName(route);

    assertTrue(strcmp(symbolicName, test) == 0, "Got wrong symbolic name, expected %s got %s", symbolicName, test);
    cpiAddress_Destroy(&nexthop);
    cpiRouteEntry_Destroy(&route);
}

// =============================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_RouteEntry);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
