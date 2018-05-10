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
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <LongBow/runtime.h>
#include <LongBow/longBow_Compiler.h>

#include <parc/algol/parc_Object.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_Signer.h>
#include <parc/security/parc_KeyStore.h>
#include <parc/algol/parc_Memory.h>

#include <parc/security/parc_Certificate.h>
#include <parc/security/parc_CertificateFactory.h>
#include <parc/security/parc_CertificateType.h>
#include <parc/security/parc_ContainerEncoding.h>
#include <parc/security/parc_KeyType.h>

#include <parc/security/parc_Pkcs12KeyStore.h>

#include <openssl/pkcs12.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>

struct parc_pkcs12_keystore {
    EVP_PKEY *private_key;
    EVP_PKEY *public_key;
    X509 *x509_cert;
    PARCSigningAlgorithm signAlgo;

    // These will be 0 length until asked for
    PARCBuffer *public_key_digest;
    PARCBuffer *certificate_digest;
    PARCBuffer *public_key_der;
    PARCBuffer *certificate_der;
    PARCBuffer *private_key_der;

    PARCCryptoHashType hashType;
    PARCCryptoHasher *hasher;
};

static bool
_parcPkcs12KeyStore_Finalize(PARCPkcs12KeyStore **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCPkcs12KeyStore pointer.");
    PARCPkcs12KeyStore *keystore = *instancePtr;

    EVP_PKEY_free(keystore->private_key);
    EVP_PKEY_free(keystore->public_key);
    X509_free(keystore->x509_cert);

    if (keystore->public_key_digest != NULL) {
        parcBuffer_Release(&keystore->public_key_digest);
    }
    if (keystore->certificate_digest != NULL) {
        parcBuffer_Release(&keystore->certificate_digest);
    }
    if (keystore->public_key_der != NULL) {
        parcBuffer_Release(&keystore->public_key_der);
    }
    if (keystore->certificate_der != NULL) {
        parcBuffer_Release(&keystore->certificate_der);
    }
    if (keystore->private_key_der != NULL) {
        parcBuffer_Release(&keystore->private_key_der);
    }

    parcCryptoHasher_Release(&(keystore->hasher));

    return true;
}

parcObject_ImplementAcquire(parcPkcs12KeyStore, PARCPkcs12KeyStore);
parcObject_ImplementRelease(parcPkcs12KeyStore, PARCPkcs12KeyStore);
parcObject_Override(PARCPkcs12KeyStore, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcPkcs12KeyStore_Finalize);

static int
_parcPkcs12KeyStore_ParseFile(PARCPkcs12KeyStore *keystore, const char *filename, const char *password)
{
    parcSecurity_AssertIsInitialized();

    FILE *fp = fopen(filename, "rb");

    assertNotNull(fp, "Error opening %s: %s", filename, strerror(errno));
    if (fp == NULL) {
        return -1;
    }

    PKCS12 *p12Keystore = NULL;
    d2i_PKCS12_fp(fp, &p12Keystore);
    fclose(fp);

    int success = PKCS12_parse(p12Keystore, password, &keystore->private_key, &keystore->x509_cert, NULL);
    PKCS12_free(p12Keystore);

    if (!success) {
        unsigned long errcode;
        while ((errcode = ERR_get_error()) != 0) {
            fprintf(stderr, "openssl error: %s\n", ERR_error_string(errcode, NULL));
        }
        return -1;
    }

    keystore->public_key = X509_get_pubkey(keystore->x509_cert);
    if (keystore->public_key) {
        switch (keystore->public_key->type) {
            case EVP_PKEY_RSA:
                keystore->signAlgo = PARCSigningAlgorithm_RSA;
                break;
            case EVP_PKEY_DSA:
                keystore->signAlgo = PARCSigningAlgorithm_DSA;
                break;
            case EVP_PKEY_EC:
                keystore->signAlgo = PARCSigningAlgorithm_ECDSA;
                break;
            default:
                fprintf(stderr, "%d bit unknown Key type\n\n", EVP_PKEY_bits(keystore->public_key));
                break;
        }
    }
    return 0;
}

