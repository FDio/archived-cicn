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


#include "testrig_GenericEther.c"
#include "../metis_EtherConnection.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

typedef struct test_data {
    MetisForwarder *metis;
    MetisGenericEther *ether;
    MetisAddressPair *pair;
    MetisIoOperations *io_ops;
} TestData;

static void
commonSetup(const LongBowTestCase *testCase, uint16_t ethertype)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));

    data->metis = metisForwarder_Create(NULL);
    data->ether = metisGenericEther_Create(data->metis, "foo", ethertype);
    data->io_ops = NULL;

    // crank the libevent handle
    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis), &((struct timeval) {0, 10000}));

    PARCBuffer *localMacBuffer = metisGenericEther_GetMacAddress(data->ether);
    CPIAddress *local = cpiAddress_CreateFromLink(parcBuffer_Overlay(localMacBuffer, 0), parcBuffer_Remaining(localMacBuffer));
    CPIAddress *remote = cpiAddress_CreateFromLink((uint8_t []) { 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }, 6);

    data->pair = metisAddressPair_Create(local, remote);

    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);

    longBowTestCase_SetClipBoardData(testCase, data);
}

static void
commonTeardown(const LongBowTestCase *testCase)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    metisDispatcher_RunDuration(metisForwarder_GetDispatcher(data->metis), &((struct timeval) {0, 10000}));
    metisGenericEther_Release(&data->ether);
    metisAddressPair_Release(&data->pair);

    if (data->io_ops) {
        data->io_ops->destroy(&data->io_ops);
    }

    // destroy metis last
    metisForwarder_Destroy(&data->metis);
    parcMemory_Deallocate((void **) &data);
}


LONGBOW_TEST_RUNNER(metis_EtherConnection)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_EtherConnection)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_EtherConnection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ===========================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisEtherConnection_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    commonSetup(testCase, 0x0801);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    commonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisEtherConnection_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    data->io_ops = metisEtherConnection_Create(data->metis, data->ether, data->pair);
    assertNotNull(data->io_ops, "Got null MetisIoOperations from metisEtherConnection_Create");
}

// ===========================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_DestroyOperations);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_FillInMacAddress);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_GetAddressPair);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_GetConnectionId);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_GetRemoteAddress);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_IsLocal);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_IsUp);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_Send);
    LONGBOW_RUN_TEST_CASE(Local, _setConnectionState);
    LONGBOW_RUN_TEST_CASE(Local, _metisEtherConnection_getConnectionType);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    commonSetup(testCase, 0x0801);

    // for the local tests we also pre-create the EtherConnection
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->io_ops = metisEtherConnection_Create(data->metis, data->ether, data->pair);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    commonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_DestroyOperations)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_FillInMacAddress)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_GetAddressPair)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _MetisEtherState *etherConn = (_MetisEtherState *) metisIoOperations_GetClosure(data->io_ops);

    const MetisAddressPair *test = _metisEtherConnection_GetAddressPair(data->io_ops);

    assertTrue(metisAddressPair_Equals(test, etherConn->addressPair), "Address pair did not compare");
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_GetConnectionId)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _MetisEtherState *etherConn = (_MetisEtherState *) metisIoOperations_GetClosure(data->io_ops);
    unsigned connid = _metisEtherConnection_GetConnectionId(data->io_ops);

    assertTrue(connid == etherConn->id, "Wrong connection id, got %u exepected %u", connid, etherConn->id);
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_GetRemoteAddress)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    const CPIAddress *test = _metisEtherConnection_GetRemoteAddress(data->io_ops);

    _MetisEtherState *etherConn = (_MetisEtherState *) metisIoOperations_GetClosure(data->io_ops);

    assertTrue(cpiAddress_Equals(test, metisAddressPair_GetRemote(etherConn->addressPair)), "Remote addresses did not compare");
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_IsLocal)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool isLocal = _metisEtherConnection_IsLocal(data->io_ops);
    assertFalse(isLocal, "Ethernet should always be remote");
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_IsUp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    bool isUp = _metisEtherConnection_IsUp(data->io_ops);
    assertTrue(isUp, "Ethernet should be up");
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_Send)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, metisForwarder_GetLogger(data->metis));

    bool success = _metisEtherConnection_Send(data->io_ops, NULL, message);
    assertTrue(success, "Failed to write message to ethernet");

    // we should now be able to read the ethernet frame from the test socket
    int testSocket = mockGenericEther_GetTestDescriptor(data->ether);
    assertTrue(testSocket > 0, "Error getting test socket from mock ethernet");

    uint32_t testBufferSize = 2048;
    uint8_t testBuffer[testBufferSize];
    ssize_t bytesRead = read(testSocket, testBuffer, testBufferSize);

    size_t expectedRead = sizeof(struct ether_header) + sizeof(metisTestDataV0_EncodedInterest);

    assertTrue(bytesRead == expectedRead, "Wrong read size, got %zd expected %zu", bytesRead, expectedRead);

    uint8_t *frame = testBuffer + sizeof(struct ether_header);
    assertTrue(memcmp(frame, metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest)) == 0, "Buffers do not match");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Local, _setConnectionState)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _MetisEtherState *etherConn = (_MetisEtherState *) metisIoOperations_GetClosure(data->io_ops);

    _setConnectionState(etherConn, false);
}

LONGBOW_TEST_CASE(Local, _metisEtherConnection_getConnectionType)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CPIConnectionType connType = _metisEtherConnection_getConnectionType(data->io_ops);

    assertTrue(connType == cpiConnection_L2, "Wrong connection type expected %d got %d", cpiConnection_L2, connType);
}

// ===========================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_EtherConnection);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
