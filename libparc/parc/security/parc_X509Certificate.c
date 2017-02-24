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
 * parc_X509Certificate.c
 * PARC Library
 */

#include <config.h>

#include <parc/security/parc_X509Certificate.h>
#include <parc/security/parc_Certificate.h>

#include <parc/security/parc_CryptoHash.h>
#include <parc/security/parc_Security.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/pkcs12.h>

static PARCCryptoHash *_getPublicKeyDigest(void *interfaceContext);
static PARCCryptoHash *_getCertificateDigest(void *interfaceContext);
static PARCBuffer *_getDEREncodedCertificate(void *interfaceContext);
static PARCBuffer *_getDEREncodedPublicKey(void *interfaceContext);
static PARCCertificateType _getCertificateType(const void *cert);
static PARCContainerEncoding _getContainerEncoding(const void *cert);

PARCCertificateInterface *PARCX509CertificateInterface = &(PARCCertificateInterface) {
    .GetPublicKeyDigest = _getPublicKeyDigest,
    .GetCertificateDigest = _getCertificateDigest,
    .GetDEREncodedCertificate = _getDEREncodedCertificate,
    .GetDEREncodedPublicKey = _getDEREncodedPublicKey,
    .GetCertificateType = _getCertificateType,
    .GetContainerEncoding = _getContainerEncoding
};

struct parc_X509_certificate {
    PARCCertificateType type;
    PARCContainerEncoding encoding;

    // Cache of results
    PARCCryptoHash *keyDigest;
    PARCCryptoHash *certificateDigest;
    PARCBuffer *derEncodedCertificate;
    PARCBuffer *derEncodedKey;

    BIO *certificateBIO;
    X509 *certificate;
    EVP_PKEY *publicKey;
};

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

static PARCCryptoHash *
_getPublicKeyDigest(void *interfaceContext)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(interfaceContext, "Parameter must be non-null PARCX509Certificate");

    PARCX509Certificate *certificate = (PARCX509Certificate *) interfaceContext;

    if (certificate->keyDigest == NULL) {
        PARCBuffer *derEncodedKey = _getDEREncodedPublicKey(certificate);
        PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
        parcCryptoHasher_Init(hasher);
        parcCryptoHasher_UpdateBuffer(hasher, derEncodedKey);
        PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);

        certificate->keyDigest = parcCryptoHash_Create(PARCCryptoHashType_SHA256, parcCryptoHash_GetDigest(hash));
        parcCryptoHash_Release(&hash);
        parcCryptoHasher_Release(&hasher);
    }

    return certificate->keyDigest;
}

static PARCCryptoHash *
_getCertificateDigest(void *interfaceContext)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(interfaceContext, "Parameter must be non-null PARCX509Certificate");

    PARCX509Certificate *certificate = (PARCX509Certificate *) interfaceContext;

    if (certificate->certificateDigest == NULL) {
        uint8_t digestBuffer[SHA256_DIGEST_LENGTH];
        int result = X509_digest(certificate->certificate, EVP_sha256(), digestBuffer, NULL);
        if (result) {
            PARCBuffer *digest =
                parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(SHA256_DIGEST_LENGTH), SHA256_DIGEST_LENGTH, digestBuffer));
            certificate->certificateDigest = parcCryptoHash_Create(PARCCryptoHashType_SHA256, digest);
            parcBuffer_Release(&digest);
        }
    }

    return certificate->certificateDigest;
}

static PARCBuffer *
_getDEREncodedCertificate(void *interfaceContext)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(interfaceContext, "Parameter must be non-null PARCX509Certificate");

    PARCX509Certificate *certificate = (PARCX509Certificate *) interfaceContext;

    if (certificate->derEncodedCertificate == NULL) {
        uint8_t *der = NULL;

        // this allocates memory for der
        int derLength = i2d_X509(certificate->certificate, &der);
        if (derLength > 0) {
            certificate->derEncodedCertificate =
                parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(derLength), derLength, der));
        }
        OPENSSL_free(der);
    }

    return certificate->derEncodedCertificate;
}

