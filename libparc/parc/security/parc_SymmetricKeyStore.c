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
/**
 * To compute an HMAC, we need to interfaceement our own CryptoHasher so we can
 * initialize it with the secret key
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

#ifndef _WIN32
#include <unistd.h>
#endif

#include <config.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/safestack.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/hmac.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_KeyStore.h>
#include <parc/security/parc_SymmetricKeyStore.h>

#define AES_KEYSTORE_VERSION 1L
#define IV_SIZE 16
#define AES_MAX_DIGEST_SIZE 128
#define AES_DEFAULT_DIGEST_ALGORITHM "SHA256"

struct parc_symmetric_keystore {
    PARCBuffer *secretKey;
};

static PARCBuffer *
_createDerivedKey(const char *key, size_t keylength, unsigned char *salt, unsigned int saltlen)
{
    unsigned char buffer[SHA256_DIGEST_LENGTH];
    HMAC(EVP_sha256(), key, (int) keylength, salt, saltlen, buffer, NULL);
    return parcBuffer_PutArray(parcBuffer_Allocate(SHA256_DIGEST_LENGTH), SHA256_DIGEST_LENGTH, buffer);
}

static PARCCryptoHash *
_getSecretKeyDigest(PARCSymmetricKeyStore *keyStore)
{
    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);

    parcCryptoHasher_UpdateBuffer(hasher, keyStore->secretKey);
    PARCCryptoHash *result = parcCryptoHasher_Finalize(hasher);
    parcCryptoHasher_Release(&hasher);

    return result;
}

static bool
_parcSymmetricKeyStore_Finalize(PARCSymmetricKeyStore **interfacePtr)
{
    PARCSymmetricKeyStore *keyStore = *interfacePtr;
    if (keyStore->secretKey != NULL) {
        parcBuffer_Release(&keyStore->secretKey);
    }
    return true;
}

PARCKeyStoreInterface *PARCSymmetricKeyStoreAsKeyStore = &(PARCKeyStoreInterface) {
    .getVerifierKeyDigest = (PARCKeyStoreGetVerifierKeyDigest *)  _getSecretKeyDigest,
    .getCertificateDigest = NULL,
    .getDEREncodedCertificate = NULL,
    .getDEREncodedPublicKey = NULL,
    .getDEREncodedPrivateKey = NULL,
};

parcObject_ImplementAcquire(parcSymmetricKeyStore, PARCSymmetricKeyStore);
parcObject_ImplementRelease(parcSymmetricKeyStore, PARCSymmetricKeyStore);

parcObject_Override(PARCSymmetricKeyStore, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcSymmetricKeyStore_Finalize);

/**
 * The openssl ASN1 representation of the PARC symmetric key keystore.
 * It will be written to disk in DER format with an i2d call
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef struct PARCAESKeystore_info_st {
    ASN1_INTEGER *version;
    ASN1_OBJECT *algorithm_oid;
    ASN1_OCTET_STRING *encrypted_key;
} _PARCSymmeticSignerFileStoreInfo;

// This generates a name that is not compliant with the PARC Naming conventions.
DECLARE_ASN1_FUNCTIONS(_PARCSymmeticSignerFileStoreInfo)

ASN1_SEQUENCE(_PARCSymmeticSignerFileStoreInfo) = {
    ASN1_SIMPLE(_PARCSymmeticSignerFileStoreInfo, version,       ASN1_INTEGER),
    ASN1_SIMPLE(_PARCSymmeticSignerFileStoreInfo, algorithm_oid, ASN1_OBJECT),
    ASN1_SIMPLE(_PARCSymmeticSignerFileStoreInfo, encrypted_key, ASN1_OCTET_STRING)
} ASN1_SEQUENCE_END(_PARCSymmeticSignerFileStoreInfo)

IMPLEMENT_ASN1_FUNCTIONS(_PARCSymmeticSignerFileStoreInfo)

static int
_i2d_AESKeystore_fp(FILE *fp, _PARCSymmeticSignerFileStoreInfo *aki)
{
    return ASN1_item_i2d_fp(ASN1_ITEM_rptr(_PARCSymmeticSignerFileStoreInfo), fp, aki);
}

static _PARCSymmeticSignerFileStoreInfo *
_d2iAESKeystoreFp(FILE *fp, _PARCSymmeticSignerFileStoreInfo *aki)
{
    return ASN1_item_d2i_fp(ASN1_ITEM_rptr(_PARCSymmeticSignerFileStoreInfo), fp, aki);
}

static bool
_createKeyStore(const char *filename, const char *password, PARCBuffer *key)
{
    FILE *fp = NULL;
    int fd = -1;
    int ans = -1;
    int nid;

    _PARCSymmeticSignerFileStoreInfo *keystore = NULL;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
#else
    EVP_CIPHER_CTX ctx;
#endif
    unsigned char *encrypted_key = NULL;

    int ekl = IV_SIZE + (int) parcBuffer_Remaining(key) + SHA256_DIGEST_LENGTH + AES_BLOCK_SIZE;
    int encrypt_length;

    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0600);

    if (fd == -1) {
        goto Bail;
    }
    fp = fdopen(fd, "wb");
    if (fp == NULL) {
        goto Bail;
    }

    PARCBuffer *aes_key = _createDerivedKey(password, strlen(password), (unsigned char *) "\0", 1);
    PARCBuffer *mac_key = _createDerivedKey(password, strlen(password), (unsigned char *) "\1", 1);

    encrypted_key = malloc(ekl);
    if (!encrypted_key) {
        goto Bail;
    }
    RAND_bytes(encrypted_key, IV_SIZE);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    EVP_CIPHER_CTX_init(ctx);
    if (!EVP_EncryptInit(ctx, EVP_aes_256_cbc(), parcByteArray_Array(parcBuffer_Array(aes_key)), encrypted_key)) {
        goto Bail;
    }

    unsigned char *p;
    p = encrypted_key + IV_SIZE;
    if (!EVP_EncryptUpdate(ctx, p, &encrypt_length, parcByteArray_Array(parcBuffer_Array(key)), (int) parcBuffer_Remaining(key))) {
        goto Bail;
    }
    p += encrypt_length;
    if (!EVP_EncryptFinal(ctx, p, &encrypt_length)) {
        goto Bail;
    }
#else
    EVP_CIPHER_CTX_init(&ctx);
    if (!EVP_EncryptInit(&ctx, EVP_aes_256_cbc(), parcByteArray_Array(parcBuffer_Array(aes_key)), encrypted_key)) {
        goto Bail;
    }

    unsigned char *p;
    p = encrypted_key + IV_SIZE;
    if (!EVP_EncryptUpdate(&ctx, p, &encrypt_length, parcByteArray_Array(parcBuffer_Array(key)), (int) parcBuffer_Remaining(key))) {
        goto Bail;
    }
    p += encrypt_length;
    if (!EVP_EncryptFinal(&ctx, p, &encrypt_length)) {
        goto Bail;
    }
#endif
    p += encrypt_length;
    HMAC(EVP_sha256(), parcByteArray_Array(parcBuffer_Array(mac_key)), SHA256_DIGEST_LENGTH, encrypted_key, p - encrypted_key, p, NULL);

    if (!(keystore = _PARCSymmeticSignerFileStoreInfo_new())) {
        goto Bail;
    }
    if (!(keystore->version = ASN1_INTEGER_new())) {
        goto Bail;
    }
    if (!ASN1_INTEGER_set(keystore->version, AES_KEYSTORE_VERSION)) {
        goto Bail;
    }

    keystore->algorithm_oid = OBJ_txt2obj(AES_DEFAULT_DIGEST_ALGORITHM, 0);
    nid = OBJ_obj2nid(keystore->algorithm_oid);
    if (nid == NID_undef) {
        goto Bail;  // Shouldn't happen now but could later if we support more algorithms
    }
    if (!ASN1_OCTET_STRING_set(keystore->encrypted_key, encrypted_key, ekl)) {
        goto Bail;
    }
    _i2d_AESKeystore_fp(fp, keystore);
    ans = 0;
    goto cleanup;

 Bail:
    ans = -1;
 cleanup:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    EVP_CIPHER_CTX_free(ctx);
#endif
    parcBuffer_Release(&aes_key);
    parcBuffer_Release(&mac_key);

    if (fp != NULL) {
        fclose(fp);
    }
    if (encrypted_key) {
        free(encrypted_key);
    }
    if (keystore) {
        _PARCSymmeticSignerFileStoreInfo_free(keystore);
    }
    if (fd != -1) {
        close(fd);
    }
    return (ans);
}

static PARCBuffer *
_AESKeyStoreInit(const char *filename, const char *password)
{
    PARCBuffer *secret_key = NULL;

    FILE *fp = NULL;
    _PARCSymmeticSignerFileStoreInfo *ki = NULL;
    int version;
    char oidstr[80];

    PARCBuffer *aes_key = NULL;
    PARCBuffer *mac_key = NULL;

    unsigned char check[SHA256_DIGEST_LENGTH];
    unsigned char *keybuf = NULL;
    int check_start;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
#else
    EVP_CIPHER_CTX ctx;
#endif
    int length = 0;
    int final_length = 0;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        goto Bail;
    }

    ki = _d2iAESKeystoreFp(fp, NULL);
    fclose(fp);
    if (ki == NULL) {
        goto Bail;
    }

    version = (int) ASN1_INTEGER_get(ki->version);
    if (version != AES_KEYSTORE_VERSION) {
        goto Bail;
    }

    OBJ_obj2txt(oidstr, sizeof(oidstr), ki->algorithm_oid, 0);
    if (strcasecmp(oidstr, AES_DEFAULT_DIGEST_ALGORITHM)) {
        goto Bail;
    }

    if (ki->encrypted_key->length < IV_SIZE + (SHA256_DIGEST_LENGTH * 2) + AES_BLOCK_SIZE) {
        goto Bail;
    }

    aes_key = _createDerivedKey(password, strlen(password), (unsigned char *) "\0", 1);
    mac_key = _createDerivedKey(password, strlen(password), (unsigned char *) "\1", 1);

    check_start = ki->encrypted_key->length - SHA256_DIGEST_LENGTH;
    HMAC(EVP_sha256(),
         parcByteArray_Array(parcBuffer_Array(mac_key)),
         SHA256_DIGEST_LENGTH,
         ki->encrypted_key->data,
         check_start,
         check,
         NULL);

    if (memcmp(&ki->encrypted_key->data[check_start], check, SHA256_DIGEST_LENGTH)) {
        goto Bail;
    }
    keybuf = malloc(SHA256_DIGEST_LENGTH + AES_BLOCK_SIZE);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    EVP_CIPHER_CTX_init(ctx);
    if (!EVP_DecryptInit(ctx, EVP_aes_256_cbc(), parcByteArray_Array(parcBuffer_Array(aes_key)), ki->encrypted_key->data)) {
        goto Bail;
    }
    if (!EVP_DecryptUpdate(ctx, keybuf, &length, &ki->encrypted_key->data[IV_SIZE],
                           ki->encrypted_key->length - IV_SIZE - SHA256_DIGEST_LENGTH)) {
        goto Bail;
    }

    if (!EVP_DecryptFinal(ctx, keybuf + length, &final_length)) {
        goto Bail;
    }
#else
    EVP_CIPHER_CTX_init(&ctx);
    if (!EVP_DecryptInit(&ctx, EVP_aes_256_cbc(), parcByteArray_Array(parcBuffer_Array(aes_key)), ki->encrypted_key->data)) {
        goto Bail;
    }
    if (!EVP_DecryptUpdate(&ctx, keybuf, &length, &ki->encrypted_key->data[IV_SIZE],
                           ki->encrypted_key->length - IV_SIZE - SHA256_DIGEST_LENGTH)) {
        goto Bail;
    }

    if (!EVP_DecryptFinal(&ctx, keybuf + length, &final_length)) {
        goto Bail;
    }
#endif
    secret_key = parcBuffer_CreateFromArray(keybuf, length);
    parcBuffer_Flip(secret_key);

    goto out;

 Bail:
    free(keybuf);

 out:
    if (aes_key) {
        parcBuffer_Release(&aes_key);
    }

    if (mac_key) {
        parcBuffer_Release(&mac_key);
    }
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    EVP_CIPHER_CTX_free(ctx);
#endif
    return secret_key;
}

/**
 * Create a symmetric (secret) key of the given bit length (e.g. 256)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *
parcSymmetricKeyStore_CreateKey(unsigned bits)
{
    parcAssertTrue((bits & 0x07) == 0, "bits must be a multiple of 8");

    unsigned keylength = bits / 8;
    uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * keylength);
    RAND_bytes(buffer, keylength);
    PARCBuffer *parcBuffer = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(keylength), keylength, buffer));
    free(buffer);
    return parcBuffer;
}

PARCBuffer *
parcSymmetricKeyStore_GetKey(PARCSymmetricKeyStore *keyStore)
{
    return keyStore->secretKey;
}

PARCCryptoHash *
parcSymmetricKeyStore_GetVerifierKeyDigest(PARCSymmetricKeyStore *keyStore)
{
    return _getSecretKeyDigest(keyStore);
}

/**
 * Creates a PARC format symmetric keystore.  It only contains a single key.
 *
 * The final filename will be "file_prefix.
 *
 * Returns 0 on success, -1 on error.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool
parcSymmetricKeyStore_CreateFile(const char *filename, const char *password, PARCBuffer *secret_key)
{
    parcAssertTrue(parcBuffer_Remaining(secret_key) > 0, "The secret_key buffer is not flipped.  See parcBuffer_Flip()");
    return _createKeyStore(filename, password, secret_key) == 0;
}

/**
 * Create a PKCS12 signing context for use in ccnx_Signing.  It is destroyed
 * by ccnx_Signing when the signing context is destroyed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSymmetricKeyStore *
parcSymmetricKeyStore_OpenFile(const char *filename, const char *password, PARCCryptoHashType hmacHashType)
{
    PARCBuffer *secretKey = _AESKeyStoreInit(filename, password);
    parcAssertNotNull(secretKey, "Could not read AES keystore %s", filename);

    PARCSymmetricKeyStore *keyStore = parcSymmetricKeyStore_Create(secretKey);
    parcBuffer_Release(&secretKey);

    return keyStore;
}

/**
 * Create a PKCS12 signing context for use in ccnx_Signing from the provided key.  It is destroyed
 * by parc_Signing when the signing context is destroyed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSymmetricKeyStore *
parcSymmetricKeyStore_Create(PARCBuffer *secret_key)
{
    PARCSymmetricKeyStore *keyStore = parcObject_CreateAndClearInstance(PARCSymmetricKeyStore);
    parcAssertNotNull(keyStore, "parcObject_CreateAndClearInstance returned NULL, cannot allocate keystore");

    keyStore->secretKey = parcBuffer_Acquire(secret_key);

    return keyStore;
}

