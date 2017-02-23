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
 * @file ccnxCodecSchemaV1_ValidationDecoder.h
 * @brief Decode the validation algorithm and payload
 *
 * <#Detailed Description#>
 *
 */

#ifndef __CCNx_Common__ccnxCodecSchemaV1_ValidationDecoder__
#define __CCNx_Common__ccnxCodecSchemaV1_ValidationDecoder__

#include <stdbool.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>

/**
 * The decode the validation algorithm
 *
 * The decoder should point to byte 0 of the valdiation algorithm "value"
 *
 * The results are put in the provided dictionary.
 * It is an error if the "value" does not extend to the end of
 * the decoder.
 *
 * @param [in] decoder The decoder to parse
 * @param [in] packetDictionary The results go directly in to the provided dictionary.
 *
 * @return true Fully parsed, no errors
 * @return false Error decoding, decoder is left pointing to the first byte of the error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecSchemaV1ValidationDecoder_DecodeAlg(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary);

/**
 * The decode the validation payload
 *
 * The decoder should point to byte 0 of the valdiation payload "value"
 * The payload is an opaque block, so this function will just put the "value" in to the proper
 * dictionary location.  There's no real parsing.
 *
 * @param [in] decoder The decoder to parse
 * @param [in] packetDictionary The results go directly in to the provided dictionary.
 *
 * @return true Fully parsed, no errors
 * @return false Error decoding, decoder is left pointing to the first byte of the error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecSchemaV1ValidationDecoder_DecodePayload(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary);

#endif /* defined(__CCNx_Common__ccnxCodecSchemaV1_ValidationDecoder__) */
