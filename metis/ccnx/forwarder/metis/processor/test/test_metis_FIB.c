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
#include "../metis_FIB.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

LONGBOW_TEST_RUNNER(metis_FIB)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_FIB)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_FIB)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisFib_AddOrUpdate_Add);
    LONGBOW_RUN_TEST_CASE(Global, metisFib_AddOrUpdate_Update);
    LONGBOW_RUN_TEST_CASE(Global, metisFib_Create_Destroy);

    LONGBOW_RUN_TEST_CASE(Global, metisFib_Match_Exists);
    LONGBOW_RUN_TEST_CASE(Global, metisFib_Match_NotExists);
    LONGBOW_RUN_TEST_CASE(Global, metisFib_Match_ExcludeIngress);

    LONGBOW_RUN_TEST_CASE(Global, metisFib_Remove_NoEntry);
    LONGBOW_RUN_TEST_CASE(Global, metisFib_Remove_ExistsNotLast);
    LONGBOW_RUN_TEST_CASE(Global, metisFib_Remove_ExistsIsLast);

    LONGBOW_RUN_TEST_CASE(Global, metisFIB_Length);
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

LONGBOW_TEST_CASE(Global, metisFib_AddOrUpdate_Add)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    metisLogger_Release(&logger);

    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);
    unsigned interfaceIndex = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    CPIRouteEntry *route = cpiRouteEntry_Create(ccnxName, interfaceIndex, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);

    metisFIB_AddOrUpdate(fib, route, "random");
    size_t hashCodeTableLength = parcHashCodeTable_Length(fib->tableByName);

    MetisFibEntry *fibEntry = parcHashCodeTable_Get(fib->tableByName, tlvName);
    size_t nexthopCount = metisFibEntry_NexthopCount(fibEntry);

    cpiRouteEntry_Destroy(&route);
    metisTlvName_Release(&tlvName);
    metisFIB_Destroy(&fib);

    assertTrue(hashCodeTableLength == 1, "Wrong hash table length, expected %u got %zu", 1, hashCodeTableLength);
    assertTrue(nexthopCount == 1, "Wrong hash table length, expected %u got %zu", 1, nexthopCount);
}

LONGBOW_TEST_CASE(Global, metisFib_AddOrUpdate_Update)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    metisLogger_Release(&logger);

    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);
    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    // ----- Add
    CPIRouteEntry *route_1 = cpiRouteEntry_Create(ccnxName_Copy(ccnxName), interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, route_1, "random");

    // ----- Update
    unsigned interfaceIndex_2 = 33;
    CPIRouteEntry *route_2 = cpiRouteEntry_Create(ccnxName_Copy(ccnxName), interfaceIndex_2, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, route_2, "random");

    // ----- Measure
    size_t hashCodeTableLength = parcHashCodeTable_Length(fib->tableByName);
    MetisFibEntry *fibEntry = parcHashCodeTable_Get(fib->tableByName, tlvName);
    size_t nexthopCount = metisFibEntry_NexthopCount(fibEntry);


    cpiRouteEntry_Destroy(&route_1);
    cpiRouteEntry_Destroy(&route_2);
    ccnxName_Release(&ccnxName);
    metisTlvName_Release(&tlvName);
    metisFIB_Destroy(&fib);

    assertTrue(hashCodeTableLength == 1, "Wrong hash table length, expected %u got %zu", 1, hashCodeTableLength);
    assertTrue(nexthopCount == 2, "Wrong hash table length, expected %u got %zu", 2, nexthopCount);
}

LONGBOW_TEST_CASE(Global, metisFib_Create_Destroy)
{
    size_t beforeMemory = parcMemory_Outstanding();
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    metisLogger_Release(&logger);

    metisFIB_Destroy(&fib);
    size_t afterMemory = parcMemory_Outstanding();

    assertTrue(beforeMemory == afterMemory, "Memory imbalance on create/destroy: expected %zu got %zu", beforeMemory, afterMemory);
}

/**
 * Add /hello/ouch and lookup that name
 */
LONGBOW_TEST_CASE(Global, metisFib_Match_Exists)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);

    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    // ----- Add
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd, "random");

    // ----- Match
    MetisFibEntry *entry = metisFIB_Match(fib, interest);

    // ----- Measure
    size_t nexthopsLength = metisNumberSet_Length(metisFibEntry_GetNexthops(entry));

    // ----- Cleanup
    cpiRouteEntry_Destroy(&routeAdd);
    metisMessage_Release(&interest);
    metisFIB_Destroy(&fib);

    // ----- Validate
    assertTrue(nexthopsLength == 1, "Wrong nexthops length, expected %u got %zu", 1, nexthopsLength);
}

/**
 * Add /foo/bar to connection 10
 * Add /foo to connection 11
 * Forward an Interest /foo/bar/cat from connection 10.  Should select 11.
 */
