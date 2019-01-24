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
 * @file parc_ByteArray.h
 * @ingroup memory
 * @brief A simple reference counted unsigned byte array.
 *
 * @htmlonly
 * <svg xmlns="http://www.w3.org/2000/svg" xmlns:xl="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" version="1.1" viewBox="48 74 304 68" width="304pt" height="68pt">
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
 * <text transform="translate(271 88.5)" fill="black">
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x=".185" y="10" textLength="39.63">Capacity</tspan>
 * </text>
 * <text transform="translate(83 88.5)" fill="black">
 * <tspan font-family="Helvetica Neue" font-size="10" font-weight="500" x=".245" y="10" textLength="23.51">Array</tspan>
 * </text>
 * <path d="M 78 96.483333 C 73.6671 96.98884 67.757544 94.49623 65 98 C 63.49089 99.917494 62.925312 103.63153 62.52875 107.66717" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <rect x="59" y="118" width="281" height="13" fill="white"/>
 * <rect x="59" y="118" width="281" height="13" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * <path d="M 316 97.11628 C 321.9994 97.744123 330.42197 95.601626 334 99 C 335.8631 100.769544 336.41326 104.04195 336.67595 107.643264" marker-end="url(#FilledArrow_Marker)" stroke="black" stroke-linecap="round" stroke-linejoin="round" stroke-width="1"/>
 * </g>
 * </g>
 * </svg>
 * @endhtmlonly
 *
 * `PARCByteArray` is a simple reference counted array of `uint8_t` values.
 * Instances of `PARCByteArray` are created either by dynamically allocating the byte array,
 * via `parcByteArray_Allocate()`,
 * or by wrapping a static `uint8_t` array,
 * via `parcByteArray_Wrap()`.
 *
 * New references to an existing instance of `PARCByteArray` are created via `parcByteArray_Acquire()`.
 *
 * A `PARCByteArray` reference is released via `parcByteArray_Release`.
 * Only the last invocation will deallocated the `PARCByteArray`.
 * If the `PARCByteArray` references dynamically allocated memory,
 * that memory is freed with the `PARCByteArray` when the last reference is released.
 *
 */
#ifndef libparc_parc_ByteArray_h
#define libparc_parc_ByteArray_h
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

struct parc_byte_array;
/**
 *
 * @see {@link parcByteArray_Allocate}
 * @see {@link parcByteArray_Wrap}
 */
typedef struct parc_byte_array PARCByteArray;

#include <parc/algol/parc_HashCode.h>

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcByteArray_OptionalAssertValid(_instance_)
#else
#  define parcByteArray_OptionalAssertValid(_instance_) parcByteArray_AssertValid(_instance_)
#endif

/**
 * Assert that an instance of `PARCByteArray` is valid.
 *
 * If the instance is not valid, terminate via {@link parcTrapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a `PARCByteArray` instance.
 */
void parcByteArray_AssertValid(const PARCByteArray *instance);

/**
 * Determine if an instance of `PARCByteArray` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a `PARCByteArray` instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 */
bool parcByteArray_IsValid(const PARCByteArray *instance);

/**
 * Dynamically allocate a `PARCByteArray` of a specific capacity.
 *
 * @param [in] capacity The number of bytes in the byte array.
 *
 * @return A pointer to an allocated `PARCByteArray` instance which must be released via {@link parcByteArray_Release()}.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *byteArray = parcByteArray_Allocate(100);
 *
 *     parcByteArray_Release(&byteArray);
 * }
 * @endcode
 *
 * @see parcByteArray_Release
 */
PARCByteArray *parcByteArray_Allocate(const size_t capacity);

/**
 * Wrap existing memory in a {@link PARCByteArray}.
 *
 * As in all `Wrap` functions, a copy of the memory is not made.
 * Be sure to only wrap memory that is either global,
 * or used within on stack-frame or functional block.
 * Otherwise, corruption is likly to occur.
 *
 * @param [in] capacity The maximum capacity of the backing array.
 * @param [in] array A pointer to the backing array.
 *
 * @return A pointer to an allocated `PARCByteArray` instance which must be released via {@link parcByteArray_Release()}.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *
 *     parcByteArray_Release(&byteArray);
 * }
 * @endcode
 */
