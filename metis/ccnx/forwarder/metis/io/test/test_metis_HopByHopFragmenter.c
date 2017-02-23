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

#include "../metis_HopByHopFragmenter.c"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <LongBow/unit-test.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

typedef struct test_data {
    unsigned mtu;
    MetisLogger *logger;
    MetisHopByHopFragmenter *fragmenter;
} TestData;

static TestData *
_createTestData(void)
{
    TestData *data = parcMemory_Allocate(sizeof(TestData));

    data->mtu = 2000;

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    data->logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(data->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    data->fragmenter = metisHopByHopFragmenter_Create(data->logger, data->mtu);
    return data;
}

static void
_destroyTestData(TestData **dataPtr)
{
    TestData *data = *dataPtr;
    metisHopByHopFragmenter_Release(&data->fragmenter);
    metisLogger_Release(&data->logger);
    parcMemory_Deallocate((void **) dataPtr);
}

/*
 * Create a well-formed packet with the given length.
 * length is the total packet length, including fixed header.
 */
static uint8_t *
_conjurePacket(size_t length)
{
    _HopByHopHeader header;
    memset(&header, 0, sizeof(header));

    size_t payloadLength = length - sizeof(header);

    header.version = 1;
    header.packetType = 2; // interest return -- does not require a name.
    header.packetLength = htons(length);
    header.headerLength = 8;
    header.tlvType = 0;
    header.tlvLength = htons(payloadLength);

    uint8_t *packet = parcMemory_Allocate(length);
    memcpy(packet, &header, sizeof(header));
    return packet;
}

// ============================================================

LONGBOW_TEST_RUNNER(metis_HopByHopFragmenter)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_HopByHopFragmenter)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_HopByHopFragmenter)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisHopByHopFragmenter_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisHopByHopFragmenter_Receive_NotHopByHop);
    LONGBOW_RUN_TEST_CASE(Global, metisHopByHopFragmenter_Receive_ReceiveQueueFull);
    LONGBOW_RUN_TEST_CASE(Global, metisHopByHopFragmenter_Receive_Ok);

    LONGBOW_RUN_TEST_CASE(Global, metisHopByHopFragmenter_Send_OneMtu);
    LONGBOW_RUN_TEST_CASE(Global, metisHopByHopFragmenter_Send_ReceiveQueueFull);
    LONGBOW_RUN_TEST_CASE(Global, metisHopByHopFragmenter_Send_Ok);
}


LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = _createTestData();
    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _destroyTestData(&data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisHopByHopFragmenter_Create)
{
    // nothing really to do here, just need to make sure there's no memory leak in teardown
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    assertNotNull(data->fragmenter, "Got null fragmenter");
}

/*
 * Receive a non-hop-by-hop packet.  Should go straight in to the receive queue
 */
LONGBOW_TEST_CASE(Global, metisHopByHopFragmenter_Receive_NotHopByHop)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    MetisTicks startTicks = 1111111;
    unsigned ingressId = 77;

    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV1_Interest_AllFields, sizeof(metisTestDataV1_Interest_AllFields), ingressId, startTicks, data->logger);
    metisHopByHopFragmenter_Receive(data->fragmenter, message);

    /*
     * 1) Make a metis message out of the reassembly buffer,
     * 2) put the message in the receive queue (discard if queue full)
     * 3) allocate a new reassembly buffer
     * 4) reset the parser
     */

    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Got null reassembled message");

    assertTrue(test == message, "Message not in receive queue, expected %p got %p", (void *) message, (void *) test);

    metisMessage_Release(&message);
    metisMessage_Release(&test);
}

