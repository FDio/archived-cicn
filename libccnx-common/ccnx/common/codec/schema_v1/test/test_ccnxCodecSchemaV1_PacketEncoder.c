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
#include "../ccnxCodecSchemaV1_PacketEncoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_keyid1_rsasha256.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_all_fields.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_nameA_crc32c.h>

#include "testrig_encoder.c"

#include <ccnx/common/ccnx_Interest.h>
#include <ccnx/common/ccnx_InterestReturn.h>
#include <ccnx/common/ccnx_ContentObject.h>

#include <ccnx/common/validation/ccnxValidation_CRC32C.h>


// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_PacketEncoder)
{
    LONGBOW_RUN_TEST_FIXTURE(ContentObject);
    LONGBOW_RUN_TEST_FIXTURE(Interest);
    LONGBOW_RUN_TEST_FIXTURE(InterestReturn);
    LONGBOW_RUN_TEST_FIXTURE(Control);
    LONGBOW_RUN_TEST_FIXTURE(UnknownType);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_PacketEncoder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_PacketEncoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


// =========================================================================

LONGBOW_TEST_FIXTURE(ContentObject)
{
    LONGBOW_RUN_TEST_CASE(ContentObject, v1_content_nameA_keyid1_rsasha256);
    LONGBOW_RUN_TEST_CASE(ContentObject, zero_length_payload);
    LONGBOW_RUN_TEST_CASE(ContentObject, null_payload);
    LONGBOW_RUN_TEST_CASE(ContentObject, no_cryptosuite);
}

LONGBOW_TEST_FIXTURE_SETUP(ContentObject)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(ContentObject)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * Make a dictionary equivalent to v1_content_nameA_keyid1_rsasha256 and encode it then compare
 */
LONGBOW_TEST_CASE(ContentObject, v1_content_nameA_keyid1_rsasha256)
{
    CCNxName *name = ccnxName_CreateFromCString(v1_content_nameA_keyid1_rsasha256_URI);

    TlvExtent payloadExtent =
        getTruthTableExtent(TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256), V1_MANIFEST_OBJ_PAYLOAD);
    PARCBuffer *payload = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256,
                                          sizeof(v1_content_nameA_keyid1_rsasha256),
                                          payloadExtent.offset,
                                          payloadExtent.offset + payloadExtent.length);

    CCNxTlvDictionary *message =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_KEY, payload);

    TlvExtent fragmentExtent =
        getTruthTableHeaderExtent(TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256), V1_MANIFEST_OBJ_E2EFRAG);
    PARCBuffer *fragment = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256,
                                           sizeof(v1_content_nameA_keyid1_rsasha256),
                                           fragmentExtent.offset,
                                           fragmentExtent.offset + fragmentExtent.length);
    ccnxTlvDictionary_PutBuffer(message, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_OBJFRAG, fragment);

    uint64_t expiryTime = 1388534400000ULL;
    ccnxContentObject_SetExpiryTime(message, expiryTime);

    uint64_t endChunkNumber = 0x06050403;
    ccnxContentObject_SetFinalChunkNumber(message, endChunkNumber);

    TlvExtent keyIdExtent = getTruthTableExtent(TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256), V1_MANIFEST_OBJ_KEYID);
    PARCBuffer *keyid = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256,
                                        sizeof(v1_content_nameA_keyid1_rsasha256),
                                        keyIdExtent.offset,
                                        keyIdExtent.offset + keyIdExtent.length);

    TlvExtent keyExtent = getTruthTableExtent(TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256), V1_MANIFEST_OBJ_PUBKEY);
    PARCBuffer *key = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256,
                                      sizeof(v1_content_nameA_keyid1_rsasha256),
                                      keyExtent.offset,
                                      keyExtent.offset + keyExtent.length);

    ccnxValidationFacadeV1_SetCryptoSuite(message, PARCCryptoSuite_RSA_SHA256);
    ccnxValidationFacadeV1_SetKeyId(message, keyid);
    ccnxValidationFacadeV1_SetPublicKey(message, key);

    TlvExtent sigExtent =
        getTruthTableExtent(TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256), V1_MANIFEST_OBJ_SIGBITS);
    PARCBuffer *sig = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256,
                                      sizeof(v1_content_nameA_keyid1_rsasha256),
                                      sigExtent.offset,
                                      sigExtent.offset + sigExtent.length);

    ccnxValidationFacadeV1_SetPayload(message, sig);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    assertFalse(length < 0, "Got encoding error: %s",
                ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    assertTrue(length == sizeof(v1_content_nameA_keyid1_rsasha256),
               "Wrong length, expected %zu got %zd", sizeof(v1_content_nameA_keyid1_rsasha256), length);

    // verify
    PARCBuffer *truth = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256,
                                        sizeof(v1_content_nameA_keyid1_rsasha256),
                                        0,
                                        sizeof(v1_content_nameA_keyid1_rsasha256));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);

        uint8_t *truthOverlay = parcBuffer_Overlay(truth, 0);
        uint8_t *testOverlay = parcBuffer_Overlay(test, 0);

        for (ssize_t i = 0; i < length; i++) {
            if (truthOverlay[i] != testOverlay[i]) {
                printf("Buffers differ at byte %zd, expected 0x%02x got 0x%02x\n", i, truthOverlay[i], testOverlay[i]);
                break;
            }
        }
    }

    // cleanup
    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&sig);
    parcBuffer_Release(&key);
    parcBuffer_Release(&keyid);
    parcBuffer_Release(&payload);
    parcBuffer_Release(&fragment);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&message);
}


