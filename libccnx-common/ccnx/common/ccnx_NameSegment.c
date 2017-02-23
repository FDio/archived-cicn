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
#include <string.h>
#include <strings.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_URI.h>
#include <parc/algol/parc_URISegment.h>
#include <parc/algol/parc_Varint.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_Object.h>

#include <ccnx/common/ccnx_NameSegment.h>

struct ccnx_name_segment {
    const CCNxNameLabel *label;
    CCNxNameLabelType type;
    PARCBuffer *value;
};

static bool
_ccnxNameSegment_destructor(CCNxNameSegment **segmentP)
{
    assertNotNull(segmentP, "Parameter must be a non-null pointer to a CCNxNameSegment pointer.");

    CCNxNameSegment *segment = *segmentP;
    ccnxNameLabel_Release((CCNxNameLabel **) &(segment->label));
    parcBuffer_Release(&segment->value);
    return true;
}

parcObject_Override(CCNxNameSegment, PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxNameSegment_destructor,
                    .copy = (PARCObjectCopy *) ccnxNameSegment_Copy,
                    .equals = (PARCObjectEquals *) ccnxNameSegment_Equals,
                    .compare = (PARCObjectCompare *) ccnxNameSegment_Compare,
                    .hashCode = (PARCObjectHashCode *) ccnxNameSegment_HashCode,
                    .toString = (PARCObjectToString *) ccnxNameSegment_ToString);

parcObject_ImplementAcquire(ccnxNameSegment, CCNxNameSegment);

parcObject_ImplementRelease(ccnxNameSegment, CCNxNameSegment);

CCNxNameSegment *
ccnxNameSegment_CreateLabelValue(const CCNxNameLabel *label, const PARCBuffer *value)
{
    CCNxNameSegment *result = parcObject_CreateInstance(CCNxNameSegment);
    if (result != NULL) {
        result->label = ccnxNameLabel_Acquire(label);
        result->type = ccnxNameLabel_GetType(label);
        result->value = parcBuffer_Acquire(value);
    }
    return result;
}

CCNxNameSegment *
ccnxNameSegment_CreateTypeValue(CCNxNameLabelType type, const PARCBuffer *value)
{
    CCNxNameSegment *result = NULL;
    CCNxNameLabel *label = ccnxNameLabel_Create(type, NULL);

    if (label != NULL) {
        result = ccnxNameSegment_CreateLabelValue(label, value);
        ccnxNameLabel_Release(&label);
    }
    return result;
}

CCNxNameSegment *
ccnxNameSegment_CreateTypeValueArray(CCNxNameLabelType type, size_t length, const char array[length])
{
    PARCBuffer *value = parcBuffer_PutArray(parcBuffer_Allocate(length), length, (const uint8_t *) array);
    parcBuffer_Flip(value);

    CCNxNameSegment *result = ccnxNameSegment_CreateTypeValue(type, value);
    parcBuffer_Release(&value);

    return result;
}

CCNxNameSegment *
ccnxNameSegment_ParseURISegment(const PARCURISegment *uriSegment)
{
    CCNxNameSegment *result = NULL;

    PARCBuffer *buffer = parcURISegment_GetBuffer(uriSegment);

    size_t originalPosition = parcBuffer_Position(buffer);
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);

    if (ccnxNameLabel_IsValid(label)) {
        PARCBuffer *value = parcBuffer_Slice(buffer);

        CCNxNameLabelType nameType = ccnxNameLabel_GetType(label);
        if (nameType != CCNxNameLabelType_Unknown) {
            result = ccnxNameSegment_CreateLabelValue(label, value);
        }

        ccnxNameLabel_Release(&label);
        parcBuffer_Release(&value);

        parcBuffer_SetPosition(buffer, originalPosition);
    }

    return result;
}

CCNxNameSegment *
ccnxNameSegment_Copy(const CCNxNameSegment *segment)
{
    PARCBuffer *value = parcBuffer_Copy(segment->value);

    CCNxNameLabel *label = ccnxNameLabel_Copy(segment->label);

    CCNxNameSegment *result = ccnxNameSegment_CreateLabelValue(label, value);
    ccnxNameLabel_Release(&label);

    parcBuffer_Release(&value);
    return result;
}

bool
ccnxNameSegment_Equals(const CCNxNameSegment *segmentA, const CCNxNameSegment *segmentB)
{
    bool result = false;

    if (segmentA == segmentB) {
        result = true;
    } else if (segmentA == NULL || segmentB == NULL) {
        result = false;
    } else {
        if (ccnxNameLabel_Equals(segmentA->label, segmentB->label)) {
            if (parcBuffer_Equals(ccnxNameSegment_GetValue(segmentA), ccnxNameSegment_GetValue(segmentB))) {
                result = true;
            }
        }
    }

    return result;
}

