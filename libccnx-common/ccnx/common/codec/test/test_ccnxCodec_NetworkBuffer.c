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
#include "../ccnxCodec_NetworkBuffer.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_PublicKeySigner.h>

typedef struct test_data {
    CCNxCodecNetworkBuffer *buffer;
} TestData;

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->buffer = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
    return data;
}

static void
_commonTeardown(TestData *data)
{
    ccnxCodecNetworkBuffer_Release(&data->buffer);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(ccnx_NetworkBuffer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(SetLimit);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_NetworkBuffer)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_NetworkBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBufferIoVec_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_Acquire);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_ComputeSignature);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_CreateFromArray);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_CreateIoVec);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_Display);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_GetUint8);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_GetUint8_NotCurrentBlock);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_Position);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_SpaceOk);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_SpaceToZero);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_NoSpace);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_SpanThree);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutBuffer);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint16);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint64);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint8_SpaceOk);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint8_SpaceToZero);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint8_NoSpace);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint32_OK);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint32_2bytes);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint32_2bytes_withnext);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_SetPosition_BeyondLimit);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_SetPosition_InCurrent);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecNetworkBuffer_SetPosition_InDifferent);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNetworkbufferIoVec_GetArray);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNetworkbufferIoVec_GetCount);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNetworkbufferIoVec_Length);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNetworkbufferIoVec_Display);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBufferIoVec_Acquire)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxCodecNetworkBufferIoVec *first = ccnxCodecNetworkBuffer_CreateIoVec(data->buffer);
    assertTrue(first->refcount == 1, "Wrong refcount, got %u expected %u", first->refcount, 1);

    CCNxCodecNetworkBufferIoVec *second = ccnxCodecNetworkBufferIoVec_Acquire(first);
    assertTrue(first->refcount == 2, "Wrong refcount, got %u expected %u", first->refcount, 2);

    ccnxCodecNetworkBufferIoVec_Release(&second);
    ccnxCodecNetworkBufferIoVec_Release(&first);
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_Acquire)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxCodecNetworkBuffer *second = ccnxCodecNetworkBuffer_Acquire(data->buffer);
    assertTrue(data->buffer->refcount == 2, "wrong refcount, got %u expected %u", data->buffer->refcount, 2);
    ccnxCodecNetworkBuffer_Release(&second);
    assertTrue(data->buffer->refcount == 1, "wrong refcount, got %u expected %u", data->buffer->refcount, 1);
}

