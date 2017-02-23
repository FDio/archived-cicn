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
 */
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>

ssize_t
ccnxCodecSchemaV1FixedHeaderEncoder_EncodeHeader(CCNxCodecTlvEncoder *fixedHeaderEncoder, const CCNxCodecSchemaV1FixedHeader *header)
{
    assertNotNull(fixedHeaderEncoder, "Parameter fixedHeaderEncoder must be non-null");
    assertNotNull(header, "Parameter header must be non-null");
    trapIllegalValueIf(header->version != 1, "Header wrong version, must be 1");

    CCNxCodecSchemaV1InterestHeader copy;

    memcpy(&copy, header, sizeof(CCNxCodecSchemaV1InterestHeader));

    copy.packetLength = htons(header->packetLength);

    switch (header->packetType) {
        case CCNxCodecSchemaV1Types_PacketType_Interest:
            copy.returnCode = 0;
            break;

        case CCNxCodecSchemaV1Types_PacketType_InterestReturn:
            // nothing to do, all fields used
            break;

        default:
            copy.hopLimit = 0;
            copy.returnCode = 0;
            copy.flags = 0;
            break;
    }

    ccnxCodecTlvEncoder_AppendRawArray(fixedHeaderEncoder, sizeof(CCNxCodecSchemaV1FixedHeader), (uint8_t *) &copy);
    return sizeof(CCNxCodecSchemaV1FixedHeader);
}
