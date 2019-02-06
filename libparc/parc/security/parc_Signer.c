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
#include <stdio.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <parc/security/parc_Signer.h>
#include <parc/security/parc_KeyStore.h>

struct parc_signer {
    PARCObject *instance;
    PARCSigningInterface *interface;
};

static bool
_parcSigner_FinalRelease(PARCSigner **signerPtr)
{
    PARCSigner *signer = *signerPtr;
    if (signer->instance != NULL) {
        parcObject_Release(&(signer->instance));
    }
    return true;
}

void
parcSigner_AssertValid(const PARCSigner *signer)
{
    parcAssertNotNull(signer, "Parameter must be non-null PARCSigner");
}

parcObject_ImplementAcquire(parcSigner, PARCSigner);
parcObject_ImplementRelease(parcSigner, PARCSigner);

parcObject_Override(PARCSigner, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcSigner_FinalRelease);

PARCSigner *
parcSigner_Create(PARCObject *instance, PARCSigningInterface *interfaceContext)
{
    parcAssertNotNull(interfaceContext, "Parameter must be non-null implementation pointer");

    PARCSigner *signer = parcObject_CreateInstance(PARCSigner);
    if (signer != NULL) {
        signer->instance = parcObject_Acquire(instance);
        signer->interface = interfaceContext;
    }
    return signer;
}

PARCKey *
parcSigner_CreatePublicKey(PARCSigner *signer)
{
    PARCKeyStore *keyStore = parcSigner_GetKeyStore(signer);

    PARCCryptoHash *hash = parcKeyStore_GetVerifierKeyDigest(keyStore);

    PARCKeyId *keyid = parcKeyId_Create(parcCryptoHash_GetDigest(hash));
    parcCryptoHash_Release(&hash);

    PARCBuffer *derEncodedKey = parcKeyStore_GetDEREncodedPublicKey(keyStore);

    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid,
                                                         parcSigner_GetSigningAlgorithm(signer),
                                                         derEncodedKey);

    parcBuffer_Release(&derEncodedKey);
    parcKeyId_Release(&keyid);

    return key;
}

PARCKeyId *
parcSigner_CreateKeyId(const PARCSigner *signer)
{
    PARCCryptoHash *hash = parcKeyStore_GetVerifierKeyDigest(parcSigner_GetKeyStore(signer));
    PARCBuffer *keyidBytes = parcCryptoHash_GetDigest(hash);
    PARCKeyId *result = parcKeyId_Create(keyidBytes);

    parcCryptoHash_Release(&hash);
    return result;
}

PARCCryptoHasher *
parcSigner_GetCryptoHasher(const PARCSigner *signer)
{
    parcSigner_OptionalAssertValid(signer);

    return signer->interface->GetCryptoHasher(signer->instance);
}

PARCSignature *
parcSigner_SignDigest(const PARCSigner *signer, const PARCCryptoHash *parcDigest, uint8_t * signature, uint32_t sig_len)
{
    parcSigner_OptionalAssertValid(signer);

    parcAssertNotNull(parcDigest, "parcDigest to sign must not be null");
    return signer->interface->SignDigest(signer->instance, parcDigest, signature, sig_len);
}

PARCSignature *
parcSigner_SignBuffer(const PARCSigner *signer, const PARCBuffer *buffer, uint8_t * signature_buf, uint32_t sig_len)
{
    parcSigner_OptionalAssertValid(signer);
    parcAssertNotNull(buffer, "buffer to sign must not be null");

    PARCCryptoHashType hashType = parcSigner_GetCryptoHashType(signer);
    PARCCryptoHasher *hasher = parcCryptoHasher_Create(hashType);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBuffer(hasher, buffer);
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    parcCryptoHasher_Release(&hasher);

    PARCSignature *signature = parcSigner_SignDigest(signer, hash, signature_buf, sig_len);
    parcCryptoHash_Release(&hash);

    return signature;
}

PARCSigningAlgorithm
parcSigner_GetSigningAlgorithm(PARCSigner *signer)
{
    parcSigner_OptionalAssertValid(signer);

    return signer->interface->GetSigningAlgorithm(signer->instance);
}

PARCCryptoHashType
parcSigner_GetCryptoHashType(const PARCSigner *signer)
{
    parcSigner_OptionalAssertValid(signer);

    return signer->interface->GetCryptoHashType(signer->instance);
}

PARCCryptoSuite
parcSigner_GetCryptoSuite(const PARCSigner *signer)
{
    parcSigner_OptionalAssertValid(signer);

    PARCCryptoHashType hash = signer->interface->GetCryptoHashType(signer->instance);
    PARCSigningAlgorithm signAlgo = signer->interface->GetSigningAlgorithm(signer->instance);
    return parcCryptoSuite_GetFromSigningHash(signAlgo, hash);
}

PARCKeyStore *
parcSigner_GetKeyStore(const PARCSigner *signer)
{
    parcSigner_OptionalAssertValid(signer);

    return signer->interface->GetKeyStore(signer->instance);
}

size_t
parcSigner_GetSignatureSize(const PARCSigner *signer)
{
    parcSigner_OptionalAssertValid(signer);

    return signer->interface->GetSignatureSize(signer->instance);
}
