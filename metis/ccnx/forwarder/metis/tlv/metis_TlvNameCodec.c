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

#include <config.h>
#include <stdio.h>

#include <arpa/inet.h>

#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

#include "metis_TlvNameCodec.h"
#include "metis_Tlv.h"

CCNxName *
metisTlvNameCodec_Decode(uint8_t *buffer, size_t offset, size_t end)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");
    assertTrue(end >= offset, "Buffer must be at least 4 bytes");

    CCNxName *ccnxName = ccnxName_Create();

    while (offset < end) {
        trapIllegalValueIf(end < offset + 4, "Buffer must be at least 4 bytes")
        {
            ccnxName_Release(&ccnxName);
        }

        MetisTlvType *tlv = (MetisTlvType *) (buffer + offset);
        uint16_t type = htons(tlv->type);
        uint16_t length = htons(tlv->length);

        offset += sizeof(MetisTlvType);

        trapIllegalValueIf(offset + length > end, "name component extends beyond end of name")
        {
            ccnxName_Release(&ccnxName);
        }

        PARCBuffer *nameValue = parcBuffer_Wrap(&buffer[offset], length, 0, length);
        CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(type, nameValue);
        parcBuffer_Release(&nameValue);

        ccnxName_Append(ccnxName, segment);
        ccnxNameSegment_Release(&segment);

        offset += length;
    }

    return ccnxName;
}

