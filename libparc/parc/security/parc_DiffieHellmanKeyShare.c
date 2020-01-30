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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

#include <parc/security/parc_DiffieHellmanKeyShare.h>
#include <parc/security/parc_CryptoHasher.h>
#include <parc/security/parc_CryptoHash.h>

#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bn.h>

struct parc_diffie_hellman_keyshare {
    PARCDiffieHellmanGroup groupType;
    EVP_PKEY *privateKey;
};

static bool
_parcDiffieHellmanKeyShare_Destructor(PARCDiffieHellmanKeyShare **pointer)
{
    PARCDiffieHellmanKeyShare *share = *pointer;

    if (share->privateKey != NULL) {
        EVP_PKEY_free(share->privateKey);
    }

    return true;
}

parcObject_Override(PARCDiffieHellmanKeyShare, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcDiffieHellmanKeyShare_Destructor);

parcObject_ImplementAcquire(parcDiffieHellmanKeyShare, PARCDiffieHellmanKeyShare);
parcObject_ImplementRelease(parcDiffieHellmanKeyShare, PARCDiffieHellmanKeyShare);

static EVP_PKEY *
_parcDiffieHellmanKeyShare_CreateShare(int curveid)
{
    EVP_PKEY_CTX *pctx;
    EVP_PKEY_CTX *kctx;

    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    if (pctx == NULL) {
        return NULL;
    }

    int result = EVP_PKEY_paramgen_init(pctx);
    if (result != 1) {
        EVP_PKEY_CTX_free(pctx);
        return NULL;
    }

    result = EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, curveid);
    if (result != 1) {
        EVP_PKEY_CTX_free(pctx);
        return NULL;
    }

    EVP_PKEY *params = NULL;
    result = EVP_PKEY_paramgen(pctx, &params);
    if (result != 1) {
        EVP_PKEY_CTX_free(pctx);
        return NULL;
    }

    kctx = EVP_PKEY_CTX_new(params, NULL);
    if (kctx == NULL) {
        EVP_PKEY_CTX_free(pctx);
        EVP_PKEY_free(params);
        return NULL;
    }

    result = EVP_PKEY_keygen_init(kctx);
    if (result != 1) {
        EVP_PKEY_CTX_free(pctx);
        EVP_PKEY_free(params);
        return NULL;
    }

    EVP_PKEY *pkey = NULL;
    result = EVP_PKEY_keygen(kctx, &pkey);
    if (result != 1) {
        EVP_PKEY_CTX_free(pctx);
        EVP_PKEY_free(params);
        EVP_PKEY_CTX_free(kctx);
        return NULL;
    }

    EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_CTX_free(kctx);
    EVP_PKEY_free(params);

    return pkey;
}

PARCDiffieHellmanKeyShare *
parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup groupType)
{
    PARCDiffieHellmanKeyShare *keyShare = parcObject_CreateInstance(PARCDiffieHellmanKeyShare);

    if (keyShare != NULL) {
        keyShare->groupType = groupType;

        switch (groupType) {
            case PARCDiffieHellmanGroup_Prime256v1:
                keyShare->privateKey = _parcDiffieHellmanKeyShare_CreateShare(NID_X9_62_prime256v1);
                break;
            case PARCDiffieHellmanGroup_Secp521r1:
                keyShare->privateKey = _parcDiffieHellmanKeyShare_CreateShare(NID_secp521r1);
                break;
            case PARCDiffieHellmanGroup_Curve2559:
            default:
                break;
        }

        if (keyShare->privateKey == NULL) {
            parcAssertTrue(false, "Unable to instantiate a private key.");
            parcDiffieHellmanKeyShare_Release(&keyShare);
        }
    }

    return keyShare;
}

PARCBuffer *
parcDiffieHellmanKeyShare_SerializePublicKey(PARCDiffieHellmanKeyShare *keyShare)
{
    EC_KEY *ecKey = EVP_PKEY_get1_EC_KEY(keyShare->privateKey);

    BN_CTX *bnctx = BN_CTX_new();
    point_conversion_form_t form = EC_KEY_get_conv_form(ecKey);
    const EC_POINT *point = EC_KEY_get0_public_key(ecKey);
    const EC_GROUP *group = EC_KEY_get0_group(ecKey);
    char *keyBuffer = EC_POINT_point2hex(group, point, form, bnctx);
    int length = (int)strlen(keyBuffer);

    PARCBuffer *publicKey = parcBuffer_Allocate(length);
    parcBuffer_PutArray(publicKey, length, (uint8_t *) keyBuffer);
    parcBuffer_Flip(publicKey);

    free(keyBuffer);
    BN_CTX_free(bnctx);

    return publicKey;
}

