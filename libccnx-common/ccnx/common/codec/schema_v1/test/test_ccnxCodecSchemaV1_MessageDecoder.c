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
#include "../ccnxCodecSchemaV1_MessageDecoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_keyid1_rsasha256.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_all_fields.h>

#include "testrig_packetwrapper.c"


// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_MessageDecoder)
{
    LONGBOW_RUN_TEST_FIXTURE(ContentObject);
    LONGBOW_RUN_TEST_FIXTURE(Interest);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_MessageDecoder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_MessageDecoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// =========================================================================
// Utility functions to get a field in the format we expect for the testrig.
// We cannot use the facades because those try to assert a type on the dictionary which
// is not part of the MessageDecoder.

static CCNxName *
_getName(CCNxTlvDictionary *contentObjectDictionary)
{
    return ccnxTlvDictionary_GetName(contentObjectDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
}

static PARCBuffer *
_getPayload(const CCNxTlvDictionary *contentObjectDictionary)
{
    return ccnxTlvDictionary_GetBuffer(contentObjectDictionary,
                                       CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
}

static int64_t
_getPayloadType(CCNxTlvDictionary *contentObjectDictionary)
{
    return (int64_t) ccnxTlvDictionary_GetInteger(contentObjectDictionary,
                                                  CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE);
}

static int64_t
_getExpiryTime(CCNxTlvDictionary *contentObjectDictionary)
{
    return (int64_t) ccnxTlvDictionary_GetInteger(contentObjectDictionary,
                                                  CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME);
}

static int64_t
_getEndChunkNumber(CCNxTlvDictionary *contentObjectDictionary)
{
    return (int64_t) ccnxTlvDictionary_GetInteger(contentObjectDictionary,
                                                  CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT);
}

static PARCCryptoHash *
_getKeyIdRestriction(CCNxTlvDictionary *messageDictionary)
{
    return (PARCCryptoHash *) ccnxTlvDictionary_GetObject(messageDictionary,
                                                          CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION);
}

static PARCCryptoHash *
_getHashRestriction(CCNxTlvDictionary *messageDictionary)
{
    return (PARCCryptoHash *) ccnxTlvDictionary_GetObject(messageDictionary,
                                                          CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION);
}


// =========================================================================

LONGBOW_TEST_FIXTURE(ContentObject)
{
    LONGBOW_RUN_TEST_CASE(ContentObject, Name);
    LONGBOW_RUN_TEST_CASE(ContentObject, Payload);
    LONGBOW_RUN_TEST_CASE(ContentObject, PayloadType);
    LONGBOW_RUN_TEST_CASE(ContentObject, ExpiryTime);
    LONGBOW_RUN_TEST_CASE(ContentObject, EndChunkNumber);
}

LONGBOW_TEST_FIXTURE_SETUP(ContentObject)
{
    longBowTestCase_SetClipBoardData(testCase,
                                     commonSetup(v1_content_nameA_keyid1_rsasha256,
                                                 sizeof(v1_content_nameA_keyid1_rsasha256),
                                                 v1_content_nameA_keyid1_rsasha256_truthTableEntries,
                                                 V1_MANIFEST_OBJ_CONTENTOBJECT));

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(ContentObject)
{
    commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(ContentObject, Name)
{
//    TestData *data = longBowTestCase_GetClipBoardData(testCase);
//    testNameGetter(data, V1_MANIFEST_OBJ_NAME, ccnxCodecSchemaV1MessageDecoder_Decode, _getName);
}

LONGBOW_TEST_CASE(ContentObject, Payload)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    testBufferGetter(data, V1_MANIFEST_OBJ_PAYLOAD, ccnxCodecSchemaV1MessageDecoder_Decode, _getPayload);
}

LONGBOW_TEST_CASE(ContentObject, PayloadType)
{
    // The payload type is translated from the wire format value to the CCNxPayloadType value,
    // so this test cannot use the automated framework as the value in the dictionary will not
    // be the same as the value in the wire format

    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    bool decodeSuccess = ccnxCodecSchemaV1MessageDecoder_Decode(data->decoder, data->dictionary);
    assertTrue(decodeSuccess, "failure on ccnxCodecSchemaV1MessageDecoder_Decode");

    CCNxPayloadType testPayloadType = (CCNxPayloadType) _getPayloadType(data->dictionary);

    // look up the true name buffer from the truth table
    TlvExtent extent = getTruthTableExtent(data->truthTable, V1_MANIFEST_OBJ_PAYLOADTYPE);

    PARCBuffer
    *truthbuffer = parcBuffer_Wrap(data->packet, data->packetLength, extent.offset, extent.offset + extent.length);
    uint64_t truthvalue = -2;
    ccnxCodecTlvUtilities_GetVarInt(truthbuffer, parcBuffer_Remaining(truthbuffer), &truthvalue);

    CCNxPayloadType truthPayloadType = -1;
    bool success =
        _translateWirePayloadTypeToCCNxPayloadType((CCNxCodecSchemaV1Types_PayloadType) truthvalue, &truthPayloadType);
    assertTrue(success, "failure in _translateWirePayloadTypeToCCNxPayloadType for wireFormatValue %"
               PRId64, truthvalue);

    parcBuffer_Release(&truthbuffer);

    assertTrue(truthPayloadType == testPayloadType, "Wrong value, got %d expected %d", testPayloadType,
               truthPayloadType);
}

LONGBOW_TEST_CASE(ContentObject, ExpiryTime)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    testInt64Getter(data, V1_MANIFEST_OBJ_EXPIRY_TIME, ccnxCodecSchemaV1MessageDecoder_Decode, _getExpiryTime);
}

LONGBOW_TEST_CASE(ContentObject, EndChunkNumber)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    testInt64Getter(data, V1_MANIFEST_OBJ_ENDSEGMENT, ccnxCodecSchemaV1MessageDecoder_Decode, _getEndChunkNumber);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(Interest)
{
    LONGBOW_RUN_TEST_CASE(Interest, Name);
    LONGBOW_RUN_TEST_CASE(Interest, Payload);
    LONGBOW_RUN_TEST_CASE(Interest, KeyIdRestriction);
    LONGBOW_RUN_TEST_CASE(Interest, HashRestriction);
}

LONGBOW_TEST_FIXTURE_SETUP(Interest)
{
    longBowTestCase_SetClipBoardData(testCase,
                                     commonSetup(v1_interest_all_fields,
                                                 sizeof(v1_interest_all_fields),
                                                 TRUTHTABLENAME(v1_interest_all_fields),
                                                 V1_MANIFEST_INT_INTEREST));
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Interest)
{
    commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Interest, Name)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    testNameGetter(data, V1_MANIFEST_INT_NAME, ccnxCodecSchemaV1MessageDecoder_Decode, _getName);
}

LONGBOW_TEST_CASE(Interest, Payload)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    testBufferGetter(data, V1_MANIFEST_INT_PAYLOAD, ccnxCodecSchemaV1MessageDecoder_Decode, _getPayload);
}

LONGBOW_TEST_CASE(Interest, KeyIdRestriction)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    testHashGetter(data, V1_MANIFEST_INT_KEYID, ccnxCodecSchemaV1MessageDecoder_Decode, _getKeyIdRestriction);
}

