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
 * @file ccnxCodecSchemaV1_PacketDecoder.h
 * @brief Decoder for the version 1 TLV Packet
 *
 * The Schema version 1 Dictionary is organized in containers: FixedHeader, OptionalHeaders, (Interest, ContentObject, Control), Verification.
 *
 * Each container is its own dictionary.
 *
 * Example:
 * @code
 * {
 *      CCNxTlvDictionary *packetDictionary = ccnxTlvDictionary_Create();
 *      ccnxCodecSchemaV1PacketDecoder_Decode(packetBuffer, packetDictionary);
 *      // the fields in the packetDictionary are now set
 * }
 * @endcode
 *
 */

#ifndef CCNxCodecSchemaV1_PacketDecoder_h
#define CCNxCodecSchemaV1_PacketDecoder_h

#include <parc/algol/parc_Buffer.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>


/**
 * Decode a packet in to a dictionary.
 *
 * The buffer should be set at the start of the fixed header.  This call is equivalent
 * to ccnxCodecSchemaV1PacketDecoder_Decode(), except it allocates and destroys a temporary
 * CCNxCodecTlvDecoder.
 *
 * The dictionary will be filled in with all fields available in the packetBuffer.
 *
 * Caveat: there is no way to find out where the error was if returned "false"
 *
 * @param [in] buffer The packet buffer
 * @param [in] packetDictionary The dictionary to fill in
 *
 * @return true Successful decode
 * @return false There was an error somewhere
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
bool ccnxCodecSchemaV1PacketDecoder_BufferDecode(PARCBuffer *packetBuffer, CCNxTlvDictionary *packetDictionary);

/**
 * Decode in to in to a dictionary.
 *
 * The buffer should be set at the start of the fixed header.
 *
 * The dictionary will be filled in with all fields available in the packetDecoder.
 *
 * Caveat: there is no way to find out where the error was if returned "false"
 *
 * @param [in] buffer The packet buffer
 * @param [in] packetDictionary The dictionary to fill in
 *
 * @return true Successful decode
 * @return false There was an error somewhere
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
bool ccnxCodecSchemaV1PacketDecoder_Decode(CCNxCodecTlvDecoder *packetDecoder, CCNxTlvDictionary *packetDictionary);

#endif // CCNxCodecSchemaV1_PacketDecoder_h
