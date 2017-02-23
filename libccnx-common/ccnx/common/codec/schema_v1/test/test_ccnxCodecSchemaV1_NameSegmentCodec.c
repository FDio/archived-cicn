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
#include "../ccnxCodecSchemaV1_NameSegmentCodec.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>

LONGBOW_TEST_RUNNER(ccnxTlvCodec_Name)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxTlvCodec_Name)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxTlvCodec_Name)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecNameSegment_Decode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecNameSegment_Decode_TLShort);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecNameSegment_Decode_VShort);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecNameSegment_Decode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecNameSegment_Encode);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecNameSegment_Decode)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("brandywine");
    CCNxNameSegment *truth = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buffer);
    parcBuffer_Release(&buffer);

    uint8_t decodeBytes[] = { 0x00, CCNxNameLabelType_NAME, 0x00, 0x0A, 'b', 'r', 'a', 'n', 'd', 'y', 'w', 'i', 'n', 'e' };
    PARCBuffer *decodeBuffer = parcBuffer_Wrap(decodeBytes, sizeof(decodeBytes), 0, sizeof(decodeBytes));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(decodeBuffer);
    CCNxNameSegment *test = ccnxCodecSchemaV1NameSegmentCodec_Decode(decoder);

    assertTrue(ccnxNameSegment_Equals(truth, test), "Name segments do not match");

    ccnxNameSegment_Release(&test);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&decodeBuffer);
    ccnxNameSegment_Release(&truth);
}

/**
 * In this case, there are not enough bytes in the buffer to decode the T and L
 */
LONGBOW_TEST_CASE(Global, ccnxTlvCodecNameSegment_Decode_TLShort)
{
    uint8_t decodeBytes[] = { 0x00, CCNxNameLabelType_NAME, 0x00, 0x0A, 'b', 'r', 'a' };
    PARCBuffer *decodeBuffer = parcBuffer_Wrap(decodeBytes, sizeof(decodeBytes), 0, sizeof(decodeBytes));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(decodeBuffer);
    CCNxNameSegment *test = ccnxCodecSchemaV1NameSegmentCodec_Decode(decoder);

    assertNull(test, "Name segments should have been null because there are not enough bytes in buffer");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&decodeBuffer);
}

/**
 * In this case, we decode the T and L, but there are not enough bytes for the V
 */
LONGBOW_TEST_CASE(Global, ccnxTlvCodecNameSegment_Decode_VShort)
{
    uint8_t decodeBytes[] = { 0x00, CCNxNameLabelType_NAME, 0x00 };
    PARCBuffer *decodeBuffer = parcBuffer_Wrap(decodeBytes, sizeof(decodeBytes), 0, sizeof(decodeBytes));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(decodeBuffer);
    CCNxNameSegment *test = ccnxCodecSchemaV1NameSegmentCodec_Decode(decoder);

    assertNull(test, "Name segments should have been null because there are not enough bytes in buffer");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&decodeBuffer);
}


LONGBOW_TEST_CASE(Global, ccnxTlvCodecNameSegment_Encode)
{
    uint8_t truthBytes[] = { 0x00, CCNxNameLabelType_NAME, 0x00, 0x0A, 'b', 'r', 'a', 'n', 'd', 'y', 'w', 'i', 'n', 'e' };
    PARCBuffer *truth = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);

    PARCBuffer *buffer = parcBuffer_WrapCString("brandywine");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buffer);
    parcBuffer_Release(&buffer);

    ccnxCodecSchemaV1NameSegmentCodec_Encode(encoder, segment);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "Buffers do not match");

    parcBuffer_Release(&test);
    ccnxNameSegment_Release(&segment);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxTlvCodec_Name);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
