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

/**
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnxCodec_EncodingBuffer.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

typedef struct test_data {
    CCNxCodecEncodingBuffer *encodingBuffer;
} TestData;

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->encodingBuffer = ccnxCodecEncodingBuffer_Create();
    return data;
}

static void
_commonTeardown(TestData *data)
{
    ccnxCodecEncodingBuffer_Release(&data->encodingBuffer);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(ccnxCodec_EncodingBuffer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodec_EncodingBuffer)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodec_EncodingBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_AppendBuffer_FirstAppend);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_AppendBuffer_SameArray);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_AppendBuffer_SecondArray);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_CreateIOVec);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_CreateIOVec_Empty);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_Display);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecEncodingBuffer_Length);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_AppendBuffer_FirstAppend)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Wrap("hello", 5, 0, 5);
    size_t position = ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer);
    assertTrue(position == 0, "Wrong position, got %zu expected %d", position, 0);

    _ccnxCodecEncodingBuffer_Validate(data->encodingBuffer);
    assertTrue(data->encodingBuffer->totalCount == 1, "Wrong count, got %u expected %u", data->encodingBuffer->totalCount, 1);
    assertTrue(data->encodingBuffer->totalBytes == 5, "Wrong bytes, got %zu expected %u", data->encodingBuffer->totalBytes, 5);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_AppendBuffer_SameArray)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Wrap("hello", 5, 0, 5);

    // do two appends
    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer);
    size_t position = ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer);
    assertTrue(position == 1, "Wrong position, got %zu expected %d", position, 1);

    _ccnxCodecEncodingBuffer_Validate(data->encodingBuffer);
    assertTrue(data->encodingBuffer->totalCount == 2, "Wrong count, got %u expected %u", data->encodingBuffer->totalCount, 2);
    assertTrue(data->encodingBuffer->totalBytes == 10, "Wrong bytes, got %zu expected %u", data->encodingBuffer->totalBytes, 10);

    // should still be in the first listarray
    assertTrue(data->encodingBuffer->head == data->encodingBuffer->tail, "Head != tail")
    {
        ccnxCodecEncodingBuffer_Display(data->encodingBuffer, 0);
    }

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_AppendBuffer_SecondArray)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Wrap("hello", 5, 0, 5);

    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer);

    // now fake out the list array so it thinks its full.
    // Do this by reducing the capacity so there are no undefined buffers on the list
    data->encodingBuffer->head->capacity = data->encodingBuffer->head->count;

    size_t position = ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer);
    assertTrue(position == 1, "Wrong position, got %zu expected %u", position, 1);

    _ccnxCodecEncodingBuffer_Validate(data->encodingBuffer);
    assertTrue(data->encodingBuffer->totalCount == 2, "Wrong count, got %u expected %u", data->encodingBuffer->totalCount, 2);
    assertTrue(data->encodingBuffer->totalBytes == 10, "Wrong bytes, got %zu expected %u", data->encodingBuffer->totalBytes, 10);

    // should now have different head and tail
    assertTrue(data->encodingBuffer->head != data->encodingBuffer->tail, "Head == tail")
    {
        ccnxCodecEncodingBuffer_Display(data->encodingBuffer, 0);
    }

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    assertNotNull(data->encodingBuffer, "Got null buffer from create");
    assertNull(data->encodingBuffer->head, "buffer head is not null");
    assertNull(data->encodingBuffer->tail, "Buffer tail is not null");
    assertTrue(data->encodingBuffer->totalCount == 0, "Buffer itemCount is not 0");
    assertTrue(data->encodingBuffer->totalBytes == 0, "Buffer totalBytes is not 0");
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_CreateIOVec)
{
    char foo[] = "foo";
    char bar[] = "barbar";

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer_1 = parcBuffer_Wrap(foo, sizeof(foo), 0, sizeof(foo));
    PARCBuffer *buffer_2 = parcBuffer_Wrap(bar, sizeof(bar), 0, sizeof(bar));
    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer_2);
    ccnxCodecEncodingBuffer_PrependBuffer(data->encodingBuffer, buffer_1);

    CCNxCodecEncodingBufferIOVec *iov = ccnxCodecEncodingBuffer_CreateIOVec(data->encodingBuffer);
    assertNotNull(iov, "Got null iov from CreateIOVec");
    assertTrue(iov->iovcnt == 2, "Wrong iovec count, got %d expected %d", iov->iovcnt, 2);
    assertTrue(iov->iov[0].iov_base == foo, "WRong iov[0].iov_base, got %p exected %p", iov->iov[0].iov_base, foo);
    assertTrue(iov->iov[1].iov_base == bar, "WRong iov[1].iov_base, got %p exected %p", iov->iov[1].iov_base, bar);
    assertTrue(iov->iov[0].iov_len == sizeof(foo), "WRong iov[1].iov_base, got %zu exected %zu", iov->iov[0].iov_len, sizeof(foo));
    assertTrue(iov->iov[1].iov_len == sizeof(bar), "WRong iov[1].iov_base, got %zu exected %zu", iov->iov[1].iov_len, sizeof(bar));

    // Slice crossing two iovec arrays
    CCNxCodecEncodingBuffer *bufferSlice = ccnxCodecEncodingBuffer_Slice(data->encodingBuffer, 1, 6);
    CCNxCodecEncodingBufferIOVec *iovSlice = ccnxCodecEncodingBuffer_CreateIOVec(bufferSlice);
    assertTrue(iovSlice->iovcnt == 2, "Wrong iovec count, got %d expected %d", iovSlice->iovcnt, 2);
    assertTrue(iovSlice->iov[0].iov_base == foo + 1, "WRong iovSlice[0].iov_base, got %p exected %p", iovSlice->iov[0].iov_base, foo + 1);
    assertTrue(iovSlice->iov[0].iov_len == sizeof(foo) - 1, "WRong iovSlice[1].iov_len, got %zu exected %zu", iovSlice->iov[0].iov_len, sizeof(foo) - 1);
    assertTrue(iovSlice->iov[1].iov_len == 6 - (sizeof(foo) - 1), "WRong iovSlice[1].iov_base, got %zu exected %lu", iovSlice->iov[1].iov_len, 6 - (sizeof(foo) - 1));
    ccnxCodecEncodingBufferIOVec_Release(&iovSlice);
    ccnxCodecEncodingBuffer_Release(&bufferSlice);

    // Slice within one iovec array
    bufferSlice = ccnxCodecEncodingBuffer_Slice(data->encodingBuffer, 1, 1);
    iovSlice = ccnxCodecEncodingBuffer_CreateIOVec(bufferSlice);
    assertTrue(iovSlice->iovcnt == 1, "Wrong iovec count, got %d expected %d", iovSlice->iovcnt, 1);
    assertTrue(iovSlice->iov[0].iov_base == foo + 1, "WRong iovSlice[0].iov_base, got %p exected %p", iovSlice->iov[0].iov_base, foo + 1);
    assertTrue(iovSlice->iov[0].iov_len == 1, "WRong iovSlice[1].iov_len, got %zu exected %d", iovSlice->iov[0].iov_len, 1);
    ccnxCodecEncodingBufferIOVec_Release(&iovSlice);
    ccnxCodecEncodingBuffer_Release(&bufferSlice);

    // Slice beyond contents
    bufferSlice = ccnxCodecEncodingBuffer_Slice(data->encodingBuffer, sizeof(foo) + sizeof(bar), 1);
    assertNull(bufferSlice, "ccnxCodecEncodingBuffer_Slice returned allocation for slice outside of buffer");

    // Slice including all and beyond
    bufferSlice = ccnxCodecEncodingBuffer_Slice(data->encodingBuffer, 0, sizeof(foo) + sizeof(bar) + 10);
    iovSlice = ccnxCodecEncodingBuffer_CreateIOVec(bufferSlice);
    assertTrue(iovSlice->iovcnt == 2, "Wrong iovec count, got %d expected %d", iovSlice->iovcnt, 2);
    assertTrue(iovSlice->iov[0].iov_base == foo, "WRong iovSlice[0].iov_base, got %p exected %p", iovSlice->iov[0].iov_base, foo);
    assertTrue(iovSlice->iov[0].iov_len == sizeof(foo), "WRong iovSlice[1].iov_len, got %zu exected %zu", iovSlice->iov[0].iov_len, sizeof(foo));
    assertTrue(iovSlice->iov[1].iov_len == sizeof(bar), "WRong iovSlice[1].iov_base, got %zu exected %lu", iovSlice->iov[1].iov_len, sizeof(bar));
    ccnxCodecEncodingBufferIOVec_Release(&iovSlice);
    ccnxCodecEncodingBuffer_Release(&bufferSlice);

    ccnxCodecEncodingBufferIOVec_Release(&iov);

    parcBuffer_Release(&buffer_1);
    parcBuffer_Release(&buffer_2);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_CreateIOVec_Empty)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxCodecEncodingBufferIOVec *iov = ccnxCodecEncodingBuffer_CreateIOVec(data->encodingBuffer);
    assertNotNull(iov, "Got null iov from CreateIOVec");
    assertTrue(iov->iovcnt == 0, "Wrong iovec count, got %d expected %d", iov->iovcnt, 0);

    // single allocation means that the iov will never be null
    assertNotNull(iov->iov, "iov->iov should not be null");

    ccnxCodecEncodingBufferIOVec_Release(&iov);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_Display)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = parcBuffer_Wrap("hello", 5, 0, 5);
    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer);
    parcBuffer_Release(&buffer);

    ccnxCodecEncodingBuffer_Display(data->encodingBuffer, 0);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_Length)
{
    char foo[] = "foo";
    char bar[] = "barbar";

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer_1 = parcBuffer_Wrap(foo, sizeof(foo), 0, sizeof(foo));
    PARCBuffer *buffer_2 = parcBuffer_Wrap(bar, sizeof(bar), 0, sizeof(bar));
    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer_1);
    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer_2);

    size_t length = ccnxCodecEncodingBuffer_Length(data->encodingBuffer);
    size_t truth = sizeof(foo) + sizeof(bar);
    assertTrue(length == truth, "Wrong length, got %zu expected %zu", length, truth);

    parcBuffer_Release(&buffer_1);
    parcBuffer_Release(&buffer_2);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_Size)
{
    char foo[] = "foo";
    char bar[] = "barbar";

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer_1 = parcBuffer_Wrap(foo, sizeof(foo), 0, sizeof(foo));
    PARCBuffer *buffer_2 = parcBuffer_Wrap(bar, sizeof(bar), 0, sizeof(bar));
    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer_1);
    ccnxCodecEncodingBuffer_AppendBuffer(data->encodingBuffer, buffer_2);

    size_t size = ccnxCodecEncodingBuffer_Size(data->encodingBuffer);
    assertTrue(size == 2, "Wrong size, got %zu expected %u", size, 2);

    parcBuffer_Release(&buffer_1);
    parcBuffer_Release(&buffer_2);
}

LONGBOW_TEST_CASE(Global, ccnxCodecEncodingBuffer_Example)
{
    PARCBuffer *name = parcBuffer_Wrap("marc", 4, 0, 4);
    PARCBuffer *space = parcBuffer_Wrap(" ", 1, 0, 1);
    PARCBuffer *email = parcBuffer_Wrap("<marc@example.com>", 18, 0, 18);

    CCNxCodecEncodingBuffer *encodingBuffer = ccnxCodecEncodingBuffer_Create();
    ccnxCodecEncodingBuffer_AppendBuffer(encodingBuffer, name);
    ccnxCodecEncodingBuffer_AppendBuffer(encodingBuffer, space);
    parcBuffer_Release(&space);
    parcBuffer_Release(&name);

    CCNxCodecEncodingBuffer *emailBuffer = ccnxCodecEncodingBuffer_Create();
    ccnxCodecEncodingBuffer_AppendBuffer(emailBuffer, email);
    parcBuffer_Release(&email);

    ccnxCodecEncodingBuffer_Release(&emailBuffer);

    CCNxCodecEncodingBufferIOVec *iov = ccnxCodecEncodingBuffer_CreateIOVec(encodingBuffer);
    ssize_t nwritten = writev(STDOUT_FILENO, iov->iov, iov->iovcnt);
    assertTrue(nwritten > -1, "Error writev");

    ccnxCodecEncodingBufferIOVec_Release(&iov);

    ccnxCodecEncodingBuffer_Release(&encodingBuffer);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodec_EncodingBuffer);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