LONGBOW_TEST_CASE(ContentObject, zero_length_payload)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/no/payload");
    PARCBuffer *payload = parcBuffer_Allocate(0);

    CCNxTlvDictionary *message =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_DATA, payload);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    assertFalse(length < 0, "Got encoding error: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertNotNull(encoded, "got null output buffer");

    parcBuffer_Display(encoded, 3);

    parcBuffer_Release(&encoded);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&message);
    parcBuffer_Release(&payload);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(ContentObject, null_payload)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/no/payload");

    CCNxTlvDictionary *message =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_DATA, NULL);
    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    assertFalse(length < 0, "Got encoding error: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertNotNull(encoded, "got null output buffer");

    parcBuffer_Release(&encoded);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&message);
    ccnxName_Release(&name);
}

/*
 * A content object without a cryptosuite should not be signed
 */
LONGBOW_TEST_CASE(ContentObject, no_cryptosuite)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/no/payload");

    CCNxTlvDictionary *message =
        ccnxContentObject_CreateWithImplAndPayload(&CCNxContentObjectFacadeV1_Implementation,
                                                   name, CCNxPayloadType_DATA, NULL);
    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    assertFalse(length < 0, "Got encoding error: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertNotNull(encoded, "got null output buffer");

    // it should be 33 bytes without a signature
    assertTrue(parcBuffer_Remaining(encoded) == 38, "Wrong length exepcted 38 got %zu", parcBuffer_Remaining(encoded));

    parcBuffer_Release(&encoded);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&message);
    ccnxName_Release(&name);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(Interest)
{
    LONGBOW_RUN_TEST_CASE(Interest, v1_interest_nameA_crc32c);
    LONGBOW_RUN_TEST_CASE(Interest, v1_interest_nameA_crc32c_IoVec);
}

LONGBOW_TEST_FIXTURE_SETUP(Interest)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Interest)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * Make an interest equivalent to v1_interest_nameA_crc32c and encode it
 */
