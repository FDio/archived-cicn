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
 * @file ccnx_Interest.h
 * @ingroup Interest
 * @brief A CCNx Interest expresses an interest in a piece of named data.
 *
 * The canonical CCN Interest. An Interest contains a {@link CCNxName},
 * the desired payload, and two optional restrictions (ContentObjectHash and
 * KeyId) to limit responses
 * to a specific publisher or a specific Content Object.
 *
 * @see {@link CCNxContentObject}
 * @see {@link CCNxName}
 *
 */

#ifndef libccnx_ccnx_Interest_h
#define libccnx_ccnx_Interest_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/internal/ccnx_InterestInterface.h>

/**
 * @typedef CCNxInterest
 * @brief The CCNx Interest Message
 */

typedef CCNxTlvDictionary CCNxInterest;

/**
 * Create a new instance of `CCNxInterest` for the specified name, with the specified lifetime and
 * publisher's key digest, using the specified {@link CCNxInterestInterface}, using dynamically allocated memory.
 *
 * The name and key digest are used for matching against ContentObjects. If the specified key digest is NULL,
 * this Interest will match against any ContentObject with the same name (regardless of whether the
 * ContentObject has matching key digest). If the specified key digest is NOT NULL, then this Interest
 * will match only against any ContentObject with both the same name and the same key digest. The key digest
 * comparison test is a simple buffer comparison.
 *
 * The lifetime, specified in milliseconds, is a hint to the system how long the application is
 * willing to wait for a response before it will re-send the same Interest. It is used by forwarders as a
 * guideline on how long they should attempt to keep the Interest alive in their tables. It is not a guarantee
 * that they will do so, however.
 *
 * The created instance of `CCNxInterest` must be released by calling {@link ccnxInterest_Release}().
 *
 * @param [in] implementation A pointer to the {@link CCNxInterestInterface} to be used to build this Interest.
 * @param [in] name A pointer to a {@link CCNxName} expressing the name of the content you are interested in.
 * @param [in] lifetime A `uint32_t` specifying the number of milliseconds the application
 *             will wait before re-sending the Interest.
 * @param [in] keyId A pointer to a {@link PARCBuffer} containing a key digest that should be matched
 *             against ContentObjects whose names match our specified name. This value may be NULL and, if so, will
 *             not be used in the matching process.
 * @param [in] contentObjectHash A pointer to a `PARCBuffer` containing the hash of the ContentObject that is expected
 *             to be returned in response to this Interest.
 *
 * @return A new instance of a `CCNxInterest`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/temp/2");
 *
 *     PARCBuffer *keyId = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(keyId, 1234L);
 *
 *     uint32_t lifetime = CCNxInterestDefaultLifetimeMilliseconds;
 *
 *     CCNxInterest *interest = ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
 *                                                          name, lifetime, keyId, NULL);
 *
 *     ...
 *
 *     parcBuffer_Release(&keyId);
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interest);
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_Release}
 */
CCNxInterest *ccnxInterest_CreateWithImpl(const CCNxInterestInterface *implementation,
                                          const CCNxName *name,
                                          const uint32_t interestLifetime,
                                          const PARCBuffer *keyId,
                                          const PARCBuffer *contentObjectHash,
                                          const uint32_t hopLimit);

/**
 * Create a new instance of `CCNxInterest` for the specified name, with the specified lifetime and
 * publisher's key digest, using dynamically allocated memory.
 *
 * The name and key digest are used for matching against ContentObjects. If the specified key digest is NULL,
 * this Interest will match against any ContentObject with the same name (regardless of whether the
 * ContentObject has matching key digest). If the specified key digest is NOT NULL, then this Interest
 * will match only against any ContentObject with both the same name and the same key digest. The key digest
 * comparison test is a simple buffer comparison.
 *
 * The lifetime, specified in milliseconds, is a hint to the system how long the application is
 * willing to wait for a response before it will re-send the same Interest. It is used by forwarders as a
 * guideline on how long they should attempt to keep the Interest alive in their tables. It is not a guarantee
 * that they will do so, however.
 *
 * The created instance of `CCNxInterest` must be released by calling {@link ccnxInterest_Release}().
 *
 * @param [in] name A pointer to a {@link CCNxName} expressing the name of the content you are interested in.
 * @param [in] lifetime A `uint32_t` specifying the number of milliseconds the application
 *             will wait before re-sending the Interest.
 * @param [in] keyId A pointer to a {@link PARCBuffer} containing a key digest that should be matched
 *             against ContentObjects whose names match our specified name. This value may be NULL and, if so, will
 *             not be used in the matching process.
 * @param [in] contentObjectHash A pointer to a `PARCBuffer` containing the hash of the ContentObject that is expected
 *             to be returned in response to this Interest.
 *
 * @return A new instance of a `CCNxInterest`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/temp/2");
 *
 *     PARCBuffer *keyId = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(keyId, 1234L);
 *
 *     uint32_t lifetime = CCNxInterestDefaultLifetimeMilliseconds;
 *
 *     CCNxInterest *interest = ccnxInterest_Create(name, lifetime, keyId, NULL);
 *
 *     ...
 *
 *     parcBuffer_Release(&keyId);
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interest);
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_Release}
 */
