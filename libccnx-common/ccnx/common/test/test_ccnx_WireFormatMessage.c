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
// This permits internal static functions to be visible to this Test Runner.
#include "../ccnx_WireFormatMessage.c"

#include <stdio.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

#include <ccnx/common/codec/ccnxCodec_TlvPacket.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_nameA.h>
#include <ccnx/common/ccnx_ContentObject.h>


LONGBOW_TEST_RUNNER(ccnx_WireFormatMessage)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_WireFormatMessage)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_WireFormatMessage)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_FromContentObjectPacketType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_FromControlPacketType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_FromInterestPacketType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_FromInterestPacketTypeIoVec);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_GetDictionary);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_PutGetIoVec);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_GetWireFormatBuffer);
    //
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_CreateContentObjectHash);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_HashProtectedRegion);
    //
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_PutWireFormatBuffer);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_SetProtectedRegionLength);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_SetProtectedRegionStart);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_WriteToFile);
    LONGBOW_RUN_TEST_CASE(Global, ccnxWireFormatMessage_SetHopLimit);
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

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_AcquireRelease)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromInterestPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    CCNxWireFormatMessage *ref = ccnxWireFormatMessage_Acquire(message);
    assertNotNull(ref, "Expected a non-NULL reference to be acquired");

    ccnxWireFormatMessage_Release(&message);
    assertNotNull(ref, "Expected a non-NULL reference to be acquired");
    assertNull(message, "Expeced original message to be NULL");

    ccnxWireFormatMessage_Release(&ref);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_AssertValid)
{
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_Create(wireFormat);
    assertNotNull(message, "Got null CCNxWireFormatMessage, after attempting to create with buffer");

    ccnxWireFormatMessage_AssertValid(message);

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&wireFormat);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_FromContentObjectPacketType)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromContentObjectPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    assertTrue(ccnxTlvDictionary_IsContentObject((CCNxTlvDictionary *) message), "Wrong message type");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_FromControlPacketType)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromControlPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    assertTrue(ccnxTlvDictionary_IsControl((CCNxTlvDictionary *) message), "Wrong message type");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_FromInterestPacketType)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromInterestPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    assertTrue(ccnxTlvDictionary_IsInterest((CCNxTlvDictionary *) message), "Wrong message type");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_Create)
{
    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_Create(wireFormat);
    assertNotNull(message, "Got null CCNxWireFormatMessage, after attempting to create with buffer");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&wireFormat);
}

/***
 ***  allocator(), deallocator(), createNetworkBufferIoVec() taken from test_ccnx_WireFormatFacadeV1.c
 ***/

/*
 * Small size allocator for creating a network buffer
 */