PARCByteArray *parcByteArray_Wrap(size_t capacity, uint8_t *array);

/**
 * Returns the pointer to the `uint8_t` array that backs this `PARCByteArray`.
 *
 * Modifications to the given `PARCByteArray` content will cause the returned array's content to be modified, and vice versa.
 *
 * The reference count for the given `PARCByteArray` is not modified.
 *
 * <b>Use with caution.</b>
 *
 * @param [in] byteArray Pointer to a `PARCByteArray` instance from which the underlying array is extracted.
 *
 * @return The pointer to the `uint8_t` array that backs this `PARCByteArray`.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *
 *     uint8_t *raw = parcByteArray_Array(byteArray);
 *     // updates on the raw array (pointer) are seen in the byteArray instance
 *     // use with caution
 *
 *     parcByteArray_Release(&byteArray);
 * }
 * @endcode
 */
uint8_t *parcByteArray_Array(const PARCByteArray *byteArray);

/**
 * Increase the number of references to a `PARCByteArray`.
 *
 * Note that a new `PARCByteArray` is not created,
 * only that the given `PARCByteArray` reference count is incremented.
 * Discard the reference by invoking {@link parcByteArray_Release}.
 *
 * @param [in] instance A pointer to the original `PARCByteArray` instance.
 *
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *x = parcByteArray_Allocate(10);
 *
 *     PARCByteArray *x_2 = parcByteArray_Acquire(x);
 *
 *     parcByteArray_Release(&x);
 *     parcByteArray_Release(&x_2);
 * }
 * @endcode
 *
 * @see parcByteArray_Release
 */
PARCByteArray *parcByteArray_Acquire(const PARCByteArray *instance);

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
 * @param [in,out] byteArrayPtr A pointer to a pointer to the instance of `PARCByteArray` to release.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *x = parcByteArray_Acquire(...);
 *
 *     parcByteArray_Release(&x);
 * }
 * @endcode
 */
void parcByteArray_Release(PARCByteArray **byteArrayPtr);

/**
 * Put an `uint8_t` value into the byte array at the given index.
 *
 * The value of @p index must be greater than or equal to 0, and less than the capacity.
 *
 * @param [in,out] result A pointer to the instance of `PARCByteArray` that will receive the data.
 * @param [in] index The index at which the byte will be inserted.
 * @param [in] byte The byte to be inserted into the array.
 *
 * @return The `PARCByteArray` that receive the data.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *
 *     parcByteArray_PutByte(byteArray, 5, 123);
 *
 *     parcByteArray_Release(&byteArray);
 * }
 * @endcode
 */
PARCByteArray *parcByteArray_PutByte(PARCByteArray *result, size_t index, uint8_t byte);

/**
 * Get the value at a specific index from the given `PARCByteArray`.
 *
 * @param [in] result The instance of `PARCByteArray` that will produce the data.
 * @param [in] index The index from which the byte will be retrieved.
 *
 * @return The value at a specific index from the given `PARCByteArray`.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *
 *     parcByteArray_PutByte(byteArray, 5, 123);
 *
 *     parcByteArray_GetByte(byteArray, 5);
 *
 *     parcByteArray_Release(&byteArray);
 * }
 * @endcode
 */
uint8_t parcByteArray_GetByte(const PARCByteArray *result, size_t index);

/**
 * Compares instance a with instance b for order.
 *
 * Returns a negative integer, zero, or a positive integer as instance a is less than, equal to, or greater than instance b.
 *
 * @param [in] a A pointer to a `PARCByteArray` instance 'a'.
 * @param [in] b A pointer to another `PARCByteArray` instance 'b'.
 *
 * @return <0 Instance a is less then instance b.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray1 = parcByteArray_Wrap(array, 10);
 *     PARCByteArray *byteArray2 = parcByteArray_Wrap(array, 10);
 *
 *     if (parcByteArray_Compare(byteArray1, byteArray2) == 0) {
 *         printf("Equal\n");
 *     }
 *
 *     parcByteArray_Release(&byteArray1);
 *     parcByteArray_Release(&byteArray2);
 * }
 * @endcode
 *
 * @see parcByteArray_Equal
 */
