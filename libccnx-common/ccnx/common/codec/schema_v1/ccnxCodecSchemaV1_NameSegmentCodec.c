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
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameSegmentCodec.h>

size_t
ccnxCodecSchemaV1NameSegmentCodec_Encode(CCNxCodecTlvEncoder *encoder, CCNxNameSegment *segment)
{
    assertTrue(ccnxNameSegment_Length(segment) <= UINT16_MAX,
               "Name segment too long!  length %zu maximum %u",
               ccnxNameSegment_Length(segment),
               UINT16_MAX);

    uint16_t segment_type = ccnxNameSegment_GetType(segment);
    PARCBuffer *value = ccnxNameSegment_GetValue(segment);

    return ccnxCodecTlvEncoder_AppendBuffer(encoder, segment_type, value);
}

CCNxNameSegment *
ccnxCodecSchemaV1NameSegmentCodec_Decode(CCNxCodecTlvDecoder *decoder)
{
    CCNxNameSegment *segment = NULL;

    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, 4)) {
        uint16_t type = ccnxCodecTlvDecoder_GetType(decoder);
        uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

        if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, length)) {
            PARCBuffer *value = ccnxCodecTlvDecoder_GetValue(decoder, length);
            segment = ccnxNameSegment_CreateTypeValue(type, value);
            parcBuffer_Release(&value);
        }
    }

    return segment;
}
