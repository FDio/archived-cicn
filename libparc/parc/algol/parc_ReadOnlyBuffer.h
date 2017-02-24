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
 * @file parc_ReadOnlyBuffer.h
 * @ingroup memory
 * @brief An indexable, linear buffer of read-only bytes.
 *
 * A `PARCReadOnlyBuffer` is a {@link PARCBuffer} that cannot be modified,
 * but retains a position, limit and capacity.
 *
 */
#ifndef libparc_parc_ReadOnlyBuffer_h
#define libparc_parc_ReadOnlyBuffer_h

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_ByteArray.h>

struct parc_readonly_buffer;
typedef struct parc_readonly_buffer PARCReadOnlyBuffer;

/**
 * Create a new instance of `PARCBuPARCReadOnlyBuffer` referencing the content of the given {@link PARCBuffer}.
 *
 * A reference to the content of the given `PARCBuffer` is acquired.
 *
 * The new buffer's position, limit, capacity and mark will be the same as the given `PARCBuffer`.
 *
 * If capacity is zero, the buffer contains no underlying byte array.
 *
 * @param [in] buffer A pointed to a valid `PARCBuffer` instance.
 *
 * @return A `PARCReadOnlyBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     ...
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Create(PARCBuffer *buffer);

/**
 * Create a new instance of `PARCReadOnlyBuffer` using program supplied memory.
 *
 * The new buffer will be backed by the given byte array.
 * Modifications to the array will be visible through the buffer.
 *
 * The new buffer's capacity will be @p arrayLength,
 * its position will be @p position,
 * its limit will be @p limit,
 * and its mark will be undefined.
 *
 * Its backing array will be the given array,
 * and its array offset will be zero.
 *
 * @param [in] array A pointer to a memory array.
 * @param [in] arrayLength The length, in `uint8_t` units, of the memory array.
 * @param [in] position The initial value for the buffer's position.
 * @param [in] limit The initial value for the buffer's limit.
 *
 * @return A `PARCReadOnlyBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(array, 10, 0, 10);
 *
 *     ...
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Wrap(uint8_t *array, size_t arrayLength, size_t position, size_t limit);

/**
 * Increase the number of references to a `PARCReadOnlyBuffer`.
 *
 * Note that new `PARCReadOnlyBuffer` is not created,
 * only that the given `PARCReadOnlyBuffer` reference count is incremented.
 * Discard the reference by invoking `parcBuffer_Release`.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` instance.
 *
 * @return The input `PARCReadOnlyBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     PARCReadOnlyBuffer *handle = parcReadOnlyBuffer_Acquire(roBuffer);
 *
 *     ...
 *
 *     parcReadOnlyBuffer_Release(&handle);
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Acquire(const PARCReadOnlyBuffer *buffer);

/**
 * Release a `PARCReadOnlyBuffer` reference.
 *
 * Only the last invocation where the reference count is decremented to zero,
 * will actually destroy the `PARCReadOnlyBuffer`.
 *
 * @param [in,out] bufferPtr Pointer to the `PARCReadOnlyBuffer` instance to be released.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     ...
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
void parcReadOnlyBuffer_Release(PARCReadOnlyBuffer **bufferPtr);

/**
 * Returns this buffer's capacity.
 *
 * @param [in] buffer A pointer to a `PARCReadOnlyBuffer` instance.
 *
 * @return The given buffer's capacity.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(array, 10, 0, 10);
 *
 *     size_t capacity = parcReadOnlyBuffer_Capacity(roBuffer);
 *     // capacity will be 10
 *
 *     ...
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 * }
 * @endcode
 */
size_t parcReadOnlyBuffer_Capacity(const PARCReadOnlyBuffer *buffer);