/*
 * Uses a test set generated by openssl:
 *     openssl genrsa -out test_rsa_key.pem
 *     openssl rsa -pubout -in test_rsa_key.pem -out test_rsa_pub.pem
 *     openssl req -new -key test_rsa_key.pem  -out test_rsa.csr
 *     openssl x509 -req -days 365 -in test_rsa.csr -signkey test_rsa_key.pem -out test_rsa.crt
 *     openssl pkcs12 -export -in test_rsa.crt -inkey test_rsa_key.pem -out test_rsa.p12 -name ccnxuser -CAfile test_rsa.crt -caname root -chain -passout pass:blueberry
 *     openssl sha -sha256 -sign test_rsa_key.pem -out test_random_bytes.sig < test_random_bytes
 *
 * In English: generate a public private key, put it in a PKCS12 file (test_rsa.p12), then use that to sign
 * a buffer (test_random_bytes) and put the signature in a file (test_random_bytes.sig).
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_ComputeSignature)
{
    parcSecurity_Init();

    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);
    PARCPublicKeySigner *publicKeySigner = parcPublicKeySigner_Create(keyStore, PARCCryptoSuite_RSA_SHA256);
    PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&publicKeySigner);

    parcKeyStore_Release(&keyStore);
    assertNotNull(signer, "Got null result from opening openssl pkcs12 file");

    // read the buffer to sign
    int fd = open("test_random_bytes", O_RDONLY);
    assertTrue(fd != -1, "Cannot open test_random_bytes file.");
    uint8_t buffer_to_sign[2048];
    ssize_t read_bytes = read(fd, buffer_to_sign, 2048);
    close(fd);

    // Put it in a NetworkBuffer
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxCodecNetworkBuffer_PutArray(data->buffer, read_bytes, buffer_to_sign);

    // Sign it
    PARCSignature *testSignature = ccnxCodecNetworkBuffer_ComputeSignature(data->buffer, 0, ccnxCodecNetworkBuffer_Limit(data->buffer), signer);
    PARCBuffer *testBytes = parcSignature_GetSignature(testSignature);

    // now read the "true" signature
    uint8_t scratch_buffer[1024];
    fd = open("test_random_bytes.sig", O_RDONLY);
    assertTrue(fd != -1, "Cannot open test_random_bytes.sig file.");
    read_bytes = read(fd, scratch_buffer, 1024);
    assertTrue(read_bytes == 128, "read incorrect size signature from disk: %zu", read_bytes);
    close(fd);

    PARCBuffer *truth = parcBuffer_Wrap(scratch_buffer, read_bytes, 0, read_bytes);

    assertTrue(parcBuffer_Equals(testBytes, truth), "Signatures do not match")
    {
        parcBuffer_Display(testBytes, 0);
        parcBuffer_Display(truth, 0);
    }

    parcBuffer_Release(&truth);
    parcSignature_Release(&testSignature);
    parcSigner_Release(&signer);

    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_Create)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    assertNotNull(data->buffer, "null buffer");
    assertTrue(data->buffer->head == data->buffer->current && data->buffer->current == data->buffer->tail,
               "wrong pointers, head should equal current should equal tail");
    assertTrue(data->buffer->refcount == 1, "wrong refcount, got %u expected %u", data->buffer->refcount, 1);
    assertTrue(data->buffer->position == 0, "wrong position, got %zu expected %u", data->buffer->position, 1);
}


LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_CreateFromArray)
{
    size_t length = 64;
    uint8_t *memory = parcMemory_Allocate(length);
    assertNotNull(memory, "parcMemory_Allocate(%zu) returned NULL", length);
    for (int i = 0; i < length; i++) {
        memory[i] = i * 3;
    }

    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, length, memory);

    assertNotNull(netbuff, "Got null from createFromArray");

    PARCBuffer *test = ccnxCodecNetworkBuffer_CreateParcBuffer(netbuff);
    PARCBuffer *truth = parcBuffer_Wrap(memory, length, 0, length);

    assertTrue(parcBuffer_Equals(test, truth), "Buffers do not match")
    {
        ccnxCodecNetworkBuffer_Display(netbuff, 3);
        parcBuffer_Display(test, 3);
        parcBuffer_Display(truth, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecNetworkBuffer_Release(&netbuff);
}


LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_CreateIoVec)
{
    // Write an array that will span 3 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(data->buffer);

    assertTrue(vec->iovcnt == 5, "iovcnt wrong got %d expected %d", vec->iovcnt, 5);
    assertTrue(vec->totalBytes == 8192, "Wrong total bytes, got %zu expected %u", vec->totalBytes, 8192)
    {
        ccnxCodecNetworkBufferIoVec_Display(vec, 3);
    }

    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

/*
 * not much to do excpet make sure there's no leaks or assertions
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_Display)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxCodecNetworkBuffer_Display(data->buffer, 0);
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_Position)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    data->buffer->position = 22;

    size_t test = ccnxCodecNetworkBuffer_Position(data->buffer);
    assertTrue(test == 22, "wrong position, got %zu expected %u", test, 22);
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_SpaceOk)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t array[] = { 1, 2, 3, 4, 5, 6 };
    size_t nextPosition = data->buffer->position + sizeof(array);

    ccnxCodecNetworkBuffer_PutArray(data->buffer, sizeof(array), array);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);
    assertTrue(memcmp(&data->buffer->current->memory[0], array, sizeof(array)) == 0, "wrong memory");
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_SpaceToZero)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t array[] = { 1, 2, 3, 4, 5, 6 };

    size_t startPosition = data->buffer->capacity - sizeof(array);
    size_t nextPosition = startPosition + sizeof(array);

    data->buffer->position = startPosition;

    ccnxCodecNetworkBuffer_PutArray(data->buffer, sizeof(array), array);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);
    assertTrue(memcmp(&data->buffer->current->memory[startPosition], array, sizeof(array)) == 0, "wrong memory");
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_NoSpace)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t array[] = { 1, 2, 3, 4, 5, 6 };

    // 3 elements in the current block, 3 in the next block
    size_t startPosition = data->buffer->capacity - 3;
    size_t nextPosition = startPosition + sizeof(array);

    data->buffer->position = startPosition;

    ccnxCodecNetworkBuffer_PutArray(data->buffer, sizeof(array), array);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);
    assertTrue(memcmp(&data->buffer->head->memory[startPosition], array, 3) == 0, "wrong memory");
    assertTrue(memcmp(&data->buffer->tail->memory[0], array + 3, 3) == 0, "wrong memory");
    // and we should have a new buffer
    assertTrue(data->buffer->head != data->buffer->tail, "head should not be equal to tail");
}


LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutArray_SpanThree)
{
    // Write an array that will span 3 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);

    CCNxCodecNetworkBufferMemory *block = data->buffer->head;
    size_t offset = 0;
    while (offset < arrayLength) {
        size_t remaining = (arrayLength - offset > block->capacity) ? block->capacity : arrayLength - offset;
        assertTrue(memcmp(&block->memory[0], array + offset, remaining) == 0, "wrong memory");
        offset += remaining;
        block = block->next;
    }
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutBuffer)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    uint8_t array[] = { 1, 2, 3, 4, 5, 6 };
    PARCBuffer *buffer = parcBuffer_Wrap(array, sizeof(array), 0, sizeof(array));

    size_t nextPosition = data->buffer->position + sizeof(array);

    ccnxCodecNetworkBuffer_PutBuffer(data->buffer, buffer);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);
    assertTrue(memcmp(&data->buffer->current->memory[0], array, sizeof(array)) == 0, "wrong memory");

    parcBuffer_Release(&buffer);
}


LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint16)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint16_t value = 0x2587;
    size_t nextPosition = data->buffer->position + sizeof(value);

    ccnxCodecNetworkBuffer_PutUint16(data->buffer, value);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);

    uint16_t testValue = htons(value);
    assertTrue(memcmp(&data->buffer->current->memory[0], &testValue, sizeof(testValue)) == 0, "wrong memory")
    {
        ccnxCodecNetworkBuffer_Display(data->buffer, 0);
    }
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint64)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint64_t value = 0xABCDEF0122334455;
    size_t nextPosition = data->buffer->position + sizeof(value);

    ccnxCodecNetworkBuffer_PutUint64(data->buffer, value);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);

    uint8_t truthValue[] = { 0xAB, 0xCD, 0xEF, 0x01, 0x22, 0x33, 0x44, 0x55 };
    assertTrue(memcmp(&data->buffer->current->memory[0], truthValue, sizeof(truthValue)) == 0, "wrong memory")
    {
        ccnxCodecNetworkBuffer_Display(data->buffer, 0);
    }
}

/*
 * Put a uint32 in to a block with plenty of space
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint32_OK)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint32_t value = 0xABCDEF01;
    size_t nextPosition = data->buffer->position + 4;

    ccnxCodecNetworkBuffer_PutUint32(data->buffer, value);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);

    uint32_t testValue = htonl(value);
    assertTrue(memcmp(&data->buffer->current->memory[0], &testValue, sizeof(testValue)) == 0, "wrong memory")
    {
        ccnxCodecNetworkBuffer_Display(data->buffer, 0);
    }
}

/*
 * The current block only has 2 bytes left and there is no next pointer.  Should throw away
 * those 2 bytes, allocate a new block, then write the whole thing there.
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint32_2bytes)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // set limit and position out to capacity -2
    data->buffer->current->limit = data->buffer->current->capacity - 2;
    data->buffer->position = data->buffer->current->limit;

    uint32_t value = 0xABCDEF01;
    size_t nextPosition = data->buffer->position + 4;

    ccnxCodecNetworkBuffer_PutUint32(data->buffer, value);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);

    uint32_t testValue = htonl(value);
    assertTrue(memcmp(&data->buffer->current->memory[0], &testValue, sizeof(testValue)) == 0, "wrong memory")
    {
        ccnxCodecNetworkBuffer_Display(data->buffer, 0);
    }
}

/*
 * The current block only has 2 bytes left and there is a next block.  Because the current
 * block is frozen, it will need to split the write over the two blocks.
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint32_2bytes_withnext)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // this is where we'll want to start our write
    size_t start = data->buffer->current->capacity - 2;
    size_t nextPosition = start + 4;

    // set limit and position out to capacity then allocate another block
    data->buffer->current->limit = data->buffer->current->capacity;
    data->buffer->position = data->buffer->current->limit;
    _ccnxCodecNetworkBuffer_AllocateIfNeeded(data->buffer);

    ccnxCodecNetworkBuffer_SetPosition(data->buffer, start);
    uint32_t value = 0x33445566;
    ccnxCodecNetworkBuffer_PutUint32(data->buffer, value);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);

    uint32_t testValue = htonl(value);
    // check the value is split between the two buffers
    assertTrue(memcmp(&data->buffer->head->memory[start], &testValue, 2) == 0, "wrong memory in first buffer")
    {
        ccnxCodecNetworkBuffer_Display(data->buffer, 0);
    }

    assertTrue(memcmp(&data->buffer->tail->memory[0], (uint8_t *) &testValue + 2, 2) == 0, "wrong memory in second buffer")
    {
        ccnxCodecNetworkBuffer_Display(data->buffer, 0);
    }
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_GetUint8)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint8_t value = 1;

    ccnxCodecNetworkBuffer_PutUint8(data->buffer, value);

    uint8_t test = ccnxCodecNetworkBuffer_GetUint8(data->buffer, 0);
    assertTrue(data->buffer->current->memory[0] == test, "wrong memory, got %u expected %u", test, data->buffer->current->memory[0]);
}

/*
 * Write stuff that spans two blocks, then get the uint8 from the second block
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_GetUint8_NotCurrentBlock)
{
    // Write an array that will span 5 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);

    uint8_t test = ccnxCodecNetworkBuffer_GetUint8(data->buffer, 4777);
    assertTrue(test == array[4777], "Data at index 4777 wrong, got %02X expected %02X",
               test, array[4777]);
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint8_SpaceOk)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint8_t value = 1;
    size_t relativePosition = data->buffer->position - data->buffer->current->begin;
    size_t nextPosition = data->buffer->position + 1;

    ccnxCodecNetworkBuffer_PutUint8(data->buffer, value);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);
    assertTrue(data->buffer->current->memory[relativePosition] == value, "wrong memory");
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint8_SpaceToZero)
{
    // put the position to just before the end of the buffer
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint8_t value = 1;

    data->buffer->position = data->buffer->current->capacity - 1;
    size_t relativePosition = data->buffer->position - data->buffer->current->begin;
    size_t nextPosition = data->buffer->position + 1;

    ccnxCodecNetworkBuffer_PutUint8(data->buffer, value);
    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);
    assertTrue(data->buffer->current->memory[relativePosition] == value, "wrong memory");
}

LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_PutUint8_NoSpace)
{
    // put the position at the end of the current buffer, force an allocation
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint8_t value = 1;

    // pretend we've written all the way out to the capacity
    data->buffer->position = data->buffer->current->capacity;
    data->buffer->current->limit = data->buffer->current->capacity;

    size_t nextPosition = data->buffer->position + 1;

    ccnxCodecNetworkBuffer_PutUint8(data->buffer, value);

    size_t relativePosition = 0;

    assertTrue(data->buffer->position == nextPosition, "Wrong position, got %zu expected %zu", data->buffer->position, nextPosition);
    assertTrue(data->buffer->current->memory[relativePosition] == value, "wrong memory");
    // and we should have a new buffer
    assertTrue(data->buffer->head != data->buffer->tail, "head should not be equal to tail");
}

/*
 * Set position beyond the limit of what's been written
 */
