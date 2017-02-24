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
 * @file parc_Buffer.h
 * @ingroup memory
 * @brief An indexable, linear buffer of bytes.
 *
 * A `PARCBuffer` is a linear, finite sequence of bytes.
 * The essential properties of a buffer are its content, its capacity, limit, and position:
 *
 * @htmlonly
 * <svg xmlns="http://www.w3.org/2000/svg" xmlns:xl="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" version="1.1" viewBox="48 143 304 126" width="304pt" height="126pt">
 * <defs>
 * <font-face font-family="Helvetica Neue" font-size="10" panose-1="2 0 5 3 0 0 0 2 0 4" units-per-em="1000" underline-position="-100" underline-thickness="50" slope="0" x-height="517" cap-height="714" ascent="951.99585" descent="-212.99744" font-weight="500">
 * <font-face-src>
 * <font-face-name name="HelveticaNeue"/>
 * </font-face-src>
 * </font-face>
 * <marker orient="auto" overflow="visible" markerUnits="strokeWidth" id="FilledArrow_Marker" viewBox="-1 -4 10 8" markerWidth="10" markerHeight="8" color="black">
 * <g>
 * <path d="M 8 0 L 0 -3 L 0 3 Z" fill="currentColor" stroke="currentColor" stroke-width="1"/>
 * </g>
 * </marker>
 * </defs>
 * <g stroke="none" stroke-opacity="1" stroke-dasharray="none" fill="none" fill-opacity="1">
 * <title>icon_512x512</title>
 * <rect fill="white" width="512" height="512"/>
 * <g>
 * <title>Layer 1</title>
 * <text transform="translate(73 231.5)" fill="black">
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x=".055" y="10" textLength="13.37">Off</tspan>
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x="13.425" y="10" textLength="13.52">set</tspan>
 * </text>
 * <text transform="translate(178.5 231)" fill="black">
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x=".445" y="10" textLength="36.11">Position</tspan>
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x="7.2" y="22" textLength="22.6">Mark</tspan>
 * </text>
 * <text transform="translate(294 231.5)" fill="black">
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x=".16" y="10" textLength="21.68">Limit</tspan>
 * </text>
 * <text transform="translate(271 157.5)" fill="black">
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x=".185" y="10" textLength="39.63">Capacity</tspan>
 * </text>
 * <text transform="translate(83 157.5)" fill="black">
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x=".245" y="10" textLength="23.51">Array</tspan>
 * </text>
 * <path d="M 78 165.48333 C 73.6671 165.98884 67.757544 163.49623 65 167 C 63.49089 168.91749 62.925312 172.63153 62.52875 176.66717" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <rect x="59" y="187" width="281" height="13" fill="#205469"/>
 * <rect x="59" y="187" width="281" height="13" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <path d="M 316 166.11628 C 321.9994 166.74412 330.42197 164.60163 334 168 C 335.8631 169.76954 336.41326 173.04195 336.67595 176.64326" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <rect x="116" y="181" width="160" height="27" fill="#f60"/>
 * <rect x="116" y="181" width="160" height="27" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <path d="M 105 234.05085 C 108.6663 233.3673 114.16685 236.34137 116 232 C 117.15073 229.27477 116.856755 223.66586 116.47841 217.88884" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <path d="M 289 235.56897 C 284.6671 235.04603 278.16645 238.59437 276 234 C 274.57827 230.98495 275.02256 224.46193 275.496 217.88448" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <line x1="173.5" y1="232.84568" x2="125.08789" y2="211.92687" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <line x1="220.5" y1="232.58861" x2="266.94855" y2="212.01014" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * </g>
 * </g>
 * </svg>
 * @endhtmlonly
 *
 * * A buffer's capacity is the number of bytes it contains.
 *   The capacity of a buffer is never negative and never changes.
 *
 * * A buffer's limit is the index of the first byte that should not be read or written.
 *   A buffer's limit is never negative and is never greater than its capacity.
 *
 * * A buffer's position is a cursor to or index of the next byte to be read or written.
 *   A buffer's position is never negative and is never greater than its limit.
 *
 * A PARCBuffer's capacity may be larger than the extent of data manipulated by the buffer.
 * The extent of the data manipulated by the buffer is manipulated via:
 * * {@link parcBuffer_Position},
 * * {@link parcBuffer_SetPosition},
 * * {@link parcBuffer_Limit},
 * * {@link parcBuffer_SetLimit} and
 * * {@link parcBuffer_Flip}.
 *
 * Strictly speaking, these relations are always true: _0 <= mark <= position <= limit <= capacity_
 *
 * The general model for use is to:
 * * Create a buffer using a form of {@link parcBuffer_Allocate} or {@link parcBuffer_Wrap}.
 * * Optionally insert data into the buffer via put operations,
 *   ultimately setting the position at the end of the valid data.
 * * 'Flip' the buffer using the {@link parcBuffer_Flip} function to set the position to 0 and the limit at the end
 *   of the valid data.
 * * Optionally get data from the buffer via one of the many get operations.
 * * Use {@link parcBuffer_Rewind} function to set the position to 0 again, leaving the limit at the end of the valid data.
 *
 * Data is placed into a `PARCBuffer` via `Put` functions, and retreived from the buffer via `Get` operations.
 * Both `Put` and `Get` perform their operations at the position of the buffer and update the position to the location of the
 * next element of data.
 * Both `Put` and `Get` operations have a full compliment of intrinsic data types that operate on data at
 * relative positions in the buffer.
 *
 * The function {@link parcBuffer_GetAtIndex} provides absolute index access to the buffer for bytes.
 *
 * * {@link parcBuffer_PutUint8},
 * * {@link parcBuffer_PutUint16},
 * * {@link parcBuffer_PutUint32},
 * * {@link parcBuffer_PutUint64},
 * * {@link parcBuffer_PutAtIndex}
 *
 * * {@link parcBuffer_GetUint8},
 * * {@link parcBuffer_GetUint16},
 * * {@link parcBuffer_GetUint32},
 * * {@link parcBuffer_GetUint64},
 * * {@link parcBuffer_GetAtIndex}
 *
 */