/**
 * Clear the given buffer: The position is set to zero,
 * the limit is set to the capacity,
 * and the mark is invalidated.
 *
 * The mark is made invalid and any subsequent operation on the resulting
 * `PARCBuffer` that requires the mark will abort until the mark
 * is set again via `parcReadOnlyBuffer_Mark`.
 *
 * @param [in,out] buffer A pointer to the `PARCReadOnlyBuffer` instance to be modified.
 *
 * @return The value of @p buffer.
 *
 * Example:
 * @code
 * {
 *     parcReadOnlyBuffer_Clear(buffer);
 * }
 * @endcode
 *
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Clear(PARCReadOnlyBuffer *buffer);

/**
 * Determine if two `PARCReadOnlyBuffer` instances are equal.
 *
 * The following equivalence relations on non-null `PARCReadOnlyBuffer` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcReadOnlyBuffer_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcReadOnlyBuffer_Equals(x, y)` must return true if and only if
 *        `parcReadOnlyBuffer_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcReadOnlyBuffer_Equals(x, y)` returns true and
 *        `parcReadOnlyBuffer_Equals(y, z)` returns true,
 *        then `parcReadOnlyBuffer_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcReadOnlyBuffer_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcReadOnlyBuffer_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] x A pointer to a `PARCReadOnlyBuffer` instance.
 * @param [in] y A pointer to a `PARCReadOnlyBuffer` instance.
 *
 * @return true `PARCReadOnlyBuffer` x and y are equal.
 * @return false `PARCReadOnlyBuffer` x and y are not equal.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *     PARCReadOnlyBuffer *roBuffer1 = parcReadOnlyBuffer_Create(array, 10, 0, 10);
 *     PARCReadOnlyBuffer *roBuffer2 = parcReadOnlyBuffer_Create(array, 10, 0, 10);
 *
 *     if (parcReadOnlyBuffer_Equals(roBuffer1, roBuffer2)) {
 *         printf("ROBuffers are equal\n");
 *     } else {
 *         printf("ROBuffers are NOT equal\n");
 *     }
 *
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 * }
 * @endcode
 */
bool parcReadOnlyBuffer_Equals(const PARCReadOnlyBuffer *x, const PARCReadOnlyBuffer *y);

/**
 * Return a pointer to the {@link PARCByteArray} that backs this buffer.
 *
 * If this `PARCReadOnlyBuffer` has a capacity of zero,
 * there is no array of bytes and this function returns NULL.
 *
 * Modifications to the `PARCByteArray` will cause the returned array's content to be modified, and vice versa.
 *
 * The caller must obtain its own reference to the `PARCByteArray` if it intends to store it elsewhere.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The pointer to the `PARCByteArray` for the given `PARCReadOnlyBuffer`.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(array, 10, 0, 10);
 *
 *     PARCByteArray *byteArray = parcReadOnlyBuffer_Array(roBuffer);
 *
 *     ...
 *
 *     parcByteArray_Release(&byteArray);
 *     parcReadOnlyBuffer_Release(&roBuffer);
 * }
 * @endcode
 */
PARCByteArray *parcReadOnlyBuffer_Array(const PARCReadOnlyBuffer *buffer);

/**
 * Create a copy of the given `PARCReadOnlyBuffer`.
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A `PARCReadOnlyBuffer` instance.
 *
 * @return A `PARCReadOnlyBuffer` instance which is an identical, independent copy of the original.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(array, 10, 0, 10);
 *
 *     PARCReadOnlyBuffer *copy = parcReadOnlyBuffer_Copy(roBuffer);
 *
 *     if (parcReadOnlyBuffer_Equals(roBuffer, copy)) {
 *         printf("ROBuffers are equal\n");
 *     }
 *
 *     parcReadOnlyBuffer_Release(&copy);
 *     parcReadOnlyBuffer_Release(&roBuffer);
 * }
 * @endcode
 *
 * @see parcReadOnlyBuffer_Equals
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Copy(const PARCReadOnlyBuffer *original);

/**
 * Returns the offset within this buffer's backing {@link PARCByteArray} of the first element.
 *
 * Buffer position <i>p</i> corresponds to array index <i>p + arrayOffset()</i>.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The offset within this `PARCReadOnlyBuffer`'s array of the first element of the buffer
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(array, 10, 0, 10);
 *
 *     size_t arrayOffset = parcReadOnlyBuffer_ArrayOffset(buffer);
 *     // offset will be 0 since the contents of the buffer start at the beginning
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 * }
 * @endcode
 */
size_t parcReadOnlyBuffer_ArrayOffset(const PARCReadOnlyBuffer *buffer);

