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

#include <LongBow/testing.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Pkcs12KeyStore.c"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>
#include <parc/security/parc_PublicKeySigner.h>

const char *filename = "/tmp/filekeystore.p12";

LONGBOW_TEST_RUNNER(ccnx_FileKeystore)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(openssl_commandline);
    LONGBOW_RUN_TEST_FIXTURE(ccnx_internal);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_FileKeystore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_FileKeystore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcPkcs12KeyStore_Open);
    LONGBOW_RUN_TEST_CASE(Global, parcPkcs12KeyStore_badpass);
    LONGBOW_RUN_TEST_CASE(Global, parcPkcs12KeyStore_CreateAndOpen);
    LONGBOW_RUN_TEST_CASE(Global, parcPkcs12KeyStore_CreateFile_Fail);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    unlink(filename);
    parcSecurity_Fini();
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcPkcs12KeyStore_Open)
{
    // open our test p12 file created with openssl
    parcSecurity_Init();

    PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);

    assertNotNull(keyStore, "Got null result from opening openssl pkcs12 file");

    parcPkcs12KeyStore_Release(&keyStore);
    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(Global, parcPkcs12KeyStore_badpass)
{
    // open our test p12 file created with openssl

    fprintf(stderr, "The next openssl error is expected, we're using the wrong password\n");
    PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "orange", PARCCryptoHashType_SHA256);

    assertNull(keyStore, "Got null result from opening openssl pkcs12 file");
}

LONGBOW_TEST_CASE(Global, parcPkcs12KeyStore_CreateAndOpen)
{
    // create a file and open it
    const char *filename = "/tmp/parcPkcs12KeyStore_CreateAndOpen.p12";
    const char *password = "12345";
    const char *subject = "alice";
    bool result;

    result = parcPkcs12KeyStore_CreateFile(filename, password, subject, PARCSigningAlgorithm_RSA, 1024, 32);
    assertTrue(result, "got error from parcPkcs12KeyStore_CreatePkcs12File");

    PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);

    assertNotNull(keyStore, "Got null result from opening openssl pkcs12 file");

    parcPkcs12KeyStore_Release(&keyStore);
    unlink(filename);
}

LONGBOW_TEST_CASE(Global, parcPkcs12KeyStore_CreateFile_Fail)
{
    // create a file and open it
    const char *filename = "/tmp/parcPkcs12KeyStore_CreateAndOpen.p12";
    const char *password = "12345";
    const char *subject = "alice";
    bool result;

    result = parcPkcs12KeyStore_CreateFile(filename, password, subject, PARCSigningAlgorithm_RSA, -1, 32);
    assertFalse(result, "Expected false result from parcPkcs12KeyStore_CreateFile()");

    unlink(filename);
}


// =====================================================
// These are tests based on internally-generated pkcs12

LONGBOW_TEST_FIXTURE(ccnx_internal)
{
    LONGBOW_RUN_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetCertificateDigest);
    LONGBOW_RUN_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetPublicKeyDigest);
    LONGBOW_RUN_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetEncodedCertificate);
    LONGBOW_RUN_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetEncodedPublicKey);
}

LONGBOW_TEST_FIXTURE_SETUP(ccnx_internal)
{
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(ccnx_internal)
{
    unlink(filename);
    parcSecurity_Fini();
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetCertificateDigest)
{
    // create a file and open it
    const char *password = "12345";
    const char *subject = "alice";
    bool result;

    result = parcPkcs12KeyStore_CreateFile(filename, password, subject, PARCSigningAlgorithm_RSA, 1024, 32);
    assertTrue(result, "got error from parcPkcs12KeyStore_CreatePkcs12File");

    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    assertNotNull(publicKeyStore, "Got null result from opening openssl pkcs12 file");
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCCryptoHash *cert_digest = parcKeyStore_GetCertificateDigest(keyStore);
    assertNotNull(cert_digest, "got null public key digest for external pkcs12");

    size_t bb_length = parcBuffer_Remaining(parcCryptoHash_GetDigest(cert_digest));
    assertTrue(bb_length == SHA256_DIGEST_LENGTH,
               "Incorrect digest length returned from GetPublicKeyDigest: %zu", bb_length);

    parcKeyStore_Release(&keyStore);
    parcCryptoHash_Release(&cert_digest);
}

/**
 * Use a ccnx-generated pkcs12 file
 */
LONGBOW_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetPublicKeyDigest)
{
    // create a file and open it
    const char *password = "12345";
    const char *subject = "alice";
    bool result;

    result = parcPkcs12KeyStore_CreateFile(filename, password, subject, PARCSigningAlgorithm_RSA, 1024, 32);
    assertTrue(result, "got error from parcPkcs12KeyStore_CreatePkcs12File");

    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    assertNotNull(publicKeyStore, "Got null result from opening openssl pkcs12 file");
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCCryptoHash *pkd = parcKeyStore_GetVerifierKeyDigest(keyStore);
    assertNotNull(pkd, "got null public key digest for external pkcs12");

    size_t bb_length = parcBuffer_Remaining(parcCryptoHash_GetDigest(pkd));
    assertTrue(bb_length == SHA256_DIGEST_LENGTH, "Incorrect digest length returned from GetPublicKeyDigest: %zu", bb_length);

    parcKeyStore_Release(&keyStore);
    parcCryptoHash_Release(&pkd);
}