static EVP_PKEY *
_parcDiffieHellman_DeserializePublicKeyShare(PARCDiffieHellmanKeyShare *keyShare, PARCBuffer *keyBuffer)
{
    EC_KEY *ecKey = EVP_PKEY_get1_EC_KEY(keyShare->privateKey);
    const EC_GROUP *myGroup = EC_KEY_get0_group(ecKey);

    EC_KEY *newKey = EC_KEY_new();
    BN_CTX *newCtx = BN_CTX_new();
    int result = EC_KEY_set_group(newKey, myGroup);
    if (result != 1) {
        BN_CTX_free(newCtx);
        EC_KEY_free(newKey);
        return NULL;
    }

    EC_POINT *newPoint = EC_POINT_new(myGroup);
    char *keyString = parcBuffer_ToString(keyBuffer);
    newPoint = EC_POINT_hex2point(myGroup, keyString, newPoint, newCtx);
    if (newPoint == NULL) {
        parcMemory_Deallocate(&keyString);
        BN_CTX_free(newCtx);
        EC_KEY_free(newKey);
        EC_POINT_free(newPoint);
        return NULL;
    }

    result = EC_KEY_set_public_key(newKey, newPoint);
    if (result != 1) {
        parcMemory_Deallocate(&keyString);
        BN_CTX_free(newCtx);
        EC_KEY_free(newKey);
        EC_POINT_free(newPoint);
        return NULL;
    }

    EVP_PKEY *peerkey = EVP_PKEY_new();
    result = EVP_PKEY_set1_EC_KEY(peerkey, newKey);
    if (result != 1) {
        parcMemory_Deallocate(&keyString);
        BN_CTX_free(newCtx);
        EC_KEY_free(newKey);
        EC_POINT_free(newPoint);
        EVP_PKEY_free(peerkey);
        return NULL;
    }

    BN_CTX_free(newCtx);
    EC_KEY_free(newKey);
    parcMemory_Deallocate((void **) &keyString);
    EC_POINT_free(newPoint);

    return peerkey;
}

static PARCBuffer *
_parcDiffieHellmanKeyShare_HashSharedSecret(PARCBuffer *secret)
{
    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBuffer(hasher, secret);
    PARCCryptoHash *digest = parcCryptoHasher_Finalize(hasher);

    PARCBuffer *sharedSecret = parcBuffer_Acquire(parcCryptoHash_GetDigest(digest));

    parcCryptoHash_Release(&digest);
    parcCryptoHasher_Release(&hasher);

    return sharedSecret;
}

PARCBuffer *
parcDiffieHellmanKeyShare_Combine(PARCDiffieHellmanKeyShare *keyShare, PARCBuffer *theirs)
{
    EVP_PKEY *peerkey = _parcDiffieHellman_DeserializePublicKeyShare(keyShare, theirs);
    if (peerkey == NULL) {
        return NULL;
    }

    EVP_PKEY *privateKey = keyShare->privateKey;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(privateKey, NULL);
    if (ctx == NULL) {
        EVP_PKEY_free(peerkey);
        return NULL;
    }

    int result = EVP_PKEY_derive_init(ctx);
    if (result != 1) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peerkey);
        return NULL;
    }

    result = EVP_PKEY_derive_set_peer(ctx, peerkey);
    if (result != 1) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peerkey);
        return NULL;
    }

    size_t secretLength = 0;
    result = EVP_PKEY_derive(ctx, NULL, &secretLength);
    if (result != 1) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peerkey);
        return NULL;
    }

    unsigned char *secret = OPENSSL_malloc(secretLength);
    if (secret == NULL) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peerkey);
        return NULL;
    }

    result = EVP_PKEY_derive(ctx, secret, &secretLength);
    if (result != 1) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peerkey);
        OPENSSL_free(secret);
        return NULL;
    }

    PARCBuffer *secretBuffer = parcBuffer_Allocate(secretLength);
    parcBuffer_PutArray(secretBuffer, secretLength, secret);
    parcBuffer_Flip(secretBuffer);

    PARCBuffer *sharedSecret = _parcDiffieHellmanKeyShare_HashSharedSecret(secretBuffer);
    parcBuffer_Release(&secretBuffer);

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(peerkey);
    OPENSSL_free(secret);

    return sharedSecret;
}
