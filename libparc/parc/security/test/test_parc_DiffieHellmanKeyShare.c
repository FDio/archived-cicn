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
#include <stdio.h>
#include <LongBow/unit-test.h>
#include <parc/testing/parc_ObjectTesting.h>

#include "../parc_DiffieHellmanKeyShare.c"
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/algol/parc_Buffer.h>

LONGBOW_TEST_RUNNER(parc_DiffieHellmanKeyShare)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(parc_DiffieHellmanKeyShare)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_DiffieHellmanKeyShare)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_SerializePublicKey);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_SerializeDeserializePublicKey);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_DeserializePublicKey_ErrorWrongGroup);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_DeserializePublicKey_ErrorInvalidEncoding);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_Combine);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellmanKeyShare_Combine_Error_PublicKeyDeserializationFail);
    LONGBOW_RUN_TEST_CASE(Global, _parcDiffieHellmanKeyShare_HashSharedSecret);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_AcquireRelease)
{
    PARCDiffieHellmanKeyShare *dh = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    parcObjectTesting_AssertAcquireReleaseContract(parcDiffieHellmanKeyShare_Acquire, dh);
    parcDiffieHellmanKeyShare_Release(&dh);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_Create)
{
    PARCDiffieHellmanKeyShare *dh = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(dh, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");
    parcDiffieHellmanKeyShare_Release(&dh);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_SerializePublicKey)
{
    PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(keyShare, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");

    PARCBuffer *publicKey = parcDiffieHellmanKeyShare_SerializePublicKey(keyShare);
    assertNotNull(publicKey, "Expected the public key to be serialized to a non-NULL PARCBuffer");

    const size_t sec521r1KeySize = 266;
    assertTrue(parcBuffer_Remaining(publicKey) == sec521r1KeySize, "Expected the public key size to be %zu, got %zu", sec521r1KeySize, parcBuffer_Remaining(publicKey));

    parcBuffer_Release(&publicKey);
    parcDiffieHellmanKeyShare_Release(&keyShare);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_SerializeDeserializePublicKey)
{
    PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(keyShare, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");

    PARCBuffer *publicKey = parcDiffieHellmanKeyShare_SerializePublicKey(keyShare);
    assertNotNull(publicKey, "Expected the public key to be serialized to a non-NULL PARCBuffer");

    const size_t sec521r1KeySize = 266;
    assertTrue(parcBuffer_Remaining(publicKey) == sec521r1KeySize, "Expected the public key size to be %zu, got %zu", sec521r1KeySize, parcBuffer_Remaining(publicKey));

    // Deserialize the public key to get the OpenSSL EVP_PKEY type
    EVP_PKEY *rawPublicKey = _parcDiffieHellman_DeserializePublicKeyShare(keyShare, publicKey);
    assertNotNull(rawPublicKey, "Expected the raw public key to be deserialized");

    // Extract the public portions of the private key share and public key share
    EC_KEY *publicEcKey = EVP_PKEY_get1_EC_KEY(rawPublicKey);
    const EC_POINT *publicPoint = EC_KEY_get0_public_key(publicEcKey);

    EC_KEY *privateEcKey = EVP_PKEY_get1_EC_KEY(keyShare->privateKey);
    const EC_POINT *privatePoint = EC_KEY_get0_public_key(privateEcKey);

    // Compare the public portions of the key shares
    const EC_GROUP *group = EC_KEY_get0_group(publicEcKey);
    BN_CTX *bigNumberContext = BN_CTX_new();
    int equalResult = EC_POINT_cmp(group, publicPoint, privatePoint, bigNumberContext);
    assertTrue(equalResult == 0, "Expected the two public points to be equal.");

    BN_CTX_free(bigNumberContext);
    EVP_PKEY_free(rawPublicKey);
    parcBuffer_Release(&publicKey);
    parcDiffieHellmanKeyShare_Release(&keyShare);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_DeserializePublicKey_ErrorWrongGroup)
{
    PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(keyShare, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");

    PARCBuffer *publicKey = parcDiffieHellmanKeyShare_SerializePublicKey(keyShare);
    assertNotNull(publicKey, "Expected the public key to be serialized to a non-NULL PARCBuffer");

    PARCDiffieHellmanKeyShare *alternateKeyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Prime256v1);

    // Deserialize the public key with a different group and hit the failure
    EVP_PKEY *rawPublicKey = _parcDiffieHellman_DeserializePublicKeyShare(alternateKeyShare, publicKey);
    assertNull(rawPublicKey, "Expected the raw public key to not be deserialized");

    parcBuffer_Release(&publicKey);
    parcDiffieHellmanKeyShare_Release(&keyShare);
    parcDiffieHellmanKeyShare_Release(&alternateKeyShare);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_DeserializePublicKey_ErrorInvalidEncoding)
{
    PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(keyShare, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");

    // Deserialize the public key with a different group
    PARCBuffer *publicKey = parcBuffer_Allocate(32);
    EVP_PKEY *rawPublicKey = _parcDiffieHellman_DeserializePublicKeyShare(keyShare, publicKey);
    assertNull(rawPublicKey, "Expected the raw public key to not be deserialized");

    parcBuffer_Release(&publicKey);
    parcDiffieHellmanKeyShare_Release(&keyShare);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_Combine)
{
    PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(keyShare, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");

    PARCBuffer *publicKey = parcDiffieHellmanKeyShare_SerializePublicKey(keyShare);
    assertNotNull(publicKey, "Expected the public key to be serialized to a non-NULL PARCBuffer");

    PARCBuffer *sharedSecret = parcDiffieHellmanKeyShare_Combine(keyShare, publicKey);
    assertNotNull(sharedSecret, "Expected the shared secret to be non-NULL");

    const size_t secretSize = 32; // = 256 / 8 bytes
    assertTrue(parcBuffer_Remaining(sharedSecret) == secretSize, "invalid size");

    parcBuffer_Release(&sharedSecret);
    parcBuffer_Release(&publicKey);
    parcDiffieHellmanKeyShare_Release(&keyShare);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellmanKeyShare_Combine_Error_PublicKeyDeserializationFail)
{
    PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(keyShare, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");

    PARCBuffer *publicKey = parcBuffer_Allocate(32);
    PARCBuffer *sharedSecret = parcDiffieHellmanKeyShare_Combine(keyShare, publicKey);
    assertNull(sharedSecret, "Expected the shared secret to be non-NULL");

    parcBuffer_Release(&publicKey);
    parcDiffieHellmanKeyShare_Release(&keyShare);
}

LONGBOW_TEST_CASE(Global, _parcDiffieHellmanKeyShare_HashSharedSecret)
{
    PARCBuffer *input = parcBuffer_Allocate(1024);
    PARCBuffer *digestValue = _parcDiffieHellmanKeyShare_HashSharedSecret(input);
    size_t digestLength = parcBuffer_Remaining(digestValue);
    size_t expectedLength = 32; // 256 bits for PARCCryptoHashType_SHA256
    assertTrue(digestLength == 32, "Expected a %zu byte digest, got %zu", expectedLength, digestLength);

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBuffer(hasher, input);
    PARCCryptoHash *digest = parcCryptoHasher_Finalize(hasher);

    PARCBuffer *computedDigest = parcBuffer_Acquire(parcCryptoHash_GetDigest(digest));

    parcCryptoHash_Release(&digest);
    parcCryptoHasher_Release(&hasher);

    assertTrue(parcBuffer_Equals(digestValue, computedDigest), "Expected the secret input to be hashed correctly.");

    parcBuffer_Release(&input);
    parcBuffer_Release(&digestValue);
    parcBuffer_Release(&computedDigest);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_DiffieHellmanKeyShare);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
