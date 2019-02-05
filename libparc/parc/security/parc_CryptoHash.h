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
 * @file parc_CryptoHash.h
 * @ingroup security
 *
 * @brief PARCDigest holds a cryptographic digest, which is comprised
 * the bytes of the digest and the algorithm used for the digest.
 *
 */
#ifndef libparc_parc_CryptoHash_h
#define libparc_parc_CryptoHash_h

#include <parc/algol/parc_Buffer.h>
#include <parc/security/parc_CryptoHashType.h>

struct parc_crypto_hash;
typedef struct parc_crypto_hash PARCCryptoHash;

/**
 * Create a Digest holding the type and digest buffer.
 *
 * Creates a new reference to the given PARCBuffer `digest`.
 *
 * @param [in] digestType The type of hash digest algorithm used to compute this digest.
 * @param [in] digestBuffer The actual hash digest instance.
 *
 * @return A newly allocated `PARCCryptoHash` instance that must be freed via `parcCryptoHash_Release()`
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *digestBuffer = ...
 *     PARCCryptoHash *digest = parcCryptoHash_Create(PARCCryptoHashType_SHA256, digestBuffer);
 *     ...
 *     parcCryptoHash_Release(&digest);
 * }
 * @endcode
 */
PARCCryptoHash *parcCryptoHash_Create(PARCCryptoHashType digestType, const PARCBuffer *digestBuffer);

/**
 * Increase the number of references to a `PARCCryptoHash` instance.
 *
 * Note that a new `PARCCryptoHash` is not created,
 * only that the given `PARCCryptoHash` reference count is incremented.
 * Discard the reference by invoking {@link parcCryptoHash_Release}.
 *
 * @param [in] hash A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCCryptoHash *x = ...
 *     PARCCryptoHash *x2 = parcCryptoHash_Acquire(x);
 *
 *     parcCryptoHash_Release(&x);
 *     parcCryptoHash_Release(&x2);
 * }
 * @endcode
 *
 * @see {@link parcCryptoHash_Release}
 */
PARCCryptoHash *parcCryptoHash_Acquire(const PARCCryptoHash *hash);

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
 * @param [in,out] hashP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCCryptoHash *x = ...
 *
 *     parcCryptoHash_Release(&x);
 * }
 * @endcode
 *
 * @see {@link parcCryptoHash_Acquire}
 */
void parcCryptoHash_Release(PARCCryptoHash **hashP);

/**
 * Create a digest, copying the buffer.
 *
 * @param [in] digestType The type of hash digest algorithm used to compute this digest.
 * @param [in] buffer Pointer to array containing the raw digest bytes.
 * @param [in] length Length of the digest byte array.
 *
 * @return A newly allocated `PARCCryptoHash` instance that must be freed via `parcCryptoHash_Release()`
 *
 * Example:
 * @code
 * {
 *     size_t bufferLen = 32;
 *     uint8_t *buffer = ...
 *     PARCCryptoHash *digest = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer, bufferLen);
 *     ...
 *     parcCryptoHash_Release(&digest);
 *     // free the raw buffer as needed
 * }
 * @endcode
 */
PARCCryptoHash *parcCryptoHash_CreateFromArray(PARCCryptoHashType digestType, const void *buffer, size_t length);

/**
 * Destroy the specified `PARCCryptoHash` instance.
 *
 * @param [in,out] parcDigestPtr Pointer to the instance to be destroyed.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = ...
 *     PARCCryptoHash *digest = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
 *     ...
 *     parcCryptoHash_Release(&digest);
 * }
 * @endcode
 */
void parcCryptoHash_Release(PARCCryptoHash **parcDigestPtr);

/**
 * Returns the digest algorithm, of type `PARCCryptoHashType`.
 *
 * @param [in] parcDigest The `PARCCryptoHash` instance being examined.
 *
 * @return A `PARCCryptoHashType` value.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = ...
 *     PARCCryptoHash *digest = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
 *     PARCCryptoHashType type = parcCryptoHash_GetDigestType(digest);
 *     // type will be PARCCryptoHashType_SHA256
 *     ...
 *     parcCryptoHash_Release(&digest);
 * }
 * @endcode
 */
PARCCryptoHashType parcCryptoHash_GetDigestType(const PARCCryptoHash *parcDigest);

/**
 * Returnes the digest buffer, copy if you will destroy
 *
 *   Returns the inner digest buffer. You must copy it if you will make
 *   changes or destroy it.
 *
 * @param [in] cryptoHash The `PARCCryptoHash` instance being examined.
 *
 * @return A `PARCBuffer` instance containing the raw hash digest.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = ...
 *     PARCCryptoHash *digest = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
 *     PARCBuffer *rawDigest = parcCryptoHash_GetDigest(digest);
 *     // use the raw digest as necessary
 *     ...
 *     parcCryptoHash_Release(&digest);
 * }
 * @endcode
 */
PARCBuffer *parcCryptoHash_GetDigest(const PARCCryptoHash *cryptoHash);

/**
 * Determine if two PARCCryptoHash instances are equal.
 *
 * The following equivalence relations on non-null `PARCCryptoHash` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcCryptoHash_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcCryptoHash_Equals(x, y)` must return true if and only if
 *        parcCryptoHash_Equals(y x) returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcCryptoHash_Equals(x, y)` returns true and
 *        `parcCryptoHash_Equals(y, z)` returns true,
 *        then  `parcCryptoHash_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcCryptoHash_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcCryptoHash_Equals(x, NULL)` must return false.
 *
 * @param [in] a A pointer to a PARCCryptoHash instance.
 * @param [in] b A pointer to a PARCCryptoHash instance.
 *
 * @return True if the given PARCCryptoHash instances are equal, false otherwise.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = ...
 *     PARCCryptoHash *orig = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
 *     PARCCryptoHash *copy = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
 *
 *     if (parcCryptoHash_Equals(orig, copy)) {
 *         // true
 *     } else {
 *         // false
 *     }
 *
 *     parcCryptoHash_Release(&orig);
 *     parcCryptoHash_Release(&copy);
 * }
 * @endcode
 */
bool parcCryptoHash_Equals(const PARCCryptoHash *a, const PARCCryptoHash *b);
#endif // libparc_parc_CryptoHash_h
