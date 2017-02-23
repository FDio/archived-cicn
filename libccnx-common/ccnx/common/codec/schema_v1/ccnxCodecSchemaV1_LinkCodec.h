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
 * @file ccnxCodecSchemaV1_LinkCodec.h
 * @brief A Link is a well-known value, not a TLV field
 *
 * A Link may be the "value" of a TLV element, such as the KeyName.  It can also occur
 * in the payload of a ContentObject whose PayloadType is Link.
 *
 */

#ifndef __CCNx_Common__ccnxCodecSchemaV1_LinkCodec__
#define __CCNx_Common__ccnxCodecSchemaV1_LinkCodec__

#include <ccnx/common/ccnx_Link.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>

/**
 * Encodes the link, but without a "TL" container
 *
 * Will append the Link in it's well-known TLV format, but without any
 * "TL" container.
 *
 * If the link does not have a name, will return -1 with the error TLV_MISSING_MANDATORY.
 *
 * @param [in] encoder The link will be appended to the encoder
 * @param [in] link The link to append
 *
 * @retval non-negative The number of bytes appended to the encoder
 * @retval negative An error, look at the CCNxCodecError of the encoder
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t ccnxCodecSchemaV1LinkCodec_Encode(CCNxCodecTlvEncoder *encoder, const CCNxLink *link);

/**
 * The decoder points to the first byte of the "value" of something that is a Link
 *
 * For a KeyName, decoder should be pointed to the "value" of the KeyName.  for a ContentObject
 * of type Link, it should be the first byte of the Payload.
 *
 * A link is the tuple {Name, [KeyId], [Hash]}, where KeyId is the keyIdRestriction and
 * Hash is the ContentObjectHash restriction to use in an Interest for Name.
 * No additional fields are allowed in the Link.
 *
 * @param [in] decoder The Tlv Decoder pointing to the start of the Name value
 * @param [in] length the length of the Link value
 *
 * @return non-null A parsed name
 * @return null An error, check the decoder's error message
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxLink *ccnxCodecSchemaV1LinkCodec_DecodeValue(CCNxCodecTlvDecoder *decoder, uint16_t length);

#endif /* defined(__CCNx_Common__ccnxCodecSchemaV1_LinkCodec__) */
