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
#include <pthread.h>

#include <arpa/inet.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_EventBuffer.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_EventBuffer.c"

LONGBOW_TEST_RUNNER(parc_EventBuffer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_EventBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_EventBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_GetLength_Append);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_Prepend_Pullup);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_ReadIntoBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_AppendBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_Read);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_WriteToFileDescriptor);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_ReadFromFileDescriptor);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_ReadLine_FreeLine);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventBuffer_GetQueueBuffer);
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

LONGBOW_TEST_CASE(Global, parc_EventBuffer_Create_Destroy)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    assertNotNull(parcEventBuffer, "parcEventBuffer_Create returned a null reference");

    parcEventBuffer_Destroy(&parcEventBuffer);
    parcEventScheduler_Destroy(&parcEventScheduler);
    parcEventBuffer_DisableDebug();
}

static int _dataLength = 8192;

LONGBOW_TEST_CASE(Global, parc_EventBuffer_GetLength_Append)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    assertNotNull(parcEventBuffer, "parcEventBuffer_Create returned a null reference");

    char data[_dataLength];
    parcEventBuffer_Append(parcEventBuffer, data, _dataLength);
    assertTrue(parcEventBuffer_GetLength(parcEventBuffer) == _dataLength, "Buffer length does not match length of appended data");

    parcEventBuffer_Destroy(&parcEventBuffer);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

static uint8_t prependedDataValue = '1';

LONGBOW_TEST_CASE(Global, parc_EventBuffer_Prepend_Pullup)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    assertNotNull(parcEventBuffer, "parcEventBuffer_Create returned a null reference");

    char data[_dataLength];
    data[0] = 2;
    parcEventBuffer_Append(parcEventBuffer, data, _dataLength);
    assertTrue(parcEventBuffer_GetLength(parcEventBuffer) == _dataLength, "Buffer length does not match length of appended data");

    uint8_t prependedData[1];
    prependedData[0] = prependedDataValue;
    parcEventBuffer_Prepend(parcEventBuffer, prependedData, sizeof(uint8_t));
    size_t bufferSize = parcEventBuffer_GetLength(parcEventBuffer);
    assertTrue(bufferSize == (_dataLength + 1), "Buffer length does not match length plus prepended data length");

    uint8_t *completeBuffer = parcEventBuffer_Pullup(parcEventBuffer, -1);
    assertTrue(completeBuffer[0] == prependedDataValue, "Prepended data doesn't match %d != %d", completeBuffer[0], prependedDataValue);
    assertTrue(completeBuffer[1] == 2, "Consolidated data doesn't match %d != %d", completeBuffer[1], 2);

    parcEventBuffer_Destroy(&parcEventBuffer);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventBuffer_ReadIntoBuffer)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBufferSource = parcEventBuffer_Create();
    assertNotNull(parcEventBufferSource, "parcEventBuffer_Create returned a null reference");
    char sourceData[_dataLength];
    parcEventBuffer_Append(parcEventBufferSource, sourceData, _dataLength);

    PARCEventBuffer *parcEventBufferDestination = parcEventBuffer_Create();
    assertNotNull(parcEventBufferDestination, "parcEventBuffer_Create returned a null reference");
    char destinationData[_dataLength];
    parcEventBuffer_Append(parcEventBufferDestination, destinationData, _dataLength);

    parcEventBuffer_ReadIntoBuffer(parcEventBufferSource, parcEventBufferDestination, -1);
    size_t bufferSize = parcEventBuffer_GetLength(parcEventBufferDestination);
    assertTrue(bufferSize == (_dataLength * 2), "Destination buffer size doesn't match expected length");

    parcEventBuffer_Destroy(&parcEventBufferSource);
    parcEventBuffer_Destroy(&parcEventBufferDestination);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventBuffer_AppendBuffer)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBufferSource = parcEventBuffer_Create();
    assertNotNull(parcEventBufferSource, "parcEventBuffer_Create returned a null reference");
    char sourceData[_dataLength];
    parcEventBuffer_Append(parcEventBufferSource, sourceData, _dataLength);

    PARCEventBuffer *parcEventBufferDestination = parcEventBuffer_Create();
    assertNotNull(parcEventBufferDestination, "parcEventBuffer_Create returned a null reference");
    char destinationData[_dataLength];
    parcEventBuffer_Append(parcEventBufferDestination, destinationData, _dataLength);

    parcEventBuffer_AppendBuffer(parcEventBufferSource, parcEventBufferDestination);
    size_t bufferSize = parcEventBuffer_GetLength(parcEventBufferDestination);
    assertTrue(bufferSize == (_dataLength * 2), "Destination buffer size doesn't match expected length, %zu != %d", bufferSize, _dataLength * 2);

    parcEventBuffer_Destroy(&parcEventBufferSource);
    parcEventBuffer_Destroy(&parcEventBufferDestination);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventBuffer_Read)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    assertNotNull(parcEventBuffer, "parcEventBuffer_Create returned a null reference");

    char sourceData[64] = "This is a test";
    parcEventBuffer_Append(parcEventBuffer, sourceData, 64);

    char readDataBuffer[64];
    int length = parcEventBuffer_Read(parcEventBuffer, readDataBuffer, 32);

    assertTrue(strncmp(sourceData, readDataBuffer, 32) == 0,
               "Buffer contents written do not match contents read");
    assertTrue(length == 32, "parcEventBuffer_Read length unexpected %d != 32\n", length);

    length = parcEventBuffer_Read(parcEventBuffer, NULL, 64);
    assertTrue(length == 0, "Drain of parcEventBuffer returned %d", length);

    parcEventBuffer_Destroy(&parcEventBuffer);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventBuffer_WriteToFileDescriptor)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    assertNotNull(parcEventBuffer, "parcEventBuffer_Create returned a null reference");

    char sourceData[100] = "This is a test";
    parcEventBuffer_Append(parcEventBuffer, sourceData, 64);
    size_t written = parcEventBuffer_WriteToFileDescriptor(parcEventBuffer, fds[0], 64);

    assertTrue(written == 64, "Length written does not match buffer length.");
    assertTrue(read(fds[1], sourceData, 100) == 64, "Length read does not match length written.");

    parcEventBuffer_Destroy(&parcEventBuffer);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

