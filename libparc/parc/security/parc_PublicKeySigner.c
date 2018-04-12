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
 */
#include <config.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

#include <parc/security/parc_PublicKeySigner.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_CryptoHash.h>
#include <parc/security/parc_Security.h>

#include <openssl/pkcs12.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>

struct PARCPublicKeySigner {
    PARCKeyStore *keyStore;
    PARCSigningAlgorithm signingAlgorithm;
    PARCCryptoHashType hashType;
    PARCCryptoHasher *hasher;
};

static bool
_parcPublicKeySigner_Finalize(PARCPublicKeySigner **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCPublicKeySigner pointer.");

    PARCPublicKeySigner *instance = *instancePtr;

    if (instance->keyStore != NULL) {
        parcKeyStore_Release(&(instance->keyStore));
    }
    if (instance->hasher != NULL) {
        parcCryptoHasher_Release(&(instance->hasher));
    }

    return true;
}

void
parcPublicKeySigner_AssertValid(const PARCPublicKeySigner *instance)
{
    assertTrue(parcPublicKeySigner_IsValid(instance), "PARCPublicKeySigner is not valid.");
}

bool
parcPublicKeySigner_Equals(const PARCPublicKeySigner *x, const PARCPublicKeySigner *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (x->signingAlgorithm == y->signingAlgorithm) {
            if (x->hashType == y->hashType) {
                return true;
            }
        }
    }

    return result;
}

