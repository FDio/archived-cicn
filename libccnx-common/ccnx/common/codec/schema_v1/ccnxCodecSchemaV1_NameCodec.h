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
 * @file ccnxCodecSchemaV1_NameCodec.h
 * @brief TLV codec for CCNx types
 *
 * <#Detailed Description#>
 *
 */

#ifndef CCNxCodecSchemaV1_NameCodec_h
#define CCNxCodecSchemaV1_NameCodec_h

#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>
#include <ccnx/common/ccnx_Name.h>

/**
 * Encodes the name to the TLV Encoder
 *
 * Will append the Name after the current encoder location
 *
 * @param [in] type The TLV type to use for the Name container
 *
 * @return bytes The number of bytes appended to the encoder
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecSchemaV1NameCodec_Encode(CCNxCodecTlvEncoder *encoder, uint16_t type, const CCNxName *name);

/**
 * Decode the buffer as a CCNxName beginning at the current position
 *
 * The buffer must be pointing to the beginnig of the "type".  The decoder will
 * verify that the type matches `type'.  If it does not match, it will return NULL.
 *
 * @param [in] decoder The decoder
 * @param [in] type The TLV type that the decoder should currently be pointing at
 *
 * @return non-null The CCNxName decoded
 * @return null An error: either type did not match or some other error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxName *ccnxCodecSchemaV1NameCodec_Decode(CCNxCodecTlvDecoder *decoder, uint16_t type);

/**
 * The decoder points to the first byte of the Name "value"
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] decoder The Tlv Decoder pointing to the start of the Name value
 * @param [in] length the length of the Name value
 *
 * @return non-null A parsed name
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxName *ccnxCodecSchemaV1NameCodec_DecodeValue(CCNxCodecTlvDecoder *decoder, uint16_t length);
#endif // CCNxCodecSchemaV1_NameCodec_h