int parcByteArray_Compare(const PARCByteArray *a, const PARCByteArray *b);

/**
 * Copy data from an external array into a `PARCByteArray`.
 *
 * Provided that the underlying `PARCByteArray` is large enough,
 * copy the bytes from the given `array` for `length` bytes.
 *
 * @param [in,out] result The `PARCByteArray` that will receive the data.
 * @param [in] offset The offset into the `PARCByteArray` that will receive the data.
 * @param [in] length The number of bytes to copy in.
 * @param [in] source The uint8_t array containing the original data.
 *
 * @return The given `PARCByteArray` pointer
 *
 * @throws SIGABRT `parcTrapOutOfBounds` if the underlying `PARCByteArray` is not large enough
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *     uint8_t inner[3];
 *     inner[0] = 0xFF;
 *     inner[1] = 0xFF;
 *     inner[2] = 0xFF;
 *
 *     parcByteArray_PutBytes(byteArray, 1, 3, inner);
 *     // Bytes 1,2,3 of byteArray will be 0xFF (offset was 1, not 0)
 *
 *     parcByteArray_Release(&byteArray);
 * }
 * @endcode
 */
PARCByteArray *parcByteArray_PutBytes(PARCByteArray *result, size_t offset, size_t length, const uint8_t *source);

/**
 * Copy data from a `PARCByteArray` into an external array.
 *
 * Provided that the underlying `PARCByteArray` has at least `length` bytes
 * copy the bytes from the `PARCByteArray` into the given `array`.
 *
 * @param [in] source The `PARCByteArray` that will produce the data.
 * @param [in] offset The offset into the `PARCByteArray` from which the data will be copied.
 * @param [in] length The number of bytes to copy from the <code>PARCByteArray</code> into @p destination.
 * @param [in] destination The `uint8_t` array That will contain the data.
 *
 * @return The given `PARCByteArray` @p source pointer.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *
 *     uint8_t inner[3];
 *     parcByteArray_GetBytes(byteArray, 0, 3, inner);
 *     // inner will contain a copy of bytes 0,1,2 from the byteArray
 *
 *     parcByteArray_Release(&byteArray);
 * }
 * @endcode
 */
PARCByteArray *parcByteArray_GetBytes(const PARCByteArray *source, size_t offset, size_t length, uint8_t *destination);


/**
 * Copy a portion of one `PARCByteArray` into another.
 *
 * The sum of (destination/source) offset and length must be within the bounds of the
 * respective (destination/source) `PARCByteArray` instances.
 *
 * @param [in,out] destination Pointer to the destination `PARCByteArray` instance.
 * @param [in] destOffset Offset within the destination `PARCByteArray` instance.
 * @param [in] source Pointer to the source `PARCByteArray` instance.
 * @param [in] srcOffset Offset within the source `PARCByteArray` instance.
 * @param [in] length Number of bytes to copy from the source to the destination.
 *
 * @return PARCByteArray* A pointer to the destination.
 *
 * Example:
 * @code
 * {
 *     uint8_t array1[10];
 *     PARCByteArray *byteArray1 = parcByteArray_Wrap(array1, 10);
 *     uint8_t array2[3];
 *     array2[0] = 0xAA;
 *     array2[1] = 0xBB;
 *     array2[2] = 0xCC;
 *     PARCByteArray *byteArray2 = parcByteArray_Wrap(array2, 3);
 *
 *     parcByteArray_ArrayCopy(byteArray1, 0, byteArray2, 0, 3);
 *     // the first three bytes of byteArray1 will now be 0xAA,0xBB,0xCC
 * }
 * @endcode
 */
PARCByteArray *parcByteArray_ArrayCopy(PARCByteArray *destination, size_t destOffset, const PARCByteArray *source, size_t srcOffset, size_t length);

/**
 * Get the capacity of a `PARCByteArray`.
 *
 * The `PARCByteArray`'s capacity is the number of bytes stored in the backing store of a `PARCByteArray`.
 *
 * @param [in] byteArray Pointer to the `PARCByteArray` instance being examined.
 *
 * @return The capacity of a `PARCByteArray`.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *
 *     size_t capacity = parcByteArray_Capacity(byteArray);
 * }
 * @endcode
 */
