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
 * @file ccnxCodecSchemaV1_ManifestDecoder.h
 * @brief Decode the body of a CCNx Manifest.
 *
 */

#ifndef TransportRTA_ccnxCodecSchemaV1_ManifestDecoder_h
#define TransportRTA_ccnxCodecSchemaV1_ManifestDecoder_h

#include <stdbool.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>

/**
 * Decode a V1 Manifest.
 *
 * The decoder should point to byte 0 of a Manifest (message) TLV.
 * The results are put in the provided dictionary.
 * It is an error if the message does not extend to the end of
 * the decoder.
 *
 * @param [in] decoder The decoder to parse
 * @param [in] manifestDictionary The results go directly in to the provided dictionary.
 *
 * @return true Fully parsed interest, no errors
 * @return false Error decoding, decoder is left pointing to the first byte of the error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecSchemaV1ManifestDecoder_Decode(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *manifestDictionary);

#endif // TransportRTA_ccnxCodecSchemaV1_ManifestDecoder_h