LONGBOW_TEST_CASE(Global, metisHopByHopFragmenter_Receive_ReceiveQueueFull)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // create a full recieve queue
    parcRingBuffer1x1_Release(&data->fragmenter->receiveQueue);
    data->fragmenter->receiveQueue = parcRingBuffer1x1_Create(2, _ringBufferDestroyer);

    void *fakeData = (void *) 1;
    parcRingBuffer1x1_Put(data->fragmenter->receiveQueue, fakeData);

    assertTrue(parcRingBuffer1x1_Remaining(data->fragmenter->receiveQueue) == 0, "expected queue to be full");

    // === run test
    MetisTicks startTicks = 1111111;
    unsigned ingressId = 77;

    data->fragmenter->nextReceiveFragSequenceNumber = 1;

    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV1_Interest_AllFields, sizeof(metisTestDataV1_Interest_AllFields), ingressId, startTicks, data->logger);
    metisHopByHopFragmenter_Receive(data->fragmenter, message);

    // should still only be the fake data in the queue
    void *test = NULL;
    parcRingBuffer1x1_Get(data->fragmenter->receiveQueue, &test);
    assertTrue(test == fakeData, "Wrong pointer, expected %p got %p", fakeData, test);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisHopByHopFragmenter_Receive_Ok)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    MetisTicks startTicks = 1111111;
    unsigned ingressId = 77;

    data->fragmenter->nextReceiveFragSequenceNumber = 1;

    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Begin, sizeof(metisTestDataV1_HopByHopFrag_Begin), ingressId, startTicks, data->logger);
    metisHopByHopFragmenter_Receive(data->fragmenter, message);

    /*
     * We should now be in the Busy state
     */
    assertTrue(data->fragmenter->parserState == _ParserState_Busy, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisHopByHopFragmenter_Send_OneMtu)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // make a packet shorter than one MTU (so it will fit with the fragment overhead)
    size_t length = data->fragmenter->mtu - 100;
    uint8_t *packet = _conjurePacket(length);
    MetisMessage *message = metisMessage_CreateFromArray(packet, length, 1, 2, data->logger);
    assertNotNull(message, "Could not conjure packet");

    bool success = metisHopByHopFragmenter_Send(data->fragmenter, message);

    assertTrue(success, "Failed to send fragments");
    MetisMessage *fragment = metisHopByHopFragmenter_PopSendQueue(data->fragmenter);
    assertNotNull(fragment, "Did not find a fragment in the send queue");

    // ===
    // defragment it

    metisHopByHopFragmenter_Receive(data->fragmenter, fragment);
    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Should have gotten the original message back");
    assertTrue(metisMessage_Length(test) == metisMessage_Length(message),
               "Reconstructed message length wrong expected %zu got %zu",
               metisMessage_Length(message),
               metisMessage_Length(test));

    // ===
    // cleanup

    metisMessage_Release(&fragment);
    metisMessage_Release(&message);
    metisMessage_Release(&test);
    parcMemory_Deallocate((void **) &packet);
}

LONGBOW_TEST_CASE(Global, metisHopByHopFragmenter_Send_ReceiveQueueFull)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // create a full send queue
    parcRingBuffer1x1_Release(&data->fragmenter->sendQueue);
    data->fragmenter->sendQueue = parcRingBuffer1x1_Create(2, _ringBufferDestroyer);

    void *fakeData = (void *) 1;
    parcRingBuffer1x1_Put(data->fragmenter->sendQueue, fakeData);


    // less than 1 MTU
    size_t length = data->fragmenter->mtu - 100;
    uint8_t *packet = _conjurePacket(length);
    MetisMessage *message = metisMessage_CreateFromArray(packet, length, 1, 2, data->logger);
    assertNotNull(message, "Could not conjure packet");


    bool success = metisHopByHopFragmenter_Send(data->fragmenter, message);
    assertFalse(success, "Should have failed to send fragments");

    // ===
    // cleanup

    // manually pop this off as it is not a proper MetisMessage
    parcRingBuffer1x1_Get(data->fragmenter->sendQueue, &fakeData);
    metisMessage_Release(&message);
    parcMemory_Deallocate((void **) &packet);
}

LONGBOW_TEST_CASE(Global, metisHopByHopFragmenter_Send_Ok)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // Take up 2 MTUs (minus a little for fragmentation overhead)
    size_t length = 2 * data->fragmenter->mtu - 100;
    uint8_t *packet = _conjurePacket(length);
    MetisMessage *message = metisMessage_CreateFromArray(packet, length, 1, 2, data->logger);
    assertNotNull(message, "Could not conjure packet");


    bool success = metisHopByHopFragmenter_Send(data->fragmenter, message);
    assertTrue(success, "Failed to send fragments");

    // ===
    // defragment it

    MetisMessage *fragment;
    while ((fragment = metisHopByHopFragmenter_PopSendQueue(data->fragmenter)) != NULL) {
        metisHopByHopFragmenter_Receive(data->fragmenter, fragment);
        metisMessage_Release(&fragment);
    }
    ;


    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Should have gotten the original message back");
    assertTrue(metisMessage_Length(test) == metisMessage_Length(message),
               "Reconstructed message length wrong expected %zu got %zu",
               metisMessage_Length(message),
               metisMessage_Length(test));

    // ===
    // cleanup

    metisMessage_Release(&message);
    metisMessage_Release(&test);
    parcMemory_Deallocate((void **) &packet);
}

