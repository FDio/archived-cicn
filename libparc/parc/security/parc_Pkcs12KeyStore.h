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
 * @file parc_Pkcs12KeyStore.h
 * @ingroup security
 * @brief A concrete implementation of PARCKeyStore based on a PCKS12 keystore.
 *
 */
#ifndef libparc_parc_Pkcs12KeyStore_h
#define libparc_parc_Pkcs12KeyStore_h

#include <parc/security/parc_KeyStore.h>
#include <parc/security/parc_Signer.h>

struct parc_pkcs12_keystore;
typedef struct parc_pkcs12_keystore PARCPkcs12KeyStore;

extern PARCKeyStoreInterface *PARCPkcs12KeyStoreAsKeyStore;

/**
 * Increase the number of references to a `PARCPublicKeySigner` instance.
 *
 * Note that new `PARCPublicKeySigner` is not created,
 * only that the given `PARCPublicKeySigner` reference count is incremented.
 * Discard the reference by invoking `parcPublicKeySigner_Release`.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     parcPkcs12KeyStore_CreateFile(...);
 *     PARCPkcs12KeyStore *a = parcPkcs12Store_Open(...)
 *
 *     PARCPkcs12KeyStore *b = parcPkcs12KeyStore_Acquire();
 *
 *     parcPkcs12KeyStore_Release(&a);
 *     parcPkcs12KeyStore_Release(&b);
 * }
 * @endcode
 */
PARCPkcs12KeyStore *parcPkcs12KeyStore_Acquire(const PARCPkcs12KeyStore *instance);

/**
 * Release a previously acquired reference to the given `PARCPkcs12KeyStore` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCPkcs12KeyStore *a = parcPkcs12Store_Open(...);
 *
 *     parcPkcs12KeyStore_Release(&a);
 * }
 * @endcode
 */
void parcPkcs12KeyStore_Release(PARCPkcs12KeyStore **instancePtr);

/**
 * Creates a PKCS12 keystore identity with a self-signed certifiate. Note that this call currently
 * aborts if keystore i/o access fails, behavior that may change in the future.
 *
 * @param [in] filename The name of the PKCS12 file.
 * @param [in] password The password to open the PKCS12 file.
 * @param [in] subjectName The certificate subject associated with the PKCS12 file.
 * @param [in] keyLength The length of the public key associated with the PKCS12 file.
 * @param [in] validityDays The validity (in days) of the certificate associated with the PKCS12 file.
 *
 * @return true on success, false if certificate creation fails, and will abort if keystore i/o fails.
 *
 * Example:
 * @code
 * {
 *     const char *filename = "/tmp/ccnxFileKeyStore_Pkcs12Open_CreateAndOpen.p12";
 *     const char *password = "12345";
 *     const char *subject  = "alice";
 *     bool result = parcPkcs12KeyStore_CreateFile(filename, password, subject, 1024, 32);
 * }
 * @endcode
 */
bool parcPkcs12KeyStore_CreateFile(const char *filename, const char *password, const char *subjectName,
                                   unsigned keyLength, unsigned validityDays);

/**
 * Create a `PARCPkcs12KeyStore` instance.
 *
 * @param [in] filename The name of a file containing the PKCS12 keystore.
 * @param [in] password The password to decrypt/unlock the determines how the signer digests data. Supports PARCCryptoHashType_SHA256 and PARCCryptoHashType_SHA512.
 * @param [in] hashType Determines how the signer digests data. Possible values are PARCCryptoHashType_SHA256 and PARCCryptoHashType_SHA512.
 *
 * @return A `PARCPkcs12KeyStore` instance using the public/private key pair contained within the PKCS12 file.
 *
 * Example:
 * @code
 * {
 *     const char *filename = "/tmp/ccnxFileKeyStore_Pkcs12Open_CreateAndOpen.p12";
 *     const char *password = "12345";
 *     const char *subject  = "alice";
 *     bool result = parcPkcs12KeyStore_CreateFile(filename, password, subject, 1024, 32);
 *
 *     ...
 *
 *     PARCSigningInterface *interface = parcPublicKeySignerPkcs12Store_Open(filename, password, PARCCryptoHashType_SHA256);
 *
 *     ...
 * }
 * @endcode
 */
PARCPkcs12KeyStore *parcPkcs12KeyStore_Open(const char *filename, const char *password, PARCCryptoHashType hashType);

#endif // libparc_parc_PublicKeySignerPkcs12Store_h
