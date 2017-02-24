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
 * @file parc_URI.h
 * @ingroup networking
 * @brief A Universal Resource Identifier
 *
 * An RF2396 compliant URI implementation with facilities for composition, decomposition,
 * comparison, manipulation, etc.
 *
 */
#ifndef libparc_parc_URI_h
#define libparc_parc_URI_h

#include <parc/algol/parc_URIPath.h>
#include <parc/algol/parc_BufferComposer.h>

struct parc_uri;
typedef struct parc_uri PARCURI;

/**
 * Create a new `PARCURI` object.
 *
 * The new `PARCURI` object is empty.
 *
 * @return A pointer to a `PARCURI` instance.
 *
 * Example:
 * @code
 * {
 *     PARCURI *uri = parcURI_Create();
 *     ...
 *     parcURI_Release(&uri);
 * }
 * @endcode
 */
PARCURI *parcURI_Create(void);

/**
 * Create a new instance of `PARCURI` from the given format string and arguments.
 *
 * @param [in] format A pointer to a nul-terminated printf format string
 * @param [in] argList A pointer to a valid `va_list`
 *
 * @return non-NULL A pointer to a valid PARCURI instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     char *
 *     createFormatString(const char * restrict format, ...)
 *     {
 *         va_list argList;
 *         va_start(argList, format);
 *
 *         PARCURI *uri = parcURI_CreateFromValist(format, argList);
 *
 *         va_end(argList);
 *         return uri;
 *     }
 * }
 * @endcode
 */
PARCURI *parcURI_CreateFromValist(const char *restrict format, va_list argList);

/**
 * Create a new instance of `PARCURI` from the given format string and variable number of parameters.
 *
 * @param [in] format A pointer to a nul-terminated printf format string
 * @param [in] ... A variable number of parameters.
 *
 * @return non-NULL A pointer to a valid PARCURI instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCURI *uri = parcURI_CreateFromFormatString("http://%s/index.html", "127.0.0.1");
 * }
 * @endcode
 */
PARCURI *parcURI_CreateFromFormatString(const char *restrict format, ...);

/**
 * Increase the number of references to a `PARCURI` instance.
 *
 * Note that new `PARCURI` is not created,
 * only that the given `PARCURI` reference count is incremented.
 * Discard the reference by invoking {@link parcURI_Release}.
 *
 * @param uri A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCURI *x = parcURI_Create();
 *
 *     PARCURI *x2 = parcURI_Acquire(x);
 *
 *     parcURI_Release(&x);
 *     parcURI_Release(&x2);
 * }
 * @endcode
 *
 * @see parcURI_Release
 */
PARCURI *parcURI_Acquire(const PARCURI *uri);

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
 * @param [in,out] uriPtr A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     PARCURI *x = parcURI_Create();
 *
 *     parcURI_Release(&x);
 * }
 * @endcode
 */
void parcURI_Release(PARCURI **uriPtr);

/**
 * Parse a well-formed URI into a `PARCURI` instance.
 *
 *   The URI string must be null-terminated so that parsing can detect the
 *   end of the path correctly.
 *
 * @param [in] string The URI C-string used to create the `PARCURI` instance.
 *
 * @return non-NULL A pointer to an allocated `PARCURI` structure which must be destroyed via {@link parcURI_Release()}.
 * @return NULL The URI was malformed.
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com";
 *
 *     PARCURI *uri = parcURI_Parse(uriString);
 *     if (uri == NULL) {
 *         printf("Malformed URI '%s', uriString);
 *     }
 * }
 * @endcode
 */
PARCURI *parcURI_Parse(const char *string);

/**
 * Create a deep copy of the given `PARCURI` instance.
 *
 * @param [in] uri A `PARCURI` pointer.
 *
 * @return A deep copy of the given `PARCURI` instance.
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *
 *     PARCURI *copy = parcURI_Copy(uri);
 *     // uri and copy will contain equivalent paths
 *
 *     parcURI_Release(&uri);
 *     parcURI_Release(&copy);
 * }
 * @endcode
 */
PARCURI *parcURI_Copy(const PARCURI *uri);

/**
 * Return true if two `PARCURI` instances are equal.
 *
 * The following equivalence relations on non-null `PARCURI` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, equals(x, x) must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, equals(x, y) must return true if and only if
 *        equals(y x) returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        equals(x, y) returns true and
 *        equals(y, z) returns true,
 *        then  equals(x, z) must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of equals(x, y)
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, equals(x, NULL)) must return false.
 *
 * @param [in] uriA The first `PARCURI` instance.
 * @param [in] uriB The second `PARCURI` instance.
 *
 * @return true the given `PARCURI` instances are equal.
 * @return false the given `PARCURI` instances are not equal.
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *     PARCURI *copy = parcURI_Copy(uri);
 *
 *     if (parcURI_Equals(uri, copy)) {
 *         printf("Paths are equal.\n");
 *     } else {
 *         printf("Paths are NOT equal.\n");
 *     }
 *
 *     parcURI_Release(&uri);
 *     parcURI_Release(&copy);
 * }
 * @endcode
 */
