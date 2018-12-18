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

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/security/parc_Security.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_CryptoHasher.c"

#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

const int bufferLength = 1024;

/*
 * Ground truth set derived from CRC RevEng http://reveng.sourceforge.net
 * e.g. reveng -c  -m CRC-32C 313233343536373839 gives the canonical check value 0xe306928e
 *
 * You can also calcaulate them online at http://www.zorc.breitbandkatze.de/crc.html using
 * CRC polynomial 0x1EDC6F41, init 0xFFFFFFFF, final 0xFFFFFFFF, reverse data bytes (check),
 * and reverse CRC result before final XOR (check).
 *
 */
struct test_vector {
    uint32_t crc32c;
    int length;
    uint8_t *buffer;
} vectors[] = {
    { .crc32c = 0xe3069283, .length = 9,  .buffer = (uint8_t []) { '1',  '2', '3', '4', '5', '6', '7', '8', '9' } },
    { .crc32c = 0xddb65633, .length = 1,  .buffer = (uint8_t []) { 0x3D } },
    { .crc32c = 0xc203c1fd, .length = 2,  .buffer = (uint8_t []) { 0x3D, 0x41 } },
    { .crc32c = 0x80a9d169, .length = 3,  .buffer = (uint8_t []) { 'b',  'e', 'e' } },
    { .crc32c = 0xa099f534, .length = 4,  .buffer = (uint8_t []) { 'h',  'e', 'l', 'l' } },
    { .crc32c = 0x9a71bb4c, .length = 5,  .buffer = (uint8_t []) { 'h',  'e', 'l', 'l', 'o' } },
    { .crc32c = 0x2976E503, .length = 6,  .buffer = (uint8_t []) { 'g',  'r', 'u', 'm', 'p', 'y' } },
    { .crc32c = 0xe627f441, .length = 7,  .buffer = (uint8_t []) { 'a',  'b', 'c', 'd', 'e', 'f', 'g' } },
    { .crc32c = 0x2d265c1d, .length = 13, .buffer = (uint8_t []) { 'a',  'b', 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'c', 'd', 'e', 'f'} },
    { .crc32c = 0,          .length = 0,  .buffer = NULL }
};


LONGBOW_TEST_RUNNER(parc_CryptoHasher)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
//    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_CryptoHasher)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_CryptoHasher)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHasher_Create);

    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHasher_Bytes_256);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHasher_Buffer_256);

    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHasher_Bytes_512);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHasher_Buffer_512);

    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHasher_CRC32);

    LONGBOW_RUN_TEST_CASE(Global, parcCryptoHasher_CustomHasher);
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

LONGBOW_TEST_CASE(Global, parcCryptoHasher_Create)
{
    PARCCryptoHasher *hasher;

    hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Release(&hasher);

    hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA512);
    PARCCryptoHasher *handle = parcCryptoHasher_Acquire(hasher);

    assertTrue(parcObject_GetReferenceCount(handle) == 2, "Expected 2 references");

    parcCryptoHasher_Release(&hasher);
    parcCryptoHasher_Release(&handle);
}

LONGBOW_TEST_CASE(Global, parcCryptoHasher_Bytes_256)
{
    int fd_buffer = open("test_digest_bytes_128.bin", O_RDONLY);
    int fd_truth = open("test_digest_bytes_128.sha256", O_RDONLY);
    assertFalse(fd_buffer < 0, "Could not open %s: %s", "test_digest_bytes_128.bin", strerror(errno));
    assertFalse(fd_truth < 0, "Could not open %s: %s", "test_digest_bytes_128.sha256", strerror(errno));

    uint8_t scratch[bufferLength];
    ssize_t read_length;

    read_length = read(fd_truth, scratch, bufferLength);

    PARCCryptoHash *digestTruth = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch, read_length);

    read_length = read(fd_buffer, scratch, bufferLength);

    PARCCryptoHasher *digester = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(digester);
    parcCryptoHasher_UpdateBytes(digester, scratch, read_length);

    PARCCryptoHash *digestTest = parcCryptoHasher_Finalize(digester);

    assertTrue(parcCryptoHash_Equals(digestTruth, digestTest),
               "sha256 digest of 128-byte buffer using Update_Buffer does not match");

    parcCryptoHasher_Release(&digester);
    parcCryptoHash_Release(&digestTruth);
    parcCryptoHash_Release(&digestTest);

    close(fd_buffer);
    close(fd_truth);
}

