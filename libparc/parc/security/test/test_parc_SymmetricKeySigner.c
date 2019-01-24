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
#include "../parc_SymmetricKeySigner.c"
#include <sys/param.h>

#include <errno.h>
#include <config.h>
#include <fcntl.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <LongBow/longBow_Compiler.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

static PARCSymmetricKeySigner *
_createSigner()
{
    PARCBuffer *secret_key = parcSymmetricKeyStore_CreateKey(256);

    PARCSymmetricKeyStore *symmetricKeyStore = parcSymmetricKeyStore_Create(secret_key);
    parcBuffer_Release(&secret_key);

    PARCSymmetricKeySigner *privateKeySigner = parcSymmetricKeySigner_Create(symmetricKeyStore, PARCCryptoHashType_SHA256);
    parcSymmetricKeyStore_Release(&symmetricKeyStore);

    return privateKeySigner;
}

LONGBOW_TEST_RUNNER(parc_SymmetricSigner)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_SymmetricSigner)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_SymmetricSigner)
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
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCSymmetricKeySigner *instance = _createSigner();

    assertNotNull(instance, "Expected non-null result from parcSymmetricKeySigner_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcSymmetricKeySigner_Acquire, instance);

    parcSymmetricKeySigner_Release(&instance);
    assertNull(instance, "Expected null result from parcSymmetricKeySigner_Release();");
}

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, test_hmac_sha256);
    LONGBOW_RUN_TEST_CASE(Specialization, test_hmac_sha512);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}
LONGBOW_TEST_CASE(Specialization, test_hmac_sha256)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_CTX *ctx = HMAC_CTX_new();
#else
    HMAC_CTX ctx;
#endif
    char key[] = "apple_pie_is_good";
    int fd;
    uint8_t to_digest_buffer[MAXPATHLEN];
    ssize_t to_digest_length;

    uint8_t true_hmac_buffer[MAXPATHLEN];
    ssize_t true_hmac_length;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_Init_ex(ctx, key, sizeof(key), EVP_sha256(), NULL);
#else
    LONGBOW_STOP_DEPRECATED_WARNINGS
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, sizeof(key), EVP_sha256(), NULL);
    LONGBOW_START_DEPRECATED_WARNINGS
#endif
    fd = open("test_random_bytes", O_RDONLY);
    assertTrue(fd > 0, "Could not open input file: %s", strerror(errno));
    to_digest_length = read(fd, to_digest_buffer, sizeof(to_digest_buffer));
    assertTrue(to_digest_length > 0, "Could not read input file: %s", strerror(errno));
    close(fd);

    fd = open("test_random_bytes.hmac_sha256", O_RDONLY);
    assertTrue(fd > 0, "Could not open input file: %s", strerror(errno));
    true_hmac_length = read(fd, true_hmac_buffer, sizeof(true_hmac_buffer));
    assertTrue(true_hmac_length > 0, "Could not read input file: %s", strerror(errno));
    close(fd);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    _hmacInit(ctx);
    _hmacUpdate(ctx, to_digest_buffer, to_digest_length);
    PARCBuffer *output = _hmacFinalize(ctx);
#else
    _hmacInit(&ctx);
    _hmacUpdate(&ctx, to_digest_buffer, to_digest_length);
    PARCBuffer *output = _hmacFinalize(&ctx);
#endif

    assertTrue(parcBuffer_Position(output) == true_hmac_length,
               "hmac wrong length, expected %zu got %zu",
               true_hmac_length,
               parcBuffer_Position(output));

    assertTrue(memcmp(parcByteArray_Array(parcBuffer_Array(output)), true_hmac_buffer, true_hmac_length) == 0,
               "hmac values did not match");

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_CTX_free(ctx);
#else
    LONGBOW_STOP_DEPRECATED_WARNINGS
    HMAC_cleanup(&ctx);
    LONGBOW_START_DEPRECATED_WARNINGS
#endif
    parcBuffer_Release(&output);
}

LONGBOW_TEST_CASE(Specialization, test_hmac_sha512)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_CTX *ctx = HMAC_CTX_new();
#else
    HMAC_CTX ctx;
#endif
    char key[] = "apple_pie_is_good";
    int fd;
    uint8_t to_digest_buffer[MAXPATHLEN];
    ssize_t to_digest_length;

    uint8_t true_hmac_buffer[MAXPATHLEN];
    ssize_t true_hmac_length;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_Init_ex(ctx, key, sizeof(key), EVP_sha512(), NULL);
#else
    LONGBOW_STOP_DEPRECATED_WARNINGS
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, sizeof(key), EVP_sha512(), NULL);
    LONGBOW_START_DEPRECATED_WARNINGS
#endif
    fd = open("test_random_bytes", O_RDONLY);
    assertTrue(fd > 0, "Could not open input file: %s", strerror(errno));
    to_digest_length = read(fd, to_digest_buffer, sizeof(to_digest_buffer));
    assertTrue(to_digest_length > 0, "Could not read input file: %s", strerror(errno));
    close(fd);

    fd = open("test_random_bytes.hmac_sha512", O_RDONLY);
    assertTrue(fd > 0, "Could not open input file: %s", strerror(errno));
    true_hmac_length = read(fd, true_hmac_buffer, sizeof(true_hmac_buffer));
    assertTrue(true_hmac_length > 0, "Could not read input file: %s", strerror(errno));
    close(fd);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    _hmacInit(ctx);
    _hmacUpdate(ctx, to_digest_buffer, to_digest_length);
    PARCBuffer *output = _hmacFinalize(ctx);
#else
    _hmacInit(&ctx);
    _hmacUpdate(&ctx, to_digest_buffer, to_digest_length);
    PARCBuffer *output = _hmacFinalize(&ctx);
#endif

    assertTrue(parcBuffer_Position(output) == true_hmac_length,
               "hmac wrong length, expected %zu got %zu",
               true_hmac_length,
               parcBuffer_Position(output));

    assertTrue(memcmp(parcByteArray_Array(parcBuffer_Array(output)), true_hmac_buffer, true_hmac_length) == 0,
               "hmac values did not match");
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_CTX_free(ctx);
#else
    LONGBOW_STOP_DEPRECATED_WARNINGS
    HMAC_cleanup(&ctx);
    LONGBOW_START_DEPRECATED_WARNINGS
#endif
    parcBuffer_Release(&output);
}

LONGBOW_TEST_CASE(Global, parcSymmetricSigner_SignatureSize)
{
    PARCSymmetricKeySigner *symmetricSigner = _createSigner();

    PARCSigner *signer = parcSigner_Create(symmetricSigner, PARCSymmetricKeySignerAsSigner);

    assertTrue(parcSigner_GetSignatureSize(signer) == 32, "Key size unexpected %zu ", parcSigner_GetSignatureSize(signer));
    parcSigner_Release(&signer);
    parcSymmetricKeySigner_Release(&symmetricSigner);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_SymmetricSigner);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
