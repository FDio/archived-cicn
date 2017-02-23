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
 * A name built around the TLV representation.
 *
 * A common operation is to get a sub-string of the name, specifically prefixes.  Use
 * metisTlvName_Slice() for that.
 *
 * To be efficient about Slice(), the reference counters are pointers, so every allocated
 * copy shares the reference counter and all the allocated memory.  Each Slice() will do
 * one malloc for the new shell and do a shallow memcpy of the struct.  Destroy will always
 * free the shell, but will only free the guts when the shared reference count goes to zero.
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_BufferComposer.h>

#include <ccnx/forwarder/metis/tlv/metis_TlvName.h>
#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvNameCodec.h>

#include <LongBow/runtime.h>

struct metis_tlv_name {
    uint8_t *memory;
    size_t memoryLength;

    // the refcount is shared between all copies
    unsigned  *refCountPtr;

    // the memory extents of each path segment
    MetisTlvExtent *segmentArray;
    size_t segmentArrayLength;

    // hashes of the name through different prefix lengths
    // It is allocated out to the limit (same assegmentArrayLength),
    // but only computed so far through segmentCumulativeHashArrayLength
    // This is to avoid computing the hash over unnecessary suffix segments.
    size_t segmentCumulativeHashArrayLimit;

    // the cumulative hash array length is shared between all copies, so if
    // one copy extends the array, all copies see it
    size_t *segmentCumulativeHashArrayLengthPtr;
    uint32_t *segmentCumulativeHashArray;
};

// =====================================================

static unsigned
_getRefCount(const MetisTlvName *name)
{
    return *name->refCountPtr;
}

static void
_incrementRefCount(MetisTlvName *name)
{
    assertTrue(*name->refCountPtr > 0, "Illegal State: Trying to increment a 0 refcount!");
    (*name->refCountPtr)++;
}

static void
_decrementRefCount(MetisTlvName *name)
{
    assertTrue(*name->refCountPtr > 0, "Illegal State: Trying to decrement a 0 refcount!");
    (*name->refCountPtr)--;
}

// ============================================================================

/**
 * Common parts of setting up a MetisTlvName after the backing memory has been allocated and copied in to.
 *
 * PRECONDITIONS: name->memory and name->memoryLength set
 */
static void
_setup(MetisTlvName *name)
{
    name->refCountPtr = parcMemory_Allocate(sizeof(unsigned));
    assertNotNull(name->refCountPtr, "parcMemory_Allocate(%zu) returned NULL", sizeof(unsigned));
    *name->refCountPtr = 1;

    metisTlv_NameSegments(name->memory, name->memoryLength, &name->segmentArray, &name->segmentArrayLength);

    name->segmentCumulativeHashArray = parcMemory_Allocate(name->segmentArrayLength * sizeof(uint32_t));
    assertNotNull(name->segmentCumulativeHashArray, "parcMemory_Allocate(%zu) returned NULL", name->segmentArrayLength * sizeof(uint32_t));

    name->segmentCumulativeHashArrayLengthPtr = parcMemory_Allocate(sizeof(size_t));
    assertNotNull(name->segmentCumulativeHashArrayLengthPtr, "parcMemory_Allocate(%zu) returned NULL", sizeof(size_t));

    *name->segmentCumulativeHashArrayLengthPtr = 1;
    name->segmentCumulativeHashArrayLimit = name->segmentArrayLength;


    if (name->segmentArrayLength > 0) {
        // always hash the 1st name component.  This is needed as the initial case
        // to do the cumulative hashes in metisTlvName_HashCode
        name->segmentCumulativeHashArray[0] = parcHash32_Data(&name->memory[name->segmentArray[0].offset], name->segmentArray[0].length);
    }
}

MetisTlvName *
metisTlvName_Create(const uint8_t *memory, size_t memoryLength)
{
    MetisTlvName *name = parcMemory_AllocateAndClear(sizeof(MetisTlvName));
    assertNotNull(name, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisTlvName));

    name->memory = parcMemory_Allocate(memoryLength);
    assertNotNull(name->memory, "parcMemory_Allocate(%zu) returned NULL", memoryLength);

    memcpy(name->memory, memory, memoryLength);
    name->memoryLength = memoryLength;

    _setup(name);

    return name;
}

MetisTlvName *
metisTlvName_CreateFromCCNxName(const CCNxName *ccnxName)
{
    // to avoid reallocs, calculate the exact size we need
    size_t memoryLength = 0;
    for (size_t i = 0; i < ccnxName_GetSegmentCount(ccnxName); i++) {
        CCNxNameSegment *segment = ccnxName_GetSegment(ccnxName, i);
        memoryLength += 4 + ccnxNameSegment_Length(segment);
    }

    MetisTlvName *name = parcMemory_AllocateAndClear(sizeof(MetisTlvName));
    assertNotNull(name, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisTlvName));

    name->memoryLength = memoryLength;
    name->memory = parcMemory_Allocate(memoryLength);
    assertNotNull(name->memory, "parcMemory_Allocate(%zu) returned NULL", memoryLength);

    uint8_t *p = name->memory;
    uint8_t *end = p + memoryLength;
    for (size_t i = 0; i < ccnxName_GetSegmentCount(ccnxName); i++) {
        CCNxNameSegment *segment = ccnxName_GetSegment(ccnxName, i);
        uint16_t type = ccnxNameSegment_GetType(segment);
        uint16_t length = ccnxNameSegment_Length(segment);

        *(uint16_t *) p = htons(type);
        p += 2;

        *(uint16_t *) p = htons(length);
        p += 2;

        if (length >0) {
            PARCBuffer *buffer = ccnxNameSegment_GetValue(segment);
            uint8_t *overlay = parcBuffer_Overlay(buffer, 0);
            memcpy(p, overlay, length);

            p += length;
        }

        // sanity check
        assertTrue(p <= end, "Wrote past the end of buffer, now at %p end at %p", p, end);
    }

    _setup(name);
    return name;
}

