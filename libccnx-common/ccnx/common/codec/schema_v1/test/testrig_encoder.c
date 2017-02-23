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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_EncodingBuffer.h>

#include <ccnx/common/codec/schema_v1/testdata/v1_InterestSchema.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_ContentObjectSchema.h>


#include <ccnx/common/codec/schema_v1/testdata/v1_testrig_truthTable.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>

#include <ccnx/common/codec/test/testrig_Compare.c>

/**
 * Finds a row in the truthtable where bodyManifest is TRUE and the
 * indexOrKey equals 'key'.
 */
TlvExtent
getTruthTableExtent(TruthTableEntry *ttentry, int key)
{
    for (int i = 0; ttentry[i].indexOrKey != T_INVALID; i++) {
        if (ttentry[i].bodyManifest && ttentry[i].indexOrKey == key) {
            return ttentry[i].extent;
        }
    }
    return (TlvExtent) { 0, 0 };
}

/**
 * Finds a row in the truthtable where bodyManifest is FALSE and the
 * indexOrKey equals 'key'.
 */
TlvExtent
getTruthTableHeaderExtent(TruthTableEntry *ttentry, int key)
{
    for (int i = 0; ttentry[i].indexOrKey != T_INVALID; i++) {
        if (!ttentry[i].bodyManifest && ttentry[i].indexOrKey == key) {
            return ttentry[i].extent;
        }
    }
    return (TlvExtent) { 0, 0 };
}

typedef struct test_data {
    // the memory region extracted from a Truth Table entry
    PARCBuffer *memoryRegion;

    CCNxCodecTlvEncoder *encoder;
    CCNxTlvDictionary *dictionary;

    uint8_t *packet;
    size_t packetLength;
    TruthTableEntry *truthTable;

    // If the user creates one of these, we'll destroy it.
    PARCSigner *signer;
} TestData;

static SchemaV1ManifestContentObjectBody manifestContentObjectContainerArray[] = {
    V1_MANIFEST_OBJ_CONTENTOBJECT,
    V1_MANIFEST_OBJ_NAMEAUTH,
    V1_MANIFEST_OBJ_ValidationPayload,
    V1_MANIFEST_OBJ_KEYNAME,
    V1_MANIFEST_OBJ_METADATA,
    V1_MANIFEST_OBJ_ValidationAlg,
    V1_MANIFEST_OBJ_BODYEND
};

static bool
isContentObjectContainer(SchemaV1ManifestContentObjectBody value)
{
    for (int i = 0; manifestContentObjectContainerArray[i] != V1_MANIFEST_OBJ_BODYEND; i++) {
        if (value == manifestContentObjectContainerArray[i]) {
            return true;
        }
    }
    return false;
}

/**
 * The testdata truth tables were written with the tlv_1.0 array indicies, so we
 * need to translate those old indicies to the new indicies.
 */
static CCNxCodecSchemaV1TlvDictionary_MessageFastArray
translateTestDataManifestToSchemaKey(SchemaV1ManifestContentObjectBody oldKey)
{
    switch (oldKey) {
        case V1_MANIFEST_INT_NAME:
        // fallthrough
        case V1_MANIFEST_OBJ_NAME:
            return CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME;

        case V1_MANIFEST_OBJ_PAYLOAD:
            return CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD;

        case V1_MANIFEST_OBJ_KEYID:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYID;

        case V1_MANIFEST_OBJ_CRYPTO_SUITE:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE;

        case V1_MANIFEST_OBJ_KEY:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEY;

        case V1_MANIFEST_OBJ_CERT:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CERT;

        case V1_MANIFEST_OBJ_KEYNAME_NAME:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_NAME;

        case V1_MANIFEST_OBJ_KEYNAME_OBJHASH:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_OBJHASH;

        case V1_MANIFEST_OBJ_OBJ_TYPE:
            return CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE;

        case V1_MANIFEST_OBJ_SIGBITS:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_PAYLOAD;

        case V1_MANIFEST_OBJ_SigningTime:
            return (CCNxCodecSchemaV1TlvDictionary_MessageFastArray) CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_SIGNTIME;

        case V1_MANIFEST_OBJ_ENDSEGMENT:
            return CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT;

        case V1_MANIFEST_INT_KEYID:
            return CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION;

        case V1_MANIFEST_INT_OBJHASH:
            return CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION;

        default:
            trapIllegalValue(oldKey, "Unexpected old manifest value: %d", oldKey);
            break;
    }
    return -1;
}

/**
 * The testdata truth tables were written with the tlv_1.0 array indicies, so we
 * need to translate those old indicies to the new indicies.
 */