PARCHashCode
parcPublicKeySigner_HashCode(const PARCPublicKeySigner *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcPublicKeySigner_IsValid(const PARCPublicKeySigner *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

char *
parcPublicKeySigner_ToString(const PARCPublicKeySigner *instance)
{
    char *result = parcMemory_Format("PARCPublicKeySigner@%p\n", instance);

    return result;
}

parcObject_ImplementAcquire(parcPublicKeySigner, PARCPublicKeySigner);
parcObject_ImplementRelease(parcPublicKeySigner, PARCPublicKeySigner);

parcObject_Override(PARCPublicKeySigner, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcPublicKeySigner_Finalize,
                    .toString = (PARCObjectToString *) parcPublicKeySigner_ToString,
                    .equals = (PARCObjectEquals *) parcPublicKeySigner_Equals,
                    .hashCode = (PARCObjectHashCode *) parcPublicKeySigner_HashCode);

PARCPublicKeySigner *
parcPublicKeySigner_Create(PARCKeyStore *keyStore, PARCCryptoSuite suite)
{
    PARCPublicKeySigner *result = parcObject_CreateInstance(PARCPublicKeySigner);

    PARCSigningAlgorithm signAlgo = parcSigningAlgorithm_GetSigningAlgorithm(suite);
    PARCCryptoHashType hashType = parcCryptoSuite_GetCryptoHash(suite);


    if (result != NULL) {
        result->keyStore = parcKeyStore_Acquire(keyStore);
        result->signingAlgorithm = signAlgo;
        result->hashType = hashType;
        result->hasher = parcCryptoHasher_Create(hashType);
    }

    return result;
}

static PARCSigningAlgorithm
_GetSigningAlgorithm(PARCPublicKeySigner *signer)
{
    assertNotNull(signer, "Parameter must be non-null PARCCryptoHasher");
    return signer->signingAlgorithm;
}

static PARCCryptoHashType
_GetCryptoHashType(PARCPublicKeySigner *signer)
{
    assertNotNull(signer, "Parameter must be non-null PARCCryptoHasher");
    return signer->hashType;
}

static PARCCryptoHasher *
_GetCryptoHasher(PARCPublicKeySigner *signer)
{
    assertNotNull(signer, "Parameter must be non-null PARCCryptoHasher");
    return signer->hasher;
}

static PARCKeyStore *
_GetKeyStore(PARCPublicKeySigner *signer)
{
    assertNotNull(signer, "Parameter must be non-null PARCCryptoHasher");
    return signer->keyStore;
}

static inline int _SignDigestRSA(const PARCCryptoHash *digestToSign, PARCBuffer *privateKeyBuffer, int opensslDigestType, uint8_t ** sig, unsigned * sigLength)
{
    EVP_PKEY *privateKey = NULL;
    size_t keySize = parcBuffer_Remaining(privateKeyBuffer);
    uint8_t *bytes = parcBuffer_Overlay(privateKeyBuffer, keySize);
    privateKey = d2i_PrivateKey(EVP_PKEY_RSA, &privateKey, (const unsigned char **) &bytes, keySize);

    RSA *rsa = EVP_PKEY_get1_RSA(privateKey);
    *sig = parcMemory_Allocate(RSA_size(rsa));

    assertNotNull(*sig, "parcMemory_Allocate(%u) returned NULL", RSA_size(rsa));

    *sigLength = 0;
    PARCBuffer *bb_digest = parcCryptoHash_GetDigest(digestToSign);
    int result = RSA_sign(opensslDigestType,
                          (unsigned char *) parcByteArray_Array(parcBuffer_Array(bb_digest)),
                          (int) parcBuffer_Remaining(bb_digest),
                          *sig,
                          sigLength,
                          rsa);
    assertTrue(result == 1, "Got error from RSA_sign: %d", result);
    RSA_free(rsa);
    return result;
}

static inline int _SignDigestECDSA(const PARCCryptoHash *digestToSign, PARCBuffer *privateKeyBuffer, int opensslDigestType, uint8_t ** sig, unsigned * sigLength)
{
    EVP_PKEY *privateKey = NULL;
    size_t keySize = parcBuffer_Remaining(privateKeyBuffer);
    uint8_t *bytes = parcBuffer_Overlay(privateKeyBuffer, keySize);
    privateKey = d2i_PrivateKey(EVP_PKEY_EC, &privateKey, (const unsigned char **) &bytes, keySize);

    EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(privateKey);

    *sig = parcMemory_Allocate(ECDSA_size(ec_key));
    assertNotNull(sig, "parcMemory_Allocate(%u) returned NULL", ECDSA_size(ec_key));

    *sigLength = 0;
    PARCBuffer *bb_digest = parcCryptoHash_GetDigest(digestToSign);
    int result = ECDSA_sign(opensslDigestType,
                          (unsigned char *) parcByteArray_Array(parcBuffer_Array(bb_digest)),
                          (int) parcBuffer_Remaining(bb_digest),
                          *sig,
                          sigLength,
                          ec_key);
    assertTrue(result == 1, "Got error from ECDSA_sign: %d", result);
    EC_KEY_free(ec_key);

}

static PARCSignature *
_SignDigest(PARCPublicKeySigner *signer, const PARCCryptoHash *digestToSign)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(signer, "Parameter must be non-null CCNxFileKeystore");
    assertNotNull(digestToSign, "Buffer to sign must not be null");

    // TODO: what is the best way to expose this?
    PARCKeyStore *keyStore = signer->keyStore;
    PARCBuffer *privateKeyBuffer = parcKeyStore_GetDEREncodedPrivateKey(keyStore);

    int opensslDigestType;
    uint8_t *sig;
    unsigned sigLength;

    switch (parcCryptoHash_GetDigestType(digestToSign)) {
        case PARCCryptoHashType_SHA256:
            opensslDigestType = NID_sha256;
            break;
        case PARCCryptoHashType_SHA512:
            opensslDigestType = NID_sha512;
            break;
        default:
            trapUnexpectedState("Unknown digest type: %s",
                                parcCryptoHashType_ToString(parcCryptoHash_GetDigestType(digestToSign)));
    }

    switch (signer->signingAlgorithm) {
        case PARCSigningAlgorithm_RSA:
            _SignDigestRSA(digestToSign, privateKeyBuffer, opensslDigestType, &sig, &sigLength);
            break;
        case PARCSigningAlgorithm_ECDSA:
            _SignDigestECDSA(digestToSign, privateKeyBuffer, opensslDigestType, &sig, &sigLength);
            break;
        default:
            return NULL;
    }

    PARCBuffer *bbSign = parcBuffer_Allocate(sigLength);
    parcBuffer_Flip(parcBuffer_PutArray(bbSign, sigLength, sig));
    parcMemory_Deallocate((void **) &sig);

    PARCSignature *signature =
        parcSignature_Create(_GetSigningAlgorithm(signer),
                             parcCryptoHash_GetDigestType(digestToSign),
                             bbSign
                             );
    parcBuffer_Release(&bbSign);
    parcBuffer_Release(&privateKeyBuffer);
    return signature;
}

PARCSigningInterface *PARCPublicKeySignerAsSigner = &(PARCSigningInterface) {
    .GetCryptoHasher = (PARCCryptoHasher * (*)(void *))_GetCryptoHasher,
    .SignDigest = (PARCSignature * (*)(void *, const PARCCryptoHash *))_SignDigest,
    .GetSigningAlgorithm = (PARCSigningAlgorithm (*)(void *))_GetSigningAlgorithm,
    .GetCryptoHashType = (PARCCryptoHashType (*)(void *))_GetCryptoHashType,
    .GetKeyStore = (PARCKeyStore * (*)(void *))_GetKeyStore,
};
