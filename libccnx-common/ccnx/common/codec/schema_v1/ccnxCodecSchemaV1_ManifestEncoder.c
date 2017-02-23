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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BufferComposer.h>

#include <ccnx/common/ccnx_Manifest.h>
#include <ccnx/common/ccnx_ManifestHashGroup.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_ManifestEncoder.h>

#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>
#include <ccnx/common/ccnx_PayloadType.h>
#include <ccnx/common/ccnx_InterestReturn.h>

static size_t
_appendPointer(CCNxCodecTlvEncoder *encoder, CCNxManifestHashGroupPointer *ptr)
{
    const PARCBuffer *digest = ccnxManifestHashGroupPointer_GetDigest(ptr);
    CCNxManifestHashGroupPointerType type = ccnxManifestHashGroupPointer_GetType(ptr);

    ssize_t length = -1;
    switch (type) {
        case CCNxManifestHashGroupPointerType_Data:
            length = ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroup_DataPointer, (PARCBuffer *) digest);
            break;
        case CCNxManifestHashGroupPointerType_Manifest:
            length = ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroup_ManifestPointer, (PARCBuffer *) digest);
            break;
        default:
            assertTrue(false, "Invalid pointer type %d", type);
    }

    if (length < 0) {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_MISSING_MANDATORY, __func__, __LINE__, ccnxCodecTlvEncoder_Position(encoder));
        ccnxCodecTlvEncoder_SetError(encoder, error);
        ccnxCodecError_Release(&error);
    }

    return length;
}

ssize_t
_appendMetadata(CCNxCodecTlvEncoder *encoder, CCNxManifestHashGroup *group)
{
    ssize_t length = 0;

    // Pre-populate this field -- we'll come back and fill in the length after we're done
    size_t startPosition = ccnxCodecTlvEncoder_Position(encoder);
    ccnxCodecTlvEncoder_AppendContainer(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroup_Metadata, length);

    // Now append all metadata that exists in the hash group.
    const CCNxName *locator = ccnxManifestHashGroup_GetLocator(group);
    if (locator != NULL) {
        char *nameString = ccnxName_ToString(locator);
        PARCBuffer *nameBuffer = parcBuffer_AllocateCString(nameString);
        length += ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_Locator, nameBuffer);
        parcBuffer_Release(&nameBuffer);
        parcMemory_Deallocate(&nameString);
    }

    size_t dataSize = ccnxManifestHashGroup_GetDataSize(group);
    if (dataSize > 0) {
        length += ccnxCodecTlvEncoder_AppendUint64(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_DataSize, dataSize);
    }

    size_t blockSize = ccnxManifestHashGroup_GetBlockSize(group);
    if (blockSize > 0) {
        length += ccnxCodecTlvEncoder_AppendUint64(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_BlockSize, blockSize);
    }

    size_t entrySize = ccnxManifestHashGroup_GetEntrySize(group);
    if (entrySize > 0) {
        length += ccnxCodecTlvEncoder_AppendUint64(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_EntrySize, entrySize);
    }

    size_t treeSize = ccnxManifestHashGroup_GetTreeHeight(group);
    if (treeSize > 0) {
        length += ccnxCodecTlvEncoder_AppendUint64(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_TreeHeight, treeSize);
    }

    const PARCBuffer *dataDigest = ccnxManifestHashGroup_GetOverallDataDigest(group);
    if (dataDigest != NULL) {
        length += ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_OverallDataSha256, (PARCBuffer *) dataDigest);
    }

    // Rewind back to the container opening and fill in the length
    size_t endPosition = ccnxCodecTlvEncoder_Position(encoder);
    ccnxCodecTlvEncoder_PutUint16(encoder, startPosition, CCNxCodecSchemaV1Types_CCNxManifestHashGroup_Metadata);
    ccnxCodecTlvEncoder_PutUint16(encoder, startPosition + 2, length);
    ccnxCodecTlvEncoder_SetPosition(encoder, endPosition);

    return endPosition - startPosition;
}

ssize_t
ccnxCodecSchemaV1ManifestEncoder_Encode(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = 0;

    ssize_t numHashGroups = ccnxTlvDictionary_ListSize(packetDictionary, CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST);
    for (size_t i = 0; i < numHashGroups; i++) {
        // Skip past the TL of the hash group to append the pointers inside
        ssize_t groupLength = 0;
        ccnxCodecTlvEncoder_AppendContainer(encoder, CCNxCodecSchemaV1Types_CCNxMessage_HashGroup, groupLength);

        CCNxManifestInterface *interface = ccnxManifestInterface_GetInterface(packetDictionary);
        CCNxManifestHashGroup *group = interface->getHashGroup(packetDictionary, i);

        // Encode any metadata, if present.
        if (ccnxManifestHashGroup_HasMetadata(group)) {
            groupLength += _appendMetadata(encoder, group);
        }

        // Append the HashGroup pointers
        size_t numPointers = ccnxManifestHashGroup_GetNumberOfPointers(group);
        for (size_t p = 0; p < numPointers; p++) {
            CCNxManifestHashGroupPointer *ptr = ccnxManifestHashGroup_GetPointerAtIndex(group, p);
            ssize_t ptrLength = _appendPointer(encoder, ptr);
            if (ptrLength < 0) {
                return ptrLength;
            }
            groupLength += ptrLength;
        }

        // Now that we know the overall length, rewind back to the start and append the TL
        // part of the container.
        size_t endPosition = ccnxCodecTlvEncoder_Position(encoder);
        ssize_t offset = endPosition - groupLength - 4;
        ccnxCodecTlvEncoder_PutUint16(encoder, offset, CCNxCodecSchemaV1Types_CCNxMessage_HashGroup);
        ccnxCodecTlvEncoder_PutUint16(encoder, offset + 2, groupLength);
        ccnxCodecTlvEncoder_SetPosition(encoder, endPosition);

        length += groupLength + 4;

        ccnxManifestHashGroup_Release(&group);
    }

    return length;
}