static size_t
_allocator(void *userarg, size_t bytes, void **output)
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
_deallocator(void *userarg, void **memoryPtr)
{
    parcMemory_Deallocate(memoryPtr);
}
static const CCNxCodecNetworkBufferMemoryBlockFunctions memory = { .allocator = _allocator, .deallocator = _deallocator };

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
_createNetworkBufferIoVec(size_t allocationSize, size_t padlen, uint8_t pad[padlen], size_t datalen, uint8_t data[datalen])
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

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_FromInterestPacketTypeIoVec)
{
    uint8_t data[64];

    uint8_t pad[32];
    CCNxCodecNetworkBufferIoVec *vec = _createNetworkBufferIoVec(512, 32, pad, 64, data);

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromInterestPacketTypeIoVec(CCNxTlvDictionary_SchemaVersion_V1, vec);

    assertNotNull(message, "Got null CCNxWireFormatMessage");
    assertTrue(ccnxTlvDictionary_IsInterest(message), "Wrong message type");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(message) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(message), CCNxTlvDictionary_SchemaVersion_V1);

    ccnxWireFormatMessage_Release(&message);
    ccnxCodecNetworkBufferIoVec_Release(&vec);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_GetDictionary)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromInterestPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    CCNxTlvDictionary *dictionary = ccnxWireFormatMessage_GetDictionary(message);
    assertNotNull(dictionary, "Expected to retrieve the dictionary from the CCNxWireFormatMessage");
    assertTrue(ccnxTlvDictionary_IsInterest(dictionary), "Wrong message type");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_PutGetIoVec)
{
    uint8_t *data = parcMemory_Allocate(64);
    memset(data, 0, 64);

    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, 64, data);
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);

    CCNxTlvDictionary *packet = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxWireFormatMessage_PutIoVec((CCNxWireFormatMessage *) packet, iovec);

    CCNxCodecNetworkBufferIoVec *test = ccnxWireFormatMessage_GetIoVec((CCNxWireFormatMessage *) packet);
    assertTrue(test == iovec, "Failed to get iovec from dictionary, expected %p got %p", (void *) iovec, (void *) test);

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    ccnxCodecNetworkBuffer_Release(&netbuff);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_GetWireFormatBuffer)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromInterestPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    PARCBuffer *test = ccnxWireFormatMessage_GetWireFormatBuffer(message);
    assertTrue(test == buffer, "Retrieved unexpected buffer: got %p expected %p", (void *) test, (void *) buffer);

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_PutWireFormatBuffer)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxTlvDictionary *packet = ccnxTlvDictionary_Create(20, 20);
    ccnxTlvDictionary_SetMessageType_Interest(packet, CCNxTlvDictionary_SchemaVersion_V1);
    bool success = ccnxWireFormatMessage_PutWireFormatBuffer(packet, buffer);

    assertTrue(success, "Failed to put buffer in to dictionary");

    PARCBuffer *test = ccnxWireFormatMessage_GetWireFormatBuffer(packet);
    assertTrue(test == buffer, "Retrieved unexpected buffer: got %p expected %p", (void *) test, (void *) buffer);

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_HashProtectedRegion)
{
    //                         >1234<
    const char string[] = "Hello dev null\n";

    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));
    size_t start = 5;
    size_t length = 4;

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromContentObjectPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    ccnxWireFormatMessage_SetProtectedRegionStart(message, start);
    ccnxWireFormatMessage_SetProtectedRegionLength(message, length);

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    PARCCryptoHash *hash = ccnxWireFormatMessage_HashProtectedRegion(message, hasher);

    // the correctness of the has is tested in _ccnxWireFormatFacadeV1_ComputeHash
    assertNotNull(hash, "Got null hash from a good packet");

    ccnxWireFormatMessage_Release(&message);
    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&buffer);
    parcCryptoHasher_Release(&hasher);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_SetProtectedRegionLength)
{
    const char string[] = "Hello dev null\n";
    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromContentObjectPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    size_t length = 5;
    bool success = ccnxWireFormatMessage_SetProtectedRegionLength(message, length);
    assertTrue(success, "Failed to put integer in to dictionary");

    assertTrue(ccnxTlvDictionary_IsValueInteger(message, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ProtectedLength), "ProtectedLength not set");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_SetProtectedRegionStart)
{
    const char string[] = "Hello dev null\n";
    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromContentObjectPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    size_t start = 5;
    bool success = ccnxWireFormatMessage_SetProtectedRegionStart(message, start);
    assertTrue(success, "Failed to put integer in to dictionary");

    assertTrue(ccnxTlvDictionary_IsValueInteger(message, CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ProtectedStart), "ProtectedStart not set");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

static PARCBuffer *
_iovecToParcBuffer(const CCNxCodecNetworkBufferIoVec *iovec)
{
    PARCBuffer *result = NULL;

    size_t iovcnt = ccnxCodecNetworkBufferIoVec_GetCount((CCNxCodecNetworkBufferIoVec *) iovec);
    const struct iovec *array = ccnxCodecNetworkBufferIoVec_GetArray((CCNxCodecNetworkBufferIoVec *) iovec);

    size_t totalbytes = 0;
    for (int i = 0; i < iovcnt; i++) {
        totalbytes += array[i].iov_len;
    }

    result = parcBuffer_Allocate(totalbytes);
    for (int i = 0; i < iovcnt; i++) {
        parcBuffer_PutArray(result, array[i].iov_len, array[i].iov_base);
    }

    parcBuffer_Flip(result);


    return result;
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_CreateContentObjectHash)
{
    //                         >1234<
    const char string[] = "Hello dev null\n";

    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromContentObjectPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    PARCCryptoHash *hash = ccnxWireFormatMessage_CreateContentObjectHash(message);
    ccnxWireFormatMessage_Release(&message);
    assertNull(hash, "Expect NULL for hash as it hasn't been encoded yet");

    // We need to create a content object that is hashable
    CCNxName *name = ccnxName_CreateFromCString("lci:/test/content");
    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, buffer);
    ccnxName_Release(&name);

    // This next stuff is to force an encode/decode to setup hash extents
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvPacket_DictionaryEncode(contentObject, NULL);
    ccnxContentObject_Release(&contentObject);
    PARCBuffer *encodedMessage = _iovecToParcBuffer(iovec);
    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    // Decode
    message = ccnxWireFormatMessage_Create(encodedMessage);
    CCNxTlvDictionary *dictionary = ccnxWireFormatMessage_GetDictionary(message);
    bool success = ccnxCodecTlvPacket_BufferDecode(encodedMessage, dictionary);
    assertTrue(success, "Failed to decode buffer");
    parcBuffer_Release(&encodedMessage);

    hash = ccnxWireFormatMessage_CreateContentObjectHash(message);

    // the correctness of the hash is tested in _ccnxWireFormatFacadeV1_ComputeHash
    assertNotNull(hash, "Got null hash from a good packet");

    ccnxWireFormatMessage_Release(&message);
    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_WriteToFile)
{
    const char string[] = "Hello dev null\n";
    PARCBuffer *buffer = parcBuffer_Wrap((void *) string, sizeof(string), 0, sizeof(string));
    CCNxWireFormatMessage *message = ccnxWireFormatMessage_FromInterestPacketType(CCNxTlvDictionary_SchemaVersion_V1, buffer);

    ccnxWireFormatMessage_WriteToFile(message, "/dev/null");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxWireFormatMessage_SetHopLimit)
{
    uint8_t *data = parcMemory_Allocate(64);
    memset(data, 0, 64);

    PARCBuffer *buffer = parcBuffer_Allocate(1);
    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, 64, data);
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);

    CCNxTlvDictionary *packet = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    ccnxWireFormatMessage_PutIoVec((CCNxWireFormatMessage *) packet, iovec);

    ccnxWireFormatMessage_SetHopLimit(packet, 10);

    CCNxCodecNetworkBufferIoVec *test = ccnxWireFormatMessage_GetIoVec((CCNxWireFormatMessage *) packet);
    assertTrue(test == iovec, "Failed to get iovec from dictionary, expected %p got %p", (void *) iovec, (void *) test);

    ccnxTlvDictionary_Release(&packet);
    parcBuffer_Release(&buffer);
    ccnxCodecNetworkBufferIoVec_Release(&iovec);
    ccnxCodecNetworkBuffer_Release(&netbuff);
}
LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, _getImplForSchema);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxWireFormatMessage_CreateWithImpl);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Static, _ccnxWireFormatMessage_CreateWithImpl)
{
    PARCBuffer *wireFormatV1 = parcBuffer_Wrap(v1_interest_nameA, sizeof(v1_interest_nameA), 0, sizeof(v1_interest_nameA));

    CCNxWireFormatMessage *message = _ccnxWireFormatMessage_CreateWithImpl(&CCNxWireFormatFacadeV1_Implementation, wireFormatV1);
    assertNotNull(message, "Expected to create a V1 CCNxWireFormatMessage");

    ccnxWireFormatMessage_Release(&message);
    parcBuffer_Release(&wireFormatV1);
}

LONGBOW_TEST_CASE(Static, _getImplForSchema)
{
    CCNxWireFormatMessageInterface *impl = _getImplForSchema(CCNxTlvDictionary_SchemaVersion_V1);
    assertTrue(impl = &CCNxWireFormatFacadeV1_Implementation, "Expected to see CCNxWireFormatFacadeV1_Implementation");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_WireFormatMessage);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
