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
 * @file parc_KeyStore.h
 * @ingroup security
 * @brief A container of Key Store information.
 *
 * A Key Store is a repository of key information typically accessable
 * through some authentication and authorisation system.
 * The PARCKeyStore type contains the necessary information to successfully
 * gain access to a Key Store.
 *
 */
#ifndef libparc_parc_KeyStore_h
#define libparc_parc_KeyStore_h

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/security/parc_CryptoHash.h>

struct parc_key_store;
typedef struct parc_key_store PARCKeyStore;

/**
 * The hash of the signer's public key (or secret key for HMAC).
 *
 * Try using `parcSigner_CreateKeyId` for a sinterfaceer interface.
 * You must destroy the returned PARCCryptoHash.
 * For public key, its the SHA256 digest of the public key.
 * For HMAC, its the SHA256 digest of the secret key.
 *
 * Equivalent of (for rsa/sha256):
 *    openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der
 *    openssl sha256 -out test_rsa_pub_sha256.bin -sha256 -binary < test_rsa_pub.der
 *
 * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
 *
 * @return A PARCCryptoHash value.
 */
typedef PARCCryptoHash *(PARCKeyStoreGetVerifierKeyDigest)(const void *interfaceContext);

/**
 * Returns a copy of the the certificate digest.
 * Returns NULL for symmetric keystores.
 *
 * Equivalent of (for rsa/sha256):
 *    openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
 *    openssl sha256 -out test_rsa_crt_sha256.bin -sha256 -binary < test_rsa_crt.der
 * Which is also the same as (but not in der format)
 *    openssl x509 -in test_rsa.crt -fingerprint -sha256
 *
 * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
 *
 * @return A `PARCCryptoHash` instance which internally contains a hash digest of the certificate used by the signer.
 */
typedef PARCCryptoHash *(PARCKeyStoreGetCertificateDigest)(const void *interfaceContext);

/**
 * Returns a copy of the DER encoded certificate.
 * Returns NULL for symmetric keystores.
 *
 * Equivalent of:
 *   openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
 *
 * @param [in] interfaceContextPtr A pointer to a concrete PARCKeyStore instance.
 *
 * @return A pointer to a PARCBuffer containing the encoded certificate.
 */
typedef PARCBuffer *(PARCKeyStoreGetDEREncodedCertificate)(const void *interfaceContext);

/**
 * Returns a copy of the encoded public key in Distinguished Encoding Rules (DER) form.
 *
 * Equivalent of (for rsa/sha256):
 *   `openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der`
 *
 * @param [in] interfaceContextPtr A pointer to a concrete PARCKeyStore instance.
 *
 * @return A pointer to a PARCBuffer containing the encoded public key.
 */
typedef PARCBuffer *(PARCKeyStoreGetDEREncodedPublicKey)(const void *interfaceContext);

/**
 * Returns a copy of the encoded private key in Distinguished Encoding Rules (DER) form.
 *
 * Equivalent of (for rsa/sha256):
 *   `openssl rsa -in test_rsa_key.pem -outform DER -out test_rsa.der`
 *
 * @param [in] interfaceContextPtr A pointer to a concrete PARCKeyStore instance.
 *
 * @return A pointer to a PARCBuffer containing the encoded private key.
 */
typedef PARCBuffer *(PARCKeyStoreGetDEREncodedPrivateKey)(const void *interfaceContext);


typedef struct parc_keystore_interface {
    /**
     * The hash of the signer's public key (or secret key for HMAC).
     *
     * Try using `parcSigner_CreateKeyId` for a sinterfaceer interface.
     * You must destroy the returned PARCCryptoHash.
     * For public key, its the SHA256 digest of the public key.
     * For HMAC, its the SHA256 digest of the secret key.
     *
     * Equivalent of (for rsa/sha256):
     *    openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der
     *    openssl sha256 -out test_rsa_pub_sha256.bin -sha256 -binary < test_rsa_pub.der
     *
     * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
     *
     * @return A PARCCryptoHash value.
     */
    PARCKeyStoreGetVerifierKeyDigest *getVerifierKeyDigest;

    /**
     * Returns a copy of the the certificate digest.
     * Returns NULL for symmetric keystores.
     *
     * Equivalent of (for rsa/sha256):
     *    openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
     *    openssl sha256 -out test_rsa_crt_sha256.bin -sha256 -binary < test_rsa_crt.der
     * Which is also the same as (but not in der format)
     *    openssl x509 -in test_rsa.crt -fingerprint -sha256
     *
     * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
     *
     * @return A `PARCCryptoHash` instance which internally contains a hash digest of the certificate used by the signer.
     */
    PARCKeyStoreGetCertificateDigest *getCertificateDigest;

    /**
     * Returns a copy of the DER encoded certificate.
     * Returns NULL for symmetric keystores.
     *
     * Equivalent of:
     *   openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
     *
     * @param [in] interfaceContextPtr A pointer to a concrete PARCKeyStore instance.
     *
     * @return A pointer to a PARCBuffer containing the encoded certificate.
     */
    PARCKeyStoreGetDEREncodedCertificate *getDEREncodedCertificate;

    /**
     * Returns a copy of the encoded public key in Distinguished Encoding Rules (DER) form.
     *
     * Equivalent of (for rsa/sha256):
     *   `openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der`
     *
     * @param [in] interfaceContextPtr A pointer to a concrete PARCKeyStore instance.
     *
     * @return A pointer to a PARCBuffer containing the encoded public key.
     */
    PARCKeyStoreGetDEREncodedPublicKey *getDEREncodedPublicKey;

    /**
     * Returns a copy of the encoded private key in Distinguished Encoding Rules (DER) form.
     *
     * Equivalent of (for rsa/sha256):
     *   `openssl rsa -in test_rsa_key.pem -outform DER -out test_rsa.der`
     *
     * @param [in] interfaceContextPtr A pointer to a concrete PARCKeyStore instance.
     *
     * @return A pointer to a PARCBuffer containing the encoded private key.
     */
    PARCKeyStoreGetDEREncodedPrivateKey *getDEREncodedPrivateKey;
} PARCKeyStoreInterface;