// =============================================================
LONGBOW_STOP_DEPRECATED_WARNINGS
// =============================================================

PKCS12 *_createPkcs12KeyStore_RSA(
    PARCBuffer *privateKeyBuffer,
    PARCCertificate *certificate,
    const char *password)
{
    // Extract the private key
    EVP_PKEY *privateKey = NULL;
    uint8_t *privateKeyBytes = parcBuffer_Overlay(privateKeyBuffer, parcBuffer_Limit(privateKeyBuffer));
    d2i_PrivateKey(EVP_PKEY_RSA, &privateKey, (const unsigned char **) &privateKeyBytes, parcBuffer_Limit(privateKeyBuffer));
    parcBuffer_Release(&privateKeyBuffer);
    
    // Extract the certificate
    PARCBuffer *certBuffer = parcCertificate_GetDEREncodedCertificate(certificate);
    uint8_t *certBytes = parcBuffer_Overlay(certBuffer, parcBuffer_Limit(certBuffer));
    X509 *cert = NULL;
    d2i_X509(&cert, (const unsigned char **) &certBytes, parcBuffer_Limit(certBuffer));
    
    parcCertificate_Release(&certificate);
    
    PKCS12 *pkcs12 = PKCS12_create((char *) password,
                                   "ccnxuser",
                                   privateKey,
                                   cert,
                                   NULL,
                                   0,
                                   0,
                                   0 /*default iter*/,
                                   PKCS12_DEFAULT_ITER /*mac_iter*/,
                                   0);
    X509_free(cert);
    EVP_PKEY_free(privateKey);
    return pkcs12;
}

PKCS12 *_createPkcs12KeyStore_ECDSA(
    PARCBuffer *privateKeyBuffer,
    PARCCertificate *certificate,
    const char *password)
{
    // Extract the private key
    EVP_PKEY *privateKey = NULL;
    uint8_t *privateKeyBytes = parcBuffer_Overlay(privateKeyBuffer, parcBuffer_Limit(privateKeyBuffer));
    d2i_PrivateKey(EVP_PKEY_EC, &privateKey, (const unsigned char **) &privateKeyBytes, parcBuffer_Limit(privateKeyBuffer));
    parcBuffer_Release(&privateKeyBuffer);
    
    // Extract the certificate
    PARCBuffer *certBuffer = parcCertificate_GetDEREncodedCertificate(certificate);
    uint8_t *certBytes = parcBuffer_Overlay(certBuffer, parcBuffer_Limit(certBuffer));
    X509 *cert = NULL;
    d2i_X509(&cert, (const unsigned char **) &certBytes, parcBuffer_Limit(certBuffer));
    
    parcCertificate_Release(&certificate);
    
    PKCS12 *pkcs12 = PKCS12_create((char *) password,
                                   "ccnxuser",
                                   privateKey,
                                   cert,
                                   NULL,
                                   0,
                                   0,
                                   0 /*default iter*/,
                                   PKCS12_DEFAULT_ITER /*mac_iter*/,
                                   0);
    X509_free(cert);
    EVP_PKEY_free(privateKey);
    return pkcs12;
}

