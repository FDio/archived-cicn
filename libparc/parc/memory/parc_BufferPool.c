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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_LinkedList.h>

#include "parc_BufferPool.h"

struct PARCBufferPool {
    size_t bufferSize;
    size_t limit;
    size_t largestPoolSize;
    size_t totalInstances;
    size_t cacheHits;
    PARCLinkedList *freeList;
    PARCObjectDescriptor *descriptor;
    const PARCObjectDescriptor *originalDescriptor;
};

static bool
_parcBufferPool_Destructor(PARCBufferPool **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCBufferPool pointer.");

    PARCBufferPool *pool = *instancePtr;

    parcLinkedList_Apply(pool->freeList, (void (*))parcObject_SetDescriptor, (const void *) &PARCBuffer_Descriptor);

    parcLinkedList_Release(&pool->freeList);
    parcObjectDescriptor_Destroy(&pool->descriptor);

    return true;
}

static bool
_parcBufferPool_ObjectDestructor(PARCBuffer **bufferPtr)
{
    PARCBuffer *buffer = *bufferPtr;

    PARCBufferPool *bufferPool = parcObjectDescriptor_GetTypeState(parcObject_GetDescriptor(buffer));

    parcObject_Synchronize(bufferPool->freeList)
    {
        size_t freeListSize = parcLinkedList_Size(bufferPool->freeList);

        if (bufferPool->limit > freeListSize) {
            parcLinkedList_Append(bufferPool->freeList, buffer);
            freeListSize++;
            if (bufferPool->largestPoolSize < freeListSize) {
                bufferPool->largestPoolSize = freeListSize;
            }
        } else {
            parcBuffer_Acquire(buffer);
            parcObject_SetDescriptor(buffer, &PARCBuffer_Descriptor);
            parcBuffer_Release(&buffer);
        }
    }

    *bufferPtr = 0;
    return false;
}

parcObject_ImplementAcquire(parcBufferPool, PARCBufferPool);

parcObject_ImplementRelease(parcBufferPool, PARCBufferPool);

parcObject_Override(PARCBufferPool, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcBufferPool_Destructor,
                    .isLockable = true);


void
parcBufferPool_AssertValid(const PARCBufferPool *instance)
{
    assertTrue(parcBufferPool_IsValid(instance),
               "PARCBufferPool is not valid.");
}

PARCBufferPool *
parcBufferPool_CreateExtending(const PARCObjectDescriptor *originalDescriptor, size_t limit, size_t bufferSize)
{
    PARCBufferPool *result = parcObject_CreateInstance(PARCBufferPool);

    if (result != NULL) {
        result->limit = limit;
        result->totalInstances = 0;
        result->cacheHits = 0;
        result->largestPoolSize = 0;
        result->bufferSize = bufferSize;
        result->freeList = parcLinkedList_Create();

        result->originalDescriptor = originalDescriptor;

        char *string = parcMemory_Format("PARCBufferPool=%zu", bufferSize);
        result->descriptor = parcObjectDescriptor_CreateExtension(result->originalDescriptor, string);
        result->descriptor->destructor = (PARCObjectDestructor *) _parcBufferPool_ObjectDestructor;
        result->descriptor->typeState = (PARCObjectTypeState *) result;
        parcMemory_Deallocate(&string);
    }

    return result;
}

PARCBufferPool *
parcBufferPool_Create(size_t limit, size_t bufferSize)
{
    PARCBufferPool *result = parcBufferPool_CreateExtending(&parcObject_DescriptorName(PARCBuffer), limit, bufferSize);

    return result;
}

void
parcBufferPool_Display(const PARCBufferPool *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCBufferPool@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcBufferPool_IsValid(const PARCBufferPool *bufferPool)
{
    bool result = false;

    if (bufferPool != NULL) {
        result = parcLinkedList_IsValid(bufferPool->freeList);
    }

    return result;
}

PARCBuffer *
parcBufferPool_GetInstance(PARCBufferPool *bufferPool)
{
    PARCBuffer *result = NULL;

    parcObject_Synchronize(bufferPool->freeList)
    {
        if (parcLinkedList_Size(bufferPool->freeList) > 0) {
            result = parcLinkedList_RemoveFirst(bufferPool->freeList);
            bufferPool->cacheHits++;
        } else {
            result = parcBuffer_Allocate(bufferPool->bufferSize);
            parcObject_SetDescriptor(result, bufferPool->descriptor);
        }
        bufferPool->totalInstances++;
    }

    return result;
}

size_t
parcBufferPool_Drain(PARCBufferPool *bufferPool)
{
    size_t result = 0;

    parcObject_Synchronize(bufferPool->freeList)
    {
        size_t freeListSize = parcLinkedList_Size(bufferPool->freeList);
        if (freeListSize > bufferPool->limit) {
            result = freeListSize - bufferPool->limit;
            for (size_t i = bufferPool->limit; i < freeListSize; i++) {
                PARCObject *object = parcLinkedList_RemoveLast(bufferPool->freeList);
                parcObject_SetDescriptor(object, &PARCBuffer_Descriptor);
                parcObject_Release(&object);
            }
        }
    }

    return result;
}

size_t
parcBufferPool_SetLimit(PARCBufferPool *bufferPool, size_t limit)
{
    size_t oldLimit = bufferPool->limit;

    if (limit < bufferPool->limit) {
        bufferPool->largestPoolSize = bufferPool->limit;
    }

    bufferPool->limit = limit;

    return oldLimit;
}

size_t
parcBufferPool_GetLimit(const PARCBufferPool *bufferPool)
{
    return bufferPool->limit;
}

size_t
parcBufferPool_GetCurrentPoolSize(const PARCBufferPool *bufferPool)
{
    size_t result = parcLinkedList_Size(bufferPool->freeList);

    return result;
}

size_t
parcBufferPool_GetLargestPoolSize(const PARCBufferPool *bufferPool)
{
    return bufferPool->largestPoolSize;
}

size_t
parcBufferPool_GetTotalInstances(const PARCBufferPool *bufferPool)
{
    return bufferPool->totalInstances;
}

size_t
parcBufferPool_GetCacheHits(const PARCBufferPool *bufferPool)
{
    return bufferPool->cacheHits;
}