// ============================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _compareSequenceNumbers);
    LONGBOW_RUN_TEST_CASE(Local, _incrementSequenceNumber);
    LONGBOW_RUN_TEST_CASE(Local, _resetParser);
    LONGBOW_RUN_TEST_CASE(Local, _applySequenceNumberRules_InOrder);
    LONGBOW_RUN_TEST_CASE(Local, _applySequenceNumberRules_Early);
    LONGBOW_RUN_TEST_CASE(Local, _applySequenceNumberRules_Late);
    LONGBOW_RUN_TEST_CASE(Local, _finalizeReassemblyBuffer_NotFull);
    LONGBOW_RUN_TEST_CASE(Local, _finalizeReassemblyBuffer_Full);
    LONGBOW_RUN_TEST_CASE(Local, _appendFragmentToReassemblyBuffer_Once);
    LONGBOW_RUN_TEST_CASE(Local, _appendFragmentToReassemblyBuffer_Multiple);

    LONGBOW_RUN_TEST_CASE(Local, _receiveInIdleState_BFrame);
    LONGBOW_RUN_TEST_CASE(Local, _receiveInIdleState_BEFrame);
    LONGBOW_RUN_TEST_CASE(Local, _receiveInIdleState_OtherFrame);

    LONGBOW_RUN_TEST_CASE(Local, _receiveInBusyState_EFrame);
    LONGBOW_RUN_TEST_CASE(Local, _receiveInBusyState_NoFlagFrame);
    LONGBOW_RUN_TEST_CASE(Local, _receiveInBusyState_OtherFrame);

    LONGBOW_RUN_TEST_CASE(Local, _receiveFragment_IdleState);
    LONGBOW_RUN_TEST_CASE(Local, _receiveFragment_BusyState);

    LONGBOW_RUN_TEST_CASE(Local, _sendFragments_OneFragment);
    LONGBOW_RUN_TEST_CASE(Local, _sendFragments_TwoFragments);
    LONGBOW_RUN_TEST_CASE(Local, _sendFragments_ThreeFragments);
    LONGBOW_RUN_TEST_CASE(Local, _sendFragments_SendQueueFull);

    LONGBOW_RUN_TEST_CASE(Local, _ringBufferDestroyer);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    TestData *data = _createTestData();
    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _destroyTestData(&data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _compareSequenceNumbers)
{
    struct test_vector {
        uint32_t a;
        uint32_t b;
        int signum;
        bool sentinel;
    } testVectors[] = {
        // compared to b = 0, then a = {1 ... 0x07FFFF} is greater than b
        // compared to b = 0, then a = {0x080000 ... 0x0FFFFF} is less than b

        { .a = 0x00000000, .b = 0x00000000, .signum = 0,  .sentinel = false },
        { .a = 0x00000001, .b = 0x00000000, .signum = +1, .sentinel = false },
        { .a = 0x0007FFFF, .b = 0x00000000, .signum = +1, .sentinel = false },
        { .a = 0x00080000, .b = 0x00000000, .signum = -1, .sentinel = false },
        { .a = 0x000FFFFF, .b = 0x00000000, .signum = -1, .sentinel = false },

        // now do the same thing but use b = 0x00040000
        { .a = 0x00040000, .b = 0x00040000, .signum = 0,  .sentinel = false },
        { .a = 0x00040001, .b = 0x00040000, .signum = +1, .sentinel = false },
        { .a = 0x000BFFFF, .b = 0x00040000, .signum = +1, .sentinel = false },
        { .a = 0x000C0000, .b = 0x00040000, .signum = -1, .sentinel = false },
        { .a = 0x0003FFFF, .b = 0x00040000, .signum = -1, .sentinel = false },

        // end test set
        { .a = 0x00000000, .b = 0x00000000, .signum = 0,  .sentinel = true  },
    };

    for (int i = 0; !testVectors[i].sentinel; i++) {
        int result = _compareSequenceNumbers(testVectors[i].a, testVectors[i].b);

        if (testVectors[i].signum == 0) {
            assertTrue(result == 0, "Wrong result, expected 0 got %d index %d a 0x%08x b 0x%08x",
                       result, i, testVectors[i].a, testVectors[i].b);
        }

        if (testVectors[i].signum < 0) {
            assertTrue(result < 0, "Wrong result, expected negative got %d index %d a 0x%08x b 0x%08x",
                       result, i, testVectors[i].a, testVectors[i].b);
        }

        if (testVectors[i].signum > 0) {
            assertTrue(result > 0, "Wrong result, expected positive got %d index %d a 0x%08x b 0x%08x",
                       result, i, testVectors[i].a, testVectors[i].b);
        }
    }
}

LONGBOW_TEST_CASE(Local, _incrementSequenceNumber)
{
    struct test_vector {
        uint32_t a;
        uint32_t b;
        bool sentinel;
    } testVectors[] = {
        { .a = 0x00000000, .b = 0x00000001, .sentinel = false },
        { .a = 0x00000001, .b = 0x00000002, .sentinel = false },
        { .a = 0x0007FFFF, .b = 0x00080000, .sentinel = false },
        { .a = 0x000FFFFF, .b = 0x00000000, .sentinel = false },
        // end test set
        { .a = 0x00000000, .b = 0x00000000, .sentinel = true  },
    };

    for (int i = 0; !testVectors[i].sentinel; i++) {
        uint32_t result = _incrementSequenceNumber(testVectors[i].a, 0x000FFFFF);

        assertTrue(result == testVectors[i].b, "Wrong result 0x%08X index %d, got for a 0x%08x b 0x%08x",
                   result, i, testVectors[i].a, testVectors[i].b);
    }
}

LONGBOW_TEST_CASE(Local, _resetParser)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // put something in the buffer and set the parser state to Busy
    data->fragmenter->parserState = _ParserState_Busy;
    parcEventBuffer_Append(data->fragmenter->currentReceiveBuffer, data, sizeof(data));

    _resetParser(data->fragmenter);
    assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Idle, data->fragmenter->parserState);

    size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
    assertTrue(length == 0, "Wrong length, expected 0 bytes, got %zu", length);
}