#ifndef libparc_parc_Buffer_h
#define libparc_parc_Buffer_h

typedef struct parc_buffer PARCBuffer;

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_ByteArray.h>

extern parcObjectDescriptor_Declaration(PARCBuffer);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcBuffer_OptionalAssertValid(_instance_)
#else
#  define parcBuffer_OptionalAssertValid(_instance_) parcBuffer_AssertValid(_instance_)
#endif

/**
 * Assert that an instance of `PARCBuffer` is valid.
 *
 * If the instance is not valid, terminate via `trapIllegalValue()`
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a `PARCBuffer` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(64);
 *
 *     parcBuffer_AssertValid(array);
 * }
 * @endcode
 * @see parcBuffer_OptionalAssertValid
 */
void parcBuffer_AssertValid(const PARCBuffer *instance);

/**
 * Determine if an instance of `PARCBuffer` is valid.
 *
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(64);
 *
 *     if (parcBuffer_IsValid(buffer)) {
 *         printf("Buffer is valid.\n");
 *     }
 * }
 * @endcode
 */
bool parcBuffer_IsValid(const PARCBuffer *buffer);

/**
 * Create a new instance of `PARCBuffer` using dynamically allocated memory.
 *
 * The new buffer's position will be zero,
 * its limit will be set to `length`,
 * its mark will be undefined,
 * and each of its elements will be initialized to zero.
 *
 * If capacity is zero, the buffer contains no underlying byte array.
 *
 * @param [in] capacity The number of bytes to allocate.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a `PARCBuffer` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(64);
 *
 *     parcBuffer_Release(&&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Allocate(size_t capacity);

/**
 * Create a new instance of `PARCBuffer` using using program supplied static memory (rather than allocated).
 *
 * The new buffer will be backed by the given array, @p array.
 * Modifications to the buffer will cause the array to be modified and vice versa.
 *
 * The new buffer's capacity will be @p arrayLength,
 * its initial position will be @p position ,
 * the index of the first byte that should not be read or written will be @p limit,
 * and its mark will be undefined.
 *
 * In all cases, _0 <= position <= limit <= capacity_
 *
 * Its backing array will be the given array, starting at index 0 of that array.
 *
 * @param [in] array A pointer to a memory array.
 * @param [in] arrayLength The length, in `uint8_t` units, of the memory array.
 * @param [in] position The initial value for the buffer's position.
 * @param [in] limit The initial value for the buffer's limit.
 *
 * @return A `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[64];
 *
 *     PARCBuffer *buffer = parcBuffer_Wrap(array, sizeof(array), 0, sizeof(array));
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_Allocate
 * @see parcBuffer_Release
 */
PARCBuffer *parcBuffer_Wrap(void *array, size_t arrayLength, size_t position, size_t limit);

/**
 * Create a new instance of `PARCBuffer` using referencing the given {@link PARCByteArray}.
 *
 * A reference to the `PARCByteArray` is acquired.
 *
 * The new buffer will be backed by the given `PARCByteArray`.
 * Modifications to the buffer will cause the array to be modified and vice versa.
 *
 * The new buffer's capacity will be @p arrayLength,
 * its initial position will be @p position ,
 * the index of the first byte that should not be read or written will be @p limit,
 * and its mark will be undefined.
 *
 * In all cases, _0 <= position <= limit <= capacity_
 *
 * The new buffer's
 * capacity will be the length of the `PARCByteArray`,
 * its initial position will be @p position ,
 * the index of the first byte that should not be read or written will be @p limit,
 * and its mark will be undefined.
 *
 * @param [in] byteArray A pointer to a `PARCByteArray` instance.
 * @param [in] position The initial value for the buffer's position.
 * @param [in] limit The initial value for the buffer's limit which must be less than or equal to the PARCByteArray's capacity.
 *
 * @return A `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *array = parcByteArray_Allocate(64);
 *
 *     PARCBuffer *buffer = parcBuffer_WrapByteArray(array, 0, parcByteArray_Capacity(array));
 *
 *     parcBuffer_Release(&&buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_Allocate
 * @see parcBuffer_Wrap
 */
PARCBuffer *parcBuffer_WrapByteArray(PARCByteArray *byteArray, size_t position, size_t limit);

/**
 * Create a new instance of `PARCBuffer` wrapping the given null-terminated C string as its value.
 *
 * The new buffer's capacity will be the length of the string excluding the terminating nul character.
 * its initial position will be 0,
 * the index of the first byte that should not be read or written will be @p limit,
 * and its mark will be undefined.
 *
 * @param [in] string A pointer to a C-string to copy and then wrap.
 *
 * @return A `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_Allocate
 * @see parcBuffer_Wrap
 */
PARCBuffer *parcBuffer_WrapCString(char *string);

