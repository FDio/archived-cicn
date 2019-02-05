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
 * @file parc_Signer.h
 * @ingroup security
 * @brief The API a crytography provider must interfaceement.
 *
 * A signer IS NOT THREAD-SAFE.
 *
 */
#ifndef libparc_parc_Signer_h
#define libparc_parc_Signer_h

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/security/parc_CryptoHasher.h>
#include <parc/security/parc_CryptoSuite.h>
#include <parc/security/parc_Signature.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_Key.h>
#include <parc/security/parc_KeyStore.h>

struct parc_signer;
/**
 * @typedef PARCSigner
 * @brief The structure for PARCSigner
 */

typedef struct parc_signer PARCSigner;

/**
 * @def parcSigner_OptionalAssertValid
 * Optional validation of the given instance.
 *
 * Define `PARCLibrary_DISABLE_VALIDATION` to disable validation.
 */
#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcSigner_OptionalAssertValid(_instance_)
#else
#  define parcSigner_OptionalAssertValid(_instance_) parcSigner_AssertValid(_instance_)
#endif

/**
 * @typedef PARCSigningInterface
 * @brief The CCN signing implementation structure.
 *
 * This defines the contract that any concrete implementation provides.
 *
 */
typedef struct parc_signer_interface {
    /**
     * Returns a the hasher to use for the signature.  This is important for
     * symmetric key HMAC to use this hasher, not one from PARCCryptoHasher.
     *
     * DO NOT DESTROY THIS HASHER!  Just call _Init, _Update, and _Finalize.
     *
     * The hash type will be based on how the underlying implementation was initialized
     *
     * Equivalent of (for rsa/sha256)
     *    openssl rsautl -sign -inkey test_rsa_key.pem -in infile_digest -out infile.sig
     *
     * @param [in] interfaceContextPtr A pointer to a concrete PARCSigner instance.
     */
    PARCCryptoHasher *(*GetCryptoHasher)(void *interfaceContext);

    /**
     * Compute the signature of the given PARCCryptoHash.
     *
     * Equivalent of (for rsa/sha256)
     *    openssl rsautl -sign -inkey test_rsa_key.pem -in infile_digest -out infile.sig
     *
     * @param [in] interfaceContextPtr A pointer to a concrete PARCSigner instance.
     * @param [in] hashToSign The output of the given digest to sign
     * @param [in] signature Portion of memory that will contain the signature (expected to be large enough to contain the signature)
     * @param [in] sig_len Size in bytes of the supplied buffer
     *
     * @return A pointer to a PARCSignature instance that must be released via parcSignature_Release()
     */
    PARCSignature *(*SignDigest)(void *interfaceContext, const PARCCryptoHash * parcDigest, uint8_t * signature, uint32_t sign_len);

    /**
     * Return the PARSigningAlgorithm used for signing with the given `PARCSigner`
     *
     * @param [in] interfaceContext A pointer to a concrete PARCSigner instance.
     *
     * @return A PARSigningAlgorithm value.
     */
    PARCSigningAlgorithm (*GetSigningAlgorithm)(void *interfaceContext);

    /**
     * Return the digest algorithm used by the Signer
     *
     * @param [in] interfaceContext A pointer to a concrete PARCSigner instance.
     *
     * @return A PARCCryptoHashType value.
     */
    PARCCryptoHashType (*GetCryptoHashType)(void *interfaceContext);

    /**
     * Return the PARCKeyStore for this Signer.
     *
     * @param [in] interfaceContext A pointer to a concrete PARCSigner instance.
     *
     * @return A PARCKeyStore instance.
     */
    PARCKeyStore *(*GetKeyStore)(void *interfaceContext);

    /**
     * Return the key size for this Signer.
     *
     * @param [in] interfaceContext A pointer to a concrete PARCSigner instance.
     *
     * @return A size_t
     */
    size_t (*GetSignatureSize)(void *interfaceContext);
} PARCSigningInterface;

/**
 * Assert that an instance of `PARCSigner` is valid.
 *
 * If the instance is not valid, terminate via {@link parcTrapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 *
 * Example
 * @code
 * {
 *     PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *     parcSigner_AssertValid(signer);
 * }
 * @endcode
 */
void parcSigner_AssertValid(const PARCSigner *signer);

/**
 * Create a signing context based on a concrete implementation.
 *
 * @param [in] instance A concrete implementation of a `PARCSigner`
 * @param [in] interfaceContext The interface of a concrete implementation of a `PARCSigner`
 *
 * @return NULL A `PARCSigner` could not be allocated
 * @return PARCSigner A new `PARCSigner` instance derived from the specified concrete signer context.
 *
 * Example:
 * @code
 * {
 *     PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 * }
 * @endcode
 */
PARCSigner *parcSigner_Create(PARCObject *instance, PARCSigningInterface *interfaceContext);

/**
 * Increase the number of references to the given `PARCSigner` instance.
 *
 * A new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the acquired reference by invoking `parcSigner_Release()`.
 *
 * @param [in] signer A pointer to a `PARCSigner` instance.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a PARCSigner instance.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *      PARCSigner *handle = parcSigner_Acquire(signer);
 *      // use the handle instance as needed
 * }
 * @endcode
 */
PARCSigner *parcSigner_Acquire(const PARCSigner *signer);

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
 * @param [in,out] signerPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *     parcSigner_Release(&signer);
 * }
 * @endcode
 */
