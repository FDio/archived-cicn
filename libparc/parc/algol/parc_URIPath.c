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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdarg.h>

#include <parc/algol/parc_URIPath.h>

#include <parc/algol/parc_URISegment.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

struct parc_uri_path {
    PARCArrayList *segments;
};

static void
_parcURIPath_Finalize(PARCURIPath **pathPtr)
{
    assertNotNull(pathPtr, "Parameter must be a non-null pointer to a pointer to a PARCURIPath instance.");

    PARCURIPath *path = *pathPtr;
    if (path != NULL) {
        parcArrayList_Destroy(&path->segments);
    }
}

parcObject_ExtendPARCObject(PARCURIPath, _parcURIPath_Finalize, parcURIPath_Copy, parcURIPath_ToString, parcURIPath_Equals,
                            parcURIPath_Compare, NULL, NULL);

PARCURIPath *
parcURIPath_Create(void)
{
    PARCURIPath *result = parcObject_CreateInstance(PARCURIPath);
    return result;
}

parcObject_ImplementAcquire(parcURIPath, PARCURIPath);

parcObject_ImplementRelease(parcURIPath, PARCURIPath);

PARCURIPath *
parcURIPath_Append(PARCURIPath *path, const PARCURISegment *segment)
{
    parcArrayList_Add(path->segments, segment);
    return path;
}

PARCURIPath *
parcURIPath_Trim(PARCURIPath *path, size_t numberToRemove)
{
    size_t length = parcArrayList_Size(path->segments);
    if (numberToRemove <= length) {
        while (numberToRemove--) {
            parcArrayList_RemoveAndDestroyAtIndex(path->segments, length - 1);
            length--;
        }
    }
    return path;
}

//PARCURIPath *
//parcURIPath_Parse(const char *string, const char **pointer)
//{
//    PARCURIPath *result = parcURIPath_Create();
//
//    result->segments = parcArrayList_Create((void (*)(void **))parcURISegment_Release);
//    if (*string != 0) {
//        assertTrue(*string == '/', "Expected initial '/' character.");
//        *pointer = string;
//        while (**pointer != 0 && **pointer != '?' && **pointer != '#') {
//            PARCURISegment *segment = parcURISegment_Parse(++(*pointer), pointer);
//            parcURIPath_Append(result, segment);
//        }
//    }
//
//    return result;
//}

PARCURIPath *
parcURIPath_Parse(const char *string, const char **pointer)
{
    PARCURIPath *result = parcURIPath_Create();
    result->segments = parcArrayList_Create((void (*)(void **))parcURISegment_Release);

    if (*string != 0) {
        if (*string != '/') {
            parcURIPath_Release(&result);
            return NULL;
        }

        *pointer = string;
        while (**pointer != 0 && **pointer != '?' && **pointer != '#') {
            PARCURISegment *segment = parcURISegment_Parse(++(*pointer), pointer);
            parcURIPath_Append(result, segment);
        }
    }

    return result;
}

