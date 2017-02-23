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
 * @file ccnx_ManifestHashGroup.h
 * @brief A HashGroup in a FLIC manifest.
 *
 */
#ifndef libccnx_ccnx_ManifestHashGroup_h
#define libccnx_ccnx_ManifestHashGroup_h

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/ccnx_ContentObject.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_List.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_Iterator.h>
#include <parc/algol/parc_LinkedList.h>

#include <parc/security/parc_Signature.h>

struct ccnx_manifest_hash_group;
/**
 * @typedef CCNxManifestHashGroup
 * @brief A FLIC HashGroup
 */
typedef struct ccnx_manifest_hash_group CCNxManifestHashGroup;

typedef enum {
    CCNxManifestHashGroupPointerType_Data,
    CCNxManifestHashGroupPointerType_Manifest
} CCNxManifestHashGroupPointerType;

/**
 * @typedef CCNxManifestHashGroupPointer
 * @brief A HashGroup pointer.
 */
struct ccnx_manifest_hash_group_pointer;
typedef struct ccnx_manifest_hash_group_pointer CCNxManifestHashGroupPointer;

/**
 * Retrieve the type of a `CCNxManifestHashGroupPointer`.
 *
 * @param [in] ptr A `CCNxManifestHashGroupPointer` instance.
 *
 * @retval The type of the `CCNxManifestHashGroupPointer` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroupPointer *pointer = ccnxManifestHashGroup_GetPointerAtIndex(group, 0);
 *     CCNxManifestHashGroupPointerType type = ccnxManifestHashGroupPointer_GetType(pointer);
 *     if (type == CCNxManifestHashGroupPointerType_Data) {
 *          // Data
 *     } else {
 *          // Manifest
 *     }
 * }
 * @endcode
 */
CCNxManifestHashGroupPointerType ccnxManifestHashGroupPointer_GetType(const CCNxManifestHashGroupPointer *ptr);

/**
 * Retrieve hash digest associated with the `CCNxManifestHashGroupPointer` value.
 *
 * @param [in] ptr A `CCNxManifestHashGroupPointer` instance.
 *
 * @retval The hash digest for a `CCNxManifestHashGroupPointer` in a `PARCBuffer`.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroupPointer *pointer = ccnxManifestHashGroup_GetPointerAtIndex(group, 0);
 *     PARCBuffer *digest = ccnxManifestHashGroupPointer_GetDigest(pointer);
 *     // use the digest
 * }
 * @endcode
 */
const PARCBuffer *ccnxManifestHashGroupPointer_GetDigest(const CCNxManifestHashGroupPointer *ptr);

/**
 * Create a new and empty {@link CCNxManifestHashGroup} instance.
 *
 * @return A pointer to a {@link CCNxManifestHashGroup} instance, or NULL if an error or out of memory.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *section = CCNxManifestHashGroup_Create();
 *
 *     ...
 *
 *     CCNxManifestHashGroup_Release(&section);
 * }
 * @endcode
 */
CCNxManifestHashGroup *ccnxManifestHashGroup_Create();

/**
 * Create a new {@link CCNxManifestHashGroup} instance from a
 *
 * @param [in] jsonRepresentation - A pointer to a {@link PARCJSON} object representing the `CCNxManifestHashGroup`.
 *
 * @return A pointer to a {@link CCNxManifestHashGroup} instance, or NULL if an error or out of memory.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *jsonRep = ccnxTlvDictionary_GetJson(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ManifestHashGroup);
 *     CCNxManifestHashGroup *section = CCNxManifestHashGroup_CreateFromJson(jsonRep);
 *
 *     ...
 *
 *     ccnxJson_Destroy(&jsonRep);
 *     CCNxManifestHashGroup_Release(&section);
 * }
 * @endcode
 */
CCNxManifestHashGroup *ccnxManifestHashGroup_CreateFromJson(const PARCJSON *jsonRepresentation);

/**
 * Increase the number of references to an instance of this object.
 *
 * Note that new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the reference by invoking {@link CCNxManifestHashGroup_Release()}.
 *
 * @param [in] group A pointer to the {@link CCNxManifestHashGroup} to acquire.
 * @return The value of the input parameter @p group.
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = CCNxManifestHashGroup_Acquire(instance);
 *
 *     CCNxManifestHashGroup_Release(&manifest);
 * }
 * @endcode
 *
 * @see `CCNxManifestHashGroup_Release`
 */