LONGBOW_TEST_CASE_EXPECTS(Global, ccnxCodecNetworkBuffer_SetPosition_BeyondLimit, .event = &LongBowAssertEvent)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t limit = ccnxCodecNetworkBuffer_Limit(data->buffer);
    ccnxCodecNetworkBuffer_SetPosition(data->buffer, limit + 1);
}

/*
 * Set position to good location that is in the current block
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_SetPosition_InCurrent)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxCodecNetworkBuffer_PutUint32(data->buffer, 0x12345678);

    size_t limit = ccnxCodecNetworkBuffer_Limit(data->buffer);
    ccnxCodecNetworkBuffer_SetPosition(data->buffer, limit - 1);

    assertTrue(data->buffer->current->memory[data->buffer->position] == 0x78,
               "Wrong memory got %02X expected %02X",
               data->buffer->current->memory[data->buffer->position], 0x78)
    {
        ccnxCodecNetworkBuffer_Display(data->buffer, 0);
    }
}

/*
 * Set position to a good location that is not in the current block
 */
LONGBOW_TEST_CASE(Global, ccnxCodecNetworkBuffer_SetPosition_InDifferent)
{
    // Write an array that will span 5 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);

    ccnxCodecNetworkBuffer_SetPosition(data->buffer, 4777);

    assertTrue(data->buffer->position == 4777, "Wrong position set, got %zu expected %u", data->buffer->position, 4777);
    assertTrue(_ccnxCodecNetworkBufferMemory_ContainsPosition(data->buffer->current, 4777), "Did not seek to right position");
}

