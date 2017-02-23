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
 * @file ccnx_NameSegment.h
 * @ingroup Naming
 * @brief A path segment of a CCNx Name
 *
 * An RFC3986 compliant implementation of URI segments, where each path segment carries a label.
 * See {@link CCNxName} for more information.
 *
 */
#ifndef libccnx_ccnx_NameSegment_h
#define libccnx_ccnx_NameSegment_h
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/ccnx_NameLabel.h>

#include <parc/algol/parc_URI.h>

struct ccnx_name_Segment;
/**
 * @typedef CCNxNameSegment
 * @brief A path segment of a `CCNxName`
 */
typedef struct ccnx_name_segment CCNxNameSegment;

/**
 * Create a CCNxNameSegment instance initialised with the given type and value.
 *
 * @param [in] type A valid CCNxNameLabelType
 * @param [in] value A valid PARCBuffer containing the value of the name segment.
 *
 * @return non-NULL A pointer to a valid CCNxNameSegment instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *value = parcBuffer_WrapCString("value");
 *
 *     CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, value);
 *
 *     parcBuffer_Release(&value);
 * }
 * @endcode
 */
CCNxNameSegment *ccnxNameSegment_CreateTypeValue(CCNxNameLabelType type, const PARCBuffer *value);

/**
 * Create a CCNxNameSegment instance initialised with the given type and value taken from the given array of bytes.
 *
 * @param [in] type A valid CCNxNameLabelType
 * @param [in] length The number of bytes in @p array.
 * @param [in] array A pointer to a buffer containing the bytes for the value of the name segment.
 *
 * @return non-NULL A pointer to a valid CCNxNameSegment instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValueArray(CCNxNameLabelType_NAME, 5, "value");
 * }
 * @endcode
 */
CCNxNameSegment *ccnxNameSegment_CreateTypeValueArray(CCNxNameLabelType type, size_t length, const char *array);

/**
 * Parse a `CCNxNameSegment` from a {@link PARCURISegment} consisting of type specification and value.
 *
 * The name must be in conformance with `draft-mosko-icnrg-ccnxlabeledcontent-00`
 *
 * Names that use mnemonic values for labels, must conform to thier respective specifications.
 * See `draft-scott-icnrg-ccnxnameregistry-00` for a list of assigned names and type values.
 *
 * @param [in] uriSegment A pointer to a valid `PARCURISegment`
 * @return non-NULL A pointer to an allocated `CCNxNameSegment` which must eventually be released by calling
 *         {@link ccnxNameSegment_Release}().
 * @return NULL An error occurred.
 * @see `ccnxNameSegment_Release()`
 *
 * Example:
 * @code
 * {
 *     char *lciSegment = CCNxNameLabel_Name "=" "abcde";
 *     PARC_URISegment *uriSegment = parcURISegment_Parse(lciSegment, NULL);
 *
 *     CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(uriSegment);
 *
 *     CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
 *     assertTrue(CCNxNameLabelType_NAME == type, "Expected %04x, actual %04x", 0x20, type);
 *
 *     ccnxNameSegment_Release(&actual);
 *     parcURISegment_Release(&uriSegment);
 * }
 * @endcode
 */
CCNxNameSegment *ccnxNameSegment_ParseURISegment(const PARCURISegment *uriSegment);

/**
 * Create a new `CCNxNameSegment` by copying the given `CCNxNameSegment`. This is a deep copy,
 * and the created instance must eventually be released by calling {@link ccnxNameSegment_Release}().
 *
 * @param [in] segment A `CCNxNameSegment` pointer.
 * @return An allocated `CCNxNameSegment` which must eventually be released by calling  {@link ccnxNameSegment_Release}().
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *value = parcBuffer_WrapCString("hello");
 *     CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, value);
 *
 *     CCNxNameSegment *copy = ccnxNameSegment_Copy(segment);
 *
 *     ccnxNameSegment_Release(&segment);
 *     ccnxNameSegment_Copy(&copy);
 *     parcBuffer_Release(&value);
 * }
 * @endcode
 */
CCNxNameSegment *ccnxNameSegment_Copy(const CCNxNameSegment *segment);

