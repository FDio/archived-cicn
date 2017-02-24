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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_ValidationDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_LinkCodec.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_CryptoSuite.h>

static bool
_decodeKeyName(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
{
    // At this point, the decoder should point to the 1st byte of the "value" of the (type, length) continer.
    // This is defined as a CCNxLink

    bool success = false;

    // this will set the decoder error if it fails.
    CCNxLink *link = ccnxCodecSchemaV1LinkCodec_DecodeValue(decoder, length);
    if (link != NULL) {
        const CCNxName *name = ccnxLink_GetName(link);
        if (name != NULL) {
            success = ccnxTlvDictionary_PutName(packetDictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_NAME, name);

            if (success) {
                PARCBuffer *keyid = ccnxLink_GetKeyID(link);
                if (keyid) {
                    ccnxTlvDictionary_PutBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_KEYID, keyid);
                }

                PARCBuffer *hash = ccnxLink_GetContentObjectHash(link);
                if (hash) {
                    ccnxTlvDictionary_PutBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_OBJHASH, hash);
                }
            }
        }

        if (!success) {
            CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
            ccnxCodecTlvDecoder_SetError(decoder, error);
            ccnxCodecError_Release(&error);
        }

        ccnxLink_Release(&link);
    }

    return success;
}

static bool
_decodeAlgParametersType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
{
    bool success = false;
    switch (type) {
        case CCNxCodecSchemaV1Types_ValidationAlg_Cert:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CERT);
            break;

        case CCNxCodecSchemaV1Types_ValidationAlg_KeyId:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYID);
            break;

        case CCNxCodecSchemaV1Types_ValidationAlg_KeyName:
            // The "value" is a link
            success = _decodeKeyName(decoder, packetDictionary, type, length);
            break;

        case CCNxCodecSchemaV1Types_ValidationAlg_SigTime:
            // This is a time, so put it as an Integer
            success = ccnxCodecTlvUtilities_PutAsInteger(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_SIGNTIME);
            break;

        case CCNxCodecSchemaV1Types_ValidationAlg_PublicKey:
            success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEY);
            break;

        default: {
            // if we do not know the TLV type, put it in this container's unknown list
            success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_Lists_VALIDATION_ALG_LIST);
        }
        break;
    }

    if (!success) {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
        ccnxCodecTlvDecoder_SetError(decoder, error);
        ccnxCodecError_Release(&error);
    }
    return success;
}

/**
 * Called by _decodeAlgType() via ccnxCodecTlvUtilities_DecodeSubcontainer() to decode the
 * algorithm specific parameters
 */
static bool
_decodeAlgParameters(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary)
{
    return ccnxCodecTlvUtilities_DecodeContainer(decoder, packetDictionary, _decodeAlgParametersType);
}

static bool
_decodeAlgType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
{
    bool success = false;

    PARCCryptoSuite parcSuite;
    bool match = ccnxCodecSchemaV1CryptoSuite_TlvToParc((CCNxCodecSchemaV1TlvDictionary_CryptoSuite) type, &parcSuite);

    if (match) {
        success = ccnxTlvDictionary_PutInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE, parcSuite);

        if (success) {
            success = ccnxCodecTlvUtilities_DecodeSubcontainer(decoder, packetDictionary, type, length, _decodeAlgParameters);
        }
    } else {
        // if we do not know the TLV type, put it in this container's unknown list
        success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV1TlvDictionary_Lists_VALIDATION_ALG_LIST);
    }

    if (!success) {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
        ccnxCodecTlvDecoder_SetError(decoder, error);
        ccnxCodecError_Release(&error);
    }
    return success;
}

// ==================
// Public API

bool
ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary)
{
    return ccnxCodecTlvUtilities_DecodeContainer(decoder, packetDictionary, _decodeAlgType);
}


bool
ccnxCodecSchemaV1ValidationDecoder_DecodePayload(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary)
{
    bool success = false;
    // A 0-length payload is treaded like an error
    size_t remaining = ccnxCodecTlvDecoder_Remaining(decoder);
    if (remaining > 0) {
        PARCBuffer *payload = ccnxCodecTlvDecoder_GetValue(decoder, remaining);
        success = ccnxTlvDictionary_PutBuffer(packetDictionary, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_PAYLOAD, payload);
        parcBuffer_Release(&payload);
    }
    return success;
}