LONGBOW_TEST_CASE(Interest, v1_interest_nameA_crc32c)
{
    CCNxName *name = ccnxName_CreateFromCString(v1_interest_nameA_crc32c_URI);

    CCNxTlvDictionary *message =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    name, CCNxInterestDefault_LifetimeMilliseconds, NULL, NULL, 32);

    TlvExtent fragmentExtent = getTruthTableHeaderExtent(TRUTHTABLENAME(v1_interest_nameA_crc32c), V1_MANIFEST_INT_E2EFRAG);
    PARCBuffer *fragment = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), fragmentExtent.offset, fragmentExtent.offset + fragmentExtent.length);
    ccnxTlvDictionary_PutBuffer(message, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG, fragment);

    ccnxValidationFacadeV1_SetCryptoSuite(message, PARCCryptoSuite_NULL_CRC32C);

    TlvExtent sigExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_nameA_crc32c), V1_MANIFEST_INT_ValidationPayload);
    PARCBuffer *sig = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), sigExtent.offset, sigExtent.offset + sigExtent.length);

    ccnxValidationFacadeV1_SetPayload(message, sig);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    assertFalse(length < 0, "Got encoding error: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    assertTrue(length == sizeof(v1_interest_nameA_crc32c), "Wrong length, expected %zu got %zd", sizeof(v1_interest_nameA_crc32c), length);

    // verify
    PARCBuffer *truth = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), 0, sizeof(v1_interest_nameA_crc32c));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);

        uint8_t *truthOverlay = parcBuffer_Overlay(truth, 0);
        uint8_t *testOverlay = parcBuffer_Overlay(test, 0);

        for (ssize_t i = 0; i < length; i++) {
            if (truthOverlay[i] != testOverlay[i]) {
                printf("Buffers differ at byte %zd, expected 0x%02x got 0x%02x", i, truthOverlay[i], testOverlay[i]);
                break;
            }
        }
    }

    // cleanup
    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&sig);
    parcBuffer_Release(&fragment);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&message);
}

/*
 * Make an interest equivalent to v1_interest_nameA_crc32c and encode it
 */
LONGBOW_TEST_CASE(Interest, v1_interest_nameA_crc32c_IoVec)
{
    CCNxName *name = ccnxName_CreateFromCString(v1_interest_nameA_crc32c_URI);

    CCNxTlvDictionary *message =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    name, CCNxInterestDefault_LifetimeMilliseconds, NULL, NULL, 32);

    TlvExtent fragmentExtent = getTruthTableHeaderExtent(TRUTHTABLENAME(v1_interest_nameA_crc32c), V1_MANIFEST_INT_E2EFRAG);
    PARCBuffer *fragment = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), fragmentExtent.offset, fragmentExtent.offset + fragmentExtent.length);
    ccnxTlvDictionary_PutBuffer(message, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG, fragment);

    ccnxValidationFacadeV1_SetCryptoSuite(message, PARCCryptoSuite_NULL_CRC32C);

    TlvExtent sigExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_nameA_crc32c), V1_MANIFEST_INT_ValidationPayload);
    PARCBuffer *sig = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), sigExtent.offset, sigExtent.offset + sigExtent.length);

    ccnxValidationFacadeV1_SetPayload(message, sig);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecSchemaV1PacketEncoder_DictionaryEncode(message, NULL);
    assertNotNull(iovec, "Got null iovec from ccnxCodecSchemaV1PacketEncoder_DictionaryEncode");

    size_t length = ccnxCodecNetworkBufferIoVec_Length(iovec);
    assertTrue(length == sizeof(v1_interest_nameA_crc32c), "Wrong length, expected %zu got %zd", sizeof(v1_interest_nameA_crc32c), length);

    // verify
    PARCBuffer *truth = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), 0, sizeof(v1_interest_nameA_crc32c));

    int iovecCount = ccnxCodecNetworkBufferIoVec_GetCount(iovec);
    PARCBuffer *test = parcBuffer_Allocate(length);
    const struct iovec *array = ccnxCodecNetworkBufferIoVec_GetArray(iovec);
    for (int i = 0; i < iovecCount; i++) {
        parcBuffer_PutArray(test, array[i].iov_len, array[i].iov_base);
    }
    parcBuffer_Flip(test);

    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);

        uint8_t *truthOverlay = parcBuffer_Overlay(truth, 0);
        uint8_t *testOverlay = parcBuffer_Overlay(test, 0);

        for (ssize_t i = 0; i < length; i++) {
            if (truthOverlay[i] != testOverlay[i]) {
                printf("Buffers differ at byte %zd, expected 0x%02x got 0x%02x", i, truthOverlay[i], testOverlay[i]);
                break;
            }
        }
    }

    // cleanup
    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    parcBuffer_Release(&sig);
    parcBuffer_Release(&fragment);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&message);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(InterestReturn)
{
    LONGBOW_RUN_TEST_CASE(InterestReturn, v1_interest_return);
}

