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

#include <stdio.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_LinkedList.h>

#include "parc_SimpleBufferPool.h"

struct PARCSimpleBufferPool {
    size_t bufferSize;
    size_t limit;
    PARCLinkedList *freeList;
    PARCObjectDescriptor *descriptor;
};

static bool
_parcSimpleBufferPool_Destructor(PARCSimpleBufferPool **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCSimpleBufferPool pointer.");

    PARCSimpleBufferPool *pool = *instancePtr;

    parcLinkedList_Apply(pool->freeList, (void (*))parcObject_SetDescriptor, (const void *) &PARCBuffer_Descriptor);

    parcLinkedList_Release(&pool->freeList);

    return true;
}

static bool
_parcSimpleBufferPool_BufferDestructor(PARCBuffer **bufferPtr)
{
    PARCBuffer *buffer = *bufferPtr;
    *bufferPtr = 0;

    PARCSimpleBufferPool *bufferPool = parcObjectDescriptor_GetTypeState(parcObject_GetDescriptor(buffer));

    if (bufferPool->limit > parcLinkedList_Size(bufferPool->freeList)) {
        parcLinkedList_Append(bufferPool->freeList, buffer);
    } else {
        parcBuffer_Acquire(buffer);
        parcObject_SetDescriptor(buffer, &parcObject_DescriptorName(PARCBuffer));
        parcBuffer_Release(&buffer);
    }

    return false;
}

parcObject_ImplementAcquire(parcSimpleBufferPool, PARCSimpleBufferPool);

parcObject_ImplementRelease(parcSimpleBufferPool, PARCSimpleBufferPool);

parcObject_Override(PARCSimpleBufferPool, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcSimpleBufferPool_Destructor);

PARCSimpleBufferPool *
parcSimpleBufferPool_Create(size_t limit, size_t bufferSize)
{
    PARCSimpleBufferPool *result = parcObject_CreateInstance(PARCSimpleBufferPool);

    if (result != NULL) {
        result->limit = limit;
        result->bufferSize = bufferSize;
        result->freeList = parcLinkedList_Create();

        char *string;
        asprintf(&string, "PARCSimpleBufferPool=%zu", bufferSize);
        result->descriptor = parcObjectDescriptor_CreateExtension(&parcObject_DescriptorName(PARCBuffer), string);
        free(string);
        result->descriptor->destructor = (PARCObjectDestructor *) _parcSimpleBufferPool_BufferDestructor;
        result->descriptor->typeState = (PARCObjectTypeState *) result;
    }

    return result;
}

PARCBuffer *
parcSimpleBufferPool_GetInstance(PARCSimpleBufferPool *bufferPool)
{
    PARCBuffer *result;

    if (parcLinkedList_Size(bufferPool->freeList) > 0) {
        result = parcLinkedList_RemoveFirst(bufferPool->freeList);
    } else {
        result = parcBuffer_Allocate(bufferPool->bufferSize);
        parcObject_SetDescriptor(result, bufferPool->descriptor);
    }

    return result;
}
