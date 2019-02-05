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
 * @file parc_RingBuffer_NxM.h
 * @ingroup threading
 * @brief A multiple producer, multiple consumer ring buffer
 *
 * This is useful for synchronizing one or more producers with one or more consumers.
 * The implementation may use locks.
 *
 * Complies with the PARCRingBuffer generic facade.
 *
 */

#ifndef libparc_parc_RingBuffer_NxM_h
#define libparc_parc_RingBuffer_NxM_h

#include <stdbool.h>
#include <stdint.h>
#include <parc/concurrent/parc_RingBuffer_1x1.h>

struct parc_ringbuffer_NxM;
/**
 * @typedef PARCRingBufferNxM
 */
typedef struct parc_ringbuffer_NxM PARCRingBufferNxM;

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
PARCRingBufferNxM *parcRingBufferNxM_Create(uint32_t elements, RingBufferEntryDestroyer *destroyer);

/**
 * A reference counted copy of the buffer.
 *
 * A RING WITHOUT LOCKS CAN ONLY HAVE 2 REFERENCES.
 *
 * @param [in] ring A pointer to the `PARCRingBufferNxM` to be acquired.
 *
 * @return non-null A reference counted copy of the ring buffer
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCRingBufferNxM *parcRingBufferNxM_Acquire(PARCRingBufferNxM *ring);

/**
 * Releases a reference.  The buffer will be destroyed after the last release.
 *
 * If the destroyer was specified on create, it will be called on each entry in the buffer
 * when the buffer is destroyed.
 *
 * @param [in,out] ringPtr A pointer to the pointer to the `PARCRingBufferNxM` to be released.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcRingBufferNxM_Release(PARCRingBufferNxM **ringPtr);

/**
 * Non-blocking attempt to put item on ring.  May return false if ring is full.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in,out] ring A pointer to the `PARCRingBufferNxM` on which to put @p data.
 * @param [in] data A pointer to data to put on @p ring.
 *
 * @return `true` Data was put on the queue
 * @return `false` Would have blocked, the queue was full
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcRingBufferNxM_Put(PARCRingBufferNxM *ring, void *data);

/**
 * Gets the next item off the ring, or returns false if would have blocked.
 *
 * Non-blocking, gets an item off the ring, or returns false if would block
 *
 * @param [in] ring The ring buffer
 * @param [out] outputDataPtr The output pointer
 *
 * @return `true` Data returned in the output argument
 * @return `false` Ring is empty, no data returned.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcRingBufferNxM_Get(PARCRingBufferNxM *ring, void **outputDataPtr);

/**
 * Returns the remaining capacity of the ring
 *
 * Returns the remaining capacity of the ring.  This does not guarantee the next
 * Put will not block, as other producers might consumer the space between calls.
 *
 * @param [in] ring The ring buffer
 *
 * @return the remaining capacity of @p ring.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint32_t parcRingBufferNxM_Remaining(PARCRingBufferNxM *ring);
#endif // libparc_parc_RingBuffer_NxM_h
