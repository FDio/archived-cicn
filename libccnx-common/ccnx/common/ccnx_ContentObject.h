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
 * @file ccnx_ContentObject.h
 * @brief A CCNx ContentObject contains content to be sent in response to an Interest.
 *
 * The canonical CCN content object. A content object contains a payload, a {@link CCNxName},
 * and security binding information. It's sent in response to a CCN Interest.
 *
 * @see {@link CCNxInterest}
 *
 */

#ifndef libccnx_ccnx_ContentObject_h
#define libccnx_ccnx_ContentObject_h
#include <stdbool.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/ccnx_PayloadType.h>
#include <ccnx/common/ccnx_KeyLocator.h>

#include <ccnx/common/internal/ccnx_ContentObjectInterface.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

#include <parc/security/parc_Signature.h>

#include <parc/algol/parc_Buffer.h>

/**
 * @typedef CCNxContentObject
 * @brief The CCNx Content Object
 */
typedef CCNxTlvDictionary CCNxContentObject;

/**
 * Create a new instance of a `CCNxContentObject`, using dynamically allocated memory, with
 * the specified name and payload.
 *
 * The created instance must be released by calling {@link ccnxContentObject_Release()}.
 *
 * @param [in] contentName The CCNxName associated with this `CCNxContentObject`.
 * @param [in] payload The data to be encapsulated by this `CCNxContentObject`. May be NULL.
 *
 * @return A new instance of a `CCNxContentObject`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 *
 * @see {@link ccnxContentObject_Release}
 */
CCNxContentObject *ccnxContentObject_CreateWithNameAndPayload(const CCNxName *contentName,
                                                              const PARCBuffer *payload);

/**
 * Create a new instance of a `CCNxContentObject`, using dynamically allocated memory, with
 * the specified payload. This will be a "nameless" Content Object.
 *
 * The created instance must be released by calling {@link ccnxContentObject_Release()}.
 *
 * @param [in] payload The data to be encapsulated by this `CCNxContentObject`. May be NULL.
 *
 * @return A new instance of a `CCNxContentObject`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 *
 * @see {@link ccnxContentObject_Release}
 */
CCNxContentObject *ccnxContentObject_CreateWithPayload(const PARCBuffer *payload);

/**
 * Create a new instance of a `CCNxContentObject`, using dynamically allocated memory, with
 * the specified payload, using the specified {@link CCNxContentObjectInterface}.
 *
 * The created instance must be released by calling {@link ccnxContentObject_Release}().
 *
 * @param [in] implementation The interface's underlying implementation to use to build this ContentObject.
 * @param [in] contentName The CCNxName associated with this `CCNxContentObject`.
 * @param [in] type the type of the payload. Must be a valid {@link CCNxPayloadType}.
 * @param [in] payload The data to be encapsulated by this `CCNxContentObject`. May be NULL.
 *
 * @return A new instance of a `CCNxContentObject`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject =
 *         ccnxContentObject_CreateWithImplAndPayload(&contentObjectImpl_Facade_V0,
 *                                                    name,
 *                                                    CCNxPayloadType_DATA,
 *                                                    payload);
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 *
 * @see ccnxContentObject_Release
 */
CCNxContentObject *ccnxContentObject_CreateWithImplAndPayload(const CCNxContentObjectInterface *implementation,
                                                              const CCNxName *contentName,
                                                              const CCNxPayloadType type,
                                                              const PARCBuffer *payload);


/**
 * Return a pointer to the {@link CCNxName} associated with this `CCNxContentObject`.
 *
 * This is memory managed by the `CCNxContentObject` and does not have to be released separately
 * unless {@link ccnxName_Acquire()} is called to acquire it.
 *
 * @param [in] contentObject An pointer to an instance of a `CCNxContentObject`.
 *
 * @return A pointer to the `CCNxName` associated with the specified `CCNxContentObject`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
 *
 *     CCNxName *pointerToName = ccnxContentObject_GetName(contentObject);
 *
 *     ...
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 *
 * @see `CCNxName`
 */
