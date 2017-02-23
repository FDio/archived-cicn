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
 * @file ccnx_Link.h
 * @ingroup ContentObject
 * @brief A generic CCNx link that contains a hash, name, KeyID.
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#ifndef libccnx_ccnx_Link_h
#define libccnx_ccnx_Link_h

#include <ccnx/common/ccnx_Name.h>

#include <parc/security/parc_Signature.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Buffer.h>

struct ccnx_link;

/**
 * @typedef CCNxLink
 * @brief A generic CCNx link that contains a hash, name, KeyID.
 * @see {@link ccnxLink_Create}
 */
typedef struct ccnx_link CCNxLink;

/**
 * Create a new `CCNxLink` instance.
 *
 * @param [in] name The {@link CCNxName} for the new `CCNxLink`.
 * @param [in] keyId A {@link PARCBuffer} containing the KeyID for the new `CCNxLink`.
 * @param [in] contentObjectHash A `PARCBuffer` containing the Content Object Hash for the new `CCNxLink`.
 * @return A pointer to a new `CCNxLink` instance, or NULL if an error or out of memory.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxLink *ccnxLink_Create(const CCNxName *name, PARCBuffer *keyId, PARCBuffer *contentObjectHash);

/**
 * Create a new `CCNxLink` instance as a copy of the original.
 *
 * @param [in] original The {@link CCNxLink} to copy.
 * @return A pointer to a new `CCNxLink` instance, or NULL if an error or out of memory.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxLink *ccnxLink_Copy(const CCNxLink *original);

/**
 * Fetch the name associated with this `CCNxLink`.
 *
 * @param [in] link  A pointer to a `CCNxLink` instance
 * @return The {@link CCNxName} associated with the @p link.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const CCNxName *ccnxLink_GetName(const CCNxLink *link);

/**
 * Fetch the KeyID associated with this `CCNxLink`.
 *
 * @param [in] link  A pointer to a `CCNxLink` instance
 * @return a {@link PARCBuffer} containing the KeyID associated with the @p link.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxLink_GetKeyID(const CCNxLink *link);

/**
 * Fetch the ContentObjectHash associated with this `CCNxLink`.
 *
 * @param [in] link  A pointer to a `CCNxLink` instance
 * @return a {@link PARCBuffer} containing the Content Object Hash associated with the @p link.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxLink_GetContentObjectHash(const CCNxLink *link);

/**
 * Determine if two `CCNxLink` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxLink` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `ccnxLink_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxLink_Equals(x, y)` must return true if and only if
 *        `ccnxLink_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxLink_Equals(x, y)` returns true and
 *        `ccnxLink_Equals(y, z)` returns true,
 *        then  `ccnxLink_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxLink_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxLink_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] x A pointer to a `CCNxLink` instance.
 * @param [in] y A pointer to a `CCNxLink` instance.
 * @return `true` if the referenced `CCNxLink` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CCNxLink *a = ccnxLink_Create();
 *    CCNxLink *b = ccnxLink_Create();
 *
 *    if (ccnxLink_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool ccnxLink_Equals(const CCNxLink *x, const CCNxLink *y);

/**
 * Create a null-terminated string representation of the given `CCNxLink`.
 *
 * The returned value must be freed by the caller using {@link parcMemory_Deallocate()}.
 *
 * @param [in] link  A pointer to a `CCNxLink` instance.
 * @return A pointer to null-terminated string of characters that must be freed by the caller by `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
char *ccnxLink_ToString(const CCNxLink *link);

/**
 * Increase the number of references to an instance of this object.
 *
 * Note that new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the reference by invoking {@link ccnxLink_Release()}.
 *
 * @param [in] link  A pointer to a `CCNxLink` instance to acquire.
 * @return The value of the input parameter @p link.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxLink *link = ccnxLink_Acquire(instance);
 *
 *     ccnxLink_Release(&link);
 *
 * }
 * @endcode
 *
 * @see ccnxLink_Release
 */
CCNxLink *ccnxLink_Acquire(const CCNxLink *link);

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
 * @param [in,out] linkP A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxLink *link = ccnxLink_Acquire(instance);
 *
 *     ccnxLink_Release(&link);
 *
 * }
 * @endcode
 */
void ccnxLink_Release(CCNxLink **linkP);

/**
 * Check that the pointer to the `CCNxLink` is valid. It should be non-null,
 * and any referenced data should also be valid.
 *
 * @param [in] link  A pointer to a `CCNxLink` instance
 *
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxLink *link = ccnxLink_Acquire(instance);
 *
 *     ccnxLink_AssertValid(link);
 *
 * }
 * @endcode
 */
void ccnxLink_AssertValid(const CCNxLink *link);

/**
 * Determine if a `CCNxLink` instance is valid.
 *
 * @param [in] link  A pointer to a `CCNxLink` instance
 * @return true The instance is valid.
 * @return false The instance is invalid.
 *
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxLink *link = ccnxLink_Acquire(instance);
 *
 *     ccnxLink_AssertValid(link);
 *
 * }
 * @endcode
 */
bool ccnxLink_IsValid(const CCNxLink *link);
#endif // libccnx_ccnx_Link_h
