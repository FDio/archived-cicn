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
#include <inttypes.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeaderEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_MessageEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_ValidationEncoder.h>

#include <ccnx/common/internal/ccnx_InterestDefault.h>
#include <ccnx/common/internal/ccnx_ValidationFacadeV1.h>
#include <ccnx/common/internal/ccnx_WireFormatMessageInterface.h>

// =====================================================
// Private API

static uint8_t
_getHopLimit(CCNxTlvDictionary *packetDictionary)
{
    uint8_t hoplimit = (uint8_t) CCNxInterestDefault_HopLimit;

    if (ccnxTlvDictionary_IsValueInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT)) {
        hoplimit = (uint8_t) ccnxTlvDictionary_GetInteger(packetDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT);
    }
    return hoplimit;
}

static uint8_t
_getInterestReturnCode(CCNxTlvDictionary *packetDictionary)
{
    uint8_t returnCode = (uint8_t) 0;

    if (ccnxTlvDictionary_IsValueInteger(packetDictionary,
                                         CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestReturnCode)) {
        returnCode =
            (uint8_t) ccnxTlvDictionary_GetInteger(packetDictionary,
                                                   CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestReturnCode);
    }
    return returnCode;
}

/**
 * Creates a fixed header from the given parameters and encodes in network byte order
 *
 * All parameters in host byte order.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-negative The total bytes appended to the encode buffer
 * @return -1 An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static ssize_t
_encodeFixedHeader(CCNxCodecTlvEncoder *fixedHeaderEncoder,
                   CCNxTlvDictionary *packetDictionary,
                   int packetType,
                   ssize_t headerLength,
                   ssize_t packetLength)
{
    CCNxCodecSchemaV1FixedHeader fixedHeader;
    memset(&fixedHeader, 0, sizeof(fixedHeader));

    fixedHeader.version = 1;
    fixedHeader.packetType = packetType;
    fixedHeader.packetLength = packetLength;
    fixedHeader.headerLength = headerLength;

    if ((packetType == CCNxCodecSchemaV1Types_PacketType_Interest) ||
        (packetType == CCNxCodecSchemaV1Types_PacketType_InterestReturn)) {
        CCNxCodecSchemaV1InterestHeader *interestHeader = (CCNxCodecSchemaV1InterestHeader *) &fixedHeader;
        interestHeader->hopLimit = _getHopLimit(packetDictionary);
        if (packetType == CCNxCodecSchemaV1Types_PacketType_InterestReturn) {
            interestHeader->returnCode = _getInterestReturnCode(packetDictionary);
        }
    }

    return ccnxCodecSchemaV1FixedHeaderEncoder_EncodeHeader(fixedHeaderEncoder, &fixedHeader);
}

static ssize_t
_encodeOptionalHeaders(CCNxCodecTlvEncoder *optionalHeaderEncoder, CCNxTlvDictionary *packetDictionary)
{
    // Optional Headers do not have a container, so just append them right to the buffer
    size_t optionalHeadersLength = 0;
    optionalHeadersLength = ccnxCodecSchemaV1OptionalHeadersEncoder_Encode(optionalHeaderEncoder, packetDictionary);
    return optionalHeadersLength;
}

/**
 * CPI payload is simply a dump of the PAYLOAD dictionary entry.
 *
 * There are no inner TLVs of this message, so it is not encoded like a normal message
 * with a call to ccnxCodecSchemaV1MessageEncoder_Encode().  Rather it is written here.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return non-negative The number of bytes appended to the buffer
 * @return negative An error
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static ssize_t
_encodeCPI(CCNxCodecTlvEncoder *cpiEncoder, CCNxTlvDictionary *packetDictionary)
{
    // Optional Headers do not have a container, so just append them right to the buffer
    size_t payloadLength = 0;

    if (ccnxTlvDictionary_IsValueJson(packetDictionary,
                                      CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD)) {
        PARCJSON *json = ccnxTlvDictionary_GetJson(packetDictionary,
                                                   CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
        if (json) {
            char *jsonString = parcJSON_ToCompactString(json);

            payloadLength = strlen(jsonString);
            ccnxCodecTlvEncoder_AppendRawArray(cpiEncoder, payloadLength, (uint8_t * ) jsonString);
            parcMemory_Deallocate((void **) &jsonString);
        }
    } else {
        PARCBuffer *payload = ccnxTlvDictionary_GetBuffer(packetDictionary,
                                                          CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);

        payloadLength = parcBuffer_Remaining(payload);
        uint8_t *overlay = parcBuffer_Overlay(payload, 0);
        ccnxCodecTlvEncoder_AppendRawArray(cpiEncoder, payloadLength, overlay);
    }
    return payloadLength;
}

/**
 * Encode the CCNx Message
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [out] packetTypePtr The type to use for the PacketType based on the message type
 *
 * @retval non-negative the bytes appended to the encoder
 * @retval negative An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static ssize_t
_encodeMessage(CCNxCodecTlvEncoder *packetEncoder, CCNxTlvDictionary *packetDictionary, CCNxCodecSchemaV1Types_PacketType *packetTypePtr)
{
    ssize_t startPosition = ccnxCodecTlvEncoder_Position(packetEncoder);
    ssize_t innerLength = -1;

    // what kind of message is it?  need this to set the packetTypePtr

    if (ccnxTlvDictionary_IsInterest(packetDictionary)) {
        *packetTypePtr = CCNxCodecSchemaV1Types_PacketType_Interest;
        ccnxCodecTlvEncoder_AppendContainer(packetEncoder, CCNxCodecSchemaV1Types_MessageType_Interest, 0);
        innerLength = ccnxCodecSchemaV1MessageEncoder_Encode(packetEncoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsInterestReturn(packetDictionary)) {
        *packetTypePtr = CCNxCodecSchemaV1Types_PacketType_InterestReturn;
        ccnxCodecTlvEncoder_AppendContainer(packetEncoder, CCNxCodecSchemaV1Types_MessageType_Interest, 0);
        innerLength = ccnxCodecSchemaV1MessageEncoder_Encode(packetEncoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsContentObject(packetDictionary)) {
        *packetTypePtr = CCNxCodecSchemaV1Types_PacketType_ContentObject;
        ccnxCodecTlvEncoder_AppendContainer(packetEncoder, CCNxCodecSchemaV1Types_MessageType_ContentObject, 0);
        innerLength = ccnxCodecSchemaV1MessageEncoder_Encode(packetEncoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsControl(packetDictionary)) {
        *packetTypePtr = CCNxCodecSchemaV1Types_PacketType_Control;
        ccnxCodecTlvEncoder_AppendContainer(packetEncoder, CCNxCodecSchemaV1Types_MessageType_Control, 0);
        innerLength = _encodeCPI(packetEncoder, packetDictionary);
    } else if (ccnxTlvDictionary_IsManifest(packetDictionary)) {
        *packetTypePtr = CCNxCodecSchemaV1Types_PacketType_ContentObject;
        ccnxCodecTlvEncoder_AppendContainer(packetEncoder, CCNxCodecSchemaV1Types_MessageType_Manifest, 0);
        innerLength = ccnxCodecSchemaV1MessageEncoder_Encode(packetEncoder, packetDictionary);
    }

    if (innerLength >= 0) {
        // For a 0 length message, we do not backup and erase the TLV container.
        ccnxCodecTlvEncoder_SetContainerLength(packetEncoder, startPosition, innerLength);
        ssize_t endPosition = ccnxCodecTlvEncoder_Position(packetEncoder);
        innerLength = endPosition - startPosition;
    } else {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_MISSING_MANDATORY, __func__, __LINE__, ccnxCodecTlvEncoder_Position(packetEncoder));
        ccnxCodecTlvEncoder_SetError(packetEncoder, error);
        ccnxCodecError_Release(&error);
    }

    return innerLength;
}

static ssize_t
_encodeValidationAlg(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t innerLength = 0;

    // There must be a CryptoSuite in the packet to sign it.
    // Temporary exception for Content Objects, which are all signed if the codec has a signer.
    if (ccnxValidationFacadeV1_HasCryptoSuite(packetDictionary) || ccnxTlvDictionary_IsContentObject(packetDictionary)) {
        ssize_t startPosition = ccnxCodecTlvEncoder_Position(encoder);

        ccnxCodecTlvEncoder_AppendContainer(encoder, CCNxCodecSchemaV1Types_MessageType_ValidationAlg, 0);
        innerLength = ccnxCodecSchemaV1ValidationEncoder_EncodeAlg(encoder, packetDictionary);

        if (innerLength == 0) {
            // backup and erase the container
            ccnxCodecTlvEncoder_SetPosition(encoder, startPosition);
        } else if (innerLength >= 0) {
            ccnxCodecTlvEncoder_SetContainerLength(encoder, startPosition, innerLength);
            ssize_t endPosition = ccnxCodecTlvEncoder_Position(encoder);
            return endPosition - startPosition;
        }
    }

    return innerLength;
}

static ssize_t
_encodeValidationPayload(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t startPosition = ccnxCodecTlvEncoder_Position(encoder);

    ccnxCodecTlvEncoder_AppendContainer(encoder, CCNxCodecSchemaV1Types_MessageType_ValidationPayload, 0);
    ssize_t innerLength = ccnxCodecSchemaV1ValidationEncoder_EncodePayload(encoder, packetDictionary);

    if (innerLength == 0) {
        // backup and erase the container
        ccnxCodecTlvEncoder_SetPosition(encoder, startPosition);
    } else if (innerLength > 0) {
        ccnxCodecTlvEncoder_SetContainerLength(encoder, startPosition, innerLength);
        ssize_t endPosition = ccnxCodecTlvEncoder_Position(encoder);
        return endPosition - startPosition;
    }

    return innerLength;
}

// =====================================================
// Public API

CCNxCodecNetworkBufferIoVec *
ccnxCodecSchemaV1PacketEncoder_DictionaryEncode(CCNxTlvDictionary *packetDictionary, PARCSigner *signer)
{
    CCNxCodecNetworkBufferIoVec *outputBuffer = NULL;

    CCNxCodecTlvEncoder *packetEncoder = ccnxCodecTlvEncoder_Create();

    if (signer) {
//        ccnxCodecTlvEncoder_SetSigner(packetEncoder, signer);
    }

    ssize_t encodedLength = ccnxCodecSchemaV1PacketEncoder_Encode(packetEncoder, packetDictionary);
    if (encodedLength > 0) {
        ccnxCodecTlvEncoder_Finalize(packetEncoder);
        outputBuffer = ccnxCodecTlvEncoder_CreateIoVec(packetEncoder);
    }

    trapUnexpectedStateIf(encodedLength < 0 && !ccnxCodecTlvEncoder_HasError(packetEncoder),
                          "Got error length but no error set");

    assertFalse(ccnxCodecTlvEncoder_HasError(packetEncoder), "ENCODING ERROR")
    {
        printf("ERROR: %s\n", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(packetEncoder)));
        ccnxTlvDictionary_Display(packetDictionary, 3);
    }

    ccnxCodecTlvEncoder_Destroy(&packetEncoder);

    // return a reference counted copy so it won't be destroyed by ccnxCodecTlvEncoder_Destroy
    return outputBuffer;
}

ssize_t
ccnxCodecSchemaV1PacketEncoder_Encode(CCNxCodecTlvEncoder *packetEncoder, CCNxTlvDictionary *packetDictionary)
{
    ssize_t length = -1;

    // We will need to go back and fixedup the headers
    ssize_t fixedHeaderPosition = ccnxCodecTlvEncoder_Position(packetEncoder);
    ssize_t fixedHeaderLength = _encodeFixedHeader(packetEncoder, packetDictionary, -1, 0, 0);

    ssize_t optionalHeadersLength = _encodeOptionalHeaders(packetEncoder, packetDictionary);

    if (optionalHeadersLength >= 0) {
        ccnxCodecTlvEncoder_MarkSignatureStart(packetEncoder);

        CCNxCodecSchemaV1Types_PacketType messageType = -1;

        ssize_t messageLength = _encodeMessage(packetEncoder, packetDictionary, &messageType);

        if (messageLength >= 0) {
            // validation is optional, so it's ok if its 0 length
            ssize_t validationAlgLength = _encodeValidationAlg(packetEncoder, packetDictionary);
            ssize_t validationPayloadLength = 0;
            if (validationAlgLength > 0) {
                ccnxCodecTlvEncoder_MarkSignatureEnd(packetEncoder);

                validationPayloadLength = _encodeValidationPayload(packetEncoder, packetDictionary);
            }

            if (validationAlgLength >= 0 && validationPayloadLength >= 0) {
                // now fix up the fixed header
                size_t endPosition = ccnxCodecTlvEncoder_Position(packetEncoder);

                size_t headerLength = fixedHeaderLength + optionalHeadersLength;
                size_t packetLength = headerLength + messageLength + validationAlgLength + validationPayloadLength;

                // Will this work for InterestReturn?  As long as _encodeMessage returns InterestReturn it
                // will be ok.
                int packetType = messageType;

                ccnxCodecTlvEncoder_SetPosition(packetEncoder, fixedHeaderPosition);
                _encodeFixedHeader(packetEncoder, packetDictionary, packetType, headerLength, packetLength);
                ccnxCodecTlvEncoder_SetPosition(packetEncoder, endPosition);
                length = endPosition - fixedHeaderPosition;

                trapUnexpectedStateIf(packetLength != length, "packet length %zu not equal to measured length %zd", packetLength, length);
            }
        }
    }

    return length;
}