/**
 * Create a new instance of a `PARCBuffer` copying the given null-terminated C string as its value.
 *
 * The new buffer's capacity will be the length of the string excluding the terminating nul character.
 * its initial position will be 0,
 * the index of the first byte that should not be read or written will be @p limit,
 * and its mark will be undefined.
 *
 * @param [in] string A pointer to C-string to copy and then wrap.
 *
 * @return A `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *buffer = parcBuffer_AllocateCString("test string");
 *
 *     parcBUffer_Release(&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_AllocateCString(const char *string);

/**
 * Create a `PARCBuffer` initalised with a copy of the contents of given byte array.
 *
 * The length must be non-negative (> 0) and the array pointer must not be NULL.
 * The contents of the given array are used to initialize the `PARCBuffer` instance,
 *   and the size of the new instance is equal to the specified length (just wide enough to fit the array).
 *
 * @param [in] bytes A pointer to an array of bytes.
 * @param [in] length The number of bytes to copy to the `PARCBuffer`.
 *
 * @return A newly allocated `PARCBuffer` instance that must be freed via `parcBuffer_Release()`.
 *
 * Example:
 * @code
 * {
 *     unsigned char array[] = { 1, 2, 3, 4, 5 };
 *     PARCBuffer *buffer = parcBuffer_CreateFromArray(array, sizeof(array));
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_CreateFromArray(const void *bytes, size_t length);

/**
 * Parse a null-terminated hexadecimal string to create a new `PARCBuffer` instance.
 *
 *   The hex string must be null-terminated so parsing is deterministic and correct.
 *   The hex string parameter is not modified in any way.
 *   The hex string must be an even length greater than zero.
 *
 * @param [in] hexString The hex string to parse.
 *
 * @return NULL The string could not be parsed
 * @return A new `PARCElasticBuffer` instance.
 *
 * Example:
 * @code
 * {
 *     char *expected = "0123456789ABCDEF";
 *     PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_ParseHexString(expected));
 *     printf("String: %s\n", parcBuffer_ToString(buffer));
 *
 *     parcBuffer_Release(buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_ParseHexString(const char *hexString);

/**
 * Increase or decrease the capacity of an existing PARCBuffer.
 *
 * If the new capacity is greater than the old capacity and the limit is currently set to the old capacity,
 * then set the new limit to the new capacity.
 * Otherwise, if the limit is not currently set to the capacity, then leave the limit unchanged.
 *
 * If the new capacity is less than the old capacity and the limit is currently set to the old capacity,
 * then set the new limit to the new capacity.
 * Otherwise, set the limit to the the lesser of the old limit or the new capacity.
 * If the limit is not currently set to the capacity,
 * the set the limit to the the lesser of the old limit or the new capacity.
 *
 * If the original mark exceeds the new limit, the new mark is invalidated and any subsequent
 * operation on the resulting `PARCBuffer` that requires the mark will abort until the mark
 * is set again via `parcBuffer_Mark`.
 *
 * If the original position of the buffer is beyond the new limit of the buffer, the position is set to the new limit.
 *
 * The contents of the old buffer are preserved from the origin to the new limit.
 *
 * This operation may induce a memory copy.
 * As a consequence, any `PARCBuffer` instances previously created via {@link parcBuffer_Slice}
 * refer to memory previously used by this `PARCBuffer`.
 *
 * A PARCBuffer originally created via any of the `parcBuffer_Wrap` forms,
 * may no longer refer to the original wrapped data.
 *
 * @param [in] buffer A pointer to a valid `PARCBuffer` instance.
 * @param [in] capacity The new capacity of `PARCBuffer`
 *
 * @return PARCBuffer A new `PARCBuffer` instance initialized with the contents of the given buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
 *     parcBuffer_Resize(buffer, 4);
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_Allocate
 * @see parcBuffer_Wrap
 */
PARCBuffer *parcBuffer_Resize(PARCBuffer *buffer, size_t capacity);

/**
 * Increase the number of references to a `PARCBuffer`.
 *
 * Note that new `PARCBuffer` is not created,
 * only that the given `PARCBuffer` reference count is incremented.
 * Discard the reference by invoking `parcBuffer_Release`.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The input `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *x = parcBuffer_Allocate(10);
 *
 *     PARCBuffer *x_2 = parcBuffer_Acquire(x);
 *
 *     parcBuffer_Release(&x);
 *     parcBuffer_Release(&x_2);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Acquire(const PARCBuffer *buffer);

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
 * @param [in,out] bufferPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
void parcBuffer_Release(PARCBuffer **bufferPtr);

/**
 * Returns this buffer's capacity.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The given buffer's capacity.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *
 *     size_t capacity = parcBuffer_Capacity(buffer);
 *
 *     parcBuffer_Release(&capacity);
 * }
 * @endcode
 */
size_t parcBuffer_Capacity(const PARCBuffer *buffer);

