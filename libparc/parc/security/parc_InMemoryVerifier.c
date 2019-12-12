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
 * @header parc_InMemoryVerifier.c
 * <#Abstract#>
 *
 *     <#Discussion#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#include <config.h>
#include <stdio.h>

#include <parc/assert/parc_Assert.h>
#include <parc/security/parc_InMemoryVerifier.h>
#include <parc/security/parc_CryptoHasher.h>
#include <parc/security/parc_CryptoCache.h>
#include <parc/algol/parc_Memory.h>

#include <openssl/x509v3.h>
#include <openssl/ecdsa.h>


struct parc_inmemory_verifier {
    PARCCryptoHasher *hasher_sha256;
    PARCCryptoHasher *hasher_sha512;
    PARCCryptoCache *key_cache;
};

static bool
_parcInMemoryVerifier_Destructor(PARCInMemoryVerifier **verifierPtr)
{
    PARCInMemoryVerifier *verifier = *verifierPtr;

    parcCryptoHasher_Release(&(verifier->hasher_sha256));
    parcCryptoHasher_Release(&(verifier->hasher_sha512));
    parcCryptoCache_Destroy(&(verifier->key_cache));
    parcMemory_Deallocate((void **)verifierPtr);
    *verifierPtr = NULL;

    return true;
}

parcObject_ImplementAcquire(parcInMemoryVerifier, PARCInMemoryVerifier);
parcObject_ImplementRelease(parcInMemoryVerifier, PARCInMemoryVerifier);

parcObject_Override(PARCInMemoryVerifier, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcInMemoryVerifier_Destructor);

PARCInMemoryVerifier *
parcInMemoryVerifier_Create()
{
    PARCInMemoryVerifier *verifier = parcObject_CreateInstance(PARCInMemoryVerifier);
    if (verifier != NULL) {
        // right now only support sha-256.  need to figure out how to make this flexible
        verifier->hasher_sha256 = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
        verifier->hasher_sha512 = parcCryptoHasher_Create(PARCCryptoHashType_SHA512);
        verifier->key_cache = parcCryptoCache_Create();
    }

    return verifier;
}


// ======================================

static PARCCryptoHasher *
_parcInMemoryVerifier_GetCryptoHasher(void *interfaceContext, PARCKeyId *keyid, PARCCryptoHashType hashType)
{
    PARCInMemoryVerifier *verifier = (PARCInMemoryVerifier *) interfaceContext;

    const PARCKey *key = parcCryptoCache_GetKey(verifier->key_cache, keyid);
    if (key == NULL) {
        return false;
    }

    switch (hashType) {
        case PARCCryptoHashType_SHA256:
            return verifier->hasher_sha256;

        case PARCCryptoHashType_SHA512:
            return verifier->hasher_sha512;

        default:
            parcTrapUnexpectedState("unsupported hash type: %d", hashType);
            return NULL;
    }
}

static bool
_parcInMemoryVerifier_AllowedCryptoSuite(void *interfaceContext, PARCKeyId *keyid, PARCCryptoSuite suite)
{
    PARCInMemoryVerifier *verifier = (PARCInMemoryVerifier *) interfaceContext;

    const PARCKey *key = parcCryptoCache_GetKey(verifier->key_cache, keyid);
    if (key == NULL) {
        return false;
    }

    switch (parcKey_GetSigningAlgorithm(key)) {
        case PARCSigningAlgorithm_RSA:
            switch (suite) {
                case PARCCryptoSuite_RSA_SHA256:
                    return true;

                case PARCCryptoSuite_RSA_SHA512:
                    return true;

                default:
                    return false;
            }
            break;

      case PARCSigningAlgorithm_ECDSA:
            switch (suite) {
                case PARCCryptoSuite_ECDSA_SHA256:
                    return true;

                default:
                    return false;
            }
            break;

      case PARCSigningAlgorithm_DSA:
            switch (suite) {
                default:
                    return false;
            }
            break;

      case PARCSigningAlgorithm_HMAC:
            switch (suite) {
                case PARCCryptoSuite_HMAC_SHA256:
                    return true;
                default:
                    return false;
            }
            break;

      default:
            parcTrapUnexpectedState("Unknown signing algorithm: %s",
                                parcSigningAlgorithm_ToString(parcKey_GetSigningAlgorithm(key)));
            return false;
    }

    return false;
}

static bool _parcInMemoryVerifier_RSAKey_Verify(PARCInMemoryVerifier *verifier, PARCCryptoHash *localHash,
                                                PARCSignature *signatureToVerify, PARCBuffer *derEncodedKey);

static bool _parcInMemoryVerifier_ECDSAKey_Verify(PARCInMemoryVerifier *verifier, PARCCryptoHash *localHash,
                                                  PARCSignature *signatureToVerify, PARCBuffer *derEncodedKey);

