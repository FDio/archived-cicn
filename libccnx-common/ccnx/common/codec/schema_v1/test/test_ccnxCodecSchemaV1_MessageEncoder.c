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
#include "../ccnxCodecSchemaV1_MessageEncoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_keyid1_rsasha256.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_all_fields.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameless_nosig.h>

#include "testrig_encoder.c"

#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/ccnx_Interest.h>

#include <ccnx/common/internal/ccnx_InterestDefault.h>

// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_MessageEncoder)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(UnknownType);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_MessageEncoder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_MessageEncoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


// =========================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(ContentObject, Interest);
    LONGBOW_RUN_TEST_CASE(ContentObject, ContentObject);
    LONGBOW_RUN_TEST_CASE(ContentObject, ContentObjectNameless);
    LONGBOW_RUN_TEST_CASE(InterestReturn, InterestReturn);
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

LONGBOW_TEST_CASE(ContentObject, Interest)
{
    // create an Interest that replicates v1_interest_all_fields
    TlvExtent keyidExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_KEYID);
    TlvExtent hashExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_OBJHASH);
    TlvExtent payloadExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_PAYLOAD);
    TlvExtent interestExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_INTEREST);

    CCNxName *name = ccnxName_CreateFromCString("lci:/3=cool");
    uint32_t lifetime = CCNxInterestDefault_LifetimeMilliseconds;
    uint32_t hoplimit = CCNxInterestDefault_HopLimit;

    printf("%d %d\n", keyidExtent.offset + 4, keyidExtent.offset + keyidExtent.length - 4);
    PARCBuffer *keyid = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), keyidExtent.offset + 4, keyidExtent.offset + keyidExtent.length);
    PARCBuffer *hash = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), hashExtent.offset + 4, hashExtent.offset + hashExtent.length);
    PARCBuffer *payload = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), payloadExtent.offset, payloadExtent.offset + payloadExtent.length);

    CCNxTlvDictionary *interest =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    name, lifetime, keyid, hash, hoplimit);

    ccnxInterest_SetPayloadAndId(interest, payload);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecSchemaV1MessageEncoder_Encode(encoder, interest);

    PARCBuffer *truth = parcBuffer_Wrap(v1_interest_all_fields,
                                        sizeof(v1_interest_all_fields),
                                        interestExtent.offset,
                                        interestExtent.offset + interestExtent.length);

    testCompareEncoderToBuffer(encoder, truth);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&payload);
    parcBuffer_Release(&hash);
    parcBuffer_Release(&keyid);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&interest);
}

LONGBOW_TEST_CASE(InterestReturn, InterestReturn)
{
    // create an Interest that replicates v1_interest_all_fields
    TlvExtent keyidExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_KEYID);
    TlvExtent hashExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_OBJHASH);
    TlvExtent payloadExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_PAYLOAD);
    TlvExtent interestExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_all_fields), V1_MANIFEST_INT_INTEREST);

    CCNxName *name = ccnxName_CreateFromCString("lci:/3=cool");
    uint32_t lifetime = CCNxInterestDefault_LifetimeMilliseconds;
    uint32_t hoplimit = CCNxInterestDefault_HopLimit;

    PARCBuffer *keyid = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), keyidExtent.offset + 4, keyidExtent.offset + keyidExtent.length);
    PARCBuffer *hash = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), hashExtent.offset + 4, hashExtent.offset + hashExtent.length);
    PARCBuffer *payload = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), payloadExtent.offset, payloadExtent.offset + payloadExtent.length);

    CCNxTlvDictionary *interest =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    name, lifetime, keyid, hash, hoplimit);
    parcBuffer_Release(&hash);
    parcBuffer_Release(&keyid);
    ccnxName_Release(&name);

    ccnxInterest_SetPayloadAndId(interest, payload);
    parcBuffer_Release(&payload);

    CCNxInterestReturn *interestReturn =
        ccnxInterestReturn_Create(interest, CCNxInterestReturn_ReturnCode_HopLimitExceeded);
    ccnxTlvDictionary_Release(&interest);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecSchemaV1MessageEncoder_Encode(encoder, interestReturn);

    size_t packetSize = sizeof(v1_interest_all_fields);
    uint8_t *testPacket = (uint8_t *) parcSafeMemory_Allocate(packetSize);
    memcpy(testPacket, v1_interest_all_fields, packetSize);
    testPacket[1] = 0x02; //InterestReturn
    testPacket[5] = CCNxInterestReturn_ReturnCode_HopLimitExceeded;

    PARCBuffer *truth = parcBuffer_Wrap(testPacket, packetSize,
                                        interestExtent.offset,
                                        interestExtent.offset + interestExtent.length);

    testCompareEncoderToBuffer(encoder, truth);

    parcSafeMemory_Deallocate((void **) &testPacket);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
    ccnxInterestReturn_Release(&interestReturn);
}

