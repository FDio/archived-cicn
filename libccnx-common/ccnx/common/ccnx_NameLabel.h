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
 * @file ccnx_NameLabel.h
 * @ingroup Naming
 * @brief The possible types of CCNx Name Segments, and utilities to extract and encode them.
 *
 * Every CCNxName is comprised of CCNxNameSegments, and each CCNxNameSegment has a type associated with it.
 * For example, it may specify a simple name (`CCNxNameLabelType_NAME`),
 * content chunk numbers (`CCNxNameLabelType_CHUNK`), or any other type defined in `CCNxNameLabelType`.
 *
 * The type of a name is comprised of a label and an option parameter.
 * The label may be a decimal or hexadecimal representation of the type value, or a mnemonic like "Name", "Serial".
 *
 */
#ifndef libccnx_ccnx_NameType_h
#define libccnx_ccnx_NameType_h

#include <stdint.h>
#include <stdbool.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BufferComposer.h>

#define CCNxNameLabelType_APP0    ((CCNxNameLabelType) 0x1000)
#define CCNxNameLabelType_APP4096 ((CCNxNameLabelType) (CCNxNameLabelType_APP0 + 4096))

/**
 * Compose a CCNx Name Label Type in the Application-specific type space.
 */
#define CCNxNameLabelType_App(_n_) (CCNxNameLabelType_APP0 + (CCNxNameLabelType) _n_)

/**
 * @typedef CCNxNameLabelType
 * @brief An enumeration of possible CCNxName types.
 */
typedef enum {
    CCNxNameLabelType_BADNAME = 0x0000,
    CCNxNameLabelType_NAME = 0x0001,      // Name: CCNx Messages in TLV Format
    CCNxNameLabelType_PAYLOADID = 0x0002, // Payload Hash: CCNx Messages in TLV Format
    CCNxNameLabelType_BINARY = 0x0003,    // Binary segment
    CCNxNameLabelType_CHUNK = 0x0010,     // Segment Number: CCNx Content Object Segmentation
    CCNxNameLabelType_CHUNKMETA = 0x0011, // Metadata
    CCNxNameLabelType_TIME = 0x0012,      // Time: CCNx Publisher Serial Versioning
    CCNxNameLabelType_SERIAL = 0x0013,    // Serial Number: CCNx Publisher Serial Versioning
    CCNxNameLabelType_Unknown = 0xfffff
} CCNxNameLabelType;

/**
 * These definitions should agree with the CCNxNameLabelType enumeration to avoid confusion.
 * These definitions must agree with the specification and with the CCNxNameLabelType_String definitions.
 *
 *  Use these definition in constructing string representations of names.
 *  For example "lci://" CCNxNameLabel_Serial "/1"
 *
 * @def CCNxNameLabelType_NameLabel  The name
 */
#define CCNxNameLabel_Name               "Name"
#define CCNxNameLabel_InterestPayloadId  "PayloadId"
#define CCNxNameLabel_Chunk              "Chunk"
#define CCNxNameLabel_ChunkMeta          "ChunkMeta" // "17"
#define CCNxNameLabel_Time               "Time"
#define CCNxNameLabel_Serial             "Serial"
#define CCNxNameLabel_App                "App"

#define CCNxNameLabelType_LabelApp(_n_) "App:" #_n_

struct ccnx_name_label;
typedef struct ccnx_name_label CCNxNameLabel;

/**
 * Parse a PARCBuffer containing a CCN LCI Name Label.
 *
 * When complete, the buffer's position will be set to the first byte of the value portion.
 *
 * @param [in] buffer A pointer to a valid PARCBuffer instance.
 *
 * @return NULL Memory could not be allocated
 * @return non-NULL A pointer to a valid CCNxNameLabel instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("App:1=value");
 *     CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
 *
 *     parcBuffer_Release(&buffer);
 *     ccnxNameLabel_Release(&label);
 * }
 * @endcode
 */
CCNxNameLabel *ccnxNameLabel_Parse(PARCBuffer *buffer);

/**
 * Create an instance of `CCNxNameLabel`.
 *
 * @param [in] type A CCNxNameLabelType value
 * @param [in] parameter The value NULL or a pointer to a valid `PARCBuffer` instance.
 *
 * @return NULL Memory could not be allocated, or the given `CCNxNameLabelType` is one of `CCNxNameLabelType_BADNAME` or `CCNxNameLabelType_Unknown`.
 * @return non-NULL A pointer to a valid CCNxNameLabelType instance.
 *
 * Example:
 * @code
 * {
 *     CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_NAME, NULL);
 *
 *     ccnxNameLabel_Release(&label);
 * }
 * @endcode
 */
CCNxNameLabel *ccnxNameLabel_Create(CCNxNameLabelType type, const PARCBuffer *parameter);

/**
 * Increase the number of references to a `CCNxNameLabel`.
 *
 * Note that new `CCNxNameLabel` is not created,
 * only that the given `CCNxNameLabel` reference count is incremented.
 * Discard the reference by invoking `ccnxNameLabel_Release`.
 *
 * @param [in] label A pointer to a `CCNxNameLabel` instance.
 *
 * @return The input `CCNxNameLabel` pointer.
 *
 * Example:
 * @code
 * {
 *     CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_NAME, NULL);
 *
 *     CCNxNameLabel *x_2 = ccnxNameLabel_Acquire(label);
 *
 *     ccnxNameLabel_Release(&label);
 *     ccnxNameLabel_Release(&x_2);
 * }
 * @endcode
 */
CCNxNameLabel *ccnxNameLabel_Acquire(const CCNxNameLabel *label);

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
 * @param [in,out] labelPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_NAME, NULL);
 *
 *     ccnxNameLabel_Release(&label);
 * }
 * @endcode
 */