/**
 * Determine if two `CCNxNameSegment` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxNameSegment` instances are maintained:
 *
 * * It is reflexive: for any non-null reference value x, `ccnxNameSegment_Equal(x, x)` must return true.
 *
 * * It is symmetric: for any non-null reference values x and y, `ccnxNameSegment_Equal(x, y)` must return true if and only if
 *        `ccnxNameSegment_Equal(y x)` returns true.
 *
 * * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxNameSegment_Equal(x, y)` returns true and
 *        `ccnxNameSegment_Equal(y, z)` returns true,
 *        then  `ccnxNameSegment_Equal(x, z)` must return true.
 *
 * * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxNameSegment_Equal(x, y)`
 *         consistently return true or consistently return false.
 *
 * * For any non-null reference value x, `ccnxNameSegment_Equal(x, NULL)` must return false.
 *
 * @param [in] segmentA A pointer to a `CCNxNameSegment` instance.
 * @param [in] segmentB A pointer to a `CCNxNameSegment` instance to be compared to `segmentA`.
 *
 * @return `true` if the given `CCNxNameSegment` instances are equal.
 *
 * Example:
 * @code
 * {
 *     char *lciSegment = CCNxNameLabelType_Label_Chunk "=" "123";
 *     PARC_URISegment *uriSegment = parcURISegment_Parse(lciSegment, NULL);
 *     CCNxNameSegment *segmentA = ccnxNameSegment_ParseURISegment(uriSegment);
 *
 *     PARCBuffer *buf = parcBuffer_WrapCString("123");
 *     CCNxNameSegment *segmentB = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_CHUNK, buf);
 *
 *     if (ccnxNameSegment_Equals(expected, actual)) {
 *          // true
 *      } else {
 *          // false
 *      }
 *     ccnxNameSegment_Release(&segmentA);
 *     ccnxNameSegment_Release(&segmentB);
 *     parcURISegment_Release(&uriSegment);
 *     parcBuffer_Release(&buf);
 * }
 * @endcode
 */
bool ccnxNameSegment_Equals(const CCNxNameSegment *segmentA, const CCNxNameSegment *segmentB);

/**
 * A signum function comparing two `CCNxNameSegment` instances.
 *
 * Used to determine the ordering relationship of two `CCNxNameSegment` instances.
 *
 * @param [in] segmentA A pointer to a `CCNxNameSegment` instance.
 * @param [in] segmentB A pointer to a `CCNxNameSegment` instance to be compared to `segmentA`.
 *
 * @return 0 if `segmentA` and `segmentB` are equivalent
 * @return < 0 if `segmentA` < `segmentB`
 * @return > 0 if `segmentA` > `segmentB`
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *valueA = parcBuffer_WrapCString("apple");
 *     CCNxNameSegment *segmentA = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, valueA);
 *
 *     PARCBuffer *valueB = parcBuffer_WrapCString("banana");
 *     CCNxNameSegment *segmentB = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, valueB);
 *
 *     int signum = ccnxNameSegment_Compare(segmentA, segmentB);
 *
 *     ccnxNameSegment_Release(&segmentA);
 *     parcBuffer_Release(&bufA);
 *     ccnxNameSegment_Release(&segmentB);
 *     parcBuffer_Release(&bufB);
 * }
 * @endcode
 */
int ccnxNameSegment_Compare(const CCNxNameSegment *segmentA, const CCNxNameSegment *segmentB);

/**
 * Get a pointer to the underlying PARCBuffer storing the value of the given `CCNxNameSegment`.
 *
 * A new reference to the instance is not created.
 * If the caller requires a reference to live beyond the lifetime of the `CCNxNameSegment`
 * it must acquire a reference via {@link parcBuffer_Acquire}.
 *
 * Any modifications to the buffer's position, limit or mark will affect subsequent use of the buffer.
 *
 * @param [in] segment A pointer to a `CCNxNameSegment` instance.
 * @return A pointer to the underlying @link PARCBuffer.
 *
 * Example:
 * @code
 * {
 *     PARC_URISegment *uriSegment = parcURISegment_Parse("label:param=value");
 *
 *     CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(uriSegment);
 *
 *     PARCBuffer *value = ccnxNameSegment_GetValue(actual);
 *
 *     ccnxNameSegment_Release(&actual);
 *     parcURISegment_Release(&uriSegment);
 * }
 * @endcode
 *
 * @see {@link ccnxNameSegment_Length}()
 */
