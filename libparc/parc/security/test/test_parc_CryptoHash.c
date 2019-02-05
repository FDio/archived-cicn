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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_CryptoHash.c"
#include "../parc_CryptoHasher.h"

#include <LongBow/unit-test.h>
#include <parc/testing/parc_ObjectTesting.h>
#include <fcntl.h>
#include <errno.h>

const int bufferLength = 1024;

LONGBOW_TEST_RUNNER(parc_CryptoHash)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_CryptoHash)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_CryptoHash)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHash_CreateFromArray);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHash_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHash_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHash_GetDigest);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHash_GetDigestType);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcCryptoHash_CreateFromArray)
{
    int fd = open("test_digest_bytes_128.sha256", O_RDONLY);
    assertFalse(fd < 0, "Could not open %s: %s", "test_digest_bytes_128.sha256", strerror(errno));

    uint8_t scratch[bufferLength];
    ssize_t read_length = read(fd, scratch, bufferLength);

    PARCCryptoHash *hash = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch, read_length);
    assertNotNull(hash, "Expected to be non null");

    parcCryptoHash_Release(&hash);
    close(fd);
}

LONGBOW_TEST_CASE(Global, parcCryptoHash_Release)
{
    int fd = open("test_digest_bytes_128.sha256", O_RDONLY);
    assertFalse(fd < 0, "Could not open %s: %s", "test_digest_bytes_128.sha256", strerror(errno));

    uint8_t scratch[bufferLength];
    ssize_t read_length = read(fd, scratch, bufferLength);

    PARCCryptoHash *hash = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch, read_length);
    assertNotNull(hash, "Expected to be non null");

    parcCryptoHash_Release(&hash);
    assertNull(hash, "Expected to be null");
    close(fd);
}

LONGBOW_TEST_CASE(Global, parcCryptoHash_Equals)
{
    int fd1 = open("test_digest_bytes_128.sha256", O_RDONLY);
    assertFalse(fd1 < 0, "Could not open %s: %s", "test_digest_bytes_128.sha256", strerror(errno));

    uint8_t scratch1[bufferLength];
    ssize_t read_length = read(fd1, scratch1, bufferLength);

    PARCCryptoHash *hash1 = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch1, read_length);
    PARCCryptoHash *hash2 = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch1, read_length);
    PARCCryptoHash *hash3 = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch1, read_length);

    int fd2 = open("test_digest_bytes_128.sha512", O_RDONLY);
    assertFalse(fd2 < 0, "Could not open %s: %s", "test_digest_bytes_128.sha512", strerror(errno));

    uint8_t scratch2[bufferLength];
    read_length = read(fd2, scratch2, bufferLength);

    PARCCryptoHash *unequalhash = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch2, read_length);

    parcObjectTesting_AssertEqualsFunction(parcObject_Equals, hash1, hash2, hash3, unequalhash);

    parcCryptoHash_Release(&hash1);
    parcCryptoHash_Release(&hash2);
    parcCryptoHash_Release(&hash3);
    parcCryptoHash_Release(&unequalhash);

    close(fd1);
    close(fd2);
}

LONGBOW_TEST_CASE(Global, parcCryptoHash_GetDigest)
{
    int fd_buffer = open("test_digest_bytes_128.bin", O_RDONLY);
    int fd_truth = open("test_digest_bytes_128.sha256", O_RDONLY);
    assertFalse(fd_buffer < 0, "Could not open %s: %s", "test_digest_bytes_128.bin", strerror(errno));
    assertFalse(fd_truth < 0, "Could not open %s: %s", "test_digest_bytes_128.sha256", strerror(errno));

    uint8_t scratch[bufferLength];
    ssize_t read_length = read(fd_truth, scratch, bufferLength);

    PARCCryptoHash *hashTruth = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch, read_length);

    read_length = read(fd_buffer, scratch, bufferLength);

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, scratch, read_length);

    PARCCryptoHash *hashTest = parcCryptoHasher_Finalize(hasher);

    assertTrue(parcBuffer_Equals(parcCryptoHash_GetDigest(hashTruth), parcCryptoHash_GetDigest(hashTest)), "Expected to be true");

    parcCryptoHasher_Release(&hasher);
    parcCryptoHash_Release(&hashTruth);
    parcCryptoHash_Release(&hashTest);

    close(fd_buffer);
    close(fd_truth);
}

LONGBOW_TEST_CASE(Global, parcCryptoHash_GetDigestType)
{
    int fd = open("test_digest_bytes_128.sha256", O_RDONLY);
    assertFalse(fd < 0, "Could not open %s: %s", "test_digest_bytes_128.sha256", strerror(errno));

    uint8_t scratch[bufferLength];
    ssize_t read_length = read(fd, scratch, bufferLength);

    PARCCryptoHash *hash = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch, read_length);
    assertNotNull(hash, "Expected to be non null");

    assertTrue(PARCCryptoHashType_SHA256 == parcCryptoHash_GetDigestType(hash), "Expected to be true");

    parcCryptoHash_Release(&hash);
    close(fd);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_CryptoHash);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
