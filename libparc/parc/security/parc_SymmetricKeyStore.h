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
 * @file parc_SymmetricKeyStore.h
 * @ingroup security
 * @brief A PARCKeyStore instance for symmetric keys that can be used to produce,
 *        for example, HMAC authenticator tags.
 *
 * The secret key is stored in a PARC proprietary file format.
 *
 * ---------------------------------------------------------------------------
 * From the Java implementation code comments:
 *
 * This is a specialized keystore for storing symmetric keys. We looked at PKCS #11 for this but decided
 * against it for now because industry doesn't seem to be standardizing around it - at least not yet, and
 * standard support for it is somewhat sketchy at this point.
 *
 * The keystore can be used for only one key at a time and is located by naming it with a suffix
 * created from the key's digest.
 *
 * Following is the formula for the KeyStore
 *
 * Let P=passphrase
 * Let PT = symmetric key to store
 * Let IV = random 16-bytes
 *
 * aesK = HMAC-SHA256(P, '\0')
 * macK = HMAC-SHA256(P, '\1')
 * AES256-CBC(IV, key, PT) - performs AES256 in CBC mode
 *
 * SK = IV || AES256-CBC(IV, aesK, PT) || HMAC-SHA256(macK, AES256-CBC(IV, aesK, PT))
 *
 * SK is the symmetric keystore ciphertext
 *
 * ASN1 encoded KeyStore = Version || Key algorithm OID || SK
 * ---------------------------------------------------------------------------
 *
 */
#ifndef libparc_parc_SymmetricKeyStore_h
#define libparc_parc_SymmetricKeyStore_h

#include <parc/security/parc_Signer.h>
#include <parc/algol/parc_Buffer.h>

struct parc_symmetric_keystore;
typedef struct parc_symmetric_keystore PARCSymmetricKeyStore;

extern PARCKeyStoreInterface *PARCSymmetricKeyStoreAsKeyStore;

/**
 * Increase the number of references to a `PARCSymmetricKeyStore` instance.
 *
 * Note that new `PARCSymmetricKeyStore` is not created,
 * only that the given `PARCSymmetricKeyStore` reference count is incremented.
 * Discard the reference by invoking `parcSymmetricKeyStore_Release`.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeyStore instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     parcSymmetricKeyStore_CreateFile(...);
 *     PARCSymmetricKeyStore *a = parcSymmetricKeyStore_OpenFile(...)
 *
 *     PARCSymmetricKeyStore *b = parcSymmetricKeyStore_Acquire();
 *
 *     parcSymmetricKeyStore_Release(&a);
 *     parcSymmetricKeyStore_Release(&b);
 * }
 * @endcode
 */
PARCSymmetricKeyStore *parcSymmetricKeyStore_Acquire(const PARCSymmetricKeyStore *instance);

/**
 * Release a previously acquired reference to the given `PARCSymmetricKeyStore` instance,
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
 *     PARCSymmetricKeyStore *a = parcSymmetricKeyStore_Open(...);
 *
 *     parcSymmetricKeyStore_Release(&a);
 * }
 * @endcode
 */
void parcSymmetricKeyStore_Release(PARCSymmetricKeyStore **instancePtr);

/**
 * Create a symmetric (secret) key of the given bit length (e.g. 256)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *parcSymmetricKeyStore_CreateKey(unsigned bits);

PARCBuffer *parcSymmetricKeyStore_GetKey(PARCSymmetricKeyStore *keyStore);

PARCCryptoHash *parcSymmetricKeyStore_GetVerifierKeyDigest(PARCSymmetricKeyStore *keyStore);

/**
 * Creates a PARC format symmetric keystore.  It only contains a single key.
 *
 * Return 0 on success, -1 on failure
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcSymmetricKeyStore_CreateFile(const char *filename, const char *password, PARCBuffer *secret_key);

/**
 * Create a PKCS12 signing context for use in ccnx_Signing by reading the PARC symmetric key
 * file given by filename.  It is destroyed
 * by parc_Signing when the signing context is destroyed.
 *
 * @param [in] filename The filename.
 * @param [in] password The password to use.
 * @param [in] hmacHashType is for the HMAC, e.g. PARCCryptoHashType_SHA256
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSymmetricKeyStore *parcSymmetricKeyStore_OpenFile(const char *filename, const char *password, PARCCryptoHashType hmacHashType);

/**
 * Create a PKCS12 signing context for use in ccnx_Signing from the provided key.
 * This is an in-memory only signer.
 * It is destroyed by parc_Signing when the signing context is destroyed.
 *
 * @param secret_key is the shared secret, we take ownership of the buffer.
 * @param hmacHashType is for the HMAC, e.g. PARCCryptoHashType_SHA256
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSymmetricKeyStore *parcSymmetricKeyStore_Create(PARCBuffer *secret_key);
#endif // libparc_parc_SymmetricKeyStore_h
