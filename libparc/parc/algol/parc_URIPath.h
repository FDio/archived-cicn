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
 * @file parc_URIPath.h
 * @ingroup networking
 * @brief A Universal Resource Identifier (URI) Path
 *
 */
#ifndef libparc_PARCURIPath_h
#define libparc_PARCURIPath_h

#include <stdlib.h>
#include <stdarg.h>

#include <parc/algol/parc_URISegment.h>

struct parc_uri_path;
typedef struct parc_uri_path PARCURIPath;

/**
 * Create a new `PARCURIPath` object.
 *
 * The new `PARCURIPath` object is empty.
 *
 * @return A pointer to a `PARCURIPath` instance.
 *
 * Example:
 * @code
 * {
 *     PARCURIPath *path = parcURIPath_Create();
 *     ...
 *     parcURIPath_Release(&path);
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_Create(void);

/**
 * Increase the number of references to a `PARCURIPath` instance.
 *
 * Note that new `PARCURIPath` is not created,
 * only that the given `PARCURIPath` reference count is incremented.
 * Discard the reference by invoking {@link parcURIPath_Release}.
 *
 * @param auth A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCURIPath *x = parcURIPath_Create();
 *
 *     PARCURIPath *x2 = parcURIPath_Acquire(x);
 *
 *     parcURIPath_Release(&x);
 *     parcURIPath_Release(&x2);
 * }
 * @endcode
 *
 * @see parcURIPath_Release
 */
PARCURIPath *parcURIPath_Acquire(const PARCURIPath *auth);

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
 * @param [in,out] pathPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCURIPath *path = parcURIPath_Create();
 *
 *     parcURIPath_Release(&auth);
 * }
 * @endcode
 */
void parcURIPath_Release(PARCURIPath **pathPtr);

/**
 * Parse a complete URI path composed of URI segments.
 *
 *   The parsed path is expected to conform to the syntax of '/' segment ['/' segment]
 *   terminated by either a null, '?', or '#' character.
 *
 * @param [in] string A pointer to character array containing the first '/' of the path.
 * @param [in] pointer A pointer to character pointer that will point to the first character that is not the path.
 *
 * @return A newly allocated `PARCURIPath` instance that must be freed via {@link parcURIPath_Release()}
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/bar/, &pointer);
 *     // use the complete URI path
 *     parcURIPath_Release(&path);
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_Parse(const char *string, const char **pointer);

/**
 * Compares two `PARCURIPath` instances for order.
 *
 * As strings, URI paths are compared in normal lexographical order. This
 * is analogous to strcmp(...).
 *
 * @param [in] pathA A `PARCURIPath` pointer, or NULL.
 * @param [in] pathB A `PARCURIPath` pointer, or NULL.
 *
 * @return A negative integer, zero, or a positive integer as a is less than, equal to, or greater than b, accordingly.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *pathA = parcURIPath_Parse("lci:/foo/bar/, &pointer);
 *     PARCURIPath *pathB = parcURIPath_Parse("lci:/foo/bar/, &pointer);
 *     int cmp = parcURIPath_Compare(pathA, pathB);
 *     // cmp will be zero since both paths are the same
 * }
 * @endcode
 */
int parcURIPath_Compare(const PARCURIPath *pathA, const PARCURIPath *pathB);