CCNxName *ccnxContentObject_GetName(const CCNxContentObject *contentObject);

/**
 * Return a pointer to the payload associated with this `CCNxContentObject`.
 *
 * The returned pointer points to the {@link PARCBuffer} containing the `CCNxContentObject`'s payload.
 * This is memory managed by the `CCNxContentObject` and does not have to be released separately
 * unless {@link parcBuffer_Acquire()} is called to acquire it.
 *
 * @param [in] contentObject A pointer to an instance of a `CCNxContentObject`.
 *
 * @return A pointer to the `PARCBuffer` containing the specified `CCNxContentObject`'s payload.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
 *
 *     CCNxName *pointerToPayload = ccnxContentObject_GetPayload(contentObject);
 *
 *     ...
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 *
 * @see `PARCBuffer`
 */
PARCBuffer *ccnxContentObject_GetPayload(const CCNxContentObject *contentObject);

/**
 * Return the type of payload for this `CCNxContentObject`.
 *
 * The enumeration must be one of the defined values in {@link CCNxPayloadType}.
 *
 * @param [in] contentObject A pointer to an instance of a `CCNxContentObject`.
 *
 * @return The `CCNxContentObject` instances payload type.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
 *
 *     ...
 *
 *     CCNxPayloadType payloadType = ccnxContentObject_GetPayloadType(contentObject);
 *     if (payloadType == CCNx_PAYLOAD_DATA) {
 *         printf("Payload type is CCNx_PAYLOAD_DATA (raw data)");
 *     }
 *
 *     ...
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 */
CCNxPayloadType ccnxContentObject_GetPayloadType(const CCNxContentObject *contentObject);

/**
 * Set the number of the final chunk necessary to represent the content
 * in this ContentObject.
 *
 * The final chunk number is the 0-based count of the last chunk necessary
 * to encapsulate the content in this ContentObject. For example, if it would
 * take 10 chunks to encapsulate some content, the final chunk number would be 9.
 *
 * @param [in,out] contentObject A pointer to the `CCNxContentObject` to which to assign the final chunk number.
 * @param [in] finalChunkNumber The number of the final chunk
 *
 * @return `true` If the final chunk number was succesfully set.
 * @return `false` If the final chunk number could not be set. This might happen if the
 *         underlying transport format doesn't support final chunk numbers.
 * Example:
 * @code
 * {
 *     uint64_t finalChunkNumber = 2803;
 *     ccnxContentObject_SetFinalChunkNumber(contentObject, finalChunkNumber);
 * }
 * @endcode
 *
 * @see ccnxContentObject_GetFinalChunkNumber
 */
bool ccnxContentObject_SetFinalChunkNumber(CCNxContentObject *contentObject, const uint64_t finalChunkNumber);

/**
 * Returns `true` if this `CCNxContentObject` has a final chunk number.
 *
 * @param [in] contentObject A pointer to the `CCNxContentObject`
 *
 * @return `true` If the object has a final chunk number set.
 * @return `false` If the object has no final chunk number set.
 *
 * Example:
 * @code
 * {
 *     if (ccnxContentObject_HasFinalChunkNumber(contentObject)) {
 *        uint64_t finalChunkNumber = ccnxContentObject_GetFinalChunkNumber(contentObject);
 *        ...
 *     }
 * }
 * @endcode
 *
 * @see {@link ccnxContentObject_GetFinalChunkNumber}
 */
bool ccnxContentObject_HasFinalChunkNumber(const CCNxContentObject *contentObject);

/**
 * Return the final chunk number specified by this `CCNxContentObject`.
 *
 * @param [in] contentObject A pointer to the `CCNxContentObject` to get the final chunk number from.
 *
 * @return The final chunk number of the specified `CCNxContentObject`.
 *
 * Example:
 * @code
 * {
 *     uint64_t finalChunkNumber = ccnxContentObject_SetFinalChunkNumber(contentObject);
 * }
 * @endcode
 *
 * @see {@link ccnxContentObject_SetFinalChunkNumber}
 */