PARCBuffer *ccnxNameSegment_GetValue(const CCNxNameSegment *segment);

/**
 * Get a pointer to the underlying PARCBuffer storing the parameter of the given `CCNxNameSegment`.
 *
 * A new reference to the instance is not created.
 * If the caller requires a reference to live beyond the lifetime of the `CCNxNameSegment`
 * it must acquire a reference via {@link parcBuffer_Acquire}.
 *
 * Any modifications to the buffer's position, limit or mark will affect subsequent use of the buffer.
 *
 * @param [in] segment A pointer to a `CCNxNameSegment` instance.
 * @return NULL The given `CCNxNameSegment` does not specify a parameter value.
 * @return non-NULL A pointer to the underlying `PARCBuffer`.
 *
 * Example:
 * @code
 * {
 *     CCNxNameSegment *segment = ccnxNameSegment_ParseURISegment("label:param=value");
 *
 *     PARCBuffer *parameter = ccnxNameSegment_GetParameter(segment);
 *
 *     ccnxNameSegment_Release(&segment);
 * }
 * @endcode
 *
 * @see {@link ccnxNameSegment_Length}()
 */
PARCBuffer *ccnxNameSegment_GetParameter(const CCNxNameSegment *segment);

/**
 * Produce a nul-terminated C string representation of the given `CCNxNameSegment`.
 *
 * A string representation, such as "1=foo".
 *
 * @param [in] segment A CCNxNameSegment pointer.
 * @return An allocated null-terminated byte array that must be deallocated by `{@link parcMemory_Deallocate}()`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/212");
 *     CCNxNameSegment *segment = ccnxName_GetSegment(name, 3);
 *
 *     char *string = ccnxNameSegment_ToString(segment);
 *     printf("Hello: %s\n", string);
 *
 *     parcMemory_Deallocate(string);
 *     ccnxName_Release(&name);
 * }
 * @endcode
 *
 * @see `parcMemory_Deallocate`
 */
char *ccnxNameSegment_ToString(const CCNxNameSegment *segment);

/**
 * Print a human readable representation of the given `CCNxNameSegment`.
 *
 * @param [in] segment A pointer to the instance to display.
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/212");
 *     CCNxNameSegment *segment = ccnxName_GetSegment(name, 3);
 *
 *     ccnxNameSegment_Display(segment, 0);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 *
 */
void ccnxNameSegment_Display(const CCNxNameSegment *segment, int indentation);

/**
 * Append a printable-character representation of the specified instance to the given {@link PARCBufferComposer}.
 *
 * @param [in] segment A pointer to the `CCNxNameSegment` instance.
 * @param [in,out] composer A pointer to the `PARCBufferComposer` instance onto which to append our printable representation.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The @p composer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *
 *     ccnxNameSegment_BuildString(instance, result);
 *
 *     char *string = parcBuffer_ToString(parcBufferComposer_ProduceBuffer(result));
 *     printf("Hello: %s\n", string);
 *     parcMemory_Deallocate(string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 *
 * @see PARCBufferComposer
 */
PARCBufferComposer *ccnxNameSegment_BuildString(const CCNxNameSegment *segment, PARCBufferComposer *composer);

/**
 * Return the length of the specified `CCNxNameSegment`, in bytes.
 *
 * @param [in] segment A pointer to a `CCNxNameSegment` instance.
 * @return The number of bytes for the value of the given `CCNxNameSegment`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/humidity/12");
 *     CCNxNameSegment *segment = ccnxName_GetSegment(name, 3);
 *
 *     int length = ccnxNameSegment_Length(segment);
 *
 *     ccnxName_Release(&name);
 * }
 * @endcode
 */
size_t ccnxNameSegment_Length(const CCNxNameSegment *segment);

/**
 * Get the label of the given `CCNxNameSegment`.
 *
 * @param [in] segment A pointer to a `CCNxNameSegment` instance.
 * @return non-NULL A pointer to a PARCBuffer instance containing the label.
 * @return NULL The CCNxNameSegment instance does not have a label.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buf = parcBuffer_WrapCString("apple");
 *     CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);
 *
 *     CCNxNameLabelType type = ccnxNameSegment_GetType(segment);
 *
 *     ccnxNameSegment_Release(&segment);
 *     parcBuffer_Release(&buf);
 * }
 * @endcode
 *
 * @see `CCNxNameLabelType`
 */
