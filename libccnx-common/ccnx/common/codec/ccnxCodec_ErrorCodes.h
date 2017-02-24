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
 * @file ccnxCodec_ErrorCodes.h
 * @brief The error codes used by CCNxCodecError.
 *
 * The codecs in schema_v0 and schema_v1 use these error codes to report problems inside
 * CCNxCodec_TlvEncoder and CCNxCodec_TlvDecoder.
 *
 */

#ifndef CCNx_Common_ccnxCodec_ErrorCodes_h
#define CCNx_Common_ccnxCodec_ErrorCodes_h

/**
 * @typedef <#CCNBHeaderType#>
 * @abstract <#Abstract#>
 * @constant <#name#> <#description#>
 * @discussion <#Discussion#>
 */
typedef enum ccnx_codec_error_codes {
    TLV_ERR_NO_ERROR,
    TLV_ERR_VERSION,
    TLV_ERR_PACKETTYPE,
    TLV_ERR_BEYOND_PACKET_END,
    TLV_ERR_TOO_LONG,
    TLV_ERR_NOT_FIXED_SIZE,
    TLV_ERR_DUPLICATE_FIELD,
    TLV_ERR_EMPTY_SPACE,

    // generic error for decoding error
    TLV_ERR_DECODE,

    TLV_ERR_PACKETLENGTH_TOO_SHORT,
    TLV_ERR_HEADERLENGTH_TOO_SHORT,
    TLV_ERR_PACKETLENGTHSHORTER,


    // errors for missing mandatory fields
    TLV_MISSING_MANDATORY,
} CCNxCodecErrorCodes;
#endif