LONGBOW_TEST_CASE(Local, _applySequenceNumberRules_InOrder)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->fragmenter->parserState = _ParserState_Busy;
    data->fragmenter->nextReceiveFragSequenceNumber = 1000;

    _HopByHopHeader header;
    memset(&header, 0, sizeof(header));

    _hopByHopHeader_SetSeqnum(&header, 1000);

    _applySequenceNumberRules(data->fragmenter, &header);

    // should still be in Busy mode and expecting 1001
    assertTrue(data->fragmenter->parserState == _ParserState_Busy, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    assertTrue(data->fragmenter->nextReceiveFragSequenceNumber == 1001, "Wrong next seqnum, expected 1001 got %u", data->fragmenter->nextReceiveFragSequenceNumber);
}

LONGBOW_TEST_CASE(Local, _applySequenceNumberRules_Early)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->fragmenter->parserState = _ParserState_Busy;
    data->fragmenter->nextReceiveFragSequenceNumber = 1000;

    _HopByHopHeader header;
    memset(&header, 0, sizeof(header));

    _hopByHopHeader_SetSeqnum(&header, 998);

    _applySequenceNumberRules(data->fragmenter, &header);

    // should reset state and set next to 999
    assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    assertTrue(data->fragmenter->nextReceiveFragSequenceNumber == 999, "Wrong next seqnum, expected 999 got %u", data->fragmenter->nextReceiveFragSequenceNumber);
}

LONGBOW_TEST_CASE(Local, _applySequenceNumberRules_Late)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->fragmenter->parserState = _ParserState_Busy;
    data->fragmenter->nextReceiveFragSequenceNumber = 1000;

    _HopByHopHeader header;
    memset(&header, 0, sizeof(header));

    _hopByHopHeader_SetSeqnum(&header, 1001);

    _applySequenceNumberRules(data->fragmenter, &header);

    // should reset state and set next to 1002
    assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    assertTrue(data->fragmenter->nextReceiveFragSequenceNumber == 1002, "Wrong next seqnum, expected 1002 got %u", data->fragmenter->nextReceiveFragSequenceNumber);
}

LONGBOW_TEST_CASE(Local, _finalizeReassemblyBuffer_NotFull)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    MetisTicks startTicks = 1111111;
    unsigned ingressId = 77;

    // set up as just finished with a message, so the currentReceiveBuffer has
    // a complete CCNx message array in it
    data->fragmenter->parserState = _ParserState_Busy;
    data->fragmenter->currentReceiveBufferIngressId = ingressId;
    data->fragmenter->currentReceiveBufferStartTicks = startTicks;
    parcEventBuffer_Append(data->fragmenter->currentReceiveBuffer, metisTestDataV1_Interest_AllFields, sizeof(metisTestDataV1_Interest_AllFields));

    _finalizeReassemblyBuffer(data->fragmenter);

    /*
     * 1) Make a metis message out of the reassembly buffer,
     * 2) put the message in the receive queue (discard if queue full)
     * 3) allocate a new reassembly buffer
     * 4) reset the parser
     */

    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Got null reassembled message");
    assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    assertNotNull(data->fragmenter->currentReceiveBuffer, "Current receive buffer should not be null");
    assertTrue(parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer) == 0, "Current receive buffer should be empty, got %zu bytes", parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer));

    assertTrue(metisMessage_GetIngressConnectionId(test) == ingressId, "Wrong ingress id expected %u got %u", ingressId, metisMessage_GetIngressConnectionId(test));
    assertTrue(metisMessage_GetReceiveTime(test) == startTicks, "Wrong receive time expected %" PRIu64 " got %" PRIu64,
               startTicks, metisMessage_GetReceiveTime(test));

    metisMessage_Release(&test);
}

LONGBOW_TEST_CASE(Local, _finalizeReassemblyBuffer_Full)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    MetisTicks startTicks = 1111111;
    unsigned ingressId = 77;

    // set up as just finished with a message, so the currentReceiveBuffer has
    // a complete CCNx message array in it
    data->fragmenter->parserState = _ParserState_Busy;
    data->fragmenter->currentReceiveBufferIngressId = ingressId;
    data->fragmenter->currentReceiveBufferStartTicks = startTicks;
    parcEventBuffer_Append(data->fragmenter->currentReceiveBuffer, metisTestDataV1_Interest_AllFields, sizeof(metisTestDataV1_Interest_AllFields));

    // create a full recieve queue
    parcRingBuffer1x1_Release(&data->fragmenter->receiveQueue);
    data->fragmenter->receiveQueue = parcRingBuffer1x1_Create(2, _ringBufferDestroyer);

    void *fakeData = (void *) 1;
    parcRingBuffer1x1_Put(data->fragmenter->receiveQueue, fakeData);

    assertTrue(parcRingBuffer1x1_Remaining(data->fragmenter->receiveQueue) == 0, "expected queue to be full");

    /*
     * Call with a full receive queue
     */
    _finalizeReassemblyBuffer(data->fragmenter);

    void *test = NULL;
    parcRingBuffer1x1_Get(data->fragmenter->receiveQueue, &test);
    assertTrue(test == fakeData, "Wrong pointer, expected %p got %p", fakeData, test);

    // teardown should show no memory leak
}

