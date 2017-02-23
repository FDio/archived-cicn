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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_Name.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_URI.h>
#include <parc/algol/parc_URIPath.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Object.h>

struct ccnx_name {
    PARCLinkedList *segments;
};

static bool
_ccnxName_Destructor(CCNxName **pointer)
{
    CCNxName *name = *pointer;

    parcLinkedList_Release(&name->segments);
    return true;
}

parcObject_Override(CCNxName, PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxName_Destructor,
                    .copy = (PARCObjectCopy *) ccnxName_Copy,
                    .equals = (PARCObjectEquals *) ccnxName_Equals,
                    .compare = (PARCObjectCompare *) ccnxName_Compare,
                    .hashCode = (PARCObjectHashCode *) ccnxName_HashCode,
                    .toString = (PARCObjectToString *) ccnxName_ToString,
                    .display = (PARCObjectDisplay *) ccnxName_Display);
CCNxName *
ccnxName_Create(void)
{
    CCNxName *result = parcObject_CreateInstance(CCNxName);

    if (result != NULL) {
        result->segments = parcLinkedList_Create();
    }

    return result;
}

parcObject_ImplementAcquire(ccnxName, CCNxName);

parcObject_ImplementRelease(ccnxName, CCNxName);

void
ccnxName_AssertValid(const CCNxName *name)
{
    trapIllegalValueIf(ccnxName_IsValid(name) == false, "CCNxName instance is not valid.");
}

bool
ccnxName_IsValid(const CCNxName *name)
{
    bool result = false;

    if (name != NULL) {
        result = parcLinkedList_IsValid(name->segments);
    }

    return result;
}

CCNxName *
ccnxName_Copy(const CCNxName *originalName)
{
    ccnxName_OptionalAssertValid(originalName);

    CCNxName *result = ccnxName_Create();

    if (result != NULL) {
        for (int i = 0; i < ccnxName_GetSegmentCount(originalName); i++) {
            CCNxNameSegment *component = ccnxName_GetSegment(originalName, i);
            CCNxNameSegment *copy = ccnxNameSegment_Copy(component);
            ccnxName_Append(result, copy);
            ccnxNameSegment_Release(&copy);
        }
    }

    return result;
}

bool
ccnxName_Equals(const CCNxName *a, const CCNxName *b)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }

    if (ccnxName_GetSegmentCount(a) == ccnxName_GetSegmentCount(b)) {
        return parcLinkedList_Equals(a->segments, b->segments);
    }
    return false;
}

CCNxName *
ccnxName_CreateFormatString(const char *restrict format, ...)
{
    va_list argList;
    va_start(argList, format);

    PARCURI *uri = parcURI_CreateFromValist(format, argList);

    CCNxName *result = ccnxName_FromURI(uri);
    parcURI_Release(&uri);

    return result;
}

CCNxName *
ccnxName_FromURI(const PARCURI *uri)
{
    CCNxName *result = NULL;

    PARCURIPath *path = parcURI_GetPath(uri);
    if (path != NULL) {
        result = ccnxName_Create();

        for (int i = 0; i < parcURIPath_Count(path); i++) {
            CCNxNameSegment *segment = ccnxNameSegment_ParseURISegment(parcURIPath_Get(path, i));
            if (segment == NULL) {
                ccnxName_Release(&result);
                break;
            }
            parcLinkedList_Append(result->segments, segment);
            ccnxNameSegment_Release(&segment);
        }
    }

    return result;
}

CCNxName *
ccnxName_CreateFromCString(const char *uri)
{
    CCNxName *result = NULL;

    PARCURI *parcURI = parcURI_Parse(uri);
    if (parcURI != NULL) {
        const char *scheme = parcURI_GetScheme(parcURI);
        if (strcmp("lci", scheme) == 0 || strcmp("ccnx", scheme) == 0) {
            result = ccnxName_FromURI(parcURI);
        }
        parcURI_Release(&parcURI);
    }

    return result;
}

CCNxName *
ccnxName_CreateFromBuffer(const PARCBuffer *buffer)
{
    char *string = parcBuffer_ToString(buffer);
    CCNxName *result = ccnxName_CreateFromCString(string);
    parcMemory_Deallocate(&string);

    return result;
}

CCNxName *
ccnxName_ComposeNAME(const CCNxName *name, const char *suffix)
{
    CCNxNameSegment *suffixSegment = ccnxNameSegment_CreateTypeValueArray(CCNxNameLabelType_NAME, strlen(suffix), suffix);

    CCNxName *result = ccnxName_Append(ccnxName_Copy(name), suffixSegment);
    ccnxNameSegment_Release(&suffixSegment);

    return result;
}

CCNxName *
ccnxName_Append(CCNxName *name, const CCNxNameSegment *segment)
{
    ccnxName_OptionalAssertValid(name);
    ccnxNameSegment_OptionalAssertValid(segment);

    parcLinkedList_Append(name->segments, segment);

    return name;
}

PARCBufferComposer *
ccnxName_BuildString(const CCNxName *name, PARCBufferComposer *composer)
{
    parcBufferComposer_PutString(composer, "ccnx:");

    size_t count = ccnxName_GetSegmentCount(name);
    if (count == 0) {
        parcBufferComposer_PutString(composer, "/");
    } else {
        for (size_t i = 0; i < count; i++) {
            parcBufferComposer_PutString(composer, "/");
            CCNxNameSegment *component = ccnxName_GetSegment(name, i);
            ccnxNameSegment_BuildString(component, composer);
        }
    }
    return composer;
}

