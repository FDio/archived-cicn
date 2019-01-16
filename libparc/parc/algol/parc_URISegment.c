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

#include <parc/assert/parc_Assert.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdarg.h>

#include <parc/algol/parc_URISegment.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

struct parc_uri_segment {
    PARCBuffer *buffer;
    bool requiresEscaping;
};

static char *hexDigitsUpper = "0123456789ABCDEF";
static char *hexDigitsLower = "0123456789abcdef";

// Given a value, return the low nibble as a hex character.
static char
_toHexDigit(const char value)
{
    return hexDigitsUpper[value & 0xF];
}

/**
 * @function _fromHexDigit
 * @abstract Given a hex character, return the decimal value.
 * @discussion
 *   Given a simple character containing the ASCII value for a hexadecimal digit, return the actual value.
 *
 * @param hexDigit A hexadecimal digit as a character from the set of characters <code>0123456789ABCDEFabcdef</code>
 * @return Return the decimal value of the given hex character.  If not a hex character return a value > 15.
 */
static char
_fromHexDigit(const char hexDigit)
{
    for (char i = 0; i < 16; i++) {
        if (hexDigit == hexDigitsLower[(int) i] || hexDigit == hexDigitsUpper[(int) i]) {
            return i;
        }
    }
    return -1;
}

static const char *
_parsePercentEncoded(const char *string, unsigned char *value)
{
    char c = *string++;
    if (c != 0) {
        unsigned char hi = _fromHexDigit(c);
        if (hi > 15) {
            return NULL;
        }
        c = *string++;
        if (c != 0) {
            unsigned char lo = _fromHexDigit(c);
            if (lo > 15) {
                return NULL;
            }
            *value = (unsigned char) (hi << 4 | lo);
            return string;
        }
    }
    return NULL;
}

#define uriPlainSegmentChar(c) (c != 0 && strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~", c) != NULL)

static PARCBufferComposer *
_parcURISegment_BuildString(const PARCURISegment *segment, PARCBufferComposer *composer)
{
    parcAssertNotNull(composer, "Parameter must be a non-null pointer to a PARCBufferComposer.");

    for (size_t i = 0; i < parcBuffer_Limit(segment->buffer) && composer != NULL; i++) {
        unsigned char c = parcBuffer_GetAtIndex(segment->buffer, i);
        if (uriPlainSegmentChar(c)) {
            parcBufferComposer_PutChar(composer, c);
        } else {
            parcBufferComposer_PutChar(composer, '%');
            parcBufferComposer_PutChar(composer, _toHexDigit(c >> 4));
            parcBufferComposer_PutChar(composer, _toHexDigit(c));
        }
    }

    return composer;
}

static void
_parcURISegment_Finalize(PARCURISegment **segmentPtr)
{
    PARCURISegment *segment = *segmentPtr;

    parcBuffer_Release(&(segment->buffer));
}

parcObject_ExtendPARCObject(PARCURISegment, _parcURISegment_Finalize, parcURISegment_Clone, parcURISegment_ToString,
                            parcURISegment_Equals, parcURISegment_Compare, NULL, NULL);

PARCURISegment *
parcURISegment_CreateFromBuffer(PARCBuffer *buffer)
{
    PARCURISegment *result = parcObject_CreateInstance(PARCURISegment);
    if (result != NULL) {
        result->buffer = parcBuffer_Acquire(buffer);
    }
    return result;
}

PARCURISegment *
parcURISegment_Create(size_t length, const unsigned char segment[length])
{
    PARCURISegment *result = NULL;

    PARCBuffer *buffer = parcBuffer_Allocate(length);
    if (buffer != NULL) {
        parcBuffer_PutArray(buffer, length, segment);
        parcBuffer_Flip(buffer);
        result = parcURISegment_CreateFromBuffer(buffer);
        parcBuffer_Release(&buffer);
    }
    return result;
}

PARCURISegment *
parcURISegment_Parse(const char *string, const char **pointer)
{
    parcAssertFalse(*string == '/', "Input parameter '%s' must NOT point to an initial '/' character.", string);

    unsigned char *segment = parcMemory_AllocateAndClear((strlen(string) + 1) * sizeof(unsigned char));
    parcAssertNotNull(segment, "parcMemory_AllocateAndClear(%zu) returned NULL", (strlen(string) + 1) * sizeof(unsigned char));
    size_t length = 0;

    unsigned char *r = segment;

    const char *p = string;
    while (*p && *p != '/' && *p != '?' && *p != '#') {
        if (*p == '%') {
            unsigned char value;
            if ((p = _parsePercentEncoded(p + 1, &value)) == NULL) {
                parcMemory_Deallocate((void **) &segment);
                return NULL;
            }
            *r = value;
        } else {
            *r = *p++;
        }
        length++;
        r++;
    }
    if (*p != 0) {
        // absorb any extra slash characters.
        while (p[1] == '/') {
            p++;
        }
    }

    PARCURISegment *result = parcURISegment_Create(length, segment);
    parcMemory_Deallocate((void **) &segment);
    if (pointer != NULL) {
        *pointer = p;
    }
    return result;
}

parcObject_ImplementAcquire(parcURISegment, PARCURISegment);

parcObject_ImplementRelease(parcURISegment, PARCURISegment);

PARCBuffer *
parcURISegment_GetBuffer(const PARCURISegment *segment)
{
    parcBuffer_Rewind(segment->buffer);
    return segment->buffer;
}

size_t
parcURISegment_Length(const PARCURISegment *segment)
{
    parcBuffer_Rewind(segment->buffer);
    return parcBuffer_Remaining(segment->buffer);
}

bool
parcURISegment_Equals(const PARCURISegment *segmentA, const PARCURISegment *segmentB)
{
    if (segmentA == segmentB) {
        return true;
    }
    if (segmentA == NULL || segmentB == NULL) {
        return false;
    }
    return parcBuffer_Equals(segmentA->buffer, segmentB->buffer);
}

PARCURISegment *
parcURISegment_Clone(const PARCURISegment *segment)
{
    parcAssertNotNull(segment, "Parameter must be a non-null PARC_URISegment pointer.");

    PARCBuffer *copy = parcBuffer_Copy(segment->buffer);
    PARCURISegment *result = parcURISegment_CreateFromBuffer(copy);
    parcBuffer_Release(&copy);
    return result;
}

int
parcURISegment_Compare(const PARCURISegment *a, const PARCURISegment *b)
{
    if (a == NULL) {
        if (b == NULL) {
            return 0;
        }
        return -1;
    } else {
        if (b == NULL) {
            return +1;
        }
    }

    if (parcURISegment_Length(a) < parcURISegment_Length(b)) {
        return -1;
    }
    if (parcURISegment_Length(a) > parcURISegment_Length(b)) {
        return +1;
    }
    return parcBuffer_Compare(a->buffer, b->buffer);
}

PARCBufferComposer *
parcURISegment_BuildString(const PARCURISegment *segment, PARCBufferComposer *composer)
{
    composer = _parcURISegment_BuildString(segment, composer);

    return composer;
}

char *
parcURISegment_ToString(const PARCURISegment *segment)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    char *result = NULL;

    if (parcURISegment_BuildString(segment, composer)) {
        PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
        result = parcBuffer_ToString(tempBuffer);
        parcBuffer_Release(&tempBuffer);
    }
    parcBufferComposer_Release(&composer);

    return result;
}