bool
parcPkcs12KeyStore_CreateFile(
    const char *filename,
    const char *password,
    const char *subjectName,
    PARCSigningAlgorithm signAlgo,
    unsigned keyLength,
    unsigned validityDays)
{
    parcSecurity_AssertIsInitialized();

    bool result = false;

    PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_DER);

    PARCBuffer *privateKeyBuffer;
    PARCCertificate *certificate = parcCertificateFactory_CreateSelfSignedCertificate(factory, &privateKeyBuffer, (char *) subjectName,signAlgo, keyLength, validityDays);

    parcCertificateFactory_Release(&factory);

    if (certificate != NULL) {

        PKCS12 *pkcs12;
        switch (signAlgo){
            case PARCSigningAlgorithm_RSA:
                pkcs12 = _createPkcs12KeyStore_RSA(privateKeyBuffer, certificate, password);
                break;
            case PARCSigningAlgorithm_ECDSA:
                pkcs12 = _createPkcs12KeyStore_ECDSA(privateKeyBuffer, certificate, password);
                break;
            default:
                return result;
        }
        
        if (pkcs12 != NULL) {
            int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0600);
            if (fd != -1) {
                FILE *fp = fdopen(fd, "wb");
                if (fp != NULL) {
                    i2d_PKCS12_fp(fp, pkcs12);
                    fclose(fp);
                    result = true;
                } else {
                    trapUnrecoverableState("Cannot fdopen(3) the file descriptor %d", fd);
                }
                close(fd);
            } else {
                trapUnrecoverableState("Cannot open(2) the file '%s': %s", filename, strerror(errno));
            }
            PKCS12_free(pkcs12);
        } else {
            unsigned long errcode;
            while ((errcode = ERR_get_error()) != 0) {
                fprintf(stderr, "openssl error: %s\n", ERR_error_string(errcode, NULL));
            }
            trapUnrecoverableState("PKCS12_create returned a NULL value.");
        }
    }

    return result;
}

PARCPkcs12KeyStore *
parcPkcs12KeyStore_Open(const char *filename, const char *password, PARCCryptoHashType hashType)
{
    PARCPkcs12KeyStore *keyStore = parcObject_CreateAndClearInstance(PARCPkcs12KeyStore);
    if (keyStore != NULL) {
        keyStore->hasher = parcCryptoHasher_Create(hashType);
        keyStore->public_key_digest = NULL;
        keyStore->certificate_digest = NULL;
        keyStore->public_key_der = NULL;
        keyStore->certificate_der = NULL;
        keyStore->hashType = hashType;

        if (_parcPkcs12KeyStore_ParseFile(keyStore, filename, password) != 0) {
            parcPkcs12KeyStore_Release(&keyStore);
        }
    }

    return keyStore;
}

PARCSigningAlgorithm _GetSigningAlgorithm(PARCPkcs12KeyStore *keystore)
{
    return keystore->signAlgo;
}

static PARCCryptoHash *
_GetPublickKeyDigest(PARCPkcs12KeyStore *keystore)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(keystore, "Parameter must be non-null PARCPkcs12KeyStore");

#if 0
    if (keystore->public_key_digest == NULL) {
        AUTHORITY_KEYID  *akid = X509_get_ext_d2i(keystore->x509_cert, NID_authority_key_identifier, NULL, NULL);
        if (akid != NULL) {
            ASN1_OCTET_STRING *skid = X509_get_ext_d2i(keystore->x509_cert, NID_subject_key_identifier, NULL, NULL);
            if (skid != NULL) {
                keystore->public_key_digest =
                    parcBuffer_PutArray(parcBuffer_Allocate(skid->length), skid->length, skid->data);
                parcBuffer_Flip(keystore->public_key_digest);
                ASN1_OCTET_STRING_free(skid);
            }
            AUTHORITY_KEYID_free(akid);
        }
    }
#endif

    // If we could not load the digest from the certificate, then calculate it from the public key.
    if (keystore->public_key_digest == NULL) {
        uint8_t digestBuffer[SHA256_DIGEST_LENGTH];

        int result = ASN1_item_digest(ASN1_ITEM_rptr(X509_PUBKEY),
                                      EVP_sha256(),
                                      X509_get_X509_PUBKEY(keystore->x509_cert),
                                      digestBuffer,
                                      NULL);
        if (result != 1) {
            assertTrue(0, "Could not compute digest over certificate public key");
        } else {
            keystore->public_key_digest =
                parcBuffer_PutArray(parcBuffer_Allocate(SHA256_DIGEST_LENGTH), SHA256_DIGEST_LENGTH, digestBuffer);
            parcBuffer_Flip(keystore->public_key_digest);
        }
    }

    // This stores a reference, so keystore->public_key_digest will remain valid
    // even if the cryptoHasher is destroyed
    return parcCryptoHash_Create(PARCCryptoHashType_SHA256, keystore->public_key_digest);
}

