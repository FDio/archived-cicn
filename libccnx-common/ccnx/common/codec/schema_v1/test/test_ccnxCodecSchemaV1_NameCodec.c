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
#include "../ccnxCodecSchemaV1_NameCodec.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

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
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecName_Decode_RightType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecName_Decode_WrongType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecName_Encode);
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

LONGBOW_TEST_CASE(Global, ccnxTlvCodecName_Decode_RightType)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("brandywine");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buffer);
    CCNxName *truth = ccnxName_Append(ccnxName_Create(), segment);
    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buffer);

    uint8_t decodeBytes[] = { 0x10, 0x20, 0x00, 0x0E, 0x00, CCNxNameLabelType_NAME, 0x00, 0x0A, 'b', 'r', 'a', 'n', 'd', 'y', 'w', 'i', 'n', 'e' };
    PARCBuffer *decodeBuffer = parcBuffer_Wrap(decodeBytes, sizeof(decodeBytes), 0, sizeof(decodeBytes));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(decodeBuffer);
    CCNxName *test = ccnxCodecSchemaV1NameCodec_Decode(decoder, 0x1020);

    assertTrue(ccnxName_Equals(truth, test), "Name segments do not match");

    ccnxName_Release(&test);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&decodeBuffer);
    ccnxName_Release(&truth);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecName_Decode_WrongType)
{
    uint8_t decodeBytes[] = { 0x10, 0x20, 0x00, 0x0E, 0x00, CCNxNameLabelType_NAME, 0x00, 0x0A, 'b', 'r', 'a', 'n', 'd', 'y', 'w', 'i', 'n', 'e' };
    PARCBuffer *decodeBuffer = parcBuffer_Wrap(decodeBytes, sizeof(decodeBytes), 0, sizeof(decodeBytes));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(decodeBuffer);
    CCNxName *test = ccnxCodecSchemaV1NameCodec_Decode(decoder, 0xFFFF);

    assertNull(test, "Name should have returned NULL because the name type does not match");
    assertTrue(ccnxCodecTlvDecoder_Position(decoder) == 0, "Position should not have moved, expected 0, got %zu", ccnxCodecTlvDecoder_Position(decoder));

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&decodeBuffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecName_Encode)
{
    uint8_t truthBytes[] = { 0x10, 0x20, 0x00, 0x0E, 0x00, CCNxNameLabelType_NAME, 0x00, 0x0A, 'b', 'r', 'a', 'n', 'd', 'y', 'w', 'i', 'n', 'e' };
    PARCBuffer *truth = parcBuffer_Wrap(truthBytes, sizeof(truthBytes), 0, sizeof(truthBytes));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(encoder);

    PARCBuffer *buffer = parcBuffer_WrapCString("brandywine");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buffer);

    CCNxName *name = ccnxName_Append(ccnxName_Create(), segment);
    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buffer);

    ccnxCodecSchemaV1NameCodec_Encode(encoder, 0x1020, name);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    if (!parcBuffer_Equals(truth, test)) {
        printf("Buffers do not match\n");
        printf("Excpected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
        assertTrue(parcBuffer_Equals(truth, test), "Buffers do not match");
    }

    ccnxName_Release(&name);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxTlvCodec_Name);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
