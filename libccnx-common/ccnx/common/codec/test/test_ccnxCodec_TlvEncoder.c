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
#include "../ccnxCodec_TlvEncoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <ccnx/common/validation/ccnxValidation_CRC32C.h>

LONGBOW_TEST_RUNNER(parc_Tlv)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Encoder);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Tlv)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Tlv)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==========================================

LONGBOW_TEST_FIXTURE(Encoder)
{
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendArray);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendRawArray);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendBuffer);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendBuffer_TestReturn);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendContainer);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint8);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint16);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint32);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint64);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendVarInt);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_PutUint8);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_PutUint16);

    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Create);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Finalize);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Finalize_TrimLimit_Buffer);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Finalize_TrimLimit_IoVec);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Initialize);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Initialize_Twice);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Position);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetContainerLength);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetPosition);

    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetError_Present);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetError_Missing);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_GetError);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_ClearError_Present);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_ClearError_Missing);

    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_MarkSignatureEnd);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_MarkSignatureStart);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_ComputeSignature);
    LONGBOW_RUN_TEST_CASE(Encoder, ccnxCodecTlvEncoder_GetSigner);
}

LONGBOW_TEST_FIXTURE_SETUP(Encoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Encoder)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * We will create a TLV structure that looks like this:
 *   { T = 1,  L = 19 },
 *      { T = 2, L = 5, V = "hello" }
 *      { T = 3, L = 6, V = "mr tlv" }
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendArray)
{
    uint8_t truthBytes[] = { 0x00, 0x01, 0x00, 0x13,
                             0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
                             0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v' };

    PARCBuffer *truth = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    uint8_t helloString[] = "hello";
    uint8_t mrTlvString[] = "mr tlv";

    CCNxCodecTlvEncoder *innerEncoder = ccnxCodecTlvEncoder_Create();

    // sizeof -1 to account for null byte at end of string
    ccnxCodecTlvEncoder_AppendArray(innerEncoder, 2, sizeof(helloString) - 1, helloString);
    ccnxCodecTlvEncoder_AppendArray(innerEncoder, 3, sizeof(mrTlvString) - 1, mrTlvString);

    ccnxCodecTlvEncoder_Finalize(innerEncoder);
    PARCBuffer *inner = ccnxCodecTlvEncoder_CreateBuffer(innerEncoder);

    CCNxCodecTlvEncoder *outerEncoder = ccnxCodecTlvEncoder_Create();

    ccnxCodecTlvEncoder_AppendBuffer(outerEncoder, 1, inner);
    ccnxCodecTlvEncoder_Finalize(outerEncoder);
    PARCBuffer *container = ccnxCodecTlvEncoder_CreateBuffer(outerEncoder);

    parcBuffer_Release(&inner);

    ccnxCodecTlvEncoder_Destroy(&innerEncoder);
    ccnxCodecTlvEncoder_Destroy(&outerEncoder);

    // parcBuffer_ToString() will not work well as we have 0 bytes, use parcBuffer_ToHexString() case 1017
    assertTrue(parcBuffer_Equals(truth, container), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(container, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&container);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendRawArray)
{
    uint8_t truthBytes[] = { 0x00, 0x01, 0x00, 0x13,
                             0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
                             0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v' };

    PARCBuffer *truth = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    uint8_t helloString[] = "hello";
    uint8_t mrTlvString[] = "mr tlv";

    CCNxCodecTlvEncoder *innerEncoder = ccnxCodecTlvEncoder_Create();

    // sizeof -1 to account for null byte at end of string
    ccnxCodecTlvEncoder_AppendContainer(innerEncoder, 2, sizeof(helloString) - 1);
    ccnxCodecTlvEncoder_AppendRawArray(innerEncoder, sizeof(helloString) - 1, helloString);
    ccnxCodecTlvEncoder_AppendContainer(innerEncoder, 3, sizeof(mrTlvString) - 1);
    ccnxCodecTlvEncoder_AppendRawArray(innerEncoder, sizeof(mrTlvString) - 1, mrTlvString);

    ccnxCodecTlvEncoder_Finalize(innerEncoder);
    PARCBuffer *inner = ccnxCodecTlvEncoder_CreateBuffer(innerEncoder);

    CCNxCodecTlvEncoder *outerEncoder = ccnxCodecTlvEncoder_Create();

    ccnxCodecTlvEncoder_AppendBuffer(outerEncoder, 1, inner);
    ccnxCodecTlvEncoder_Finalize(outerEncoder);
    PARCBuffer *container = ccnxCodecTlvEncoder_CreateBuffer(outerEncoder);

    parcBuffer_Release(&inner);

    ccnxCodecTlvEncoder_Destroy(&innerEncoder);
    ccnxCodecTlvEncoder_Destroy(&outerEncoder);

    // parcBuffer_ToString() will not work well as we have 0 bytes, use parcBuffer_ToHexString() case 1017
    assertTrue(parcBuffer_Equals(truth, container), "Buffer mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(container, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&container);
}


/**
 * We will create a TLV structure that looks like this:
 *   { T = 1,  L = 19 },
 *      { T = 2, L = 5, V = "hello" }
 *      { T = 3, L = 6, V = "mr tlv" }
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendBuffer)
{
    uint8_t truthBytes[] = { 0x00, 0x01, 0x00, 0x13,
                             0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
                             0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v' };

    PARCBuffer *truth = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    uint8_t helloString[] = "hello";
    PARCBuffer *hello = parcBuffer_Wrap(helloString, 5, 0, 5);

    uint8_t mrTlvString[] = "mr tlv";
    PARCBuffer *mrTlv = parcBuffer_Wrap(mrTlvString, 6, 0, 6);

    CCNxCodecTlvEncoder *innerEncoder = ccnxCodecTlvEncoder_Create();

    ccnxCodecTlvEncoder_AppendBuffer(innerEncoder, 2, hello);
    ccnxCodecTlvEncoder_AppendBuffer(innerEncoder, 3, mrTlv);

    ccnxCodecTlvEncoder_Finalize(innerEncoder);
    PARCBuffer *inner = ccnxCodecTlvEncoder_CreateBuffer(innerEncoder);

    CCNxCodecTlvEncoder *outerEncoder = ccnxCodecTlvEncoder_Create();

    ccnxCodecTlvEncoder_AppendBuffer(outerEncoder, 1, inner);
    ccnxCodecTlvEncoder_Finalize(outerEncoder);
    PARCBuffer *container = ccnxCodecTlvEncoder_CreateBuffer(outerEncoder);

    parcBuffer_Release(&inner);
    parcBuffer_Release(&hello);
    parcBuffer_Release(&mrTlv);

    ccnxCodecTlvEncoder_Destroy(&innerEncoder);
    ccnxCodecTlvEncoder_Destroy(&outerEncoder);

    // parcBuffer_ToString() will not work well as we have 0 bytes, use parcBuffer_ToHexString() case 1017
    assertTrue(parcBuffer_Equals(truth, container),
               "buffers not equal\nexpected '%s'\ngot      '%s'\n",
               parcBuffer_ToString(truth),
               parcBuffer_ToString(container)
               );

    parcBuffer_Release(&truth);
    parcBuffer_Release(&container);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendBuffer_TestReturn)
{
    uint8_t helloString[] = "hello";
    PARCBuffer *hello = parcBuffer_Wrap(helloString, 5, 0, 5);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);

    size_t trueLength = 2 + 2 + 5;
    size_t length = ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, hello);
    assertTrue(length == trueLength, "AppendBuffer returned wrong length, expected %zu got %zu", trueLength, length);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&hello);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendContainer)
{
    uint8_t truthString[] = { 0x00, 0x02, 0xF1, 0x07 };
    PARCBuffer *truth = parcBuffer_Wrap(truthString, 4, 0, 4);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);

    size_t trueLength = 2 + 2;
    size_t length = ccnxCodecTlvEncoder_AppendContainer(encoder, 2, 0xF107);
    assertTrue(length == trueLength, "AppendBuffer returned wrong length, expected %zu got %zu", trueLength, length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    assertTrue(parcBuffer_Equals(test, truth), "Buffer is incorrect.");

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
}

/**
 * Check for memory leaks and correct isInitialized state
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Create)
{
    size_t before = parcMemory_Outstanding();
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    // add a signer to make sure it is destroyed
    PARCSigner *signer = ccnxValidationCRC32C_CreateSigner();
    ccnxCodecTlvEncoder_SetSigner(encoder, signer);
    parcSigner_Release(&signer);

    // add an error to make sure its destroyed too
    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, "foo", 1, 1);
    ccnxCodecTlvEncoder_SetError(encoder, error);
    ccnxCodecError_Release(&error);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    size_t after = parcMemory_Outstanding();
    assertTrue(before == after, "Memory leak, expected %zu got %zu bytes\n", before, after);
}

/**
 * Check for memory leaks
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Finalize)
{
    size_t before = parcMemory_Outstanding();
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    parcBuffer_Release(&test);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    size_t after = parcMemory_Outstanding();
    assertTrue(before == after, "Memory leak, expected %zu got %zu bytes\n", before, after);
}

/*
 * Do a long write, then backup the position.
 * After Finalize, the Limit should have trimmed off the erased part.
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Finalize_TrimLimit_Buffer)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    uint8_t array[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    ccnxCodecTlvEncoder_AppendRawArray(encoder, sizeof(array), array);
    ccnxCodecTlvEncoder_SetPosition(encoder, 3);
    ccnxCodecTlvEncoder_Finalize(encoder);

    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Remaining(test) == 3, "Wrong length, expected 3 got %zu", parcBuffer_Remaining(test));

    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

/*
 * Do a long write, then backup the position.
 * After Finalize, the Limit should have trimmed off the erased part.
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Finalize_TrimLimit_IoVec)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    uint8_t array[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    ccnxCodecTlvEncoder_AppendRawArray(encoder, sizeof(array), array);
    ccnxCodecTlvEncoder_SetPosition(encoder, 3);
    ccnxCodecTlvEncoder_Finalize(encoder);

    CCNxCodecNetworkBufferIoVec *iov = ccnxCodecTlvEncoder_CreateIoVec(encoder);
    assertTrue(ccnxCodecNetworkBufferIoVec_Length(iov) == 3, "Wrong length, expected 3 got %zu", ccnxCodecNetworkBufferIoVec_Length(iov));

    ccnxCodecNetworkBufferIoVec_Release(&iov);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

/**
 * Check for memory leaks and correct isInitialized state
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Initialize)
{
    size_t before = parcMemory_Outstanding();
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    ccnxCodecTlvEncoder_Initialize(encoder);

    ccnxCodecTlvEncoder_Destroy(&encoder);

    size_t after = parcMemory_Outstanding();
    assertTrue(before == after, "Memory leak, expected %zu got %zu bytes\n", before, after);
}

/**
 * Make sure calling Initialized on an Initialized buffer does not leak
 */
LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Initialize_Twice)
{
    size_t before = parcMemory_Outstanding();
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_Destroy(&encoder);

    size_t after = parcMemory_Outstanding();
    assertTrue(before == after, "Memory leak, expected %zu got %zu bytes\n", before, after);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_Position)
{
    uint8_t helloString[] = "hello";
    PARCBuffer *hello = parcBuffer_Wrap(helloString, 5, 0, 5);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);

    size_t length = ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, hello);
    size_t position = ccnxCodecTlvEncoder_Position(encoder);

    assertTrue(length == position, "Position not right expected %zu got %zu", length, position);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&hello);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetContainerLength)
{
    uint8_t helloString[] = "hello";
    PARCBuffer *hello = parcBuffer_Wrap(helloString, 5, 0, 5);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);

    size_t containerPosition = ccnxCodecTlvEncoder_Position(encoder);
    ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, hello);
    ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, hello);

    size_t currentPosition = ccnxCodecTlvEncoder_Position(encoder);

    // when I set the length of the first container, we should be positioned back to
    // the current location.

    ccnxCodecTlvEncoder_SetContainerLength(encoder, containerPosition, 99);
    size_t testPosition = ccnxCodecTlvEncoder_Position(encoder);

    assertTrue(testPosition == currentPosition, "Position not right expected %zu got %zu", currentPosition, testPosition);

    // and make sure the length was updated
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *output = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    parcBuffer_SetPosition(output, 2);
    uint16_t testlength = parcBuffer_GetUint16(output);

    assertTrue(testlength == 99, "Updated length wrong, expected %u got %u", 99, testlength);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&hello);
    parcBuffer_Release(&output);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint8)
{
    PARCBuffer *truth = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x20, 0x00, 0x01, 0xFF }, 5, 0, 5);
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint8(encoder, 0x1020, 0xFF);
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers not equal\n");
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal");
    }
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint16)
{
    PARCBuffer *truth = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x21, 0x00, 0x02, 0xFF, 0x01 }, 6, 0, 6);
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint16(encoder, 0x1021, 0xFF01);
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers not equal\n");
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal");
    }
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint32)
{
    PARCBuffer *truth = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x22, 0x00, 0x04, 0xFF, 0x01, 0x02, 0x03 }, 8, 0, 8);
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint32(encoder, 0x1022, 0xFF010203);
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers not equal\n");
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal");
    }
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendUint64)
{
    PARCBuffer *truth = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00, 0x08, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }, 12, 0, 12);
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint64(encoder, 0x1023, 0xFF01020304050607ULL);
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers not equal\n");
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal");
    }
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_AppendVarInt)
{
    struct test_vector {
        uint64_t value;
        bool valid;
        int length;
        uint8_t *array;
    } vectors[] = {
        { .value = 0,                     .valid = true,  .length = 5,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x01, 0x00 } },
        { .value = 0xFF,                  .valid = true,  .length = 5,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x01, 0xFF } },
        { .value = 0x0101,                .valid = true,  .length = 6,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x02, 0x01, 0x01} },
        { .value = 0xFF01,                .valid = true,  .length = 6,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x02, 0xFF, 0x01} },
        { .value = 0x010001,              .valid = true,  .length = 7,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x03, 0x01, 0x00, 0x01} },
        { .value = 0xFF0001,              .valid = true,  .length = 7,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x03, 0xFF, 0x00, 0x01} },
        { .value = 0x01000000,            .valid = true,  .length = 8,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00} },
        { .value = 0xFF002001,            .valid = true,  .length = 8,  .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x04, 0xFF, 0x00, 0x20, 0x01} },
        { .value = 0xFF00200103040506ULL, .valid = true,  .length = 12, .array = (uint8_t[]) { 0x10, 0x23, 0x00, 0x08, 0xFF, 0x00, 0x20, 0x01, 0x03, 0x04, 0x05, 0x06} },
        // sentinal is NULL array
        { .value = 0,                     .valid = false, .length = 0,  .array = NULL },
    };

    for (int i = 0; vectors[i].array != NULL; i++) {
        // only do the valid ones for the encode test
        if (vectors[i].valid) {
            PARCBuffer *truth = parcBuffer_Wrap(vectors[i].array, vectors[i].length, 0, vectors[i].length);

            CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
            ccnxCodecTlvEncoder_Initialize(encoder);
            size_t length = ccnxCodecTlvEncoder_AppendVarInt(encoder, 0x1023, vectors[i].value);

            assertTrue(length == vectors[i].length, "Wrong length index %d, got %zu expected %d", i, length, vectors[i].length);

            ccnxCodecTlvEncoder_Finalize(encoder);
            PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

            assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal index %d", i)
            {
                printf("Expected\n");
                parcBuffer_Display(truth, 3);
                printf("Got\n");
                parcBuffer_Display(test, 3);
            }

            parcBuffer_Release(&test);
            ccnxCodecTlvEncoder_Destroy(&encoder);
            parcBuffer_Release(&truth);
        }
    }
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_MarkSignatureEnd)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint8(encoder, 0x1020, 0xFF);

    ccnxCodecTlvEncoder_MarkSignatureEnd(encoder);
    assertTrue(encoder->signatureEnd == 5, "Wrong end position, expected %u got %zu", 5, encoder->signatureEnd)
    {
        ccnxCodecNetworkBuffer_Display(encoder->buffer, 3);
    }

    assertTrue(encoder->signatureStartEndSet == END_SET, "Wrong flag, expected %d got %d", END_SET, encoder->signatureStartEndSet);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_MarkSignatureStart)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint8(encoder, 0x1020, 0xFF);

    ccnxCodecTlvEncoder_MarkSignatureStart(encoder);
    assertTrue(encoder->signatureStart == 5, "Wrong start position, expected %u got %zu", 5, encoder->signatureStart)
    {
        ccnxCodecNetworkBuffer_Display(encoder->buffer, 3);
    }

    assertTrue(encoder->signatureStartEndSet == START_SET, "Wrong flag, expected %d got %d", START_SET, encoder->signatureStartEndSet);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_ComputeSignature)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_MarkSignatureStart(encoder);
    ccnxCodecTlvEncoder_AppendUint8(encoder, 0x1020, 0xFF);
    ccnxCodecTlvEncoder_MarkSignatureEnd(encoder);

    PARCSigner *signer = ccnxValidationCRC32C_CreateSigner();
    ccnxCodecTlvEncoder_SetSigner(encoder, signer);
    parcSigner_Release(&signer);

    assertTrue(encoder->signatureStartEndSet == BOTH_SET, "Wrong flag, expected %d got %d", BOTH_SET, encoder->signatureStartEndSet);

    PARCSignature *sig = ccnxCodecTlvEncoder_ComputeSignature(encoder);
    assertNotNull(sig, "Got null signature");

    uint8_t truesig[] = { 0xA3, 0xAA, 0xC8, 0x4B };
    PARCBuffer *truesigBuffer = parcBuffer_Rewind(parcBuffer_CreateFromArray(truesig, sizeof(truesig)));
    PARCBuffer *test = parcSignature_GetSignature(sig);
    assertTrue(parcBuffer_Equals(truesigBuffer, test), "wrong crc value")
    {
        printf("Expected\n");
        parcBuffer_Display(truesigBuffer, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truesigBuffer);
    parcSignature_Release(&sig);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_GetSigner)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    PARCSigner *signer = ccnxValidationCRC32C_CreateSigner();
    ccnxCodecTlvEncoder_SetSigner(encoder, signer);

    PARCSigner *test = ccnxCodecTlvEncoder_GetSigner(encoder);
    assertTrue(test == signer, "Did not return the right signer, expected %p got %p", (void *) signer, (void *) test);
    parcSigner_Release(&signer);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_PutUint8)
{
    PARCBuffer *truth = parcBuffer_Wrap((uint8_t[]) { 0x10, 0xEE, 0x00, 0x01, 0xFF }, 5, 0, 5);
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint8(encoder, 0x1020, 0xFF);

    ccnxCodecTlvEncoder_PutUint8(encoder, 1, 0xEE);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers not equal\n");
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal");
    }
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_PutUint16)
{
    PARCBuffer *truth = parcBuffer_Wrap((uint8_t[]) { 0x10, 0xEE, 0xDD, 0x01, 0xFF }, 5, 0, 5);
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);
    ccnxCodecTlvEncoder_AppendUint8(encoder, 0x1020, 0xFF);

    ccnxCodecTlvEncoder_PutUint16(encoder, 1, 0xEEDD);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers not equal\n");
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal");
    }
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}


LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetPosition)
{
    uint8_t helloString[] = "hello";
    PARCBuffer *hello = parcBuffer_Wrap(helloString, 5, 0, 5);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);

    ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, hello);
    // position is now at 9 (2+2+5)

    ccnxCodecTlvEncoder_SetPosition(encoder, 2);

    size_t position = ccnxCodecTlvEncoder_Position(encoder);

    assertTrue(2 == position, "Position not right expected %u got %zu", 2, position);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&hello);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetError_Present)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, "foo", 1, 1);
    ccnxCodecTlvEncoder_SetError(encoder, error);

    // now try to set a second time
    bool success = ccnxCodecTlvEncoder_SetError(encoder, error);
    ccnxCodecError_Release(&error);


    assertFalse(success, "Returned success when should have failed");
    assertNotNull(encoder->error, "Encoder has null error member");

    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_SetError_Missing)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, "foo", 1, 1);
    bool success = ccnxCodecTlvEncoder_SetError(encoder, error);
    ccnxCodecError_Release(&error);

    assertTrue(success, "Returned failure when should have succeeded");
    assertNotNull(encoder->error, "Encoder has null error member");

    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_GetError)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, "foo", 1, 1);
    ccnxCodecTlvEncoder_SetError(encoder, error);
    ccnxCodecError_Release(&error);

    CCNxCodecError *test = ccnxCodecTlvEncoder_GetError(encoder);
    assertNotNull(test, "Encoder has null error member");

    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_ClearError_Present)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, "foo", 1, 1);
    ccnxCodecTlvEncoder_SetError(encoder, error);
    ccnxCodecError_Release(&error);

    ccnxCodecTlvEncoder_ClearError(encoder);
    assertNull(encoder->error, "Encoder does not have a null error");

    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Encoder, ccnxCodecTlvEncoder_ClearError_Missing)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    ccnxCodecTlvEncoder_ClearError(encoder);
    assertNull(encoder->error, "Encoder does not have a null error");

    ccnxCodecTlvEncoder_Destroy(&encoder);
}