static PARCBuffer *
_getDEREncodedPublicKey(void *interfaceContext)
{
    parcSecurity_AssertIsInitialized();

    assertNotNull(interfaceContext, "Parameter must be non-null PARCX509Certificate");

    PARCX509Certificate *certificate = (PARCX509Certificate *) interfaceContext;

    if (certificate->derEncodedKey == NULL) {
        uint8_t *der = NULL;

        // this allocates memory for der
        int derLength = i2d_PUBKEY(certificate->publicKey, &der);
        if (derLength > 0) {
            certificate->derEncodedKey = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(derLength), derLength, der));
        }
        OPENSSL_free(der);
    }

    return certificate->derEncodedKey;
}

static PARCCertificateType
_getCertificateType(const void *instance)
{
    PARCX509Certificate *certificate = (PARCX509Certificate *) instance;
    return certificate->type;
}

static PARCContainerEncoding
_getContainerEncoding(const void *instance)
{
    PARCX509Certificate *certificate = (PARCX509Certificate *) instance;
    return certificate->encoding;
}

static void
_parcX509Certificate_FinalRelease(PARCX509Certificate **certP)
{
    PARCX509Certificate *cert = (PARCX509Certificate *) *certP;
    if (cert->certificateBIO != NULL) {
        BIO_free_all(cert->certificateBIO);
    }
    if (cert->publicKey != NULL) {
        EVP_PKEY_free(cert->publicKey);
    }
    if (cert->certificate != NULL) {
        X509_free(cert->certificate);
    }
    if (cert->keyDigest != NULL) {
        parcCryptoHash_Release(&cert->keyDigest);
    }
    if (cert->certificateDigest != NULL) {
        parcCryptoHash_Release(&cert->certificateDigest);
    }
    if (cert->derEncodedCertificate != NULL) {
        parcBuffer_Release(&cert->derEncodedCertificate);
    }
    if (cert->derEncodedKey != NULL) {
        parcBuffer_Release(&cert->derEncodedKey);
    }
}

parcObject_ExtendPARCObject(PARCX509Certificate, _parcX509Certificate_FinalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

static PARCX509Certificate *
_createEmptyCertificate()
{
    PARCX509Certificate *cert = parcObject_CreateInstance(PARCX509Certificate);
    cert->certificateBIO = NULL;
    cert->certificate = NULL;
    cert->publicKey = NULL;
    cert->keyDigest = NULL;
    cert->certificateDigest = NULL;
    cert->derEncodedCertificate = NULL;
    cert->derEncodedKey = NULL;
    assertNotNull(cert, "Failure allocating memory for a new PARCX509Certificate instance");

    return cert;
}

static bool
_addCertificateExtensionWithContext(X509 *cert, int nid, char *value)
{
    X509_EXTENSION *extension;
    X509V3_CTX context;

    X509V3_set_ctx_nodb(&context);
    X509V3_set_ctx(&context, cert, cert, NULL, NULL, 0);
    extension = X509V3_EXT_conf_nid(NULL, &context, nid, value);
    if (extension == NULL) {
        return false;
    }
    X509_add_ext(cert, extension, -1);
    X509_EXTENSION_free(extension);
    return true;
}

static bool
_addCertificateExtension(X509 *cert, int nid, char *value)
{
    X509_EXTENSION *extension = X509V3_EXT_conf_nid(NULL, NULL, nid, value);
    if (extension == NULL) {
        return false;
    }
    X509_add_ext(cert, extension, -1);
    X509_EXTENSION_free(extension);
    return true;
}

static bool
_addKeyIdentifier(X509 *cert)
{
    unsigned char spkid[SHA256_DIGEST_LENGTH];
    char spkid_hex[1 + 2 * SHA256_DIGEST_LENGTH];

    /* Generate a KeyID which is the SHA256 digest of the DER encoding
     * of a SubjectPublicKeyInfo.  Note that this is slightly uncommon,
     * but it is more general and complete than digesting the BIT STRING
     * component of the SubjectPublicKeyInfo itself (and no standard dictates
     * how you must generate a key ID).  This code must produce the same result
     * as the Java version applied to the same SubjectPublicKeyInfo.
     */

    if (ASN1_item_digest(ASN1_ITEM_rptr(X509_PUBKEY), EVP_sha256(), X509_get_X509_PUBKEY(cert), spkid, NULL)) {
        for (int i = 0; i < 32; i++) {
            snprintf(&spkid_hex[2 * i], 3, "%02X", (unsigned) spkid[i]);
        }
        if (_addCertificateExtension(cert, NID_subject_key_identifier, spkid_hex) == true) {
            if (_addCertificateExtensionWithContext(cert, NID_authority_key_identifier, "keyid:always") == true) {
                return true;
            }
        }
    }
    return false;
}

static bool
_addSubjectName(X509 *cert, const char *subjectname)
{
    // Set up the simple subject name and issuer name for the certificate.
    X509_NAME *name = X509_get_subject_name(cert);
    assertNotNull(name, "Got null name from X509_get_subject_name");

    if (X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *) subjectname, -1, -1, 0)) {
        if (X509_set_issuer_name(cert, name)) {
            return true;
        }
    }
    return false;
}