LONGBOW_TEST_CASE(ContentObject, ContentObjectNameless)
{
    // create a Content Object that replicates v1_content_nameA_keyid1_rsasha256
    TlvExtent payloadExtent = getTruthTableExtent(TRUTHTABLENAME(v1_content_nameless_nosig), V1_MANIFEST_OBJ_PAYLOAD);
    TlvExtent contentObjectExtent = getTruthTableExtent(TRUTHTABLENAME(v1_content_nameless_nosig), V1_MANIFEST_OBJ_CONTENTOBJECT);

    PARCBuffer *payload = parcBuffer_Wrap(v1_content_nameless_nosig, sizeof(v1_content_nameless_nosig), payloadExtent.offset, payloadExtent.offset + payloadExtent.length);

    CCNxTlvDictionary *contentobject =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   NULL, CCNxPayloadType_KEY, payload);

    ccnxContentObject_SetExpiryTime(contentobject, 0x01434B198400ULL);
    ccnxContentObject_SetFinalChunkNumber(contentobject, 0x06050403);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecSchemaV1MessageEncoder_Encode(encoder, contentobject);

    PARCBuffer *truth = parcBuffer_Wrap(v1_content_nameless_nosig, sizeof(v1_content_nameless_nosig), contentObjectExtent.offset, contentObjectExtent.offset + contentObjectExtent.length);

    testCompareEncoderToBuffer(encoder, truth);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&payload);
    ccnxTlvDictionary_Release(&contentobject);
}

LONGBOW_TEST_CASE(ContentObject, ContentObject)
{
    // create a Content Object that replicates v1_content_nameA_keyid1_rsasha256
    TlvExtent payloadExtent = getTruthTableExtent(TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256), V1_MANIFEST_OBJ_PAYLOAD);
    TlvExtent contentObjectExtent = getTruthTableExtent(TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256), V1_MANIFEST_OBJ_CONTENTOBJECT);

    CCNxName *name = ccnxName_CreateFromCString("lci:/3=hello/0xf000=ouch");

    PARCBuffer *payload = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256, sizeof(v1_content_nameA_keyid1_rsasha256), payloadExtent.offset, payloadExtent.offset + payloadExtent.length);

    CCNxTlvDictionary *contentobject =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_KEY, payload);

    ccnxContentObject_SetExpiryTime(contentobject, 0x01434B198400ULL);
    ccnxContentObject_SetFinalChunkNumber(contentobject, 0x06050403);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecSchemaV1MessageEncoder_Encode(encoder, contentobject);

    PARCBuffer *truth = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256, sizeof(v1_content_nameA_keyid1_rsasha256), contentObjectExtent.offset, contentObjectExtent.offset + contentObjectExtent.length);

    testCompareEncoderToBuffer(encoder, truth);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&payload);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&contentobject);
}


// =========================================================================

LONGBOW_TEST_FIXTURE(Interest)
{
    LONGBOW_RUN_TEST_CASE(Interest, ccnxCodecSchemaV1MessageEncoder_Encode);
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
    testrigencoder_CommonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Interest, ccnxCodecSchemaV1MessageEncoder_Encode)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxCodecSchemaV1MessageEncoder_Encode(data->encoder, data->dictionary);

    testCompareEncoderToBuffer(data->encoder, data->memoryRegion);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(InterestReturn)
{
    LONGBOW_RUN_TEST_CASE(InterestReturn, ccnxCodecSchemaV1MessageEncoder_Encode);
}