LONGBOW_TEST_CASE(Global, ccnxNetworkbufferIoVec_GetArray)
{
    // Write an array that will span 3 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(data->buffer);

    const struct iovec *iov = ccnxCodecNetworkBufferIoVec_GetArray(vec);
    assertTrue(iov == vec->array, "Got wrong iovec array, got %p expected %p", (void *) iov, (void *) vec->array);

    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

LONGBOW_TEST_CASE(Global, ccnxNetworkbufferIoVec_GetCount)
{
    // Write an array that will span 3 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(data->buffer);

    assertTrue(ccnxCodecNetworkBufferIoVec_GetCount(vec) == 5, "iovcnt wrong got %d expected %d", ccnxCodecNetworkBufferIoVec_GetCount(vec), 5);

    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

LONGBOW_TEST_CASE(Global, ccnxNetworkbufferIoVec_Length)
{
    // Write an array that will span 3 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(data->buffer);

    assertTrue(ccnxCodecNetworkBufferIoVec_Length(vec) == arrayLength, "Wrong length got %zu expected %zu", ccnxCodecNetworkBufferIoVec_Length(vec), arrayLength);

    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

LONGBOW_TEST_CASE(Global, ccnxNetworkbufferIoVec_Display)
{
    // Write an array that will span 3 blocks
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    size_t arrayLength = 8192;
    uint8_t array[arrayLength];

    // fill the array with stuff so we have a pattern that must match
    for (size_t i = 0; i < arrayLength; i++) {
        array[i] = i;
    }

    ccnxCodecNetworkBuffer_PutArray(data->buffer, arrayLength, array);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(data->buffer);

    ccnxCodecNetworkBufferIoVec_Display(vec, 0);

    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

// =====================================================================

LONGBOW_TEST_FIXTURE(SetLimit)
{
    LONGBOW_RUN_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_EndOfTail);
    LONGBOW_RUN_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_MidOfTail);
    LONGBOW_RUN_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_StartOfTail);
    LONGBOW_RUN_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_EndOfMid);
    LONGBOW_RUN_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_MidOfMid);
    LONGBOW_RUN_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_StartOfMid);
    LONGBOW_RUN_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_Zero);
}

