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
 * @file ccnxCodecSchemaV1_NameSegmentCodec.h
 * @brief TLV codec for CCNx types
 *
 * Encode/decode a CCNx name segment using the V1 schema
 *
 */

#ifndef CCNxCodecSchemaV1_NameSegmentCodec_h
#define CCNxCodecSchemaV1_NameSegmentCodec_h

#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>
#include <ccnx/common/ccnx_Name.h>

/**
 * Encodes the name segment using the segment type as the TLV type
 *
 * Appends the name segment to the encoder.  The TLV type is implicit in
 * the CCNxNameSegment.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return number The number of bytes appended, including the type and length.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecSchemaV1NameSegmentCodec_Encode(CCNxCodecTlvEncoder *encoder, CCNxNameSegment *segment);

/**
 * Decodes the current location of the decoder as a CCNxNameSegment
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] decoder The decoder object
 *
 * @return non-null A CCNxNameSement
 * @return null An error, such as buffer underrun
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxNameSegment *ccnxCodecSchemaV1NameSegmentCodec_Decode(CCNxCodecTlvDecoder *decoder);
#endif // CCNxCodecSchemaV1_NameSegmentCodec_h