bool parcURI_Equals(const PARCURI *uriA, const PARCURI *uriB);

/**
 * Get the scheme part of the given URI.
 *
 * The scheme of a URI path is the "type", e.g., labeled content
 * identifier "lci" for "lci:/foo/bar".
 *
 * @param [in] uri A `PARCURI` pointer.
 *
 * @return char* The scheme of the given `PARCURI` instance
 * @return NULL If no scheme is available.
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *
 *     const char *scheme = parcURI_GetScheme(uri); // will be "lci"
 *
 *     parcURI_Release(&uri);
 * }
 * @endcode
 */
const char *parcURI_GetScheme(const PARCURI *uri);

/**
 * Get the authority part of the given `PARCURI`.
 *
 * The authority part of a URI is the string: username at host:port
 *
 * @param [in] uri A `PARCURI` pointer.
 *
 * @return char* The authority of the given `PARCURI` instance
 * @return NULL If no scheme is available.
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://chris@parc.com:80";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *
 *     const char *authority = parcURI_GetAuthority(uri); // will be "chris@parc.com:80"
 *
 *     parcURI_Release(&uri);
 * }
 * @endcode
 */
const char *parcURI_GetAuthority(const PARCURI *uri);

/**
 * Get the {@link PARCURIPath} part of the given URI.
 *
 *   Every `PARCURI` contains a `PARCURIPath` consisting of the path portion of the URI.
 *
 * @param [in] uri A `PARCURI` pointer.
 *
 * @return The `PARCURIPath` part of the given URI.
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com/foo/bar/";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *
 *     const char *path = parcURI_GetPath(uri); // will be "/foo/bar/"
 *
 *     parcURI_Release(&uri);
 * }
 * @endcode
 */
PARCURIPath *parcURI_GetPath(const PARCURI *uri);

/**
 * Get the query part of the given PARCURI`.
 *
 *   Queries are contiguous key-value string segments of a URI.
 *   For example, the query of URI string "http://parc.com/x=1&y=2&z=3" is "x=1&y=2&z=3"
 *
 * @param [in] uri A `PARCURI` pointer.
 *
 * @return char* The query string, if present, for the given URI
 * @return NULL If no query string is present
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com/x=1&y=2&z=3";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *
 *     const char *path = parcURI_GetQuery(uri); // will be "x=1&y=2&z=3"
 *
 *     parcURI_Release(&uri);
 * }
 * @endcode
 */
const char *parcURI_GetQuery(const PARCURI *uri);

/**
 * Get the fragment part of the given `PARCURI`.
 *
 *   The fragment of a URI is the string component following the '#' character.
 *   For example, the fragment of URI string "http://parc.com/index.html#info" is "info"
 *
 * @param [in] uri A `PARCURI` pointer.
 *
 * @return char* The fragment of the URI, if present
 * @return NULL If no fragment is in the URI
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com/index.html#info";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *
 *     const char *path = parcURI_GetFragment(uri); // will be "info"
 *
 *     parcURI_Release(&uri);
 * }
 * @endcode
 */
const char *parcURI_GetFragment(const PARCURI *uri);

/**
 * Append a representation of the specified `PARCURI` instance to the given `PARCBufferComposer`.
 *
 * @param [in] uri A pointer to a `PARCURI` instance whose representation should be appended to the @p composer.
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance to be modified.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The @p composer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *
 *     parcURI_BuildString(instance, result);
 *
 *     char *string = parcBufferComposer_ToString(result);
 *     printf("Hello: %s\n", string);
 *     parcMemory_Deallocate(string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcURI_BuildString(const PARCURI *uri, PARCBufferComposer *composer);

/**
 * Produce an allocated null-terminated string representation of the given URI.
 *
 *   The returned value must be destroyed via `parcMemory_Deallocate`
 *
 * @param [in] uri The `PARCURI` instance to format.
 *
 * @return An allocated null-terminated string representation of the given URI.
 *
 * Example:
 * @code
 * {
 *     const char *uriString = "http://parc.com";
 *     PARCURI *uri = parcURI_Parse(uriString);
 *
 *     printf("path string = %s\n", parcURI_ToString(uri));
 *
 *     parcURI_Release(&uri);
 * }
 * @endcode
 */
char *parcURI_ToString(const PARCURI *uri);
#endif // libparc_parc_URI_h