LONGBOW_TEST_FIXTURE_SETUP(InterestReturn)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(InterestReturn)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * Make an interest return and encode it
 */
LONGBOW_TEST_CASE(InterestReturn, v1_interest_return)
{
    CCNxName *name = ccnxName_CreateFromCString(v1_interest_nameA_crc32c_URI);

    CCNxInterest *interest =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    name, CCNxInterestDefault_LifetimeMilliseconds, NULL, NULL, 32);
    ccnxName_Release(&name);

    TlvExtent fragmentExtent = getTruthTableHeaderExtent(TRUTHTABLENAME(v1_interest_nameA_crc32c), V1_MANIFEST_INT_E2EFRAG);
    PARCBuffer *fragment = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), fragmentExtent.offset, fragmentExtent.offset + fragmentExtent.length);
    ccnxTlvDictionary_PutBuffer(interest, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG, fragment);
    parcBuffer_Release(&fragment);

    ccnxValidationFacadeV1_SetCryptoSuite(interest, PARCCryptoSuite_NULL_CRC32C);

    TlvExtent sigExtent = getTruthTableExtent(TRUTHTABLENAME(v1_interest_nameA_crc32c), V1_MANIFEST_INT_ValidationPayload);
    PARCBuffer *sig = parcBuffer_Wrap(v1_interest_nameA_crc32c, sizeof(v1_interest_nameA_crc32c), sigExtent.offset, sigExtent.offset + sigExtent.length);

    ccnxValidationFacadeV1_SetPayload(interest, sig);
    parcBuffer_Release(&sig);

    CCNxTlvDictionary *message =
        ccnxInterestReturn_Create(interest, CCNxInterestReturn_ReturnCode_NoResources);
    ccnxInterest_Release(&interest);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    ssize_t expectedLength = sizeof(v1_interest_nameA_crc32c);
    assertFalse(length < 0, "Got encoding error: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    assertTrue(length == expectedLength, "Wrong length, expected %zu got %zd", expectedLength, length);

    // verify
    PARCBuffer *truth = parcBuffer_Wrap(v1_interest_nameA_crc32c_returned, sizeof(v1_interest_nameA_crc32c_returned), 0, sizeof(v1_interest_nameA_crc32c_returned));
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);

        uint8_t *truthOverlay = parcBuffer_Overlay(truth, 0);
        uint8_t *testOverlay = parcBuffer_Overlay(test, 0);

        for (ssize_t i = 0; i < length; i++) {
            if (truthOverlay[i] != testOverlay[i]) {
                printf("Buffers differ at byte %zd, expected 0x%02x got 0x%02x\n", i, truthOverlay[i], testOverlay[i]);
            }
        }
    }

    // cleanup
    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&message);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(Control)
{
    LONGBOW_RUN_TEST_CASE(Control, payload);
    LONGBOW_RUN_TEST_CASE(Control, cryptosuite);
}