/**
 * Clear the given buffer restoring it to its initial state:
 * The position is set to zero,
 * the limit is set to the capacity,
 * and the mark is invalidated.
 *
 * The mark is made invalid and any subsequent operation on the resulting
 * `PARCBuffer` that requires the mark will abort until the mark
 * is set again via `parcBuffer_Mark`.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The value of @p buffer.
 *
 * Example:
 * @code
 * {
 *     parcBuffer_Clear(buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Clear(PARCBuffer *buffer);

/**
 * Determine if two `PARCBuffer` instances are equal.
 *
 * The following equivalence relations on non-null `PARCBuffer` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcBuffer_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcBuffer_Equals(x, y)` must return true if and only if
 *        `parcBuffer_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcBuffer_Equals(x, y)` returns true and
 *        `parcBuffer_Equals(y, z)` returns true,
 *        then `parcBuffer_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcBuffer_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcBuffer_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] x A pointer to a `PARCBuffer` instance.
 * @param [in] y A pointer to a `PARCBuffer` instance.
 *
 * @return true `PARCBuffers` x and y are equal.
 * @return false `PARCBuffers` x and y are not equal.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *bufferA = parcBuffer_Allocate(10);
 *     PARCBuffer *bufferB = parcBuffer_Allocate(10);
 *
 *     if (parcBuffer_Equals(bufferA, bufferB)) {
 *         printf("Buffers are equal.\n");
 *     } else {
 *         printf("Buffers are NOT equal.\n");
 *     }
 *
 *     parcBuffer_Release(&bufferA);
 *     parcBuffer_Release(&bufferB);
 * }
 * @endcode
 *
 * @see parcBuffer_HashCode
 */
bool parcBuffer_Equals(const PARCBuffer *x, const PARCBuffer *y);

/**
 * Compares instance a with instance b for order.
 *
 * Returns a negative integer, zero, or a positive integer as instance
 * a is less than, equal to, or greater than instance b.
 *
 * The buffer's position, limit, and mark are not modified.
 *
 * @param [in] a A pointer to the first instance of `PARCBuffer`.
 * @param [in] b A pointer to the second instance  of `PARCBuffer`.
 *
 * @return <0 Instance a is less than instance b.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *bufferA = parcBuffer_Allocate(10);
 *     PARCBuffer *bufferB = parcBuffer_Allocate(10);
 *
 *     if (parcBuffer_Compare(bufferA, bufferB) == 0) {
 *         printf("Buffers are equal.\n");
 *     }
 *
 *     parcBuffer_Release(&bufferA);
 *     parcBuffer_Release(&bufferB);
 * }
 * @endcode
 *
 * @see parcBuffer_Equals
 */
int parcBuffer_Compare(const PARCBuffer *a, const PARCBuffer *b);

/**
 * Return a pointer to the {@link PARCByteArray} that backs this buffer.
 *
 * If this `PARCBuffer` has a capacity of zero,
 * there is no array of bytes and this function returns NULL.
 *
 * Modifications to the contents of the `PARCByteArray` will visible to the given
 * `PARCBuffer` and vice-versa.
 *
 * The origin of the given `PARCBuffer` may not be the same as the origin of the underlying
 * `PARCByteArray`.
 * Use {@link parcBuffer_ArrayOffset} to obtain the origin of the given `PARCBuffer`
 * relative to the origin of the underlying `PARCByteArray`
 *
 * The caller must obtain its own reference to the `PARCByteArray` if it intends to store it elsewhere.
 *
 * Note: Many hard to find bugs can be caused by using this function.
 * Use the functions provided to manipulate the `PARCBuffer` and its contents.
 *
 * @param [in] buffer A `PARCBuffer` pointer.
 *
 * @return NULL There is no `PARCByteArray` backing the given `PARCBuffer` (no capacity).
 * @return non-NULL The pointer to the `PARCByteArray` for the given `PARCBuffer`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *
 *     PARCByteArray *array = parcBuffer_Array(buffer);
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcByteArray_Acquire
 */
PARCByteArray *parcBuffer_Array(const PARCBuffer *buffer);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCBuffer` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     PARCBuffer *copy = parcBuffer_Copy(buffer);
 *
 *     parcBuffer_Release(&copy);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 */
PARCBuffer *parcBuffer_Copy(const PARCBuffer *buffer);

/**
 * Creates a new buffer that shares the original buffer's content.
 *
 * The content of the new buffer will be that of this buffer.
 * Changes to the buffer's content will be visible in both buffers,
 * however the two buffers' position, limit, and mark values will be independent.
 *
 * The new buffer's capacity, limit, position, and mark values will be identical to those of the original buffer.
 *
 * @param [in] original The orignal PARCBuffer instance that will be duplicated.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to ta valid `PARCBuffer` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_Duplicate(buffer2);
 *
 *     parcBuffer_Release(&buffer);
 *     parcBuffer_Release(&buffer2);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Duplicate(const PARCBuffer *original);

/**
 * Returns the offset within this buffer's backing {@link PARCByteArray} of the first element.
 *
 * Buffer position <i>p</i> corresponds to array index <i>p + arrayOffset()</i>.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return `size_t` The offset within this `PARCBuffer`'s array of the first element of the buffer
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     size_t arrayOffset = parcBuffer_ArrayOffset(buffer);
 *     // offset will be 0 since the contents of the buffer start at the beginning
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
size_t parcBuffer_ArrayOffset(const PARCBuffer *buffer);

/**
 * Rewinds this `PARCBuffer`: The position is set to zero and the mark is invalidated.
 *
 * The mark is made invalid and any subsequent operation on the resulting
 * `PARCBuffer` that requires the mark will abort until the mark
 * is set again via `parcBuffer_Mark`.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The given `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     parcBuffer_Rewind(buffer); // bring it back to zero
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Rewind(PARCBuffer *buffer);

