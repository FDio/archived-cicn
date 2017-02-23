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
#include "../ccnxCodecSchemaV1_ManifestDecoder.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include "testrig_encoder.c"

#include <ccnx/common/ccnx_Manifest.h>

// =========================================================================

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_ManifestDecoder)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_ManifestDecoder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_ManifestDecoder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


// =========================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_Decode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_DecodeType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_DecodeHashGroup);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_DecodeHashGroupMetadata);
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

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_Decode)
{
    uint8_t rawManifest[40] = { 0x00, 0x07, 0x00, 0x24,
                                0x00, 0x02, 0x00, 0x20,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46 };
    PARCBuffer *wireFormat = parcBuffer_Flip(parcBuffer_CreateFromArray(rawManifest, 40));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(wireFormat);
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateManifest();

    bool result = ccnxCodecSchemaV1ManifestDecoder_Decode(decoder, dict);
    assertTrue(result, "Expected the manifest to be decoded correctly");

    ccnxTlvDictionary_Release(&dict);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&wireFormat);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_DecodeType)
{
    uint8_t rawManifest[40] = { 0x00, 0x07, 0x00, 0x24,
                                0x00, 0x02, 0x00, 0x20,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46 };
    PARCBuffer *wireFormat = parcBuffer_Flip(parcBuffer_CreateFromArray(rawManifest, 40));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(wireFormat);
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateManifest();

    uint16_t type = ccnxCodecTlvDecoder_GetType(decoder);
    uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

    bool result = _decodeType(decoder, dict, type, length);
    assertTrue(result, "Expected the manifest type to be correctly decoded at the top level");

    ccnxManifest_Release(&dict);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&wireFormat);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_DecodeHashGroup)
{
    uint8_t rawManifest[40] = { 0x00, 0x07, 0x00, 0x24,
                                0x00, 0x02, 0x00, 0x20,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46,
                                0x46, 0x46, 0x46, 0x46 };
    PARCBuffer *wireFormat = parcBuffer_Flip(parcBuffer_CreateFromArray(rawManifest, 40));

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(wireFormat);
    CCNxTlvDictionary *dict = ccnxCodecSchemaV1TlvDictionary_CreateManifest();

    ccnxCodecTlvDecoder_GetType(decoder); // swallow type
    uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    bool result = _decodeHashGroup(decoder, dict, group, length);
    assertTrue(result, "Expected hash group to be decoded correctly.");

    PARCBuffer *expectedPointer = parcBuffer_AllocateCString("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    CCNxManifestHashGroupPointer *pointer = ccnxManifestHashGroup_GetPointerAtIndex(group, 0);
    const PARCBuffer *actualPointer = ccnxManifestHashGroupPointer_GetDigest(pointer);
    assertTrue(parcBuffer_Equals(expectedPointer, actualPointer), "Expected decoded pointer to equal %s, got %s", parcBuffer_ToHexString(expectedPointer), parcBuffer_ToHexString(actualPointer));

    parcBuffer_Release(&expectedPointer);

    ccnxManifestHashGroup_Release(&group);
    ccnxManifest_Release(&dict);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&wireFormat);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1ManifestDecoder_DecodeHashGroupMetadata)
{
    // Re-build the expected metadata from the manifest
    CCNxName *groupLocator = ccnxName_CreateFromCString("ccnx:/locator");
    PARCBuffer *digest = parcBuffer_Allocate(16);
    for (size_t i = 0; i < parcBuffer_Limit(digest); i++) {
        parcBuffer_PutUint8(digest, 0);
    }
    parcBuffer_Flip(digest);
    size_t entrySize = 1;
    size_t dataSize = 2;
    size_t blockSize = 3;
    size_t treeHeight = 4;

    // Compute the expected size of this metadata group.
    size_t metadataSize = 4 * (4 + 8) + 4 + parcBuffer_Limit(digest) + 4 + strlen("ccnx:/locator");

    // See test_ccnxCodecSchemaV1_ManifestEncoder.c for the packet construction details.
    uint8_t rawMetadata[89] = { 0x00, CCNxCodecSchemaV1Types_CCNxManifestHashGroup_Metadata,
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
                                0x00, 0x00 };
    PARCBuffer *wireFormat = parcBuffer_Flip(parcBuffer_CreateFromArray(rawMetadata, sizeof(rawMetadata)));

    // Create the encoder and swallow the top level container
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(wireFormat);
    ccnxCodecTlvDecoder_GetType(decoder); // swallow type
    uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

    // Decode the metadata
    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    bool result = _decodeHashGroupMetadata(decoder, group, length);
    assertTrue(result, "Expected hash group metadata to be decoded correctly.");

    const CCNxName *actualLocator = ccnxManifestHashGroup_GetLocator(group);
    size_t actualEntrySize = ccnxManifestHashGroup_GetEntrySize(group);
    size_t actualDataSize = ccnxManifestHashGroup_GetDataSize(group);
    size_t actualBlockSize = ccnxManifestHashGroup_GetBlockSize(group);
    size_t actualTreeHeight = ccnxManifestHashGroup_GetTreeHeight(group);
    const PARCBuffer *actualDigest = ccnxManifestHashGroup_GetOverallDataDigest(group);

    assertTrue(ccnxName_Equals(groupLocator, actualLocator), "Expected decoded locator to equal %s, got %s", ccnxName_ToString(groupLocator), ccnxName_ToString(actualLocator));
    assertTrue(entrySize == actualEntrySize, "Expected %zu entry size, got %zu", entrySize, actualEntrySize);
    assertTrue(dataSize == actualDataSize, "Expected %zu data size, got %zu", dataSize, actualDataSize);
    assertTrue(blockSize == actualBlockSize, "Expected %zu block size, got %zu", blockSize, actualBlockSize);
    assertTrue(treeHeight == actualTreeHeight, "Expected %zu tree height, got %zu", treeHeight, actualTreeHeight);
    assertTrue(parcBuffer_Equals(digest, actualDigest), "Expected %s digest, got %s", parcBuffer_ToHexString(digest), parcBuffer_ToHexString(actualDigest));

    parcBuffer_Release(&digest);
    ccnxName_Release(&groupLocator);

    ccnxManifestHashGroup_Release(&group);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    parcBuffer_Release(&wireFormat);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_ManifestDecoder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