LONGBOW_TEST_CASE(Global, parcCryptoHasher_Buffer_256)
{
    int fd_buffer = open("test_digest_bytes_128.bin", O_RDONLY);
    int fd_truth = open("test_digest_bytes_128.sha256", O_RDONLY);
    assertFalse(fd_buffer < 0, "Could not open %s: %s", "test_digest_bytes_128.bin", strerror(errno));
    assertFalse(fd_truth < 0, "Could not open %s: %s", "test_digest_bytes_128.sha256", strerror(errno));

    uint8_t scratch[bufferLength];

    ssize_t read_length = read(fd_buffer, scratch, bufferLength);
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutArray(composer, scratch, read_length);
    PARCBuffer *bb_to_digest = parcBufferComposer_ProduceBuffer(composer);

    read_length = read(fd_truth, scratch, bufferLength);
    PARCCryptoHash *digestTruth = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA256, scratch, read_length);

    PARCCryptoHasher *digester = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(digester);
    parcCryptoHasher_UpdateBuffer(digester, bb_to_digest);
    PARCCryptoHash *digestTest = parcCryptoHasher_Finalize(digester);

    assertTrue(parcCryptoHash_Equals(digestTruth, digestTest),
               "sha256 digest of 128-byte buffer using Update_Buffer does not match");

    parcCryptoHasher_Release(&digester);
    parcBuffer_Release(&bb_to_digest);
    parcBufferComposer_Release(&composer);
    parcCryptoHash_Release(&digestTruth);
    parcCryptoHash_Release(&digestTest);

    close(fd_buffer);
    close(fd_truth);
}

// ==== 512

LONGBOW_TEST_CASE(Global, parcCryptoHasher_Bytes_512)
{
    PARCCryptoHasher *digester;

    int fd_buffer = open("test_digest_bytes_128.bin", O_RDONLY);
    int fd_truth = open("test_digest_bytes_128.sha512", O_RDONLY);
    assertFalse(fd_buffer < 0, "Could not open %s: %s", "test_digest_bytes_128.bin", strerror(errno));
    assertFalse(fd_truth < 0, "Could not open %s: %s", "test_digest_bytes_128.sha512", strerror(errno));

    uint8_t scratch[bufferLength];
    ssize_t read_length;

    read_length = read(fd_truth, scratch, bufferLength);
    PARCCryptoHash *digestTruth = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA512, scratch, read_length);

    read_length = read(fd_buffer, scratch, bufferLength);


    digester = parcCryptoHasher_Create(PARCCryptoHashType_SHA512);
    parcCryptoHasher_Init(digester);
    parcCryptoHasher_UpdateBytes(digester, scratch, read_length);
    PARCCryptoHash *digestTest = parcCryptoHasher_Finalize(digester);

    assertTrue(parcCryptoHash_Equals(digestTruth, digestTest), "sha512 digest of 128-byte buffer using Update_Buffer does not match");

    parcCryptoHasher_Release(&digester);
    parcCryptoHash_Release(&digestTruth);
    parcCryptoHash_Release(&digestTest);

    close(fd_buffer);
    close(fd_truth);
}

LONGBOW_TEST_CASE(Global, parcCryptoHasher_Buffer_512)
{
    int fd_buffer = open("test_digest_bytes_128.bin", O_RDONLY);
    int fd_truth = open("test_digest_bytes_128.sha512", O_RDONLY);
    assertFalse(fd_buffer < 0, "Could not open %s: %s", "test_digest_bytes_128.bin", strerror(errno));
    assertFalse(fd_truth < 0, "Could not open %s: %s", "test_digest_bytes_128.sha512", strerror(errno));

    uint8_t scratch[bufferLength];
    ssize_t read_length;

    read_length = read(fd_buffer, scratch, bufferLength);
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutArray(composer, scratch, read_length);
    PARCBuffer *bb_to_digest = parcBufferComposer_ProduceBuffer(composer);

    read_length = read(fd_truth, scratch, bufferLength);
    PARCCryptoHash *digestTruth = parcCryptoHash_CreateFromArray(PARCCryptoHashType_SHA512, scratch, read_length);

    PARCCryptoHasher *digester = parcCryptoHasher_Create(PARCCryptoHashType_SHA512);
    parcCryptoHasher_Init(digester);
    parcCryptoHasher_UpdateBuffer(digester, bb_to_digest);
    PARCCryptoHash *digestTest = parcCryptoHasher_Finalize(digester);

    assertTrue(parcCryptoHash_Equals(digestTruth, digestTest),
               "sha512 digest of 128-byte buffer using Update_Buffer does not match");

    parcCryptoHasher_Release(&digester);
    parcBuffer_Release(&bb_to_digest);
    parcBufferComposer_Release(&composer);
    parcCryptoHash_Release(&digestTruth);
    parcCryptoHash_Release(&digestTest);

    close(fd_buffer);
    close(fd_truth);
}