char *
ccnxName_ToString(const CCNxName *name)
{
    char *result = NULL;

    PARCBufferComposer *composer = parcBufferComposer_Create();
    if (composer != NULL) {
        ccnxName_BuildString(name, composer);
        PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
        result = parcBuffer_ToString(tempBuffer);
        parcBuffer_Release(&tempBuffer);
        parcBufferComposer_Release(&composer);
    }

    return result;
}

CCNxNameSegment *
ccnxName_GetSegment(const CCNxName *name, size_t index)
{
    return parcLinkedList_GetAtIndex(name->segments, index);
}

size_t
ccnxName_GetSegmentCount(const CCNxName *name)
{
    return parcLinkedList_Size(name->segments);
}

int
ccnxName_Compare(const CCNxName *name1, const CCNxName *name2)
{
    if (name1 == NULL) {
        if (name2 == NULL) {
            return 0;
        }
        return -1;
    }

    // name1 is not NULL
    if (name2 == NULL) {
        return +1;
    }

    // neither is NULL

    size_t name1SegmentCount = ccnxName_GetSegmentCount(name1);
    size_t name2SegmentCount = ccnxName_GetSegmentCount(name2);

    size_t mininimumSegments = name1SegmentCount < name2SegmentCount ? name1SegmentCount : name2SegmentCount;

    int result = 0;

    for (size_t i = 0; i < mininimumSegments; i++) {
        CCNxNameSegment *segment1 = ccnxName_GetSegment(name1, i);
        CCNxNameSegment *segment2 = ccnxName_GetSegment(name2, i);
        result = ccnxNameSegment_Compare(segment1, segment2);
        if (result != 0) {
            break;
        }
    }

    if (result == 0) {
        // we got to the end of the shortest name and they are still equal.

        // name1 is shorter than name 2
        if (name1SegmentCount < name2SegmentCount) {
            result = -1;
        }

        // name1 is longer than name2
        if (name1SegmentCount > name2SegmentCount) {
            result = +1;
        }
    }

    return result;
}

PARCHashCode
ccnxName_HashCode(const CCNxName *name)
{
    return ccnxName_LeftMostHashCode(name, ccnxName_GetSegmentCount(name));
}

PARCHashCode
ccnxName_LeftMostHashCode(const CCNxName *name, size_t count)
{
    if (count > ccnxName_GetSegmentCount(name)) {
        count = ccnxName_GetSegmentCount(name);
    }

    PARCHashCode result = 0;
    for (int i = 0; i < count; i++) {
        PARCHashCode hashCode = ccnxNameSegment_HashCode(ccnxName_GetSegment(name, i));
        result = parcHashCode_HashHashCode(result, hashCode);
    }

    return result;
}

CCNxName *
ccnxName_Trim(CCNxName *name, size_t numberToRemove)
{
    if (numberToRemove > ccnxName_GetSegmentCount(name)) {
        numberToRemove = ccnxName_GetSegmentCount(name);
    }

    for (int i = 0; i < numberToRemove; i++) {
        CCNxNameSegment *segment = parcLinkedList_RemoveLast(name->segments);
        ccnxNameSegment_Release(&segment);
    }

    return name;
}

bool
ccnxName_StartsWith(const CCNxName *name, const CCNxName *prefix)
{
    if (ccnxName_GetSegmentCount(prefix) > ccnxName_GetSegmentCount(name)) {
        return false;
    }

    for (int i = 0; i < ccnxName_GetSegmentCount(prefix); i++) {
        CCNxNameSegment *prefix_comp = ccnxName_GetSegment(prefix, i);
        CCNxNameSegment *other_comp = ccnxName_GetSegment(name, i);

        if (ccnxNameSegment_Compare(prefix_comp, other_comp) != 0) {
            return false;
        }
    }
    return true;
}

void
ccnxName_Display(const CCNxName *name, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "CCNxName@%p {", name);
    if (name != NULL) {
        for (int i = 0; i < ccnxName_GetSegmentCount(name); i++) {
            CCNxNameSegment *segment = ccnxName_GetSegment(name, i);
            ccnxNameSegment_Display(segment, indentation + 1);
        }
    }
    parcDisplayIndented_PrintLine(indentation, "}");
}

CCNxName *
ccnxName_ComposeFormatString(const CCNxName *baseName, const char *restrict format, ...)
{
    va_list argList;
    va_start(argList, format);

    char *baseString = ccnxName_ToString(baseName);

    char *suffix;
    vasprintf(&suffix, format, argList);

    char *uri;
    asprintf(&uri, "%s/%s", baseString, suffix);
    free(suffix);

    CCNxName *result = ccnxName_CreateFromCString(uri);
    free(uri);

    return result;
}

CCNxName *
ccnxName_CreatePrefix(const CCNxName *name, size_t length)
{
    CCNxName *result = ccnxName_Create();

    if (result != NULL) {
        if (length > 0) {
            size_t numberOfSegmentsAvailable = parcLinkedList_Size(name->segments);

            if (length > numberOfSegmentsAvailable) {
                length = numberOfSegmentsAvailable;
            }

            for (size_t i = 0; i < length; i++) {
                ccnxName_Append(result, ccnxName_GetSegment(name, i));
            }
        }
    }

    return result;
}
