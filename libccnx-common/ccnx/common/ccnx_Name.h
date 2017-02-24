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
 * @file ccnx_Name.h
 * @ingroup Naming
 * @brief The basic CCNx Name.
 *
 * This implements an RFC3986 URI compliant identifier in which each path segment carries a label.
 * This allows differentiation between resources with otherwise similar identifiers that are not related.
 * For example, one resource could be named `"/parc/csl/7"` meaning the 7th version of `"/parc/csl"`,
 * while another could mean the 7th page of the resource.
 * With labeled segments, the two resources would have unambiguous names,
 * such as `"/parc/csl/version=7"` and `"/parc/csl/page=7"`.
 *
 */
#ifndef libccnx_ccnx_Name_h
#define libccnx_ccnx_Name_h

#include <parc/algol/parc_HashCode.h>
#include <ccnx/common/ccnx_NameSegment.h>

struct ccnx_name;
/**
 * @typedef CCNxName
 * @brief An RFC3986 URI compliant identifier in which each path segment carries a label.
 */
typedef struct ccnx_name CCNxName;

/**
 * Create a new instance of `CCNxName`, initialized from the given string representation of an LCI URI,
 * using dynamically allocated memory.
 *
 * The URI must be a well-formed URI.
 *
 * The `CCNxName` instance must be released by calling {@link ccnxName_Release}.
 *
 * @param [in] uri A null-terminated string representation of the CCNx Name.
 * @return non-NULL A pointer to a `CCNxName` instance initialised from the given URI.
 * @return NULL An error occured parsing the URI, or the URI contained an invalid specification.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/h2162");
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
CCNxName *ccnxName_CreateFromCString(const char *uri);

/**
 * Create a new instance of `CCNxName`, initialized from the given `PARCURI` representation of an LCI URI,
 * using dynamically allocated memory.
 *
 * The URI must be a well-formed LCI URI.
 *
 * The `CCNxName` instance must be released by calling {@link ccnxName_Release}.
 *
 * @param [in] uri A valid instance of PARCURI.
 * @return non-NULL A pointer to a `CCNxName` instance initialised from the given URI.
 * @return NULL An error occured parsing the URI, or the URI contained an invalid specification.
 *
 * Example:
 * @code
 * {
 *     PARCURI *uri = parcURI_Parse("lci:/parc/csl/media/h2162");
 *     CCNxName *name = ccnxName_CreateFromCString(uri);
 *
 *     parcURI_Release(&uri);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
CCNxName *ccnxName_FromURI(const PARCURI *uri);

/**
 * Create a new instance of `CCNxName` from the given format string and variable number of parameters.
 *
 * @param [in] format A pointer to a nul-terminated printf format string
 * @param [in] ... A variable number of parameters.
 *
 * @return non-NULL A pointer to a valid CCNxName instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFormatString("lci://%s/object", "parc.com");
 * }
 * @endcode
 */
CCNxName *ccnxName_CreateFormatString(const char *restrict format, ...);

/**
 * Create a new instance of `CCNxName`, initialized from a string representation of a LCI URI contained in the given PARCBuffer,
 * starting at the current position and terminating with a zero-byte in the buffer.
 * using dynamically allocated memory.
 *
 * The URI must be a well-formed URI.
 *
 * The `CCNxName` instance must be released by calling {@link ccnxName_Release}.
 *
 * @param [in] uri A null-terminated string representation of the CCNx Name.
 * @return non-NULL A pointer to a `CCNxName` instance initialised from the given URI.
 * @return NULL An error occured parsing the URI, or the URI contained an invalid specification.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_AllocateCString("lci:/parc/csl/media/h2162");
 *     CCNxName *name = ccnxName_CreateFromBuffer(buffer);
 *
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
CCNxName *ccnxName_CreateFromBuffer(const PARCBuffer *buffer);

/**
 * Increase the number of references to a `CCNxName` instance.
 *
 * Note that new `CCNxName` is not created,
 * only that the given `CCNxName` reference count is incremented.
 * Discard the reference by invoking {@link ccnxName_Release}.
 *
 * @param [in] name A pointer to the original `CCNxName`.
 * @return The value of the input parameter @p name.
 *
 * Example:
 * @code
 * {
 *     CCNxName *original = ccnxName_Create();
 *
 *     CCNxName *reference = ccnxName_Acquire(original);
 *
 *     ccnxName_Release(&original);
 *     ccnxName_Release(&reference);
 * }
 * @endcode
 *
 * @see ccnxName_Release
 */