/**
 * Resets the given `PARCBuffer`'s position to the previously-marked position.
 *
 * Invoking this method neither changes nor invalidates the mark's value.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The given `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     buffer = parcBuffer_Reset(buffer);
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Reset(PARCBuffer *buffer);

/**
 * Return the given `PARCBuffer`'s limit.
 *
 * A buffer's limit is the index of the first element that should not be read or written.
 * A buffer's limit is never negative and is never greater than its capacity.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The given `PARCBuffer`'s limit.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     size_t limit = parcBuffer_Limit(buffer);
 *     // limit will be 10
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
size_t parcBuffer_Limit(const PARCBuffer *buffer);

/**
 * Sets this buffer's mark at its position.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The value of @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *     parcBuffer_Mark(buffer);
 *     // since the position was 0, the mark remains at 0
 *
 *     ...
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Mark(PARCBuffer *buffer);

/**
 * Sets this `PARCBuffer`'s limit.
 *
 * If the position is larger than the new limit then it is set to the new limit.
 *
 * If the mark is defined and larger than the new limit then the mark is invalidated and
 * any subsequent operation that requires the mark will abort until the mark
 * is set again via `parcBuffer_Mark`
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 * @param newLimit The new limit value; must be no larger than this `PARCBuffer`'s capacity.
 *
 * @return The given `PARCBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint8(buffer, (uint8_t)'A');
 *
 *     parcBuffer_SetLimit(buffer, 8);
 *
 *     size_t limit = parcBuffer_Limit(buffer);
 *     size_t capacity = parcBuffer_Capacity(buffer);
 *     // capacity is 10, limit is 8
 *
 *     ...
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_SetLimit(PARCBuffer *buffer, size_t newLimit);

/**
 * Return a pointer to buffer memory starting the buffer's current position.
 *
 * The @p length parameter must be less than or equal to the remaining bytes in the buffer
 * and has no effect on the return value,
 * except that if the buffer's position is equal to the limit, then this traps with OutOfBounds
 *
 * The current position of the buffer is advanced by @p length bytes.
 * It is acceptable for the @p length parameter to be zero,
 * thereby causing the current position to remain unchanged.
 *
 * This does not guarantee any particular memory alignment.
 * Therefore, it is possible to obtain a pointer to memory that cannot be accessed
 * as a native type because of CPU architecture alignment requirements.
 *
 * The function returns a pointer to contiguous memory within a `PARCBuffer`,
 * but does not acquire a reference to the `PARCBuffer` instance,
 * the underlying {@link PARCByteArray}, nor the actual memory array.
 * If the {@link PARCBuffer} or the underlying {@link PARCByteArray} is released finally,
 * the result from a previous call to `parcBuffer_Overlay` will point to undefined values.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
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
 *     char *actual = parcBuffer_Overlay(buffer, length);
 *     struct timeval *tm = parcBuffer_Overlay(buffer, sizeof(struct timeval));
 * }
 * @endcode
 */
void *parcBuffer_Overlay(PARCBuffer *buffer, size_t length);

/**
 * Return the given `PARCBuffer`'s position.
 *
 * A buffer's position is the index of the next element to be read or written.
 * A buffer's position is never negative and is never greater than its limit.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 * @return The given `PARCBuffer`'s position.
 *
 * Example:
 * @code
 * {
 *     size_t currentPosition = parcBuffer_Position(buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_SetPosition
 */
size_t parcBuffer_Position(const PARCBuffer *buffer);

/**
 * Set the given `PARCBuffer`'s position.
 *
 * A buffer's position is the index of the next element to be read or written.
 * A buffer's position is never negative and is never greater than its limit.
 *
 * If the mark is defined and larger than the new position then the mark
 * is invalidated and any subsequent operation on the resulting
 * `PARCBuffer` that requires the mark will abort until the mark
 * is set again via `parcBuffer_Mark`.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 * @param [in] newPosition The buffer's new position which must be less than or equal to the current limit.
 *
 * @return The given `PARCBuffer`'s position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_SetPosition(buffer, 5);
 *     parcBuffer_Remaining(buffer); // Returns 5.
 * }
 * @endcode
 *
 * @see parcBuffer_Limit
 */
PARCBuffer *parcBuffer_SetPosition(PARCBuffer *buffer, size_t newPosition);

/**
 * Returns the number of elements between the current position and the limit.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The number of elements remaining in this `PARCBuffer`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_SetPosition(buffer, 5);
 *     parcBuffer_Remaining(buffer); // Returns 5.
 * }
 * @endcode
 */
size_t parcBuffer_Remaining(const PARCBuffer *buffer);

/**
 * Tells whether there are any elements between the current position and the limit.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return true The `PARCBuffer` contains at least one more element.
 * @return false The `PARCBuffer` does not contain any more elements.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_SetPosition(buffer, 5);
 *     bool hasRemaining = parcBuffer_HasRemaining(buffer); // returns true since #remaining = 5
 * }
 * @endcode
 */
bool parcBuffer_HasRemaining(const PARCBuffer *buffer);