void parcSigner_Release(PARCSigner **signerPtr);

/**
 * Create a PARCKeyId instance from the given pinter to a `PARCSigner` instance.
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 *
 * @return A pointer to a created PARCKeyId instance that must be released via `parcKeyId_Release()`
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCKeyId *keyId = parcSigner_CreateKeyId(signer);
 * }
 * @endcode
 */
PARCKeyId *parcSigner_CreateKeyId(const PARCSigner *signer);

/**
 * Get the DER encoded public key and keyid wrapped in a `PARCKey` instance.
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 *
 * @return A newly allocated `PARCKey` instance containing the public key used to verify signatures computed by this signer.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCKey *publicKey = parcSigner_CreatePublicKey(signer);
 * }
 * @endcode
 */
PARCKey *parcSigner_CreatePublicKey(PARCSigner *signer);

/**
 * Returns a the hasher to use for the signature.  This is important for
 * symmetric key HMAC to use this hasher, not one from PARCCryptoHasher.
 *
 * DO NOT DESTROY THIS HASHER!  Just call _Init, _Update, and _Finalize.
 *
 * The hash type will be based on how the underlying implementation was initialized
 *
 * Equivalent of (for rsa/sha256)
 *    openssl rsautl -sign -inkey test_rsa_key.pem -in infile_digest -out infile.sig
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCCryptoHasher hasher = parcSigner_GetCryptoHasher(signer);
 * }
 * @endcode
 */
PARCCryptoHasher *parcSigner_GetCryptoHasher(const PARCSigner *signer);

/**
 * Compute the signature of the given PARCCryptoHash.
 *
 * Equivalent of (for rsa/sha256)
 *    openssl rsautl -sign -inkey test_rsa_key.pem -in infile_digest -out infile.sig
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 * @param [in] hashToSign The output of the given digest
 * @param [in] signature Portion of memory that will contain the signature (expected to be large enough to contain the signature)
 * @param [in] sig_len Size in bytes of the supplied buffer
 *
 * @return A pointer to a PARCSignature instance that must be released via parcSignature_Release()
 *
 * Example:
 * @code
 * {
 *     PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *     PARCCryptoHasher *hasher = parcSigner_GetCryptoHasher(signer);
 *     parcCryptoHasher_Init(hasher);
 *     parcCryptoHasher_Update_Bytes(hasher, &block->memory[relativePosition], length);
 *     PARCCryptoHash *hashToSign = parcCryptoHasher_Finalize(hasher);
 *
 *     PARCSignature signature = parcSigner_SignDigest(signer, hashToSign);
 * }
 * @endcode
 */
PARCSignature *parcSigner_SignDigest(const PARCSigner *signer, const PARCCryptoHash *hashToSign, uint8_t * signature, uint32_t sig_len);

/**
 * Compute the signature of a given `PARCBuffer`.
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 * @param [in] buffer The input to be hashed and signed.
 *
 * @return A pointer to a PARCSignature instance that must be released via parcSignature_Release()
 *
 * Example:
 * @code
 * {
 *     PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *     PARCBuffer *inputBuffer = ...
 *
 *     PARCSignature signature = parcSigner_SignBuffer(signer, inputBuffer);
 * }
 * @endcode
 */
PARCSignature *parcSigner_SignBuffer(const PARCSigner *signer, const PARCBuffer *buffer, uint8_t * signature, uint32_t sig_len);

/**
 * Return the PARSigningAlgorithm used for signing with the given `PARCSigner`
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 *
 * @return A PARSigningAlgorithm value.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCSigningAlgorithm suite = parcSigner_GetSigningAlgorithm(signer);
 * }
 * @endcode
 */
PARCSigningAlgorithm parcSigner_GetSigningAlgorithm(PARCSigner *signer);

/**
 * Return the digest algorithm used by the Signer
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 *
 * @return A PARCCryptoHashType value.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCCryptoHashType suite = parcSigner_GetCryptoHashType(signer);
 * }
 * @endcode
 */
PARCCryptoHashType parcSigner_GetCryptoHashType(const PARCSigner *signer);

/**
 * Return the crypto suite used by the Signer
 *
 * @param [in] signer A pointer to a PARCSigner instance.
 *
 * @return A PARCCryptoSuite value.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCCryptoSuite suite = parcSigner_GetCryptoSuite(signer);
 * }
 * @endcode
 */
PARCCryptoSuite parcSigner_GetCryptoSuite(const PARCSigner *signer);

/**
 * Given a `PARCSigner` instance, return the `PARCKeyStore` containing its public key information.
 *
 * @param [in] signer A pointer to a `PARCSigner` instance.
 *
 * @return A `PARCKeyStore` instance.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCKeyStore *keyStore = parcSigner_GetKeyStore(signer);
 * }
 * @endcode
 */
PARCKeyStore *parcSigner_GetKeyStore(const PARCSigner *signer);

/**
 * Given a `PARCSigner` instance, return the expected size of the signature.
 *
 * @param [in] signer A pointer to a `PARCSigner` instance.
 *
 * @return A size_t with the size of the key.
 *
 * Example:
 * @code
 * {
 *      PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCRSASignerAsSigner);
 *
 *      PARCKeyStore *keyStore = parcSigner_GetKeyStore(signer);
 * }
 * @endcode
 */
size_t parcSigner_GetSignatureSize(const PARCSigner *signer);
#endif // libparc_parc_Signer_h