PARCBuffer *ccnxNameSegment_GetLabel(const CCNxNameSegment *segment);

/**
 * Get the {@link CCNxNameLabelType} of the given `CCNxNameSegment`.
 *
 * @param [in] segment A pointer to a `CCNxNameSegment` instance.
 * @return The `CCNxNameLabelType` of the specified `CCNxNameSegment` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buf = parcBuffer_WrapCString("apple");
 *     CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);
 *
 *     CCNxNameLabelType type = ccnxNameSegment_GetType(segment);
 *
 *     ccnxNameSegment_Release(&segment);
 *     parcBuffer_Release(&buf);
 * }
 * @endcode
 *
 * @see `CCNxNameLabelType`
 */
CCNxNameLabelType ccnxNameSegment_GetType(const CCNxNameSegment *segment);

/**
 * Return a hashcode for the given `CCNxNameSegment`.
 *
 * Whenever `HashCode()` is invoked on the same instance more than once within the same execution environment,
 * the HashCode function must consistently return the same value, provided no information used in its corresponding
 * `Equals()` comparisons on the same instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 *
 * If two instances are equal according to the `Equals()` function,
 * then calling the `HashCode` function on each of the two objects must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the `Equals()` function,
 * then calling the HashCode() method on each of the two instances must produce distinct integer results.
 * However, the programmer should be aware that producing distinct results for unequal instances
 * may improve the performance of some data structures.
 *
 * @param segment A pointer to a `CCNxNameSegment` instance.
 * @return An unsigned 32-bit integer hash code value.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *bufA =parcBuffer_WrapCString("Test");
 *
 *     CCNxNameSegment *segmentA = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufA);
 *
 *     uint32_t hashCode = ccnxNameSegment_HashCode(segmentA);
 *
 *     ccnxNameSegment_Release(&segmentA);
 *     parcBuffer_Release(&bufA);
 * }
 * @endcode
 *
 * @see `parcHash32Bits_Hash`
 */
PARCHashCode ccnxNameSegment_HashCode(const CCNxNameSegment *segment);

/**
 * Increase the number of references to a `CCNxNameSegment`.
 *
 * Note that a new `CCNxNameSegment` is not created,
 * only that the given `CCNxNameSegment` reference count is incremented.
 * Discard the reference by invoking {@link ccnxNameSegment_Release}().
 *
 * @param [in] segment A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxNameSegment *segment = ccnxNameSegment_Acquire(instance);
 *
 *     ccnxNameSegment_Release(&segment);
 *
 * }
 * @endcode
 *
 * @see ccnxNameSegment_Release
 */
CCNxNameSegment *ccnxNameSegment_Acquire(const CCNxNameSegment *segment);

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
 * @param [in,out] segmentP A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxNameSegment *segment = ccnxNameSegment_Acquire(instance);
 *
 *     ccnxNameSegment_Release(&segment);
 *
 * }
 * @endcode
 *
 * @see ccnxNameSegment_Acquire
 */
void ccnxNameSegment_Release(CCNxNameSegment **segmentP);

#ifdef Libccnx_DISABLE_VALIDATION
#  define ccnxNameSegment_OptionalAssertValid(_instance_)
#else
#  define ccnxNameSegment_OptionalAssertValid(_instance_) ccnxNameSegment_AssertValid(_instance_)
#endif

/**
 * Assert that an instance of `CCNxNameSegment` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] segment A pointer to the instance to check.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     ccnxNameSegment_AssertValid(segment);
 *
 * }
 * @endcode
 */
void ccnxNameSegment_AssertValid(const CCNxNameSegment *segment);

/**
 * Determine if an instance of `CCNxNameSegment` is valid.
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of referenced objects.
 *
 * @param [in] segment A pointer to the instance to check.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     ccnxNameSegment_IsValid(segment);
 *
 * }
 * @endcode
 */
bool ccnxNameSegment_IsValid(const CCNxNameSegment *segment);
#endif // libccnx_ccnx_NameSegment_h