CCNxInterest *ccnxInterest_Create(const CCNxName *name,
                                  uint32_t lifetime,
                                  const PARCBuffer *keyId,
                                  const PARCBuffer *contentObjectHash);


/**
 * Create a new instance of `CCNxInterest` for the specified name, using dynamically allocated memory.
 *
 * The created instance will specify a default lifetime, and will not specify a publisher's key
 * digest for matching. Only the specified name will be used for matching. To specify values
 * for the lifetime or publisher's key digest, see {@link ccnxInterest_Create()}.
 *
 * The created instance of `CCNxInterest` must be released by calling {@link ccnxInterest_Release}.
 *
 * @param [in] name A pointer to a `CCNxName` expressing the name of the content you are interested in.
 *
 * @return A new instance of a `CCNxInterest'.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/humidity/4");
 *
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interest);
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_Create}
 * @see {@link ccnxInterest_Release}
 */
CCNxInterest *ccnxInterest_CreateSimple(const CCNxName *name);

/**
 * Determine if two `CCNxInterest` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxInterest` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `ccnxInterest_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `ccnxInterest_Equals(x, y)` must return true if and only if
 *        `ccnxInterest_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxInterest_Equals(x, y)` returns true and
 *        `ccnxInterest_Equals(y, z)` returns true,
 *        then  `ccnxInterest_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `ccnxInterest_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `ccnxInterest_Equals(x, NULL)` must
 *      return false.
 *
 * @param interestA A pointer to a `CCNxInterest` instance.
 * @param interestB A pointer to a `CCNxInterest` instance.
 * @return true if the two `CCNxInterest` instances are equal.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     uint32_t lifetime = 15 * 1000; // 15 seconds, in milliseconds
 *
 *     PARCBuffer *keyDigest1 = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(keyDigest1, 1234L);
 *
 *     PARCBuffer *keyDigest2 = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(keyDigest2, 6789L); // different buffer contents than keyDigest1
 *
 *     CCNxInterest *interestA = ccnxInterest_Create(name, lifetime, keyDigest1, NULL);
 *     CCNxInterest *interestB = ccnxInterest_Create(name, lifetime, keyDigest1, NULL); // same as A
 *
 *     CCNxInterest *interestC = ccnxInterest_Create(name, lifetime, keyDigest2); // different key digest
 *
 *     if (ccnxInterest_Equals(interestA, interestB)) {
 *         // this is expected...
 *     }
 *
 *     if (ccnxInterest_Equals(interestA, interestC)) {
 *         // this is NOT expected
 *     }
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&keyDigest1);
 *     parcBuffer_Release(&keyDigest2);
 *     ccnxInterest_Release(&interestA);
 *     ccnxInterest_Release(&interestB);
 *     ccnxInterest_Release(&interestC);
 * }
 * @endcode
 */
bool ccnxInterest_Equals(const CCNxInterest *interestA, const CCNxInterest *interestB);

/**
 * Return a pointer to the {@link CCNxName} associated with the given `CCNxInterest`.
 *
 * The pointer points to memory managed by the `CCNxInterest` and does not have to
 * be released unless {@link ccnxName_Acquire()} is called to acquire it.
 *
 * @param [in] interest A pointer to a `CCNxInterest` instance.
 *
 * @return A pointer to the CCNxName associated with the specified `CCNxInterest`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxInterest_GetName(interest);
 * }
 * @endcode
 *
 * @see {@link CCNxName}
 */
CCNxName *ccnxInterest_GetName(const CCNxInterest *interest);

/**
 * Assign a lifetime value to the specified `CCNxInterest`.
 *
 * The lifetime, specified in milliseconds, is a hint to the system how long the application is
 * willing to wait for a response before it will re-send the same Interest. It is used by forwarders as a
 * guideline on how long they should attempt to keep the Interest alive in their tables. It is not a guarantee
 * that they will do so, however.
 *
 * @param [in] interest A pointer to a `CCNxInterest` instance.
 * @param [in] lifetime A `uint32_t` containing the desired lifetime of this Interest, in milliseconds.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/Ab00se");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     uint32_t lifetime = 15 * 1000; // 15 seconds, in milliseconds
 *     ccnxInterest_SetLifetime(interest, lifetime);
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interest);
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_GetLifetime}
 */