LONGBOW_TEST_CASE(Local, _appendFragmentToReassemblyBuffer_Once)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    unsigned connid = 7;
    MetisTicks receiveTime = 9999;

    MetisMessage *fragment = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Begin, sizeof(metisTestDataV1_HopByHopFrag_Begin), connid, receiveTime, data->logger);
    _appendFragmentToReassemblyBuffer(data->fragmenter, fragment);

    int fragmentLength = sizeof(metisTestDataV1_HopByHopFrag_Begin_Fragment);

    assertTrue(parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer) == fragmentLength,
               "currentReceiveBuffer wrong lenth, expected %d got %zu",
               fragmentLength,
               parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer));

    uint8_t *test = parcEventBuffer_Pullup(data->fragmenter->currentReceiveBuffer, -1);
    assertTrue(memcmp(test, metisTestDataV1_HopByHopFrag_Begin_Fragment, fragmentLength) == 0, "Fragment payload did not match");

    metisMessage_Release(&fragment);
}

LONGBOW_TEST_CASE(Local, _appendFragmentToReassemblyBuffer_Multiple)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    unsigned connid = 7;
    MetisTicks receiveTime = 9999;

    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Begin, sizeof(metisTestDataV1_HopByHopFrag_Begin), connid, receiveTime, data->logger);
    _appendFragmentToReassemblyBuffer(data->fragmenter, fragment1);

    MetisMessage *fragment2 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Middle, sizeof(metisTestDataV1_HopByHopFrag_Middle), connid, receiveTime, data->logger);
    _appendFragmentToReassemblyBuffer(data->fragmenter, fragment2);

    MetisMessage *fragment3 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_End, sizeof(metisTestDataV1_HopByHopFrag_End), connid, receiveTime, data->logger);
    _appendFragmentToReassemblyBuffer(data->fragmenter, fragment3);

    int fragmentLength = sizeof(metisTestDataV1_HopByHopFrag_BeginEnd_Fragment);

    assertTrue(parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer) == fragmentLength,
               "currentReceiveBuffer wrong lenth, expected %d got %zu",
               fragmentLength,
               parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer));

    uint8_t *test = parcEventBuffer_Pullup(data->fragmenter->currentReceiveBuffer, -1);

    // compares against the fragment metisTestDataV1_HopByHopFrag_BeginEnd which has the whole payload
    assertTrue(memcmp(test, metisTestDataV1_HopByHopFrag_BeginEnd_Fragment, fragmentLength) == 0, "Fragment payload did not match");

    metisMessage_Release(&fragment1);
    metisMessage_Release(&fragment2);
    metisMessage_Release(&fragment3);
}


/*
 * B frame should be added to currentReceiveBuffer and state should become Busy.
 * Also, the currentReceiveBufferIngressId and currentReceiveBufferReceiveTime should be set.
 */
LONGBOW_TEST_CASE(Local, _receiveInIdleState_BFrame)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // ensure we're in Idle state
    _resetParser(data->fragmenter);

    unsigned connid = 7;
    MetisTicks receiveTime = 9999;
    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Begin, sizeof(metisTestDataV1_HopByHopFrag_Begin), connid, receiveTime, data->logger);

    const _HopByHopHeader *header = (const _HopByHopHeader *) metisTestDataV1_HopByHopFrag_Begin;
    _receiveInIdleState(data->fragmenter, fragment1, header);

    size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
    assertTrue(length == sizeof(metisTestDataV1_HopByHopFrag_Begin_Fragment), "Wrong reassembly buffer length expected %zu got %zu", sizeof(metisTestDataV1_HopByHopFrag_Begin_Fragment), length);
    assertTrue(data->fragmenter->parserState == _ParserState_Busy, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    assertTrue(data->fragmenter->currentReceiveBufferIngressId == connid, "Wrong ingress id expected %u got %u", connid, data->fragmenter->currentReceiveBufferIngressId);
    assertTrue(data->fragmenter->currentReceiveBufferStartTicks == receiveTime, "Wrong receive time expected %" PRIu64 " got %" PRIu64,
               receiveTime, data->fragmenter->currentReceiveBufferStartTicks);
    metisMessage_Release(&fragment1);
}

/*
 * BE frame should be added to currentReceiveBuffer and finalized.
 * State should stay in Idle but the receiveQueue should have the frame in it.
 */
