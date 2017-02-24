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
 * @file parc_CertificateFactory.h
 * @ingroup security
 * @brief A factory to build certificates.
 *
 */
#ifndef libparc_parc_CertificateFactory_h
#define libparc_parc_CertificateFactory_h

#include <parc/security/parc_Certificate.h>
#include <parc/security/parc_CertificateType.h>
#include <parc/security/parc_ContainerEncoding.h>

struct parc_certificate_factory;
typedef struct parc_certificate_factory PARCCertificateFactory;

/**
 * Create a `PARCCertificateFactory` to build certificates with the specified type and encoding.
 *
 * @param [in] type The `PARCCertificateType` of certificates to construct.
 * @param [in] encoding The `PARCContainerEncoding` of certificates to construct.
 *
 * @return NULL The memory could not be allocated.
 * @return non-NULL A newly allocated `PARCCertificateFactory`.
 *
 * Example:
 * @code
 * {
 *     PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_PEM);
 * }
 * @endcode
 */
PARCCertificateFactory *parcCertificateFactory_Create(PARCCertificateType type, PARCContainerEncoding encoding);

/**
 * Create a `PARCCertificate` from the specified filename and password.
 *
 * @param [in] factory The `PARCCertificateFactory` instance used to build the certificate.
 * @param [in] filename A nul-terminated path to the certificate file.
 * @param [in] password A nul-terminated password.
 *
 * @return NULL The memory could not be allocated.
 * @return non-NULL A newly allocated `PARCCertificate`.
 *
 * Example:
 * @code
 * {
 *     char *pathToCertificate = "file.pem";
 *     PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_PEM);
 *     PARCCertificate *certificate = parcCertificateFactory_CreateCertificateFromFile(factory, pathToCertificate, NULL);
 * }
 * @endcode
 */
PARCCertificate *parcCertificateFactory_CreateCertificateFromFile(PARCCertificateFactory *factory, char *filename, char *password);

/**
 * Create a `PARCCertificate` from the specified `PARCBuffer`.
 *
 * @param [in] factory The `PARCCertificateFactory` instance used to build the certificate.
 * @param [in] buffer A `PARCBuffer` encoding a `PARCCertificate` instance.
 *
 * @return NULL The memory could not be allocated.
 * @return non-NULL A newly allocated `PARCCertificate`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = ...;
 *     PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_DER);
 *     PARCCertificate *certificate = parcCertificateFactory_CreateCertificateFromBuffer(factory, buffer);
 * }
 * @endcode
 */
PARCCertificate *parcCertificateFactory_CreateCertificateFromBuffer(PARCCertificateFactory *factory, PARCBuffer *buffer);

/**
 * Create a self-signed `PARCCertificate` using the specified parameters and return
 * the corresponding private key.
 *
 * Note: this is equivalent to the following OpenSSL command:
 *   XXXXYYYYZZZZ
 *   TODO TODO TODO
 *
 * @param [in] factory The `PARCCertificateFactory` instance used to build the certificate.
 * @param [in, out] privateKey A pointer to a `PARCBuffer` pointer where the new certificate private key will be stored.
 * @param [in] subjectName The name of the certificate subject.
 * @param [in] keyLength The length of the public key to be derived.
 * @param [in] validityDays The validity period.
 *
 * @return NULL The memory could not be allocated.
 * @return non-NULL A newly allocated `PARCCertificate`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = ...;
 *     PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_DER);
 *     PARCCertificate *certificate = parcCertificateFactory_CreateCertificateFromBuffer(factory, buffer);
 * }
 * @endcode
 */
PARCCertificate *parcCertificateFactory_CreateSelfSignedCertificate(PARCCertificateFactory *factort, PARCBuffer **privateKey, char *subjectName, size_t keyLength, size_t valdityDays);

/**
 * Increase the number of references to a `PARCCertificateFactory` instance.
 *
 * Note that a new `PARCCertificateFactory` is not created,
 * only that the given `PARCCertificateFactory` reference count is incremented.
 * Discard the reference by invoking {@link parcCertificateFactory_Release}.
 *
 * @param [in] certificate A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCCertificateFactory *x = parcCertificateFactory_CreateFromFile(...);
 *     PARCCertificateFactory *x2 = parcCertificateFactory_Acquire(x);
 *
 *     parcCertificateFactory_Release(&x);
 *     parcCertificateFactory_Release(&x2);
 * }
 * @endcode
 *
 * @see {@link parcCertificateFactory_Release}
 */
PARCCertificateFactory *parcCertificateFactory_Acquire(const PARCCertificateFactory *factory);

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
 * @param [in,out] factoryP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCCertificateFactory *x = parcCertificateFactory_CreateFromFile(...);
 *
 *     parcCertificateFactory_Release(&x);
 * }
 * @endcode
 */
void parcCertificateFactory_Release(PARCCertificateFactory **factoryP);
#endif // libparc_parc_CertificateFactory_h
