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
 * @file ccnx_Manifest.h
 * @ingroup ContentObject
 * @brief The generic manifest.
 *
 */
#ifndef libccnx_ccnx_Manifest_h
#define libccnx_ccnx_Manifest_h

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/ccnx_ManifestHashGroup.h>

#include <ccnx/common/internal/ccnx_ManifestInterface.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

#include <parc/security/parc_Signature.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_Buffer.h>

struct ccnx_manifest;

/**
 * @typedef CCNxManifest
 * @brief Structure of the CCNxManifest
 */
typedef CCNxTlvDictionary CCNxManifest;

/**
 * Create a new `CCNxManifest` instance.
 *
 * @param [in] nameLink A pointer to a `CCNxName`
 * @param [in] payload A pointer to a `CCNxManifestSection`
 *
 * @return A pointer to a `CCNxManifest` instance, or NULL if an error or out of memory.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromURI("lci:/foo/bar/manifest");
 *
 *
 *     CCNxManifest *object = ccnxManifest_Create(name, payload);
 * }
 * @endcode
 */
CCNxManifest *ccnxManifest_Create(const CCNxName *name);

/**
 * Create a new nameless `CCNxManifest` instance.
 *
 * @return A pointer to a `CCNxManifest` instance, or NULL if an error or out of memory.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *object = ccnxManifest_CreateNameless();
 * }
 * @endcode
 */
CCNxManifest *ccnxManifest_CreateNameless(void);

/**
 * Increase the number of references to an instance of this object.
 *
 * Note that new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the reference by invoking {@link ccnxManifest_Release()}.
 *
 * @param [in] manifest A pointer to the instance of `CCNxManifest` to acquire.
 *
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ccnxManifest_Acquire(instance);
 *
 *     ccnxManifest_Release(&manifest);
 * }
 * @endcode
 *
 * @see `ccnxManifest_Release`
 */
CCNxManifest *ccnxManifest_Acquire(const CCNxManifest *manifest);

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
 * @param [in,out] manifestP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ccnxManifest_Acquire(instance);
 *
 *     ccnxManifest_Release(&manifest);
 *
 * }
 * @endcode
 */
void ccnxManifest_Release(CCNxManifest **manifestP);

/**
 * Check that the pointer to the `CCNxManifest` is valid. It should be non-null,
 * and any referenced data should also be valid.
 *
 * @param [in] manifest A pointer to an instance of `CCNxManifest` to check.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ccnxManifest_Acquire(instance);
 *
 *     ccnxManifest_AssertValid(manifest);
 *
 * }
 * @endcode
 */
void ccnxManifest_AssertValid(const CCNxManifest *manifest);

/**
 * Add a HashGroup to the given `CCNxManifest`.
 *
 * @param [in] manifest A pointer to an instance of `CCNxManifest`.
 * @param [in] group A pointer to an instance of `CCNxManifestHashGroup`.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ...
 *     CCNxManifestHashGroup *group = ...;
 *
 *     ccnxManifest_AddHashGroup(manifest, group);
 * }
 * @endcode
 */
void ccnxManifest_AddHashGroup(CCNxManifest *manifest, const CCNxManifestHashGroup *group);

/**
 * Get the `CCNxManifestHashGroup` corresponding to the specified index.
 *
 * @param [in] manifest A pointer to an instance of `CCNxManifest`.
 * @param [in] index The index of the `CCNxManifestHashGroup` to retrieve.
 *
 * @return A pointer to a `CCNxManifestHashGroup`.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ...
 *     CCNxManifestHashGroup *group = ...;
 *
 *     // Add the first group
 *     ccnxManifest_AddHashGroup(manifest, group);
 *
 *     CCNxManifestHashGroup *expected = ccnxManifest_GetHashGroup(manifest, 0);
 * }
 * @endcode
 */
CCNxManifestHashGroup *ccnxManifest_GetHashGroupByIndex(const CCNxManifest *manifest, size_t index);

/**
 * Get the number of {@link CCNxManifestHashGroup} instances in the specified manifest.
 *
 * @param [in] manifest A pointer to an instance of {@link CCNxManifest}.
 *
 * @return A pointer to a {@link CCNxManifestHashGroup}.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ...
 *     CCNxManifestHashGroup *group = ...;
 *
 *     // Add the first group
 *     ccnxManifest_AddHashGroup(manifest, group);
 *
 *     // Add more groups...
 *
 *     printf("Number of hash groups: %d\n", ccnxManifest_GetNumberOfHashGroups(manifest));
 * }
 * @endcode
 */
size_t ccnxManifest_GetNumberOfHashGroups(const CCNxManifest *manifest);

/**
 * Create a list of `CCNxInterest` instances that can be created from this single
 * `CCNxManifest` instance.
 *
 * @param [in] manifest A pointer to an instance of `CCNxManifest`.
 * @param [in] name A `CCNxName` locator for the interests in this list.
 *
 * @return A `PARCLinkedList` containing the set of all Interests that can be
 *         constructed from this Manifest
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ...;
 *     CCNxName *locator = ...;
 *
 *     PARCLinkedList *interests = ccnxManifest_CreateInterestList(manifest, locator);
 * }
 * @endcode
 */
PARCLinkedList *ccnxManifest_CreateInterestList(const CCNxManifest *manifest, const CCNxName *name);

/**
 * Get the `CCNxName` for the given `CCNxManifest`.
 *
 * @param [in] manifest A pointer to an instance of `CCNxManifest`.
 * @return A pointer to the `CCNxName`.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ...;
 *
 *     CCNxName *name = ccnxManifest_GetName(manifest);
 * }
 * @endcode
 */
const CCNxName *ccnxManifest_GetName(const CCNxManifest *manifest);

/**
 * Determine if two `CCNxManifest` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxManifest` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `ccnxManifest_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxManifest_Equals(x, y)` must return true if and only if
 *        `ccnxManifest_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxManifest_Equals(x, y)` returns true and
 *        `ccnxManifest_Equals(y, z)` returns true,
 *        then  `ccnxManifest_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxManifest_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxManifest_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] x A pointer to a `CCNxManifest` instance.
 * @param [in] y A pointer to a `CCNxManifest` instance.
 * @return `true` if the referenced `CCNxManifest` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CCNxManifest *a = CCNxManifest_Create(...);
 *    CCNxManifest *b = CCNxManifest_Create(...);
 *
 *    if (CCNxManifest_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool ccnxManifest_Equals(const CCNxManifest *x, const CCNxManifest *y);

/**
 * Create a null-terminated string representation of the given {@link CCNxManifest}.
 *
 * The returned value must be freed by the caller using {@link parcMemory_Deallocate()}.
 *
 * @param [in] manifest A pointer to an instance of {@link CCNxManifest}.
 * @return A pointer to null-terminated string of characters that must be freed by the caller by `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = ...;
 *
 *     char *stringForm = ccnxManifest_ToString(manifest);
 * }
 * @endcode
 */
char *ccnxManifest_ToString(const CCNxManifest *manifest);
#endif // libccnx_ccnx_Manifest_h
