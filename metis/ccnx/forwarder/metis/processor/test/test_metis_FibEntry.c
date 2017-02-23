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
#include "../metis_FibEntry.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_FibEntry)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_FibEntry)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_FibEntry)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisFibEntry_AddNexthop);
    LONGBOW_RUN_TEST_CASE(Global, metisFibEntry_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisFibEntry_SetStrategy);
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

LONGBOW_TEST_CASE(Global, metisFibEntry_AddNexthop)
{
    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);
    MetisFibEntry *fibEntry = metisFibEntry_Create(tlvName,"random");

    CCNxName *ccnxName1 = ccnxName_CreateFromCString("lci:/foo/bar");
    CPIRouteEntry *cpiRouteEntry1 = cpiRouteEntry_Create(ccnxName1, 1, NULL, 0, 0, NULL, 1);
    CCNxName *ccnxName2 = ccnxName_CreateFromCString("lci:/foo/bar");
    CPIRouteEntry *cpiRouteEntry2 = cpiRouteEntry_Create(ccnxName2, 2, NULL, 0, 0, NULL, 1);
    metisFibEntry_AddNexthop(fibEntry, cpiRouteEntry1);
    metisFibEntry_AddNexthop(fibEntry, cpiRouteEntry2);

    assertTrue(metisFibEntry_NexthopCount(fibEntry) == 2, "wrong nexthop length, expected %u got %zu", 2, metisFibEntry_NexthopCount(fibEntry));
    
    cpiRouteEntry_Destroy(&cpiRouteEntry1);
    cpiRouteEntry_Destroy(&cpiRouteEntry2);    
    metisFibEntry_Release(&fibEntry);
    metisTlvName_Release(&tlvName);
    ccnxName_Release(&ccnxName);
}

LONGBOW_TEST_CASE(Global, metisFibEntry_Create_Destroy)
{
    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);

    size_t beforeMemory = parcMemory_Outstanding();
    MetisFibEntry *fibEntry = metisFibEntry_Create(tlvName, "random");
    metisFibEntry_Release(&fibEntry);
    size_t afterMemory = parcMemory_Outstanding();

    metisTlvName_Release(&tlvName);
    ccnxName_Release(&ccnxName);

    assertTrue(beforeMemory == afterMemory, "Memory imbalance on create/destroy: expected %zu got %zu", beforeMemory, afterMemory);
}


LONGBOW_TEST_CASE(Global, metisFibEntry_SetStrategy)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_FIXTURE(Local)
{
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

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_FibEntry);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
