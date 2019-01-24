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
 * @file parc_BufferComposer.h
 * @ingroup memory
 * @brief An elastic memory composer of PARCBuffer instances.
 *
 * A `PARCBufferComposer` is a dynamically allocated buffer that can be used to incrementally add (append)
 * intrinsic values and/or `PARCBuffer` instance contents to a single location. It is meant to be a general
 * purpose buffer in that all native types may be added to the buffer. When finished, the user can finalize
 * the composer and produce a flipped `PARCBuffer` instance.
 *
 */
#ifndef libparc_parc_BufferComposer_h
#define libparc_parc_BufferComposer_h

struct parc_buffer_composer;
typedef struct parc_buffer_composer PARCBufferComposer;

#include <parc/algol/parc_Buffer.h>

extern parcObjectDescriptor_Declaration(PARCBufferComposer);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcBufferComposer_OptionalAssertValid(_instance_)
#else
#  define parcBufferComposer_OptionalAssertValid(_instance_) parcBufferComposer_AssertValid(_instance_)
#endif

/**
 * Create an empty (zero-length) `PARCBufferComposer`.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to the new `PARCBufferComposer`.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Create();
 *
 *     // insert contents...
 *
 *     parcBufferComposer_Release(&composer);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_Create(void);

/**
 * Create a new instance of `PARCBufferComposer` starting with an initial amount of dynamically allocated memory.
 *
 * The new buffer's position will be zero, its limit will be set to `length`, its capacity will be set to limit,
 * its mark will be undefined, and each of its elements will be initialized to zero.
 *
 * @param [in] length The number of bytes to pre-allocate.
 *
 * @return A pointer to a `PARCBufferComposer` instance
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *buffer = parcBufferComposer_Allocate(10);
 *
 *     parcBufferComposer_Release(&buffer);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_Allocate(size_t length);

/**
 * Assert that an instance of `PARCBufferComposer` is valid.
 *
 * If the instance is not valid, terminate via `parcTrapIllegalValue()`.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a PARCBufferComposer instance.
 */
void parcBufferComposer_AssertValid(const PARCBufferComposer *instance);

/**
 * This function returns a pointer to a shared `PARCBufferComposer`.
 * An implementation may choose to produce a whole copy of the original `PARCBufferComposer`,
 * or a reference counted pointer to a common copy.
 *
 * {@link parcBufferComposer_Release()} must perform the right operations to take care of a shared `PARCBufferComposer`,
 *  or simple copies.
 *
 * @param [in] original A pointer to a `PARCBufferComposer` that will be copied.
 *
 * @return A pointer to a `PARCBufferComposer`
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *buffer = parcBufferComposer_Allocate(10);
 *
 *     PARCBufferComposer *handle = parcBufferComposer_Acquire(buffer);
 *     // handle and buffer will be equal
 *
 *     parcBufferComposer_Release(&handle);
 *     parcBufferComposer_Release(&buffer);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_Acquire(const PARCBufferComposer *original);

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
 * @param [in,out] composerPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Create();
 *
 *     parcBufferComposer_Release(&composer);
 * }
 * @endcode
 */
void parcBufferComposer_Release(PARCBufferComposer **composerPtr);

/**
 * Determine if two `PARCBufferComposer` instances are equal.
 *
 * The following equivalence relations on non-null `PARCBufferComposer` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcBufferComposer_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcBufferComposer_Equals(x, y)` must return true if and only if
 *        `parcBufferComposer_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcBufferComposer_Equals(x, y)` returns true and
 *        `parcBufferComposer_Equals(y, z)` returns true,
 *        then `parcBufferComposer_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcBufferComposer_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcBufferComposer_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] x A pointer to a `PARCBufferComposer` instance.
 * @param [in] y A pointer to a `PARCBufferComposer` instance.
 *
 * @return true `PARCBufferComposer` x and y are equal.
 * @return false `PARCBufferComposer` x and y are not equal.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composerA = parcBuffer_Allocate(10);
 *     PARCBufferComposer *composerB = parcBuffer_Allocate(10);
 *
 *     if (parcBufferComposer_Equals(composerA, composerB)) {
 *         printf("Composers are equal.\n");
 *     } else {
 *          printf("Composers are NOT equal.\n");
 *     }
 *
 *     parcBufferComposer_Release(&composerA);
 *     parcBufferComposer_Release(&composerB);
 * }
 * @endcode
 */
bool parcBufferComposer_Equals(const PARCBufferComposer *x, const PARCBufferComposer *y);

