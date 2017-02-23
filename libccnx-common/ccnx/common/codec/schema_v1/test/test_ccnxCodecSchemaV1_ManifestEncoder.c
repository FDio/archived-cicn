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
#include "../ccnxCodecSchemaV1_ManifestEncoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include "testrig_encoder.c"

#include <ccnx/common/ccnx_Manifest.h>

// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_ManifestEncoder)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_ManifestEncoder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_ManifestEncoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


// =========================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeEmpty);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeSingleHashGroup);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeSingleHashGroup_WithMetadata);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_AddPointer);
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

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeEmpty)
{
    CCNxName *locator = ccnxName_CreateFromCString("lci:/name");
    CCNxManifest *manifest = ccnxManifest_Create(locator);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    size_t result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, manifest);

    assertTrue(result == 0, "Expected an empty Manifest to be encoded to size 0, got %zu", result);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxManifest_Release(&manifest);
    ccnxName_Release(&locator);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_AddPointer)
{
    CCNxName *locator = ccnxName_CreateFromCString("ccnx:/name");
    CCNxManifest *manifest = ccnxManifest_Create(locator);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    PARCBuffer *pointer = parcBuffer_Flip(parcBuffer_ParseHexString("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
    ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Data, pointer);

    ccnxManifest_AddHashGroup(manifest, group);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    size_t result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, manifest);
    size_t expected = 4 + 4 + parcBuffer_Remaining(pointer); // hash group TL, pointer TL, pointer V

    assertTrue(result == expected, "Expected an empty Manifest to be encoded to size %zu, got %zu", expected, result);

    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvEncoder_CreateIoVec(encoder);
    const struct iovec *vector = ccnxCodecNetworkBufferIoVec_GetArray(iovec);

    assertTrue(vector->iov_len == expected, "Expected the IO vector to contain the encoded manifest");
    assertTrue(memcmp(vector->iov_base + 8, parcBuffer_Overlay(pointer, parcBuffer_Remaining(pointer)), vector->iov_len - 8) == 0, "Expected the same pointer to be encoded");

    uint16_t expectedType = CCNxCodecSchemaV1Types_CCNxManifestHashGroup_DataPointer;
    uint8_t *base = (uint8_t *) vector->iov_base;
    uint16_t actualType = (base[4] << 8) | base[5];
    assertTrue(expectedType == actualType, "Expected the type to be written correctly as CCNxCodecSchemaV1Types_CCNxManifestHashGroup_DataPointer");

    ccnxCodecNetworkBufferIoVec_Release(&iovec);

    // Cleanup
    parcBuffer_Release(&pointer);
    ccnxManifestHashGroup_Release(&group);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxManifest_Release(&manifest);
    ccnxName_Release(&locator);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeSingleHashGroup)
{
    CCNxName *locator = ccnxName_CreateFromCString("ccnx:/name");
    CCNxManifest *manifest = ccnxManifest_Create(locator);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    PARCBuffer *pointer = parcBuffer_Flip(parcBuffer_ParseHexString("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
    ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Data, pointer);

    ccnxManifest_AddHashGroup(manifest, group);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    size_t result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, manifest);
    size_t expected = 4 + 4 + parcBuffer_Remaining(pointer); // hash group TL, pointer TL, pointer V

    assertTrue(result == expected, "Expected an empty Manifest to be encoded to size %zu, got %zu", expected, result);

    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvEncoder_CreateIoVec(encoder);
    const struct iovec *vector = ccnxCodecNetworkBufferIoVec_GetArray(iovec);

    uint8_t expectedVector[24] = { 0x00, 0x07, 0x00, 0x14,
                                   0x00, 0x02, 0x00, 0x10,
                                   0xFF, 0xFF, 0xFF, 0xFF,
                                   0xFF, 0xFF, 0xFF, 0xFF,
                                   0xFF, 0xFF, 0xFF, 0xFF,
                                   0xFF, 0xFF, 0xFF, 0xFF };
    assertTrue(vector->iov_len == expected, "Expected the IO vector to contain the encoded manifest");
    assertTrue(memcmp(vector->iov_base, expectedVector, vector->iov_len) == 0, "Expected the same pointer to be encoded");

    ccnxCodecNetworkBufferIoVec_Release(&iovec);

    // Cleanup
    parcBuffer_Release(&pointer);
    ccnxManifestHashGroup_Release(&group);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxManifest_Release(&manifest);
    ccnxName_Release(&locator);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestEncoder_EncodeSingleHashGroup_WithMetadata)
{
    CCNxName *locator = ccnxName_CreateFromCString("ccnx:/name");
    CCNxManifest *manifest = ccnxManifest_Create(locator);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    PARCBuffer *pointer = parcBuffer_Flip(parcBuffer_ParseHexString("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
    ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Data, pointer);

    // Set the metadata now
    CCNxName *groupLocator = ccnxName_CreateFromCString("ccnx:/locator");
    ccnxManifestHashGroup_SetLocator(group, groupLocator);

    PARCBuffer *digest = parcBuffer_Allocate(16);
    for (size_t i = 0; i < parcBuffer_Limit(digest); i++) {
        parcBuffer_PutUint8(digest, 0);
    }
    parcBuffer_Flip(digest);
    ccnxManifestHashGroup_SetOverallDataDigest(group, digest);

    size_t entrySize = 1;
    ccnxManifestHashGroup_SetEntrySize(group, entrySize);

    size_t dataSize = 2;
    ccnxManifestHashGroup_SetDataSize(group, dataSize);

    size_t blockSize = 3;
    ccnxManifestHashGroup_SetBlockSize(group, blockSize);

    size_t treeHeight = 4;
    ccnxManifestHashGroup_SetTreeHeight(group, treeHeight);

    // Add the hash group to the manifest
    ccnxManifest_AddHashGroup(manifest, group);

    CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
    size_t result = ccnxCodecSchemaV1ManifestEncoder_Encode(encoder, manifest);

    // Compute the expected size with all of the metadata.
    // This packet was crafted by hand.
    size_t expected = 4; // hash group TL
    expected += 4 + parcBuffer_Remaining(pointer); // pointer TL, pointer V
    expected += 4; // metadata TL
    expected += 4 * (4 + 8); // 64-bit integer property TLs, Vs
    expected += 4 + parcBuffer_Remaining(digest); // digest T and V
    expected += 4 + strlen("ccnx:/locator"); // name TL, segment TL, and segment V

    // Compute only the size of the metadata
    size_t metadataSize = expected;
    metadataSize -= 4; // top-level TL container
    metadataSize -= (4 + parcBuffer_Remaining(pointer)); // pointer TLV
    metadataSize -= 4; // metadata TL container

    assertTrue(result == expected, "Expected an empty Manifest to be encoded to size %zu, got %zu", expected, result);

    // Now do the encoding
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvEncoder_CreateIoVec(encoder);
    const struct iovec *vector = ccnxCodecNetworkBufferIoVec_GetArray(iovec);

    uint8_t expectedVector[129] = { 0x00, 0x07,
                                    0x00, expected - 4,
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroup_Metadata,
                                    0x00, metadataSize,
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_Locator,
                                    0x00, strlen("ccnx:/locator"),
                                    'c',  'c',
                                    'n',  'x',
                                    ':',  '/',
                                    'l',  'o',
                                    'c',  'a',
                                    't',  'o',
                                    'r',
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_DataSize,
                                    0x00, 0x08,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, dataSize,
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_BlockSize,
                                    0x00, 0x08,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, blockSize,
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_EntrySize,
                                    0x00, 0x08,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, entrySize,
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_TreeHeight,
                                    0x00, 0x08,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, treeHeight,
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_OverallDataSha256,
                                    0x00, parcBuffer_Remaining(digest),
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroup_DataPointer,
                                    0x00, parcBuffer_Remaining(pointer),
                                    0xFF, 0xFF,
                                    0xFF, 0xFF,
                                    0xFF, 0xFF,
                                    0xFF, 0xFF,
                                    0xFF, 0xFF,
                                    0xFF, 0xFF,
                                    0xFF, 0xFF,
                                    0xFF, 0xFF };
    assertTrue(vector->iov_len == expected, "Expected the IO vector to contain the encoded manifest");
    assertTrue(memcmp(vector->iov_base, expectedVector, vector->iov_len) == 0, "Expected the same HashGroup to be encoded");

    ccnxCodecNetworkBufferIoVec_Release(&iovec);

    // Cleanup
    parcBuffer_Release(&pointer);
    parcBuffer_Release(&digest);
    ccnxManifestHashGroup_Release(&group);
    ccnxName_Release(&groupLocator);

    ccnxCodecTlvEncoder_Destroy(&encoder);
    ccnxManifest_Release(&manifest);
    ccnxName_Release(&locator);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_ManifestEncoder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
