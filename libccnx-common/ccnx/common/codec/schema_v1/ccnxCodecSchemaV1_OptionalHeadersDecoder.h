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
 * @file ccnxCodecSchemaV1_OptionalHeadersDecoder.h
 * @brief Decode the list of optional headers
 *
 * A reference to each optional header will be stored in the provided CCNxTlvDictionary.
 *
 */

#ifndef TransportRTA_ccnxCodecSchemaV1_OptionalHeadersDecoder_h
#define TransportRTA_ccnxCodecSchemaV1_OptionalHeadersDecoder_h

#include <stdbool.h>

#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>

/**
 * The decode a list of Version 1 optional headers
 *
 * The decoder should point to the first byte of the first optional header.
 * The decoder will advance until the end of the buffer.
 * It is an error for the last optional header to either go beyond the end of the
 * decoder or for it to underrun the end of the decoder. It must exactly align.
 *
 * @param [in] decoder The decoder to parse
 * @param [in] packetDictionary The results go directly in to the provided dictionary.
 *
 * @return true Fully parsed interest, no errors
 * @return false Error decoding, decoder is left pointing to the first byte of the error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecSchemaV1OptionalHeadersDecoder_Decode(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary);

/**
 * A convenience function to return the Interest Fragment header buffer
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] packetDictionary The packet dictionary
 *
 * @return non-null The header buffer
 * @return null The header does not exist
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxCodecSchemaV1OptionalHeadersDecoder_GetInterestFragmentHeader(CCNxTlvDictionary *packetDictionary);

/**
 * A convenience function to return the Content Object Fragment header buffer
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] packetDictionary The packet dictionary
 *
 * @return non-null The header buffer
 * @return null The header does not exist
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxCodecSchemaV1OptionalHeadersDecoder_GetContentObjectFragmentHeader(CCNxTlvDictionary *packetDictionary);

/**
 * A convenience function to return the Interest Lifetime header value
 *
 * Returns the byte array of the Interest Lifetime, which is encoded as a uint64_t milli-seconds
 * since the UTC epoch.  The PARCBuffer returned wraps the underlying memory so any changes to the
 * buffer will be reflected in the header.
 *
 * @param [in] packetDictionary The packet dictionary
 *
 * @return number The Interest Lifetime
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint64_t ccnxCodecSchemaV1OptionalHeadersDecoder_GetInterestLifetimeHeader(CCNxTlvDictionary *packetDictionary);

/**
 * A convenience function to return the Recommended Cache Time (RCT) header
 *
 * Returns the byte array of the RCT, which is encoded as a uint64_t milli-seconds
 * since the UTC epoch.  The PARCBuffer returned wraps the underlying memory so any changes to the
 * buffer will be reflected in the header.
 *
 * @param [in] packetDictionary The packet dictionary
 *
 * @return number The Recommended Cache Time
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint64_t ccnxCodecSchemaV1OptionalHeadersDecoder_GetRecommendedCacheTimeHeader(CCNxTlvDictionary *packetDictionary);

uint64_t ccnxCodecSchemaV1OptionalHeadersDecoder_GetRecommendedPathLabel(CCNxTlvDictionary *packetDictionary);

/**
 * Retrieves a TLV header that is not part of the V1 schema spec
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-NULL The header
 * @return NULL The header field does not exist
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxCodecSchemaV1OptionalHeadersDecoder_GetCustomType(CCNxTlvDictionary *packetDictionary, uint32_t key);

#endif // TransportRTA_ccnxCodecSchemaV1_OptionalHeadersDecoder_h