/**
 * Creates a new byte buffer whose content is a shared subsequence of this buffer's content.
 *
 * The content of the new buffer will start at this buffer's current position.
 * Changes to this buffer's content will be visible in the new buffer,
 * and vice versa;
 * the two buffers' position, limit,
 * and mark values will be independent.
 *
 * The new buffer's position will be zero,
 * its capacity and its limit will be the number of bytes remaining in this buffer,
 * and its mark will be undefined.
 *
 * @param [in] original A pointer to a `PARCBuffer` instance.
 *
 * @return non-NULL A pointer to a new `PARCBuffer` whose content is a shared subsequence of the original buffer's content.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_SetPosition(buffer, 5);
 *     parcBuffer_SetLimit(buffer, 8);
 *
 *     PARCBuffer *slice = parcBuffer_Slice(buffer);
 *     // the slice will be the subset of bytes 5,6,7, and will
 *     // have limit and capacity of 3 (= 8 - 5)
 *
 *     ...
 *
 *     parcBuffer_Release(&buffer);
 *     parcBuffer_Release(&slice);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Slice(const PARCBuffer *original);

/**
 * Set the limit to the current position,
 * then set the position to zero.
 * If the mark is defined, it is invalidated.
 *
 * Any subsequent operation that requires the mark will abort until the mark
 * is set again via `parcBuffer_Mark`.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The same value as @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutByte(buffer, 'X');
 *     parcBuffer_Flip(buffer);
 *     uint8_t actual = parcBuffer_GetUint8(buffer);
 *
 *     ...
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_Flip(PARCBuffer *buffer);

/**
 * Get the single `uint8_t` at the index specified.
 *
 * The buffer's position is not modified.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 * @param [in] index The index into the @p buffer to find the `uint8_t`.
 *
 * @return The `uint8_t` value at @p index.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutByte(buffer, 'X');
 *     uint8_t actual = parcBuffer_GetAtIndex(buffer, 0);
 *     // actual == (uint8_t) 'X'
 *
 *     ...
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
uint8_t parcBuffer_GetAtIndex(const PARCBuffer *buffer, size_t index);

/**
 * Read the unsigned 8-bit value at the buffer's current position,
 * and then increment the position by 1.
 *
 * @param [in] buffer The pointer to a `PARCBuffer` instance containing the `uint8_t` value.
 *
 * @return The `uint8_t` at the buffer's current position
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutByte(buffer, 'X');
 *     parcBuffer_Flip(buffer);
 *     uint8_t actual = parcBuffer_GetUint8(buffer);
 *
 *     ...
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 */
uint8_t parcBuffer_GetUint8(PARCBuffer *buffer);

/**
 * Read the unsigned 16-bit value in network order at the buffer's current position,
 * and then increment the position by 2.
 *
 * @param [in,out] buffer The pointer to the `PARCBuffer` instance containing the value.
 *
 * @return The `uint16_t` at the buffer's current position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *      (buffer, 0x1234);
 *     parcBuffer_Flip(buffer);
 *     uint16_t actual = parcBuffer_GetUint16(buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_Overlay
 */
uint16_t parcBuffer_GetUint16(PARCBuffer *buffer);

/**
 * Read the unsigned 32-bit value in network order at the buffer's current position,
 * and then increment the position by the 4.
 *
 * @param [in,out] buffer The pointer to the instance of `PARCBuffer` containing the value.
 *
 * @return The `uint32_t` at the buffer's current position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint32(buffer, 0x12345678);
 *     parcBuffer_Flip(buffer);
 *     uint32_t actual = parcBuffer_GetUint32(buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_Overlay
 */
uint32_t parcBuffer_GetUint32(PARCBuffer *buffer);

/**
 * Read the unsigned 64-bit value in network order at the buffer's current position,
 * and then increment the position by 8.
 *
 * @param [in,out] buffer The pointer to the instance of `PARCBuffer` containing the value.
 *
 * @return The `uint64_t` at the buffer's current position.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint64(buffer, 0x12345678);
 *     parcBuffer_Flip(buffer);
 *     uint64_t actual = parcBuffer_GetUint64(buffer);
 * }
 * @endcode
 *
 * @see parcBuffer_Overlay
 */
uint64_t parcBuffer_GetUint64(PARCBuffer *buffer);

/**
 * Read an array of length bytes from the given PARCBuffer, copying them to an array.
 *
 * The buffer's position is incremented by @p length.
 *
 * @param [in,out] buffer The pointer to the instance of `PARCBuffer` containing the `uint8_t` value.
 * @param [in] length The number of `uint8_t` elements to get.
 * @param [in] array The `uint8_t` array to receive @p length bytes.
 *
 * @return A pointer to the given `PARCBuffer` instance
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(5);
 *     parcBuffer_PutUint8(buffer, 'A');
 *     parcBuffer_PutUint8(buffer, 'B');
 *     parcBuffer_PutUint8(buffer, 'C');
 *
 *     uint8_t array[3];
 *     parcBuffer_GetBytes(buffer, 3, array);
 *     // array[0] == 'A'
 *     // array[1] == 'B'
 *     // array[2] == 'C'
 * }
 * @endcode
 *
 * @see parcBuffer_Overlay
 */
PARCBuffer *parcBuffer_GetBytes(PARCBuffer *buffer, size_t length, uint8_t *array);

