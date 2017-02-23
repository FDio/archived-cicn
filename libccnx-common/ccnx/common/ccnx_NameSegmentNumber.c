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
#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_NameSegmentNumber.h>
#include <ccnx/common/ccnx_NameSegment.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BufferComposer.h>

bool
ccnxNameSegmentNumber_IsValid(const CCNxNameSegment *nameSegment)
{
    bool result = false;

    size_t remaining = parcBuffer_Remaining(ccnxNameSegment_GetValue(nameSegment));
    if (remaining > 0 && remaining < 8) {
        result = true;
    }

    return result;
}

void
ccnxNameSegmentNumber_AssertValid(const CCNxNameSegment *nameSegment)
{
    assertTrue(ccnxNameSegmentNumber_IsValid(nameSegment), "Encountered an invalid CCNxNameSegment");
}

CCNxNameSegment *
ccnxNameSegmentNumber_Create(CCNxNameLabelType type, uint64_t value)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    bool mustContinue = false;
    for (int byte = 7; byte >= 0; byte--) {
        uint8_t b = (value >> (byte * 8)) & 0xFF;
        if (b != 0 || byte == 0 || mustContinue) {
            parcBufferComposer_PutUint8(composer, b);
            mustContinue = true;
        }
    }
    PARCBuffer *buffer = parcBuffer_Flip(parcBufferComposer_GetBuffer(composer));
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(type, buffer);
    parcBufferComposer_Release(&composer);

    return segment;
}

uint64_t
ccnxNameSegmentNumber_Value(const CCNxNameSegment *nameSegment)
{
    const PARCBuffer *buffer = ccnxNameSegment_GetValue(nameSegment);

    uint64_t result = 0;

    for (size_t i = 0; i < parcBuffer_Remaining(buffer); i++) {
        result = (result << 8) | parcBuffer_GetAtIndex(buffer, i);
    }
    return result;
}
