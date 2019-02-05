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
 * @file parc_Key.h
 * @ingroup security
 * @brief A `PARCKey` encapsulates a raw public (asymmetric) or private (symmetric) key.
 *
 * The PARC security library supports both public (asymmetric) digital signature and
 * private (symmetric) MAC algorithms. A key is used in each such scheme for computing
 * the signature or MAC. This type encapsulates both the raw key used in such schemes, but also
 * a KeyId used to identify the key for hash-based data structures and the target signing/MAC
 * scheme to which the key is applied.
 *
 */
#include <parc/security/parc_KeyId.h>

#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_SigningAlgorithm.h>

#include <parc/algol/parc_Buffer.h>

#ifndef libparc_parc_Key_h
#define libparc_parc_Key_h

struct parc_key;
typedef struct parc_key PARCKey;

/**
 * Increase the number of references to an instance of this object.
 *
 * Note that new instance is not created, only that the given instance's reference count
 * is incremented. Discard the reference by invoking `parcKey_Release()`.
 *
 * @param [in] key A pointer to the original instance.
 *
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCKey *key = parcKey_Acquire(keyInstance);
 *
 *     parcKey_Release(&key);
 * }
 * @endcode
 *
 * @see parcKey_Release
 */
PARCKey *parcKey_Acquire(const PARCKey *key);

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
 * @param [in] keyPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCKey *key = parcKey_Acquire(keyInstance);
 *
 *     parcKey_Release(&key);
 * }
 * @endcode
 */
void parcKey_Release(PARCKey **keyPtr);

/**
 * Check that the `PARCKey` instance is valid. It should be non-null,
 * and any referenced data should also be valid.
 *
 * @param [in] key A pointer to the instance to check.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCKey *key = parcKey_Acquire(keyInstance);
 *
 *     parcKey_AssertValid(key);
 * }
 * @endcode
 */
void parcKey_AssertValid(PARCKey *key);

/**
 * Create a Key for use with the specified signing algorithm.
 *
 * This method supports Public Key algorithms. For such algorithms,
 * the buffer should be a DER encoded key.
 *
 * @param [in] keyid A `PARCKeyId` instance for the new key
 * @param [in] signingAlg The signing algorithm which is to be associated with this key
 * @param [in] derEncodedKey The raw, DER-encoded key
 *
 * @return NULL An error occurred
 * @return non-NULL A pointer to a new PARCKey instance that must be deallocated via `parcKey_Destory()`
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *derEncodedKey = ....;
 *      PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, derEncodedKey);
 *      parcBuffer_Release(&derEncodedKey);
 *      parcKeyId_Destroy(&keyid);
 *      // do stuff with key
 *      parcKey_Release(&key);
 * }
 */
PARCKey *parcKey_CreateFromDerEncodedPublicKey(PARCKeyId *keyid, PARCSigningAlgorithm signingAlg, PARCBuffer *derEncodedKey);

/**
 * Create a Key for use with the specified signing algorithm.
 *
 * This method support HMAC with symmetric keys.
 *
 * The secretkey is a set of random bytes.
 *
 * @param [in] keyid A pointer to a PARCKeyId instance.
 * @param [in] signingAlg A PARCSigningAlgorithm value (only MAC-like algorithms allowed)
 * @param [in] secretkey A pointer to a PARCBuffer instance containing the secret key.
 *
 * @return NULL An error occurred
 * @return non-NULL A pointer to a new PARCKey instance that must be deallocated via `parcKey_Destory()`
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *symmetricKey = ....;
 *      PARCKey *key = parcKey_CreateFromSymmetricKey(keyid, PARCSigningAlgorithm_HMAC, symmetricKey);
 *      parcBuffer_Release(&symmetricKey);
 *      parcKeyId_Destroy(&keyid);
 *      // do stuff with key
 *      parcKey_Release(&key);
 * }
 * @endcode
 *
 * @see parcKey_Destory
 */
PARCKey *parcKey_CreateFromSymmetricKey(PARCKeyId *keyid, PARCSigningAlgorithm signingAlg, PARCBuffer *secretkey);

/**
 * Create an independent, deep-copy of the given instance.
 *
 * A new instance is created as a complete,
 * independent copy of the original such that `Equals(original, copy) == true`.
 *
 * @param [in] key A pointer to a `PARCKey` instance.
 *
 * @return NULL An error occurred
 * @return non-NULL A pointer to a new PARCKey instance that must be deallocated via `parcKey_Release()`
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *derEncodedKey = ....;
 *      PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, derEncodedKey);
 *      parcBuffer_Release(&derEncodedKey);
 *      parcKeyId_Destroy(&keyid);
 *      PARCKey *copy = parcKey_Copy(key);
 *      // do stuff with the key copy, which is equals to the original key instance
 *      parcKey_Release(&key);
 *      parcKey_Release(&copy);
 * }
 * @endcode
 */