/**
 * Rewinds this `PARCReadOnlyBuffer`: The position is set to zero and the mark is invalidated.
 *
 * The mark is made invalid and any subsequent operation on the resulting
 * {@link PARCBuffer} that requires the mark will abort until the mark
 * is set again via {@link parcBuffer_Mark}.
 *
 * @param [in,out] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The given `PARCReadOnlyBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     parcReadOnlyBuffer_Rewind(roBuffer);
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Rewind(PARCReadOnlyBuffer *buffer);

/**
 * Resets the given `PARCReadOnlyBuffer`'s position to the previously-marked position.
 *
 * Invoking this method neither changes nor invalidates the mark's value.
 *
 * @param [in,out] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The given `PARCReadOnlyBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     parcReadOnlyBuffer_Reset(roBuffer);
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Reset(PARCReadOnlyBuffer *buffer);

/**
 * Return the given `PARCReadOnlyBuffer`'s limit.
 *
 * A buffer's limit is the index of the first element that should not be read or written.
 * A buffer's limit is never negative and is never greater than its capacity.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The given `PARCReadOnlyBuffer`'s limit.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     size_t limit = parcReadOnlyBuffer_Limit(roBuffer);
 *     // limit will be 10
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
size_t parcReadOnlyBuffer_Limit(const PARCReadOnlyBuffer *buffer);

/**
 * Sets this buffer's mark at its position.
 *
 * @param [in,out] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The given `PARCReadOnlyBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     parcReadOnlyBuffer_Mark(roBuffer);
 *     // since the position was 0, the mark remains at 0
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Mark(PARCReadOnlyBuffer *buffer);

/**
 * Sets this `PARCReadOnlyBuffer`'s limit.
 *
 * If the position is larger than the new limit then it is set to the new limit.
 *
 * If the mark is defined and larger than the new limit then the mark is invalidated and
 * any subsequent operation that requires the mark will abort until the mark
 * is set again via {@link parcReadOnlyBuffer_Mark()}.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 * @param [in] newLimit The new limit value; must be no larger than this `PARCReadOnlyBuffer`'s capacity
 *
 * @return The given `PARCReadOnlyBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     parcReadOnlyBuffer_SetLimit(roBuffer, 8);
 *     // the limit is now 8, but the capacity is 10
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_SetLimit(PARCReadOnlyBuffer *buffer, size_t newLimit);

/**
 * Return a pointer to memory that can be cast to a specific type.
 *
 * This does not guarantee proper memory alignment.
 * It is possible to obtain a pointer to memory that cannot be accessed
 * as integer or other types because of CPU memory alignment requirements.
 *
 * @param [in,out] buffer A `PARCReadOnlyBuffer` pointer.
 * @param [in] length The number of bytes to advance the buffer's position.
 *
 * @return non-NULL A pointer to memory.
 *
 * Example:
 * @code
 * {
 *     char *expected = "Hello World";
 *     struct timeval theTime = { .tv_sec = 123, .tv_usec = 456};
 *
 *     PARCBuffer *buffer = parcBuffer_Allocate(sizeof(uint16_t) + strlen(expected) + sizeof(theTime));
 *
 *     parcBuffer_PutUint16(buffer, strlen(expected));
 *     parcBuffer_PutUint8(buffer, expected, strlen(expected));
 *     parcBuffer_PutUint8(buffer, &theTime, sizeof(theTime));
 *     parcBuffer_Flip();
 *
 *     uint16_t length = parcBuffer_GetUint16(buffer);
 *
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     char *actual = parcReadOnlyBuffer_Overlay(roBuffer, length);
 *
 *     struct timeval *tm = parcReadOnlyBuffer_Overlay(roBuffer, sizeof(struct timeval));
 * }
 * @endcode
 */
void *parcReadOnlyBuffer_Overlay(PARCReadOnlyBuffer *buffer, size_t length);

