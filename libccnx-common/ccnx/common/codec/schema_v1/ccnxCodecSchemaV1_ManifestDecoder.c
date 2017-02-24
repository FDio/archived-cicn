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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_ManifestDecoder.h>

#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>
#include <ccnx/common/ccnx_PayloadType.h>
#include <ccnx/common/ccnx_InterestReturn.h>
#include <ccnx/common/ccnx_Manifest.h>
#include <ccnx/common/ccnx_ManifestHashGroup.h>

static bool
_decodeHashGroupMetadata(CCNxCodecTlvDecoder *decoder, CCNxManifestHashGroup *group, size_t length)
{
    size_t offset = 0;
    bool success = true;

    while (offset < length) {
        uint16_t type = ccnxCodecTlvDecoder_GetType(decoder);
        uint16_t value_length = ccnxCodecTlvDecoder_GetLength(decoder);
        PARCBuffer *value = ccnxCodecTlvDecoder_GetValue(decoder, value_length);

        offset += (4 + value_length);

        switch (type) {
            case CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_Locator: {
                char *nameString = parcBuffer_ToString(value);
                const CCNxName *locator = ccnxName_CreateFromCString(nameString);
                ccnxManifestHashGroup_SetLocator(group, locator);
                parcMemory_Deallocate(&nameString);
                ccnxName_Release((CCNxName **) &locator);
                break;
            }
            case CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_DataSize: {
                uint64_t dataSize = parcBuffer_GetUint64(value);
                ccnxManifestHashGroup_SetDataSize(group, dataSize);
                break;
            }
            case CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_BlockSize: {
                uint64_t blockSize = parcBuffer_GetUint64(value);
                ccnxManifestHashGroup_SetBlockSize(group, blockSize);
                break;
            }
            case CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_EntrySize: {
                uint64_t entrySize = parcBuffer_GetUint64(value);
                ccnxManifestHashGroup_SetEntrySize(group, entrySize);
                break;
            }
            case CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_TreeHeight: {
                uint64_t treeHeight = parcBuffer_GetUint64(value);
                ccnxManifestHashGroup_SetTreeHeight(group, treeHeight);
                break;
            }
            case CCNxCodecSchemaV1Types_CCNxManifestHashGroupMetadata_OverallDataSha256: {
                ccnxManifestHashGroup_SetOverallDataDigest(group, value);
                break;
            }
        }

        parcBuffer_Release(&value);
    }

    return success;
}

static bool
_decodeHashGroup(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, CCNxManifestHashGroup *group, size_t length)
{
    bool success = true;
    size_t offset = 0;

    while (offset < length) {
        uint16_t type = ccnxCodecTlvDecoder_GetType(decoder);
        uint16_t value_length = ccnxCodecTlvDecoder_GetLength(decoder);

        offset += (4 + value_length);

        switch (type) {
            case CCNxCodecSchemaV1Types_CCNxManifestHashGroup_Metadata: {
                success = _decodeHashGroupMetadata(decoder, group, value_length);
                if (!success) {
                    return false;
                }
                break;
            }

            case CCNxCodecSchemaV1Types_CCNxManifestHashGroup_DataPointer: {
                PARCBuffer *buffer = ccnxCodecTlvDecoder_GetValue(decoder, value_length);
                ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Data, buffer);
                parcBuffer_Release(&buffer);
                break;
            }

            case CCNxCodecSchemaV1Types_CCNxManifestHashGroup_ManifestPointer: {
                PARCBuffer *buffer = ccnxCodecTlvDecoder_GetValue(decoder, value_length);
                ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Manifest, buffer);
                parcBuffer_Release(&buffer);
                break;
            }

            default:
                // if we do not know the TLV type, put it in this container's unknown list
                success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, value_length, CCNxCodecSchemaV1TlvDictionary_Lists_MESSAGE_LIST);
                break;
        }

        if (!success) {
            CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
            ccnxCodecTlvDecoder_SetError(decoder, error);
            ccnxCodecError_Release(&error);
        }
    }

    CCNxManifestInterface *manifest = ccnxManifestInterface_GetInterface(packetDictionary);
    manifest->addHashGroup(packetDictionary, group);

    return success;
}

static bool
_decodeType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
{
    bool success = false;
    switch (type) {
        case CCNxCodecSchemaV1Types_CCNxMessage_Name: {
            success = ccnxCodecTlvUtilities_PutAsName(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
            break;
        }

        case CCNxCodecSchemaV1Types_CCNxMessage_HashGroup: {
            CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
            success = _decodeHashGroup(decoder, packetDictionary, group, length);
            ccnxManifestHashGroup_Release(&group);
            break;
        }

        default:
            // if we do not know the TLV type, put it in this container's unknown list
            success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_Lists_MESSAGE_LIST);
            break;
    }

    if (!success) {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
        ccnxCodecTlvDecoder_SetError(decoder, error);
        ccnxCodecError_Release(&error);
    }
    return success;
}

/*
 * We are given a decoder that points to the first TLV of a list of TLVs.  We keep walking the
 * list until we come to the end of the decoder.
 */
bool
ccnxCodecSchemaV1ManifestDecoder_Decode(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *manifestDictionary)
{
    return ccnxCodecTlvUtilities_DecodeContainer(decoder, manifestDictionary, _decodeType);
}
