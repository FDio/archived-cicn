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
#include "../ccnxCodecSchemaV1_LinkCodec.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <ccnx/common/codec/ccnxCodec_Error.h>

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
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_NameOnly);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_AllFields);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_NoName);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_ExtraField);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_DupName);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_DupKeyId);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_DupHash);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_FieldOverrun);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_Underrun);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1LinkCodec_Encode);
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

LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_NameOnly)
{
    CCNxName *truth = ccnxName_CreateFromCString("lci:/3=rope");

    uint8_t encoded[] = {
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNotNull(link, "got null link: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    const CCNxName *test = ccnxLink_GetName(link);
    assertTrue(ccnxName_Equals(truth, test), "Wrong name")
    {
        printf("Expected\n");
        ccnxName_Display(truth, 3);
        printf("Got\n");
        ccnxName_Display(test, 3);
    }

    PARCBuffer *testKeyid = ccnxLink_GetKeyID(link);
    assertNull(testKeyid, "Got a keyid without the wire encoding for it");

    PARCBuffer *testHash = ccnxLink_GetContentObjectHash(link);
    assertNull(testHash, "got a hash without the wire encoding for it");

    ccnxLink_Release(&link);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
    ccnxName_Release(&truth);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_AllFields)
{
    CCNxName *truth = ccnxName_CreateFromCString("lci:/3=rope");

    uint8_t encoded[] = {
        // -- name
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e',
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNotNull(link, "got null link: %s", ccnxCodecError_ToString(ccnxCodecTlvDecoder_GetError(decoder)));

    const CCNxName *test = ccnxLink_GetName(link);
    assertTrue(ccnxName_Equals(truth, test), "Wrong name")
    {
        printf("Expected\n");
        ccnxName_Display(truth, 3);
        printf("Got\n");
        ccnxName_Display(test, 3);
    }

    PARCBuffer *keyid = parcBuffer_Wrap(encoded, sizeof(encoded), 16, 24);
    PARCBuffer *testKeyid = ccnxLink_GetKeyID(link);
    assertTrue(parcBuffer_Equals(keyid, testKeyid), "Wrong keyid")
    {
        printf("Expected\n");
        parcBuffer_Display(keyid, 3);
        printf("Got\n");
        parcBuffer_Display(testKeyid, 3);
    }

    PARCBuffer *hash = parcBuffer_Wrap(encoded, sizeof(encoded), 28, 44);
    PARCBuffer *testHash = ccnxLink_GetContentObjectHash(link);
    assertTrue(parcBuffer_Equals(hash, testHash), "Wrong hash")
    {
        printf("Expected\n");
        parcBuffer_Display(hash, 3);
        printf("Got\n");
        parcBuffer_Display(testHash, 3);
    }

    parcBuffer_Release(&hash);
    parcBuffer_Release(&keyid);

    ccnxLink_Release(&link);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
    ccnxName_Release(&truth);
}

/*
 * wire format missing name
 */
LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_NoName)
{
    uint8_t encoded[] = {
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(link, "got non-null link when it should have been an error (null)");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNotNull(error, "Got null error when it should have been set");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

/*
 * Wire format has an extra TLV in it that's not part of the spec
 */
LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_ExtraField)
{
    uint8_t encoded[] = {
        // -- name
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e',
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
        // -- extra
        0x00, 0xFF, 0x00, 4,
        0xc0, 0xc1, 0xc2, 0xc3,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(link, "got non-null link when it should have been an error (null)");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNotNull(error, "Got null error when it should have been set");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_DupName)
{
    uint8_t encoded[] = {
        // -- name
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e',
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
        // -- name
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e',
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(link, "got non-null link when it should have been an error (null)");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNotNull(error, "Got null error when it should have been set");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_DupKeyId)
{
    uint8_t encoded[] = {
        // -- name
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e',
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(link, "got non-null link when it should have been an error (null)");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNotNull(error, "Got null error when it should have been set");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_DupHash)
{
    uint8_t encoded[] = {
        // -- name
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e',
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(link, "got non-null link when it should have been an error (null)");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNotNull(error, "Got null error when it should have been set");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_FieldOverrun)
{
    // name length is beyond end of fragment
    uint8_t encoded[] = {
        0x00, 0x00, 0x00, 30,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e'
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(link, "got non-null link when it should have been an error (null)");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNotNull(error, "Got null error when it should have been set");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxTlvCodecLink_DecodeValue_Underrun)
{
    // buffer is too short to even parse the T and L
    uint8_t encoded[] = {
        0x00, 0x00, 0x00, 0x00, 0x00
    };

    // the "2" makes it too short to parse
    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, 2);

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, sizeof(encoded));
    assertNull(link, "got non-null link when it should have been an error (null)");

    CCNxCodecError *error = ccnxCodecTlvDecoder_GetError(decoder);
    assertNotNull(error, "Got null error when it should have been set");

    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&buffer);
}

// ============

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1LinkCodec_Encode)
{
    uint8_t encoded[] = {
        // -- name
        0x00, 0x00, 0x00, 8,
        0x00, 0x03, 0x00, 4,
        'r',  'o',  'p',  'e',
        // -- keyid
        0x00, 0x01, 0x00, 8,
        0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7,
        // -- hash
        0x00, 0x02, 0x00, 16,
        0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb,
        0xbc, 0xbd, 0xbe, 0xbf,
    };

    CCNxName *trueName = ccnxName_CreateFromCString("lci:/3=rope");
    PARCBuffer *trueKeyId = parcBuffer_Wrap(encoded, sizeof(encoded), 16, 24);
    PARCBuffer *trueHash = parcBuffer_Wrap(encoded, sizeof(encoded), 28, 44);
    PARCBuffer *trueEncoding = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxLink *link = ccnxLink_Create(trueName, trueKeyId, trueHash);

    // now encode it and compare the the trueEncoding
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    ssize_t length = ccnxCodecSchemaV1LinkCodec_Encode(encoder, link);
    assertFalse(length < 0, "Got error on encode: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zd got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *testEncoding = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(trueEncoding, testEncoding), "Wrong hash")
    {
        printf("Expected\n");
        parcBuffer_Display(trueEncoding, 3);
        printf("Got\n");
        parcBuffer_Display(testEncoding, 3);
    }

    parcBuffer_Release(&testEncoding);
    parcBuffer_Release(&trueEncoding);
    parcBuffer_Release(&trueHash);
    parcBuffer_Release(&trueKeyId);
    ccnxName_Release(&trueName);

    ccnxLink_Release(&link);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxTlvCodec_Name);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
