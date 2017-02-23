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
#include "../metis_ConnectionList.c"
#include <parc/algol/parc_SafeMemory.h>
#include "testrig_MetisIoOperations.h"
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metis_ConnectionList)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_ConnectionList)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_ConnectionList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisConnectionList_Append);
    LONGBOW_RUN_TEST_CASE(Global, metisConnectionList_Create_Destroy);

    LONGBOW_RUN_TEST_CASE(Global, metisConnectionList_Get);
    LONGBOW_RUN_TEST_CASE(Global, metisConnectionList_Length);
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

LONGBOW_TEST_CASE(Global, metisConnectionList_Append)
{
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 3, true, true, true);
    MetisConnection *connection = metisConnection_Create(ops);

    MetisConnectionList *list = metisConnectionList_Create();
    metisConnectionList_Append(list, connection);
    metisConnection_Release(&connection);

    assertTrue(parcArrayList_Size(list->listOfConnections) == 1,
               "Got wrong list size, got %zu expected %u",
               parcArrayList_Size(list->listOfConnections), 1);

    metisConnectionList_Destroy(&list);
    mockIoOperationsData_Destroy(&ops);
}

LONGBOW_TEST_CASE(Global, metisConnectionList_Create_Destroy)
{
    MetisConnectionList *list = metisConnectionList_Create();
    assertNotNull(list, "Got null from Create");

    metisConnectionList_Destroy(&list);
    assertNull(list, "Destroy did not null the parameter");
}

LONGBOW_TEST_CASE(Global, metisConnectionList_Get)
{
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 3, true, true, true);
    MetisConnection *connection = metisConnection_Create(ops);

    MetisConnectionList *list = metisConnectionList_Create();
    metisConnectionList_Append(list, connection);

    MetisConnection *test = metisConnectionList_Get(list, 0);
    assertTrue(test == connection,
               "Got wrong connection, got %p expected %p",
               (void *) test, (void *) connection);

    metisConnection_Release(&connection);
    metisConnectionList_Destroy(&list);
    mockIoOperationsData_Destroy(&ops);
}

LONGBOW_TEST_CASE(Global, metisConnectionList_Length)
{
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 3, true, true, true);
    MetisConnection *connection = metisConnection_Create(ops);

    MetisConnectionList *list = metisConnectionList_Create();
    metisConnectionList_Append(list, connection);

    size_t length = metisConnectionList_Length(list);
    assertTrue(length == 1,
               "Got wrong list size, got %zu expected %u",
               length, 1);

    metisConnection_Release(&connection);
    metisConnectionList_Destroy(&list);
    mockIoOperationsData_Destroy(&ops);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionList_ArrayDestroyer);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, metisConnectionList_ArrayDestroyer)
{
    testUnimplemented("");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_ConnectionList);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