LONGBOW_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetEncodedCertificate)
{
    // create a file and open it
    const char *password = "12345";
    const char *subject = "alice";
    bool result;

    result = parcPkcs12KeyStore_CreateFile(filename, password, subject, PARCSigningAlgorithm_RSA, 1024, 32);
    assertTrue(result, "got error from parcPkcs12KeyStore_CreatePkcs12File");

    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    assertNotNull(publicKeyStore, "Got null result from opening openssl pkcs12 file");
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCBuffer *certificate_der = parcKeyStore_GetDEREncodedCertificate(keyStore);
    assertNotNull(certificate_der, "got null public key digest for external pkcs12");

    // 557 (64-bit) and 553 (32-bit) are pre-etermined sizes of how big a DER encoded
    // certificate with a 1024-bit key should be
    size_t expectedMinimumLength = 545;
    size_t expectedMaximumLength = 560;
    size_t bb_length = parcBuffer_Remaining(certificate_der);
    assertTrue(expectedMinimumLength <= bb_length && bb_length <= expectedMaximumLength,
               "Digest unexpected size: got %zu expected %zu - %zu", bb_length, expectedMinimumLength, expectedMaximumLength);

    parcKeyStore_Release(&keyStore);
    parcBuffer_Release(&certificate_der);
}

LONGBOW_TEST_CASE(ccnx_internal, parcPkcs12KeyStore_GetEncodedPublicKey)
{
    // create a file and open it
    const char *password = "12345";
    const char *subject = "alice";
    bool result;

    result = parcPkcs12KeyStore_CreateFile(filename, password, subject, PARCSigningAlgorithm_RSA, 1024, 32);
    assertTrue(result, "got error from parcPkcs12KeyStore_CreatePkcs12File");

    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    assertNotNull(publicKeyStore, "Got null result from opening openssl pkcs12 file");
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCBuffer *pubkey_der = parcKeyStore_GetDEREncodedPublicKey(keyStore);
    assertNotNull(pubkey_der, "got null public key digest for external pkcs12");

    size_t bb_length = parcBuffer_Remaining(pubkey_der);
    assertTrue(bb_length == 162, "Incorrect digest length returned from GetPublicKeyDigest: %zu", bb_length);

    parcKeyStore_Release(&keyStore);
    parcBuffer_Release(&pubkey_der);
}

// =====================================================
// These are tests based on pre-generated material from the openssl command line

LONGBOW_TEST_FIXTURE(openssl_commandline)
{
    LONGBOW_RUN_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetCertificateDigest);
    LONGBOW_RUN_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetPublicKeyDigest);
    LONGBOW_RUN_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetEncodedCertificate);
    LONGBOW_RUN_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetEncodedPublicKey);
}

LONGBOW_TEST_FIXTURE_SETUP(openssl_commandline)
{
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(openssl_commandline)
{
    unlink(filename);
    parcSecurity_Fini();
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * read in the openssl command-line generated pkcs12 file
 */
LONGBOW_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetPublicKeyDigest)
{
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *PublicKeySigner = parcPublicKeySigner_Create(keyStore, PARCCryptoSuite_RSA_SHA256);
    parcKeyStore_Release(&keyStore);
    PARCSigner *signer = parcSigner_Create(PublicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&PublicKeySigner);

    assertNotNull(signer, "parcPkcs12KeyStore_Open(\"test_rsa.p12\", \"blueberry\", PARCCryptoHashType_SHA256) returned NULL");

    PARCCryptoHash *pkd = parcKeyStore_GetVerifierKeyDigest(parcSigner_GetKeyStore(signer));
    assertNotNull(pkd, "got null public key digest for external pkcs12");

    // read in the "truth" from the command line utilities

    int fd = open("test_rsa_pub_sha256.bin", O_RDONLY);
    uint8_t true_digest[SHA256_DIGEST_LENGTH];
    ssize_t read_bytes = read(fd, true_digest, SHA256_DIGEST_LENGTH);
    close(fd);

    assertTrue(read_bytes == SHA256_DIGEST_LENGTH, "could not read %d byte digest from test_rsa_pub_sha256.bin", SHA256_DIGEST_LENGTH);

    PARCBuffer *digest = parcCryptoHash_GetDigest(pkd);
    const uint8_t *bb_buffer = parcByteArray_Array(parcBuffer_Array(digest));
    size_t bb_length = parcBuffer_Remaining(digest);
    assertTrue(bb_length == SHA256_DIGEST_LENGTH,
               "Incorrect digest length returned from GetPublicKeyDigest: %zu", bb_length);

    assertTrue(memcmp(bb_buffer, true_digest, SHA256_DIGEST_LENGTH) == 0, "digests did not match");


    parcSigner_Release(&signer);
    parcCryptoHash_Release(&pkd);
}

/**
 * Get the certificate digest from the openssl command line pkcs12
 */
LONGBOW_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetCertificateDigest)
{
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *PublicKeySigner = parcPublicKeySigner_Create(keyStore, PARCCryptoSuite_RSA_SHA256);
    parcKeyStore_Release(&keyStore);
    PARCSigner *signer = parcSigner_Create(PublicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&PublicKeySigner);

    assertNotNull(signer, "Got null result from opening openssl pkcs12 file");

    PARCCryptoHash *cert_digest = parcKeyStore_GetCertificateDigest(parcSigner_GetKeyStore(signer));
    assertNotNull(cert_digest, "got null public key digest for external pkcs12");

    // read in the "truth" from the command line utilities

    int fd = open("test_rsa_crt_sha256.bin", O_RDONLY);
    uint8_t true_digest[SHA256_DIGEST_LENGTH];
    ssize_t read_bytes = read(fd, true_digest, SHA256_DIGEST_LENGTH);
    close(fd);

    assertTrue(read_bytes == SHA256_DIGEST_LENGTH, "could not read %d byte digest from test_rsa_pub_sha256.bin", SHA256_DIGEST_LENGTH);

    const uint8_t *bb_buffer = parcByteArray_Array(parcBuffer_Array(parcCryptoHash_GetDigest(cert_digest)));
    size_t bb_length = parcBuffer_Remaining(parcCryptoHash_GetDigest(cert_digest));
    assertTrue(bb_length == SHA256_DIGEST_LENGTH,
               "Incorrect digest length returned from GetCertificateDigest: %zu", bb_length);

    assertTrue(memcmp(bb_buffer, true_digest, SHA256_DIGEST_LENGTH) == 0, "digests did not match");

    parcSigner_Release(&signer);
    parcCryptoHash_Release(&cert_digest);
}