/**
 * Insert an unsigned 8-bit value into the given `PARCBuffer` at the current position.
 *
 * Advance the current position by 1.
 *
 * @param [in,out] buffer A pointer to the `PARCBuffer` instance.
 * @param [in] value The value to be inserted into the`PARCBuffer` instance at the current position.
 * @return The `PARCBuffer`
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutByte(buffer, 'X');
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutUint8(PARCBuffer *buffer, uint8_t value);

/**
 * Insert an unsigned 16-bit value into the given `PARCBuffer` at the current position,
 * in big-endian, network-byte-order.
 *
 * Advance the current position by 2.
 *
 * @param [in,out] buffer A pointer to the `PARCBuffer` instance.
 * @param [in] value The value to be inserted
 * @return The pointer to `PARCBuffer`
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint16(buffer, 0x1234);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutUint16(PARCBuffer *buffer, uint16_t value);

/**
 * Insert an unsigned 32-bit value into the given `PARCBuffer` at the current position,
 * in big-endian, network-byte-order.
 *
 * Advance the current position by 4.
 *
 * @param [in,out] buffer A pointer to the `PARCBuffer` instance.
 * @param [in] value The value to be inserted
 * @return The `PARCBuffer`
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint32(buffer, 0x12345678);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutUint32(PARCBuffer *buffer, uint32_t value);

/**
 * Insert an unsigned 64-bit value into the given `PARCBuffer` at the current position,
 * in big-endian, network-byte-order.
 *
 * Advance the current position by 8.
 *
 * @param [in,out] buffer A pointer to the `PARCBuffer` instance.
 * @param [in] value The value to be inserted
 * @return The `PARCBuffer`
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutUint64(buffer, 0x1234);
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutUint64(PARCBuffer *buffer, uint64_t value);

/**
 * Insert unsigned 8-bit value to the given `PARCBuffer` at given index.
 *
 * The buffer's position is unchanged.
 *
 * @param [in,out] buffer A pointer to the `PARCBuffer` instance.
 * @param [in] index The index at which to insert @p value
 * @param [in] value The value to be inserted
 *
 * @return The value of @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutAtIndex(buffer, 3, 'X');
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutAtIndex(PARCBuffer *buffer, size_t index, uint8_t value);

/**
 * Copy `arrayLength` bytes from the given array into the `PARCBuffer`.
 *
 * The position is incremented by `arrayLength`
 *
 * @param [in] buffer A pointer to the `PARCBuffer` instance.
 * @param [in] arrayLength The number of bytes to copy into the buffer.
 * @param [in] array A pointer to the array of bytes.
 *
 * @return The value of @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(5);
 *
 *     uint8_t array[3];
 *     array[0] = 'A';
 *     array[1] = 'B';
 *     array[2] = 'C';
 *
 *     parcBuffer_PutArray(buffer, 3, array);
 *     // the buffer will now contain ['A','B','C'] at indices 0,1,2
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutArray(PARCBuffer *buffer, size_t arrayLength, const uint8_t *array);

/**
 * Copy the contents of the given nul-terminated C string into a PARCBuffer, including the terminating nul byte.
 *
 * The position is incremented by the length of the string plus 1.
 *
 * @param [in] buffer A pointer to the `PARCBuffer` instance.
 * @param [in] string A pointer to nul-terminated C string.
 *
 * @return The value of @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(5);
 *
 *     char *string = "ABC";
 *
 *     parcBuffer_PutCString(buffer, string);
 *     // the buffer will now contain ['A','B','C', 0] at indices 0, 1, 2, and 3
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutCString(PARCBuffer *buffer, const char *string);

/**
 * Put the contents of a `PARCBuffer` into another.
 *
 * @param [in] buffer A pointer to the destination `PARCBuffer` instance.
 * @param [in] source A pointer to the source `PARCBuffer` instance.
 *
 * @return The value of @p result.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     PARCBuffer *insertee = parcBuffer_AllocateCString("Hello");
 *     parcBuffer_PutBuffer(buffer, insertee);
 *     // buffer will now contain "Hello" in the first 5 byte indices
 * }
 * @endcode
 */
PARCBuffer *parcBuffer_PutBuffer(PARCBuffer *buffer, const PARCBuffer *source);

/**
 * Returns a hash code value for the given instance.
 *
 * The hash code of a `PARCBuffer` depends only upon its remaining elements from the current position to the limit.
 *
 * Because `PARCBuffer` hash codes are content-dependent, be careful when using them as keys in `PARCHashMap`
 * and other similar data structures unless it is known that their contents will not change.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the `HashCode` function must consistently return the same value,
 * provided no information used in a corresponding {@link parcByteArray_Equals}
 * comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link parcByteArray_Equals} method,
 * then calling the {@link parcBuffer_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the {@link parcBuffer_Equals} function,
 * then calling the `parcBuffer_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] buffer A pointer to the `PARCBuffer` instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Allocate(10);
 *     uint32_t hashValue = parcBuffer_HashCode(buffer);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcByteArray_HashCode
 */
PARCHashCode parcBuffer_HashCode(const PARCBuffer *buffer);

/**
 * Return the position of the first `uint8_t` value that matches the given byte.
 *
 * If the value does not exist between the current position and the limit,
 * return `SIZE_MAX` (<stdint.h>).
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 * @param [in] byte The byte to search for within the buffer.
 *
 * @return The index of the first byte equal to `byte`, or `SIZE_MAX` (<stdint.h>)
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Wrap("Hello World", 10, 0, 10);
 *
 *     size_t ePosition = parcBuffer_FindUint8(buffer, 'e');
 *
 *     // ePosition is equal to 1.
 *
 *     size_t xPosition = parcBuffer_FindUint8(buffer, 'x');
 *
 *     // ePosition is equal to SIZE_MAX.
 * }
 * @endcode
 */
size_t parcBuffer_FindUint8(const PARCBuffer *buffer, uint8_t byte);

