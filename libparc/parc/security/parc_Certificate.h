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
 * @file parc_Certificate.h
 * @ingroup security
 * @brief The API for a generic certificate.
 *
 */

#ifndef libparc_parc_Certificate_h
#define libparc_parc_Certificate_h

#include <parc/algol/parc_Buffer.h>
#include <parc/security/parc_CertificateType.h>
#include <parc/security/parc_ContainerEncoding.h>
#include <parc/security/parc_CryptoHasher.h>
#include <parc/security/parc_Signature.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_Key.h>

struct parc_certificate;
/**
 * @typedef PARCCertificate
 * @brief The structure for PARCCertificate
 */
typedef struct parc_certificate PARCCertificate;

typedef struct parc_certificate_interface {
    /**
     * The hash of the certificate's public key.
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
     * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
     *
     * @return A `PARCCryptoHash` instance.
     */
    PARCCryptoHash *(*GetPublicKeyDigest)(void *certificate);

    /**
     * Returns a copy of the the certificate digest.
     *
     * Returns NULL for symmetric keystores.
     *
     * Equivalent of (for rsa/sha256):
     *    openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
     *    openssl sha256 -out test_rsa_crt_sha256.bin -sha256 -binary < test_rsa_crt.der
     * Which is also the same as (but not in der format)
     *    openssl x509 -in test_rsa.crt -fingerprint -sha256
     *
     * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
     *
     * @return A `PARCCryptoHash` instance which internally contains a hash digest of the certificate.
     */
    PARCCryptoHash *(*GetCertificateDigest)(void *certificate);

    /**
     * Returns a copy of the DER encoded certificate.
     *
     * Returns NULL for symmetric keystores.
     *
     * Equivalent of:
     *   openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
     *
     * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
     *
     * @return A pointer to a `PARCBuffer` containing the encoded certificate.
     */
    PARCBuffer *(*GetDEREncodedCertificate)(void *certificate);

    /**
     * Returns a copy of the encoded public key in DER form.
     *
     * Equivalent of (for rsa/sha256):
     *   `openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der`
     *
     * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
     *
     * @return A pointer to a `PARCBuffer` containing the encoded public key.
     */
    PARCBuffer *(*GetDEREncodedPublicKey)(void *certificate);

    /**
     * Returns the `PARCCertificateType` of this certificate, i.e., PEM, DER, PKCS12.
     *
     * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
     *
     * @return The `PARCCertificateType` associated with this certificate.
     */
    PARCCertificateType (*GetCertificateType)(const void *certificate);

    /**
     * Returns the `PARCContainerEncoding` of this certificate, e.g., X509.
     *
     * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
     *
     * @return The `PARCContainerEncoding` associated with this certificate.
     */
    PARCContainerEncoding (*GetContainerEncoding)(const void *certificate);
} PARCCertificateInterface;

/**
 * Create a generic `PARCCertificate` instance from a concrete `PARCCertificate` instance.
 *
 * NOTE: This function should not be used directly. Construct certificates using the
 * `PARCCertificateFactory` instead.
 *
 * @param [in] impl A pointer to a concrete `PARCCertificate` interface implementation.
 * @param [in] instance A pointer to the instance that implements this interface.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A newly allocated `PARCCertificate` instance containing the concrete
 *                  `PARCCertificate` instance.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = ...;
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 * }
 * @endcode
 */
PARCCertificate *parcCertificate_CreateFromInstance(PARCCertificateInterface *impl, void *instance);

/**
 * Increase the number of references to a `PARCCertificate` instance.
 *
 * Note that a new `PARCCertificate` is not created,
 * only that the given `PARCCertificate` reference count is incremented.
 * Discard the reference by invoking {@link parcCertificate_Release}.
 *
 * @param [in] certificate A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCCertificate *x = parcCertificate_CreateFromInstance(...);
 *     PARCCertificate *x2 = parcCertificate_Acquire(x);
 *
 *     parcCertificate_Release(&x);
 *     parcCertificate_Release(&x2);
 * }
 * @endcode
 *
 * @see {@link parcCertificate_Release}
 */
PARCCertificate *parcCertificate_Acquire(const PARCCertificate *certificate);

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
 * @param [in,out] certificateP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCCertificate *x = parcCertificate_Acquire(...);
 *
 *     parcCertificate_Release(&x);
 * }
 * @endcode
 *
 * @see {@link parcCertificate_Acquire}
 */
