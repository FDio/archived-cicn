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
 * @file ccnxCodecSchemaV1_PacketEncoder.h
 * @brief Encoder for the version 1 TLV Packet
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 *
 */

#ifndef CCNxCodecSchemaV1_PacketEncoder_h
#define CCNxCodecSchemaV1_PacketEncoder_h

#include <parc/algol/parc_Buffer.h>
#include <parc/security/parc_Signer.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/ccnxCodec_EncodingBuffer.h>
#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>

/**
 * Encode the packetDictionary to wire format
 *
 * Will only use the PacketType from FixedHeader in the dictionary, if provided.  The packet Version is fixed at "1",
 * the PayloadLength and HeaderLength are calculated.  If the FixedHeaderDictionary is not provided, the
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
CCNxCodecNetworkBufferIoVec *ccnxCodecSchemaV1PacketEncoder_DictionaryEncode(CCNxTlvDictionary *packetDictionary, PARCSigner *signer);

/**
 * Encode a packetDictionary to wire format.
 *
 * Will only use the PacketType from FixedHeader in the dictionary, if provided.  The packet Version is fixed at "1",
 * the PayloadLength and HeaderLength are calculated.  If the FixedHeaderDictionary is not provided, the
 * PacketType is inferred from the type of CCNx message.
 *
 * You must use ccnxCodecTlvEncoder_SetSigner(signer) if you require a signature or MAC on the packet.
 *
 * @param [in] packetEncoder A TLV packet will be appended to the encoder
 * @param [in] packetDictionary The dictionary representation of the packet to encode
 *
 * @retval non-negative The total bytes appended to the encode buffer
 * @retval -1 An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t ccnxCodecSchemaV1PacketEncoder_Encode(CCNxCodecTlvEncoder *packetEncoder, CCNxTlvDictionary *packetDictionary);

#endif // CCNxCodecSchemaV1_PacketEncoder_h