size_t parcByteArray_Capacity(const PARCByteArray *byteArray);

/**
 * Create a copy of an existing `PARCByteArray`.
 *
 * The copy is equal to, but shares nothing in common with, the original `PARCByteArray`
 *
 * @param [in] original A non-null pointer to the `PARCByteArray` to copy.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to the new `PARCByteArray` instance.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray = parcByteArray_Wrap(array, 10);
 *     PARCByteArray *copy = parcByteArray_Copy(byteArray);
 * }
 * @endcode
 *
 * @see parcByteArray_Allocate
 * @see parcByteArray_Wrap
 */
PARCByteArray *parcByteArray_Copy(const PARCByteArray *original);

/**
 * Determine if two `PARCByteArray` instances are equal.
 *
 * Two `PARCByteArray` instances are equal if, and only if,
 * * They have the same number of elements, and
 * * The two sequences of remaining elements are pointwise equal.
 *
 * The following equivalence relations on non-null `PARCByteArray` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcByteArray_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcByteArray_Equals(x, y)` must return true if and only if
 *        `parcByteArray_Equals(y, x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcByteArray_Equals(x, y)` returns true and
 *        `parcByteArray_Equals(y, z)` returns true,
 *        then  `parcByteArray_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcByteArray_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcByteArray_Equals(x, NULL)` must return false.
 *
 * @param [in] a A pointer to a `PARCByteArray` instance.
 * @param [in] b A pointer to a `PARCByteArray` instance.
 *
 * @return true if the two `PARCByteArray` instances are equal.
 *
 * Example:
 * @code
 * {
 *     uint8_t array[10];
 *     PARCByteArray *byteArray1 = parcByteArray_Wrap(array, 10);
 *     PARCByteArray *byteArray2 = parcByteArray_Wrap(array, 10);
 *
 *     if (parcByteArray_Equals(byteArray1, byteArray2)) {
 *         printf("Equal\n");
 *     } else {
 *         printf("NOT Equal\n");
 *     }
 *
 *     parcByteArray_Release(&byteArray1);
 *     parcByteArray_Release(&byteArray2);
 * }
 * @endcode
 */
bool parcByteArray_Equals(const PARCByteArray *a, const PARCByteArray *b);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the {@link parcByteArray_HashCode} function must consistently return the same value,
 * provided no information used in a corresponding {@link parcByteArray_Equals}
 * comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link parcByteArray_Equals} method,
 * then calling the {@link parcByteArray_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the {@link parcByteArray_Equals} function,
 * then calling the {@link parcByteArray_HashCode}
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] array A pointer to the `PARCByteArray` instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *array = parcByteArray_Allocate(10);
 *     uint32_t hashValue = parcByteArray_HashCode(array);
 *     parcByteArray_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcByteArray_HashCode
 */
PARCHashCode parcByteArray_HashCode(const PARCByteArray *array);

/**
 * Return the memory address as a `uint8_t *` to the location specified by `index`.
 *
 * @param [in] array A pointer to the `PARCByteArray` instance.
 * @param [in] index The index of the address.
 *
 * @return A pointer to the location offset by index bytes within the array.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *byteArray = parcByteArray_Allocate(10);
 *     uint8_t *offsetAddress = parcByteArray_AddressOfIndex(byteArray, 4);
 *     // offsetAddress now points to the array at offset 4 within byteArray
 * }
 * @endcode
 */
uint8_t *parcByteArray_AddressOfIndex(const PARCByteArray *array, size_t index);

/**
 * Pretty print the given `PARCByteArray` instance.
 *
 * @param [in] indentation The amount of indentation to prefix each line of output
 * @param [in] array The `PARCByteArray` instance to be printed.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *byteArray = parcByteArray_Allocate(10);
 *     parcByteArray_Display(byteArray, 0);
 * }
 * @endcode
 */
void parcByteArray_Display(const PARCByteArray *array, int indentation);
#endif // libparc_parc_ByteArray_h