/**
 * Append `length` number of bytes from the given byte array `bytes` to the given `PARCBufferComposer`.
 *
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to the PARCBufferComposer to receive the data.
 * @param [in] bytes A pointer to the array that contains the data.
 * @param [in] length The number of bytes of data to put into @p buffer.
 *
 * @return NULL Memory could not be allocated, and the buffer is unmodified.
 * @return non-NULL The value of the parameter @p buffer.
 *
 * Example:
 * @code
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(10);
 *     uint8_t string[6] = {'H', 'e', 'l', 'l', 'o', '\0'};
 *
 *     parcBufferComposer_PutArray(composer, string, strlen(string));
 *
 *     parcBufferComposer_Release(&composer);
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_PutArray(PARCBufferComposer *composer, const unsigned char *bytes, size_t length);

/**
 * Append a single char to the given `PARCBufferComposer` at the current position.
 *
 * The buffer's position will be advanced by 1 (== sizeof(char)).
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance that will receive the char value.
 * @param [in] value A single char value.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(10);
 *     parcBufferComposer_PutChar(composer, 0x12);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_PutChar(PARCBufferComposer *composer, char value);

/**
 * Append a single uint8_t to the given `PARCBufferComposer` at the current position.
 *
 * The buffer's position will be advanced by 1 (== sizeof(uint8_t)).
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance that will receive the uint8_t value.
 * @param [in] value A uint8_t value.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(10);
 *     parcBufferComposer_PutUint8(composer, 0x12);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_PutUint8(PARCBufferComposer *composer, uint8_t value);

/**
 * Append a single uint16_t to the given `PARCBufferComposer` at the current position.
 *
 * The buffer's position will be advanced by 2.
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance that will receive the uint16_t value.
 * @param [in] value A uint16_t value.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(10);
 *     parcBufferComposer_PutUint16(composer, 0x1234);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_PutUint16(PARCBufferComposer *composer, uint16_t value);

/**
 * Append a single uint16_t to the given `PARCBufferComposer` at the current position.
 *
 * The buffer's position will be advanced by 4.
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance that will receive the uint32_t value.
 * @param [in] value A `uint32_t` value.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p composer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(10);
 *     parcBufferComposer_PutUint32(composer, 0x12345678);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_PutUint32(PARCBufferComposer *composer, uint32_t value);

/**
 * Append a single uint64_t to the given `PARCBufferComposer` at the current position.
 *
 * The value is encoded in full, 8 byte, form in big-endian format, or network byte order.
 * The buffer's position will be advanced by 8.
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance that will receive the uint64_t value.
 * @param [in] value A `uint64_t` value.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p composer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(20);
 *     parcBufferComposer_PutUint32(composer, 0x0123456789ABCDEF);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_PutUint64(PARCBufferComposer *composer, uint64_t value);

/**
 * Put (append) the content of the source buffer into the destination buffer.
 *
 * The contents are taken from the current position of the source buffer
 * to its limit. The destination buffer is expanded as necessary.
 *
 * When complete, the source buffer's position is set to its limit.
 *
 * Both the input `PARCBufferComposer` and `PARCBuffer` instances are modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance.
 * @param [in] sourceBuffer The buffer that will produce the data.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p composer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(1024);
 *     PARCBuffer *buffer = parcBuffer_AllocateCString("Hello, World!");
 *     parcBufferComposer_PutBuffer(composer, buffer);
 * }
 * @endcode
 *
 * @see parcBufferComposer_GetBuffer
 */
PARCBufferComposer *parcBufferComposer_PutBuffer(PARCBufferComposer *composer, const PARCBuffer *sourceBuffer);

/**
 * Put (append) the content of the null-terminated, C-style string into the destination buffer.
 *
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance.
 * @param [in] cString A pointer to a null-terminated C string to append to this `PARCBufferComposer`.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p composer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(1024);
 *     parcBufferComposer_PutString(composer, "Hello, World!");
 *     // Hello, World!
 * }
 * @endcode
 *
 * @see parcBufferComposer_PutBuffer
 * @see parcBufferComposer_GetBuffer
 */
PARCBufferComposer *parcBufferComposer_PutString(PARCBufferComposer *composer, const char *cString);

/**
 * Put (append) the content of an arbitrary number of null-terminated, C-style strings
 * into the destination buffer.
 *
 * The input `PARCBufferComposer` instance is modified.
 *
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance.
 * @param [in] ... The null-terminated, C-style strings to append to the given `PARCBufferComposer`.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL The value of the parameter @p buffer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(1024);
 *     parcBufferComposer_PutString(composer, "Hello");
 *     parcBufferComposer_PutStrings(composer, ", ", "World", "!");
 *     // Hello, World!
 * }
 * @endcode
 *
 * @see parcBufferComposer_PutString
 * @see parcBufferComposer_PutBuffer
 * @see parcBufferComposer_GetBuffer
 */
PARCBufferComposer *parcBufferComposer_PutStrings(PARCBufferComposer *composer, ...);

/**
 * Append a formatted nul-terminated, C string string to the given `PARCBufferComposer` instance.
 * The input `PARCBufferComposer` instance is modified.
 *
 * The format string is a nul-terminated C string compatible with the `vasprintf(3)` C library function.
 *
 * @param [in,out] composer A pointer to `PARCBufferComposer`.
 * @param [in] format The format string compatible with the `vasprintf(3)` C library function.
 * @param [in] ... Remaining parameters used to format the string.
 *
 * @return The same pointer as the `composer` parameter.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(1024);
 *
 *     parcBufferComposer_Format(composer, "Hello %s\n", "World");
 *
 *     char *string = parcBuffer_ToString(parcBufferComposer_ProduceBuffer(string)));
 *     printf("String = %s\n", string);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 */