PARCKey *parcKey_Copy(const PARCKey *key);

/**
 * Determine if two `PARCKey` instances are equal.
 * Two instances of PARCKey are equal iff the key digests are equal,
 * the signing algorithms are equal, and the keys are equal.
 *
 * Two NULL keys are equal, but NULL does not equal any non-NULL
 *
 * The following equivalence relations on non-null `XXX` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `Equals(x, y)` must return true if and only if
 *        `Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `Equals(x, y)` returns true and
 *        `Equals(y, z)` returns true,
 *        then `Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `Equals(x, NULL)` must return false.
 *
 * @param [in] keyA A pointer to a `PARCKey` instance.
 * @param [in] keyB A pointer to a `PARCKey` instance.
 *
 * @return true `PARCKey` keyA and keyB are equal.
 * @return false `PARCKey` keyA and keyB are not equal.
 *
 * Example:
 * @code
 * {
 *     PARCKey *keyA = parcKey_Create(...);
 *     PARCKey *keyB = parcKey_Create(...);
 *
 *     if (parcBuffer_Equals(keyA, keyB)) {
 *         printf("Keys are equal.\n");
 *     } else {
 *         printf("Keys are NOT equal.\n");
 *     }
 *
 *     parcKey_Release(&bufferA);
 *     parcKey_Release(&bufferB);
 * }
 * @endcode
 */
bool parcKey_Equals(const PARCKey *keyA, const PARCKey *keyB);

/**
 * Retrieve the `PARCKeyId` associated with the specified `PARCKey` instance.
 *
 * You must Aqcuire your own reference if you will store the key.
 * Do not release this instance.
 *
 * @param [in] key A pointer to the `PARCKey` instance
 *
 * @return PARCKeyId A pointer to the `PARCKeyId` associated with this `PARCKey` instance. A handle is not acquired.
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *derEncodedKey = ....;
 *      PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, derEncodedKey);
 *      parcBuffer_Release(&derEncodedKey);
 *      parcKeyId_Destroy(&keyid);
 *      PARCKeyId *innerKeyId = parcKey_GetKeyId(key);
 *      // use the innerKeyId as needed - DO NOT RELEASE IT via parcKeyId_Release()
 * }
 * @endcode
 */
PARCKeyId *parcKey_GetKeyId(const PARCKey *key);

/**
 * Retrieve the `PARCSigningAlgorithm` associated with the specified `PARCKey` instance.
 *
 * @param [in] key A pointer to the `PARCKey` instance
 *
 * @return PARCSigningAlgorithm A PARCSigningAlgorithm value
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *derEncodedKey = ....;
 *      PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, derEncodedKey);
 *      parcBuffer_Release(&derEncodedKey);
 *      parcKeyId_Destroy(&keyid);
 *      PARCSigningAlgorithm signingAlg = parcKey_GetSigningAlgorithm(key);
 *      // use the signingAlg value as needed
 * }
 * @endcode
 */
PARCSigningAlgorithm parcKey_GetSigningAlgorithm(const PARCKey *key);

/**
 * Returns the key's instance of the key buffer.
 *
 * You must Aqcuire your own reference if you will store the key.
 * Do not release this instance.
 *
 * @param [in] key A pointer to the `PARCKey` instance
 *
 * @return PARCBuffer A pointer to the `PARCBuffer` associated with this `PARCKey` instance. A handle is not acquired.
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *derEncodedKey = ....;
 *      PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, derEncodedKey);
 *      parcBuffer_Release(&derEncodedKey);
 *      parcKeyId_Destroy(&keyid);
 *      PARCBuffer *innerKey = parcKey_GetKey(key);
 *      // use the innerKey as needed - DO NOT RELEASE IT via parcBuffer_Release()
 * }
 * @endcode
 */
PARCBuffer *parcKey_GetKey(const PARCKey *key);

/**
 * Create a null-terminated string representation of the given `PARCKey`.
 *
 * The returned value must be freed by the caller using {@link parcMemory_Deallocate()}.
 *
 * @param [in] link  A pointer to a `PARCKey` instance.
 * @return A pointer to null-terminated string of characters that must be freed by the caller by `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
char *parcKey_ToString(const PARCKey *key);
#endif // libparc_parc_Key_h
