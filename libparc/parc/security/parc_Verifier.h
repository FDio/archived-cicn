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
 * @file parc_Verifier.h
 * @ingroup security
 * @brief  Structures and functions to support verification.
 *
 */
#ifndef libparc_parc_Verifier_h
#define libparc_parc_Verifier_h

#include <parc/algol/parc_Object.h>

#include <parc/security/parc_CryptoHasher.h>
#include <parc/security/parc_Signature.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_Key.h>
#include <parc/security/parc_CryptoSuite.h>

struct parc_verifier;
typedef struct parc_verifier PARCVerifier;

/**
 * @typedef PARCVerifierInterface
 * @brief The interface for `PARCVerifier`
 */
typedef struct parc_verifier_interface {
    /** @see parcVerifier_GetCryptoHasher */
    PARCCryptoHasher *(*GetCryptoHasher)(PARCObject * interfaceContext, PARCKeyId * keyid, PARCCryptoHashType hashType);

    /** @see parcVerifier_VerifyDigest */
    bool (*VerifyDigest)(PARCObject *interfaceContext, PARCKeyId *keyid, PARCCryptoHash *locallyComputedHash,
                         PARCCryptoSuite suite, PARCSignature *signatureToVerify);

    /** @see parcVerifier_AddKey */
    void (*AddKey)(PARCObject *interfaceContext, PARCKey *key);

    /** @see parcVerifier_RemoveKeyId */
    void (*RemoveKeyId)(PARCObject *interfaceContext, PARCKeyId *keyid);

    /** @see parcVerifier_AllowedCryptoSuite */
    bool (*AllowedCryptoSuite)(PARCObject *interfaceContext, PARCKeyId *keyid, PARCCryptoSuite suite);
} PARCVerifierInterface;

/**
 * Create a verifier context based on a concrete implementation.
 *
 * @param [in] instance A concrete implementation of a `PARCVerifier`
 * @param [in] interfaceContext The interface of a concrete implementation of a `PARCVerifier`
 *
 * @return NULL A `PARCVerifier` could not be allocated
 * @return PARCSigner A new `PARCVerifier` instance derived from the specified concrete signer context.
 *
 * Example:
 * @code
 * {
 *     PARCVerifier *verifier = parcVerifier_Create(verifierInstance, PARCInMemoryVerifierAsVerifier);
 * }
 * @endcode
 */
PARCVerifier *parcVerifier_Create(PARCObject *instance, PARCVerifierInterface *interfaceContext);

/**
 * Assert that an instance of `PARCVerifier` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] verifier A pointer to a PARCVerifier instance.
 *
 * Example
 * @code
 * {
 *     PARCVerifier *verifier = parcVerifier_Create(verifierInstance, PARCInMemoryVerifierAsVerifier);
 *
 *     parcVerifier_AssertValid(signer);
 * }
 * @endcode
 */
void parcVerifier_AssertValid(const PARCVerifier *verifier);

/**
 * Increase the number of references to the given `PARCVerifier` instance.
 *
 * A new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the acquired reference by invoking `parcVerifier_Release()`.
 *
 * @param [in] signer A pointer to a `PARCVerifier` instance.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a PARCVerifier instance.
 *
 * Example:
 * @code
 * {
 *      PARCVerifier *verifier = parcVerifier_Create(verifierInstance, PARCInMemoryVerifierAsVerifier);
 *      PARCVerifier *handle = parcVerifier_Acquire(signer);
 *      // use the handle instance as needed
 * }
 * @endcode
 */
PARCVerifier *parcVerifier_Acquire(const PARCVerifier *verifier);

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
 * The contents of the dealloced memory used for the PARC object are undefined.
 * Do not reference the object after the last release.
 *
 * @param [in,out] verifierPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCVerifier *verifier = parcVerifier_Create(verifierInstance, PARCInMemoryVerifierAsVerifier);
 *
 *     parcVerifier_Release(&verifier);
 * }
 * @endcode
 */
void parcVerifier_Release(PARCVerifier **verifierPtr);