LONGBOW_TEST_FIXTURE_SETUP(SetLimit)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(SetLimit)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

typedef struct setlimit_s {
    CCNxCodecNetworkBuffer *netbuff;
    PARCBuffer *truth;
} SetLimitData;

/*
 * In this test, SetLimit is called when we are at position 4036
 *
 *    (always in ABSOLUTE bytes)
 *                                                                         position = 4077
 *    begin = 0                  begin = 1536               begin = 3577   |
 *    |                          |                          |              |
 *   +--------------------------+--------------------------+--------------------------+
 *   |         block 0          |         block 1          |         block 2          |
 *   +--------------------------+--------------------------+--------------------------+
 *                             |                       |                   |          |
 *                          capacity = 1536        capacity = 2048         |      capacity = 2048
 *                          limit = 1536           limit = 2041        limit = 500
 *    (always in RELATIVE bytes)
 */
static SetLimitData
_allocateData(void)
{
    SetLimitData data;

    data.netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);

    size_t buffer1_length = 3577;
    uint8_t buffer1[buffer1_length];
    memset(buffer1, 0x11, buffer1_length);

    ccnxCodecNetworkBuffer_PutArray(data.netbuff, buffer1_length, buffer1);
    assertTrue(data.netbuff->position == buffer1_length, "Wrong position, expected %zu got %zu", buffer1_length, data.netbuff->position);

    // we should be in 'block1' in the diagram
    assertTrue(data.netbuff->current->limit == 2041, "wrong limit, expected %u got %zu", 2041, data.netbuff->current->limit);

    // now allocate the second buffer to move to 'block 2'.  this should freeze 'block 1' at 2000 bytes.

    // now we need to write it at 8 bytes to get block 1 to freeze
    uint64_t x = 0x1234567812345678ULL;

    ccnxCodecNetworkBuffer_PutUint64(data.netbuff, x);
    assertTrue(data.netbuff->position == 3585, "Wrong position, expected %u got %zu", 3585, data.netbuff->position);
    assertTrue(data.netbuff->current->limit == 8, "wrong limit, expected %u got %zu", 8, data.netbuff->current->limit);

    size_t buffer2_length = 492;
    uint8_t buffer2[buffer2_length];
    memset(buffer2, 0xaa, buffer2_length);

    ccnxCodecNetworkBuffer_PutArray(data.netbuff, buffer2_length, buffer2);

    assertTrue(data.netbuff->position == 4077, "Wrong position, expected %u got %zu", 4077, data.netbuff->current->limit);
    assertTrue(data.netbuff->current->limit == 500, "wrong limit, expected %u got %zu", 500, data.netbuff->current->limit);

    data.truth = parcBuffer_Allocate(buffer1_length + buffer2_length + 8);
    parcBuffer_PutArray(data.truth, buffer1_length, buffer1);
    parcBuffer_PutUint64(data.truth, x);
    parcBuffer_PutArray(data.truth, buffer2_length, buffer2);
    parcBuffer_Flip(data.truth);

    return data;
}