/**
 * Create a `PARCKeyStore` from a filename.
 *
 * @param [in] instance A concrete instance of a `PARCKeyStore.`
 * @param [in] interface The interface for the `PARCKeyStore.`
 *
 * @return A pointer to the new `PARCKeyStore`
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
PARCKeyStore *parcKeyStore_Create(PARCObject *instance, const PARCKeyStoreInterface *interface);

/**
 * Increase the number of references to an instance of this object.
 *
 * Note that new instance is not created, only that the given instance's reference count
 * is incremented. Discard the reference by invoking `parcKeyStore_Release()`.
 *
 * @param [in] keyStore A pointer to the original instance.
 *
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCKeyStore *keyStore = parcKeyStore_Acquire(keyStoreInstance);
 *
 *     parcKeyStore_Release(&keyStore);
 * }
 * @endcode
 *
 * @see parcKey_Release
 */
PARCKeyStore *parcKeyStore_Acquire(const PARCKeyStore *keyStore);

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
 * @param [in] keyStorePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCKeyStore *keyStore = parcKeyStore_Acquire(keyStoreInstance);
 *
 *     parcKeyStore_Release(&keyStore);
 * }
 * @endcode
 */
void parcKeyStore_Release(PARCKeyStore **keyStorePtr);

/**
 * The hash of the signer's public key (or secret key for HMAC).
 *
 * Try using `parcSigner_CreateKeyId` for a sinterfaceer interface.
 * You must destroy the returned PARCCryptoHash.
 * For public key, its the SHA256 digest of the public key.
 * For HMAC, its the SHA256 digest of the secret key.
 *
 * Equivalent of (for rsa/sha256):
 *    openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der
 *    openssl sha256 -out test_rsa_pub_sha256.bin -sha256 -binary < test_rsa_pub.der
 *
 * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
 *
 * @return A PARCCryptoHash value.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
PARCCryptoHash *parcKeyStore_GetVerifierKeyDigest(const PARCKeyStore *interfaceContext);

/**
 * Returns a copy of the the certificate digest.
 * Returns NULL for symmetric keystores.
 *
 * Equivalent of (for rsa/sha256):
 *    openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
 *    openssl sha256 -out test_rsa_crt_sha256.bin -sha256 -binary < test_rsa_crt.der
 * Which is also the same as (but not in der format)
 *    openssl x509 -in test_rsa.crt -fingerprint -sha256
 *
 * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
 *
 * @return A `PARCCryptoHash` instance which internally contains a hash digest of the certificate used by the signer.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
PARCCryptoHash *parcKeyStore_GetCertificateDigest(const PARCKeyStore *interfaceContext);

/**
 * Returns a copy of the DER encoded certificate.
 * Returns NULL for symmetric keystores.
 *
 * Equivalent of:
 *   openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
 *
 * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
 *
 * @return A pointer to a PARCBuffer containing the encoded certificate.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
PARCBuffer *parcKeyStore_GetDEREncodedCertificate(const PARCKeyStore *interfaceContext);

/**
 * Returns a copy of the encoded public key in Distinguished Encoding Rules (DER) form.
 *
 * Equivalent of (for rsa/sha256):
 *   `openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der`
 *
 * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
 *
 * @return A pointer to a PARCBuffer containing the encoded public key.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
PARCBuffer *parcKeyStore_GetDEREncodedPublicKey(const PARCKeyStore *interfaceContext);

/**
 * Returns a copy of the encoded private key in Distinguished Encoding Rules (DER) form.
 *
 * Equivalent of (for rsa/sha256):
 *   `openssl rsa -in test_rsa_key.pem -outform DER -out test_rsa.der`
 *
 * @param [in] interfaceContext A pointer to a concrete PARCKeyStore instance.
 *
 * @return A pointer to a PARCBuffer containing the encoded private key.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
PARCBuffer *parcKeyStore_GetDEREncodedPrivateKey(const PARCKeyStore *interfaceContext);
#endif // libparc_parc_KeyStore_h