bool
parcURIPath_Equals(const PARCURIPath *pathA, const PARCURIPath *pathB)
{
    if (pathA == pathB) {
        return true;
    }
    if (pathA == NULL || pathB == NULL) {
        return false;
    }

    if (parcArrayList_Size(pathA->segments) == parcArrayList_Size(pathB->segments)) {
        for (size_t i = 0; i < parcArrayList_Size(pathA->segments); i++) {
            PARCURISegment *segmentA = parcArrayList_Get(pathA->segments, i);
            PARCURISegment *segmentB = parcArrayList_Get(pathB->segments, i);
            if (!parcURISegment_Equals(segmentA, segmentB)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

PARCURIPath *
parcURIPath_Copy(const PARCURIPath *path)
{
    assertNotNull(path, "Parameter must be a non-null PARC_URIPath pointer.");

    PARCURIPath *result = parcURIPath_Create();
    result->segments = parcArrayList_Create((void (*)(void **))parcURISegment_Release);

    size_t nSegments = parcURIPath_Count(path);

    for (size_t i = 0; i < nSegments; i++) {
        PARCURISegment *segment = parcURIPath_Get(path, i);
        PARCURISegment *segmentCopy = parcURISegment_Clone(segment);
        parcURIPath_Append(result, segmentCopy);
    }

    return result;
}

PARCURIPath *
parcURIPath_ComposeValist(const PARCURIPath *basePath, va_list varargs)
{
    PARCURIPath *result = parcURIPath_Copy(basePath);

    for (PARCURISegment *segment = va_arg(varargs, PARCURISegment *); segment != NULL; segment = va_arg(varargs, PARCURISegment *)) {
        parcURIPath_Append(result, parcURISegment_Clone(segment));
    }

    return result;
}

PARCURIPath *
parcURIPath_Compose(const PARCURIPath *basePath, ...)
{
    va_list arglist;
    va_start(arglist, basePath);

    PARCURIPath *result = parcURIPath_ComposeValist(basePath, arglist);
    va_end(arglist);

    return result;
}

bool
parcURIPath_StartsWith(const PARCURIPath *base, const PARCURIPath *prefix)
{
    size_t prefixSegmentCount = parcURIPath_Count(prefix);
    size_t baseSegmentCount = parcURIPath_Count(base);

    if (baseSegmentCount < prefixSegmentCount) {
        return false;
    }

    for (size_t i = 0; i < prefixSegmentCount; i++) {
        PARCURISegment *baseSegment = parcURIPath_Get(base, i);
        PARCURISegment *prefixSegment = parcURIPath_Get(prefix, i);
        if (parcURISegment_Compare(baseSegment, prefixSegment) != 0) {
            return false;
        }
    }

    return true;
}

int
parcURIPath_Compare(const PARCURIPath *pathA, const PARCURIPath *pathB)
{
    if (pathA == NULL) {
        if (pathB == NULL) {
            return 0;
        }
        return -1;
    } else {
        if (pathB == NULL) {
            return +1;
        }
    }

    ssize_t countDifference = parcURIPath_Count(pathA) - parcURIPath_Count(pathB);

    if (countDifference != 0) {
        return (countDifference > 0 ? 1 : (countDifference < 0) ? -1 : 0);
    }

    size_t nSegments = parcURIPath_Count(pathA);

    for (size_t i = 0; i < nSegments; i++) {
        PARCURISegment *segmentA = parcURIPath_Get(pathA, i);
        PARCURISegment *segmentB = parcURIPath_Get(pathB, i);
        int comparison = parcURISegment_Compare(segmentA, segmentB);
        if (comparison != 0) {
            return comparison;
        }
    }
    return 0;
}

PARCURISegment *
parcURIPath_Get(const PARCURIPath *path, size_t index)
{
    return (PARCURISegment *) parcArrayList_Get(path->segments, index);
}

size_t
parcURIPath_Count(const PARCURIPath *path)
{
    size_t nSegments = parcArrayList_Size(path->segments);
    return nSegments;
}

size_t
parcURIPath_Length(const PARCURIPath *path)
{
    size_t result = 0;

    size_t nSegments = parcURIPath_Count(path);

    for (size_t i = 0; i < nSegments; i++) {
        PARCURISegment *segment = parcURIPath_Get(path, i);
        result += parcURISegment_Length(segment);
        if (i < (nSegments - 1)) {
            result++; // Include the size of the '/' separators.
        }
    }

    return result;
}

PARCBufferComposer *
parcURIPath_BuildString(const PARCURIPath *path, PARCBufferComposer *composer)
{
    size_t nSegments = parcArrayList_Size(path->segments);

    for (size_t i = 0; i < nSegments && composer != NULL; i++) {
        if (parcURISegment_BuildString(parcURIPath_Get(path, i), composer) == NULL) {
            composer = NULL;
        }
        if (i < (nSegments - 1)) {
            composer = parcBufferComposer_PutChar(composer, '/');
        }
    }

    return composer;
}

char *
parcURIPath_ToString(const PARCURIPath *path)
{
    char *result = NULL;

    PARCBufferComposer *composer = parcBufferComposer_Create();
    if (composer != NULL) {
        if (parcURIPath_BuildString(path, composer) != NULL) {
            PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
            result = parcBuffer_ToString(tempBuffer);
            parcBuffer_Release(&tempBuffer);
        }
        parcBufferComposer_Release(&composer);
    }

    return result;
}