/**
 * Return the given `PARCReadOnlyBuffer`'s position.
 *
 * A buffer's position is the index of the next element to be read or written.
 * A buffer's position is never negative and is never greater than its limit.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The given `PARCReadOnlyBuffer`'s position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     size_t position = parcReadOnlyBuffer_Position(roBuffer);
 *     // position is zero since the position of the underlying buffer is also 0
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
size_t parcReadOnlyBuffer_Position(const PARCReadOnlyBuffer *buffer);

/**
 * Set the given `PARCReadOnlyBuffer`'s position.
 *
 * A buffer's position is the index of the next element to be read or written.
 * A buffer's position is never negative and is never greater than its limit.
 *
 * If the mark is defined and larger than the new position then the mark
 * is invalidated and any subsequent operation on the resulting
 * `PARCReadOnlyBuffer` that requires the mark will abort until the mark
 * is set again via `parcReadOnlyBuffer_Mark`.
 *
 * @param [in,out] buffer A `PARCReadOnlyBuffer` pointer.
 * @param [in] newPosition The value of the new position which must be less than or equal to the buffer's current limit.
 *
 * @return The given `PARCReadOnlyBuffer`'s position.
 *
 * Example:
 * @code
 * {
 *     PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Create(parcBuffer_Allocate(10));
 *     parcReadOnlyBuffer_SetPosition(buffer, 5);
 *     parcReadOnlyBuffer_Remaining(buffer); // Returns 5.
 * }
 * @endcode
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_SetPosition(PARCReadOnlyBuffer *buffer, size_t newPosition);

/**
 * Returns the number of elements between the current position and the limit.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The number of elements remaining in this `PARCReadOnlyBuffer`.
 *
 * Example:
 * @code
 * {
 *     parcReadOnlyBuffer_SetPosition(buffer, 5);
 *     parcReadOnlyBuffer_Remaining(buffer); // Returns 5.
 * }
 * @endcode
 */
size_t parcReadOnlyBuffer_Remaining(const PARCReadOnlyBuffer *buffer);

/**
 * Tells whether there are any elements between the current position and the limit.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return true if, and only if, there is at least one element remaining in this `PARCReadOnlyBuffer`.
 *
 * Example:
 * @code
 * {
 *     parcReadOnlyBuffer_SetPosition(buffer, 5);
 *     bool remaining = parcReadOnlyBuffer_HasRemaining(buffer);
 *     // remaining is true, since #remaining = 5
 * }
 * @endcode
 */
bool parcReadOnlyBuffer_HasRemaining(const PARCReadOnlyBuffer *buffer);

/**
 * Set the limit to the current position, then set the position to zero.
 * If the mark is defined, it is invalidated.
 *
 * @param [in,out] buffer The `PARCReadOnlyBuffer` pointer to be modified.
 *
 * @return The same value as `buffer`
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutByte(buffer, 'X');
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     parcReadOnlyBuffer_Flip(roBuffer);
 *     uint8_t actual = parcReadOnlyBuffer_GetUint8(roBuffer);
 *
 *     ...
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_Flip(PARCReadOnlyBuffer *buffer);

/**
 * Get the single uint8_t at the index specified.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 * @param [in] index The index from which to retrieve the single byte in the buffer.
 *
 * @return The uint8_t value at the specified index
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutByte(buffer, 'X');
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     uint8_t byte = parcReadOnlyBuffer_GetAtIndex(roBuffer, 0);
 *
 *     ...
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 */
uint8_t parcReadOnlyBuffer_GetAtIndex(const PARCReadOnlyBuffer *buffer, size_t index);

/**
 * Read an array of length bytes from the given PARCReadOnlyBuffer, copying them to an array.
 *
 * The buffer's position is incremented by @p length.
 *
 * @param [in] buffer The `PARCReadOnlyBuffer` containing the `uint8_t` value.
 * @param [out] array The `uint8_t` array to receive @p length bytes.
 * @param [in] length The number of `uint8_t` elements to get..
 *
 * @return The given `PARCReadOnlyBuffer`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(5);
 *     parcBuffer_PutUint8(buffer, 'A');
 *     parcBuffer_PutUint8(buffer, 'B');
 *     parcBuffer_PutUint8(buffer, 'C');
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *
 *     uint8_t array[3];
 *     parcReadOnlyBuffer_GetArray(roBuffer, 3, array);
 *     // array[0] == 'A'
 *     // array[1] == 'B'
 *     // array[2] == 'C'
 * }
 * @endcode
 *
 * @see parcReadOnlyBuffer_Overlay
 */
PARCReadOnlyBuffer *parcReadOnlyBuffer_GetArray(PARCReadOnlyBuffer *buffer, uint8_t *array, size_t length);