/**
 * Produce a null-terminated string representation of the specified `PARCBuffer`
 * from the current position to the limit.
 * The buffer's position is not changed.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] buffer A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     char *helloWorld = "Hello World";
 *     PARCBuffer *instance = parcBuffer_Wrap(helloWorld, strlen(helloWorld), 0, strlen(helloWorld));
 *
 *     char *string = parcBuffer_ToString(instance);
 *
 *     if (string != NULL) {
 *         printf("Hello: %s\n", string);
 *         parcMemory_Deallocate((void **)&string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     parcBuffer_Release(&instance);
 * }
 * @endcode
 *
 * @see parcBuffer_Display
 */
char *parcBuffer_ToString(const PARCBuffer *buffer);

/**
 * Print a human readable representation of the given `PARCBuffer`.
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] buffer A pointer to the instance to display.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *instance = parcBuffer_Create();
 *
 *     parcBuffer_Display(instance, 0);
 *
 *     parcBuffer_Release(&instance);
 * }
 * @endcode
 */
void parcBuffer_Display(const PARCBuffer *buffer, int indentation);

/**
 * Return a null-terminated string containing the hex-byte representation of the given `PARCBuffer`.
 *
 * The result must be freed by the caller via `parcMemory_Deallocate()`.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *instance = parcBuffer_Create();
 *
 *     char *hexString = parcBuffer_ToHexString(instance);
 *     parcMemory_Deallocate((void **)&hexString);
 *
 *     parcBuffer_Release(&instance);
 * }
 * @endcode
 *
 * @see parcMemory_Deallocate
 */
char *parcBuffer_ToHexString(const PARCBuffer *buffer);

/**
 * Advance the position of the given buffer to the first byte that is not in the array @p bytesToSkipOver.
 *
 * The position will not exceed the PARCBuffer's limit.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 * @param [in] length Length of the byte array that includes skip bytes
 * @param [in] bytesToSkipOver A null-terminated array of bytes to skip.
 *
 * @return true The `PARCBuffer`'s position was updated.
 * @return false The `PARCBuffer`'s position reached the limit.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
 *     uint8_t skipOverBytes[] = { 'H', 0 };
 *
 *     bool actual = parcBuffer_SkipOver(buffer, 1, skipOverBytes);
 *     // the buffer position will now be 1, as it skipped over the 'H' byte in the array
 *     // actual will be true
 * }
 * @endcode
 *
 * @see parcBuffer_SkipTo
 */
bool parcBuffer_SkipOver(PARCBuffer *buffer, size_t length, const uint8_t *bytesToSkipOver);

/**
 * Advance the position of the given buffer to the first byte that is in the array @p bytesToSkipTo.
 *
 * The position will not exceed the PARCBuffer's limit.
 *
 * @param [in,out] buffer A pointer to a `PARCBuffer` instance.
 * @param [in] length Length of the byte array that includes skip bytes
 * @param [in] bytesToSkipTo A null-terminated array of bytes to find.
 *
 * @return true The PARCBuffer's position is at the first byte matched.
 * @return false The PARCBuffer's position reached the limit.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
 *     uint8_t skipOverBytes[] = { 'l', 0 };
 *
 *     bool actual = parcBuffer_SkipTo(buffer, 1, skipOverBytes);
 *     // the buffer position will now be set to the index of the first 'l' byte in the underlying array
 *     // actual will be true
 * }
 * @endcode
 *
 * @see parcBuffer_SkipOver
 */
bool parcBuffer_SkipTo(PARCBuffer *buffer, size_t length, const uint8_t *bytesToSkipTo);

/**
 * Return the byte at the given `PARCBuffers'` current position, without modifying the position.
 *
 * @param [in] buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The byte at the given `PARCBuffers'` current position
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_AllocateCString("Hello");
 *     uint8_t byte = parcBuffer_PeekByte(1);
 *     // byte == (uint8_t) 'e';
 * }
 * @endcode
 */
uint8_t parcBuffer_PeekByte(const PARCBuffer *buffer);

/**
 * Parse an ASCII representation of a hexadecimal number in the given `PARCBuffer`
 *
 * The number may be prefixed with the characters '0', 'x'.
 * The buffer's position will be left at the first non-parsable character.
 *
 * Overflow is not checked.
 *
 * @param [in] buffer A pointer to a valid `PARCBuffer` instance.
 *
 * @return A uint64_t of the hexadecimal number.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("0x10");
 *     uint64_t value = parcBuffer_ParseHexNumber(buffer);
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
uint64_t parcBuffer_ParseHexNumber(PARCBuffer *buffer);

/**
 * Parse an ASCII representation of a unsigned decimal number in the given `PARCBuffer`
 *
 * The buffer's position will be left at the first non-parsable character.
 *
 * Overflow is not checked.
 *
 * @param [in] buffer A pointer to a valid `PARCBuffer` instance.
 *
 * @return A uint64_t of the decimal number.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("100");
 *     uint64_t value = parcBuffer_ParseDecimalNumber(buffer);
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
uint64_t parcBuffer_ParseDecimalNumber(PARCBuffer *buffer);

/**
 * Parse an ASCII representation of a unsigned decimal number or a hexadecimal number in the given `PARCBuffer`
 *
 * The buffer's position will be left at the first non-parsable character.
 *
 * Overflow is not checked.
 *
 * @param [in] buffer A pointer to a valid `PARCBuffer` instance.
 *
 * @return A uint64_t of the number.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("100");
 *     uint64_t value = parcBuffer_ParseNumeric(buffer);
 *
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
uint64_t parcBuffer_ParseNumeric(PARCBuffer *buffer);
#endif // libparc_parc_Buffer_h
