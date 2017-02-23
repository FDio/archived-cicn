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
#include "../metis_Connection.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

#include "testrig_MetisIoOperations.h"

LONGBOW_TEST_RUNNER(metis_Connection)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Connection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Connection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisConnection_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, metisConnection_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisConnection_Send);

    LONGBOW_RUN_TEST_CASE(Global, metisConnection_GetConnectionId);
    LONGBOW_RUN_TEST_CASE(Global, metisConnection_GetAddressPair);
    LONGBOW_RUN_TEST_CASE(Global, metisConnection_IsUp);
    LONGBOW_RUN_TEST_CASE(Global, metisConnection_IsLocal);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 3, true, true, true);
    longBowTestCase_SetClipBoardData(testCase, ops);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    mockIoOperationsData_Destroy(&ops);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisConnection_Acquire)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    MetisConnection *conn = metisConnection_Create(ops);

    assertTrue(conn->refCount == 1, "Wrong refcount, got %u expected %u", conn->refCount, 1);

    MetisConnection *copy = metisConnection_Acquire(conn);
    assertTrue(conn->refCount == 2, "Wrong refcount, got %u expected %u", conn->refCount, 2);

    metisConnection_Release(&copy);
    assertTrue(conn->refCount == 1, "Wrong refcount, got %u expected %u", conn->refCount, 1);

    metisConnection_Release(&conn);
}

LONGBOW_TEST_CASE(Global, metisConnection_Create_Destroy)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    MetisConnection *conn = metisConnection_Create(ops);
    assertNotNull(conn, "Got null connection");
    assertTrue(conn->refCount == 1, "Wrong refcount, got %u expected %u", conn->refCount, 1);

    metisConnection_Release(&conn);
    assertNull(conn, "Release did not null pointer");

    // the mock in testrig_MetisIoOperations does not destroy the IoOperations on destroy
    // so we can still look at the counters
    assertTrue(((MockIoOperationsData *) metisIoOperations_GetClosure(ops))->destroyCount == 1,
               "Destroy count is wrong, got %u expected %u",
               ((MockIoOperationsData *) metisIoOperations_GetClosure(ops))->destroyCount,
               1);
}

LONGBOW_TEST_CASE(Global, metisConnection_Send)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);

    MetisConnection *conn = metisConnection_Create(ops);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) metisTestDataV1_Interest_AllFields,
                                                         sizeof(metisTestDataV1_Interest_AllFields), 111, 2, logger);

    metisConnection_Send(conn, message);

    assertTrue(data->sendCount == 1, "Send count wrong, got %u expected %u", data->sendCount, 1);
    assertTrue(data->lastMessage == message, "Sent wrong message, got %p expected %p", (void *) data->lastMessage, (void *) message);

    metisMessage_Release(&message);
    metisConnection_Release(&conn);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisConnection_GetConnectionId)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);

    MetisConnection *conn = metisConnection_Create(ops);
    unsigned testid = metisConnection_GetConnectionId(conn);

    assertTrue(testid == data->id, "Got wrong id, got %u expected %u", testid, data->id);
    assertTrue(data->getConnectionIdCount == 1, "Wrong getConnectionIdCount, got %u expected %u", data->getConnectionIdCount, 1);

    metisConnection_Release(&conn);
}

LONGBOW_TEST_CASE(Global, metisConnection_GetAddressPair)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);

    MetisConnection *conn = metisConnection_Create(ops);

    unsigned beforeCount = data->getAddressPairCount;
    const MetisAddressPair *pair = metisConnection_GetAddressPair(conn);

    assertTrue(metisAddressPair_Equals(pair, data->addressPair), "Got wrong address pair");
    assertTrue(data->getAddressPairCount == beforeCount + 1, "Wrong getAddressPairCount, got %u expected %u", data->getAddressPairCount, beforeCount + 1);

    metisConnection_Release(&conn);
}

LONGBOW_TEST_CASE(Global, metisConnection_IsUp)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);

    MetisConnection *conn = metisConnection_Create(ops);

    unsigned beforeCount = data->isUpCount;
    bool isup = metisConnection_IsUp(conn);

    assertTrue(isup == data->isUp, "Got wrong isup, got %d expected %d", isup, data->isUp);
    assertTrue(data->isUpCount == beforeCount + 1, "Wrong isUpCount, got %u expected %u", data->isUpCount, beforeCount + 1);

    metisConnection_Release(&conn);
}

LONGBOW_TEST_CASE(Global, metisConnection_IsLocal)
{
    MetisIoOperations *ops = longBowTestCase_GetClipBoardData(testCase);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);

    MetisConnection *conn = metisConnection_Create(ops);

    unsigned beforeCount = data->isLocalCount;
    bool islocal = metisConnection_IsLocal(conn);

    assertTrue(islocal == data->isLocal, "Got wrong islocal, got %d expected %d", islocal, data->isLocal);
    assertTrue(data->isLocalCount == beforeCount + 1, "Wrong isLocalCount, got %u expected %u", data->isLocalCount, beforeCount + 1);

    metisConnection_Release(&conn);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Connection);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