LONGBOW_TEST_CASE(Global, parc_EventBuffer_ReadFromFileDescriptor)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    assertNotNull(parcEventBuffer, "parcEventBuffer_Create returned a null reference");

    char sourceData[64] = "This is a test";
    parcEventBuffer_Append(parcEventBuffer, sourceData, 64);

    size_t written = parcEventBuffer_WriteToFileDescriptor(parcEventBuffer, fds[0], 64);
    assertTrue(written == 64, "Length written does not match buffer length.");
    size_t read = parcEventBuffer_ReadFromFileDescriptor(parcEventBuffer, fds[1], -1);

    assertTrue(read == 64, "Length read does not match amount written.");

    parcEventBuffer_Destroy(&parcEventBuffer);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

LONGBOW_TEST_CASE(Global, parc_EventBuffer_ReadLine_FreeLine)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    assertNotNull(parcEventBuffer, "parcEventBuffer_Create returned a null reference");

    char sourceData[64] = "This is a test\n";
    parcEventBuffer_Append(parcEventBuffer, sourceData, 64);
    assertTrue(parcEventBuffer_GetLength(parcEventBuffer) == 64, "parcEventBuffer has wrong length %zu.", parcEventBuffer_GetLength(parcEventBuffer));

    size_t bytesRead;
    char *lineRead = parcEventBuffer_ReadLine(parcEventBuffer, &bytesRead);
    // read up to newline and terminate, so we read the newline but don't return it in the result
    assertTrue(parcEventBuffer_GetLength(parcEventBuffer) == (64 - (bytesRead + 1)),
               "parcEventBuffer has wrong length %zu != %zu.",
               (64 - (bytesRead + 1)), parcEventBuffer_GetLength(parcEventBuffer));
    assertTrue(strncmp(sourceData, lineRead, bytesRead) == 0, "Line read doesn't match %s != %s", sourceData, lineRead);
    assertTrue((strlen(sourceData) - strlen(lineRead)) == 1, "Line length doesn't match %zu != %zu", strlen(sourceData), strlen(lineRead));

    parcEventBuffer_FreeLine(parcEventBuffer, &lineRead);

    parcEventBuffer_Destroy(&parcEventBuffer);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

LONGBOW_TEST_CASE(Global, parc_EventBuffer_GetQueueBuffer)
{
    int fds[2];
    int result = socketpair(AF_LOCAL, SOCK_DGRAM, 0, fds);
    assertFalse(result, "Socketpair creation failed.\n");

    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    parcEventBuffer_EnableDebug(parcEventScheduler_GetLogger(parcEventScheduler));

    PARCEventQueue *parcEventQueue = parcEventQueue_Create(parcEventScheduler, fds[0], 0);
    assertNotNull(parcEventQueue, "parcEventQueue_Create returned a null reference");

    PARCEventBuffer *parcEventBuffer_Output = parcEventBuffer_GetQueueBufferOutput(parcEventQueue);
    assertNotNull(parcEventBuffer_Output, "Received null output buffer from queue");

    PARCEventBuffer *parcEventBuffer_Input = parcEventBuffer_GetQueueBufferInput(parcEventQueue);
    assertNotNull(parcEventBuffer_Input, "Received null input buffer from queue");

    parcEventBuffer_Destroy(&parcEventBuffer_Output);
    parcEventBuffer_Destroy(&parcEventBuffer_Input);
    parcEventQueue_Destroy(&parcEventQueue);
    parcEventScheduler_Destroy(&parcEventScheduler);
    close(fds[0]);
    close(fds[1]);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_EventBuffer);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