CCNxName *ccnxName_Acquire(const CCNxName *name);

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
 * @param [in,out] nameP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_Create(...);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 *
 * @see {@link ccnxName_Acquire}
 */
void ccnxName_Release(CCNxName **nameP);

#ifdef Libccnx_DISABLE_VALIDATION
#  define ccnxName_OptionalAssertValid(_instance_)
#else
#  define ccnxName_OptionalAssertValid(_instance_) ccnxName_AssertValid(_instance_)
#endif

/**
 * Assert that an instance of `CCNxName` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] name A pointer to a `CCNxName` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/ccn/things/p1e");
 *
 *     ccnxName_AssertValid(name);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
void ccnxName_AssertValid(const CCNxName *name);

/**
 * Determine if an instance of `CCNxName` is valid.
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] name A pointer to a `CCNxName` instance.
 * @return true If the instance is valid.
 * @return false if the instance is not valid.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/ccn/things/p1e");
 *
 *     if (ccnxName_IsValid(name) == true) {
 *         ...
 *     }
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
bool ccnxName_IsValid(const CCNxName *name);

/**
 * Append a representation of the specified `CCNxName` instance to the given
 * {@link PARCBufferComposer}.
 *
 * The CCN URI representing the {@link CCNxNameSegment}'s associated with this `CCNxName` is
 * appended to the supplied {@link PARCBufferComposer}.
 *
 * @param [in] name A pointer to a `CCNxName` instance whose representation should be appended to the @p composer.
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
 *     ccnxName_BuildString(instance, result);
 *
 *     char *string = parcBuffer_ToString(parcBufferComposer_ProduceBuffer(result));
 *     printf("Hello: %s\n", string);
 *     parcMemory_Deallocate(string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 *
 * @see `CCNxNameSegment`
 */
PARCBufferComposer *ccnxName_BuildString(const CCNxName *name, PARCBufferComposer *composer);

/**
 * Produce a null-terminated string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] name A pointer to the `CCNxName` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated,
 *         null-terminated C string that must be deallocated via `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *instance = ccnxName_Create();
 *
 *     char *string = ccnxName_ToString(instance);
 *
 *     if (string != NULL) {
 *         printf("Hello: %s\n", string);
 *         parcMemory_Deallocate(string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     ccnxName_Release(&instance);
 * }
 * @endcode
 *
 * @see {@link ccnxName_BuildString}
 * @see {@link ccnxName_Display}
 * @see {@link parcMemory_Deallocate}
 */
char *ccnxName_ToString(const CCNxName *name);

/**
 * Print a human readable representation of the given `CCNxName`.
 *
 * @param [in] name A pointer to the `CCNxName` to display.
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *
 *     ccnxInterest_Display(name, 0);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
void ccnxName_Display(const CCNxName *name, int indentation);

/**
 * Append a {@link CCNxNameSegment} to the given `CCNxName`.
 *
 * Append the `CCNxNameSegment` to the given `CCNxName`.
 * The given `CCNxName` is modified.
 *
 * @param [in,out] name The base `CCNxName` to append the @p segment to.
 * @param [in] segment The segment to append to @p name.
 * @return The modifed @p name.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name1 = ccnxName_CreateFromCString("lci:/a/b/c/d");
 *     CCNxName *name2 = ccnxName_Create();
 *
 *     CCNxNameSegment *segment = ccnxName_GetSegment(name1, 0);
 *
 *     ccnxName_Append(name2, segment);
 *
 *     ccnxName_Release(&name1);
 *     ccnxName_Release(&name2);
 * }
 * @endcode
 */
CCNxName *ccnxName_Append(CCNxName *name, const CCNxNameSegment *segment);

/**
 * Determine if a `CCNxName` is starts with another.
 *
 * Each {@link CCNxNameSegment} in @p prefix is tested against the corresponding `CCNxNameSegment`
 * in @p name.
 *
 * @param [in] name A pointer to a `CCNxName` instance.
 * @param [in] prefix A pointer to a `CCNxName` instance to be tested as a prefix of `name`.
 *
 * @return `true` If the given @p name starts with @p prefix.
 * @return `false` If the given @p name does not start with @p prefix.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/17");
 *     CCNxName *prefix = ccnxName_CreateFromCString("lci:/parc/csl");
 *
 *     if (ccnxName_StartsWith(name, prefix)) {
 *        ...
 *     }
 *
 *     ccnxName_Release(&name);
 *     ccnxName_Release(&prefix);
 * }
 * @endcode
 */