/**
 * Create a new `PARCURIPath` comprised of a basePath concatenated with zero or more `PARCURISegment` instances.
 *
 *   Create a new `PARCURIPath` instance comprised of the given `PARCURIPath`
 *   concatenated with the null terminated list of {@link PARCURISegment} instances.
 *
 * @param [in] basePath The base prefix path used to compose a new path
 * @param [in] ... Any additional `PARCURISegment` instances that will be appended to the URI path.
 *
 * @return A newly allocated `PARCURIPath` instance
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *basePath = parcURIPath_Parse("lci:/foo/, &pointer);
 *     PARCURISegment *segment = parcURISegment_Create(3, "bar");
 *     PARCURIPath *path = parcURIPath_Compose(basePath, segment);
 *
 *     // use the new composed path as needed
 *
 *     parcURIPath_Release(&path);
 *     parcURIPath_Release(&basePath);
 *     parcURISegment_Destroy(&segment);
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_Compose(const PARCURIPath *basePath, ...);

/**
 * Create a new `PARCURIPath` comprised of a basePath concatenated with the number of given `PARCURISegment`
 * instances provided by @p varargs.
 *
 * The variable number of `PARCURISegment` instances is signaled by the value `NULL` as the last element in @p varargs.
 *
 * @param [in] basePath The base prefix path used to compose a new path
 * @param [in] varargs A valid va_list.
 *
 * @return A newly allocated `PARCURIPath` instance
 *
 * Example:
 * @code
 * PARCURIPath *
 * myFunction(const PARCURIPath *basePath, ...)
 * {
 *     va_list arglist;
 *     va_start(arglist, basePath);
 *
 *     PARCURIPath *result = parcURIPath_ComposeValist(basePath, arglist);
 *     va_end(arglist);
 *
 *     return result;
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_ComposeValist(const PARCURIPath *basePath, va_list varargs);

/**
 * Concatenate two paths together to form a single path.
 *
 *   Concatenating "/a/b/" and "/c/d" URI paths will yield: "/a/b/c/d/".
 *
 * @param [in] pathA Pointer to the first path (prefix)
 * @param [in] pathB Pointer to the second path (suffix)
 *
 * @return A `PARCURIPath` instance containing the concatenation of pathA and pathB, must be freed via {@link parcURIPath_Release()}
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *prefix = parcURIPath_Parse("lci:/foo/, &pointer);
 *     PARCURIPath *suffix = parcURIPath_Parse("/bar/", &pointer);
 *     PARCURIPath *concat = parcURIPath_Concat(prefix, suffix);
 *
 *     // use the new path as needed
 *
 *     parcURIPath_Release(&prefix);
 *     parcURIPath_Release(&suffix);
 *     parcURIPath_Release(&concat);
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_Concat(PARCURIPath *pathA, PARCURIPath *pathB);

/**
 * Determine if two `PARCURIPath` instances are equal.
 *
 *   This function implements the following equivalence relations on non-null `PARCURIPath` instances:
 *
 *   * It is reflexive: for any non-null reference value x, `parcURIPath_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcURIPath_Equals(x, y)` must return true if and only if
 *        `parcURIPath_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcURIPath_Equals(x, y)` returns true and
 *        `parcURIPath_Equals(y, z)` returns true,
 *        then  `parcURIPath_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcURIPath_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcURIPath_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] pathA First instance
 * @param [in] pathB Second instance
 *
 * @return true Equal `PARCURIPath` instances
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *pathA = parcURIPath_Parse("lci:/foo/, &pointer);
 *     PARCURIPath *pathB = parcURIPath_Parse("lci:/foo/, &pointer);
 *
 *     if (parcURIPath_Equals(pathA, pathB) {
 *         printf("Paths are equal\n");
 *     }
 *
 *    parcURIPath_Release(&pathA);
 *    parcURIPath_Release(&pathB);
 * }
 * @endcode
 */
bool parcURIPath_Equals(const PARCURIPath *pathA, const PARCURIPath *pathB);

/**
 * Create a copy of the given `PARCURIPath`.
 *
 *   This is a deep copy of the instance.
 *
 * @param [in] path The path to copy.
 *
 * @return A copy of the given `PARCURIPath`.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/, &pointer);
 *     PARCURIPath *copy = parcURIPath_Copy(path);
 *
 *     // use the copy as needed
 *
 *     parcURIPath_Release(&path);
 *     parcURIPath_Release(&copy);
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_Copy(const PARCURIPath *path);

/**
 * Append a path segment to an existing URI path.
 *
 *   Appending "bar" to "lci:/foo" will yield "lci:/foo/bar".
 *   This modifies the URI path instance in place - it does not allocate
 *   a new instance.
 *
 * @param [in,out] path The `PARCURIPath` instance to which the segment is appended
 * @param [in] segment The {@link PARCURISegment} to append to the path
 *
 * @return The modified `PARCURIPath` instance (equal to the first parameter).
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/, &pointer);
 *     PARCURISegment *segment = parcURISegment_Create(3, "bar");
 *     path = parcURIPath_Append(path, segment);
 *
 *     // use the full path as necessary
 *
 *     parcURIPath_Release(&path);
 *     parcURISegment_Destroy(&segment);
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_Append(PARCURIPath *path, const PARCURISegment *segment);

/**
 * Retrieve the URI path segment at the specified index.
 *
 *   The index must be within the range [0, number of segments]
 *   prior to invocation.  Otherwise, the program is terminated with parcTrapOutOfBounds.
 *
 * @param [in] path A `PARCURIPath` instance to be examined.
 * @param [in] index The index of the URI segment to retrieve.
 *
 * @return The {@link PARCURISegment} instance at the specified index.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/, &pointer);
 *     PARCURISegment *segment = parcURIPath_Get(path, 0);
 *     // segment will be equal to "lci:"
 *
 *     ...
 *
 *    parcURIPath_Release(&path);
 *    parcURISegment_Destroy(&segment);
 * }
 * @endcode
 */
