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
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_LinkCodec.h>

ssize_t
ccnxCodecSchemaV1LinkCodec_Encode(CCNxCodecTlvEncoder *encoder, const CCNxLink *link)
{
    ssize_t length = 0;

    const CCNxName *name = ccnxLink_GetName(link);
    if (name) {
        length += ccnxCodecSchemaV1NameCodec_Encode(encoder, CCNxCodecSchemaV1Types_Link_Name, name);

        PARCBuffer *keyid = ccnxLink_GetKeyID(link);
        if (keyid) {
            length += ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_Link_KeyIdRestriction, keyid);
        }

        PARCBuffer *hash = ccnxLink_GetContentObjectHash(link);
        if (hash) {
            length += ccnxCodecTlvEncoder_AppendBuffer(encoder, CCNxCodecSchemaV1Types_Link_ContentObjectHashRestriction, hash);
        }
    } else {
        length = -1;
        CCNxCodecError *error = ccnxCodecError_Create(TLV_MISSING_MANDATORY, __func__, __LINE__, ccnxCodecTlvEncoder_Position(encoder));
        ccnxCodecTlvEncoder_SetError(encoder, error);
        ccnxCodecError_Release(&error);
    }

    return length;
}

typedef struct decoded_link {
    CCNxName *linkName;
    PARCBuffer *linkKeyId;
    PARCBuffer *linkHash;
} _DecodedLink;

static int
_decodeField(CCNxCodecTlvDecoder *decoder, _DecodedLink *decodedLink)
{
    int errorCode = TLV_ERR_NO_ERROR;

    uint16_t type = ccnxCodecTlvDecoder_GetType(decoder);
    uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, length)) {
        switch (type) {
            case CCNxCodecSchemaV1Types_Link_Name:
                if (decodedLink->linkName == NULL) {
                    decodedLink->linkName = ccnxCodecSchemaV1NameCodec_DecodeValue(decoder, length);
                } else {
                    errorCode = TLV_ERR_DUPLICATE_FIELD;
                }
                break;

            case CCNxCodecSchemaV1Types_Link_KeyIdRestriction:
                if (decodedLink->linkKeyId == NULL) {
                    decodedLink->linkKeyId = ccnxCodecTlvDecoder_GetValue(decoder, length);
                } else {
                    errorCode = TLV_ERR_DUPLICATE_FIELD;
                }
                break;

            case CCNxCodecSchemaV1Types_Link_ContentObjectHashRestriction:
                if (decodedLink->linkHash == NULL) {
                    decodedLink->linkHash = ccnxCodecTlvDecoder_GetValue(decoder, length);
                } else {
                    errorCode = TLV_ERR_DUPLICATE_FIELD;
                }
                break;

            default:
                // we do not support unknown TLVs
                errorCode = TLV_ERR_DECODE;
                break;
        }
    } else {
        errorCode = TLV_ERR_TOO_LONG;
    }

    return errorCode;
}

static void
_decodecLinkCleanup(_DecodedLink *decodedLink)
{
    if (decodedLink->linkName) {
        ccnxName_Release(&decodedLink->linkName);
    }

    if (decodedLink->linkKeyId) {
        parcBuffer_Release(&decodedLink->linkKeyId);
    }

    if (decodedLink->linkHash) {
        parcBuffer_Release(&decodedLink->linkHash);
    }
}

CCNxLink *
ccnxCodecSchemaV1LinkCodec_DecodeValue(CCNxCodecTlvDecoder *decoder, uint16_t linkLength)
{
    int errorCode = TLV_ERR_NO_ERROR;

    CCNxLink *link = NULL;

    _DecodedLink decodedLink;
    memset(&decodedLink, 0, sizeof(_DecodedLink));

    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, linkLength)) {
        while (errorCode == TLV_ERR_NO_ERROR && ccnxCodecTlvDecoder_EnsureRemaining(decoder, 4)) {
            errorCode = _decodeField(decoder, &decodedLink);
        }
    } else {
        errorCode = TLV_ERR_TOO_LONG;
    }

    if (errorCode == TLV_ERR_NO_ERROR && decodedLink.linkName == NULL) {
        errorCode = TLV_ERR_DECODE;
    }

    if (errorCode != TLV_ERR_NO_ERROR) {
        CCNxCodecError *error = ccnxCodecError_Create(errorCode, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
        ccnxCodecTlvDecoder_SetError(decoder, error);
        ccnxCodecError_Release(&error);
    } else {
        link = ccnxLink_Create(decodedLink.linkName, decodedLink.linkKeyId, decodedLink.linkHash);
    }

    // cleanup any partial memory allocations
    _decodecLinkCleanup(&decodedLink);

    return link;
}
