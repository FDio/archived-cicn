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
 * @file parc_RingBuffer.h
 * @ingroup threading
 * @brief A thread-safe ring buffer.
 *
 * A fixed size, thread safe ring buffer. It can have multiple producers and multiple
 * consumers. All exclusion is done inside the ring buffer.
 *
 * This is a non-blocking data structure.
 *
 * If the user knows there is only one producer and one consumer, you can create the ring buffer with
 * `parcRingBuffer_CreateSingleProducerSingleConsumer`. Such a ring buffer can have at most 2 references.
 *
 */

#ifndef libparc_parc_RingBuffer_h
#define libparc_parc_RingBuffer_h

#include <stdbool.h>
#include <stdint.h>

struct parc_ringbuffer;
typedef struct parc_ringbuffer PARCRingBuffer;

/**< Will be called for each data item on destroy */
typedef void (RingBufferEntryDestroyer)(void **entryPtr);

struct parc_ringbuffer_impl;
typedef struct parc_ringbuffer_interface PARCRingBufferInterface;

struct parc_ringbuffer_interface {
    void *instance;
    void * (*acquire)(PARCRingBufferInterface *ring);
    void (*release)(PARCRingBufferInterface **ring);
    bool (*put)(PARCRingBufferInterface *ring, void *data);
    bool (*get)(PARCRingBufferInterface *ring, void *outputDataPtr);
    bool (*remaining)(PARCRingBufferInterface *ring);
};

/**
 * Creates a ring buffer of the given size, which must be a power of 2.
 *
 * The ring buffer can store up to (elements-1) items in the buffer.  The buffer can
 * be shared between multiple producers and consumers.  Each of them should be
 * given out from a call to {@link parcRingBuffer_Acquire} to create reference counted
 * copies.
 *
 * The reference count is "1" on return.
 *
 * @param [in] interface A pointer to the underlying instance and the interface functions for acquire,
 * release,put,get,and remaining.
 *
 * @return non-null An pointer to a new allocated `PARCRingBuffer`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCRingBuffer *parcRingBuffer_Create(PARCRingBufferInterface *interface);

/**
 * Acquire a new reference to an instance of `PARCRingBuffer`.
 *
 * A RING WITHOUT LOCKS CAN ONLY HAVE 2 REFERENCES.
 * The reference count to the instance is incremented.
 *
 * @param [in] ring The instance of `PARCRingBuffer` to which to refer.
 *
 * @return The same value as the input parameter @p ring
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */

PARCRingBuffer *parcRingBuffer_Acquire(const PARCRingBuffer *ring);

/**
 * Releases a reference.  The buffer will be destroyed after the last release.
 *
 * If the destroyer was specified on create, it will be called on each entry in the buffer
 * when the buffer is destroyed.
 *
 * @param [in,out] ringPtr The pointer to the pointer of the `PARCRingBuffer` to be destroyed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcRingBuffer_Release(PARCRingBuffer **ringPtr);

/**
 * Non-blocking attempt to put item on ring.  May return false if ring is full.
 *
 * @param [in,out] ring The instance of `PARCRingBuffer` to modify'
 * @param [in] data A pointer to teh data to put on the ring.
 *
 * @return true Data was put on the queue
 * @return false Would have blocked, the queue was full
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcRingBuffer_Put(PARCRingBuffer *ring, void *data);

/**
 * Gets the next item off the ring, or returns false if would have blocked.
 *
 * Non-blocking, gets an item off the ring, or returns false if would block
 *
 * @param [in] ring The pointer to the `PARCRingBuffer`
 * @param [out] outputDataPtr The output pointer
 *
 * @return true Data returned in the output argument
 * @return false Ring is empty, no data returned.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcRingBuffer_Get(PARCRingBuffer *ring, void **outputDataPtr);

/**
 * Returns the remaining capacity of the ring
 *
 * Returns the remaining capacity of the ring.  This does not guarantee the next
 * Put will not block, as other producers might consumer the space between calls.
 *
 * @param [in] ring The pointer to the `PARCRingBuffer`
 *
 * @return  The uint32_t remaining capacity of the ring.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint32_t parcRingBuffer_Remaining(PARCRingBuffer *ring);
#endif // libparc_parc_RingBuffer_h