uint64_t ccnxContentObject_GetFinalChunkNumber(const CCNxContentObject *contentObject);

/**
 * Associate the supplied keyId, signature, and keyLocator with the specified `CCNxContentObject`.
 *
 * @param [in,out] contentObject A pointer to the `CCNxContentObject` to update.
 * @param [in] keyId A pointer to the {@link PARCBuffer} containing the keyId to assign to the contentObject.
 * @param [in] signature A pointer to a {@link PARCSignature} to assign to the contentObject.
 * @param [in] keyLocator A pointer to a {@link CCNxKeyLocator} to assign to the contentObject. May be NULL.
 *
 * @return true if the signature payload was successfully set, false otherwise.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/hello/dolly");
 *     PARCBuffer *payload = parcBuffer_WrapCString("hello");
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
 *
 *     PARCBuffer *keyId = parcBuffer_WrapCString("keyhash");
 *     PARCBuffer *sigbits = parcBuffer_CreateFromArray((void *) "siggybits", strlen("siggybits"));
 *     PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256, parcBuffer_Flip(sigbits));
 *
 *     ccnxContentObject_SetSignature(contentObject, keyId, signature, NULL);
 *
 *     parcBuffer_Release(&payload);
 *     parcBuffer_Release(&sigbits);
 *     parcBuffer_Release(&keyId);
 *     parcSignature_Release(&signature);
 *     ccnxName_Release(&name);
 *     ccnxContentObject_Release(&contentObject);
 * }
 * @endcode
 *
 * @see `PARCSignature`
 * @see `CCNxKeyLocator`
 * @see `PARCBuffer`
 */
bool ccnxContentObject_SetSignature(CCNxContentObject *contentObject, const PARCBuffer *keyId,
                                    const PARCSignature *signature, const CCNxKeyLocator *keyLocator);


/**
 * Get the associated keyId from the specified `CCNxContentObject`.
 *
 * @param [in] contentObject A pointer to the `CCNxContentObject`.
 *
 * @return PARCBuffer A PARCBuffer containing the keyId or NULL if there is no keyId associated with the object.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *keyId = ccnxContentObject_GetKeyId(contentObject);
 *     if (keyId != NULL) {
 *          ...
 *      }
 * }
 * @endcode
 *
 * @see `ccnxContentObject_SetSignature`
 */
PARCBuffer *ccnxContentObject_GetKeyId(const CCNxContentObject *contentObject);

/**
 * Increase the number of references to a `CCNxContentObject`.
 *
 * Note that a new `CCNxContentObject` is not created,
 * only that the given `CCNxContentObject` reference count is incremented.
 * Discard the reference by invoking {@link ccnxContentObject_Release}.
 *
 * @param [in] contentObject A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *
 *     CCNxContentObject *reference = ccnxContentObject_Acquire(contentObject);
 *
 *     ...
 *
 *     ccnxContentObject_Release(&reference);
 *
 * }
 * @endcode
 *
 * @see {@link ccnxContentObject_Release}
 */
CCNxContentObject *ccnxContentObject_Acquire(const CCNxContentObject *contentObject);


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
 * @param [in,out] contentObjectP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxContentObject *reference = ccnxContentObject_Acquire(contentObject);
 *
 *     ...
 *
 *     ccnxContentObject_Release(&reference);
 * }
 * @endcode
 *
 * @see {@link ccnxContentObject_Acquire}
 */
void ccnxContentObject_Release(CCNxContentObject **contentObjectP);

#ifdef Libccnx_DISABLE_VALIDATION
#  define ccnxContentObject_OptionalAssertValid(_instance_)
#else
#  define ccnxContentObject_OptionalAssertValid(_instance_) ccnxContentObject_AssertValid(_instance_)
#endif