LONGBOW_TEST_CASE(Local, _receiveInIdleState_BEFrame)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // ensure we're in Idle state
    _resetParser(data->fragmenter);

    unsigned connid = 7;
    MetisTicks receiveTime = 9999;
    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_BeginEnd, sizeof(metisTestDataV1_HopByHopFrag_BeginEnd), connid, receiveTime, data->logger);

    const _HopByHopHeader *header = (const _HopByHopHeader *) metisTestDataV1_HopByHopFrag_BeginEnd;
    _receiveInIdleState(data->fragmenter, fragment1, header);

    // should not be in the reassembly buffer
    size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
    assertTrue(length == 0, "Wrong reassembly buffer length expected 0 got %zu", length);

    // it should be in the receive queue
    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Message was not in receive queue");
    metisMessage_Release(&test);

    assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Idle, data->fragmenter->parserState);


    assertTrue(data->fragmenter->currentReceiveBufferIngressId == connid, "Wrong ingress id expected %u got %u", connid, data->fragmenter->currentReceiveBufferIngressId);
    assertTrue(data->fragmenter->currentReceiveBufferStartTicks == receiveTime, "Wrong receive time expected %" PRIu64 " got %" PRIu64,
               receiveTime, data->fragmenter->currentReceiveBufferStartTicks);
    metisMessage_Release(&fragment1);
}

/*
 * Not B and Not BE frames should be ignored
 */
LONGBOW_TEST_CASE(Local, _receiveInIdleState_OtherFrame)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    struct test_vector {
        uint8_t flags;
        bool sentinel;
    } testVectors[] = {
        // All combinations except 0x40 and 0x60
        { .flags = 0x00, .sentinel = false },
        { .flags = 0x10, .sentinel = false },
        { .flags = 0x20, .sentinel = false },
        { .flags = 0x30, .sentinel = false },
        { .flags = 0x80, .sentinel = false },
        { .flags = 0x90, .sentinel = false },
        { .flags = 0xA0, .sentinel = false },
        { .flags = 0xB0, .sentinel = false },
        { .flags = 0x00, .sentinel = true  },
    };

    for (int i = 0; !testVectors[i].sentinel; i++) {
        _HopByHopHeader header;
        memset(&header, 0, sizeof(header));

        header.blob[0] |= testVectors[i].flags;

        unsigned connid = 7;
        MetisTicks receiveTime = 9999;
        MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_BeginEnd, sizeof(metisTestDataV1_HopByHopFrag_BeginEnd), connid, receiveTime, data->logger);

        _receiveInIdleState(data->fragmenter, fragment1, &header);

        metisMessage_Release(&fragment1);

        // should not be in the reassembly buffer
        size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
        assertTrue(length == 0, "Wrong reassembly buffer length expected 0 got %zu", length);

        assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Idle, data->fragmenter->parserState);
    }
}

/*
 * 2) If E flag
 * 2a) append to reassembly buffer
 * 2b) finalize the buffer (side effect: will reset the parser and place in receive queue)
 */
LONGBOW_TEST_CASE(Local, _receiveInBusyState_EFrame)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    unsigned connid = 7;
    MetisTicks receiveTime = 9999;

    // ensure we're in Busy state (the precondition of this test)
    _resetParser(data->fragmenter);
    data->fragmenter->parserState = _ParserState_Busy;

    // and put the Begin and Middle fragments in the reassembly buffer so the packet will decode properly
    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Begin, sizeof(metisTestDataV1_HopByHopFrag_Begin), connid, receiveTime, data->logger);
    _appendFragmentToReassemblyBuffer(data->fragmenter, fragment1);

    MetisMessage *fragment2 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Middle, sizeof(metisTestDataV1_HopByHopFrag_Middle), connid, receiveTime, data->logger);
    _appendFragmentToReassemblyBuffer(data->fragmenter, fragment2);

    // ====
    // Now do the test

    MetisMessage *fragment3 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_End, sizeof(metisTestDataV1_HopByHopFrag_End), connid, receiveTime, data->logger);

    const _HopByHopHeader *header = (const _HopByHopHeader *) metisTestDataV1_HopByHopFrag_End;
    _receiveInBusyState(data->fragmenter, fragment3, header);

    size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
    assertTrue(length == 0, "Wrong reassembly buffer length expected 0 got %zu", length);

    assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Idle, data->fragmenter->parserState);

    // it should be in the receive queue
    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Message was not in receive queue");
    metisMessage_Release(&test);

    metisMessage_Release(&fragment1);
    metisMessage_Release(&fragment2);
    metisMessage_Release(&fragment3);
}

/*
 * 1) If no flags
 * 1a) append to reassembly buffer
 */
LONGBOW_TEST_CASE(Local, _receiveInBusyState_NoFlagFrame)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // ensure we're in Busy state (the precondition of this test)
    _resetParser(data->fragmenter);
    data->fragmenter->parserState = _ParserState_Busy;

    unsigned connid = 7;
    MetisTicks receiveTime = 9999;
    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Middle, sizeof(metisTestDataV1_HopByHopFrag_Middle), connid, receiveTime, data->logger);

    const _HopByHopHeader *header = (const _HopByHopHeader *) metisTestDataV1_HopByHopFrag_Middle;
    _receiveInBusyState(data->fragmenter, fragment1, header);

    size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
    assertTrue(length == sizeof(metisTestDataV1_HopByHopFrag_Middle_Fragment), "Wrong reassembly buffer length expected %zu got %zu", sizeof(metisTestDataV1_HopByHopFrag_Middle_Fragment), length);

    assertTrue(data->fragmenter->parserState == _ParserState_Busy, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    metisMessage_Release(&fragment1);
}