bool ccnxName_StartsWith(const CCNxName *name, const CCNxName *prefix);

/**
 * Create a copy of the specified `CCNxName` instance, producing a new, independent, instance
 * from dynamically allocated memory.
 *
 * This a deep copy. All referenced memory is copied. The created instance of `CCNxName` must
 * be released by calling {@link ccnxName_Release}().
 *
 * @param [in] originalName The `CCNxName` to copy
 * @return A new, independent copy of the given `CCNxName`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *originalName = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/17");
 *     CCNxName *copy = ccnxName_Copy(originalName);
 *
 *     ...
 *
 *     ccnxName_Release(&originalName);
 *     ccnxName_Release(&copy);
 * }
 * @endcode
 */
CCNxName *ccnxName_Copy(const CCNxName *originalName);

/**
 * Determine if two `CCNxName` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxName` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `ccnxName_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxName_Equals(x, y)` must return true if and only if
 *        `ccnxName_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxName_Equals(x, y)` returns true and
 *        `ccnxName_Equals(y, z)` returns true,
 *        then  `ccnxName_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxName_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxName_Equals(x, NULL)` must return false.
 *
 * @param [in] name1 A pointer to a `CCNxName` instance.
 * @param [in] name2 A pointer to a `CCNxName` instance.
 *
 * @return True If the given `CCNxName` instances are equal
 * @return False Otherwise
 *
 * Example:
 * @code
 * {
 *     CCNxName *orig = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/17");
 *     CCNxName *copy = ccnxName_Copy(orig);
 *
 *     if (ccnxName_Equals(orig, copy)) {
 *         // true
 *     } else {
 *         // false
 *      }
 *     ccnxName_Release(&orig);
 *     ccnxName_Release(&copy);
 * }
 * @endcode
 *
 * @see {@link ccnxName_Compare}
 */
bool ccnxName_Equals(const CCNxName *name1, const CCNxName *name2);

/**
 * Create a new `CCNxName` object
 *
 * The instance is reference counted with an initial count of 1.
 * Additional references are acquired via the function {@link ccnxName_Acquire}
 * and these references are inidividually released via {@link ccnxName_Release}
 *
 * @return A pointer to a new instance of a `CCNxName`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_Create();
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
CCNxName *ccnxName_Create(void);

/**
 * Compare @p name1 to @p name2 using CCNx canonical ordering (shortlex).
 *
 * `NULL` is considered the shortest name, so `(NULL, non-NULL) -> -1` and
 * `(NULL, NULL) -> 0`, `(non-NULL, NULL) -> +1`.
 *
 * @param [in] name1 A pointer to a `CCNxName` instance.
 * @param [in] name2 A pointer to a `CCNxName` instance.
 *
 * Returns:
 * <ul>
 * <li>-1 for name1 < name2</li>
 * <li> 0 for name1 = name2</li>
 * <li>+1 for name1 > name2</li>
 * </ul>
 *
 * Example:
 * @code
 * {
 *     CCNxName *orig = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/17");
 *     CCNxName *copy = ccnxName_Copy(orig);
 *
 *     if (ccnxName_Compare(orig, copy)) {
 *         ...
 *     }
 *
 *     ccnxName_Release(&orig);
 *     ccnxName_Release(&copy);
 * }
 * @endcode
 *
 * @see {@link ccnxName_Equals}
 */
int ccnxName_Compare(const CCNxName *name1, const CCNxName *name2);

/**
 * Return a pointer to the {@link CCNxNameSegment} instance for the specified `CCNxName` at the given index.
 * The index must be greater than or equal to zero and less than {@link ccnxName_GetSegmentCount}().
 *
 * @param [in] name The target `CCNxName`
 * @param [in] index The index into the @p name from which to retrieve the `CCNxNameSegment`.
 *
 * @return A pointer to a `CCNxNameSegment`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/things/b00se");
 *     CCNxNameSegment *segment2 = ccnxName_GetSegment(name, 2);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
CCNxNameSegment *ccnxName_GetSegment(const CCNxName *name, size_t index);

/**
 * Get the number of `CCNxNameSegments` in the specified `CCNxName`.
 *
 * @param [in] name A pointer to an instance of `CCNxName`.
 *
 * @return The number of name segments in @p name.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/things/b00se");
 *     size_t segmentCount = ccnxName_GetSegmentCount(name);
 *
 *     printf("Number of segments: %d\n", segmentCount);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
size_t ccnxName_GetSegmentCount(const CCNxName *name);

/**
 * Return a hashcode for the given `CCNxName`.
 *
 * Whenever `HashCode()` is invoked on the same object more than once within the same execution environment,
 * the HashCode function must consistently return the same integer, provided no information used in Equals()
 * comparisons on the same object is modified.
 *
 * This integer need not remain consistent from one execution of an application to another execution of the same application.
 *
 * If two objects are equal according to the Equals() function,
 * then calling the hashCode method on each of the two objects must produce the same integer result.
 *
 * It is not required that if two objects are unequal according to the Equals() function,
 * then calling the Equals() method on each of the two objects must produce distinct integer results.
 * However, the programmer should be aware that producing distinct integer results for unequal objects
 * may improve the performance of some data structures.
 *
 * @param name [in] A pointer to an instance of `CCNxName`.
 * @return A uint32_t hash code.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/ccn/things/r00");
 *
 *     uint32_t hashCode = ccnx_LeftMostHashCode(name);
 *
 *     ccnxName_Release(&name);
 * {
 * @endcode
 */
