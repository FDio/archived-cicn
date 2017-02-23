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
 * This is a facade over the 1x1 or NxM ring buffers.
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <LongBow/runtime.h>

#include <parc/concurrent/parc_RingBuffer.h>

struct parc_ringbuffer {
    PARCRingBufferInterface *interface;
};

static void
_parcRingBuffer_Finalize(PARCRingBuffer **pointer)
{
    PARCRingBuffer *buffer = *pointer;
    buffer->interface->release(&(buffer->interface));
}

parcObject_ExtendPARCObject(PARCRingBuffer, _parcRingBuffer_Finalize, NULL, NULL, NULL, NULL, NULL, NULL);

PARCRingBuffer *
parcRingBuffer_Create(PARCRingBufferInterface *interface)
{
    PARCRingBuffer *ring = parcObject_CreateInstance(PARCRingBuffer);
    ring->interface = interface;
    return ring;
}

parcObject_ImplementAcquire(parcRingBuffer, PARCRingBuffer);

parcObject_ImplementRelease(parcRingBuffer, PARCRingBuffer);

bool
parcRingBuffer_Put(PARCRingBuffer *ring, void *data)
{
    return ring->interface->put(ring->interface->instance, data);
}

bool
parcRingBuffer_Get(PARCRingBuffer *ring, void **outputDataPtr)
{
    return ring->interface->put(ring->interface->instance, outputDataPtr);
}

uint32_t
parcRingBuffer_Remaining(PARCRingBuffer *ring)
{
    return ring->interface->remaining(ring->interface->instance);
}