LONGBOW_TEST_FIXTURE_SETUP(Control)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Control)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Control, payload)
{
    uint8_t encoded[] = {
        0x01, 0xa4, 0x00, 0x22,
        0x00, 0x00, 0x00, 0x08,

        0xbe, 0xef, 0x00, 0x16, // control message

        0x7b, 0x22, 0x74, 0x68, // {"th
        0x69, 0x73, 0x20, 0x69, //   is i
        0x73, 0x22, 0x3a, 0x22, //   s":"
        0x61, 0x6e, 0x6e, 0x6f, //   anno
        0x79, 0x69, 0x6e, 0x67, //   ying
        0x22, 0x7d              //   "}
    };


    CCNxTlvDictionary *message = ccnxCodecSchemaV1TlvDictionary_CreateControl();

    PARCJSON *json = parcJSON_Create();
    parcJSON_AddString(json, "this is", "annoying");

    ccnxTlvDictionary_PutJson(message, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, json);
    parcJSON_Release(&json);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);

    assertFalse(length < 0, "Got encoding error: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    assertTrue(length == sizeof(encoded), "Wrong length, expected %zu got %zd", sizeof(encoded), length);

    // verify
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);

        uint8_t *truthOverlay = parcBuffer_Overlay(truth, 0);
        uint8_t *testOverlay = parcBuffer_Overlay(test, 0);

        for (ssize_t i = 0; i < length; i++) {
            if (truthOverlay[i] != testOverlay[i]) {
                printf("Buffers differ at byte %zd, expected 0x%02x got 0x%02x", i, truthOverlay[i], testOverlay[i]);
                break;
            }
        }
    }

    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&message);
    parcBuffer_Release(&truth);
}