LONGBOW_TEST_CASE(Global, metisFib_Match_ExcludeIngress)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);

    CCNxName *nameFoo = ccnxName_CreateFromCString("lci:/foo");
    CCNxName *nameFooBar = ccnxName_CreateFromCString("lci:/foo/bar");

    uint8_t encodedInterest[] = {
        0x01, 0x00, 0x00,   37, // ver = 1, type = interest, length = 37
        0xFF, 0x00, 0x00,    8, // hoplimit = 255, header length = 8
        // ------------------------
        0x00, 0x01, 0x00,   25, // type = interest, length = 25
        // ------------------------
        0x00, 0x00, 0x00,   21,   // type = name, length = 21
        0x00, 0x01, 0x00,    3,   // type = name, length = 3
        'f', 'o', 'o',
        0x00, 0x01, 0x00,    3,   // type = name, length = 3
        'b', 'a', 'r',
        0x00, 0x01, 0x00,    3,   // type = name, length = 3
        'c', 'a', 't',
    };

    MetisMessage *interest = metisMessage_CreateFromArray(encodedInterest, sizeof(encodedInterest), 10, 2, logger);
    metisLogger_Release(&logger);

    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    CPIRouteEntry *routeAdd;

    // ----- Add long route to Interface 10
    routeAdd = cpiRouteEntry_Create(nameFooBar, 10, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd, "random");
    cpiRouteEntry_Destroy(&routeAdd);

    // ----- Add short route to Interface 11
    routeAdd = cpiRouteEntry_Create(nameFoo, 11, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd, "random");
    cpiRouteEntry_Destroy(&routeAdd);

    // ----- Match
    MetisFibEntry *entry = metisFIB_Match(fib, interest);

    // ----- Measure
    size_t nexthopsLength = metisNumberSet_Length(metisFibEntry_GetNexthops(entry));

    // ----- Validate
    assertTrue(nexthopsLength == 1, "Wrong nexthops length, expected %u got %zu", 1, nexthopsLength);
    bool hasEgress = metisNumberSet_Contains(metisFibEntry_GetNexthops(entry), 11);
    assertTrue(hasEgress, "Egress interface 11 not in nexthop set");

    // ----- Cleanup
    metisMessage_Release(&interest);
    metisFIB_Destroy(&fib);
}


/**
 * Add /hello/ouch and lookup /party/ouch
 */
LONGBOW_TEST_CASE(Global, metisFib_Match_NotExists)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");

    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithOtherName, sizeof(metisTestDataV0_InterestWithOtherName), 1, 2, logger);
    metisLogger_Release(&logger);

    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    // ----- Add
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd, "random");

    // ----- Match
    MetisFibEntry *entry = metisFIB_Match(fib, interest);

    // ----- Measure
    assertTrue(entry == NULL, "expected null");
        cpiRouteEntry_Destroy(&routeAdd);
        metisMessage_Release(&interest);
        metisFIB_Destroy(&fib);
    
    //size_t nexthopsLength = metisNumberSet_Length(metisFibEntry_GetNexthops(entry));

    // ----- Cleanup
    //cpiRouteEntry_Destroy(&routeAdd);
    //metisMessage_Release(&interest);
    //metisFibEntry_Release(&entry);
    //metisFIB_Destroy(&fib);

    // ----- Validate
    //assertTrue(nexthopsLength == 0, "Wrong nexthops length, expected %u got %zu", 0, nexthopsLength);
}

/**
 * Add /foo/bar and try to remove /baz
 */
LONGBOW_TEST_CASE(Global, metisFib_Remove_NoEntry)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    metisLogger_Release(&logger);

    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/foo/bar");
    CCNxName *ccnxNameToRemove = ccnxName_CreateFromCString("lci:/baz");
    MetisTlvName *tlvNameToCheck = metisTlvName_CreateFromCCNxName(ccnxNameToAdd);
    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    // ----- Add
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd, "random");

    // ----- Remove
    CPIRouteEntry *routeRemove = cpiRouteEntry_Create(ccnxNameToRemove, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_Remove(fib, routeRemove);

    // ----- Measure
    size_t hashCodeTableLength = parcHashCodeTable_Length(fib->tableByName);
    MetisFibEntry *fibEntry = parcHashCodeTable_Get(fib->tableByName, tlvNameToCheck);
    size_t nexthopCount = metisFibEntry_NexthopCount(fibEntry);

    // ----- Cleanup
    cpiRouteEntry_Destroy(&routeAdd);
    cpiRouteEntry_Destroy(&routeRemove);
    metisTlvName_Release(&tlvNameToCheck);
    metisFIB_Destroy(&fib);

    // ----- Validate
    assertTrue(hashCodeTableLength == 1, "Wrong hash table length, expected %u got %zu", 1, hashCodeTableLength);
    assertTrue(nexthopCount == 1, "Wrong hash table length, expected %u got %zu", 1, nexthopCount);
}