CCNxManifestHashGroup *ccnxManifestHashGroup_Acquire(const CCNxManifestHashGroup *group);

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
 * @param [in,out] sectionP A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *manifest = CCNxManifestHashGroup_Acquire(instance);
 *
 *     CCNxManifestHashGroup_Release(&manifest);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_Release(CCNxManifestHashGroup **sectionP);

/**
 * Check that the pointer to the {@link CCNxManifestHashGroup} is valid. It should be non-null,
 * and any referenced data should also be valid.
 *
 * @param [in] manifest A pointer to the {@link CCNxManifestHashGroup} to check.
 *
 *
 * Example:
 * @code
 * {
 *     CCNxManifest *manifest = CCNxManifestHashGroup_Acquire(instance);
 *
 *     CCNxManifestHashGroup_AssertValid(manifest);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_AssertValid(const CCNxManifestHashGroup *manifest);

/**
 * Add a new pointer to the {@link CCNxManifestHashGroup} with the specified type and hash digest.
 *
 * @param [in] group - A {@link CCNxManifestHashGroup} instance.
 * @param [in] type - The {@link CCNxManifestHashGroupPointerType} type.
 * @param [in] buffer - The {@link PARCBuffer} containing the pointer digest.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = CCNxManifestHashGroup_Create();
 *
 *     PARCBuffer *hashDigest = ...;
 *
 *     bool added = ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Data, hashDigest);
 *     // added == true if the group was not full
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
bool ccnxManifestHashGroup_AppendPointer(CCNxManifestHashGroup *group, CCNxManifestHashGroupPointerType type, const PARCBuffer *buffer);

/**
 * Prepend a new pointer to the {@link CCNxManifestHashGroup} with the specified type and hash digest.
 *
 * @param [in] group - A {@link CCNxManifestHashGroup} instance.
 * @param [in] type - The {@link CCNxManifestHashGroupPointerType} type.
 * @param [in] buffer - The {@link PARCBuffer} containing the pointer digest.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = CCNxManifestHashGroup_Create();
 *
 *     PARCBuffer *hashDigest = ...;
 *
 *     bool added = ccnxManifestHashGroup_PrependPointer(group, CCNxManifestHashGroupPointerType_Data, hashDigest);
 *     // added == true if the group was not full
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
bool ccnxManifestHashGroup_PrependPointer(CCNxManifestHashGroup *group, CCNxManifestHashGroupPointerType type, const PARCBuffer *buffer);

/**
 * Retrieve the {@link CCNxManifestHashGroupPointer} in the {@link CCNxManifestHashGroup} at
 * the specified index.
 *
 * @param [in] group - A {@link CCNxManifestHashGroup} instance.
 * @param [in] index - The index of the `CCNxManifestHashGroupPointer` to retrieve.
 *
 * @retval The `CCNxManifestHashGroupPointer` at the specified index.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = CCNxManifestHashGroup_Create();
 *
 *     PARCBuffer *hashDigest = ...;
 *     ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Data, hashDigest);
 *
 *     // ...
 *     CCNxManifestHashGroupPointer *pointer = ccnxManifestHashGroup_GetPointerAtIndex(group, 0);
 *     // use it as needed
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
CCNxManifestHashGroupPointer *ccnxManifestHashGroup_GetPointerAtIndex(const CCNxManifestHashGroup *group, size_t index);

/**
 * Determine if two {@link CCNxManifestHashGroup} instances are equal.
 *
 * The following equivalence relations on non-null {@link CCNxManifestHashGroup} instances are maintained:
 *   * It is reflexive: for any non-null reference value x, `CCNxManifestHashGroup_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `CCNxManifestHashGroup_Equals(x, y)` must return true if and only if
 *        `CCNxManifestHashGroup_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `CCNxManifestHashGroup_Equals(x, y)` returns true and
 *        `CCNxManifestHashGroup_Equals(y, z)` returns true,
 *        then  `CCNxManifestHashGroup_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `CCNxManifestHashGroup_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `CCNxManifestHashGroup_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] x A pointer to a {@link CCNxManifestHashGroup} instance.
 * @param [in] y A pointer to a {@link CCNxManifestHashGroup} instance.
 * @return `true` if the referenced {@link CCNxManifestHashGroup} instances are equal.
 *
 * Example:
 * @code
 * {
 *    CCNxManifestHashGroup *a = CCNxManifestHashGroup_Create(...);
 *    CCNxManifestHashGroup *b = CCNxManifestHashGroup_Create(...);
 *
 *    if (CCNxManifestHashGroup_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool ccnxManifestHashGroup_Equals(const CCNxManifestHashGroup *x, const CCNxManifestHashGroup *y);

/**
 * Create a null-terminated string representation of the given {@link CCNxManifestHashGroup}.
 *
 * The returned value must be freed by the caller using {@link parcMemory_Deallocate()}.
 *
 * @param [in] group A pointer to a {@link CCNxManifestHashGroup} instance.
 *
 * @return A pointer to null-terminated string of characters repesenting the @p section that must be
 *         freed by the caller by `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = CCNxManifestHashGroup_Create();
 *
 *     char *sectionDescription = CCNxManifestHashGroup_ToString(group);
 *     printf("Manifest: %s\n", sectionDescription);
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
char *ccnxManifestHashGroup_ToString(const CCNxManifestHashGroup *group);

/**
 * Create a `CCNxJson` representation of the given {@link CCNxManifestHashGroup}.
 *
 * The returned value must be freed by the caller using {@link ccnxJson_Destroy()}.
 *
 * @param [in] section A pointer to a {@link CCNxManifestHashGroup} instance.
 *
 * @return A pointer to the JSON representation of the @p section that must be freed by the caller by `ccnxJson_Destroy()`.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = CCNxManifestHashGroup_Create();
 *
 *     PARCJSON *jsonForm = CCNxManifestHashGroup_ToJson(group);
 *     printf("Manifest: %s\n", ccnxJson_ToString(jsonForm));
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
PARCJSON *ccnxManifestHashGroup_ToJson(const CCNxManifestHashGroup *section);

/**
 * Set the {@link CCNxName} locator for this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 * @param [in] locator A {@link CCNxName} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *     CCNxName *name = ccnxName_CreateFromURI("lci:/some/place");
 *
 *     ccnxManifestHashGroup_setLocator(group, name);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_SetLocator(CCNxManifestHashGroup *group, const CCNxName *locator);

/**
 * Retrieve the {@link CCNxName} locator for this {@link CCNxManifestHashGroup}.
 *
 * @param [in] CCNxManifestHashGroup A {@link CCNxManifestHashGroup} instance.
 *
 * @return non-NULL The {@link CCNxName} locator for this {@link CCNxManifestHashGroup}.
 * @return NULL There is no locator.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     CCNxName *locator = ccnxManifestHashGroup_GetLocator(group);
 *     printf("Manifest Locator: %s\n", ccnxName_ToString(locator));
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
const CCNxName *ccnxManifestHashGroup_GetLocator(const CCNxManifestHashGroup *group);

/**
 * Retrieve the number of pointers in this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval The number of pointers in the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     size_t numPointers = ccnxManifestHashGroup_GetNumberOfPointers(group);
 *     for (size_t i = 0; i < numPointers; i++) {
 *          // get i-th pointer and do something with it
 *     }
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
size_t ccnxManifestHashGroup_GetNumberOfPointers(const CCNxManifestHashGroup *group);

/**
 * Determine if more pointers can be added to a {@link CCNxManifestHashGroup} instance.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @return true More space is available
 * @return false The manifest is full.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     size_t numPointers= ccnxManifestHashGroup_GetNumberOfPointers(group);
 *     for (size_t i = 0; i < numPointers; i++) {
 *          // get i-th pointer and do something with it
 *     }
 *
 *     CCNxManifestHashGroup_Release(&group);
 * }
 * @endcode
 */