/**
 * Assert that an instance of `CCNxContentObject` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] contentObject A pointer to the instance to check.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
 *
 *     ccnxContentObject_AssertValid(contentObject);
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 */
void ccnxContentObject_AssertValid(const CCNxContentObject *contentObject);

/**
 * Produce a null-terminated C-string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to the instance.
 *
 * @return NULL  Memory could not be allocated.
 * @return non-NULL A null-terminated string that must be deallocated via `parcMemory_Deallocate`.
 *
 * Example:
 * @code
 * {
 *     ...
 *     char *stringRep = ccnxContentObject_ToString(contentObject);
 *     parcMemory_Deallocate(&stringRep);
 * }
 * @endcode
 * @see `parcMemory_Deallocate`
 */
char *ccnxContentObject_ToString(const CCNxContentObject *contentObject);

/**
 * Determine if two `CCNxContentObject` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxContentObject` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `ccnxContentObject_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `ccnxContentObject_Equals(x, y)` must return true if and only if
 *        `ccnxContentObject_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxContentObject_Equals(x, y)` returns true and
 *        `ccnxContentObject_Equals(y, z)` returns true,
 *        then  `ccnxContentObject_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `ccnxContentObject_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `ccnxContentObject_Equals(x, NULL)` must
 *      return false.
 *
 * @param objectA A pointer to a `CCNxContentObject` instance.
 * @param objectB A pointer to a `CCNxContentObject` instance.
 * @return true if the two `CCNxContentObject` instances are equal.
 *
 * Example:
 * @code
 * @endcode
 */
bool ccnxContentObject_Equals(const CCNxContentObject *objectA, const CCNxContentObject *objectB);

/**
 * Get the ExpiryTime of the specified `ContentObject` instance.
 *
 * The ExpiryTime is the time at which the Payload expires, as expressed by a timestamp containing the number of
 * milliseconds since the epoch in UTC. A cache or end system should not respond with a Content Object past its
 * ExpiryTime. Routers forwarding a Content Object do not need to check the ExpiryTime. If the ExpiryTime field
 * is missing, the Content Object has no expressed expiration and a cache or end system may use the Content
 * Object for as long as it desires.
 *
 * Note: Calling this method on a `CCNxContentObject` that has no expiry time will cause a trap. Before calling,
 * check that an ExpiryTime exists by calling {@link ccnxContentObject_HasExpiryTime} first.
 *
 * @param [in] instance A pointer to a `CCNxContentObject` instance.
 *
 * @return the ExpiryTime of the specified ContentObject.
 *
 * Example:
 * @code
 * {
 *     CCNxContentObject *contentObject = ccnxContentObject_Create<...>;
 *
 *     if (ccnxContentObject_HasExpiryTime(contentObject) {
 *         uint64_t expiryTime = ccnxContentObject_GetExpiryTime(contentObject);
 *     } else {
 *         ccnxContentObject_SetExpiryTime(contentObject, 1478188800000ULL);
 *     }
 * }
 * @endcode
 * @see `ccnxContentObject_HasExpiryTime`
 * @see `ccnxContentObject_SetExpiryTime`
 */
uint64_t ccnxContentObject_GetExpiryTime(const CCNxContentObject *contentObject);

/**
 * Set the ExpiryTime of the specified `ContentObject` instance.
 *
 * See {@link ccnxContentObject_GetExpiryTime} for a description of ExpiryTime.
 *
 * @param [in] instance A pointer to the `CCNxContentObject` instance to be updated.
 * @param [in] expiryTime The ExpiryTime to set.
 *
 * @return true, if the ExpiryTime was succesfully set.
 *
 * Example:
 * @code
 * {
 *     CCNxContentObject *contentObject = ccnxContentObject_Create<...>;
 *
 *     if (ccnxContentObject_HasExpiryTime(contentObject) {
 *         uint64_t expiryTime = ccnxContentObject_GetExpiryTime(contentObject);
 *     } else {
 *         ccnxContentObject_SetExpiryTime(contentObject, 1478188800000ULL);
 *     }
 * }
 * @endcode
 * @see `ccnxContentObject_GetExpiryTime`
 * @see `ccnxContentObject_HasExpiryTime`
 */