/**
 * Get the single `uint8_t` at the current buffer position.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The `uint8_t` value at the current position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutByte(buffer, 'X');
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *     parcReadOnlyBuffer_Flip(roBuffer);
 *
 *     uint8_t actual = parcReadOnlyBuffer_GetUint8(roBuffer);
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
uint8_t parcReadOnlyBuffer_GetUint8(PARCReadOnlyBuffer *buffer);

/**
 * Get the single `uint16_t` at the current buffer position.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The `uint16_t` value at the current position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint16(buffer, 0x1234);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *     parcReadOnlyBuffer_Flip(roBuffer);
 *
 *     uint16_t actual = parcReadOnlyBuffer_GetUint16(roBuffer);
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
uint16_t parcReadOnlyBuffer_GetUint16(PARCReadOnlyBuffer *buffer);

/**
 * Get the single `uint32_t` at the current buffer position.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` pointer.
 *
 * @return The `uint32_t` value at the current position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint32(buffer, 0x12345678);
 *     PARCReadOnlyBuffer *roBuffer = parcReadOnlyBuffer_Create(buffer);
 *     parcReadOnlyBuffer_Flip(roBuffer);
 *
 *     uint32_t actual = parcReadOnlyBuffer_GetUint32(roBuffer);
 *
 *     parcReadOnlyBuffer_Release(&roBuffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
uint32_t parcReadOnlyBuffer_GetUint32(PARCReadOnlyBuffer *buffer);

/**
 * Read the unsigned 64-bit value in network order at the buffer's current position,
 * and then increment the position by 8.
 *
 * @param [in,out] buffer The `PARCReadOnlyBuffer` containing the value.
 *
 * @return The `uint64_t` at the buffer's current position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(sizeof(uint64_t));
 *     parcBuffer_PutUint64(buffer, 0x1234567812345678);
 *     parcBuffer_Flip(buffer);
 *
 *     PARCReadOnlyBuffer *readOnly = parcReadOnlyBuffer_Create(buffer);
 *     uint64_t actual = parcReadOnlyBuffer_GetUint64(&readOnly);
 * }
 * @endcode
 */
uint64_t parcReadOnlyBuffer_GetUint64(PARCReadOnlyBuffer *buffer);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the {@link parcReadOnlyBuffer_HashCode} function must consistently return the same value,
 * provided no information used in a corresponding {@link parcReadOnlyBuffer_Equals()}
 * comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link parcReadOnlyBuffer_Equals} method,
 * then calling the {@link parcReadOnlyBuffer_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the {@link parcReadOnlyBuffer_Equals()} function,
 * then calling the {@link parcReadOnlyBuffer_HashCode()}
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] buffer A pointer to the `PARCReadOnlyBuffer` instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     uint32_t hashValue = parcReadOnlyBuffer_HashCode(buffer);
 * }
 * @endcode
 *
 * @see parcReadOnlyBuffer_Equals
 */
PARCHashCode parcReadOnlyBuffer_HashCode(const PARCReadOnlyBuffer *buffer);

/**
 * Create a null-terminated C string containing the bytes of the given `PARCReadOnlyBuffer`.
 *
 * The string consists of the bytes from the current position of the buffer to its limit.
 *
 * @param [in] buffer A `PARCReadOnlyBuffer` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to a null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     char *string = parcReadOnlyBuffer_ToString(buffer);
 *
 *     parcMemory_Deallocate((void **)&string);
 * }
 * @endcode
 */
char *parcReadOnlyBuffer_ToString(const PARCReadOnlyBuffer *buffer);

/**
 * Print a human readable representation of the given `PARCReadOnlyBuffer`.
 *
 * Print on standard output a human readable representation of the given `PARCReadOnlyBuffer` indented by the level indicated.
 *
 * @param [in] buffer A pointer to the `PARCReadOnlyBuffer` instance.
 * @param [in] indentation The number of tabs by which to indent the output strings.
 *
 * Example:
 * @code
 * {
 *     parcReadOnlyBuffer_Display(buffer, 0);
 * }
 * @endcode
 */
void parcReadOnlyBuffer_Display(const PARCReadOnlyBuffer *buffer, int indentation);
#endif // libparc_parc_ReadOnlyBuffer_h