bool ccnxManifestHashGroup_IsFull(const CCNxManifestHashGroup *group);

/**
 * Retrieve a {@link PARCIterator} to walk over each of the {@link CCNxManifestHashGroupPointer}
 * instances in the {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval A {@link PARCIterator} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *     PARCIterator *itr = ccnxManifestHashGroup_Iterator(group);
 *     while (parcIterator_HasNext(itr)) {
 *          CCNxManifestHashGroupPointer *ptr = (CCNxManifestHashGroupPointer *) parcIterator_Next(itr);
 *          // use the pointer
 *     }
 * }
 * @endcode
 */
PARCIterator *ccnxManifestHashGroup_Iterator(const CCNxManifestHashGroup *group);

/**
 * Retrieve the block size of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval The block size of the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     size_t blockSize = ccnxManifestHashGroup_GetBlockSize(group);
 *     // use it
 * }
 * @endcode
 */
size_t ccnxManifestHashGroup_GetBlockSize(const CCNxManifestHashGroup *group);

/**
 * Set the block size of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 * @param [in] blockSize The block size.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *     size_t blockSize =
 *
 *     ccnxManifestHashGroup_SetBlockSize(group, blockSize);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_SetBlockSize(CCNxManifestHashGroup *group, size_t blockSize);

/**
 * Retrieve the data size of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval The data size of the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     size_t dataSize = ccnxManifestHashGroup_GetDataSize(group);
 *     // use it
 * }
 * @endcode
 */
