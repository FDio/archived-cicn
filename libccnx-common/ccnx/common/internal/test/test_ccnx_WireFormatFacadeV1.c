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
#include "../ccnx_WireFormatFacadeV1.c"

#include <parc/algol/parc_SafeMemory.h>

#include <LongBow/unit-test.h>

#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/ccnx_WireFormatMessage.h>
#include <ccnx/common/codec/ccnxCodec_TlvPacket.h>

#include <ccnx/common/codec/schema_v1/testdata/v1_interest_nameA.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_content_nameA_crc32c.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_cpi_add_route.h>

LONGBOW_TEST_RUNNER(ccnx_WireFormatFacade)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(SchemaV1);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_WireFormatFacade)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_WireFormatFacade)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * Small size allocator for creating a network buffer
 */
static size_t
allocator(void *userarg, size_t bytes, void **output)
{
    const size_t allocationSize = *(size_t *) userarg;
    void *allocated = parcMemory_Allocate(allocationSize);
    *output = allocated;
    return allocationSize;
}

/*
 * deallocator for network buffer
 */
static void
deallocator(void *userarg, void **memoryPtr)
{
    parcMemory_Deallocate(memoryPtr);
}

static const CCNxCodecNetworkBufferMemoryBlockFunctions memory = { .allocator = allocator, .deallocator = deallocator };

/*
 * Create a network buffer that looks like this.  The actual number of iovecs might
 * be a little different, but the digest area will span several iovec.
 *
 * +-----------+-----------+-----------+-----------+-----------+
 *    iov[0]       iov[1]      iov[2]      iov[3]
 * +-----------+-----------+-----------+-----------+-----------+
 *                     ^                      ^
 *                     |                      |
 *                   start                   end
 */
static CCNxCodecNetworkBufferIoVec *
createNetworkBufferIoVec(size_t allocationSize, size_t padlen, uint8_t pad[padlen], size_t datalen, uint8_t data[datalen])
{
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&memory, &allocationSize);
    // build the network buffer
    ccnxCodecNetworkBuffer_PutArray(netbuff, padlen, pad);
    ccnxCodecNetworkBuffer_PutArray(netbuff, datalen, data);
    ccnxCodecNetworkBuffer_PutArray(netbuff, padlen, pad);

    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
    ccnxCodecNetworkBuffer_Release(&netbuff);
    return vec;
}

// =======================================================================

LONGBOW_TEST_FIXTURE(SchemaV1)
{
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromContentObjectPacketType);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromControlPacketType);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromInterestPacketType);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromInterestReturnPacketType);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Get);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Put);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_WriteToFile);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromInterestPacketTypeIoVec);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_GetIoVec);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_SetHopLimit);

    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_SetProtectedRegionStart);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_SetProtectedRegionLength);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_HashProtectedRegion_Buffer);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_HashProtectedRegion_IoVec);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_ComputeContentObjectHash);

    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_Interest);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_ContentObject);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_Control);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_InterestReturn);
    LONGBOW_RUN_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_UnknownPacketType);
}