static PARCCryptoHash *
_GetCertificateDigest(PARCPkcs12KeyStore *keystore)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(keystore, "Parameter must be non-null PARCPkcs12KeyStore");

    if (keystore->certificate_digest == NULL) {
        uint8_t digestBuffer[SHA256_DIGEST_LENGTH];
        int result = X509_digest(keystore->x509_cert, EVP_sha256(), digestBuffer, NULL);
        if (result) {
            keystore->certificate_digest =
                parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(SHA256_DIGEST_LENGTH), SHA256_DIGEST_LENGTH, digestBuffer));
        }
    }

    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, keystore->certificate_digest);
    return hash;
}

static PARCBuffer *
_GetDEREncodedCertificate(PARCPkcs12KeyStore *keystore)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(keystore, "Parameter must be non-null PARCPkcs12KeyStore");

    if (keystore->certificate_der == NULL) {
        uint8_t *der = NULL;

        // this allocates memory for der
        int derLength = i2d_X509(keystore->x509_cert, &der);
        if (derLength > 0) {
            keystore->certificate_der =
                parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(derLength), derLength, der));
        }
        OPENSSL_free(der);
    }

    return parcBuffer_Copy(keystore->certificate_der);
}

static PARCBuffer *
_GetDEREncodedPublicKey(PARCPkcs12KeyStore *keystore)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(keystore, "Parameter must be non-null PARCPkcs12KeyStore");

    if (keystore->public_key_der == NULL) {
        uint8_t *der = NULL;

        // this allocates memory for der
        int derLength = i2d_PUBKEY(keystore->public_key, &der);
        if (derLength > 0) {
            keystore->public_key_der =
                parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(derLength), derLength, der));
        }
        OPENSSL_free(der);
    }

    return parcBuffer_Copy(keystore->public_key_der);
}

static PARCBuffer *
_GetDEREncodedPrivateKey(PARCPkcs12KeyStore *keystore)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(keystore, "Parameter must be non-null PARCPkcs12KeyStore");

    if (keystore->private_key_der == NULL) {
        uint8_t *der = NULL;

        // this allocates memory for der
        int derLength = i2d_PrivateKey(keystore->private_key, &der);
        if (derLength > 0) {
            keystore->private_key_der =
                parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(derLength), derLength, der));
        }
        OPENSSL_free(der);
    }

    return parcBuffer_Copy(keystore->private_key_der);
}

PARCKeyStoreInterface *PARCPkcs12KeyStoreAsKeyStore = &(PARCKeyStoreInterface) {
    .getVerifierKeyDigest = (PARCKeyStoreGetVerifierKeyDigest *) _GetPublickKeyDigest,
    .getCertificateDigest = (PARCKeyStoreGetCertificateDigest *) _GetCertificateDigest,
    .getDEREncodedCertificate = (PARCKeyStoreGetDEREncodedCertificate *) _GetDEREncodedCertificate,
    .getDEREncodedPublicKey = (PARCKeyStoreGetDEREncodedPublicKey *) _GetDEREncodedPublicKey,
    .getDEREncodedPrivateKey = (PARCKeyStoreGetDEREncodedPrivateKey *) _GetDEREncodedPrivateKey,
    .getSigningAlgorithm = (PARCKeyStoreGetSigningAlgorithm *) _GetSigningAlgorithm,
};

// =============================================================
LONGBOW_START_DEPRECATED_WARNINGS
// =============================================================