LONGBOW_TEST_CASE(Local, _receiveInBusyState_OtherFrame)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    struct test_vector {
        uint8_t flags;
        bool sentinel;
    } testVectors[] = {
        // All combinations except 0x00 and 0x20
        { .flags = 0x10, .sentinel = false },
        { .flags = 0x40, .sentinel = false },
        { .flags = 0x80, .sentinel = false },
        { .flags = 0x50, .sentinel = false },
        { .flags = 0x90, .sentinel = false },
        { .flags = 0xC0, .sentinel = false },
        { .flags = 0x00, .sentinel = true  },
    };

    for (int i = 0; !testVectors[i].sentinel; i++) {
        _HopByHopHeader header;
        memset(&header, 0, sizeof(header));

        header.blob[0] |= testVectors[i].flags;

        unsigned connid = 7;
        MetisTicks receiveTime = 9999;
        MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_BeginEnd, sizeof(metisTestDataV1_HopByHopFrag_BeginEnd), connid, receiveTime, data->logger);

        // ensure we're in Busy state (the precondition of this test)
        _resetParser(data->fragmenter);
        data->fragmenter->parserState = _ParserState_Busy;

        _receiveInBusyState(data->fragmenter, fragment1, &header);

        metisMessage_Release(&fragment1);

        // should not be in the reassembly buffer
        size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
        assertTrue(length == 0, "Wrong reassembly buffer length expected 0 got %zu", length);

        assertTrue(data->fragmenter->parserState == _ParserState_Idle, "Wrong parser state, exepcted %d got %d", _ParserState_Idle, data->fragmenter->parserState);
    }
}

/*
 * Receive a B frame in Idle state
 */
LONGBOW_TEST_CASE(Local, _receiveFragment_IdleState)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    unsigned connid = 7;
    MetisTicks receiveTime = 9999;
    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Begin, sizeof(metisTestDataV1_HopByHopFrag_Begin), connid, receiveTime, data->logger);

    _receiveFragment(data->fragmenter, fragment1);

    size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
    assertTrue(length == sizeof(metisTestDataV1_HopByHopFrag_Begin_Fragment), "Wrong reassembly buffer length expected %zu got %zu", sizeof(metisTestDataV1_HopByHopFrag_Begin_Fragment), length);
    assertTrue(data->fragmenter->parserState == _ParserState_Busy, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    assertTrue(data->fragmenter->currentReceiveBufferIngressId == connid, "Wrong ingress id expected %u got %u", connid, data->fragmenter->currentReceiveBufferIngressId);
    assertTrue(data->fragmenter->currentReceiveBufferStartTicks == receiveTime, "Wrong receive time expected %" PRIu64 " got %" PRIu64,
               receiveTime, data->fragmenter->currentReceiveBufferStartTicks);
    metisMessage_Release(&fragment1);
}

LONGBOW_TEST_CASE(Local, _receiveFragment_BusyState)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // ensure we're in Busy state (the precondition of this test)
    // Make sure the packet will be in-order by setting the next expected seqnum.
    _resetParser(data->fragmenter);
    data->fragmenter->parserState = _ParserState_Busy;
    data->fragmenter->nextReceiveFragSequenceNumber = 2;

    unsigned connid = 7;
    MetisTicks receiveTime = 9999;
    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Middle, sizeof(metisTestDataV1_HopByHopFrag_Middle), connid, receiveTime, data->logger);

    _receiveFragment(data->fragmenter, fragment1);

    size_t length = parcEventBuffer_GetLength(data->fragmenter->currentReceiveBuffer);
    assertTrue(length == sizeof(metisTestDataV1_HopByHopFrag_Middle_Fragment), "Wrong reassembly buffer length expected %zu got %zu", sizeof(metisTestDataV1_HopByHopFrag_Middle_Fragment), length);

    assertTrue(data->fragmenter->parserState == _ParserState_Busy, "Wrong parser state, exepcted %d got %d", _ParserState_Busy, data->fragmenter->parserState);
    metisMessage_Release(&fragment1);
}

LONGBOW_TEST_CASE(Local, _sendFragments_OneFragment)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // make a packet shorter than one MTU (so it will fit with the fragment overhead)
    size_t length = data->fragmenter->mtu - 100;
    uint8_t *packet = _conjurePacket(length);
    MetisMessage *message = metisMessage_CreateFromArray(packet, length, 1, 2, data->logger);
    assertNotNull(message, "Could not conjure packet");


    bool success = _sendFragments(data->fragmenter, message);
    assertTrue(success, "Failed to send fragments");
    MetisMessage *fragment = metisHopByHopFragmenter_PopSendQueue(data->fragmenter);
    assertNotNull(fragment, "Did not find a fragment in the send queue");

    // ===
    // defragment it

    _receiveFragment(data->fragmenter, fragment);
    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Should have gotten the original message back");
    assertTrue(metisMessage_Length(test) == metisMessage_Length(message),
               "Reconstructed message length wrong expected %zu got %zu",
               metisMessage_Length(message),
               metisMessage_Length(test));

    // ===
    // cleanup

    metisMessage_Release(&message);
    metisMessage_Release(&fragment);
    metisMessage_Release(&test);
    parcMemory_Deallocate((void **) &packet);
}

