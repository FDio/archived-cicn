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
 * @file tlv_NameCodec.h
 * @brief Encode/Decode a Name tlv
 *
 * Encodes a CCNxName to a Name TLV container plus one NameComponent TLV container
 * per name segment.
 *
 * Decodes a buffer as a Name TLV that contains one NameComponent TLV per name segment.
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#ifndef Metis_metis_tlv_NameCodec_h
#define Metis_metis_tlv_NameCodec_h

#include <ccnx/common/ccnx_Name.h>

/**
 * Decodes a byte array as the segments of a Name.
 *
 * The (buffer + offset) should point to the beginning of the first NameSegment.
 *
 * The length (end - offset) may be 0 length, in which case an empty name is returned.
 * Otherwise, it must be at least 4 bytes long.
 *
 * @param [in] buffer The byte array
 * @param [in] offset The starting location of the Name
 * @param [in] end The location just past the end of the name
 *
 * @return non-null the Name
 *
 * Example:
 * @code
 * {
 *            // offset        0     1     2     3     4     5     6     7     8   9   10   11   12     13
 *                               |-- type --|-- length --|-- type --|-- length --| ----- value -----|
 *     uint8_t buffer[] = { 0xFF, 0x00, 0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0x04, 'a', 'b', 'c', 'd', 0xFF };
 *
 *     // skip the two 0xFF bytes
 *     // name = "lci:/%02=abcd"
 *     CCNxName * name = tlvName_Decode(buffer, 5, 13);
 * }
 * @endcode
 *
 */
CCNxName *metisTlvNameCodec_Decode(uint8_t *buffer, size_t offset, size_t end);
#endif // Metis_metis_tlv_NameCodec_h