static bool
_addRandomSerial(X509 *cert)
{
    unsigned long serial = 0;
    unsigned char serial_bytes[sizeof(serial)];

    // Construct random positive serial number.
    RAND_bytes(serial_bytes, sizeof(serial_bytes));
    serial_bytes[0] &= 0x7F;
    serial = 0;
    for (int i = 0; i < sizeof(serial_bytes); i++) {
        serial = (256 * serial) + serial_bytes[i];
    }
    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);
    return true;
}

static bool
_addValidityPeriod(X509 *cert, size_t validityDays)
{
    // Set the validity from now for the specified number of days.
    X509_gmtime_adj(X509_get_notBefore(cert), (long) 0);
    X509_gmtime_adj(X509_get_notAfter(cert), (long) (60 * 60 * 24 * validityDays));
    return true;
}

static bool
_addExtensions(X509 *cert)
{
    // Add the necessary extensions.
    if (_addCertificateExtension(cert, NID_basic_constraints, "critical,CA:FALSE") == true) {
        if (_addCertificateExtension(cert, NID_key_usage, "digitalSignature,nonRepudiation,keyEncipherment,dataEncipherment,keyAgreement") == true) {
            if (_addCertificateExtension(cert, NID_ext_key_usage, "clientAuth") == true) {
                return true;
            }
        }
    }
    return false;
}

static PARCX509Certificate *
_parcX509Certificate_CreateFromPEMFile(const char *filename)
{
    parcSecurity_AssertIsInitialized();

    PARCX509Certificate *cert = _createEmptyCertificate();

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        parcX509Certificate_Release(&cert);
        return NULL;
    }

    cert->certificateBIO = BIO_new(BIO_s_file());
    size_t result = BIO_read_filename(cert->certificateBIO, filename);
    assertTrue(result == 1, "Unable to open the specified file");

    cert->certificate = PEM_read_bio_X509(cert->certificateBIO, NULL, 0, NULL);
    cert->publicKey = X509_get_pubkey(cert->certificate);

    return cert;
}

parcObject_ImplementAcquire(parcX509Certificate, PARCX509Certificate);

parcObject_ImplementRelease(parcX509Certificate, PARCX509Certificate);

PARCX509Certificate *
parcX509Certificate_CreateFromPEMFile(const char *filename)
{
    PARCX509Certificate *certificate = _parcX509Certificate_CreateFromPEMFile(filename);
    if (certificate) {
        certificate->type = PARCCertificateType_X509;
        certificate->encoding = PARCContainerEncoding_PEM;
        return certificate;
    }
    return NULL;
}

