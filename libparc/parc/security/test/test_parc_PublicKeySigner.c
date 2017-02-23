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
#include "../parc_PublicKeySigner.c"
#include <sys/param.h>

#include <fcntl.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

#include <parc/security/parc_Pkcs12KeyStore.h>

LONGBOW_TEST_RUNNER(parc_PublicKeySigner)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_PublicKeySigner)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_PublicKeySigner)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    parcSecurity_Fini();
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *instance = parcPublicKeySigner_Create(keyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
    parcKeyStore_Release(&keyStore);
    assertNotNull(instance, "Expected non-null result from parcPublicKeySigner_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcPublicKeySigner_Acquire, instance);

    parcPublicKeySigner_Release(&instance);
    assertNull(instance, "Expected null result from parcPublicKeySigner_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcPublicKeySigner_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcPublicKeySigner_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcPublicKeySigner_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcPublicKeySigner_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Object)
{
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Object)
{
    parcSecurity_Fini();
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

static PARCPublicKeySigner *
_createSigner(char *path)
{
    char dirname[] = "/tmp/pubkeystore_XXXXXX";
    char filename[MAXPATHLEN];

    char *temporaryDirectory = mkdtemp(dirname);
    assertNotNull(temporaryDirectory, "tmp_dirname should not be null");
    sprintf(filename, "%s/%s", temporaryDirectory, path);

    parcPkcs12KeyStore_CreateFile(filename, "blueberry", "person", 1024, 365);
    PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open(filename, "blueberry", PARCCryptoHashType_SHA256);
    PARCKeyStore *publicKeyStore = parcKeyStore_Create(keyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&keyStore);
    PARCPublicKeySigner *pksigner = parcPublicKeySigner_Create(publicKeyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
    parcKeyStore_Release(&publicKeyStore);

    return pksigner;
}

LONGBOW_TEST_CASE(Object, parcPublicKeySigner_Equals)
{
    PARCPublicKeySigner *x = _createSigner("bananasA");
    PARCPublicKeySigner *y = _createSigner("bananasB");
    PARCPublicKeySigner *z = _createSigner("bananasC");

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcPublicKeySigner_Release(&x);
    parcPublicKeySigner_Release(&y);
    parcPublicKeySigner_Release(&z);
}

LONGBOW_TEST_CASE(Object, parcPublicKeySigner_HashCode)
{
    PARCPublicKeySigner *x = _createSigner("bananasX");
    PARCPublicKeySigner *y = _createSigner("bananasY");

    parcObjectTesting_AssertHashCode(x, y);

    parcPublicKeySigner_Release(&x);
    parcPublicKeySigner_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcPublicKeySigner_IsValid)
{
    PARCPublicKeySigner *instance = _createSigner("bananas");
    assertTrue(parcPublicKeySigner_IsValid(instance), "Expected parcPublicKeySigner_Create to result in a valid instance.");

    parcPublicKeySigner_Release(&instance);
    assertFalse(parcPublicKeySigner_IsValid(instance), "Expected parcPublicKeySigner_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcPublicKeySigner_ToString)
{
    PARCPublicKeySigner *instance = _createSigner("bananas");

    char *string = parcPublicKeySigner_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcPublicKeySigner_ToString");

    parcMemory_Deallocate((void **) &string);
    parcPublicKeySigner_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, parcPkcs12KeyStore_VerifySignature_Cert);
    LONGBOW_RUN_TEST_CASE(Specialization, parcPkcs12KeyStore_SignBuffer);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    parcSecurity_Fini();
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Specialization, parcPkcs12KeyStore_VerifySignature_Cert)
{
}

/**
 * Sign the file "test_rsa_pub_sha256.bin" using the test_rsa.p12 private key.
 */
LONGBOW_TEST_CASE(Specialization, parcPkcs12KeyStore_SignBuffer)
{
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256);
    assertNotNull(publicKeyStore, "Got null result from opening openssl pkcs12 file");
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *publicKeySigner = parcPublicKeySigner_Create(keyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
    parcKeyStore_Release(&keyStore);
    PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&publicKeySigner);

    assertNotNull(signer, "Got null result from opening openssl pkcs12 file");

    // read the buffer to sign
    int fd = open("test_random_bytes", O_RDONLY);
    assertTrue(fd != -1, "Cannot open test_random_bytes file.");
    uint8_t buffer_to_sign[2048];
    ssize_t read_bytes = read(fd, buffer_to_sign, 2048);
    close(fd);

    // Digest it
    PARCCryptoHasher *digester = parcSigner_GetCryptoHasher(signer);
    parcCryptoHasher_Init(digester);
    parcCryptoHasher_UpdateBytes(digester, buffer_to_sign, read_bytes);
    PARCCryptoHash *parcDigest = parcCryptoHasher_Finalize(digester);

    PARCSignature *bb_test_sign = parcSigner_SignDigest(signer, parcDigest);

    assertNotNull(bb_test_sign, "Got null byte buffer from SignBuffer");
    assertTrue(parcBuffer_Remaining(parcSignature_GetSignature(bb_test_sign)) == 128,
               "Incorrect signature size: %zu",
               parcBuffer_Position(parcSignature_GetSignature(bb_test_sign)));

    // now read the "true" signature
    uint8_t scratch_buffer[1024];
    fd = open("test_random_bytes.sig", O_RDONLY);
    assertTrue(fd != -1, "Cannot open test_random_bytes.sig file.");
    read_bytes = read(fd, scratch_buffer, 1024);
    assertTrue(read_bytes == 128, "read incorrect size signature from disk: %zu", read_bytes);
    close(fd);

    const unsigned char *actual = parcByteArray_Array(parcBuffer_Array(parcSignature_GetSignature(bb_test_sign)));

    assertTrue(memcmp(scratch_buffer, actual, read_bytes) == 0,
               "signatures did not match");

    parcSigner_Release(&signer);
    parcSignature_Release(&bb_test_sign);
    parcCryptoHash_Release(&parcDigest);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetCertificateDigest)
{
    char dirname[] = "pubkeystore_XXXXXX";
    char filename[MAXPATHLEN];
    const char *password = "flumox";
    unsigned key_bits = 1024;
    unsigned valid_days = 30;

    const char to_sign[] = "it was a dark and stormy night, and all through the house not a digest was creeping";

    char *tmp_dirname = mkdtemp(dirname);
    assertNotNull(tmp_dirname, "tmp_dirname should not be null");
    sprintf(filename, "%s/pubkeystore.p12", tmp_dirname);

    // create the file
    parcPkcs12KeyStore_CreateFile(filename, password, "alice", key_bits, valid_days);

    // open it as an RSA provider for the signer
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *publicKeySigner = parcPublicKeySigner_Create(keyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
    PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&publicKeySigner);

    PARCCryptoHasher *hasher = parcSigner_GetCryptoHasher(signer);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, to_sign, sizeof(to_sign));
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    PARCSignature *sig = parcSigner_SignDigest(signer, hash);

    unlink(filename);
    int rc = rmdir(tmp_dirname);
    assertTrue(rc == 0, "directory cleanup failed");

    char *s = parcSignature_ToString(sig);
    printf("Signature: %s\n", s);
    parcMemory_Deallocate((void **) &s);

    PARCCryptoHash *certDigest = parcKeyStore_GetCertificateDigest(parcSigner_GetKeyStore(signer));
    assertNotNull(certDigest, "Expected a non NULL value");
    parcCryptoHash_Release(&certDigest);

    parcKeyStore_Release(&keyStore);
    parcCryptoHash_Release(&hash);
    parcSignature_Release(&sig);
    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetDEREncodedCertificate)
{
    char dirname[] = "pubkeystore_XXXXXX";
    char filename[MAXPATHLEN];
    const char *password = "flumox";
    unsigned key_bits = 1024;
    unsigned valid_days = 30;

    const char to_sign[] = "it was a dark and stormy night, and all through the house not a digest was creeping";

    char *tmp_dirname = mkdtemp(dirname);
    assertNotNull(tmp_dirname, "tmp_dirname should not be null");
    sprintf(filename, "%s/pubkeystore.p12", tmp_dirname);

    // create the file
    parcPkcs12KeyStore_CreateFile(filename, password, "alice", key_bits, valid_days);

    // open it as an RSA provider for the signer
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *publicKeySigner = parcPublicKeySigner_Create(keyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
    PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&publicKeySigner);

    PARCCryptoHasher *hasher = parcSigner_GetCryptoHasher(signer);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, to_sign, sizeof(to_sign));
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    PARCSignature *sig = parcSigner_SignDigest(signer, hash);

    unlink(filename);
    int rc = rmdir(tmp_dirname);
    assertTrue(rc == 0, "directory cleanup failed");

    char *s = parcSignature_ToString(sig);
    printf("Signature: %s\n", s);
    parcMemory_Deallocate((void **) &s);

    PARCBuffer *certificate_der = parcKeyStore_GetDEREncodedCertificate(parcSigner_GetKeyStore(signer));
    assertNotNull(certificate_der, "Expected a non NULL value");
    parcBuffer_Release(&certificate_der);

    parcKeyStore_Release(&keyStore);
    parcCryptoHash_Release(&hash);
    parcSignature_Release(&sig);
    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_CreatePublicKey)
{
    char dirname[] = "pubkeystore_XXXXXX";
    char filename[MAXPATHLEN];
    const char *password = "flumox";
    unsigned key_bits = 1024;
    unsigned valid_days = 30;

    char *tmp_dirname = mkdtemp(dirname);
    assertNotNull(tmp_dirname, "tmp_dirname should not be null");
    sprintf(filename, "%s/pubkeystore.p12", tmp_dirname);

    // create the file
    parcPkcs12KeyStore_CreateFile(filename, password, "alice", key_bits, valid_days);

    // open it as an RSA provider for the signer
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *publicKeySigner = parcPublicKeySigner_Create(keyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
    PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&publicKeySigner);

    PARCKey *key = parcSigner_CreatePublicKey(signer);
    assertNotNull(key, "Expected a non NULL value");
    parcKey_Release(&key);
    parcKeyStore_Release(&keyStore);

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_CreateKeyId)
{
    char dirname[] = "pubkeystore_XXXXXX";
    char filename[MAXPATHLEN];
    const char *password = "flumox";
    unsigned key_bits = 1024;
    unsigned valid_days = 30;

    const char to_sign[] = "it was a dark and stormy night, and all through the house not a digest was creeping";

    char *tmp_dirname = mkdtemp(dirname);
    assertNotNull(tmp_dirname, "tmp_dirname should not be null");
    sprintf(filename, "%s/pubkeystore.p12", tmp_dirname);

    // create the file
    parcPkcs12KeyStore_CreateFile(filename, password, "alice", key_bits, valid_days);

    // open it as an RSA provider for the signer
    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open(filename, password, PARCCryptoHashType_SHA256);
    PARCKeyStore *keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&publicKeyStore);

    PARCPublicKeySigner *publicKeySigner = parcPublicKeySigner_Create(keyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
    PARCSigner *signer = parcSigner_Create(publicKeySigner, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&publicKeySigner);

    PARCCryptoHasher *hasher = parcSigner_GetCryptoHasher(signer);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, to_sign, sizeof(to_sign));
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    PARCSignature *sig = parcSigner_SignDigest(signer, hash);

    unlink(filename);
    int rc = rmdir(tmp_dirname);
    assertTrue(rc == 0, "directory cleanup failed");

    char *s = parcSignature_ToString(sig);
    printf("Signature: %s\n", s);
    parcMemory_Deallocate((void **) &s);

    PARCKeyId *keyId = parcSigner_CreateKeyId(signer);
    assertNotNull(keyId, "Expected a non NULL value");
    parcKeyId_Release(&keyId);

    parcKeyStore_Release(&keyStore);
    parcCryptoHash_Release(&hash);
    parcSignature_Release(&sig);
    parcSigner_Release(&signer);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_PublicKeySigner);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
