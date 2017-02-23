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
#include <sys/time.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/internal/ccnx_WireFormatFacadeV1.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_MessageDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_ManifestDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_ValidationDecoder.h>

typedef struct rta_tlv_schema_v1_data {
    CCNxCodecTlvDecoder *decoder;
    CCNxTlvDictionary *packetDictionary;
} _CCNxCodecSchemaV1Data;

/**
 * Decodes the per-hop optional headers
 *
 * @param [in] data The packet decoder state
 *
 * @return true successful decode
 * @return false A decoding error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_decodeOptionalHeaders(_CCNxCodecSchemaV1Data *data)
{
    size_t optionalHeaderLength = ccnxCodecSchemaV1FixedHeaderDecoder_GetOptionalHeaderLength(data->packetDictionary);
    CCNxCodecTlvDecoder *optionalHeaderDecoder = ccnxCodecTlvDecoder_GetContainer(data->decoder, optionalHeaderLength);

    bool success = ccnxCodecSchemaV1OptionalHeadersDecoder_Decode(optionalHeaderDecoder, data->packetDictionary);

    ccnxCodecTlvDecoder_Destroy(&optionalHeaderDecoder);
    return success;
}

/**
 * Decodes the "value" of the CPI "TLV"
 *
 * the CPI packet is encoded as a single TLV container of type 0xBEEF (detected in _decodeMessage).
 * At this point, the cpiDecoder wraps the CPI payload, which is the encapsulated JSON
 *
 * @param [in] cpiDecoder Decoder wrapping the value
 * @param [in] packetDictionary where to place the results
 *
 * @retval true Good decode
 * @retval false An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_decodeCPI(CCNxCodecTlvDecoder *cpiDecoder, CCNxTlvDictionary *packetDictionary)
{
    // we just take the whole contents of the decoder and put in the the PAYLOAD dictionary entry.
    size_t length = ccnxCodecTlvDecoder_Remaining(cpiDecoder);
    PARCBuffer *payload = ccnxCodecTlvDecoder_GetValue(cpiDecoder, length);

    PARCJSON *json = parcJSON_ParseBuffer(payload);

    bool success = ccnxTlvDictionary_PutJson(packetDictionary,
                                             CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, json);
    parcJSON_Release(&json);
    parcBuffer_Release(&payload);
    return success;
}

/**
 * Decodes the CCNx message inside a TLV packet
 *
 * Creates an inner decoder that slices the decode buffer then passes that and our
 * message dictionary to the appropriate inner decoder.
 *
 * @param [in] data The packet decoder state
 *
 * @return true successful decode
 * @return false A decoding error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_decodeMessage(_CCNxCodecSchemaV1Data *data)
{
    bool success = false;

    if (ccnxCodecTlvDecoder_EnsureRemaining(data->decoder, 4)) {
        // what kind of message are we looking at?
        // Note that this is based on the TLV container, not the fixed head PacketType
        uint16_t tlv_type = ccnxCodecTlvDecoder_GetType(data->decoder);
        uint16_t tlv_length = ccnxCodecTlvDecoder_GetLength(data->decoder);

        // ensure its a proper tlv type
        switch (tlv_type) {
            case CCNxCodecSchemaV1Types_MessageType_Interest: // fallthrough
            case CCNxCodecSchemaV1Types_MessageType_ContentObject: // fallthrough
            case CCNxCodecSchemaV1Types_MessageType_Control: // fallthrough
            case CCNxCodecSchemaV1Types_MessageType_Manifest: // fallthrough
                break;

            default:
                return false;
        }

        // cross check with the fixed header value
        // ccnxCodecSchemaV1FixedHeaderDecoder_Decode ensures that PacketLength is not less than HeaderLength
        size_t messageLength = ccnxCodecSchemaV1FixedHeaderDecoder_GetPacketLength(data->packetDictionary) - ccnxCodecSchemaV1FixedHeaderDecoder_GetHeaderLength(data->packetDictionary);

        if (tlv_length <= messageLength && ccnxCodecTlvDecoder_EnsureRemaining(data->decoder, tlv_length)) {
            // This decode is for the "value" of the message, it does not include the wrapper
            CCNxCodecTlvDecoder *messageDecoder = ccnxCodecTlvDecoder_GetContainer(data->decoder, tlv_length);

            if (tlv_type == CCNxCodecSchemaV1Types_MessageType_Control) {
                // the CPI messages are not a proper "message" in that there's no inner TLV, its just data
                success = _decodeCPI(messageDecoder, data->packetDictionary);
            } else if (tlv_type == CCNxCodecSchemaV1Types_MessageType_Manifest) {
                ccnxTlvDictionary_SetMessageType_Manifest(data->packetDictionary, CCNxTlvDictionary_SchemaVersion_V1);
                success = ccnxCodecSchemaV1ManifestDecoder_Decode(messageDecoder, data->packetDictionary);
            } else {
                success = ccnxCodecSchemaV1MessageDecoder_Decode(messageDecoder, data->packetDictionary);
            }

            ccnxCodecTlvDecoder_Destroy(&messageDecoder);
        } else {
            // raise an error
            CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_TOO_LONG, __func__, __LINE__, ccnxCodecTlvDecoder_Position(data->decoder));
            ccnxCodecTlvDecoder_SetError(data->decoder, error);
            ccnxCodecError_Release(&error);
        }
    }

    return success;
}

static bool
_decodeValidationAlg(_CCNxCodecSchemaV1Data *data)
{
    bool success = false;

    if (ccnxCodecTlvDecoder_EnsureRemaining(data->decoder, 4)) {
        // what kind of message are we looking at?
        // Note that this is based on the TLV container, not the fixed head PacketType
        uint16_t tlv_type = ccnxCodecTlvDecoder_GetType(data->decoder);
        uint16_t tlv_length = ccnxCodecTlvDecoder_GetLength(data->decoder);

        if (tlv_type == CCNxCodecSchemaV1Types_MessageType_ValidationAlg &&
            ccnxCodecTlvDecoder_EnsureRemaining(data->decoder, tlv_length)) {
            CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_GetContainer(data->decoder, tlv_length);

            success = ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(decoder, data->packetDictionary);

            ccnxCodecTlvDecoder_Destroy(&decoder);
        } else {
            // raise and error
            if (!ccnxCodecTlvDecoder_EnsureRemaining(data->decoder, tlv_length)) {
                // tlv_length goes beyond the decoder
                CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_TOO_LONG, __func__, __LINE__, ccnxCodecTlvDecoder_Position(data->decoder));
                ccnxCodecTlvDecoder_SetError(data->decoder, error);
                ccnxCodecError_Release(&error);
            } else {
                // not CCNxCodecSchemaV1Types_MessageType_ValidationAlg
                CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_DECODE, __func__, __LINE__, ccnxCodecTlvDecoder_Position(data->decoder));
                ccnxCodecTlvDecoder_SetError(data->decoder, error);
                ccnxCodecError_Release(&error);
            }
        }
    }

    return success;
}

static bool
_decodeValidationPayload(_CCNxCodecSchemaV1Data *data)
{
    bool success = false;

    if (ccnxCodecTlvDecoder_EnsureRemaining(data->decoder, 4)) {
        // what kind of message are we looking at?
        // Note that this is based on the TLV container, not the fixed head PacketType
        uint16_t tlv_type = ccnxCodecTlvDecoder_GetType(data->decoder);
        uint16_t tlv_length = ccnxCodecTlvDecoder_GetLength(data->decoder);

        if (tlv_type == CCNxCodecSchemaV1Types_MessageType_ValidationPayload &&
            ccnxCodecTlvDecoder_EnsureRemaining(data->decoder, tlv_length)) {
            CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_GetContainer(data->decoder, tlv_length);

            success = ccnxCodecSchemaV1ValidationDecoder_DecodePayload(decoder, data->packetDictionary);

            ccnxCodecTlvDecoder_Destroy(&decoder);
        }
    }

    return success;
}

bool
ccnxCodecSchemaV1PacketDecoder_Decode(CCNxCodecTlvDecoder *packetDecoder, CCNxTlvDictionary *packetDictionary)
{
    bool decodeSuccess = false;

    _CCNxCodecSchemaV1Data data;

    // we temporarily store this reference, but we do not destroy it.  This
    // is just to pass the reference down the decode chain, it is not
    // stored beyond the immediate scope.  Therefore, no reference acquired.
    data.packetDictionary = packetDictionary;
    data.decoder = packetDecoder;

    if (ccnxCodecSchemaV1FixedHeaderDecoder_Decode(data.decoder, data.packetDictionary)) {
        if (_decodeOptionalHeaders(&data)) {
            // Record the position we'd start the signature verification at
            size_t signatureStartPosition = ccnxCodecTlvDecoder_Position(data.decoder);


            // Mark the beginning of the ContentObject hash region.
            CCNxWireFormatFacadeV1_Implementation.setContentObjectHashRegionStart(data.packetDictionary, signatureStartPosition);

            if (_decodeMessage(&data)) {
                // If there's anything else left, it must be the validation alg and payload
                if (!ccnxCodecTlvDecoder_IsEmpty(data.decoder)) {
                    if (_decodeValidationAlg(&data)) {
                        // at this point, we've advanced to the end of the validation algorithm,
                        // that's where we would end signature verification
                        size_t signatureStopPosition = ccnxCodecTlvDecoder_Position(data.decoder);

                        CCNxWireFormatFacadeV1_Implementation.setProtectedRegionStart(data.packetDictionary, signatureStartPosition);
                        CCNxWireFormatFacadeV1_Implementation.setProtectedRegionLength(data.packetDictionary, signatureStopPosition - signatureStartPosition);

                        if (_decodeValidationPayload(&data)) {
                            decodeSuccess = true;
                        }
                    }
                } else {
                    // nothing after the message, so that's a successful decode
                    decodeSuccess = true;
                }

                // Mark the length of the ContentObject hash region (to the end of the packet).
                size_t contentObjectHashRegionLength = ccnxCodecTlvDecoder_Position(data.decoder) - signatureStartPosition;
                CCNxWireFormatFacadeV1_Implementation.setContentObjectHashRegionLength(data.packetDictionary, contentObjectHashRegionLength);
            }
        }
    }

    return decodeSuccess;
}

bool
ccnxCodecSchemaV1PacketDecoder_BufferDecode(PARCBuffer *packetBuffer, CCNxTlvDictionary *packetDictionary)
{
    CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(packetBuffer);
    bool success = ccnxCodecSchemaV1PacketDecoder_Decode(decoder, packetDictionary);
    ccnxCodecTlvDecoder_Destroy(&decoder);
    return success;
}