static CCNxCodecSchemaV1TlvDictionary_HeadersFastArray
translateOldOptionalHeadersManifestToNewKey(CCNxTlvDictionary *packetDictionary, int oldKey)
{
    if (ccnxTlvDictionary_IsInterest(packetDictionary)) {
        switch (oldKey) {
            case V1_MANIFEST_INT_LIFETIME:
                return CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime;

            case V1_MANIFEST_INT_E2EFRAG:
                return CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG;

            default:
                trapIllegalValue(oldKey, "Unexpected old manifest value: %d", oldKey);
                break;
        }
    } else if (ccnxTlvDictionary_IsContentObject(packetDictionary)) {
        switch (oldKey) {
            case V1_MANIFEST_OBJ_E2EFRAG:
                return CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_OBJFRAG;

            default:
                trapIllegalValue(oldKey, "Unexpected old manifest value: %d", oldKey);
                break;
        }
    }
    return -1;
}

static void
addBuffer(TestData *data, CCNxTlvDictionary *packetDictionary, size_t item_start, size_t item_end, uint32_t translatedKey)
{
    PARCBuffer *itemBuffer = parcBuffer_Wrap(data->packet, data->packetLength, item_start, item_end);
    ccnxTlvDictionary_PutBuffer(packetDictionary, translatedKey, itemBuffer);
    parcBuffer_Release(&itemBuffer);
}

/**
 * The extent should be treated like a CCNxName, so decode it and add it as a CCNxName.
 */
static void
addName(TestData *data, CCNxTlvDictionary *packetDictionary, size_t item_start, size_t item_end, uint32_t translatedKey)
{
    // we need to backup 4 bytes to the the TLV container
    PARCBuffer *itemBuffer = parcBuffer_Wrap(data->packet, data->packetLength, item_start - 4, item_end);

    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(itemBuffer);
    CCNxName *name = ccnxCodecSchemaV1NameCodec_Decode(decoder, CCNxCodecSchemaV1Types_CCNxMessage_Name);
    ccnxCodecTlvDecoder_Destroy(&decoder);

    ccnxTlvDictionary_PutName(packetDictionary, translatedKey, name);
    ccnxName_Release(&name);
    parcBuffer_Release(&itemBuffer);
}


/**
 * Called on the body of a content object, does not include the fixed header or optional headers
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
buildContentObjectDictionary(TestData *data, CCNxTlvDictionary *packetDictionary, TlvExtent extent)
{
    size_t start = extent.offset;
    size_t end = start + extent.length;

    ccnxTlvDictionary_SetMessageType_ContentObject(packetDictionary, CCNxTlvDictionary_SchemaVersion_V1);

    for (int i = 0; data->truthTable[i].indexOrKey != T_INVALID; i++) {
        size_t item_start = data->truthTable[i].extent.offset;
        size_t item_end = item_start + data->truthTable[i].extent.length;

        // Is this item included in the given extent?
        if (start < item_start && item_end <= end) {
            // Is it a container or a nested dictionary?  This check only applies to a ContentObject
            if (!isContentObjectContainer(data->truthTable[i].indexOrKey)) {
                uint32_t translatedKey = translateTestDataManifestToSchemaKey(data->truthTable[i].indexOrKey);

                if (data->truthTable[i].indexOrKey == V1_MANIFEST_OBJ_NAME) {
                    addName(data, packetDictionary, item_start, item_end, translatedKey);
                } else {
                    addBuffer(data, packetDictionary, item_start, item_end, translatedKey);
                }
            }
        }
    }
}

static void
buildInterestDictionary(TestData *data, CCNxTlvDictionary *packetDictionary, TlvExtent extent)
{
    size_t start = extent.offset;
    size_t end = start + extent.length;

    ccnxTlvDictionary_SetMessageType_Interest(packetDictionary, CCNxTlvDictionary_SchemaVersion_V1);

    for (int i = 0; data->truthTable[i].indexOrKey != T_INVALID && data->truthTable[i].bodyManifest; i++) {
        size_t item_start = data->truthTable[i].extent.offset;
        size_t item_end = item_start + data->truthTable[i].extent.length;

        // Is this item included in the given extent?
        if (start < item_start && item_end <= end) {
            uint32_t translatedKey = translateTestDataManifestToSchemaKey(data->truthTable[i].indexOrKey);

            if (data->truthTable[i].indexOrKey == V1_MANIFEST_INT_NAME) {
                addName(data, packetDictionary, item_start, item_end, translatedKey);
            } else {
                addBuffer(data, packetDictionary, item_start, item_end, translatedKey);
            }
        }
    }
}

/**
 * Make a dictionary entry for everything inside the selected extent, not including it
 *
 * Use the truth table and for each listed item whose extent is within the given extent,
 * add a dictionary entry
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
buildMessageDictionary(TestData *data, CCNxTlvDictionary *dictionary, TlvExtent extent)
{
    uint8_t packetType = data->packet[1];
    switch (packetType) {
        case CCNxCodecSchemaV1Types_PacketType_Interest:
            buildInterestDictionary(data, dictionary, extent);
            break;

        case CCNxCodecSchemaV1Types_PacketType_ContentObject:
            buildContentObjectDictionary(data, dictionary, extent);
            break;

        case CCNxCodecSchemaV1Types_PacketType_InterestReturn:
            trapNotImplemented("not implemented");
            break;

        default:
            trapIllegalValue(packetType, "Unknown PacketType");
    }
}

static void
buildSetDictionaryType(TestData *data, CCNxTlvDictionary *dictionary)
{
    uint8_t packetType = data->packet[1];
    switch (packetType) {
        case CCNxCodecSchemaV1Types_PacketType_Interest:
            ccnxTlvDictionary_SetMessageType_Interest(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
            break;

        case CCNxCodecSchemaV1Types_PacketType_ContentObject:
            ccnxTlvDictionary_SetMessageType_ContentObject(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
            break;

        case CCNxCodecSchemaV1Types_PacketType_InterestReturn:
            trapNotImplemented("not implemented");
            break;

        default:
            trapIllegalValue(packetType, "Unknown PacketType");
    }
}

/**
 * Builds a packet dictionary with OptionalHeaders and Message
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
buildPacketDictionary(TestData *data, CCNxTlvDictionary *packetDictionary, TlvExtent extent)
{
    buildSetDictionaryType(data, packetDictionary);

    size_t start = extent.offset;
    size_t end = start + extent.length;

    for (int i = 0; data->truthTable[i].indexOrKey != T_INVALID; i++) {
        size_t item_start = data->truthTable[i].extent.offset;
        size_t item_end = item_start + data->truthTable[i].extent.length;

        // Is this item included in the given extent?
        if (start < item_start && item_end <= end) {
            if (data->truthTable[i].bodyManifest == false) {
                PARCBuffer *itemBuffer = parcBuffer_Wrap(data->packet, data->packetLength, item_start, item_end);
                uint32_t translatedKey = translateOldOptionalHeadersManifestToNewKey(packetDictionary, data->truthTable[i].indexOrKey);
                ccnxTlvDictionary_PutBuffer(packetDictionary, translatedKey, itemBuffer);
                parcBuffer_Release(&itemBuffer);
            } else {
                buildMessageDictionary(data, packetDictionary, data->truthTable[i].extent);
            }

            // advance start to skip over whatever we just included
            start = item_end;
        }
    }
}

/**
 * Wraps the given (packet, length) in a PARCBuffer where the data->memoryRegion member will be set
 * to a given extent within that PARCBuffer.  The function will locate the truthTableKey in the truthTable and
 * use it's extent as the bounds for the wrapped packet.
 *
 * For example, if the key V1_INT_NAME has the extent {32, 12}, then the PARCBuffer will wrap the packet
 * memory and it will have and offset of 32 and a limit of 12.
 */