bool ccnxInterest_SetLifetime(CCNxInterest *interest, uint32_t lifetime);

/**
 * Retrieve the specified `CCNxInterest`'s lifetime value as a uint32_t.
 * @param [in] interest A pointer to a `CCNxInterest` instance.
 *
 * @return The lifetime, in milliseconds, specified for this `CCNxInterest`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/b00se");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     uint32_t lifetime = ccnxInterest_GetLifetime(interest);
 *     ...
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interest);
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_SetLifetime}
 */
uint32_t ccnxInterest_GetLifetime(const CCNxInterest *interest);

/**
 * Assign a key ID to the specified `CCNxInterest`.
 *
 * The key ID is used when matching this `CCNxInterest` to CCNx content. If a non-NULL
 * key ID is specified in the `CCNxInterest`, then only content with a matching key
 * digest will be matched. If the `CCNxInterest`'s key ID is NULL, then it is not used
 * when matching against content.
 *
 * @param [in,out] interest A pointer to a `CCNxInterest` instance.
 * @param [in] keyId A pointer to a {@link PARCBuffer} containing the desired key ID, or NULL.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/r00");
 *
 *     PARCBuffer *keyId = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(keyId, 1234L);
 *
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     ccnxInterest_SetKeyIdRestriction(interest, keyId);
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interest);
 *     parcBuffer_Release(&keyId);
 * }
 * @endcode
 *
 * @see ccnxInterest_GetKeyId
 */
bool ccnxInterest_SetKeyIdRestriction(CCNxInterest *interest, const PARCBuffer *keyId);

/**
 * Return a pointer to the {@link PARCBuffer} containing the publisher key digest associated with the given `CCNxInterest`.
 *
 * The pointer points to memory managed by the `CCNxInterest` and does not have to
 * be released unless {@link parcBuffer_Acquire()} is called to acquire it.
 *
 * @param [in] interest A pointer to a `CCNxInterest` instance.
 *
 * @return  A pointer to a PARCBuffer containing the `CCNxInterest`'s key ID.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/p1e");
 *
 *     PARCBuffer *keyId = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(keyId, 1234L);
 *     uint32_t lifetime = 5000;
 *
 *     CCNxInterest *interest = ccnxInterest_Create(name, lifetime, keyId, NULL);
 *
 *     PARCBuffer *keyIdP = ccnxInterest_GetKeyIdRestriction(interest);
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&keyId);
 *     ccnxInterest_Release(&interest);
 * }
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_SetKeyId}
 */
PARCBuffer *ccnxInterest_GetKeyIdRestriction(const CCNxInterest *interest);

/**
 * Assign a `ContentObjectHash` to the specified `CCNxInterest`.
 *
 * The `ContentObjectHash` is used when performing an exact match against this `CCNxInterest` and
 * the response CCNx content. If a non-NULL `ContentObjectHash` is specified in the `CCNxInterest`,
 * then only content whose hash digest matches the specified value will match and be returned. If
 * the `ContentObjectHash` field is NULL, this binary equality check is not used when checking a response
 * Content Object.
 *
 * @param [in,out] interest A pointer to a `CCNxInterest` instance.
 * @param [in] contentObjectHash A pointer to a {@link PARCBuffer} containing the desired ContentObjectHash, or NULL.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/r00");
 *
 *     PARCBuffer *contentObjectHash = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(contentObjectHashcontentObjectHash, 1234L);
 *
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     ccnxInterest_SetContentObjectHashRestriction(interest, contentObjectHash);
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interest);
 *     parcBuffer_Release(&contentObjectHash);
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_GetContentObjectHash}
 */
bool ccnxInterest_SetContentObjectHashRestriction(CCNxInterest *interest, const PARCBuffer *contentObjectHash);