int
ccnxNameSegment_Compare(const CCNxNameSegment *segmentA, const CCNxNameSegment *segmentB)
{
    if (segmentA == NULL) {
        if (segmentB == NULL) {
            return 0;
        }
        return -1;
    } else {
        if (segmentB == NULL) {
            return +1;
        }
    }

    if (ccnxNameSegment_Length(segmentA) < ccnxNameSegment_Length(segmentB)) {
        return -1;
    }
    if (ccnxNameSegment_Length(segmentA) > ccnxNameSegment_Length(segmentB)) {
        return +1;
    }

    int result = parcBuffer_Compare(ccnxNameSegment_GetValue(segmentA), ccnxNameSegment_GetValue(segmentB));
    return result;
}

CCNxNameLabelType
ccnxNameSegment_GetType(const CCNxNameSegment *segment)
{
    return ccnxNameLabel_GetType(segment->label);
}

size_t
ccnxNameSegment_Length(const CCNxNameSegment *segment)
{
    return parcBuffer_Remaining(segment->value);
}

PARCBuffer *
ccnxNameSegment_GetValue(const CCNxNameSegment *segment)
{
    return segment->value;
}

static inline bool
_ccnxNameSegment_IsEscapable(const char c)
{
    return (c == 0 || strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~", c) == NULL);
}

static inline bool
_ccnxNameSegmentValue_IsEscaped(const PARCBuffer *value)
{
    bool result = false;

    size_t length = parcBuffer_Remaining(value);

    for (size_t i = 0; i < length; i++) {
        if (_ccnxNameSegment_IsEscapable(parcBuffer_GetAtIndex(value, i))) {
            result = true;
            break;
        }
    }

    return result;
}

PARCBufferComposer *
ccnxNameSegment_BuildString(const CCNxNameSegment *segment, PARCBufferComposer *composer)
{
    // Insert the label.  However, in the case of an unescaped Name value, the Name lable portion can be left off.

    if (ccnxNameLabel_GetType(segment->label) != CCNxNameLabelType_NAME || _ccnxNameSegmentValue_IsEscaped(segment->value)) {
        ccnxNameLabel_BuildString(segment->label, composer);
    }

    if (ccnxNameSegment_Length(segment) > 0) {
        PARCURISegment *uriSegment = parcURISegment_CreateFromBuffer(ccnxNameSegment_GetValue(segment));
        parcURISegment_BuildString(uriSegment, composer);

        parcURISegment_Release(&uriSegment);
    }

    return composer;
}

PARCHashCode
ccnxNameSegment_HashCode(const CCNxNameSegment *segment)
{
    PARCHash32Bits *hash = parcHash32Bits_Create();

    parcHash32Bits_Update(hash, &segment->type, sizeof(segment->type));
    if (parcBuffer_Remaining(segment->value) > 0) {
        parcHash32Bits_UpdateUint32(hash, parcBuffer_HashCode(segment->value));
    }

    uint32_t result = parcHash32Bits_Hash(hash);

    parcHash32Bits_Release(&hash);

    return result;
}

char *
ccnxNameSegment_ToString(const CCNxNameSegment *segment)
{
    char *result = NULL;

    PARCBufferComposer *composer = parcBufferComposer_Create();
    if (composer != NULL) {
        ccnxNameSegment_BuildString(segment, composer);
        PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
        result = parcBuffer_ToString(tempBuffer);
        parcBuffer_Release(&tempBuffer);
        parcBufferComposer_Release(&composer);
    }

    return result;
}

void
ccnxNameSegment_Display(const CCNxNameSegment *segment, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "CCNxNameSegment@%p {", segment);
    parcDisplayIndented_PrintLine(indentation + 1, "type=%d", segment->type);
    parcBuffer_Display(segment->value, indentation + 1);
    parcDisplayIndented_PrintLine(indentation, "}");
}

void
ccnxNameSegment_AssertValid(const CCNxNameSegment *segment)
{
    assertTrue(ccnxNameSegment_IsValid(segment), "CCNxNameSegment is invalid.");
}

bool
ccnxNameSegment_IsValid(const CCNxNameSegment *segment)
{
    bool result = false;

    if (segment != NULL) {
        if (parcBuffer_IsValid(segment->value) == true) {
            result = true;
        }
    }

    return result;
}
