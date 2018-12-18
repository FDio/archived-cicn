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

#include <config.h>
#include <LongBow/unit-test.h>
#include <sys/param.h>
#include <errno.h>

#include "../parc_Signer.c"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>
#include <parc/testing/parc_ObjectTesting.h>

#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_KeyStore.h>
#include <parc/security/parc_PublicKeySigner.h>
#include <parc/security/parc_SymmetricKeySigner.h>

#define FAKE_SIGNATURE "signature"

typedef struct {
    PARCCryptoHasher *hasher;
    PARCKeyStore *keyStore;
} _MockSigner;

static PARCSignature *
_SignDigest(PARCSigner *interfaceContext)
{
    PARCBuffer *buffer = parcBuffer_WrapCString(FAKE_SIGNATURE);
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, buffer);
    parcBuffer_Release(&buffer);
    return signature;
}

static PARCSigningAlgorithm
_GetSigningAlgorithm(PARCSigner *interfaceContext)
{
    return PARCSigningAlgorithm_ECDSA;
}

static PARCCryptoHashType
_GetCryptoHashType(PARCSigner  *signer)
{
    return PARCCryptoHashType_SHA256;
}

static PARCCryptoHasher *
_GetCryptoHasher(_MockSigner  *signer)
{
    return signer->hasher;
}

static PARCKeyStore *
_GetKeyStore(_MockSigner *signer)
{
    return signer->keyStore;
}

static bool
_releaseSigner(_MockSigner **signer)
{
    parcCryptoHasher_Release(&((*signer)->hasher));
    parcKeyStore_Release(&((*signer)->keyStore));
    return true;
}

parcObject_ImplementAcquire(_mockSigner, _MockSigner);
parcObject_ImplementRelease(_mockSigner, _MockSigner);

parcObject_Override(_MockSigner, PARCObject,
                    .destructor = (PARCObjectDestructor *) _releaseSigner);

static _MockSigner *
_createSigner()
{
  const char *filename = "/tmp/test_ecdsa.p12";
  const char *password = "12345";
  const char *subject = "alice";
  _MockSigner *signer = parcObject_CreateInstance(_MockSigner);

  signer->hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);

  bool res = parcPkcs12KeyStore_CreateFile(filename, password, subject, PARCSigningAlgorithm_ECDSA, 256, 180);
  assertTrue(res, "Unable to create an ECDSA key");
  
  PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_ec.p12", "blueberry", PARCCryptoHashType_SHA256);
      
  assertNotNull(publicKeyStore, "Got null result from opening openssl pkcs12 file");

  signer->keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);
  parcPkcs12KeyStore_Release(&publicKeyStore);
  
  return signer;
}

static PARCSigningInterface *_MockSignerInterface = &(PARCSigningInterface) {
    .GetCryptoHasher = (PARCCryptoHasher * (*)(void *))_GetCryptoHasher,
    .SignDigest = (PARCSignature * (*)(void *, const PARCCryptoHash *))_SignDigest,
    .GetSigningAlgorithm = (PARCSigningAlgorithm (*)(void *))_GetSigningAlgorithm,
    .GetCryptoHashType = (PARCCryptoHashType (*)(void *))_GetCryptoHashType,
    .GetKeyStore = (PARCKeyStore * (*)(void *))_GetKeyStore,
};

LONGBOW_TEST_RUNNER(parc_Signer)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Signer)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Signer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_CreateKeyId);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_CreatePublicKey);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetCryptoHasher);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_SignDigest);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetSigningAlgorithm);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetCryptoHashType);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetKeyStore);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    parcSecurity_Fini();
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcSigner_Create)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    assertNotNull(signer, "Expected non-null signer");

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_AcquireRelease)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    assertNotNull(signer, "Expected non-null signer");

    parcObjectTesting_AssertAcquireReleaseContract(parcSigner_Acquire, signer);

    parcSigner_Release(&signer);
    assertNull(signer, "Expected null result from parcSigner_Release();");
}

LONGBOW_TEST_CASE(Global, parcSigner_CreateKeyId)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    PARCKeyId *keyId = parcSigner_CreateKeyId(signer);

    assertNotNull(keyId, "Expected non-NULL PARCKeyId");

    parcKeyId_Release(&keyId);
    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_CreatePublicKey)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCKey *key = parcSigner_CreatePublicKey(signer);

    // Compute the real value
    PARCCryptoHash *hash = parcKeyStore_GetVerifierKeyDigest(mock->keyStore);
    PARCKeyId *keyid = parcKeyId_Create(parcCryptoHash_GetDigest(hash));
    PARCBuffer *derEncodedKey = parcKeyStore_GetDEREncodedPublicKey(mock->keyStore);

    PARCKey *expectedKey = parcKey_CreateFromDerEncodedPublicKey(keyid,
                                                                 parcSigner_GetSigningAlgorithm(signer),
                                                                 derEncodedKey);

    parcBuffer_Release(&derEncodedKey);
    parcKeyId_Release(&keyid);

    parcCryptoHash_Release(&hash);

    assertTrue(parcKey_Equals(key, expectedKey), "Expected public keys to be computed equally.");

    parcKey_Release(&key);
    parcKey_Release(&expectedKey);
    parcSigner_Release(&signer);
    _mockSigner_Release(&mock);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetCryptoHasher)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    PARCCryptoHasher *hasher = parcSigner_GetCryptoHasher(signer);

    assertNotNull(hasher, "Expected non-NULL PARCCryptoHasher");

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_SignDigest)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
    PARCSignature *signature = parcSigner_SignDigest(signer, hash);

    assertNotNull(signature, "Expected non-NULL PARCSignature");

    PARCBuffer *bits = parcSignature_GetSignature(signature);
    char *bitstring = parcBuffer_ToString(bits);
    char *expectedString = FAKE_SIGNATURE;
    assertTrue(strcmp(bitstring, expectedString) == 0, "Expected the forced signature as output %s, got %s", expectedString, bitstring);
    parcMemory_Deallocate(&bitstring);

    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&buffer);
    parcSignature_Release(&signature);
    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetSigningAlgorithm)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    PARCSigningAlgorithm alg = parcSigner_GetSigningAlgorithm(signer);
    assertTrue(PARCSigningAlgorithm_ECDSA == alg, "Expected PARCSigningAlgorithm_ECDSA algorithm, got %d", alg);

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetCryptoHashType)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    PARCCryptoHashType type = parcSigner_GetCryptoHashType(signer);
    assertTrue(PARCCryptoHashType_SHA256 == type, "Expected PARCCryptoHashType_SHA256 algorithm, got %d", type);

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetKeyStore)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);
    _mockSigner_Release(&mock);

    PARCKeyStore *keyStore = parcSigner_GetKeyStore(signer);
    assertNotNull(keyStore, "Expected non-NULL PARCKeyStore");

    parcSigner_Release(&signer);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Signer);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
