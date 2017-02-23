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
#include "../rta_ConnectionTable.c"
#include "../rta_ProtocolStack.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(rta_ConnectionTable)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_ConnectionTable)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_ConnectionTable)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, rtaConnectionTable_AddConnection);
    LONGBOW_RUN_TEST_CASE(Global, rtaConnectionTable_AddConnection_TooMany);
    LONGBOW_RUN_TEST_CASE(Global, rtaConnectionTable_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, rtaConnectionTable_GetByApiFd);
    LONGBOW_RUN_TEST_CASE(Global, rtaConnectionTable_GetByTransportFd);
    LONGBOW_RUN_TEST_CASE(Global, rtaConnectionTable_Remove);
    LONGBOW_RUN_TEST_CASE(Global, rtaConnectionTable_RemoveByStack);
}

typedef struct test_data {
    PARCRingBuffer1x1 *commandRingBuffer;
    PARCNotifier *commandNotifier;

    RtaFramework *framework;

    // in some tests we use two protocol stacks
    RtaProtocolStack *stack_a;
    RtaProtocolStack *stack_b;
} TestData;

static RtaConnection *
createConnection(RtaProtocolStack *stack, int api_fd, int transport_fd)
{
    // -------
    // Create a connection to use in the table
    PARCJSON *params = parcJSON_ParseString("{}");
    RtaCommandOpenConnection *openConnection = rtaCommandOpenConnection_Create(stack->stack_id, api_fd, transport_fd, params);

    // Create a connection that goes in the connection table
    RtaConnection *conn = rtaConnection_Create(stack, openConnection);
    assertNotNull(conn, "Got null connection from rtaConnection_Create");

    rtaCommandOpenConnection_Release(&openConnection);
    parcJSON_Release(&params);
    return conn;
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    // ---------------------------
    // To test a connection table, we need to create a Framework and a Protocol stack

    data->commandRingBuffer = parcRingBuffer1x1_Create(128, NULL);
    data->commandNotifier = parcNotifier_Create();

    data->framework = rtaFramework_Create(data->commandRingBuffer, data->commandNotifier);

    // fake out a protocol stack
    data->stack_a = parcMemory_AllocateAndClear(sizeof(RtaProtocolStack));
    assertNotNull(data->stack_a, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(RtaProtocolStack));
    data->stack_a->stack_id = 1;
    data->stack_a->framework = data->framework;

    // fake out a protocol stack
    data->stack_b = parcMemory_AllocateAndClear(sizeof(RtaProtocolStack));
    assertNotNull(data->stack_b, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(RtaProtocolStack));
    data->stack_b->stack_id = 2;
    data->stack_b->framework = data->framework;

    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    // now cleanup everything
    rtaFramework_Destroy(&data->framework);
    parcNotifier_Release(&data->commandNotifier);
    parcRingBuffer1x1_Release(&data->commandRingBuffer);

    parcMemory_Deallocate((void **) &(data->stack_a));
    parcMemory_Deallocate((void **) &(data->stack_b));
    parcMemory_Deallocate((void **) &data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * Destroy the table before destroying the connection
 */
LONGBOW_TEST_CASE(Global, rtaConnectionTable_AddConnection)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    RtaConnection *conn = createConnection(data->stack_a, 2, 3);

    // This is the part we want to test.
    RtaConnectionTable *table = rtaConnectionTable_Create(1000, rtaConnection_Destroy);
    rtaConnectionTable_AddConnection(table, conn);

    assertTrue(table->count_elements == 1, "Incorrect table size, expected %d got %zu", 1, table->count_elements);
    rtaConnectionTable_Destroy(&table);
}


/**
 * Create a connection table with just 1 connection and make sure table
 * does the right thing on overflow
 */
LONGBOW_TEST_CASE(Global, rtaConnectionTable_AddConnection_TooMany)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    RtaConnection *conn = createConnection(data->stack_a, 2, 3);

    int res;

    // create the table with size 1
    RtaConnectionTable *table = rtaConnectionTable_Create(1, rtaConnection_Destroy);
    res = rtaConnectionTable_AddConnection(table, conn);
    assertTrue(res == 0, "Got non-zero return %d", res);
    assertTrue(table->count_elements == 1, "Incorrect table size, expected %d got %zu", 1, table->count_elements);

    // add the second connection, should return failure
    res = rtaConnectionTable_AddConnection(table, conn);
    assertTrue(res == -1, "Should have failed, expecting -1, got %d", res);

    rtaConnectionTable_Destroy(&table);
}


LONGBOW_TEST_CASE(Global, rtaConnectionTable_Create_Destroy)
{
    size_t beforeBalance = parcMemory_Outstanding();
    RtaConnectionTable *table = rtaConnectionTable_Create(1000, rtaConnection_Destroy);
    assertTrue(table->max_elements == 1000, "Initialized with wrong number of elements");
    rtaConnectionTable_Destroy(&table);
    size_t afterBalance = parcMemory_Outstanding();
    assertTrue(beforeBalance == afterBalance, "Memory imbalance after create/destroy");
}

