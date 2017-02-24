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
 * Does not do detailed tests of the decode -- those are tested in the individual schema_vX unit tests.
 * These tests make sure that we (a) get a result when we expect to get a results, and (b) will spot-check
 * the result, such as looking at the Name.
 *
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.

#include <config.h>
#include <stdio.h>
#include <fcntl.h>

#include "../ccnxCodec_TlvPacket.c"
#include <parc/algol/parc_SafeMemory.h>

#include <LongBow/unit-test.h>

#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>
#include <ccnx/common/validation/ccnxValidation_HmacSha256.h>

#include <ccnx/common/ccnx_Interest.h>
#include <ccnx/common/ccnx_ContentObject.h>

#include <ccnx/common/internal/ccnx_InterestDefault.h>

#include <ccnx/common/codec/schema_v1/testdata/v1_interest_all_fields.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_keyid1_rsasha256.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_bad_message_length.h>

LONGBOW_TEST_RUNNER(rta_TlvPacket)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_TlvPacket)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_TlvPacket)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, rtaTlvPacket_BufferDecode_V1);
    LONGBOW_RUN_TEST_CASE(Global, rtaTlvPacket_BufferDecode_VFF);

    LONGBOW_RUN_TEST_CASE(Global, rtaTlvPacket_IoVecDecode_OneBuffer);
    LONGBOW_RUN_TEST_CASE(Global, rtaTlvPacket_IoVecDecode_SeveralBuffer);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecTlvPacket_DictionaryEncode_V1);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecTlvPacket_DictionaryEncode_VFF);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecTlvPacket_Decode_V1);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecTlvPacket_Decode_VFF);

    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecTlvPacket_EncodeWithSignature);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecTlvPacket_GetPacketLength);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecTlvPacket_MinimalHeaderLength);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
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

LONGBOW_TEST_CASE(Global, rtaTlvPacket_BufferDecode_V1)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    PARCBuffer *interestMessage = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), 0, sizeof(v1_interest_all_fields));
    parcBufferComposer_PutBuffer(composer, interestMessage);
    parcBuffer_Release(&interestMessage);

    // Append extraneous data to the end of the buffer to make sure the decoder terminates at the end of the CCNx message.
    PARCBuffer *padding = parcBuffer_AllocateCString("ThisShouldNeverBeParsed");
    parcBufferComposer_PutBuffer(composer, padding);
    parcBuffer_Release(&padding);
    PARCBuffer *packetBuffer = parcBufferComposer_CreateBuffer(composer);
    parcBufferComposer_Release(&composer);
    parcBuffer_Rewind(packetBuffer);

    //PARCBuffer *packetBuffer = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), 0, sizeof(v1_interest_all_fields));
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    bool success = ccnxCodecTlvPacket_BufferDecode(packetBuffer, dict);
    assertTrue(success, "Failed to decode good v1 interest");

    CCNxName *name = ccnxInterest_GetName(dict);
    assertNotNull(name, "Did not find a name in the decoded interest");

    ccnxTlvDictionary_Release(&dict);
    parcBuffer_Release(&packetBuffer);
}

LONGBOW_TEST_CASE(Global, rtaTlvPacket_BufferDecode_VFF)
{
    uint8_t encoded[] = {
        0xFF, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    PARCBuffer *packetBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    bool success = ccnxCodecTlvPacket_BufferDecode(packetBuffer, dict);
    assertFalse(success, "Did not fail on decode of version 255 packet");

    ccnxTlvDictionary_Release(&dict);
    parcBuffer_Release(&packetBuffer);
}

typedef struct allocator_arg {
    size_t maxallocation;
} AllocatorArg;

static size_t
testAllocator(void *userarg, size_t bytes, void **output)
{
    AllocatorArg *arg = userarg;
    if (bytes > arg->maxallocation) {
        bytes = arg->maxallocation;
    }

    *output = parcMemory_Allocate(bytes);
    assertNotNull(*output, "parcMemory_Allocate(%zu) returned NULL", bytes);
    if (*output) {
        return bytes;
    }
    return 0;
}

static void
testDeallocator(void *userarg, void **memory)
{
    parcMemory_Deallocate((void **) memory);
}

const CCNxCodecNetworkBufferMemoryBlockFunctions TestMemoryBlock = {
    .allocator   = &testAllocator,
    .deallocator = &testDeallocator
};


static void
runIoVecTest(AllocatorArg maxalloc)
{
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&TestMemoryBlock, &maxalloc);

    ccnxCodecNetworkBuffer_PutArray(netbuff,
                                    sizeof(v1_interest_all_fields),
                                    v1_interest_all_fields);

    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);

    CCNxTlvDictionary *output = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);

    ccnxTlvDictionary_SetMessageType_Interest(output, CCNxTlvDictionary_SchemaVersion_V1);

    bool success = ccnxCodecTlvPacket_IoVecDecode(vec, output);
    assertTrue(success, "Failed to decode buffer in iovec format");

    ccnxTlvDictionary_Release(&output);
    ccnxCodecNetworkBufferIoVec_Release(&vec);
    ccnxCodecNetworkBuffer_Release(&netbuff);
}