/**
 * Return a pointer to the {@link PARCBuffer} containing the `ContentObjectHash` associated with the given `CCNxInterest`.
 *
 * The pointer points to memory managed by the `CCNxInterest` and does not have to
 * be released unless {@link parcBuffer_Acquire()} is called to acquire it.
 *
 * @param [in] interest A pointer to a `CCNxInterest` instance.
 *
 * @return  A pointer to a `PARCBuffer` containing the `CCNxInterest`'s `ContentObjectHash`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/p1e");
 *
 *     uint32_t lifetime = 3 * 1000; // milliseconds
 *
 *     PARCBuffer *contentObjectHash = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(contentObjectHash, 1234L);
 *
 *
 *     CCNxInterest *interest = ccnxInterest_Create(name, lifetime, NULL, contentObjectHash);
 *
 *     PARCBuffer *contentObjectHashP = ccnxInterest_GetContentObjectHashRestriction(interest);
 *
 *     ...
 *
 *     ccnxName_Release(&name);
 *     parcBuffer_Release(&contentObjectHash);
 *     ccnxInterest_Release(&interest);
 * }
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_SetContentObjectHash}
 */
PARCBuffer *ccnxInterest_GetContentObjectHashRestriction(const CCNxInterest *interest);

/**
 * Produce a null-terminated string representation of the specified instance.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate()}.
 *
 * @param [in] interest A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, nul-terminated C string that must be deallocated via `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     char *string = ccnxInterest_ToString(interest);
 *
 *     if (string != NULL) {
 *         printf("Interest looks like: %s\n", string);
 *         parcMemory_Deallocate(string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 *
 * @see ccnxInterest_Display
 * @see parcMemory_Deallocate
 */
char *ccnxInterest_ToString(const CCNxInterest *interest);

/**
 * Print a human readable representation of the given `CCNxInterest`.
 *
 * @param [in] interest A pointer to the instance to display.
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     ccnxInterest_Display(interest, 0);
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 * */
void ccnxInterest_Display(const CCNxInterest *interest, int indentation);

/**
 * Increase the number of references to a `CCNxInterest`.
 *
 * Note that a new `CCNxInterest` is not created,
 * only that the given `CCNxInterest` reference count is incremented.
 * Discard the reference by invoking {@link ccnxInterest_Release}.
 *
 * @param [in] instance A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxInterest *reference = ccnxInterest_Acquire(interest);
 *
 *     ...
 *
 *     ccnxInterest_Release(&reference);
 *
 * }
 * @endcode
 *
 * @see ccnxInterest_Release
 */
CCNxInterest *ccnxInterest_Acquire(const CCNxInterest *instance);

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
 * @param [in,out] instanceP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxInterest *reference = ccnxInterest_Acquire(contentObject);
 *
 *     ...
 *
 *     ccnxInterest_Release(&reference);
 * }
 * @endcode
 *
 * @see {@link ccnxInterest_Acquire}
 */
void ccnxInterest_Release(CCNxInterest **instanceP);

#ifdef Libccnx_DISABLE_VALIDATION
#  define ccnxInterest_OptionalAssertValid(_instance_)
#else
#  define ccnxInterest_OptionalAssertValid(_instance_) ccnxInterest_AssertValid(_instance_)
#endif
/**
 * Assert that an instance of `CCNxInterest` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] interest A pointer to the instance to check.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     ccnxInterest_AssertValid(interest);
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 */
void ccnxInterest_AssertValid(const CCNxInterest *interest);


/**
 * Set the payload on an `CCnxInterest` instance. A reference to the supplied payload is
 * acquired by the Interest, so the caller may release the payload after setting it on
 * the CCnxInterest. The payload type must be specified, and must be one of the types
 * defined in {@link CCNxPayloadType}.
 *
 * This will not append a payloadId segment to the Interest's name. If you want to do that,
 * see {@link ccnxInterest_SetPayloadAndId} and {@link ccnxInterest_SetPayloadWithId}.
 *
 * @param [in] interest A pointer to the `CCNxInstance` instance to update.
 * @param [in] payload A pointer to the PARCBuffer payload to be applied to the `CCNxInterest`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     PARCBuffer *payload = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(payload, 5432L);
 *
 *     ccnxInterest_SetPayload(interest, payload);
 *
 *     parcBuffer_Release(&payload);
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 * @see `CCNxPayloadType`
 */
bool ccnxInterest_SetPayload(CCNxInterest *interest, const PARCBuffer *payload);

/**
 * Set the payload on an `CCnxInterest` instance. A reference to the supplied payload is
 * acquired by the Interest, so the caller may release the payload after setting it on
 * the CCnxInterest. The payload type must be specified, and must be one of the types
 * defined in {@link CCNxPayloadType}.
 *
 * This will also generate and append a payloadId name segment to the interest name using a
 * sha256 hash (the default) of the payload buffer. If you'd like to take responsibility
 * for generating a payloadId, use {@link ccnxInterest_SetPayloadAndId} instead.
 *
 * @param [in] interest A pointer to the `CCNxInstance` instance to update.
 * @param [in] payload A pointer to the PARCBuffer payload to be applied to the `CCNxInterest`.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     PARCBuffer *payload = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(payload, 5432L);
 *
 *     ccnxInterest_SetPayload(interest, payload);
 *
 *     parcBuffer_Release(&payload);
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 * @see `CCNxPayloadType`
 */
