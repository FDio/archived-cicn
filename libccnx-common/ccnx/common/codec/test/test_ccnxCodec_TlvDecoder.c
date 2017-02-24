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
#include "../ccnxCodec_TlvDecoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <inttypes.h>

LONGBOW_TEST_RUNNER(parc_Tlv)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Decoder);
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

// ============================================

LONGBOW_TEST_FIXTURE(Decoder)
{
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Create);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetLength);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetType);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_PeekType);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetValue);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetValue_TooLong);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetContainer);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetContainer_TooLong);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_IsEmpty_True);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_IsEmpty_False);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Position);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_EnsureRemaining_True);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_EnsureRemaining_False);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_Good);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_Short);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_WrongType);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_WrongLength);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_Good);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_Short);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_WrongType);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_WrongLength);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_Good);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_Short);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_WrongType);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_WrongLength);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_Good);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_Short);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_WrongType);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_WrongLength);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_Good);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_WrongType);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_TooShort);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_TooLong);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Advance_Good);
    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Advance_TooLong);

    LONGBOW_RUN_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetVarInt);
}

LONGBOW_TEST_FIXTURE_SETUP(Decoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Decoder)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * check for memory leaks on create/destroy
 */
LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Create)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    size_t before = parcMemory_Outstanding();
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    size_t after = parcMemory_Outstanding();
    parcBuffer_Release(&buffer);
    assertTrue(before == after, "Memory leak, expected %zu got %zu bytes\n", before, after);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetLength)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    // we're calling this on byte 0, so the "length" will be 0x0001
    uint16_t length = ccnxCodecTlvDecoder_GetLength(outerDecoder);

    assertTrue(parcBuffer_Position(outerDecoder->buffer) == 2,
               "Did not advance buffer to right spot, expected %u got %zu",
               2, parcBuffer_Position(outerDecoder->buffer));

    assertTrue(length == 1, "Wrong length expected %u got %u", 1, length);

    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetType)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    uint16_t type = ccnxCodecTlvDecoder_GetType(outerDecoder);

    assertTrue(parcBuffer_Position(outerDecoder->buffer) == 2,
               "Did not advance buffer to right spot, expected %u got %zu",
               2, parcBuffer_Position(outerDecoder->buffer));

    assertTrue(type == 1, "Wrong type expected %u got %u", 1, type);

    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_PeekType)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    uint16_t type = ccnxCodecTlvDecoder_PeekType(outerDecoder);

    assertTrue(parcBuffer_Position(outerDecoder->buffer) == 0,
               "Did not advance buffer to right spot, expected %u got %zu",
               0, parcBuffer_Position(outerDecoder->buffer));

    assertTrue(type == 1, "Wrong type expected %u got %u", 1, type);

    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetValue)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    uint16_t type = ccnxCodecTlvDecoder_GetType(outerDecoder);
    uint16_t length = ccnxCodecTlvDecoder_GetLength(outerDecoder);

    assertTrue(type == 1, "Wrong type expected %u got %u", 1, type);
    assertTrue(length == 19, "Wrong length expected %u got %u", 19, length);

    PARCBuffer *inner = ccnxCodecTlvDecoder_GetValue(outerDecoder, length);

    // inner should now be empty
    assertTrue(ccnxCodecTlvDecoder_IsEmpty(outerDecoder), "outer decoder should be emtpy");

    CCNxCodecTlvDecoder *innerDecoder = ccnxCodecTlvDecoder_Create(inner);
    parcBuffer_Release(&inner);

    type = ccnxCodecTlvDecoder_GetType(innerDecoder);
    length = ccnxCodecTlvDecoder_GetLength(innerDecoder);

    assertTrue(type == 2, "Wrong type expected %u got %u", 2, type);
    assertTrue(length == 5, "Wrong length expected %u got %u", 5, length);

    PARCBuffer *hello = ccnxCodecTlvDecoder_GetValue(innerDecoder, length);

    parcBuffer_Release(&hello);
    ccnxCodecTlvDecoder_Destroy(&innerDecoder);
    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetValue_TooLong)
{
    // Length is beyond end of buffer
    uint8_t truthBytes[] = { 0x00, 0x02, 0x00, 0x99, 'h', 'e', 'l', 'l', 'o' };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    (void) ccnxCodecTlvDecoder_GetType(outerDecoder);
    uint16_t length = ccnxCodecTlvDecoder_GetLength(outerDecoder);
    PARCBuffer *value = ccnxCodecTlvDecoder_GetValue(outerDecoder, length);

    assertNull(value, "Value should be null because of buffer underrun");

    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_IsEmpty_True)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCBuffer *value = ccnxCodecTlvDecoder_GetValue(outerDecoder, sizeof(truthBytes));
    parcBuffer_Release(&value);

    assertTrue(ccnxCodecTlvDecoder_IsEmpty(outerDecoder), "Decoder said it was not empty when its should be empty");
    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_IsEmpty_False)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    assertFalse(ccnxCodecTlvDecoder_IsEmpty(outerDecoder), "Decoder said it was empty when its full");
    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Position)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCBuffer *value = ccnxCodecTlvDecoder_GetValue(outerDecoder, 8);
    parcBuffer_Release(&value);

    assertTrue(ccnxCodecTlvDecoder_Position(outerDecoder) == 8,
               "Decoder reports wrong position, expected %u got %zu",
               8,
               ccnxCodecTlvDecoder_Position(outerDecoder));

    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_EnsureRemaining_True)
{
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    bool success = ccnxCodecTlvDecoder_EnsureRemaining(outerDecoder, 5);

    ccnxCodecTlvDecoder_Destroy(&outerDecoder);

    assertTrue(success,
               "Decoder failed ensureRemaining check for 5 bytes when its a 19 byte buffer");
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_EnsureRemaining_False)
{
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    bool success = ccnxCodecTlvDecoder_EnsureRemaining(outerDecoder, 24);

    ccnxCodecTlvDecoder_Destroy(&outerDecoder);

    assertFalse(success,
                "Decoder passed ensureRemaining check for 24 bytes when its a 19 byte buffer");
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_Good)
{
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x20, 0x00, 0x01, 0xFF }, 5, 0, 5);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint8_t value;
    bool success = ccnxCodecTlvDecoder_GetUint8(decoder, 0x1020, &value);
    assertTrue(success, "Did not decode a correct buffer");
    assertTrue(value == 0xFF, "Incorrect value, expected 0x%X got 0x%X", 0xFF, value);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_Short)
{
    // LIMIT IS SHORT
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x20, 0x00, 0x01, 0xFF }, 5, 0, 4);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint8_t value;
    bool success = ccnxCodecTlvDecoder_GetUint8(decoder, 0x1020, &value);
    assertFalse(success, "Should have failed a short buffer");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_WrongType)
{
    // TYPE IS WRONG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0xFF, 0xFF, 0x00, 0x01, 0xFF }, 5, 0, 5);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint8_t value;
    bool success = ccnxCodecTlvDecoder_GetUint8(decoder, 0x1020, &value);
    assertFalse(success, "Should have failed because of wrong type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint8_WrongLength)
{
    // LENGTH TOO BIG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x20, 0x00, 0x99, 0xFF }, 5, 0, 5);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint8_t value;
    bool success = ccnxCodecTlvDecoder_GetUint8(decoder, 0x1020, &value);
    assertFalse(success, "Should have failed because of incorrect length");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_Good)
{
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x21, 0x00, 0x02, 0xFF, 0x01 }, 6, 0, 6);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint16_t value;
    bool success = ccnxCodecTlvDecoder_GetUint16(decoder, 0x1021, &value);
    assertTrue(success, "Did not decode a correct buffer");
    assertTrue(value == 0xFF01, "Incorrect value, expected 0x%X got 0x%X", 0xFF01, value);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_Short)
{
    // LIMIT IS SHORT
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x21, 0x00, 0x02, 0xFF, 0x01 }, 6, 0, 5);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint16_t value;
    bool success = ccnxCodecTlvDecoder_GetUint16(decoder, 0x1021, &value);
    assertFalse(success, "Should have failed because of wrong type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_WrongType)
{
    // TYPE IS WRONG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0xFF, 0xFF, 0x00, 0x02, 0xFF, 0x01 }, 6, 0, 6);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint16_t value;
    bool success = ccnxCodecTlvDecoder_GetUint16(decoder, 0x1021, &value);
    assertFalse(success, "Should have failed because of wrong type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}
LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint16_WrongLength)
{
    // LENGTH TOO BIG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x21, 0x00, 0x99, 0xFF }, 5, 0, 5);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint16_t value;
    bool success = ccnxCodecTlvDecoder_GetUint16(decoder, 0x1021, &value);
    assertFalse(success, "Should have failed because of incorrect length");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_Good)
{
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x22, 0x00, 0x04, 0xFF, 0x01, 0x02, 0x03 }, 8, 0, 8);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint32_t value;
    bool success = ccnxCodecTlvDecoder_GetUint32(decoder, 0x1022, &value);
    assertTrue(success, "Did not decode a correct buffer");
    assertTrue(value == 0xFF010203, "Incorrect value, expected 0x%X got 0x%X", 0xFF010203, value);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_Short)
{
    // LIMIT IS SHORT
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x22, 0x00, 0x04, 0xFF, 0x01, 0x02, 0x03 }, 8, 0, 7);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint32_t value;
    bool success = ccnxCodecTlvDecoder_GetUint32(decoder, 0x1022, &value);
    assertFalse(success, "Should have failed because of wrong type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_WrongType)
{
    // TYPE IS WRONG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0xFF, 0xFF, 0x00, 0x04, 0xFF, 0x01, 0x02, 0x03 }, 8, 0, 8);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint32_t value;
    bool success = ccnxCodecTlvDecoder_GetUint32(decoder, 0x1022, &value);
    assertFalse(success, "Should have failed because of wrong type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint32_WrongLength)
{
    // LENGTH TOO BIG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x22, 0x00, 0x99, 0xFF, 0x01, 0x02, 0x03 }, 8, 0, 8);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint32_t value;
    bool success = ccnxCodecTlvDecoder_GetUint32(decoder, 0x1022, &value);
    assertFalse(success, "Should have failed because of incorrect length");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_Good)
{
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00, 0x08, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }, 12, 0, 12);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint64_t value;
    uint64_t truth = 0xFF01020304050607ULL;
    bool success = ccnxCodecTlvDecoder_GetUint64(decoder, 0x1023, &value);
    assertTrue(success, "Did not decode a correct buffer");
    assertTrue(value == truth, "Incorrect value, expected %#" PRIx64 " got %#" PRIx64, truth, value);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_Short)
{
    // LIMIT IS SHORT
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00, 0x08, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }, 12, 0, 11);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint64_t value;
    bool success = ccnxCodecTlvDecoder_GetUint64(decoder, 0x1023, &value);
    assertFalse(success, "Should have failed because of wrong type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_WrongType)
{
    // TYPE IS WRONG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0xFF, 0xFF, 0x00, 0x08, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }, 12, 0, 11);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint64_t value;
    bool success = ccnxCodecTlvDecoder_GetUint64(decoder, 0x1023, &value);
    assertFalse(success, "Should have failed because of wrong type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetUint64_WrongLength)
{
    // LENGTH TOO BIG
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00, 0x99, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }, 12, 0, 12);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    uint64_t value;
    bool success = ccnxCodecTlvDecoder_GetUint64(decoder, 0x1023, &value);
    assertFalse(success, "Should have failed because of incorrect length");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_Good)
{
    PARCBuffer *truth = parcBuffer_Wrap((uint8_t[]) { 0x01, 0x02, 0x03, 0x04 }, 4, 0, 4);
    PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) { 0x00, 0x01, 0x00, 0x08, 0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04 }, 12, 0, 12);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);

    (void) ccnxCodecTlvDecoder_GetType(decoder);
    (void) ccnxCodecTlvDecoder_GetLength(decoder);

    PARCBuffer *test = ccnxCodecTlvDecoder_GetBuffer(decoder, 0xAABB);
    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers not equal\n");
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers not equal");
    }

    parcBuffer_Release(&test);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&input);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_WrongType)
{
    // INNER TYPE IS WRONG
    PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) { 0x00, 0x01, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04 }, 12, 0, 12);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);

    (void) ccnxCodecTlvDecoder_GetType(decoder);
    (void) ccnxCodecTlvDecoder_GetLength(decoder);

    PARCBuffer *test = ccnxCodecTlvDecoder_GetBuffer(decoder, 0xAABB);
    assertNull(test, "Should have returned NULL because of incorrect TLV type");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&input);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_TooShort)
{
    // OVERALL LENGTH TOO SHORT TO PARSE
    // buffer only goes to here                      ---------------------------------!
    PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) { 0x00, 0x01, 0x00, 0x08, 0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04 }, 12, 0, 6);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);

    (void) ccnxCodecTlvDecoder_GetType(decoder);
    (void) ccnxCodecTlvDecoder_GetLength(decoder);

    PARCBuffer *test = ccnxCodecTlvDecoder_GetBuffer(decoder, 0xAABB);
    assertNull(test, "Should have returned NULL because of input underrun");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&input);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetBuffer_TooLong)
{
    // VALUE (4 bytes) SHORTER THAN LENGTH (0x99)
    PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) { 0x00, 0x01, 0x00, 0x08, 0xAA, 0xBB, 0x00, 0x99, 0x01, 0x02, 0x03, 0x04 }, 12, 0, 12);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);

    (void) ccnxCodecTlvDecoder_GetType(decoder);
    (void) ccnxCodecTlvDecoder_GetLength(decoder);

    PARCBuffer *test = ccnxCodecTlvDecoder_GetBuffer(decoder, 0xAABB);
    assertNull(test, "Should have returned NULL because of value underrun");
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&input);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetContainer)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    (void) ccnxCodecTlvDecoder_GetType(outerDecoder);
    uint16_t length = ccnxCodecTlvDecoder_GetLength(outerDecoder);
    CCNxCodecTlvDecoder *innerDecoder = ccnxCodecTlvDecoder_GetContainer(outerDecoder, length);
    assertNotNull(innerDecoder, "Got a null decoder for a valid slice");
    assertTrue(ccnxCodecTlvDecoder_Position(innerDecoder) == 0, "Wrong position, expected 0 got %zu", ccnxCodecTlvDecoder_Position(innerDecoder));
    assertTrue(ccnxCodecTlvDecoder_EnsureRemaining(innerDecoder, 19), "Inner decoder does not have enough bytes in it");

    ccnxCodecTlvDecoder_Destroy(&innerDecoder);
    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetContainer_TooLong)
{
    /**
     * We will create a TLV structure that looks like this:
     *   { T = 1,  L = 19 },
     *      { T = 2, L = 5, V = "hello" }
     *      { T = 3, L = 6, V = "mr tlv" }
     */
    uint8_t truthBytes[] = {
        0x00, 0x01, 0x00, 0x13,
        0x00, 0x02, 0x00, 0x05,'h',  'e', 'l', 'l', 'o',
        0x00, 0x03, 0x00, 0x06,'m',  'r', ' ', 't', 'l', 'v'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvDecoder *outerDecoder = ccnxCodecTlvDecoder_Create(buffer);
    parcBuffer_Release(&buffer);

    (void) ccnxCodecTlvDecoder_GetType(outerDecoder);
    (void) ccnxCodecTlvDecoder_GetLength(outerDecoder);
    // ask for too many bytes
    CCNxCodecTlvDecoder *innerDecoder = ccnxCodecTlvDecoder_GetContainer(outerDecoder, 100);
    assertNull(innerDecoder, "Got a decoder for an invalid slice");
    ccnxCodecTlvDecoder_Destroy(&outerDecoder);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Advance_Good)
{
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0xFF, 0xFF, 0x00, 0x04, 0xFF, 0x01, 0x02, 0x03 }, 8, 0, 8);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);

    size_t advance = 3;
    size_t beforePosition = ccnxCodecTlvDecoder_Position(decoder);
    bool success = ccnxCodecTlvDecoder_Advance(decoder, advance);
    size_t afterPosition = ccnxCodecTlvDecoder_Position(decoder);

    assertTrue(success, "Failed to advance decoder");
    assertTrue(beforePosition + advance == afterPosition, "Wrong position, got %zu expected %zu", afterPosition, beforePosition + advance);

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_Advance_TooLong)
{
    PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0xFF, 0xFF, 0x00, 0x04, 0xFF, 0x01, 0x02, 0x03 }, 8, 0, 8);
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);

    size_t advance = 9;
    size_t beforePosition = ccnxCodecTlvDecoder_Position(decoder);
    bool success = ccnxCodecTlvDecoder_Advance(decoder, advance);
    size_t afterPosition = ccnxCodecTlvDecoder_Position(decoder);

    assertFalse(success, "Should have returned false advancing beyond end of decoder");
    assertTrue(beforePosition == afterPosition, "Wrong position, got %zu expected %zu", afterPosition, beforePosition);

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Decoder, ccnxCodecTlvDecoder_GetVarInt)
{
    struct test_vector {
        uint64_t value;
        bool valid;
        int length;
        uint8_t *array;
    } vectors[] = {
        // length 0 invalid
        { .value = 0,                     .valid = false, .length = 0, .array = (uint8_t[]) { 0x00 } },
        { .value = 0,                     .valid = true,  .length = 1, .array = (uint8_t[]) { 0x00 } },
        { .value = 0xFF,                  .valid = true,  .length = 1, .array = (uint8_t[]) { 0xFF } },
        { .value = 0x0001,                .valid = true,  .length = 2, .array = (uint8_t[]) { 0x00, 0x01} },
        { .value = 0xFF01,                .valid = true,  .length = 2, .array = (uint8_t[]) { 0xFF, 0x01} },
        { .value = 0x000001,              .valid = true,  .length = 3, .array = (uint8_t[]) { 0x00, 0x00, 0x01} },
        { .value = 0xFF0001,              .valid = true,  .length = 3, .array = (uint8_t[]) { 0xFF, 0x00, 0x01} },
        { .value = 0x00000001,            .valid = true,  .length = 4, .array = (uint8_t[]) { 0x00, 0x00, 0x00, 0x01} },
        { .value = 0xFF002001,            .valid = true,  .length = 4, .array = (uint8_t[]) { 0xFF, 0x00, 0x20, 0x01} },
        { .value = 0xFF00200103040506ULL, .valid = true,  .length = 8, .array = (uint8_t[]) { 0xFF, 0x00, 0x20, 0x01, 0x03, 0x04, 0x05, 0x06} },
        // length 9 invalid
        { .value = 0,                     .valid = false, .length = 9, .array = (uint8_t[]) { 0xFF, 0x00, 0x20, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07} },
        // sentinal is NULL array
        { .value = 0,                     .valid = false, .length = 0, .array = NULL },
    };

    for (int i = 0; vectors[i].array != NULL; i++) {
        PARCBuffer *buffer = parcBuffer_Wrap(vectors[i].array, vectors[i].length, 0, vectors[i].length);
        CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);

        uint64_t value;
        bool success = ccnxCodecTlvDecoder_GetVarInt(decoder, vectors[i].length, &value);
        ccnxCodecTlvDecoder_Destroy(&decoder);
        parcBuffer_Release(&buffer);

        assertTrue(success == vectors[i].valid, "index %d: Wrong return, got %d expected %d", i, success, vectors[i].valid);
        if (vectors[i].valid) {
            assertTrue(value == vectors[i].value, "index %d: wrong value: got %" PRIu64 " expected %" PRIu64, i, value, vectors[i].value);
        }
    }
}

// ============================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Tlv);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