void ccnxNameLabel_Release(CCNxNameLabel **labelPtr);

/**
 * Get the CCNxNameLabelType for the given `CCNxNameLabel`.
 *
 * @param [in] label A pointer to a valid `CCNxNameLabel` instance.
 *
 * @return The CCNxNameLabelType for the given `CCNxNameLabel`
 *
 * Example:
 * @code
 * {
 *     CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_NAME, NULL);
 *
 *     CCNxNameLabelType type = ccnxNameLabel_GetType(label);
 *
 *     ccnxNameLabel_Release(&label);
 * }
 * @endcode
 */
CCNxNameLabelType ccnxNameLabel_GetType(const CCNxNameLabel *label);

/**
 * Get the parameter for the given `CCNxNameLabel`.
 *
 * @param [in] label A pointer to a valid `CCNxNameLabel` instance.
 *
 * @return NULL, or a pointer to a valid `PARCBuffer` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *parameter = parcBuffer_WrapCString("2");
 *     CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_APP, parameter);
 *     parcBuffer_Release(&parameter);
 *
 *     PARCBuffer *param = ccnxNameLabel_GetParameter(label);
 *
 *     ccnxNameLabel_Release(&label);
 * }
 * @endcode
 */
PARCBuffer *ccnxNameLabel_GetParameter(const CCNxNameLabel *label);

/**
 * Append a representation of the specified `CCNxNameLabel` instance to the given
 * {@link PARCBufferComposer}.
 *
 * The representation is the canonical representation for a CCN LCI name segment.
 *
 * @param [in] name A pointer to a `CCNxNameLabel` instance whose representation should be appended to the @p composer.
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
 *     PARCBuffer *parameter = parcBuffer_WrapCString("2");
 *     CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_APP, parameter);
 *     parcBuffer_Release(&parameter);
 *
 *     PARCBuffer *param = ccnxNameLabel_GetParameter(label);
 *
 *     ccnxNameLabel_BuildString(label composer);
 *
 *     ccnxNameLabel_Release(&label);
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
PARCBufferComposer *ccnxNameLabel_BuildString(const CCNxNameLabel *label, PARCBufferComposer *composer);

/**
 * Create a copy of the specified `CCNxNameLabel` instance, producing a new, independent, instance
 * from dynamically allocated memory.
 *
 * This a deep copy. All referenced memory is copied. The created instance of `CCNxNameLabel` must
 * be released by calling {@link ccnxNameLabel_Release}().
 *
 * @param [in] label The `CCNxNameLabel` to copy.
 *
 * @return NULL  Memory could not be allocated.
 * @return non-NULL  A pointer to a new, independent copy of the given `CCNxName`.
 *
 * Example:
 * @code
 * {
 *     CCNxNameLabel *original = ccnxNameLabel_Create(CCNxNameLabelType_NAME);
 *     CCNxNameLabel *copy = ccnxNameLabel_Copy(original);
 *
 *     ...
 *
 *     ccnxNameLabel(&original);
 *     ccnxNameLabel(&copy);
 * }
 * @endcode
 */
CCNxNameLabel *ccnxNameLabel_Copy(const CCNxNameLabel *label);

/**
 * Determine if two `CCNxNameLabel` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxNameLabel` instances are maintained:
 *
 * * It is reflexive: for any non-null reference value x, `ccnxNameLabel_Equals(x, x)` must return true.
 *
 * * It is symmetric: for any non-null reference values x and y, `ccnxNameLabel_Equals(x, y)` must return true if and only if
 *        `ccnxNameLabel_Equals(y x)` returns true.
 *
 * * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxNameLabel_Equals(x, y)` returns true and
 *        `ccnxNameLabel_Equals(y, z)` returns true,
 *        then  `ccnxNameLabel_Equals(x, z)` must return true.
 *
 * * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxNameLabel_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 * * For any non-null reference value x, `ccnxNameLabel_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a `CCNxNameLabel` instance.
 * @param [in] y A pointer to a `CCNxNameLabel` instance to be compared to @p x.
 *
 * @return `true` if the given `CCNxNameLabel` instances are equal.
 *
 * Example:
 * @code
 * {
 *     CCNxNameLabel *x = ccnxNameLabel_Create(CCNxNameLabelType_SERIAL, NULL);
 *     CCNxNameLabel *y = ccnxNameLabel_Create(CCNxNameLabelType_SERIAL, NULL);
 *
 *     if (ccnxNameSegment_Equals(x, y)) {
 *          // true
 *      } else {
 *          // false
 *      }
 *     ccnxNameLabel_Release(&segmentA);
 *     ccnxNameLabel_Release(&segmentB);
 * }
 * @endcode
 */
bool ccnxNameLabel_Equals(const CCNxNameLabel *x, const CCNxNameLabel *y);

/**
 * Determine if an instance of `CCNxNameLabel` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] label A pointer to a `CCNxNameLabel` instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
bool ccnxNameLabel_IsValid(const CCNxNameLabel *label);

#ifdef Libccnx_DISABLE_VALIDATION
#  define ccnxNameLabel_OptionalAssertValid(_instance_)
#else
#  define ccnxNameLabel_OptionalAssertValid(_instance_) ccnxNameLabel_AssertValid(_instance_)
#endif

/**
 * Assert that an instance of `CCNxNameLabel` is valid.
 *
 * If the instance is not valid, terminate via `trapIllegalValue()`
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] label A pointer to a `CCNxNameLabel` instance.
 *
 * Example:
 * @code
 * {
 *     PARCByteArray *array = parcByteArray_Allocate(64);
 *
 *     parcBuffer_AssertValid(array);
 * }
 * @endcode
 *
 */
void ccnxNameLabel_AssertValid(const CCNxNameLabel *label);
#endif // libccnx_ccnx_NameType_h