LONGBOW_TEST_FIXTURE_SETUP(SchemaV1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(SchemaV1)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromContentObjectPacketType)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxTlvDictionary *wireformat = _ccnxWireFormatFacadeV1_FromContentObjectPacketType(buffer);
    assertNotNull(wireformat, "Got null wireformat");
    assertTrue(ccnxTlvDictionary_IsContentObject(wireformat), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(wireformat) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(wireformat), CCNxTlvDictionary_SchemaVersion_V1);

    ccnxTlvDictionary_Release(&wireformat);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromControlPacketType)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxTlvDictionary *wireformat = _ccnxWireFormatFacadeV1_FromControlPacketType(buffer);
    assertNotNull(wireformat, "Got null wireformat");
    assertTrue(ccnxTlvDictionary_IsControl(wireformat), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(wireformat) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(wireformat), CCNxTlvDictionary_SchemaVersion_V1);

    ccnxTlvDictionary_Release(&wireformat);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromInterestPacketType)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxTlvDictionary *wireformat = _ccnxWireFormatFacadeV1_FromInterestPacketType(buffer);
    assertNotNull(wireformat, "Got null wireformat");
    assertTrue(ccnxTlvDictionary_IsInterest(wireformat), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(wireformat) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(wireformat), CCNxTlvDictionary_SchemaVersion_V1);

    ccnxTlvDictionary_Release(&wireformat);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromInterestReturnPacketType)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxTlvDictionary *wireformat = _ccnxWireFormatFacadeV1_FromInterestReturnPacketType(buffer);
    assertNotNull(wireformat, "Got null wireformat");
    assertTrue(ccnxTlvDictionary_IsInterestReturn(wireformat), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(wireformat) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(wireformat), CCNxTlvDictionary_SchemaVersion_V1);

    ccnxTlvDictionary_Release(&wireformat);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Get)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxTlvDictionary *wireformat = _ccnxWireFormatFacadeV1_FromInterestPacketType(buffer);

    PARCBuffer *test = _ccnxWireFormatFacadeV1_GetWireFormatBuffer(wireformat);
    assertTrue(test == buffer, "Wrong iovec: got %p expected %p", (void *) test, (void *) buffer);

    ccnxTlvDictionary_Release(&wireformat);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Put)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxTlvDictionary *packet = ccnxTlvDictionary_Create(20, 20);
    ccnxTlvDictionary_SetMessageType_Interest(packet, CCNxTlvDictionary_SchemaVersion_V1);
    bool success = _ccnxWireFormatFacadeV1_PutWireFormatBuffer(packet, buffer);

    assertTrue(success, "Failed to put buffer in to dictionary");

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_WriteToFile)
{
    const char string[] = "Hello dev null\n";
    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));
    CCNxTlvDictionary *wireformat = _ccnxWireFormatFacadeV1_FromInterestPacketType(buffer);

    _ccnxWireFormatFacadeV1_WriteToFile(wireformat, "/dev/null");

    ccnxTlvDictionary_Release(&wireformat);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_FromInterestPacketTypeIoVec)
{
    uint8_t data[64];

    uint8_t pad[32];
    CCNxCodecNetworkBufferIoVec *vec = createNetworkBufferIoVec(512, 32, pad, 64, data);

    CCNxTlvDictionary *wireformat = _ccnxWireFormatFacadeV1_FromInterestPacketTypeIoVec(vec);
    assertNotNull(wireformat, "Got null wireformat");
    assertTrue(ccnxTlvDictionary_IsInterest(wireformat), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(wireformat) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(wireformat), CCNxTlvDictionary_SchemaVersion_V1);

    ccnxTlvDictionary_Release(&wireformat);
    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_GetIoVec)
{
    uint8_t *data = parcMemory_Allocate(64);
    memset(data, 0, 64);

    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, 64, data);
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);

    CCNxTlvDictionary *packet = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    _ccnxWireFormatFacadeV1_PutIoVec(packet, iovec);

    CCNxCodecNetworkBufferIoVec *test = _ccnxWireFormatFacadeV1_GetIoVec(packet);
    assertTrue(test == iovec, "Failed to get iovec from dictionary, expected %p got %p", (void *) iovec, (void *) test);

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    ccnxCodecNetworkBuffer_Release(&netbuff);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_SetHopLimit)
{
    uint8_t *data = parcMemory_Allocate(64);
    memset(data, 0, 64);

    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, 64, data);
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);

    CCNxTlvDictionary *packet = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    _ccnxWireFormatFacadeV1_PutIoVec(packet, iovec);

    _ccnxWireFormatFacadeV1_SetHopLimit(packet, 10);

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    ccnxCodecNetworkBuffer_Release(&netbuff);

    CCNxCodecSchemaV1InterestHeader header;

    buffer = parcBuffer_Wrap((void *) &header, sizeof(header), 0, sizeof(header));

    packet = _ccnxWireFormatFacadeV1_FromContentObjectPacketType(buffer);

    _ccnxWireFormatFacadeV1_SetHopLimit(packet, 10);

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_SetProtectedRegionStart)
{
    const char string[] = "Hello dev null\n";
    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));

    CCNxTlvDictionary *packet = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    _ccnxWireFormatFacadeV1_PutWireFormatBuffer(packet, buffer);

    size_t start = 5;
    bool success = _ccnxWireFormatFacadeV1_SetProtectedRegionStart(packet, start);
    assertTrue(success, "Failed to put integer in to dictionary");

    assertTrue(ccnxTlvDictionary_IsValueInteger(packet, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ProtectedStart), "ProtectedStart not set");

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_SetProtectedRegionLength)
{
    const char string[] = "Hello dev null\n";
    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));

    CCNxTlvDictionary *packet = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    _ccnxWireFormatFacadeV1_PutWireFormatBuffer(packet, buffer);

    size_t length = 5;
    bool success = _ccnxWireFormatFacadeV1_SetProtectedRegionLength(packet, length);
    assertTrue(success, "Failed to put integer in to dictionary");

    assertTrue(ccnxTlvDictionary_IsValueInteger(packet, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ProtectedLength), "ProtectedLength not set");

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_HashProtectedRegion_Buffer)
{
    //                         >1234<
    const char string[] = "Hello dev null\n";

    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));
    size_t start = 5;
    size_t length = 4;

    CCNxTlvDictionary *packet = _ccnxWireFormatFacadeV1_FromContentObjectPacketType(buffer);
    _ccnxWireFormatFacadeV1_SetProtectedRegionStart(packet, start);
    _ccnxWireFormatFacadeV1_SetProtectedRegionLength(packet, length);

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    PARCCryptoHash *hash = _ccnxWireFormatFacadeV1_HashProtectedRegion(packet, hasher);

    // the correctness of the has is tested in _ccnxWireFormatFacadeV1_ComputeHash
    assertNotNull(hash, "Got null hash from a good packet");

    ccnxTlvDictionary_Release(&packet);
    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&buffer);
    parcCryptoHasher_Release(&hasher);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_HashProtectedRegion_IoVec)
{
    uint8_t *data = parcMemory_Allocate(64);
    memset(data, 0, 64);

    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, 64, data);
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);

    CCNxTlvDictionary *packet = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    _ccnxWireFormatFacadeV1_PutIoVec(packet, iovec);

    _ccnxWireFormatFacadeV1_SetProtectedRegionStart(packet, 0);
    _ccnxWireFormatFacadeV1_SetProtectedRegionLength(packet, 64);

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    PARCCryptoHash *hash = _ccnxWireFormatFacadeV1_HashProtectedRegion(packet, hasher);

    // the correctness of the has is tested in _ccnxWireFormatFacadeV1_ComputeHash
    assertNotNull(hash, "Got null hash from a good packet");

    ccnxTlvDictionary_Release(&packet);
    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&buffer);
    parcCryptoHasher_Release(&hasher);
    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    ccnxCodecNetworkBuffer_Release(&netbuff);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_Interest)
{
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));
    CCNxTlvDictionary *test = _ccnxWireFormatFacadeV1_CreateFromV1(wireFormat);
    assertNotNull(test, "Got null dictionary for good interest");
    assertTrue(ccnxTlvDictionary_IsInterest(test), "Dictionary says it is not an Interest");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == 1, "Schema says it is not v1");
    parcBuffer_Release(&wireFormat);
    ccnxTlvDictionary_Release(&test);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_ContentObject)
{
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_content_nameA_crc32c, sizeof(v1_content_nameA_crc32c), 0, sizeof(v1_content_nameA_crc32c));
    CCNxTlvDictionary *test = _ccnxWireFormatFacadeV1_CreateFromV1(wireFormat);
    assertNotNull(test, "Got null dictionary for good content object");
    assertTrue(ccnxTlvDictionary_IsContentObject(test), "Dictionary says it is not a Content Object");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == 1, "Schema says it is not v1");
    parcBuffer_Release(&wireFormat);
    ccnxTlvDictionary_Release(&test);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_Control)
{
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_cpi_add_route, sizeof(v1_cpi_add_route), 0, sizeof(v1_cpi_add_route));
    CCNxTlvDictionary *test = _ccnxWireFormatFacadeV1_CreateFromV1(wireFormat);
    assertNotNull(test, "Got null dictionary for good control");
    assertTrue(ccnxTlvDictionary_IsControl(test), "Dictionary says it is not a control");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == 1, "Schema says it is not v1");
    parcBuffer_Release(&wireFormat);
    ccnxTlvDictionary_Release(&test);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_InterestReturn)
{
    uint8_t encoded[] = { 1, CCNxCodecSchemaV1Types_PacketType_InterestReturn, 0, 23 };
    PARCBuffer *wireFormat = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxTlvDictionary *test = _ccnxWireFormatFacadeV1_CreateFromV1(wireFormat);
    assertNotNull(test, "Got null dictionary for good InterestReturn");
    assertTrue(ccnxTlvDictionary_IsInterestReturn(test), "Expected IsInterestReturn() to be true");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == 1, "Schema says it is not v1");
    parcBuffer_Release(&wireFormat);
    ccnxTlvDictionary_Release(&test);
}

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_Create_UnknownPacketType)
{
    uint8_t encoded[] = { 1, 99, 0, 23 };
    PARCBuffer *wireFormat = parcBuffer_Wrap(encoded, sizeof(encoded), 0, sizeof(encoded));
    CCNxTlvDictionary *test = _ccnxWireFormatFacadeV1_CreateFromV1(wireFormat);
    assertNull(test, "Should have gotten null dictionary for unknown packet type");
    parcBuffer_Release(&wireFormat);
}