TestData *
commonSetup(uint8_t *packet, size_t length, TruthTableEntry *truthTable, int truthTableKey)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    TlvExtent extent = getTruthTableExtent(truthTable, truthTableKey);

    data->memoryRegion = parcBuffer_Wrap(packet, length, extent.offset, extent.offset + extent.length);
    data->encoder = ccnxCodecTlvEncoder_Create();
    ccnxCodecTlvEncoder_Initialize(data->encoder);

    // use the content object lenghts, they are the largest
    data->dictionary = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);

    data->packet = packet;
    data->packetLength = length;
    data->truthTable = truthTable;

    buildMessageDictionary(data, data->dictionary, extent);
    return data;
}

/**
 * Wraps a packet like commonSetup, but will do the whole packet including headers not just
 * the message body.  This is used by the PacketEncoder tests.
 */
TestData *
testrigencoder_CommonSetupWholePacket(uint8_t *packet, size_t length, TruthTableEntry *truthTable)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    data->memoryRegion = parcBuffer_Wrap(packet, length, 0, length);
    data->encoder = ccnxCodecTlvEncoder_Create();

    // use the content object lenghts, they are the largest
    data->dictionary = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);

    data->packet = packet;
    data->packetLength = length;
    data->truthTable = truthTable;

    buildPacketDictionary(data, data->dictionary, (TlvExtent) { 0, length });

    return data;
}

void
testrigencoder_CommonTeardown(TestData *data)
{
    ccnxTlvDictionary_Release(&data->dictionary);
    ccnxCodecTlvEncoder_Destroy(&data->encoder);
    parcBuffer_Release(&data->memoryRegion);

    if (data->signer) {
        parcSigner_Release(&data->signer);
    }

    parcMemory_Deallocate((void **) &data);
}

void
testDisplayIoVec(CCNxCodecEncodingBufferIOVec *vec)
{
    printf("Display iovec %p with %d elements\n", (void *) vec, vec->iovcnt);
    size_t totalLength = 0;
    for (int i = 0; i < vec->iovcnt; i++) {
        totalLength += vec->iov[i].iov_len;
        printf("   %3d: base %p length %4zu total length %4zu\n", i, (void *) vec->iov[i].iov_base, vec->iov[i].iov_len, totalLength);
    }
    printf("done\n\n");
}

void
testExecute(TestData *data, ssize_t (*encoderFunction)(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *dictionary))
{
    encoderFunction(data->encoder, data->dictionary);
    testCompareEncoderToBuffer(data->encoder, data->memoryRegion);
}
