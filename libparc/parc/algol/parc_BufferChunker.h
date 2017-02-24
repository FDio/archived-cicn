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
 * @file parc_BufferChunker.h
 * @ingroup ContentObject
 * @brief A BufferChunker is a chunker that segments a PARCBuffer.
 *
 */

#ifndef libparc_parc_BufferChunker_h
#define libparc_parc_BufferChunker_h

#include <parc/algol/parc_Chunker.h>

struct parc_buffer_chunker;
/**
 * @typedef PARCChunker
 * @brief The PARC Chunker
 */
typedef struct parc_buffer_chunker PARCBufferChunker;

/**
 * The mapping of a `PARCBufferChunker` to the generic `PARCChunker`.
 */
extern PARCChunkerInterface *PARCBufferChunkerAsChunker;

/**
 * Create a new chunker to segment data contained in a `PARCBuffer.`
 *
 * @param [in] data A `PARCBuffer` which contains the data.
 * @param [in] chunkSize The size per chunk.
 *
 * @retval PARCChunker A newly allocated `PARCChunker`
 * @retval NULL An error occurred.
 *
 * Example
 * @code
 * {
 *     PARCBuffer *dataToChunk = ...
 *     PARCChunker *chunker = ccnxChunker_CreateFromBuffer(dataToChunk, 32);
 * }
 */
PARCBufferChunker *parcBufferChunker_Create(PARCBuffer *data, size_t chunkSize);

/**
 * Increase the number of references to a `PARCChunker` instance.
 *
 * Note that new `PARCChunker` is not created,
 * only that the given `PARCChunker` reference count is incremented.
 * Discard the reference by invoking {@link ccnxChunker_Release}.
 *
 * @param [in] chunker A pointer to the original `PARCChunker`.
 * @return The value of the input parameter @p chunker.
 *
 * Example:
 * @code
 * {
 *     PARCBufferChunker *original = parcBufferChunker_Create(...);
 *
 *     PARCBufferChunker *reference = parcBufferChunker_Acquire(original);
 *
 *     parcBufferChunker_Release(&original);
 *     parcBufferChunker_Release(&reference);
 * }
 * @endcode
 *
 * @see parcBufferChunker_Release
 */
PARCBufferChunker *parcBufferChunker_Acquire(const PARCBufferChunker *chunker);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] chunkerP A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCBufferChunker *chunker = parcBufferChunker_Acquire(instance);
 *
 *     parcBufferChunker_Release(&chunker);
 * }
 * @endcode
 */
void parcBufferChunker_Release(PARCBufferChunker **chunkerP);

/**
 * Determine if two `PARCBufferChunker` instances are equal.
 *
 * The following equivalence relations on non-null `PARCBufferChunker` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `parcBufferChunker_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `parcBufferChunker_Equals(x, y)` must return true if and only if
 *        `parcBufferChunker_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcBufferChunker_Equals(x, y)` returns true and
 *        `parcBufferChunker_Equals(y, z)` returns true,
 *        then  `parcBufferChunker_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `parcBufferChunker_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `parcBufferChunker_Equals(x, NULL)` must
 *      return false.
 *
 * @param chunkerA A pointer to a `PARCBufferChunker` instance.
 * @param chunkerB A pointer to a `PARCBufferChunker` instance.
 * @return true if the two `PARCBufferChunker` instances are equal.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *dataToChunk = ...
 *     PARCBufferChunker *chunkerA = parcBufferChunker_Create(dataToChunk, 32);
 *     PARCBufferChunker *chunkerB = parcBufferChunker_Create(dataToChunk, 32);
 *
 *     bool equals = parcBufferChunker_Equals(chunkerA, chunkerB);
 * }
 * @endcode
 */
bool parcBufferChunker_Equals(const PARCBufferChunker *chunkerA, const PARCBufferChunker *chunkerB);

/**
 * Return an iterator to traverse the chunks of the underlying data in sequential order.
 *
 * This function can only be called once per chunker instance since the iterator
 * will mutate internal state of the chunker.
 *
 * @param [in] chunker A `PARCBufferChunker` instance.
 *
 * @return a `PARCIterator` that traverses the chunks of the underlying data.
 *
 * Example
 * @code
 * {
 *     PARCBuffer *dataToChunk = ...
 *     PARCBufferChunker *chunker = parcBufferChunker_Create(dataToChunk, 32);
 *
 *     PARCIterator *itr = parcBufferChunker_ForwardIterator(chunker);
 *
 *     // use the iterator to traverse the chunker
 * }
 * @endcode
 */
PARCIterator *parcBufferChunker_ForwardIterator(const PARCBufferChunker *chunker);

/**
 * Return an iterator to traverse the chunks of the underlying data in sequential order.
 *
 * This function can only be called once per chunker instance since the iterator
 * will mutate internal state of the chunker.
 *
 * @param [in] chunker A `PARCBufferChunker` instance.
 *
 * @return a `PARCIterator` that traverses the chunks of the underlying data.
 *
 * Example
 * @code
 * {
 *     PARCBuffer *dataToChunk = ...
 *     PARCBufferChunker *chunker = parcBufferChunker_Create(dataToChunk, 32);
 *
 *     PARCIterator *itr = parcBufferChunker_ReverseIterator(chunker);
 *
 *     // use the iterator to traverse the chunker
 * }
 * @endcode
 */
PARCIterator *parcBufferChunker_ReverseIterator(const PARCBufferChunker *chunker);

/**
 * Get the chunk size of a `PARCBufferChunker.`
 *
 * @param [in] chunker A `PARCBufferChunker` instance.
 *
 * @return the chunk size
 *
 * Example
 * @code
 * {
 *     PARCBuffer *dataToChunk = ...
 *     PARCBufferChunker *chunker = ...
 *
 *     size_t chunkSize = parcBufferChunker_GetChunkSize(chunker);
 * }
 * @endcode
 */
size_t parcBufferChunker_GetChunkSize(const PARCBufferChunker *chunker);
#endif // libparc_parc_BufferChunker_h