static uint8_t _v1_ContentObject_WithKnownHash[] = {
    //   Name: lci:/boose/roo/pie
    //   Payload: "this is the payload"
    //   Signer: CRC32
    //   CO Hash: 4FB301EA5FD523B9A71287B721DC20C94B2D4827674A8CA275B7D57C60447876

    0x01, 0x01, 0x00, 0x4e, // Fixed Header
    0x00, 0x00, 0x00, 0x08,

    0x00, 0x02, 0x00, 0x32, // Type 2 == ContentObject, length 50 (0x32)
    0x00, 0x00, 0x00, 0x17, // Name, length 23 (0x17)

    0x00, 0x01, 0x00, 0x05, // NameSegment, length 5
    0x62, 0x6f, 0x6f, 0x73, // "boose"
    0x65,

    0x00, 0x01, 0x00, 0x03, // NameSegment, length 3
    0x72, 0x6f, 0x6f, // "roo"

    0x00, 0x01, 0x00, 0x03, // NameSegment, length 3
    0x70, 0x69, 0x65, // "pie"

    0x00, 0x01, 0x00, 0x13, // Payload, length 19 (0x13)
    0x74, 0x68, 0x69, 0x73, // "this is the payload"
    0x20, 0x69, 0x73, 0x20,
    0x74, 0x68, 0x65, 0x20,
    0x70, 0x61, 0x79, 0x6c,
    0x6f, 0x61, 0x64,

    0x00, 0x03, 0x00, 0x04, // Validation Alg, length 4
    0x00, 0x02, 0x00, 0x00, // CRC32, length 0

    0x00, 0x04, 0x00, 0x04, // Validation Payload, length 4
    0x7e, 0x60, 0x54, 0xc4, // The payload (the CRC32)
};

