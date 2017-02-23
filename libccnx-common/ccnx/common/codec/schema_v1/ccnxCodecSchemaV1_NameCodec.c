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
#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameSegmentCodec.h>

size_t
ccnxCodecSchemaV1NameCodec_Encode(CCNxCodecTlvEncoder *encoder, uint16_t type, const CCNxName *name)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    assertNotNull(name, "Parameter name must be non-null");

    size_t containerPosition = ccnxCodecTlvEncoder_Position(encoder);
    size_t containerLength = ccnxCodecTlvEncoder_AppendContainer(encoder, type, 0);

    size_t segmentCount = ccnxName_GetSegmentCount(name);
    size_t innerLength = 0;
    for (int i = 0; i < segmentCount; i++) {
        CCNxNameSegment *segment = ccnxName_GetSegment(name, i);
        innerLength += ccnxCodecSchemaV1NameSegmentCodec_Encode(encoder, segment);
    }

    // now go back and fixup the container's length
    ccnxCodecTlvEncoder_SetContainerLength(encoder, containerPosition, innerLength);

    return containerLength + innerLength;
}

CCNxName *
ccnxCodecSchemaV1NameCodec_Decode(CCNxCodecTlvDecoder *decoder, uint16_t type)
{
    CCNxName *name = NULL;
    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, 4)) {
        uint16_t tlvtype = ccnxCodecTlvDecoder_PeekType(decoder);
        if (tlvtype == type) {
            // call just for the side-effect of advancing the buffer
            (void) ccnxCodecTlvDecoder_GetType(decoder);
            uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

            name = ccnxCodecSchemaV1NameCodec_DecodeValue(decoder, length);
        }
    }

    return name;
}

CCNxName *
ccnxCodecSchemaV1NameCodec_DecodeValue(CCNxCodecTlvDecoder *decoder, uint16_t length)
{
    CCNxName *name = NULL;
    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, length)) {
        name = ccnxName_Create();
        size_t nameEnd = ccnxCodecTlvDecoder_Position(decoder) + length;

        while (ccnxCodecTlvDecoder_Position(decoder) < nameEnd) {
            CCNxNameSegment *segment = ccnxCodecSchemaV1NameSegmentCodec_Decode(decoder);
            ccnxName_Append(name, segment);
            ccnxNameSegment_Release(&segment);
        }
    }
    return name;
}