LONGBOW_TEST_CASE(Global, parcCryptoHasher_CRC32)
{
    for (int i = 0; vectors[i].buffer != NULL; i++) {
        PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_CRC32C);
        parcCryptoHasher_Init(hasher);
        parcCryptoHasher_UpdateBytes(hasher, vectors[i].buffer, vectors[i].length);
        PARCCryptoHash *output = parcCryptoHasher_Finalize(hasher);
        PARCBuffer *buffer = parcCryptoHash_GetDigest(output);
        uint32_t testCrc = parcBuffer_GetUint32(buffer);
        parcCryptoHash_Release(&output);

        assertTrue(testCrc == vectors[i].crc32c,
                   "CRC32C values wrong, index %d got 0x%08x expected 0x%08x\n",
                   i, testCrc, vectors[i].crc32c);
        parcCryptoHasher_Release(&hasher);
    }
}

LONGBOW_TEST_CASE(Global, parcCryptoHasher_CustomHasher)
{
    PARCCryptoHasher *hasher = parcCryptoHasher_CustomHasher(PARCCryptoHashType_SHA512, functor_sha256);
    assertNotNull(hasher, "Expected to be non null");

    parcCryptoHasher_Release(&hasher);
}

// ================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, computeCrc32C_Software);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, computeCrc32C_Software)
{
    for (int i = 0; vectors[i].buffer != NULL; i++) {
        uint32_t testCrc = _crc32c_Init();
        testCrc = _crc32c_UpdateSoftware(testCrc, vectors[i].length, vectors[i].buffer);
        testCrc = _crc32c_Finalize(testCrc);

        assertTrue(testCrc == vectors[i].crc32c,
                   "CRC32C values wrong, index %d got 0x%08x expected 0x%08x\n",
                   i, testCrc, vectors[i].crc32c);
    }
}

// =======================================================

LONGBOW_TEST_FIXTURE(Performance)
{
    LONGBOW_RUN_TEST_CASE(Performance, computeCrc32C);
    LONGBOW_RUN_TEST_CASE(Performance, computeCrc32C_Software);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static double
runPerformance(int maxreps, uint32_t (*update)(uint32_t crc, size_t len, uint8_t p[len]))
{
    int reps = maxreps;
    int length = 100;

    uint8_t buffer[length];
    for (int i = 0; i < length; i++) {
        buffer[i] = i * 33;
    }

    // initial value doesnt really matter

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    while (--reps) {
        uint32_t crc = _crc32c_Init();
        update(crc, length, buffer);
        crc = _crc32c_Finalize(crc);
    }

    gettimeofday(&t1, NULL);

    timersub(&t1, &t0, &t1);

    double seconds = t1.tv_sec + t1.tv_usec * 1E-6;
    return seconds;
}

LONGBOW_TEST_CASE(Performance, computeCrc32C)
{
    int maxreps = 1000000;
    double seconds = runPerformance(maxreps, _crc32c_Update);
    double rate = maxreps / seconds;

    printf("Best rate = %.3f for %d iterations\n", rate, maxreps);
}

LONGBOW_TEST_CASE(Performance, computeCrc32C_Software)
{
    int maxreps = 1000000;
    double seconds = runPerformance(maxreps, _crc32c_UpdateSoftware);
    double rate = maxreps / seconds;

    printf("Best rate = %.3f for %d iterations\n", rate, maxreps);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_CryptoHasher);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