LONGBOW_TEST_CASE(SchemaV1, ccnxWireFormatFacadeV1_ComputeContentObjectHash)
{
    PARCBuffer *wireFormatBuffer = parcBuffer_Wrap(_v1_ContentObject_WithKnownHash, sizeof(_v1_ContentObject_WithKnownHash),
                                                   0, sizeof(_v1_ContentObject_WithKnownHash));

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_Create(wireFormatBuffer);

    CCNxTlvDictionary *contentObject = ccnxWireFormatMessage_GetDictionary(message);

    // We have a partially unpacked dictionary now, but we need to more fully unpack it for our processing.
    assertTrue(ccnxCodecTlvPacket_BufferDecode(wireFormatBuffer, contentObject), "Expected to decode the wireformat buffer");


    PARCCryptoHash *coHash = _ccnxWireFormatFacadeV1_ComputeContentObjectHash(contentObject);

    assertNotNull(coHash, "Expected a non-NULL content object hash");

    char *computedHash = parcBuffer_ToHexString(parcCryptoHash_GetDigest(coHash));

    char *knownHash = "4FB301EA5FD523B9A71287B721DC20C94B2D4827674A8CA275B7D57C60447876";

    assertTrue(strncasecmp(computedHash, knownHash, strlen(knownHash)) == 0, "Expected a matching ContentObject hash");

    parcMemory_Deallocate(&computedHash);
    parcCryptoHash_Release(&coHash);
    parcBuffer_Release(&wireFormatBuffer);

    ccnxContentObject_Release(&contentObject);
}

// =======================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _ccnxWireFormatFacadeV1_ComputeHash);
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


LONGBOW_TEST_CASE(Local, _ccnxWireFormatFacadeV1_ComputeHash)
{
    //                         >1234<
    const char string[] = "Hello dev null\n";
    const char substring[] = " dev";

    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));
    size_t start = 5;
    size_t length = 4;


    // compute the true hash
    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, substring, length);
    PARCCryptoHash *truthHash = parcCryptoHasher_Finalize(hasher);

    // Compute the test hash
    PARCCryptoHash *testHash = _ccnxWireFormatFacadeV1_ComputeBufferHash(buffer, hasher, start, length);

    // Test
    bool equals = parcCryptoHash_Equals(truthHash, testHash);
    assertTrue(equals, "Hashes do not match")
    {
        PARCBuffer *truthBuffer = parcCryptoHash_GetDigest(truthHash);
        PARCBuffer *testBuffer = parcCryptoHash_GetDigest(testHash);

        printf("Expected:\n");
        parcBuffer_Display(truthBuffer, 3);

        printf("Got:\n");
        parcBuffer_Display(testBuffer, 3);
    }

    parcCryptoHash_Release(&truthHash);
    parcCryptoHash_Release(&testHash);
    parcBuffer_Release(&buffer);
    parcCryptoHasher_Release(&hasher);
}

// =======================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_WireFormatFacade);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