void parcCertificate_Release(PARCCertificate **certificateP);

/**
 * Returns the `PARCCertificateType` of this certificate, i.e., X509.
 *
 * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
 *
 * @return The `PARCCertificateType` associated with this certificate.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 *
 *     PARCCertificateType type = parcCertificate_GetCertificateType(wrapper);
 *     // type == PARCCertificateType_X509
 * }
 * @endcode
 */
PARCCertificateType parcCertificate_GetCertificateType(const PARCCertificate *certificate);

/**
 * Returns the `PARCContainerEncoding` of this certificate, e.g., PEM, DER.
 *
 * @param [in] certificate A pointer to a concrete `PARCCertificate` instance.
 *
 * @return The `PARCContainerEncoding` associated with this certificate.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 *
 *     PARCContainerEncoding encoding = parcCertificate_GetCertificateType(wrapper);
 *     // encoding == PARCCertificateType_PEM
 * }
 * @endcode
 */
PARCContainerEncoding parcCertificate_GetContainerEncoding(const PARCCertificate *certificate);

/**
 * Retrieve the SHA-256 hash digest of the certificate's public key.
 *
 * You must release the returned `PARCCryptoHash` via {@link parcCryptoHash_Release}.
 *
 * Equivalent of (for rsa/sha256):
 *    openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der
 *    openssl sha256 -out test_rsa_pub_sha256.bin -sha256 -binary < test_rsa_pub.der
 *
 * @param [in] certificate A pointer to a `PARCCertificate` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A `PARCCryptoHash` value which internally contains a hash digest of the certificate key.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 *
 *     PARCCryptoHash *certificateKeyDigest = parcCertificate_GetPublicKeyDigest(wrapper);
 * }
 * @endcode
 */
PARCCryptoHash *parcCertificate_GetPublicKeyDigest(const PARCCertificate *certificate);

/**
 * Get the SHA-256 digest of the certificate.
 *
 * Equivalent of (for rsa/sha256):
 *    openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
 *    openssl sha256 -out test_rsa_crt_sha256.bin -sha256 -binary < test_rsa_crt.der
 * Which is also the same as (but not in der format)
 *    openssl x509 -in test_rsa.crt -fingerprint -sha256
 *
 * @param [in] certificate A pointer to a `PARCCertificate` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return A `PARCCryptoHash` instance which internally contains a hash digest of the certificate.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 *
 *     PARCCryptoHash *certificateKeyDigest = parcCertificate_GetPublicKeyDigest(wrapper);
 * }
 * @endcode
 */
PARCCryptoHash *parcCertificate_GetCertificateDigest(const PARCCertificate *certificate);

/**
 * Get a `PARCBuffer` containing the DER encoded representation of the certificate.
 *
 * Equivalent of:
 *   openssl x509 -outform DER -out test_rsa_crt.der -in test_rsa.crt
 *
 * @param [in] certificate A pointer to a `PARCCertificate` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a `PARCBuffer` containing the encoded certificate.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 *
 *     PARCBuffer *certificateDER = parcCertificate_GetDEREncodedCertificate(wrapper);
 * }
 * @endcode
 */
PARCBuffer *parcCertificate_GetDEREncodedCertificate(const PARCCertificate *certificate);

/**
 * Get the certificate's public key in DER encoding in a `PARCBuffer`.
 *
 * Equivalent of (for rsa/sha256):
 *   `openssl rsa -in test_rsa_key.pem -outform DER -pubout -out test_rsa_pub.der`
 *
 * @param [in] certificate A pointer to a `PARCCertificate` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a `PARCBuffer` containing the encoded certificate's public key.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 *
 *     PARCBuffer *certificateDER = parcCertificate_GetDEREncodedPublicKey(wrapper);
 * }
 * @endcode
 */
PARCBuffer *parcCertificate_GetDEREncodedPublicKey(const PARCCertificate *certificate);

/**
 * Get the `PARCKey` public key associated with this certificate.
 *
 * @param [in] certificate A pointer to a `PARCCertificate` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a `PARCKey` instance.
 *
 * Example:
 * @code
 * {
 *     PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
 *     PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
 *
 *     PARCKey *publicKey = parcCertificate_GetPublicKey(wrapper);
 * }
 * @endcode
 */
PARCKey *parcCertificate_GetPublicKey(const PARCCertificate *certificate);
#endif // libparc_parc_Certificate_h