PARCURISegment *parcURIPath_Get(const PARCURIPath *path, size_t index);

/**
 * Return the number of segments in the given Path.
 *
 * @param [in] path The `PARCURIPath` instance to be examined.
 *
 * @return The integer length of the path, in segments.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/, &pointer);
 *     size_t numSegments = parcURIPath_Count(path);
 *     ...
 *     parcURIPath_Release(&path);
 * }
 * @endcode
 */
size_t parcURIPath_Count(const PARCURIPath *path);

/**
 * Produce a null-terminated C-string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] path A pointer to the instance.
 *
 * @return NULL  Memory could not be allocated.
 * @return non-NULL A null-terminated string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/, &pointer);
 *
 *     char *stringRep = parcURIPath_ToString(path);
 *     printf("Path: %s\n", stringRep);
 *
 *     parcURIPath_Release(&path);
 * }
 * @endcode
 */
char *parcURIPath_ToString(const PARCURIPath *path);

/**
 * Remove N trailing segments from the given Path.
 *
 * @param [in,out] path The `PARCURIPath` instance being modified.
 * @param [in] numberToRemove The number of segments to remove from the end.
 *
 * @return `PARCURIPath` The given `PARCURIPath` that has been modified in place
 * @return NULL If @p numberToRemove is too large.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/, &pointer);
 *
 *     ...
 *     path = parcURIPath_Trim(path, 1); // leaves "lci:/"
 *     ...
 *
 *     parcURIPath_Release(&path);
 * }
 * @endcode
 */
PARCURIPath *parcURIPath_Trim(PARCURIPath *path, size_t numberToRemove);

/**
 * Build a string representation of the `PARCURIPath` stored in a {@link PARCBufferComposer} instance.
 *
 * @param [in] path The `PARCURIPath` instance from which the string representation is made.
 * @param [in,out] composer The `PARCBufferComposer` which is modified in place with the string representation.
 *
 * @return `PARCBufferComposer` The modified `PARCBufferComposer` that was passed in.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("/foo/bar/, &pointer);
 *     PARCBufferComposer *composer = parcBufferComposer_Create();
 *
 *     parcURIPath_BuildString(path, composer);
 *
 *     PARCBuffer *string = parcBufferComposer_ProducerBuffer(composer);
 *     printf("URI: %s\n", parcBuffer_ToString(string));
 *     parcBuffer_Release(&string);
 * }
 * @endcode
 */
PARCBufferComposer *parcURIPath_BuildString(const PARCURIPath *path, PARCBufferComposer *composer);

/**
 * Determine if a `PARCURIPath` begins with the specified URI prefix.
 *
 * @param [in] base The `PARCURIPath` instance which is being checked.
 * @param [in] prefix The `PARCURIPath` prefix used to check as the prefix.
 *
 * @return true If the base is prefixed with the given `PARCURIPath`
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *whole = parcURIPath_Parse("lci:/foo/bar, &pointer);
 *     PARCURIPath *prefix = parcURIPath_Parse("lci:/foo/, &pointer);
 *
 *     bool isPrefix(whole, prefix); // returns true
 * }
 * @endcode
 */
bool parcURIPath_StartsWith(const PARCURIPath *base, const PARCURIPath *prefix);

/**
 * Determine the length of the given `PARCURIPath` instance.
 *
 * @param [in] path The `PARCURIPath` instance which is being examined.
 *
 * @return The length of the `PARCURIPath`, in terms of the number of segments.
 *
 * Example:
 * @code
 * {
 *     char *pointer;
 *     PARCURIPath *path = parcURIPath_Parse("lci:/foo/, &pointer);
 *
 *     size_t lengthOfPath = parcURIPath_Length(path); // returns 2
 * }
 * @endcode
 */
size_t parcURIPath_Length(const PARCURIPath *path);
#endif // libparc_PARCURIPath_h
