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
#include "../metis_ConnectionManager.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metis_ConnectionManager)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_ConnectionManager)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_ConnectionManager)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisConnectionManager_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisConnectionManager_Destroy);
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

LONGBOW_TEST_CASE(Global, metisConnectionManager_Create)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisConnectionManager_Destroy)
{
    testUnimplemented("");
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_MessengerCallback);
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_NotifyApplications);
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_ProcessDownMissive);
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_ProcessQueue);
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_ProcessUpMissive);
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_ProcessDestroyMissive);

    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_ProcessCloseMissive_RemoveConnection);
    LONGBOW_RUN_TEST_CASE(Local, metisConnectionManager_ProcessCloseMissive_RemoveRoutes);
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

LONGBOW_TEST_CASE(Local, metisConnectionManager_MessengerCallback)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, metisConnectionManager_NotifyApplications)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, metisConnectionManager_ProcessDownMissive)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, metisConnectionManager_ProcessQueue)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, metisConnectionManager_ProcessUpMissive)
{
    testUnimplemented("");
}

typedef struct my_connection {
    MetisAddressPair *addressPair;
    unsigned connectionId;
} MyConnection;

static const MetisAddressPair *
getAddressPair(const MetisIoOperations *ops)
{
    MyConnection *myconn = (MyConnection *) ops->context;
    return myconn->addressPair;
}

static unsigned
getConnectionId(const MetisIoOperations *ops)
{
    MyConnection *myconn = (MyConnection *) ops->context;
    return myconn->connectionId;
}

static void
myConnectionDestroy(MetisIoOperations **opsPtr)
{
    MetisIoOperations *ops = *opsPtr;
    MyConnection *myconn = (MyConnection *) ops->context;
    metisAddressPair_Release(&myconn->addressPair);
    parcMemory_Deallocate((void **) &ops);
    *opsPtr = NULL;
}

static MetisConnection *
createConnection(unsigned connectionId)
{
    MyConnection *myconn = parcMemory_AllocateAndClear(sizeof(MyConnection));
    assertNotNull(myconn, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MyConnection));
    CPIAddress *a = cpiAddress_CreateFromInterface(1);
    myconn->addressPair = metisAddressPair_Create(a, a);
    myconn->connectionId = connectionId;

    cpiAddress_Destroy(&a);

    MetisIoOperations *ops = parcMemory_AllocateAndClear(sizeof(MetisIoOperations));
    assertNotNull(ops, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisIoOperations));
    memset(ops, 0, sizeof(MetisIoOperations));
    ops->context = myconn;
    ops->destroy = myConnectionDestroy;
    ops->getAddressPair = getAddressPair;
    ops->getConnectionId = getConnectionId;

    MetisConnection *conn = metisConnection_Create(ops);
    return conn;
}

static void
addRoute(MetisForwarder *metis, const char *name, unsigned connectionId)
{
    CCNxName *uri = ccnxName_CreateFromURI(name);
    CPIRouteEntry *route = cpiRouteEntry_Create(uri, connectionId, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);

    metisForwarder_AddOrUpdateRoute(metis, route);
    ccnxName_Release(&uri);
}

/**
 * We add a connection, then send a CLOSE message, make sure the connection
 * is no longer in the connection table.
 */
LONGBOW_TEST_CASE(Local, metisConnectionManager_ProcessCloseMissive_RemoveConnection)
{
    unsigned connectionId = 1000;

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    MetisConnection *conn = createConnection(connectionId);
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);

    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    // send close message
    metisMessenger_Send(metisForwarder_GetMessenger(metis), metisMissive_Create(MetisMissiveType_ConnectionClosed, connectionId));
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    // now make sure its gone
    const MetisConnection *test = metisConnectionTable_FindById(metisForwarder_GetConnectionTable(metis), connectionId);
    assertNull(test, "Return from connection table should be null, got %p\n", (const void *) test);

    metisForwarder_Destroy(&metis);
}

/**
 * We add a connection and a route that uses that connection, then send a CLOSE message,
 * then make sure the connection is no longer in the routing table.
 */
LONGBOW_TEST_CASE(Local, metisConnectionManager_ProcessCloseMissive_RemoveRoutes)
{
    unsigned connectionId = 1001;

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    MetisConnection *conn = createConnection(connectionId);
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);

    addRoute(metis, "lci:/foo/bar", connectionId);

    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    // send close message
    metisMessenger_Send(metisForwarder_GetMessenger(metis), metisMissive_Create(MetisMissiveType_ConnectionClosed, connectionId));
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(metis), &((struct timeval) { 0, 10000 }));

    // now make sure its gone
    MetisFibEntryList *fiblist = metisForwarder_GetFibEntries(metis);
    for (size_t i = 0; i < metisFibEntryList_Length(fiblist); i++) {
        MetisFibEntry *fibentry = metisFibEntryList_Get(fiblist, i);
        assertTrue(metisFibEntry_NexthopCount(fibentry) == 0, "Wrong nexthop count, expected 0 got %zu", metisFibEntry_NexthopCount(fibentry));
    }

    metisForwarder_Destroy(&metis);
}

LONGBOW_TEST_CASE(Local, metisConnectionManager_ProcessDestroyMissive)
{
    testUnimplemented("");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_ConnectionManager);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