bool ccnxContentObject_SetExpiryTime(CCNxContentObject *contentObject, const uint64_t expiryTime);

/**
 * Test whether the specified `ContentObject` instance has an ExpiryTime set.
 *
 * See {@link ccnxContentObject_GetExpiryTime} for a description of ExpiryTime.
 *
 * @param [in] instance A pointer to the `CCNxContentObject` instance to be updated.
 *
 * @return true, if the specified `CCNxContentObject` has an ExpiryTime.
 *
 * Example:
 * @code
 * {
 *     CCNxContentObject *contentObject = ccnxContentObject_Create<...>;
 *
 *     if (ccnxContentObject_HasExpiryTime(contentObject) {
 *         uint64_t expiryTime = ccnxContentObject_GetExpiryTime(contentObject);
 *     } else {
 *         ccnxContentObject_SetExpiryTime(contentObject, 1478188800000ULL);
 *     }
 * }
 * @endcode
 * @see `ccnxContentObject_GetExpiryTime`
 * @see `ccnxContentObject_SetExpiryTime`
 */
bool ccnxContentObject_HasExpiryTime(const CCNxContentObject *contentObject);

uint64_t ccnxContentObject_GetPathLabel(const CCNxContentObject *contentObject);
bool ccnxContentObject_SetPathLabel(CCNxContentObject *contentObject, const uint64_t pathLabel);
bool ccnxContentObject_HasPathLabel(const CCNxContentObject *contentObject);

/**
 * Set a payload on the specified `CCnxContentObject` instance.
 *
 * Currently, a payload may only be set once on a `CCnxContentObject` once, and may not be replaced.
 *
 * @param [in] contentObject A pointer to the `CCNxContentObject` instance to be updated.
 * @param [in] payloadType The type of payload. See {@link CCNxPayloadType} for available types.
 * @param [in] payload A pointer to the {@link PARCBuffer} to assign as the payload..
 *
 * @return true, if the payload was successfully added to the specified `CCNxContentObject`
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
 *
 *     CCNxName *pointerToPayload = ccnxContentObject_GetPayload(contentObject);
 *     if (pointerToPayload == NULL) {
 *         ccnxContentObject_SetPayload(contentObject, CCNxPayloadType_DATA, payload);
 *     }
 *
 *     ...
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 * @see `CCNxPayloadType`
 * @see `PARCBuffer`
 * @see `ccnxContentObject_GetPayload`
 */
bool ccnxContentObject_SetPayload(CCNxContentObject *contentObject, CCNxPayloadType payloadType, const PARCBuffer *payload);

/**
 * Get the {@link CCNxPayloadType} of the specified `CCnxContentObject` instance.
 *
 * @param [in] instance A pointer to the `CCNxContentObject` instance to be updated.
 *
 * @return true, if the payload was successfully added to the specified `CCNxContentObject`
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     PARCBuffer *payload = parcBuffer_Allocate(<...>);
 *
 *     CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
 *
 *     CCNxName *pointerToPayload = ccnxContentObject_GetPayload(contentObject);
 *     if (pointerToPayload == NULL) {
 *         ccnxContentObject_SetPayload(contentObject, CCNxPayloadType_DATA, payload);
 *     }
 *
 *     ...
 *
 *     ccnxContentObject_Release(&contentObject);
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&payload);
 * }
 * @endcode
 * @see `CCNxPayloadType`
 * @see `PARCBuffer`
 * @see `ccnxContentObject_GetPayload`
 */
CCNxPayloadType ccnxContentObject_GetPayloadType(const CCNxContentObject *contentObject);
#endif // libccnx_ccnx_ContentObject_h
