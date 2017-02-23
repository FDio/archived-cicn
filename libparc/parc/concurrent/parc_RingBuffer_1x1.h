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
 * @file parc_RingBuffer_1x1.h
 * @ingroup threading
 * @brief A single producer, single consumer ring buffer
 *
 * This is useful for synchronizing two (and exactly two) threads in one direction.  The
 * implementation will use a lock-free algorithm.
 *
 * Complies with the PARCRingBuffer generic facade.
 *
 */

#ifndef libparc_parc_RingBuffer_1x1_h
#define libparc_parc_RingBuffer_1x1_h

#include <stdbool.h>
#include <stdint.h>

struct parc_ringbuffer_1x1;
typedef struct parc_ringbuffer_1x1 PARCRingBuffer1x1;

/**< Will be called for each data item on destroy */
typedef void (RingBufferEntryDestroyer)(void **entryPtr);

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
 * @param [in] elements A power of 2, indicating the maximum size of the buffer.
 * @param [in] destroyer Will be called for each ring entry when when the ring is destroyed.  May be null.
 *
 * @return non-null An allocated ring buffer.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCRingBuffer1x1 *parcRingBuffer1x1_Create(uint32_t elements, RingBufferEntryDestroyer *destroyer);

/**
 * A reference counted copy of the buffer.
 *
 * A RING WITHOUT LOCKS CAN ONLY HAVE 2 REFERENCES.
 *
 * @param [in] ring The instance of `PARCRingBuffer1x1` to acquire.
 *
 * @return non-null A reference counted copy of the ring buffer.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCRingBuffer1x1 *parcRingBuffer1x1_Acquire(const PARCRingBuffer1x1 *ring);

/**
 * Releases a reference.  The buffer will be destroyed after the last release.
 *
 * If the destroyer was specified on create, it will be called on each entry in the buffer
 * when the buffer is destroyed.
 *
 * @param [in,out] ringPtr The pointer to the pointer of the `PARCRingBuffer1x1` to be released.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcRingBuffer1x1_Release(PARCRingBuffer1x1 **ringPtr);

/**
 * Non-blocking attempt to put item on ring.  May return `false` if ring is full.
 *
 * @param [in,out] ring The instance of `PARCRingBuffer1x1` on which to put the @p data.
 * @param [in] data The data to put on the @p ring.
 *
 * @return `true` Data was put on the queue
 * @return `false` Would have blocked, the queue was full
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcRingBuffer1x1_Put(PARCRingBuffer1x1 *ring, void *data);

/**
 * Gets the next item off the ring, or returns false if would have blocked.
 *
 * Non-blocking, gets an item off the ring, or returns false if would block
 *
 * @param [in] ring The ring buffer
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
bool parcRingBuffer1x1_Get(PARCRingBuffer1x1 *ring, void **outputDataPtr);

/**
 * Returns the remaining capacity of the ring
 *
 * Returns the remaining capacity of the ring.  This does not guarantee the next
 * Put will not block, as other producers might consumer the space between calls.
 *
 * @param [in] ring The instance of `PARCRingBuffer1x1` .
 *
 * @return The remaining capacity on the ring.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint32_t parcRingBuffer1x1_Remaining(PARCRingBuffer1x1 *ring);
#endif // libparc_parc_RingBuffer_1x1_h