LONGBOW_TEST_CASE(Interest, HashRestriction)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    testHashGetter(data, V1_MANIFEST_INT_OBJHASH, ccnxCodecSchemaV1MessageDecoder_Decode, _getHashRestriction);
}


// =========================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _translateWirePayloadTypeToCCNxPayloadType);
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

LONGBOW_TEST_CASE(Local, _translateWirePayloadTypeToCCNxPayloadType)
{
    uint32_t sentinel = 0xFFFF;

    struct {
        CCNxCodecSchemaV1Types_PayloadType wire;
        CCNxPayloadType payloadType;
        bool success;
    } vectors[] = {
        { .wire = CCNxCodecSchemaV1Types_PayloadType_Data, .payloadType = CCNxPayloadType_DATA, .success = true  },
        { .wire = CCNxCodecSchemaV1Types_PayloadType_Key,  .payloadType = CCNxPayloadType_KEY,  .success = true  },
        { .wire = CCNxCodecSchemaV1Types_PayloadType_Link, .payloadType = CCNxPayloadType_LINK, .success = true  },
        { .wire = -2,                                      .payloadType = -2,                   .success = false },
        { .wire = sentinel,                                .payloadType = sentinel }
    };

    for (int i = 0; vectors[i].wire != sentinel; i++) {
        CCNxPayloadType payloadType = -1;
        bool success = _translateWirePayloadTypeToCCNxPayloadType(vectors[i].wire, &payloadType);
        assertTrue(success == vectors[i].success, "Incorrect return index %d, expected %d got %d", i,
                   vectors[i].success, success);

        if (success) {
            assertTrue(payloadType == vectors[i].payloadType, "Wrong payloadType index %d, expected %d got %d", i,
                       vectors[i].payloadType, payloadType);
        }
    }
}

// =========================================================================


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_MessageDecoder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