/**
 * Verify the signature against the provided digest with the specified key.
 * If we do not trust the key, the signature will be rejected. In this context,
 * trusting a key means that it was previously added to this verifiers "store".
 *
 * Returns true if the signature is accepted,false if it is rejected.
 *
 * @param [in] verifier A `PARCVerifier` instance.
 * @param [in] keyId A `PARCKeyId` which identifies the verification key.
 * @param [in] hashDigest A `PARCCryptoHash` which stores the locally computed digest.
 * @param [in] suite The `PARCCryptoSuite` in which verification is performed.
 * @param [in] signature The `PARCSignature` which is to be verified.
 *
 * @retval true If the signature is valid
 * @retval false Otherwise
 *
 * Example:
 * @code
 * {
 *     PARCVerifier *verifier = parcVerifier_Create(verifierInstance, PARCInMemoryVerifierAsVerifier);
 *
 *     PARCKeyId *keyId = ...
 *     PARCCryptoHash *hash = ...
 *     PARCCryptoSuite suite = PARCCryptoSuite_RSA_SHA256;
 *     PARCSignature *signature = ...
 *
 *     bool valid = parcVerifier_VerifyDigestSignature(verifier, keyId, hash, suite, signature);
 *     if (valid) {
 *         // proceed
 *     }
 * }
 * @endcode
 */
bool
parcVerifier_VerifyDigestSignature(PARCVerifier *verifier, PARCKeyId *keyid, PARCCryptoHash *hashDigest,
                                   PARCCryptoSuite suite, PARCSignature *signatureToVerify);

/**
 * Check to see if the specified `PARCKeyId` is allowed with the given `PARCCryptoSuite`.
 *
 * A`PARCKey` identified by the given `PARCKeyId` can only be used for a particular algorithm.
 *
 * @param [in] verifier A `PARCVerifier` instance with a store of trusted `PARCKey` instances.
 * @param [in] keyId A `PARCKeyId` referring to the key we will check against (for this verifier).
 * @param [in] suite A `PARCCryptoSuite` to check against.
 *
 * @retval true If allowed
 * @retval false Otherwise
 *
 * Example:
 * @code
 * {
 *     PARCVerifier *verifeir = ...
 *     PARCKeyId *keyId = ...
 *     bool isAllowed = parcVerifier_AllowedCryptoSuite(verifier, keyId, PARCCryptoSuite_RSA_SHA256);
 *     // act accordingly
 * }
 * @endcode
 */
bool parcVerifier_AllowedCryptoSuite(PARCVerifier *verifier, PARCKeyId *keyId, PARCCryptoSuite suite);

/**
 * Returns a `PARCCryptoHasher` for use with the `PARCKeyId`. The caller should have already
 * verified that the specified `PARCCryptoHashType` is compatible with the key ID by
 * checking the AllowedCryptoSuite.
 *
 * @param [in] verifier A `PARCVerifier` instance with a store of trusted `PARCKey` instances.
 * @param [in] keyId A `PARCKeyId` referring to the key we will check against (for this verifier).
 * @param [in] suite A `PARCCryptoSuite` to check against.
 *
 * @retval non-NULL A `PARCCryptoHasher` instance.
 * @retval NULL If the PARCCryptoHashType is not compatible with the key.
 *
 * Example:
 * @code
 * {
 *     PARCVerifier *verifeir = ...
 *     PARCKeyId *keyId = ...
 *     bool isAllowed = parcVerifier_AllowedCryptoSuite(verifier, keyId, PARCCryptoHashType_SHA256);
 *     // act accordingly
 * }
 * @endcode
 */
PARCCryptoHasher *parcVerifier_GetCryptoHasher(PARCVerifier *verifier, PARCKeyId *keyid, PARCCryptoHashType hashType);

/**
 * Add the specified `PARCKey` to the trusted key store.
 *
 * @param [in] verifier A `PARCVerifier` instance with a store of trusted `PARCKey` instances.
 * @param [in] key A `PARCKey` containing the new trusted key.
 *
 * Example:
 * @code
 * {
 *     PARCVerifier *verifeir = ...
 *     PARCKey *key = ...
 *     parcVerifier_AddKey(verifier, key);
 * }
 * @endcode
 */
void parcVerifier_AddKey(PARCVerifier *verifier, PARCKey *key);

/**
 * Remove the key associated with the given `PARCKeyId` from the trusted key store.
 *
 * @param [in] verifier A `PARCVerifier` instance with a store of trusted `PARCKey` instances.
 * @param [in] keyId A `PARCKeyId` referencing the `PARCKey` to remove from the keystore.
 *
 * Example:
 * @code
 * {
 *     PARCVerifier *verifeir = ...
 *     PARCKey *key = ...
 *     parcVerifier_AddKey(verifier, key);
 *
 *     // Use the verifier with the key...
 *     ...
 *
 *     // Now remove it because we no longer need or trust it.
 *     PARCKeyId *keyId = parcKey_GetKeyId(key);
 *     parcVerifier_RemoveKeyId(verifier, keyId);
 * }
 * @endcode
 */
void parcVerifier_RemoveKeyId(PARCVerifier *verifier, PARCKeyId *keyid);
#endif // libparc_parc_Verifier_h