LONGBOW_TEST_CASE(Global, rtaConnectionTable_GetByApiFd)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    RtaConnection *conn = createConnection(data->stack_a, 2, 3);

    RtaConnectionTable *table = rtaConnectionTable_Create(1000, rtaConnection_Destroy);
    rtaConnectionTable_AddConnection(table, conn);

    RtaConnection *test;
    test = rtaConnectionTable_GetByApiFd(table, 2);
    assertTrue(test == conn, "Got wrong connection, expecting %p got %p", (void *) conn, (void *) test);

    test = rtaConnectionTable_GetByApiFd(table, 3);
    assertTrue(test == NULL, "Got wrong connection, expecting %p got %p", NULL, (void *) test);

    test = rtaConnectionTable_GetByApiFd(table, 4);
    assertTrue(test == NULL, "Got wrong connection, expecting %p got %p", NULL, (void *) test);

    rtaConnectionTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, rtaConnectionTable_GetByTransportFd)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    RtaConnection *conn = createConnection(data->stack_a, 2, 3);

    RtaConnectionTable *table = rtaConnectionTable_Create(1000, rtaConnection_Destroy);
    rtaConnectionTable_AddConnection(table, conn);

    RtaConnection *test;
    test = rtaConnectionTable_GetByTransportFd(table, 2);
    assertTrue(test == NULL, "Got wrong connection, expecting %p got %p", NULL, (void *) test);

    test = rtaConnectionTable_GetByTransportFd(table, 3);
    assertTrue(test == conn, "Got wrong connection, expecting %p got %p", (void *) conn, (void *) test);

    test = rtaConnectionTable_GetByTransportFd(table, 4);
    assertTrue(test == NULL, "Got wrong connection, expecting %p got %p", NULL, (void *) test);


    rtaConnectionTable_Destroy(&table);
}

/**
 * We create two connections and make sure that when we remove one the other
 * is still in the table
 */
LONGBOW_TEST_CASE(Global, rtaConnectionTable_Remove)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int res;
    int a_pair[2];
    int b_pair[2];

    // we have to use actual socket pairs in this test because Remove will destroy
    // the last copy of the connection and call close() on the sockets.
    socketpair(PF_LOCAL, SOCK_STREAM, 0, a_pair);
    socketpair(PF_LOCAL, SOCK_STREAM, 0, b_pair);

    RtaConnectionTable *table = rtaConnectionTable_Create(1000, rtaConnection_Destroy);

    RtaConnection *conn_a = createConnection(data->stack_a, a_pair[0], a_pair[1]);
    rtaConnectionTable_AddConnection(table, conn_a);

    RtaConnection *conn_b = createConnection(data->stack_b, b_pair[0], b_pair[1]);
    rtaConnectionTable_AddConnection(table, conn_b);

    assertTrue(table->count_elements == 2, "Wrong element count");

    res = rtaConnectionTable_Remove(table, conn_b);
    assertTrue(res == 0, "Got error from rtaConnectionTable_Remove: %d", res);
    assertTrue(table->count_elements == 1, "Wrong element count");

    RtaConnection *test = rtaConnectionTable_GetByApiFd(table, a_pair[0]);
    assertNotNull(test, "Could not retrieve connection that was supposed to still be there");

    rtaConnectionTable_Destroy(&table);
}

/**
 * Create two connections, they are in different protocol stacks.  Remove one by
 * stack id and make sure the other is still in the table
 */
LONGBOW_TEST_CASE(Global, rtaConnectionTable_RemoveByStack)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int res;
    int a_pair[2];
    int b_pair[2];

    // we have to use actual socket pairs in this test because Remove will destroy
    // the last copy of the connection and call close() on the sockets.
    socketpair(PF_LOCAL, SOCK_STREAM, 0, a_pair);
    socketpair(PF_LOCAL, SOCK_STREAM, 0, b_pair);

    RtaConnectionTable *table = rtaConnectionTable_Create(1000, rtaConnection_Destroy);

    RtaConnection *conn_a = createConnection(data->stack_a, a_pair[0], a_pair[1]);
    rtaConnectionTable_AddConnection(table, conn_a);

    RtaConnection *conn_b = createConnection(data->stack_b, b_pair[0], b_pair[1]);
    rtaConnectionTable_AddConnection(table, conn_b);

    // now remove a connection by stack id

    res = rtaConnectionTable_RemoveByStack(table, data->stack_a->stack_id);
    assertTrue(res == 0, "Got error from rtaConnectionTable_RemoveByStack: %d", res);
    assertTrue(table->count_elements == 1, "Wrong element count");

    RtaConnection *test = rtaConnectionTable_GetByApiFd(table, b_pair[0]);
    assertNotNull(test, "Could not retrieve connection that was supposed to still be there");

    rtaConnectionTable_Destroy(&table);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_ConnectionTable);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