LONGBOW_TEST_CASE(Global, rtaTlvPacket_IoVecDecode_OneBuffer)
{
    AllocatorArg maxalloc = { .maxallocation = 2048 };
    runIoVecTest(maxalloc);
}

LONGBOW_TEST_CASE(Global, rtaTlvPacket_IoVecDecode_SeveralBuffer)
{
    // 32 bytes is needed for bookkeeping, so this means we'll have a 32-byte memory block
    AllocatorArg maxalloc = { .maxallocation = 64 };
    runIoVecTest(maxalloc);
}

LONGBOW_TEST_CASE(Global, ccnxCodecTlvPacket_DictionaryEncode_V1)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/Antidisestablishmentarianism");
    CCNxTlvDictionary *message =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    name, CCNxInterestDefault_LifetimeMilliseconds, NULL, NULL, CCNxInterestDefault_HopLimit);

    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvPacket_DictionaryEncode(message, NULL);
    assertNotNull(iovec, "Got null iovec on a good dictionary");

    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    ccnxTlvDictionary_Release(&message);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxCodecTlvPacket_DictionaryEncode_VFF)
{
    CCNxTlvDictionary *message = ccnxTlvDictionary_Create(20, 20);
    ccnxTlvDictionary_SetMessageType_Interest(message, 0xFF);
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvPacket_DictionaryEncode(message, NULL);
    assertNull(iovec, "Should have gotten null result for schema version 255");
    ccnxTlvDictionary_Release(&message);
}

LONGBOW_TEST_CASE(Global, ccnxCodecTlvPacket_Decode_V1)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    PARCBuffer *interestMessage = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), 0, sizeof(v1_interest_all_fields));
    parcBufferComposer_PutBuffer(composer, interestMessage);
    parcBuffer_Release(&interestMessage);

    // Append extraneous data to the end of the buffer to make sure the decoder terminates at the end of the CCNx message.
    PARCBuffer *padding = parcBuffer_AllocateCString("ThisShouldNeverBeParsed");
    parcBufferComposer_PutBuffer(composer, padding);
    parcBuffer_Release(&padding);
    PARCBuffer *packetBuffer = parcBufferComposer_CreateBuffer(composer);
    parcBufferComposer_Release(&composer);
    parcBuffer_Rewind(packetBuffer);

    CCNxTlvDictionary *dict = ccnxCodecTlvPacket_Decode(packetBuffer);
    assertNotNull(dict, "Got null dictionary decoding good packet");

    ccnxTlvDictionary_Release(&dict);
    parcBuffer_Release(&packetBuffer);
}

LONGBOW_TEST_CASE(Global, ccnxCodecTlvPacket_Decode_VFF)
{
    uint8_t encoded[] = {
        0xFF, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    PARCBuffer *packetBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));

    CCNxTlvDictionary *dict = ccnxCodecTlvPacket_Decode(packetBuffer);
    assertNull(dict, "Got non-null dictionary decoding version 255 packet");
    parcBuffer_Release(&packetBuffer);
}


LONGBOW_TEST_CASE(Global, ccnxCodecTlvPacket_EncodeWithSignature)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/foo/bar");
    PARCBuffer *payload = parcBuffer_WrapCString("payload");
    CCNxContentObject *obj = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    ccnxName_Release(&name);
    parcBuffer_Release(&payload);

    PARCBuffer *secretKey = parcBuffer_WrapCString("abcdefghijklmnopqrstuvwxyx");
    PARCSigner *signer = ccnxValidationHmacSha256_CreateSigner(secretKey);

    // should really scrub the memory
    parcBuffer_Release(&secretKey);

    // this was breaking the signature
    PARCKeyStore *keyStore = parcSigner_GetKeyStore(signer);
    const PARCCryptoHash *secretHash = parcKeyStore_GetVerifierKeyDigest(keyStore);
    const PARCBuffer *keyid = parcCryptoHash_GetDigest(secretHash);
    ccnxValidationHmacSha256_Set(obj, keyid);
    parcCryptoHash_Release((PARCCryptoHash **) &secretHash);

    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvPacket_DictionaryEncode(obj, signer);

    ccnxCodecNetworkBufferIoVec_Display(iovec, 0);

    int fd = open("/dev/null", O_WRONLY);
    assertTrue(fd != -1, "Error opening /dev/null");

    ssize_t written = writev(fd, ccnxCodecNetworkBufferIoVec_GetArray(iovec), ccnxCodecNetworkBufferIoVec_GetCount(iovec));
    assertTrue(written != -1, "Error writting to /dev/null");
    close(fd);

    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    parcSigner_Release(&signer);
    ccnxContentObject_Release(&obj);
}