LONGBOW_TEST_CASE(Local, _sendFragments_TwoFragments)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // Take up 2 MTUs (minus a little for fragmentation overhead)
    size_t length = 2 * data->fragmenter->mtu - 100;
    uint8_t *packet = _conjurePacket(length);
    MetisMessage *message = metisMessage_CreateFromArray(packet, length, 1, 2, data->logger);
    assertNotNull(message, "Could not conjure packet");


    bool success = _sendFragments(data->fragmenter, message);
    assertTrue(success, "Failed to send fragments");

    // ===
    // defragment it

    MetisMessage *fragment;
    while ((fragment = metisHopByHopFragmenter_PopSendQueue(data->fragmenter)) != NULL) {
        _receiveFragment(data->fragmenter, fragment);
        metisMessage_Release(&fragment);
    }
    ;


    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Should have gotten the original message back");
    assertTrue(metisMessage_Length(test) == metisMessage_Length(message),
               "Reconstructed message length wrong expected %zu got %zu",
               metisMessage_Length(message),
               metisMessage_Length(test));

    // ===
    // cleanup

    metisMessage_Release(&message);
    metisMessage_Release(&test);
    parcMemory_Deallocate((void **) &packet);
}

LONGBOW_TEST_CASE(Local, _sendFragments_ThreeFragments)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // Take up 2 MTUs (minus a little for fragmentation overhead)
    size_t length = 3 * data->fragmenter->mtu - 100;
    uint8_t *packet = _conjurePacket(length);
    MetisMessage *message = metisMessage_CreateFromArray(packet, length, 1, 2, data->logger);
    assertNotNull(message, "Could not conjure packet");


    bool success = _sendFragments(data->fragmenter, message);
    assertTrue(success, "Failed to send fragments");

    // ===
    // defragment it

    MetisMessage *fragment;
    while ((fragment = metisHopByHopFragmenter_PopSendQueue(data->fragmenter)) != NULL) {
        _receiveFragment(data->fragmenter, fragment);
        metisMessage_Release(&fragment);
    }
    ;


    MetisMessage *test = metisHopByHopFragmenter_PopReceiveQueue(data->fragmenter);
    assertNotNull(test, "Should have gotten the original message back");
    assertTrue(metisMessage_Length(test) == metisMessage_Length(message),
               "Reconstructed message length wrong expected %zu got %zu",
               metisMessage_Length(message),
               metisMessage_Length(test));

    // ===
    // cleanup

    metisMessage_Release(&message);
    metisMessage_Release(&test);
    parcMemory_Deallocate((void **) &packet);
}

LONGBOW_TEST_CASE(Local, _sendFragments_SendQueueFull)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // create a full send queue
    parcRingBuffer1x1_Release(&data->fragmenter->sendQueue);
    data->fragmenter->sendQueue = parcRingBuffer1x1_Create(2, _ringBufferDestroyer);

    void *fakeData = (void *) 1;
    parcRingBuffer1x1_Put(data->fragmenter->sendQueue, fakeData);


    // Take up 2 MTUs (minus a little for fragmentation overhead)
    size_t length = 3 * data->fragmenter->mtu - 100;
    uint8_t *packet = _conjurePacket(length);
    MetisMessage *message = metisMessage_CreateFromArray(packet, length, 1, 2, data->logger);
    assertNotNull(message, "Could not conjure packet");


    bool success = _sendFragments(data->fragmenter, message);
    assertFalse(success, "Should have failed to send fragments");
    // ===
    // cleanup

    // manually pop this off as it is not a proper MetisMessage
    parcRingBuffer1x1_Get(data->fragmenter->sendQueue, &fakeData);

    metisMessage_Release(&message);
    parcMemory_Deallocate((void **) &packet);
}

LONGBOW_TEST_CASE(Local, _ringBufferDestroyer)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    unsigned connid = 7;
    MetisTicks receiveTime = 9999;
    MetisMessage *fragment1 = metisMessage_CreateFromArray(metisTestDataV1_HopByHopFrag_Middle, sizeof(metisTestDataV1_HopByHopFrag_Middle), connid, receiveTime, data->logger);

    bool success = parcRingBuffer1x1_Put(data->fragmenter->receiveQueue, fragment1);
    assertTrue(success, "Failed to put test message in queue");

    // nothing to do here.  When the fragmenter is destroyed it should destroy the message
    // and we will not trip a memory imbalance
}

// ============================================================
int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_HopByHopFragmenter);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