PARCX509Certificate *
parcX509Certificate_CreateFromDERBuffer(const PARCBuffer *buffer)
{
    parcSecurity_AssertIsInitialized();

    PARCX509Certificate *cert = _createEmptyCertificate();
    cert->type = PARCCertificateType_X509;
    cert->encoding = PARCContainerEncoding_DER;

    PARCByteArray *array = parcBuffer_Array(buffer);
    uint8_t *arrayIn = parcByteArray_Array(array);

    cert->certificate = d2i_X509(&cert->certificate, (const unsigned char **) &arrayIn, parcBuffer_Remaining(buffer));
    if (cert->certificate == NULL) {
        parcX509Certificate_Release(&cert);
        return NULL;
    }
    cert->publicKey = X509_get_pubkey(cert->certificate);

    return cert;
}

PARCX509Certificate *
parcX509Certificate_CreateSelfSignedCertificate(PARCBuffer **privateKeyBuffer, char *subjectName, int keyLength, size_t validityDays)
{
    parcSecurity_AssertIsInitialized();

    RSA *rsa = RSA_new();
    assertNotNull(rsa, "RSA_new failed.");

    EVP_PKEY *privateKey = EVP_PKEY_new();
    assertNotNull(privateKey, "EVP_PKEY_new() failed.");

    X509 *cert = X509_new();
    assertNotNull(cert, "X509_new() failed.");

    int res;
    BIGNUM *pub_exp;

    pub_exp = BN_new();

    BN_set_word(pub_exp, RSA_F4);
    res = 1;
    bool result = false;
    if (RSA_generate_key_ex(rsa, keyLength, pub_exp, NULL)) {
        if (EVP_PKEY_set1_RSA(privateKey, rsa)) {
            if (X509_set_version(cert, 2)) { // 2 => X509v3
                result = true;
            }
        }
    }
    if (result) {
        // add serial number
        if (_addRandomSerial(cert) == true) {
            if (_addValidityPeriod(cert, validityDays) == true) {
                if (X509_set_pubkey(cert, privateKey) == 1) {
                    if (_addSubjectName(cert, subjectName) == true) {
                        if (_addExtensions(cert) == true) {
                            if (_addKeyIdentifier(cert) == true) {
                                // The certificate is complete, sign it.
                                if (X509_sign(cert, privateKey, EVP_sha256())) {
                                    result = true;
                                } else {
                                    printf("error: (%d) %s\n", res, ERR_lib_error_string(res));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ERR_print_errors_fp(stdout);

    BN_free(pub_exp);

    uint8_t *certificateDerEncoding = NULL;
    int numBytes = i2d_X509(cert, &certificateDerEncoding);
    if (numBytes < 0) {
        EVP_PKEY_free(privateKey);
        RSA_free(rsa);
        X509_free(cert);

        return NULL;
    }

    PARCBuffer *derBuffer = parcBuffer_Allocate(numBytes);
    parcBuffer_Flip(parcBuffer_PutArray(derBuffer, numBytes, certificateDerEncoding));

    PARCX509Certificate *certificate = parcX509Certificate_CreateFromDERBuffer(derBuffer);
    parcBuffer_Release(&derBuffer);

    uint8_t *privateKeyBytes = NULL;
    int privateKeyByteCount = i2d_PrivateKey(privateKey, &privateKeyBytes);
    if (privateKeyByteCount < 0) {
        EVP_PKEY_free(privateKey);
        RSA_free(rsa);
        X509_free(cert);

        return NULL;
    }

    *privateKeyBuffer = parcBuffer_Allocate(privateKeyByteCount);
    parcBuffer_Flip(parcBuffer_PutArray(*privateKeyBuffer, privateKeyByteCount, privateKeyBytes));

    return certificate;
}


PARCCryptoHash *
parcX509Certificate_GetCertificateDigest(PARCX509Certificate *certificate)
{
    return _getCertificateDigest(certificate);
}

PARCCryptoHash *
parcX509Certificate_getPublicKeyDigest(PARCX509Certificate *certificate)
{
    return _getPublicKeyDigest(certificate);
}

PARCBuffer *
parcX509Certificate_GetDEREncodedCertificate(PARCX509Certificate *certificate)
{
    return _getDEREncodedCertificate(certificate);
}

PARCBuffer *
parcX509Certificate_GetDEREncodedPublicKey(PARCX509Certificate *certificate)
{
    return _getDEREncodedPublicKey(certificate);
}

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

