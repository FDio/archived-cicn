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
 * A thread-safe fixed size ring buffer.
 *
 * The multiple producer, multiple consumer version uses a pthread mutex around a NxM ring buffer.
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
#include <parc/assert/parc_Assert.h>

#include <parc/concurrent/parc_RingBuffer_1x1.h>
#include <parc/concurrent/parc_RingBuffer_NxM.h>

struct parc_ringbuffer_NxM {
    PARCRingBuffer1x1 *onebyone;

    // This protectes the overall data structure for Acquire and Release
    pthread_mutex_t allocation_mutex;

    pthread_mutex_t writer_mutex;
    pthread_mutex_t reader_mutex;

    RingBufferEntryDestroyer *destroyer;
};

/*
 * Attemps a lock and returns false if we cannot get it.
 *
 * @endcode
 */
static bool
_lock(pthread_mutex_t *mutex)
{
    int failure = pthread_mutex_lock(mutex);
    parcAssertFalse(failure, "Error locking mutex: (%d) %s\n", errno, strerror(errno));
    return true;
}

static bool
_unlock(pthread_mutex_t *mutex)
{
    int failure = pthread_mutex_unlock(mutex);
    parcAssertFalse(failure, "Error unlocking mutex: (%d) %s\n", errno, strerror(errno));
    return true;
}

static void
_destroy(PARCRingBufferNxM **ringptr)
{
    PARCRingBufferNxM *ring = *ringptr;

    if (ring->destroyer) {
        void *ptr = NULL;
        while (parcRingBufferNxM_Get(ring, &ptr)) {
            ring->destroyer(&ptr);
        }
    }
    parcRingBuffer1x1_Release(&ring->onebyone);
}


parcObject_ExtendPARCObject(PARCRingBufferNxM, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

static PARCRingBufferNxM *
_create(uint32_t elements, RingBufferEntryDestroyer *destroyer)
{
    PARCRingBufferNxM *ring = parcObject_CreateInstance(PARCRingBufferNxM);
    parcAssertNotNull(ring, "parcObject_Create returned NULL");

    ring->onebyone = parcRingBuffer1x1_Create(elements, destroyer);
    ring->destroyer = destroyer;
    pthread_mutex_init(&ring->allocation_mutex, NULL);
    pthread_mutex_init(&ring->writer_mutex, NULL);
    pthread_mutex_init(&ring->reader_mutex, NULL);
    return ring;
}

PARCRingBufferNxM *
parcRingBufferNxM_Create(uint32_t elements, RingBufferEntryDestroyer *destroyer)
{
    return _create(elements, destroyer);
}

PARCRingBufferNxM *
parcRingBufferNxM_Acquire(PARCRingBufferNxM *ring)
{
    PARCRingBufferNxM *acquired;

    _lock(&ring->allocation_mutex);
    acquired = parcObject_Acquire(ring);
    _unlock(&ring->allocation_mutex);

    return acquired;
}

void
parcRingBufferNxM_Release(PARCRingBufferNxM **ringPtr)
{
    PARCRingBufferNxM *ring = *ringPtr;
    _lock(&ring->allocation_mutex);
    parcObject_Release((void **) ringPtr);
    _unlock(&ring->allocation_mutex);
}

/**
 * Put is protected by the writer mutex.  This means that the tail mutex could
 * actually increase while this is happening. That's ok.  Increasing the tail
 * just means there is _more_ room in the ring.  We only modify writer_head.
 */
bool
parcRingBufferNxM_Put(PARCRingBufferNxM *ring, void *data)
{
    // **** LOCK
    _lock(&ring->writer_mutex);
    bool success = parcRingBuffer1x1_Put(ring->onebyone, data);
    // **** UNLOCK
    _unlock(&ring->writer_mutex);
    return success;
}

bool
parcRingBufferNxM_Get(PARCRingBufferNxM *ring, void **outputDataPtr)
{
    // **** LOCK
    _lock(&ring->reader_mutex);
    bool success = parcRingBuffer1x1_Get(ring->onebyone, outputDataPtr);
    // **** UNLOCK
    _unlock(&ring->reader_mutex);
    return success;
}

uint32_t
parcRingBufferNxM_Remaining(PARCRingBufferNxM *ring)
{
    _lock(&ring->writer_mutex);
    _lock(&ring->reader_mutex);

    uint32_t remaining = parcRingBuffer1x1_Remaining(ring->onebyone);

    _unlock(&ring->reader_mutex);
    _unlock(&ring->writer_mutex);

    return remaining;
}
