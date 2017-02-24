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
 * @file rta_TlvPacket.h
 * @brief Encode and decode a packet using the TLV 1.1 codec
 *
 * Will choose the appropriate schema based on the packet version
 *
 */

#ifndef TransportRTA_rta_TlvPacketDecoder_h
#define TransportRTA_rta_TlvPacketDecoder_h

#include <parc/algol/parc_Buffer.h>
#include <parc/security/parc_Signer.h>

#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * Decodes a packet in to a dictionary
 *
 * The buffer must point to byte 0 of the FixedHeader.  It may extend beyond the
 * end of the packet.
 *
 * @param [in] packetBuffer The wire format representation of a packet
 *
 * @retval non-null An allocated dictionary
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxTlvDictionary *ccnxCodecTlvPacket_Decode(PARCBuffer *packetBuffer);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvPacket_BufferDecode(PARCBuffer *packetBuffer, CCNxTlvDictionary *packetDictionary);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvPacket_IoVecDecode(CCNxCodecNetworkBufferIoVec *vec, CCNxTlvDictionary *packetDictionary);


/**
 * Encode the packetDictionary to wire format
 *
 * Will only use the PacketType from FixedHeader in the dictionary, if provided.  The packet Version is based
 * on the dictionary schema version, and the length fields of the fixed header are calculated.
 * If the FixedHeaderDictionary is not provided, the
 * PacketType is inferred from the type of CCNx message.
 *
 * The signer is not stored beyond the call to DictionaryEncode.
 * If the dictionary already has a ValidationAlg and ValidationPayload, those are used, not the Signer.
 * Otherwise, if the signer is not null, it is used to sign the wire format.
 *
 * @param [in] packetDictionary The dictionary representation of the packet to encode
 * @param [in] signer If not NULL will be used to sign the wire format
 *
 * @retval non-null An IoVec that can be written to the network
 * @retval null an error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxCodecNetworkBufferIoVec *ccnxCodecTlvPacket_DictionaryEncode(CCNxTlvDictionary *packetDictionary, PARCSigner *signer);

/**
 * Return the length of the wire format packet based on information in the header
 *
 * @param [in] packetBuffer a PARCBuffer containing, at least, the wire format header
 * @return length of the message in bytes
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecTlvPacket_GetPacketLength(PARCBuffer *packetBuffer);

/**
 * Return the minimal header that must be read to determine type and packet size
 *
 * @return length of the header which must be read in bytes
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecTlvPacket_MinimalHeaderLength();

#endif // TransportRTA_rta_TlvPacketDecoder_h