LONGBOW_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetEncodedCertificate)
{
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *PublicKeySigner = parcPublicKeySigner_Create(keyStore, PARCCryptoSuite_RSA_SHA256);
    parcKeyStore_Release(&keyStore);
    PARCSigner *signer = parcSigner_Create(PublicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&PublicKeySigner);

    assertNotNull(signer, "Got null result from opening openssl pkcs12 file");

    PARCBuffer *certificate_der = parcKeyStore_GetDEREncodedCertificate(parcSigner_GetKeyStore(signer));
    assertNotNull(certificate_der, "got null der certificate for external pkcs12");

    // read in the "truth" from the command line utilities

    int fd = open("test_rsa_crt.der", O_RDONLY);
    uint8_t true_der[1024];
    ssize_t read_bytes = read(fd, true_der, 1024);
    close(fd);

    assertTrue(read_bytes == 517,
               "could not read %d byte digest from test_rsa_pub_sha256.bin", 517);

    const uint8_t *bb_buffer = parcByteArray_Array(parcBuffer_Array(certificate_der));
    size_t bb_length = parcBuffer_Remaining(certificate_der);
    assertTrue(bb_length == read_bytes,
               "Incorrect digest length returned from GetCertificateDigest: %zu", bb_length);

    assertTrue(memcmp(bb_buffer, true_der, read_bytes) == 0, "digests did not match");

    parcSigner_Release(&signer);
    parcBuffer_Release(&certificate_der);
}

/**
 * Gets the DER encoded public key
 */
LONGBOW_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_GetEncodedPublicKey)
{
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *PublicKeySigner = parcPublicKeySigner_Create(keyStore, PARCCryptoSuite_RSA_SHA256);
    parcKeyStore_Release(&keyStore);
    PARCSigner *signer = parcSigner_Create(PublicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&PublicKeySigner);

    assertNotNull(signer, "Got null result from opening openssl pkcs12 file");

    PARCBuffer *pubkey_der = parcKeyStore_GetDEREncodedPublicKey(parcSigner_GetKeyStore(signer));
    assertNotNull(pubkey_der, "got null public key der for external pkcs12");

    // read in the "truth" from the command line utilities

    int fd = open("test_rsa_pub.der", O_RDONLY);
    uint8_t true_der[1024];
    ssize_t read_bytes = read(fd, true_der, 1024);
    close(fd);

    assertTrue(read_bytes == 162, "could not read %d byte digest from test_rsa_pub_sha256.bin", 162);

    const uint8_t *bb_buffer = parcByteArray_Array(parcBuffer_Array(pubkey_der));
    size_t bb_length = parcBuffer_Remaining(pubkey_der);
    assertTrue(bb_length == read_bytes, "Incorrect digest length returned from GetCertificateDigest: %zu", bb_length);
    assertTrue(memcmp(bb_buffer, true_der, read_bytes) == 0, "digests did not match");

    parcSigner_Release(&signer);
    parcBuffer_Release(&pubkey_der);
}

LONGBOW_TEST_CASE(openssl_commandline, parcPkcs12KeyStore_VerifySignature_Cert)
{
    testUnimplemented("Not Implemented");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_FileKeystore);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