size_t ccnxManifestHashGroup_GetDataSize(const CCNxManifestHashGroup *group);

/**
 * Set the data size of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 * @param [in] dataSize The data size
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *     size_t dataSize =
 *
 *     ccnxManifestHashGroup_SetDataSize(group, dataSize);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_SetDataSize(CCNxManifestHashGroup *group, size_t dataSize);

/**
 * Retrieve the entry size of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval The entry size of the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     size_t entrySize = ccnxManifestHashGroup_GetEntrySize(group);
 *     // use it
 * }
 * @endcode
 */
size_t ccnxManifestHashGroup_GetEntrySize(const CCNxManifestHashGroup *group);

/**
 * Set the entry size of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 * @param [in] entrySize The entry size.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *     size_t entrySize =
 *
 *     ccnxManifestHashGroup_SetEntrySize(group, entrySize);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_SetEntrySize(CCNxManifestHashGroup *group, size_t entrySize);

/**
 * Retrieve the tree height of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval The tree height of the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     size_t treeHeight = ccnxManifestHashGroup_GetTreeHeight(group);
 *     // use it
 * }
 * @endcode
 */
size_t ccnxManifestHashGroup_GetTreeHeight(const CCNxManifestHashGroup *group);

/**
 * Set the tree height of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 * @param [in] treeHeight The tree height of the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *     size_t treeHeight = ...
 *     ccnxManifestHashGroup_SetTreeHeight(group, treeHeight);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_SetTreeHeight(CCNxManifestHashGroup *group, size_t treeHeight);

/**
 * Retrieve the overall data digest of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval The overall data digest of the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     const PARCBuffer *digest = ccnxManifestHashGroup_GetOverallDataDigest(group);
 *     // use it
 * }
 * @endcode
 */
const PARCBuffer *ccnxManifestHashGroup_GetOverallDataDigest(const CCNxManifestHashGroup *group);

/**
 * Set the overall data digest of this {@link CCNxManifestHashGroup}.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 * @param [in] digest The overall data digest of the {@link CCNxManifestHashGroup} instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *     const PARCBuffer *digest = ...
 *
 *     ccnxManifestHashGroup_SetOverallDataDigest(group, digest);
 * }
 * @endcode
 */
void ccnxManifestHashGroup_SetOverallDataDigest(CCNxManifestHashGroup *group, const PARCBuffer *digest);

/**
 * Determine if this `CCNxManifestHashGroup` is carrying any metadata.
 *
 * @param [in] group A {@link CCNxManifestHashGroup} instance.
 *
 * @retval true If the `CCNxManifestHashGroup` has metadata.
 * @retval false Otherwise
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...
 *
 *     bool hasMetadata = ccnxManifestHashGroup_HasMetadata(group);
 * }
 * @endcode
 */
bool ccnxManifestHashGroup_HasMetadata(const CCNxManifestHashGroup *group);

/**
 * Create a list of `CCNxInterest` instances that can be created from this single
 * `CCNxManifestHashGroup` instance.
 *
 * @param [in] group A pointer to an instance of `CCNxManifestHashGroup`.
 * @param [in] name A `CCNxName` locator for the interests in this list.
 *
 * @return A `PARCLinkedList` containing the set of all Interests that can be
 *         constructed from this HashGroup.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestHashGroup *group = ...;
 *
 *     PARCLinkedList *interests = ccnxManifestHashGroup_CreateInterestList(group);
 * }
 * @endcode
 */
PARCLinkedList *ccnxManifestHashGroup_CreateInterestList(const CCNxManifestHashGroup *group, const CCNxName *locator);
#endif // libccnx_ccnx_ManifestHashGroup_h