bool ccnxInterest_SetPayloadAndId(CCNxInterest *interest, const PARCBuffer *payload);


/**
 * Set the payload and specified payloadId on an `CCnxInterest` instance. A reference to the
 * supplied payload is acquired by the Interest, so the caller may release the payload after
 * setting it on the CCnxInterest. The payload type must be specified, and must be one of the types
 * defined in {@link CCNxPayloadType}.
 *
 * This will append the supplied payloadId to the interest name, supplying a NULL payloadId will result
 * in not payloadId being appended to the interest name. See {@link CCNxInterestPayloadId}
 *
 * @param [in] interest A pointer to the `CCNxInstance` instance to update.
 * @param [in] payload A pointer to the PARCBuffer payload to be applied to the `CCNxInterest`.
 * @param [in] payloadId A pointer to a CCNxInterestPayloadId to use instead of the default. The value may
 * be NULL of no payload id is desired.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     PARCBuffer *payload = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(payload, 5432L);
 *
 *     CCNxInterestPayloadId *payloadId = ccnxInterestPayloadId_CreateAsSHA256Hash(payload);
 *     ccnxInterest_SetPayloadWithId(interest, payload, payloadId);
 *     ccnxInterestPayloadId_Release(&payloadId);
 *
 *     parcBuffer_Release(&payload);
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 * @see `CCNxPayloadType`
 */
bool ccnxInterest_SetPayloadWithId(CCNxInterest *interest, const PARCBuffer *payload, const CCNxInterestPayloadId *payloadId);


/**
 * Return a pointer to the payload attached to the specified `CCNxInterest`, if any. It is up to the
 * caller to acquire a reference to the payload if necessary.
 *
 * @param [in] interest A pointer to the `CCNxInstance` instance to update.
 * @return A pointer to the payload buffer attached to the supplied `CCNxInterest` instance. Will be NULL
 *         if the `CCNxInterest` instance has no payload.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     PARCBuffer *payload = parcBuffer_Allocate(8);
 *     parcBuffer_PutUint64(payload, 5432L);
 *
 *     ccnxInterest_SetPayload(interest, CCNxPayloadType_DATA, payload);
 *
 *     PARCBuffer *reference = ccnxInterest_GetPayload(interest);
 *
 *     parcBuffer_Release(&payload);
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 */
PARCBuffer *ccnxInterest_GetPayload(const CCNxInterest *interest);

/**
 * Set the Hop Limit for the specified `CCNxInterest` instance.
 * The Interest HopLimit element is a counter that is decremented with each hop. It limits the distance an Interest may travel.
 * The node originating the Interest may put in any value - up to the maximum - in network byte order. Each node that receives
 * an Interest with a HopLimit decrements the value upon reception. If the value is 0 after the decrement, the Interest cannot
 * be forwarded off the node.
 *
 * @param [in] interest A pointer to a `CCNxInterest` instance to update.
 * @param [in] hopLimit The hop limit to set.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     ccnxInterest_SetHopLimit(interest, 30);
 *
 *     uint32_t hopLimit = ccnxInterest_GetHopLimit(interest);
 *     assertTrue(hopLimit = 30, "Expected 30 for the hopLimit");
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 * @see `ccnxInterest_GetHopLimit`
 */
bool ccnxInterest_SetHopLimit(CCNxInterest *interest, uint32_t hopLimit);

/**
 * Get the Hop Limit for the specified `CCNxInterest` instance.
 * The Interest HopLimit element is a counter that is decremented with each hop. It limits the distance an Interest may travel.
 * The node originating the Interest may put in any value - up to the maximum - in network byte order. Each node that receives
 * an Interest with a HopLimit decrements the value upon reception. If the value is 0 after the decrement, the Interest cannot
 * be forwarded off the node.
 *
 * @param [in] interest A pointer to the `CCNxInterest` from which to retrieve the Hop Limit
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/sensors/radiation/11");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     ccnxInterest_SetHopLimit(interest, 30);
 *
 *     uint32_t hopLimit = ccnxInterest_GetHopLimit(interest);
 *     assertTrue(hopLimit = 30, "Expected 30 for the hopLimit");
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&instance);
 * }
 * @endcode
 * @see `ccnxInterest_SetHopLimit`
 */
uint32_t ccnxInterest_GetHopLimit(const CCNxInterest *interest);
#endif // libccnx_ccnx_Interest_h