// ============================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _ccnxCodecTlvEncoder_ComputeVarIntLength);
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

LONGBOW_TEST_CASE(Local, _ccnxCodecTlvEncoder_ComputeVarIntLength)
{
    struct test_vector {
        uint64_t value;
        int length;
    } vectors[] = {
        { .value = 0,                     .length = 1 },
        { .value = 0xFF,                  .length = 1 },
        { .value = 0x0101,                .length = 2 },
        { .value = 0xFF01,                .length = 2 },
        { .value = 0x010001,              .length = 3 },
        { .value = 0xFF0001,              .length = 3 },
        { .value = 0x01000000,            .length = 4 },
        { .value = 0xFF002001,            .length = 4 },
        { .value = 0x0100000000,          .length = 5 },
        { .value = 0xFF00002001,          .length = 5 },
        { .value = 0x010000000000,        .length = 6 },
        { .value = 0xFF0000002001,        .length = 6 },
        { .value = 0x01000000000000,      .length = 7 },
        { .value = 0xFF000000002001,      .length = 7 },
        { .value = 0xFF00200103040506ULL, .length = 8 },
        // sentinal is length 0
        { .value = 0,                     .length = 0 },
    };

    for (int i = 0; vectors[i].length != 0; i++) {
        unsigned test = _ccnxCodecTlvEncoder_ComputeVarIntLength(vectors[i].value);
        assertTrue(test == vectors[i].length, "Incorrect length index %d, expected %u got %u", i, vectors[i].length, test);
    }
}


// ===================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Tlv);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