LONGBOW_TEST_CASE(Global, metisFib_Remove_ExistsNotLast)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    metisLogger_Release(&logger);

    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/foo/bar");
    CCNxName *ccnxNameToRemove = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvNameToCheck = metisTlvName_CreateFromCCNxName(ccnxNameToAdd);
    unsigned interfaceIndex_1 = 11;
    unsigned interfaceIndex_2 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    // ----- Add two next hops
    CPIRouteEntry *routeAdd1 = cpiRouteEntry_Create(ccnxName_Copy(ccnxNameToAdd), interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd1, "random");

    CPIRouteEntry *routeAdd2 = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_2, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd2, "random");

    // ----- Remove
    CPIRouteEntry *routeRemove = cpiRouteEntry_Create(ccnxNameToRemove, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_Remove(fib, routeRemove);

    // ----- Measure
    size_t hashCodeTableLength = parcHashCodeTable_Length(fib->tableByName);
    MetisFibEntry *fibEntry = parcHashCodeTable_Get(fib->tableByName, tlvNameToCheck);
    size_t nexthopCount = metisFibEntry_NexthopCount(fibEntry);

    // ----- Cleanup
    cpiRouteEntry_Destroy(&routeAdd1);
    cpiRouteEntry_Destroy(&routeAdd2);
    cpiRouteEntry_Destroy(&routeRemove);
    metisTlvName_Release(&tlvNameToCheck);
    metisFIB_Destroy(&fib);

    // ----- Validate
    assertTrue(hashCodeTableLength == 1, "Wrong hash table length, expected %u got %zu", 1, hashCodeTableLength);
    assertTrue(nexthopCount == 1, "Wrong hash table length, expected %u got %zu", 1, nexthopCount);
}

/**
 * Remove the last nexthop for a route.  should remove the route
 */
LONGBOW_TEST_CASE(Global, metisFib_Remove_ExistsIsLast)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    metisLogger_Release(&logger);

    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/foo/bar");
    CCNxName *ccnxNameToRemove = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvNameToCheck = metisTlvName_CreateFromCCNxName(ccnxNameToAdd);
    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    // ----- Add
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd, "random");

    // ----- Remove
    CPIRouteEntry *routeRemove = cpiRouteEntry_Create(ccnxNameToRemove, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_Remove(fib, routeRemove);

    // ----- Measure
    size_t hashCodeTableLength = parcHashCodeTable_Length(fib->tableByName);

    // ----- Cleanup
    cpiRouteEntry_Destroy(&routeAdd);
    cpiRouteEntry_Destroy(&routeRemove);
    metisTlvName_Release(&tlvNameToCheck);
    metisFIB_Destroy(&fib);

    // ----- Validate
    assertTrue(hashCodeTableLength == 0, "Wrong hash table length, expected %u got %zu", 0, hashCodeTableLength);
}

LONGBOW_TEST_CASE(Global, metisFIB_Length)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);

    //    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/%02=hello/%F0%00=ouch");
    CCNxName *ccnxNameToAdd = ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);

    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    // ----- Add
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_1, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(fib, routeAdd, "random");

    // ----- Measure
    size_t tableLength = metisFIB_Length(fib);

    // ----- Cleanup
    cpiRouteEntry_Destroy(&routeAdd);
    metisMessage_Release(&interest);
    metisFIB_Destroy(&fib);

    // ----- Validate
    assertTrue(tableLength == 1, "Wrong table length, expected %u got %zu", 1, tableLength);
}

// ====================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _hashTableFunction_FibEntryDestroyer);
    LONGBOW_RUN_TEST_CASE(Local, _hashTableFunction_TlvNameDestroyer);
    LONGBOW_RUN_TEST_CASE(Local, _metisFIB_CreateFibEntry);
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

LONGBOW_TEST_CASE(Local, _hashTableFunction_FibEntryDestroyer)
{
    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);
    MetisFibEntry *fibEntry = metisFibEntry_Create(tlvName, "random");

    _hashTableFunction_FibEntryDestroyer((void **) &fibEntry);

    metisTlvName_Release(&tlvName);
    ccnxName_Release(&ccnxName);
    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance after hashTableFunction_TlvNameDestroyer: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Local, _hashTableFunction_TlvNameDestroyer)
{
    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);

    _hashTableFunction_TlvNameDestroyer((void **) &tlvName);
    ccnxName_Release(&ccnxName);

    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance after hashTableFunction_TlvNameDestroyer: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Local, _metisFIB_CreateFibEntry)
{
    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisFIB *fib = metisFIB_Create(logger);
    metisLogger_Release(&logger);

    _metisFIB_CreateFibEntry(fib, tlvName, "random");
    size_t hashCodeTableLength = parcHashCodeTable_Length(fib->tableByName);

    metisFIB_Destroy(&fib);
    metisTlvName_Release(&tlvName);
    ccnxName_Release(&ccnxName);

    assertTrue(hashCodeTableLength == 1, "Wrong hash table size, expected %u got %zu", 1, hashCodeTableLength);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_FIB);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
