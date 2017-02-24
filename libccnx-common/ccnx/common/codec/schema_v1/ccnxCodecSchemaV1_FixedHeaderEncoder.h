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
 * @file ccnxCodecSchemaV1_FixedHeaderEncoder.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_ccnxCodecSchemaV1_FixedHeaderEncoder_h
#define Libccnx_ccnxCodecSchemaV1_FixedHeaderEncoder_h

#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h>

/**
 * Set the values in the fixed header
 *
 * Put the provided fixed header at the given byte location.  The provided fixed header is not put
 * in as-is (i.e byte for byte), but is parsed and put in the correct byte positions and encodings
 * assuming the fixed header starts at the given position.
 *
 * The encoder is returned to its current position after putting the header.
 *
 * @param [in] encoder The encoder to append the fixed header in to
 * @param [in] The header, in host byte order
 *
 * @return Number The bytes appended, or -1 on error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t ccnxCodecSchemaV1FixedHeaderEncoder_EncodeHeader(CCNxCodecTlvEncoder *encoder, const CCNxCodecSchemaV1FixedHeader *header);
#endif // Libccnx_ccnxCodecSchemaV1_FixedHeaderEncoder_h