static uint8_t testDataV1_Interest_AllFields[] = {
    0x01, 0x00, 0x00, 100,      // ver = 1, type = interest, length = 100
    0x20, 0x00, 0x11, 14,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 82,       // type = interest, length = 82
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
    // ------------------------
    0x00, 0x02, 0x00, 16,       // type = keyid restriction, length = 16
    0xa0, 0xa1, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab,
    0xac, 0xad, 0xae, 0xaf,
    // ------------------------
    0x00, 0x03, 0x00, 32,       // type = hash restriction, length = 32
    0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb,
    0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb,
    0xcc, 0xcd, 0xce, 0xcf,
    // ------------------------
    0x00, 0x04, 0x00, 1,        // Internest payload method (1 byte)
    0x00,
    // ------------------------
    0x00, 0x01, 0x00, 5,        // type = payload, length = 5
    0xD0, 0xD1, 0xD2, 0xD3,
    0xD4,
};

LONGBOW_TEST_CASE(Global, ccnxCodecTlvPacket_GetPacketLength)
{
    PARCBuffer *packet = parcBuffer_Wrap(testDataV1_Interest_AllFields, sizeof(testDataV1_Interest_AllFields), 0, sizeof(testDataV1_Interest_AllFields));
    size_t packetLength = ccnxCodecTlvPacket_GetPacketLength(packet);
    assertTrue(packetLength == sizeof(testDataV1_Interest_AllFields), "Wrong total message length, expected %zu got %zu", sizeof(testDataV1_Interest_AllFields), packetLength);
    parcBuffer_Release(&packet);
}

LONGBOW_TEST_CASE(Global, ccnxCodecTlvPacket_MinimalHeaderLength)
{
    assertTrue(ccnxCodecTlvPacket_MinimalHeaderLength() > 0, "ccnxCodecTlvPacket_MinimalHeaderLength failed");
}

// =================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Global, _decodeV1_Interest);
    LONGBOW_RUN_TEST_CASE(Global, _decodeV1_ContentObject);
    LONGBOW_RUN_TEST_CASE(Global, _decodeV1_Control);
    LONGBOW_RUN_TEST_CASE(Global, _decodeV1_Unknown);
    LONGBOW_RUN_TEST_CASE(Global, _decodeV1_Error);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, _decodeV1_Interest)
{
    PARCBuffer *packetBuffer = parcBuffer_Wrap(v1_interest_all_fields, sizeof(v1_interest_all_fields), 0, sizeof(v1_interest_all_fields));
    CCNxTlvDictionary *dict = _decodeV1(packetBuffer);
    assertNotNull(dict, "Error decoding good packet");

    CCNxName *name = ccnxInterest_GetName(dict);
    assertNotNull(name, "Null name in decoded Interest");

    ccnxTlvDictionary_Release(&dict);
    parcBuffer_Release(&packetBuffer);
}

LONGBOW_TEST_CASE(Global, _decodeV1_ContentObject)
{
    PARCBuffer *packetBuffer = parcBuffer_Wrap(v1_content_nameA_keyid1_rsasha256, sizeof(v1_content_nameA_keyid1_rsasha256), 0, sizeof(v1_content_nameA_keyid1_rsasha256));
    CCNxTlvDictionary *dict = _decodeV1(packetBuffer);
    assertNotNull(dict, "Error decoding good packet");

    CCNxName *name = ccnxContentObject_GetName(dict);
    assertNotNull(name, "Null name in decoded Interest");

    ccnxTlvDictionary_Release(&dict);
    parcBuffer_Release(&packetBuffer);
}

LONGBOW_TEST_CASE(Global, _decodeV1_Control)
{
    testUnimplemented("V1 control not implemented yet");
}

LONGBOW_TEST_CASE(Global, _decodeV1_Unknown)
{
    uint8_t encoded[] = {
        0x01, 0xFF, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    PARCBuffer *packetBuffer = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxTlvDictionary *dict = _decodeV1(packetBuffer);
    assertNull(dict, "Should have gotten NULL dictionary from unknown packet type");

    parcBuffer_Release(&packetBuffer);
}

LONGBOW_TEST_CASE(Global, _decodeV1_Error)
{
    PARCBuffer *packetBuffer = parcBuffer_Wrap(v1_interest_bad_message_length, sizeof(v1_interest_bad_message_length), 0, sizeof(v1_interest_bad_message_length));
    CCNxTlvDictionary *dict = _decodeV1(packetBuffer);
    assertNull(dict, "Should have gotten NULL dictionary from unknown packet type");

    parcBuffer_Release(&packetBuffer);
}

// =================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_TlvPacket);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
