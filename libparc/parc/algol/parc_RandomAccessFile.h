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
 * @file parc_RandomAccessFile.h
 * @ingroup inputoutput
 * @brief A wrapper that provides random access to a file.
 *
 */
#ifndef PARCLibrary_RandomAccessFile
#define PARCLibrary_RandomAccessFile
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_File.h>

struct PARCRandomAccessFile;
typedef struct PARCRandomAccessFile PARCRandomAccessFile;

typedef enum {
    PARCRandomAccessFilePosition_Start,
    PARCRandomAccessFilePosition_End,
    PARCRandomAccessFilePosition_Current
} PARCRandomAccessFilePosition;

/**
 * Increase the number of references to a `PARCRandomAccessFile` instance.
 *
 * Note that new `PARCRandomAccessFile` is not created,
 * only that the given `PARCRandomAccessFile` reference count is incremented.
 * Discard the reference by invoking `parcRandomAccessFile_Release`.
 *
 * @param [in] instance A pointer to a valid PARCRandomAccessFile instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *
 *     PARCRandomAccessFile *b = parcRandomAccessFile_Acquire();
 *
 *     parcRandomAccessFile_Release(&a);
 *     parcRandomAccessFile_Release(&b);
 * }
 * @endcode
 */
PARCRandomAccessFile *parcRandomAccessFile_Acquire(const PARCRandomAccessFile *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcRandomAccessFile_OptionalAssertValid(_instance_)
#else
#  define parcRandomAccessFile_OptionalAssertValid(_instance_) parcRandomAccessFile_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCRandomAccessFile` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCRandomAccessFile instance.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *
 *     parcRandomAccessFile_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcRandomAccessFile_Release(&b);
 * }
 * @endcode
 */
void parcRandomAccessFile_AssertValid(const PARCRandomAccessFile *instance);

/**
 * Print a human readable representation of the given `PARCRandomAccessFile`.
 *
 * @param [in] instance A pointer to a valid PARCRandomAccessFile instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *
 *     parcRandomAccessFile_Display(a, 0);
 *
 *     parcRandomAccessFile_Release(&a);
 * }
 * @endcode
 */
void parcRandomAccessFile_Display(const PARCRandomAccessFile *instance, int indentation);

/**
 * Determine if two `PARCRandomAccessFile` instances are equal.
 *
 * The following equivalence relations on non-null `PARCRandomAccessFile` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcRandomAccessFile_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcRandomAccessFile_Equals(x, y)` must return true if and only if
 *        `parcRandomAccessFile_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcRandomAccessFile_Equals(x, y)` returns true and
 *        `parcRandomAccessFile_Equals(y, z)` returns true,
 *        then `parcRandomAccessFile_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcRandomAccessFile_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcRandomAccessFile_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCRandomAccessFile instance.
 * @param [in] y A pointer to a valid PARCRandomAccessFile instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *     PARCRandomAccessFile *b = parcRandomAccessFile_Open(..);
 *
 *     if (parcRandomAccessFile_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcRandomAccessFile_Release(&a);
 *     parcRandomAccessFile_Release(&b);
 * }
 * @endcode
 * @see parcRandomAccessFile_HashCode
 */
bool parcRandomAccessFile_Equals(const PARCRandomAccessFile *x, const PARCRandomAccessFile *y);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the `HashCode` function must consistently return the same value,
 * provided no information used in a corresponding comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link parcRandomAccessFile_Equals} method,
 * then calling the {@link parcRandomAccessFile_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcRandomAccessFile_Equals} function,
 * then calling the `parcRandomAccessFile_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCRandomAccessFile instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open..();
 *
 *     PARCHashCode hashValue = parcRandomAccessFile_HashCode(buffer);
 *     parcRandomAccessFile_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcRandomAccessFile_HashCode(const PARCRandomAccessFile *instance);

/**
 * Determine if an instance of `PARCRandomAccessFile` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCRandomAccessFile instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *
 *     if (parcRandomAccessFile_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcRandomAccessFile_Release(&a);
 * }
 * @endcode
 *
 */
bool parcRandomAccessFile_IsValid(const PARCRandomAccessFile *instance);

/**
 * Open a new `PARCRandomAccessFile` instance.
 *
 * @param [in] file A `PARCFile` which refers to the file to open for random access.
 *
 * @retval `PARCRandomAccessFile` A new instance
 * @retval NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCFile *file = parcFile_Create("/tmp/file_chunker.tmp");
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(file);
 *
 *     if (parcRandomAccessFile_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcRandomAccessFile_Release(&a);
 * }
 * @endcode
 *
 */
PARCRandomAccessFile *parcRandomAccessFile_Open(PARCFile *file);

/**
 * Release a previously acquired reference to the given `PARCRandomAccessFile` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *
 *     parcRandomAccessFile_Release(&a);
 * }
 * @endcode
 */
void parcRandomAccessFile_Release(PARCRandomAccessFile **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCRandomAccessFile instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *
 *     PARCJSON *json = parcRandomAccessFile_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcRandomAccessFile_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcRandomAccessFile_ToJSON(const PARCRandomAccessFile *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCRandomAccessFile`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCRandomAccessFile instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCRandomAccessFile *a = parcRandomAccessFile_Open(..);
 *
 *     char *string = parcRandomAccessFile_ToString(a);
 *
 *     parcRandomAccessFile_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcRandomAccessFile_Display
 */