LONGBOW_TEST_FIXTURE_SETUP(InterestReturn)
{
    size_t packetSize = sizeof(v1_interest_all_fields);
    uint8_t *testPacket = (uint8_t *) parcSafeMemory_Allocate(packetSize);
    memcpy(testPacket, v1_interest_all_fields, packetSize);
    testPacket[1] = 0x02; //InterestReturn
    testPacket[5] = CCNxInterestReturn_ReturnCode_HopLimitExceeded;

    longBowTestCase_SetClipBoardData(testCase,
                                     commonSetup(v1_interest_all_fields,
                                                 sizeof(v1_interest_all_fields),
                                                 TRUTHTABLENAME(v1_interest_all_fields),
                                                 V1_MANIFEST_INT_INTEREST));
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(InterestReturn)
{
    testrigencoder_CommonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(InterestReturn, ccnxCodecSchemaV1MessageEncoder_Encode)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    ccnxCodecSchemaV1MessageEncoder_Encode(data->encoder, data->dictionary);

    testCompareEncoderToBuffer(data->encoder, data->memoryRegion);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _encodeName);
    LONGBOW_RUN_TEST_CASE(Local, _encodePayload);
    LONGBOW_RUN_TEST_CASE(Local, _encodePayloadType);
    LONGBOW_RUN_TEST_CASE(Local, _encodeExpiryTime);
    LONGBOW_RUN_TEST_CASE(Local, _encodeEndChunkNumber);
    LONGBOW_RUN_TEST_CASE(Local, _encodeKeyIdRestriction);
    LONGBOW_RUN_TEST_CASE(Local, _encodeContentObjectHashRestriction);
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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>
#include <parc/algol/parc_Varint.h>

LONGBOW_TEST_CASE(Local, _encodeName)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/0xf001=foot/0xf002=toe/0xf003=nail");
    uint8_t encoded[] = {
        0x00, 0x00, 0x00, 23,
        0xF0, 0x01, 0x00, 4,
        'f',  'o',  'o',  't',
        0xF0, 0x02, 0x00, 3,
        't',  'o',  'e',
        0xF0, 0x03, 0x00, 4,
        'n',  'a',  'i',  'l'
    };

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ccnxTlvDictionary_PutName(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME, name);

    _encodeName(encoder, dictionary);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    assertTrue(parcBuffer_Equals(truth, test), "buffers do not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxName_Release(&name);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Local, _encodePayload)
{
    uint8_t payload[] = { 0xf1, 0xf2, 0xf3 };
    uint8_t encoded[] = {
        0x00, 0x01, 0x00, 3,
        0xf1, 0xf2, 0xf3
    };

    PARCBuffer *buffer = parcBuffer_Wrap(payload, sizeof(payload), 0, sizeof(payload));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ccnxTlvDictionary_PutBuffer(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, buffer);

    _encodePayload(encoder, dictionary);

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "buffers do not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    parcBuffer_Release(&buffer);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Local, _encodePayloadType)
{
    CCNxPayloadType type = CCNxPayloadType_LINK;

    uint8_t encoded[] = {
        0x00, 0x05, 0x00, 1,
        CCNxCodecSchemaV1Types_PayloadType_Link
    };

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ccnxTlvDictionary_PutInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE, type);

    _encodePayloadType(encoder, dictionary);

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "buffers do not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Local, _encodeExpiryTime)
{
    uint64_t expiry = 0x123456789ABCDEF0ULL;
    uint8_t encoded[] = {
        0x00, 0x06, 0x00, 8,
        0x12, 0x34, 0x56, 0x78,
        0x9A, 0xBC, 0xDE, 0xF0
    };

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ccnxTlvDictionary_PutInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME, expiry);

    _encodeExpiryTime(encoder, dictionary);

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "buffers do not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Local, _encodeEndChunkNumber)
{
    uint64_t endChunkNumber = 0x818283ULL;
    uint8_t encoded[] = {
        0x00, 0x19, 0x00, 3,
        0x81, 0x82, 0x83
    };

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ccnxTlvDictionary_PutInteger(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT, endChunkNumber);

    _encodeEndChunkNumber(encoder, dictionary);

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "buffers do not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&dictionary);
}

LONGBOW_TEST_CASE(Local, _encodeKeyIdRestriction)
{
    uint8_t encoded[] = {
        0x00, 0x02, 0x00, 0x24,
        0x00, 0x01, 0x00, 0x20,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 8, sizeof(encoded));
    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ccnxTlvDictionary_PutObject(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION, hash);

    _encodeKeyIdRestriction(encoder, dictionary);

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "buffers do not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    parcBuffer_Release(&buffer);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&dictionary);
    parcCryptoHash_Release(&hash);
}

LONGBOW_TEST_CASE(Local, _encodeContentObjectHashRestriction)
{
    uint8_t encoded[] = {
        0x00, 0x03, 0x00, 0x24,
        0x00, 0x01, 0x00, 0x20,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78,0x9A,  0xBC, 0xDE, 0xF0,
    };

    PARCBuffer *buffer = parcBuffer_Wrap(encoded, sizeof(encoded), 8, sizeof(encoded));
    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ccnxTlvDictionary_PutObject(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION, hash);

    _encodeContentObjectHashRestriction(encoder, dictionary);

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(truth, test), "buffers do not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(truth, 3);
        printf("Got:\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    parcBuffer_Release(&buffer);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&dictionary);
    parcCryptoHash_Release(&hash);
}


// =========================================================================

LONGBOW_TEST_FIXTURE(UnknownType)
{
    LONGBOW_RUN_TEST_CASE(UnknownType, Unknown);
}

LONGBOW_TEST_FIXTURE_SETUP(UnknownType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(UnknownType)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(UnknownType, Unknown)
{
    CCNxTlvDictionary *unknown = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, 1);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1MessageEncoder_Encode(encoder, unknown);
    assertTrue(length < 0, "Did not get error return when encoding unknown type");

    CCNxCodecError *error = ccnxCodecTlvEncoder_GetError(encoder);
    assertNotNull(error, "encoder did not set the error");

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&unknown);
}

// ==================================================================================


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_MessageEncoder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
