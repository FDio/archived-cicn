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
 * @file metis_Tlv.h
 * @brief The generic Tlv utilities.
 *
 * Provides generaic Tlv utilities, particularly for packets that have not been
 * decoded in to their skeleton.  Once packets are in the skeleton format, one should
 * use functions in metis_TlvSkeleton.
 *
 */

#ifndef Metis_metis_Tlv_h
#define Metis_metis_Tlv_h

#include <stdlib.h>
#include <parc/security/parc_CryptoHash.h>
#include <ccnx/api/control/cpi_ControlMessage.h>
#include <parc/algol/parc_ByteArray.h>

#include <ccnx/forwarder/metis/tlv/metis_TlvExtent.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvSkeleton.h>

/**
 * The TLV format
 *
 *   Mapping of the TLV format to a structure.  Remember to use
 *   <code>htons()</code> or <code>ntohs()</code> on the values
 *   if working in host byte order.
 *
 *   The 'length' is the length of the 'value', it does not include
 *   the T and L of the TLV.  A length of "0" is acceptable.
 *
 * @param type is in network byte order
 * @param length is in network byte order
 *
 * Example:
 * @code
 * {
 *    uint8_t *packet = // packet received from network
 *    size_t offset = // where you are in parsing the packet
 *
 *    MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
 *    uint16_t type = htons(tlv->type);
 *    uint16_t v_length = htons(tlv->length);
 * }
 * @endcode
 */
typedef struct __attribute__ ((packed)) metis_tlv_type {
    uint16_t type;
    uint16_t length;
} MetisTlvType;


/**
 * Returns the length of the fixed header
 *
 * This is assumed to be the same for all versions.  At some point this will no longer be true
 * and metis will need to be re-factored.  This function works for V0 and V1 packets.
 *
 * @return positive The bytes of the fixed header.
 *
 * Example:
 * @code
 * {
 *    if (parcEventBuffer_GetLength(input) >= metisTlv_FixedHeaderLength()) {
 *       uint8_t fixedheader[metisTlv_FixedHeaderLength()];
 *       read(fd, &fixedheader, metisTlv_FixedHeaderLength());
 *       // process fixed header
 *    }
 * }
 * @endcode
 */
size_t metisTlv_FixedHeaderLength(void);

/**
 * Returns the length of all headers, which is the offset where the CCNx message starts
 *
 * Includes both the fixed header and the per hop headers.
 * Will return "0" for unknown packet version
 *
 * @param [in] packet Pointer to bytes 0 of the fixed header
 *
 * @retval positive The total header length (minimum 8)
 * @retval 0 Unsupported packet version or other error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t metisTlv_TotalHeaderLength(const uint8_t *packet);

/**
 * The total packet length based on the fixed header
 *
 * Parses the fixed header and returns the total packet length
 * Will return "0" for unknown packet version
 *
 * @param [in] packet Packet memory pointer
 *
 * @retval number Total packet length
 *
 * Example:
 * @code
 * {
 *    // in an example packet parser (does not handle any error conditions or partial reads)
 *    if (parcEventBuffer_GetLength(input) >= metisTlv_FixedHeaderLength()) {
 *        uint8_t fixedheader[metisTlv_FixedHeaderLength()];
 *        read(fd, &fixedheader, metisTlv_FixedHeaderLength());
 *
 *        size_t remainingBytes = metisTlv_TotalPacketLength(&fixedheader) -  metisTlv_FixedHeaderLength();
 *        if (parcEventBuffer_GetLength(input) >= remainingBytes) {
 *            uint8_t *packet = parcMemory_Allocate(metisTlv_TotalPacketLength(&fixedheader));
 *            read(fd, packet + metisTlv_FixedHeaderLength(), remainingBytes);
 *        }
 *    }
 * }
 * @endcode
 */
size_t metisTlv_TotalPacketLength(const uint8_t *packet);

/**
 * @function metisTlv_NameSegments
 * @abstract Treats the input as a TLV-encoded name, generating an output array of name segment extents
 * @discussion
 *   The outputArray is an ordered list of extents, giving the offset and length of each path segment.
 *   The lengths include the path segment type, length, and value.
 *
 *   Example:  Lets represent the name as a set of of tuples of T, L, and V:
 *             (t=1, len=4, value="help"), (t=1, len=2, value="me"), (t=7, len=10, value="understand")
 *             This name as 3 path segments  The first segment is of type 1, length 4, and value "help".
 *             The total length of that segment is 4 + 4 = 8, including the T and L.
 *             The outputArray would be { {.offset=0, .length=8}, {.offset=8, .length=6}, {.offset=14, .length=14} }.
 *             The outputLenght would be 3, because there are 3 elements in the array.
 *
 * @param name is a TLV-encoded name, not including the container name TLV
 * @param nameLength is the length of the name
 * @param outputArrayPtr is an allocated array of ordered extents, must be freed with <code>parcMemory_Deallocate()</code>
 * @param outputLengthPtr is the number of elements allocated in the array.
 *
 * Example:
 * @code
 * {
 *    MetisTlvExtent *extentArray;
 *    size_t arrayLength;
 *    uint8_t encodedName[] = "\x00\x01\x00\x05" "apple" "\x00\x01\x00\x03" "pie";
 *    metisTlv_NameSegments(encodedName, sizeof(encodedName), &extentArray, &arrayLength);
 *    // arrrayLength = 2
 *    // extentArray[1].offset = 4
 *    // extentArray[1].length = 5
 *    // etc.
 *    parcMemory_Deallocate(&extentArray);
 * }
 * @endcode
 */
void metisTlv_NameSegments(uint8_t *name, size_t nameLength, MetisTlvExtent **outputArrayPtr, size_t *outputLengthPtr);

/**
 * Given a CCNxControl packet, encode it in the proper schema
 *
 * Based on the dictionary schema version, will encode the control packet with the
 * correct encoder.
 *
 * @param [in] cpiControlMessage A an allocated control message
 *
 * @retval non-null An allocation wire format buffer
 * @retval null An error (likely unsupported schema version)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *metisTlv_EncodeControlPlaneInformation(const CCNxControl *cpiControlMessage);

/**
 * Parse an extent as a VarInt
 *
 * The extent must be 1 to 8 bytes.
 *
 * @param [in] packet The packet memory pointer
 * @param [in] extent The byte extent of the varint buffer
 * @param [out] output The VarInt value
 *
 * @retval true The buffer was correctly parsed
 * @retval false The buffer was not parsed (likely extent length is 0 or greater than 8)
 *
 * Example:
 * @code
 * {
 *     uint8_t packet[] = { 0x00, 0x03, 0x00, 0x03, 0xa0, 0xa1, 0xa3 };
 *     MetisTlvExtent extent = { .offset = 4, .length = 3 };
 *     uint64_t output;
 *     metisTlv_ExtentToVarInt(packet, &extent, &output);
 *     // output = 0xa0a1a3
 * }
 * @endcode
 */
bool metisTlv_ExtentToVarInt(const uint8_t *packet, const MetisTlvExtent *extent, uint64_t *output);

#endif // Metis_metis_Tlv_h