static void
_destroyData(SetLimitData data)
{
    ccnxCodecNetworkBuffer_Release(&data.netbuff);
    parcBuffer_Release(&data.truth);
}

static void
_runDataTest(size_t position)
{
    SetLimitData data = _allocateData();
    ccnxCodecNetworkBuffer_SetPosition(data.netbuff, position);
    parcBuffer_SetLimit(data.truth, position);

    ccnxCodecNetworkBuffer_Finalize(data.netbuff);
    PARCBuffer *test = ccnxCodecNetworkBuffer_CreateParcBuffer(data.netbuff);
    assertTrue(parcBuffer_Equals(data.truth, test), "wrong value")
    {
        printf("Expected\n");
        parcBuffer_Display(data.truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&test);
    _destroyData(data);
}

/*
 * In this test, SetLimit is called when we are at position 4077
 */
LONGBOW_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_EndOfTail)
{
    _runDataTest(4077);
}

/*
 * In this test, SetLimit is called when we are at position 4000, which
 * is in the middle of 'block 2'
 */
LONGBOW_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_MidOfTail)
{
    _runDataTest(4000);
}

/*
 * In this test, SetLimit is called when we are at position 3577, which
 * is in the start of 'block 2'
 */
LONGBOW_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_StartOfTail)
{
    _runDataTest(3577);
}

/*
 * In this test, SetLimit is called when we are at position 3576, which
 * is the last byte of 'block 1'
 */
LONGBOW_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_EndOfMid)
{
    _runDataTest(3576);
}

/*
 * In this test, SetLimit is called when we are at position 2000, which
 * is the middle of 'block 1'
 */
LONGBOW_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_MidOfMid)
{
    _runDataTest(2000);
}

/*
 * 1536 is 1st byte of 'block 1'
 */
LONGBOW_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_StartOfMid)
{
    _runDataTest(1536);
}

/*
 * Wipe it all out
 */
LONGBOW_TEST_CASE(SetLimit, ccnxCodecNetworkBuffer_Finalize_Zero)
{
    _runDataTest(0);
}


// =========================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _ccnxCodecNetworkBufferMemory_Allocate);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _ccnxCodecNetworkBufferMemory_Allocate)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    size_t desired = 2048;
    CCNxCodecNetworkBufferMemory *memory = _ccnxCodecNetworkBufferMemory_Allocate(data->buffer, desired);
    assertNotNull(memory, "Got null memory");
    assertNull(memory->next, "memory->next is not null");
    assertTrue(memory->begin == 0, "memory has wrong offset, got %zu expecting %u", memory->begin, 0);
    assertTrue(memory->capacity == desired, "Wrong capacity, got %zu expecting %zu", memory->capacity, desired);

    _ccnxCodecNetworkBufferMemory_Release(data->buffer, &memory);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_NetworkBuffer);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
