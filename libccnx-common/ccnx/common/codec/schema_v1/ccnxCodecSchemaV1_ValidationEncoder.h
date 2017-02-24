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
 * @file ccnxCodecSchemaV1_ValidationEncoder.h
 * @brief Encode the Validation Algorithm and Payload
 *
 * Encodes the validation algorithm and payload from the dictionary.  Optionally computes
 * a signature if one is not specified in the dictionary and the encoder has a signer.
 *
 */

#ifndef __CCNx_Common__ccnxCodecSchemaV1_ValidationEncoder__
#define __CCNx_Common__ccnxCodecSchemaV1_ValidationEncoder__

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>

/**
 * Appends the Validation Algorithm to the packet encoder
 *
 * If the dictionary has a CryptoSuite specified, we will create a ValidationAlgorithm section
 * and fill it in as per the CryptoSuite and supplied validation algorithm arguments, such as
 * a KeyId, KeyName, Cert, etc.  For most signatures, only the KeyId is mandatory, the other
 * fields will only be specified if the user put something in the dictionary.
 *
 * the caller is responsible for writing the ValidationAlgorithm TL container.
 *
 * @param [in] encoder An allocated encoder to append to
 * @param [in] packetDictionary The dictionary containing the optional headers
 *
 * @return non-negative Total bytes appended to encoder
 * @return -1 An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t ccnxCodecSchemaV1ValidationEncoder_EncodeAlg(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary);

/**
 * Appends the Validation Payload to the packet encoder
 *
 * This will append the Valdiation Payload from the dictionary, or if it is missing and the
 * encoder has a signer, will create a signature.
 *
 * the caller is responsible for writing the ValidationPayload TL container.
 *
 * To create the signature, the caller must have used ccnxCodecTlvEncoder_MarkSignatureStart() and
 * ccnxCodecTlvEncoder_MarkSignatureEnd() functions to specify the byte locations of the start and
 * stop of the protected region.
 *
 * @param [in] encoder An allocated encoder to append to
 * @param [in] packetDictionary The dictionary containing the optional headers
 *
 * @return non-negative Total bytes appended to encoder
 * @return -1 An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t ccnxCodecSchemaV1ValidationEncoder_EncodePayload(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary);

#endif /* defined(__CCNx_Common__ccnxCodecSchemaV1_ValidationEncoder__) */