char *parcRandomAccessFile_ToString(const PARCRandomAccessFile *instance);

/**
 * Close a `PARCRandomAccessFile` instance.
 *
 * @param [in] fileHandle A `PARCRandomAccessFile.`
 *
 * @retval true The file was closed successfully.
 * @retval false An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCFile *file = parcFile_Create("/tmp/file_chunker.tmp");
 *     PARCRandomAccessFile *handle = parcRandomAccessFile_Open(file);
 *
 *     // use the handle
 *
 *     parcRandomAccessFile_Close(handle);
 *     parcRandomAccessFile_Release(&handle);
 * }
 * @endcode
 *
 * @see parcRandomAccessFile_Open
 */
bool parcRandomAccessFile_Close(PARCRandomAccessFile *fileHandle);

/**
 * Read bytes into the provided `PARCBuffer` until the buffer limit is or the source EOF
 * is reached.
 *
 * @param [in] fileHandle A `PARCRandomAccessFile` from which to read.
 * @param [in,out] buffer A `PARCBuffer` into which data is read.
 *
 * @return The number of bytes read.
 *
 * Example:
 * @code
 * {
 *     PARCFile *file = parcFile_Create("/tmp/file_chunker.tmp");
 *     PARCRandomAccessFile *handle = parcRandomAccessFile_Open(file);
 *
 *     PARCBuffer *buffer = parcBuffer_Allocate(1024);
 *     size_t numBytes = parcRandomAccessFile_Read(handle, buffer);
 *
 *     // use the data in `buffer`
 *
 *     parcRandomAccessFile_Close(handle);
 *     parcRandomAccessFile_Release(&handle);
 * }
 * @endcode
 *
 * @see parcRandomAccessFile_Write
 */
size_t parcRandomAccessFile_Read(PARCRandomAccessFile *fileHandle, PARCBuffer *buffer);

/**
 * Write bytes from the provided `PARCBuffer` to the source file until the limit is reached.
 *
 * @param [in] fileHandle A `PARCRandomAccessFile` to which data is written.
 * @param [in,out] buffer A `PARCBuffer` into which data is read.
 *
 * @return The number of bytes written.
 *
 * Example:
 * @code
 * {
 *     PARCFile *file = parcFile_Create("/tmp/file_chunker.tmp");
 *     PARCRandomAccessFile *handle = parcRandomAccessFile_Open(file);
 *
 *     PARCBuffer *buffer = parcBuffer_WrapCString("important data to go in the file");
 *     size_t numBytes = parcRandomAccessFile_Write(handle, buffer);
 *
 *     // continue writing to the file if needed
 *
 *     parcRandomAccessFile_Close(handle);
 *     parcRandomAccessFile_Release(&handle);
 * }
 * @endcode
 *
 * @see parcRandomAccessFile_Read
 */
size_t parcRandomAccessFile_Write(PARCRandomAccessFile *fileHandle, PARCBuffer *buffer);

/**
 * Seek to the position in the file specified as an offset from the position.
 *
 * The position can be one of:
 *     PARCRandomAccessFilePosition_Start
 *     PARCRandomAccessFilePosition_End
 *     PARCRandomAccessFilePosition_Current
 *
 * @param [in] fileHandle A `PARCRandomAccessFile` to which data is written.
 * @param [in] offset The number of bytes to offset from the provided position.
 * @param [in] position The base posititon from which the offset is calculated.
 *
 * @return The number of bytes seeked.
 *
 * Example:
 * @code
 * {
 *     PARCFile *file = parcFile_Create("/tmp/file_chunker.tmp");
 *     PARCRandomAccessFile *handle = parcRandomAccessFile_Open(file);
 *
 *     size_t fileSize = parcFile_GetFileSize(file);
 *     parcRandomAccessFile_Seek(handle, -1, PARCRandomAccessFilePosition_End);
 *
 *     // use the last byte of the file
 *
 *     parcRandomAccessFile_Close(handle);
 *     parcRandomAccessFile_Release(&handle);
 * }
 * @endcode
 *
 * @see parcRandomAccessFile_Read
 */
size_t parcRandomAccessFile_Seek(PARCRandomAccessFile *fileHandle, long offset, PARCRandomAccessFilePosition position);
#endif