LONGBOW_TEST_CASE(Control, cryptosuite)
{
    uint8_t encoded[] = {
        0x01, 0xA4, 0x00, 16,
        0x00, 0x00, 0x00, 8,
        0xBE, 0xEF, 0x00, 4,
        'a',  'b',  'c',  'd',
        0x00, 0x03, 0x00, 4,
        0x00, 0x02, 0x00, 0
    };

    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *payload = parcBuffer_Wrap(encoded, sizeof(encoded), 12, 16);

    CCNxTlvDictionary *message = ccnxCodecSchemaV1TlvDictionary_CreateControl();
    ccnxTlvDictionary_PutBuffer(message, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, payload);
    ccnxValidationCRC32C_Set(message);

    ccnxValidationCRC32C_Set(message);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    PARCSigner *signer = ccnxValidationCRC32C_CreateSigner();
    ccnxCodecTlvEncoder_SetSigner(encoder, signer);

    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    assertFalse(length < 0, "Got encoding error: %s", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertNotNull(test, "Got null buffer from encoder");
    uint8_t testSuite = parcBuffer_GetAtIndex(test, 21);
    assertTrue(testSuite == 2, "Wrong cryptosuite, expected 2 got %u", testSuite);

    parcSigner_Release(&signer);
    parcBuffer_Release(&test);
    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&message);
    parcBuffer_Release(&payload);
    parcBuffer_Release(&truth);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(UnknownType)
{
    LONGBOW_RUN_TEST_CASE(UnknownType, unknown);
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

/*
 * try to encode a message with unknown message type
 */
LONGBOW_TEST_CASE(UnknownType, unknown)
{
    // this initializes the dictionary to unknown type
    CCNxTlvDictionary *message = ccnxTlvDictionary_Create(20, 20);

    // encode
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = ccnxCodecSchemaV1PacketEncoder_Encode(encoder, message);
    assertTrue(length < 0, "Did not get error condition for unknown type");

    CCNxCodecError *error = ccnxCodecTlvEncoder_GetError(encoder);
    assertNotNull(error, "Did not get an error for invalid encoding");

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxTlvDictionary_Release(&message);
}

// =========================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _getHopLimit_Present);
    LONGBOW_RUN_TEST_CASE(Local, _getHopLimit_Missing);
    LONGBOW_RUN_TEST_CASE(Local, _encodeFixedHeader_ContentObject);
    LONGBOW_RUN_TEST_CASE(Local, _encodeFixedHeader_Interest);
    LONGBOW_RUN_TEST_CASE(Local, _encodeOptionalHeaders);
    LONGBOW_RUN_TEST_CASE(Local, _encodeMessage_Interest);
    LONGBOW_RUN_TEST_CASE(Local, _encodeMessage_ContentObject);
    LONGBOW_RUN_TEST_CASE(Local, _encodeCPI);
    LONGBOW_RUN_TEST_CASE(Local, _encodeMessage_Unknown);
    LONGBOW_RUN_TEST_CASE(Local, _encodeValidationAlg_Present);
    LONGBOW_RUN_TEST_CASE(Local, _encodeValidationAlg_Missing);
    LONGBOW_RUN_TEST_CASE(Local, _encodeValidationPayload_Present);
    LONGBOW_RUN_TEST_CASE(Local, _encodeValidationPayload_Missing);
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

LONGBOW_TEST_CASE(Local, _getHopLimit_Present)
{
    uint8_t hoplimit = 77;
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxTlvDictionary_PutInteger(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT, hoplimit);

    uint8_t test = _getHopLimit(dict);
    assertTrue(test == hoplimit, "Got wrong hoplimit, expected %u got %u", hoplimit, test);
    ccnxTlvDictionary_Release(&dict);
}

LONGBOW_TEST_CASE(Local, _getHopLimit_Missing)
{
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    uint8_t test = _getHopLimit(dict);
    assertTrue(test == CCNxInterestDefault_HopLimit, "Got wrong hoplimit, expected %u got %u", CCNxInterestDefault_HopLimit, test);
    ccnxTlvDictionary_Release(&dict);
}

LONGBOW_TEST_CASE(Local, _encodeFixedHeader_ContentObject)
{
    uint8_t encoded[] = {
        0x01, 0x01, 0x00, 100,      // ver = 1, type = content object, length = 100
        0x00, 0x00, 0x00, 14,       // reserved = 0x0000000, header length = 14
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = _encodeFixedHeader(encoder, dict, CCNxCodecSchemaV1Types_PacketType_ContentObject, 14, 100);
    assertTrue(length == 8, "wrong length, expected %d got %zd", 8, length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeFixedHeader_Interest)
{
    uint8_t encoded[] = {
        0x01, 0x00, 0x00, 100,      // ver = 1, type = interest, length = 100
        0x1f, 0x00, 0x00, 14,       // hoplimit = 31,  reserved = 0x0000, header length = 14
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxTlvDictionary_PutInteger(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT, 31);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = _encodeFixedHeader(encoder, dict, CCNxCodecSchemaV1Types_PacketType_Interest, 14, 100);
    assertTrue(length == 8, "wrong length, expected %d got %zd", 8, length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeOptionalHeaders)
{
    uint8_t encoded[] = {
        0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
        0xEA, 0xEB,
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxTlvDictionary_PutInteger(dict, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime, 0xEAEB);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    ssize_t length = _encodeOptionalHeaders(encoder, dict);
    assertTrue(length == sizeof(encoded), "wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeMessage_Interest)
{
    uint8_t encoded[] = {
        0x00, 0x01,                   0x00, 13,
        0x00, 0x00,                   0x00, 9,
        0x00, CCNxNameLabelType_NAME, 0x00, 5,
        'p',  'o',                    'p',  'p', 'y'
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxName *name = ccnxName_CreateFromCString("lci:/poppy");
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxTlvDictionary_PutName(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME, name);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxCodecSchemaV1Types_PacketType packetType;
    ssize_t length = _encodeMessage(encoder, dict, &packetType);
    assertTrue(length == sizeof(encoded), "wrong length, expected %zu got %zd", sizeof(encoded), length);
    assertTrue(packetType == CCNxCodecSchemaV1Types_PacketType_Interest, "Wrong packet type, expected %d got %d",
               CCNxCodecSchemaV1Types_PacketType_Interest, packetType);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeMessage_ContentObject)
{
    uint8_t encoded[] = {
        0x00, 0x02,                   0x00, 13,
        0x00, 0x00,                   0x00, 9,
        0x00, CCNxNameLabelType_NAME, 0x00, 5,
        'p',  'o',                    'p',  'p', 'y'
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxName *name = ccnxName_CreateFromCString("lci:/poppy");
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    ccnxTlvDictionary_PutName(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME, name);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxCodecSchemaV1Types_PacketType packetType;
    ssize_t length = _encodeMessage(encoder, dict, &packetType);
    assertTrue(length == sizeof(encoded), "wrong length, expected %zu got %zd", sizeof(encoded), length);
    assertTrue(packetType == CCNxCodecSchemaV1Types_PacketType_ContentObject, "Wrong packet type, expected %d got %d",
               CCNxCodecSchemaV1Types_PacketType_ContentObject, packetType);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxName_Release(&name);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeCPI)
{
    uint8_t encoded [] = {
        0x00, 0x02, 0x03, 0x99,     //
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxValidationCRC32C_Set(dict);
    ccnxTlvDictionary_PutBuffer(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, truth);

    ssize_t length = _encodeCPI(encoder, dict);
    assertTrue(length == sizeof(encoded), "wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

/*
 * This test requires that we set the message type to some unknown value, which we get if
 * we create a dictionary with ccnxTlvDictionary_Create() and dont call anything to set
 * the message type.  It will be "CCNxTlvDictionary_Unknown".
 */
LONGBOW_TEST_CASE(Local, _encodeMessage_Unknown)
{
    CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(20, 20);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();

    CCNxCodecSchemaV1Types_PacketType packetType;
    ssize_t length = _encodeMessage(encoder, dict, &packetType);
    assertTrue(length < 0, "wrong length, expected negative got %zd", length);

    CCNxCodecError *error = ccnxCodecTlvEncoder_GetError(encoder);
    assertNotNull(error, "Got null error when an error condition should have been set");

    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeValidationAlg_Present)
{
    uint8_t encoded [] = {
        0x00, 0x03, 0x00, 4,        // validation alg, length = 4
        0x00, 0x02, 0x00, 0x00,     // CRC32C
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *truePayload = parcBuffer_Wrap(encoded, sizeof(encoded), 4, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxValidationCRC32C_Set(dict);
    ccnxTlvDictionary_PutBuffer(dict, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_PAYLOAD, truePayload);

    ssize_t length = _encodeValidationAlg(encoder, dict);
    assertTrue(length == sizeof(encoded), "wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&truePayload);
    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeValidationAlg_Missing)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ssize_t length = _encodeValidationAlg(encoder, dict);
    assertTrue(length == 0, "wrong length, expected %d got %zd", 0, length);

    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeValidationPayload_Present)
{
    uint8_t encoded [] = {
        0x00, 0x04, 0x00, 4,        // validation payload, length = 4
        0x00, 0x02, 0x03, 0x99,     //
    };
    PARCBuffer *truth = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    PARCBuffer *truePayload = parcBuffer_Wrap(encoded, sizeof(encoded), 4, sizeof(encoded));

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxValidationCRC32C_Set(dict);
    ccnxTlvDictionary_PutBuffer(dict, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_PAYLOAD, truePayload);

    ssize_t length = _encodeValidationPayload(encoder, dict);
    assertTrue(length == sizeof(encoded), "wrong length, expected %zu got %zd", sizeof(encoded), length);

    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *test = ccnxCodecTlvEncoder_CreateBuffer(encoder);
    assertTrue(parcBuffer_Equals(test, truth), "Buffers mismatch")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);
        printf("Got\n");
        parcBuffer_Display(test, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&truePayload);
    parcBuffer_Release(&test);
    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}

LONGBOW_TEST_CASE(Local, _encodeValidationPayload_Missing)
{
    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();

    ssize_t length = _encodeValidationPayload(encoder, dict);
    assertTrue(length == 0, "wrong length, expected %d got %zd", 0, length);

    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvEncoder_Destroy(&encoder);
}


// ==================================================================================


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_PacketEncoder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