PARCHashCode ccnxName_HashCode(const CCNxName *name);

/**
 * Return a hash of the leftmost @p count {@link CCNxNameSegment}s in a `CCNxName`.
 *
 * See @{link ccnxName_HashCode} for more information.
 *
 * @param [in] name A pointer to a `CCNxName` instance.
 * @param [in] count The number, starting from the left, of path segments to use to compute the hash.
 *
 * @return A uint32_t hash code.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/ccn/things/r00");
 *
 *     uint32_t hashCode = ccnx_LeftMostHashCode(name, 2);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
PARCHashCode ccnxName_LeftMostHashCode(const CCNxName *name, size_t count);

/**
 * Trim @p numberToRemove segments from the end of the specified `CCNxName`.
 *
 * If @p numberToRemove is greater than the number of segments in the name,
 * all segments are removed.
 * If @p numberToRemove is 0, nothing happens.
 * The name segments are destroyed.
 *
 * @param [in,out] name A pointer to a `CCNxName` instance to trim.
 * @param [in] numberToRemove The number of rightmost segments to remove from the name.
 *
 * @return The value of @p name
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/ccn/things/b00se");
 *
 *     name = ccnxName_Trim(name, 2);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 *
 */
CCNxName *ccnxName_Trim(CCNxName *name, size_t numberToRemove);

/**
 * Compose a new CCNxName instance consisting of the given @prefix appended with @p suffix as a `CCNxNameLabelType_NAME`.
 *
 * @param [in] prefix A pointer to a valid CCNxName instance containing the prefix of the new name.
 * @param [in] suffix A pointer to a nul-terminated C string that is appended to the new name.
 *
 * @return non-NULL A pointer to a valid CCNxName instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
CCNxName *ccnxName_ComposeNAME(const CCNxName *prefix, const char *suffix);


/**
 * Get the Nth segment number of the given `CCNxName`
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] name A pointer to a valid `CCNxName` instance.
 * @param [in] nthNumber The nth segment number in the given name, or SIZE_MAX
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t ccnxName_GetNthSegmentNumber(const CCNxName *name, size_t nthNumber);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t ccnxName_GetSegmentNumber(const CCNxName *name);

/**
 * Create a new CCNxName instance composed of the given CCNxName with the parsed result of the format string appended.
 *
 * @param [in] baseName The base name of the new CCNxName
 * @param [in] format A printf(3) format string
 *
 * @return non-NULL A pointer to a valid `CCNxName` segment.
 * @return NULL The object could not be created.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
CCNxName *ccnxName_ComposeFormatString(const CCNxName *baseName, const char *restrict format, ...);

/**
 * Create a CCNxName that is a prefix of another.
 *
 * If the specified length is greater than the number of segments available,
 * the result is a new name that is a copy of the old name.
 *
 * @param [in] name A pointer to a valid `CCNxName` instance.
 * @param [in] length The number of `CCNxNameSegments` the prefix must include.
 *
 * @return non-NULL A pointer to a valid CCNxName.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     CCNxName *a = ccnxName_CreateFromCString("ccnx:/a/b/c");
 *
 *     CCNxName *prefix = ccnxName_CreatePrefix(a, 1);
 *
 *     // prefix is equal to the name "ccnx:/a"
 *
 *     ccnxName_Release(&a);
 *     ccnxName_Release(&prefix);
 *     ccnxName_Release(&actual);
 * }
 * @endcode
 */
CCNxName *ccnxName_CreatePrefix(const CCNxName *name, size_t length);
#endif // libccnx_ccnx_Name_h