void
metisTlvName_Release(MetisTlvName **namePtr)
{
    assertNotNull(namePtr, "Parameter must be non-null double pointer");
    assertNotNull(*namePtr, "Parameter must dereference to non-null pointer");

    MetisTlvName *name = *namePtr;
    _decrementRefCount(name);
    if (_getRefCount(name) == 0) {
        parcMemory_Deallocate((void **) &(name->refCountPtr));
        parcMemory_Deallocate((void **) &(name->segmentArray));
        parcMemory_Deallocate((void **) &(name->segmentCumulativeHashArray));
        parcMemory_Deallocate((void **) &(name->segmentCumulativeHashArrayLengthPtr));
        parcMemory_Deallocate((void **) &(name->memory));
    }
    parcMemory_Deallocate((void **) &name);
    *namePtr = NULL;
}

MetisTlvName *
metisTlvName_Acquire(const MetisTlvName *original)
{
    return metisTlvName_Slice(original, UINT_MAX);
}

MetisTlvName *
metisTlvName_Slice(const MetisTlvName *original, size_t segmentCount)
{
    assertNotNull(original, "Parameter must be non-null");
    MetisTlvName *copy = parcMemory_AllocateAndClear(sizeof(MetisTlvName));
    assertNotNull(copy, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisTlvName));

    memcpy(copy, original, sizeof(MetisTlvName));
    _incrementRefCount(copy);

    copy->segmentArrayLength = (copy->segmentArrayLength < segmentCount) ? copy->segmentArrayLength : segmentCount;

    // for equality to work, we need to shorten the MemoryLength to the amount
    // actually used by the number of segments.
    size_t startOfLastSegment = copy->segmentArray[ copy->segmentArrayLength - 1 ].offset;
    size_t lengthOfLastSegment = copy->segmentArray[ copy->segmentArrayLength - 1 ].length;

    copy->memoryLength = startOfLastSegment + lengthOfLastSegment;

    return copy;
}

uint32_t
metisTlvName_HashCode(const MetisTlvName *name)
{
    if ((name == NULL) || (name->segmentArrayLength == 0)) {
        return 0;
    }

    size_t lastSegment = name->segmentArrayLength - 1;

    if (lastSegment >= *name->segmentCumulativeHashArrayLengthPtr) {
        // we have not yet computed this, so lets do it now!
        // Note that we go up to and including lastSegment in the for loop.  lastSegment is not a "length", it is
        // the actual array index.
        for (size_t i = (*name->segmentCumulativeHashArrayLengthPtr); i <= lastSegment; i++) {
            // the -1 is ok in the 3rd argument, because segmentCumulativeHashArrayLength always has length at least 1
            // if there are any name components to hash.
            name->segmentCumulativeHashArray[i] = parcHash32_Data_Cumulative(&name->memory[ name->segmentArray[i].offset ],
                                                                             name->segmentArray[i].length,
                                                                             name->segmentCumulativeHashArray[i - 1]);
        }
        *name->segmentCumulativeHashArrayLengthPtr = lastSegment + 1;
    }

    return name->segmentCumulativeHashArray[lastSegment];
}

bool
metisTlvName_Equals(const MetisTlvName *a, const MetisTlvName *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");

    if (a->memoryLength == b->memoryLength) {
        return (memcmp(a->memory, b->memory, a->memoryLength) == 0);
    }
    return false;
}

int
metisTlvName_Compare(const MetisTlvName *a, const MetisTlvName *b)
{
    if (a == NULL && b == NULL) {
        return 0;
    }
    if (a == NULL) {
        return -1;
    }
    if (b == NULL) {
        return +1;
    }

    if (a->memoryLength < b->memoryLength) {
        return -1;
    }

    if (a->memoryLength > b->memoryLength) {
        return +1;
    }

    return memcmp(a->memory, b->memory, a->memoryLength);
}

bool
metisTlvName_StartsWith(const MetisTlvName *name, const MetisTlvName *prefix)
{
    assertNotNull(name, "Parameter name must be non-null");
    assertNotNull(prefix, "Parameter prefix must be non-null");

    if (prefix->memoryLength <= name->memoryLength) {
        return (memcmp(prefix->memory, name->memory, prefix->memoryLength) == 0);
    }

    return false;
}

size_t
metisTlvName_SegmentCount(const MetisTlvName *name)
{
    assertNotNull(name, "Parameter name must be non-null");
    return name->segmentArrayLength;
}

CCNxName *
metisTlvName_ToCCNxName(const MetisTlvName *name)
{
    return metisTlvNameCodec_Decode(name->memory, 0, name->memoryLength);
}