static bool _parcInMemoryVerifier_HMACKey_Verify(PARCCryptoHash *localHash, PARCSignature *signatureToVerify);
/**
 * The signature verifies if:
 * 0) we know the key for keyid
 * 1) the signing algorithm of the key corresponding to keyid is same as CCNxSignature
 * 2) The hash of the locallyComputedHash is the same type as the content object's ciphersuite
 * 3) the signature verifies
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_parcInMemoryVerifier_VerifyDigest(void *interfaceContext, PARCKeyId *keyid, PARCCryptoHash *locallyComputedHash,
                                   PARCCryptoSuite suite, PARCSignature *objectSignature)
{
    PARCInMemoryVerifier *verifier = (PARCInMemoryVerifier *) interfaceContext;

    const PARCKey *key = parcCryptoCache_GetKey(verifier->key_cache, keyid);
    if (key == NULL) {
        return false;
    }

    parcAssertTrue(_parcInMemoryVerifier_AllowedCryptoSuite(interfaceContext, keyid, suite), "Invalid crypto suite for keyid");

    if (parcKey_GetSigningAlgorithm(key) != parcSignature_GetSigningAlgorithm(objectSignature)) {
        fprintf(stdout, "Signatured failed, signing algorithms do not match: key %s sig %s\n",
                parcSigningAlgorithm_ToString(parcKey_GetSigningAlgorithm(key)),
                parcSigningAlgorithm_ToString(parcSignature_GetSigningAlgorithm(objectSignature)));
        return false;
    }

    if (parcCryptoHash_GetDigestType(locallyComputedHash) != parcCryptoSuite_GetCryptoHash(suite)) {
        fprintf(stdout, "Signatured failed, digest algorithms do not match: digest %s suite %s\n",
                parcCryptoHashType_ToString(parcCryptoHash_GetDigestType(locallyComputedHash)),
                parcCryptoHashType_ToString(parcCryptoSuite_GetCryptoHash(suite)));
        return false;
    }

    switch (parcSignature_GetSigningAlgorithm(objectSignature)) {
        case PARCSigningAlgorithm_RSA:
            return _parcInMemoryVerifier_RSAKey_Verify(verifier, locallyComputedHash, objectSignature, parcKey_GetKey(key));

        case PARCSigningAlgorithm_ECDSA:
            return _parcInMemoryVerifier_ECDSAKey_Verify(verifier, locallyComputedHash, objectSignature, parcKey_GetKey(key));

        case PARCSigningAlgorithm_HMAC:
            return _parcInMemoryVerifier_HMACKey_Verify(locallyComputedHash, objectSignature);

        case PARCSigningAlgorithm_DSA:
            parcTrapNotImplemented("DSA not supported");
            break;

        default:
            parcTrapUnexpectedState("Unknown signing algorithm: %d", parcSignature_GetSigningAlgorithm(objectSignature));
    }


    return false;
}

static void
_parcInMemoryVerifier_AddKey(void *interfaceContext, PARCKey *key)
{
    parcAssertNotNull(interfaceContext, "interfaceContext must be non-null");
    parcAssertNotNull(key, "key must be non-null");

    PARCInMemoryVerifier *verifier = (PARCInMemoryVerifier *) interfaceContext;
    bool success = parcCryptoCache_AddKey(verifier->key_cache, key);
    parcAssertTrue(success, "could not add key, it must be a duplicate");
}

static void
_parcInMemoryVerifier_RemoveKeyId(void *interfaceContext, PARCKeyId *keyid)
{
    parcAssertNotNull(interfaceContext, "interfaceContent must be non-null");
    parcAssertNotNull(keyid, "key must be non-null");

    PARCInMemoryVerifier *verifier = (PARCInMemoryVerifier *) interfaceContext;
    parcCryptoCache_RemoveKey(verifier->key_cache, keyid);
}

// ==============================================================
// Openssl specific parts

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

/**
 * Return if the signature and key verify with the local hash.
 *
 * PRECONDITION:
 *  - You know the signature and key are RSA.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_parcInMemoryVerifier_RSAKey_Verify(PARCInMemoryVerifier *verifier, PARCCryptoHash *localHash,
                                    PARCSignature *signatureToVerify, PARCBuffer *derEncodedKey)
{
    const uint8_t *der_bytes = parcByteArray_Array(parcBuffer_Array(derEncodedKey));

    long der_length = (long)parcBuffer_Remaining(derEncodedKey);
    EVP_PKEY *unwrapped_key = d2i_PUBKEY(NULL, &der_bytes, der_length);

    if (unwrapped_key != NULL) {
        int success = 0;
        RSA *rsa = EVP_PKEY_get1_RSA(unwrapped_key);

        if (rsa != NULL) {
            int openssl_digest_type;

            switch (parcCryptoHash_GetDigestType(localHash)) {
                case PARCCryptoHashType_SHA256:
                    openssl_digest_type = NID_sha256;
                    break;
                case PARCCryptoHashType_SHA512:
                    openssl_digest_type = NID_sha512;
                    break;
                default:
                    parcTrapUnexpectedState("Unknown digest type: %s",
                                        parcCryptoHashType_ToString(parcCryptoHash_GetDigestType(localHash)));
            }

            PARCBuffer *sigbits = parcSignature_GetSignature(signatureToVerify);
            PARCByteArray *bytearray = parcBuffer_Array(sigbits);
            unsigned signatureLength = (unsigned) parcBuffer_Remaining(sigbits);
            uint8_t *sigbuffer = parcByteArray_Array(bytearray);
            size_t signatureOffset = parcBuffer_ArrayOffset(sigbits);

            success = RSA_verify(openssl_digest_type,
                                 (unsigned char *) parcByteArray_Array(parcBuffer_Array(parcCryptoHash_GetDigest(localHash))),
                                 (unsigned) parcBuffer_Remaining(parcCryptoHash_GetDigest(localHash)),
                                 sigbuffer + signatureOffset,
                                 signatureLength,
                                 rsa);
            RSA_free(rsa);
        }
        EVP_PKEY_free(unwrapped_key);

        if (success == 1) {
            return true;
        }
    }
    return false;
}

/**
 * Return if the signature and key verify with the local hash.
 *
 * PRECONDITION:
 *  - You know the signature and key are ECDSA.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_parcInMemoryVerifier_ECDSAKey_Verify(PARCInMemoryVerifier *verifier, PARCCryptoHash *localHash,
                                      PARCSignature *signatureToVerify, PARCBuffer *derEncodedKey)
{
    const uint8_t *der_bytes = parcByteArray_Array(parcBuffer_Array(derEncodedKey));

    long der_length = (long)parcBuffer_Remaining(derEncodedKey);
    EVP_PKEY *unwrapped_key = d2i_PUBKEY(NULL, &der_bytes, der_length);

    if (unwrapped_key != NULL) {
        int success = 0;
        EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(unwrapped_key);

        if (ec_key != NULL) {
            int openssl_digest_type;

            switch (parcCryptoHash_GetDigestType(localHash)) {
                case PARCCryptoHashType_SHA256:
                    openssl_digest_type = NID_sha256;
                    break;
                case PARCCryptoHashType_SHA512:
                    openssl_digest_type = NID_sha512;
                    break;
                default:
                    parcTrapUnexpectedState("Unknown digest type: %s",
                                        parcCryptoHashType_ToString(parcCryptoHash_GetDigestType(localHash)));
            }

            PARCBuffer *sigbits = parcSignature_GetSignature(signatureToVerify);
            PARCByteArray *bytearray = parcBuffer_Array(sigbits);
            unsigned signatureLength = (unsigned) parcBuffer_Remaining(sigbits);
            uint8_t *sigbuffer = parcByteArray_Array(bytearray);
            size_t signatureOffset = parcBuffer_Position(sigbits);

            success = ECDSA_verify(openssl_digest_type,
                                 (unsigned char *) parcByteArray_Array(parcBuffer_Array(parcCryptoHash_GetDigest(localHash))),
                                 (unsigned) parcBuffer_Remaining(parcCryptoHash_GetDigest(localHash)),
                                 sigbuffer + signatureOffset,
                                 signatureLength,
                                 ec_key);
            EC_KEY_free(ec_key);
        }
        EVP_PKEY_free(unwrapped_key);

        if (success == 1) {
            return true;
        }
    }
    return false;
}

/**
 * Return if the signature verify with the local hash.
 *
 * PRECONDITION:
 *  - You know the signature is HMAC.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_parcInMemoryVerifier_HMACKey_Verify(PARCCryptoHash *localHash, PARCSignature *signatureToVerify)
{
  return parcBuffer_Equals(parcCryptoHash_GetDigest(localHash), parcSignature_GetSignature(signatureToVerify));
}

PARCVerifierInterface *PARCInMemoryVerifierAsVerifier = &(PARCVerifierInterface) {
    .GetCryptoHasher = _parcInMemoryVerifier_GetCryptoHasher,
    .VerifyDigest = _parcInMemoryVerifier_VerifyDigest,
    .AddKey = _parcInMemoryVerifier_AddKey,
    .RemoveKeyId = _parcInMemoryVerifier_RemoveKeyId,
    .AllowedCryptoSuite = _parcInMemoryVerifier_AllowedCryptoSuite,
};

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