PARCBufferComposer *parcBufferComposer_Format(PARCBufferComposer *composer, const char *format, ...)
__attribute__((format(printf, 2, 3)));

/**
 * Return a pointer to the underlying {@link PARCBuffer} instance currently used by the given `PARCBufferComposer`.
 *
 * WARNING: This function is not safe. Use with caution.
 *
 *          There is no guarantee that the returned `PARCBuffer` instance will not
 *          be released and deallocated before use by the caller of this function.
 *          If modifications need to be made, a reference should be acquired manually.
 *
 *          Also, if the caller modifies the state of the returned `PARCBuffer` instance,
 *          e.g., via a {@link parcBuffer_GetUint8}() call, any future writes to this
 *          same `PARCBufferComposer` will not behave as expected unless the instance is
 *          returned to its original state.
 *
 *          To safely access the underlying `PARCBuffer` instance, use
 *          the {@link parcBufferComposer_CreateBuffer}() function instead.
 *
 * No new reference is created. The caller must acquire a reference to the returned `PARCBuffer`
 * if it needs retain it beyond the life of the given `PARCBufferComposer`.
 *
 * @param composer [in] A pointer to a `PARCBufferComposer` instance.
 *
 * @return A pointer to the internal `PARCBuffer` which is wrapped by this `PARCBufferComposer`.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(1024);
 *     PARCBuffer *buffer = parcBuffer_AllocateCString("Hello, World!");
 *     parcBufferComposer_PutBuffer(composer, buffer);
 *
 *     PARCBuffer *sameBuffer = parcBufferComposer_GetBuffer(composer);
 *     // buffer and sameBuffer are equal
 * }
 * @endcode
 *
 * @see parcBufferComposer_PutBuffer
 * @see parcBufferComposer_ProduceBuffer
 */
PARCBuffer *parcBufferComposer_GetBuffer(const PARCBufferComposer *composer);

/**
 * Create a `PARCBuffer` pointing to the underlying `PARCBuffer` instance.
 *
 * This is functionally equivalent to {@link parcBufferComposer_GetBuffer} but
 * is safe since it allocates a new `PARCBuffer` instance for the same buffer.
 *
 * The result must be freed by the caller via {@link parcBuffer_Release}.
 *
 * @param [in] composer A pointer to a `PARCBufferComposer` instance.
 *
 * @return A pointer to the `PARCBuffer` which is stored by this `PARCBufferComposer`.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(1024);
 *     PARCBuffer *buffer = parcBuffer_AllocateCString("Hello, World!");
 *     parcBufferComposer_PutBuffer(composer, buffer);
 *
 *     PARCBuffer *writeBuffer = parcBufferComposer_CreateBuffer(composer);
 *
 *     // use writeBuffer as needed
 *
 *     parcBuffer_Release(&writeBuffer);
 * }
 * @endcode
 *
 * @see parcBufferComposer_GetBuffer
 * @see parcBufferComposer_ProduceBuffer
 */
PARCBuffer *parcBufferComposer_CreateBuffer(PARCBufferComposer *composer);

/**
 * Finalize this `PARCBufferComposer` and return the resulting {@link PARCBuffer}.
 *
 * Note that, unlike {@link parcBufferComposer_GetBuffer}, the return buffer is flipped
 * via {@link parcBuffer_Flip}. This effectively finalizes this `PARCBufferComposer`.
 * No more writes should be made to this instance.
 *
 * Also note that {@link parcBufferComposer_ToString} cannot be called after this function
 * is invoked.
 *
 * The result must be freed by the caller via {@link parcBuffer_Release}.
 *
 * @param [in] composer A pointer to a `PARCBufferComposer` instance.
 *
 * @return A pointer to the final `PARCBuffer` which is stored by this `PARCBufferComposer`
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *composer = parcBufferComposer_Allocate(1024);
 *     PARCBuffer *buffer = parcBuffer_AllocateCString("Hello, World!");
 *     parcBufferComposer_PutBuffer(composer, buffer);
 *
 *     PARCBuffer *readBuffer = parcBufferComposer_ProduceBuffer(composer);
 *     parcBufferComposer_Release(&composer);
 *
 *     // use readBuffer as needed
 *
 *     parcBuffer_Release(&readBuffer);
 * }
 * @endcode
 *
 * @see parcBufferComposer_GetBuffer
 */
PARCBuffer *parcBufferComposer_ProduceBuffer(PARCBufferComposer *composer);

/**
 * Produce a null-terminated string containing the characters from 0 to the current
 * position of the given `PARCBufferComposer`.
 * The composer is not modified and may continue to be used.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCBufferComposer instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *a = parcBufferComposer_Create();
 *
 *     char *string = parcBufferComposer_ToString(a);
 *
 *     parcBufferComposer_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 */
char *parcBufferComposer_ToString(PARCBufferComposer *composer);
#endif // libparc_parc_BufferComposer_h
