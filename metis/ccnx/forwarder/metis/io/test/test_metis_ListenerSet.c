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
#include "../metis_ListenerSet.c"
#include <parc/algol/parc_SafeMemory.h>

#include <LongBow/unit-test.h>

#include "testrig_MetisListenerOps.c"


LONGBOW_TEST_RUNNER(metis_ListenerSet)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_ListenerSet)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_ListenerSet)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Add_Single);
    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Add_Unique);
    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Add_Duplicate);
    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Create_Destroy);


    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Length);
    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Get);
    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Find_InSet);
    LONGBOW_RUN_TEST_CASE(Global, metisListenerSet_Find_NotInSet);
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

// Adds a single MockListenerData to the listener set.
static MockListenerData *
addSingle(MetisListenerSet *set)
{
    CPIAddress *listenAddress = cpiAddress_CreateFromInterface(44);
    MockListenerData *data = mockListenData_Create(1, listenAddress, METIS_ENCAP_ETHER);
    MetisListenerOps *listenerOps = mockListener_Create(data);

    bool success = metisListenerSet_Add(set, listenerOps);
    assertTrue(success, "Got failure adding one listener to the set");
    assertTrue(parcArrayList_Size(set->listOfListeners) == 1,
               "Got wrong list length, got %zu expected %u",
               parcArrayList_Size(set->listOfListeners), 1);

    cpiAddress_Destroy(&listenAddress);
    return data;
}

LONGBOW_TEST_CASE(Global, metisListenerSet_Add_Single)
{
    MetisListenerSet *set = metisListenerSet_Create();

    MockListenerData *data = addSingle(set);

    metisListenerSet_Destroy(&set);
    assertTrue(data->destroyCount == 1,
               "Wrong destroy count, got %u expected %u",
               data->destroyCount, 1);

    mockListenerData_Destroy(&data);
}

LONGBOW_TEST_CASE(Global, metisListenerSet_Add_Unique)
{
    CPIAddress *listenAddress_A = cpiAddress_CreateFromInterface(44);
    MockListenerData *data_A = mockListenData_Create(1, listenAddress_A, METIS_ENCAP_ETHER);
    MetisListenerOps *listenerOps_A = mockListener_Create(data_A);

    CPIAddress *listenAddress_B = cpiAddress_CreateFromInterface(55);
    MockListenerData *data_B = mockListenData_Create(1, listenAddress_B, METIS_ENCAP_ETHER);
    MetisListenerOps *listenerOps_B = mockListener_Create(data_B);


    MetisListenerSet *set = metisListenerSet_Create();
    bool success_A = metisListenerSet_Add(set, listenerOps_A);
    assertTrue(success_A, "Got failure adding listener A to the set");

    bool success_B = metisListenerSet_Add(set, listenerOps_B);
    assertTrue(success_B, "Got failure adding listener B to the set");

    cpiAddress_Destroy(&listenAddress_A);
    cpiAddress_Destroy(&listenAddress_B);
    metisListenerSet_Destroy(&set);

    mockListenerData_Destroy(&data_A);
    mockListenerData_Destroy(&data_B);
}

LONGBOW_TEST_CASE(Global, metisListenerSet_Add_Duplicate)
{
    CPIAddress *listenAddress_A = cpiAddress_CreateFromInterface(44);
    MockListenerData *data_A = mockListenData_Create(1, listenAddress_A, METIS_ENCAP_ETHER);
    MetisListenerOps *listenerOps_A = mockListener_Create(data_A);

    CPIAddress *listenAddress_B = cpiAddress_CreateFromInterface(44);
    MockListenerData *data_B = mockListenData_Create(1, listenAddress_B, METIS_ENCAP_ETHER);
    MetisListenerOps *listenerOps_B = mockListener_Create(data_B);


    MetisListenerSet *set = metisListenerSet_Create();
    bool success_A = metisListenerSet_Add(set, listenerOps_A);
    assertTrue(success_A, "Got failure adding listener A to the set");

    bool success_B = metisListenerSet_Add(set, listenerOps_B);
    assertFalse(success_B, "Got success adding listener B to the set, duplicate should have failed");

    cpiAddress_Destroy(&listenAddress_A);
    cpiAddress_Destroy(&listenAddress_B);
    metisListenerSet_Destroy(&set);

    mockListener_Destroy(&listenerOps_B);
    mockListenerData_Destroy(&data_A);
    mockListenerData_Destroy(&data_B);
}


LONGBOW_TEST_CASE(Global, metisListenerSet_Create_Destroy)
{
    MetisListenerSet *set = metisListenerSet_Create();
    assertNotNull(set, "Got null from Create");

    metisListenerSet_Destroy(&set);
    assertNull(set, "Destroy did not null parameter");
}

LONGBOW_TEST_CASE(Global, metisListenerSet_Length)
{
    MetisListenerSet *set = metisListenerSet_Create();
    MockListenerData *data = addSingle(set);

    size_t length = metisListenerSet_Length(set);

    metisListenerSet_Destroy(&set);
    mockListenerData_Destroy(&data);

    assertTrue(length == 1,
               "Wrong length, got %zu expected %u",
               length, 1);
}

LONGBOW_TEST_CASE(Global, metisListenerSet_Get)
{
    MetisListenerSet *set = metisListenerSet_Create();
    MockListenerData *data = addSingle(set);

    MetisListenerOps *ops = metisListenerSet_Get(set, 0);

    assertNotNull(ops, "Did not fetch the listener ops");

    metisListenerSet_Destroy(&set);
    mockListenerData_Destroy(&data);
}

LONGBOW_TEST_CASE(Global, metisListenerSet_Find_InSet)
{
    MetisListenerSet *set = metisListenerSet_Create();
    MockListenerData *data = addSingle(set);

    MetisListenerOps *ops = metisListenerSet_Find(set, data->encapType, data->listenAddress);
    assertNotNull(ops, "Did not retrieve the listener that is in the set");

    metisListenerSet_Destroy(&set);
    mockListenerData_Destroy(&data);
}

LONGBOW_TEST_CASE(Global, metisListenerSet_Find_NotInSet)
{
    MetisListenerSet *set = metisListenerSet_Create();
    MockListenerData *data = addSingle(set);

    // use wrong encap type
    MetisListenerOps *ops = metisListenerSet_Find(set, data->encapType + 1, data->listenAddress);
    assertNull(ops, "Should not have found anything with wrong encap type");

    metisListenerSet_Destroy(&set);
    mockListenerData_Destroy(&data);
}


LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisListenerSet_DestroyListenerOps);
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

LONGBOW_TEST_CASE(Local, metisListenerSet_DestroyListenerOps)
{
    testUnimplemented("");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_ListenerSet);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
