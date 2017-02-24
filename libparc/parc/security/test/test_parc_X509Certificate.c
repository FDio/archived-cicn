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

#include <unistd.h>
#include <fcntl.h>

#include "../parc_X509Certificate.c"
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/algol/parc_Buffer.h>

LONGBOW_TEST_RUNNER(parc_X509Certificate)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(parc_X509Certificate)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_X509Certificate)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_X509Certificate_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parc_X509Certificate_Create);
    LONGBOW_RUN_TEST_CASE(Global, parc_X509Certificate_CreateFromDERBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parc_X509Certificate_GetPublicKeyDigest);
    LONGBOW_RUN_TEST_CASE(Global, parc_X509Certificate_GetCertificateDigest);
    LONGBOW_RUN_TEST_CASE(Global, parc_X509Certificate_GetDEREncodedCertificate);
    LONGBOW_RUN_TEST_CASE(Global, parc_X509Certificate_GetDEREncodedPublicKey);
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

LONGBOW_TEST_CASE(Global, parc_X509Certificate_AcquireRelease)
{
    char *fileName = "test.pem";
    PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(fileName);
    assertNotNull(certificate, "Expected non-NULL certificate");

    PARCReferenceCount firstCount = parcObject_GetReferenceCount(certificate);
    PARCX509Certificate *copy = parcX509Certificate_Acquire(certificate);
    PARCReferenceCount secondCount = parcObject_GetReferenceCount(copy);

    assertTrue(firstCount == (secondCount - 1), "Expected incremented reference count after Acquire");

    parcX509Certificate_Release(&copy);
    PARCReferenceCount thirdCount = parcObject_GetReferenceCount(certificate);

    assertTrue(firstCount == thirdCount, "Expected equal reference counts after Release");

    parcX509Certificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_X509Certificate_Create)
{
    char *fileName = "bad.pem";
    PARCX509Certificate *certificate = _parcX509Certificate_CreateFromPEMFile(fileName);
    assertNull(certificate, "Expected NULL certificate with non-existent file");
}

LONGBOW_TEST_CASE(Global, parc_X509Certificate_CreateFromDERBuffer)
{
    char *fileName = "test.pem";
    PARCX509Certificate *certificate = _parcX509Certificate_CreateFromPEMFile(fileName);
    assertNotNull(certificate, "Expected non-NULL certificate");

    PARCBuffer *certificateBuffer = _getDEREncodedCertificate(certificate);
    PARCX509Certificate *realCertificate = parcX509Certificate_CreateFromDERBuffer(certificateBuffer);
    assertNotNull(realCertificate, "Expected non-NULL certificate to be parsed from DER buffer");

    parcX509Certificate_Release(&certificate);
    parcX509Certificate_Release(&realCertificate);
}

LONGBOW_TEST_CASE(Global, parc_X509Certificate_GetPublicKeyDigest)
{
    char *fileName = "test.pem";
    PARCX509Certificate *certificate = _parcX509Certificate_CreateFromPEMFile(fileName);

    PARCCryptoHash *digest = _getPublicKeyDigest(certificate);
    PARCBuffer *digestBuffer = parcCryptoHash_GetDigest(digest);

    size_t expectedLength = SHA256_DIGEST_LENGTH;
    size_t actualLength = parcBuffer_Remaining(digestBuffer);
    assertTrue(actualLength == expectedLength, "Digest unexpected size: got %zu expected %zu", actualLength, expectedLength);

    int fd = open("test_pubkey.bin", O_RDONLY);
    uint8_t rawDigest[SHA256_DIGEST_LENGTH];
    ssize_t numBytes = read(fd, rawDigest, SHA256_DIGEST_LENGTH);
    assertTrue(numBytes == SHA256_DIGEST_LENGTH, "Expected to read %d bytes, got %zu", SHA256_DIGEST_LENGTH, numBytes);
    close(fd);

    PARCBuffer *rawBuffer = parcBuffer_Flip(parcBuffer_CreateFromArray(rawDigest, SHA256_DIGEST_LENGTH));

    assertTrue(parcBuffer_Remaining(rawBuffer) == SHA256_DIGEST_LENGTH, "Expected %d length buffer", SHA256_DIGEST_LENGTH);

    parcBuffer_Release(&rawBuffer);
    parcX509Certificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_X509Certificate_GetCertificateDigest)
{
    char *fileName = "test.pem";
    PARCX509Certificate *certificate = _parcX509Certificate_CreateFromPEMFile(fileName);

    PARCCryptoHash *digest = _getCertificateDigest(certificate);
    PARCBuffer *digestBuffer = parcCryptoHash_GetDigest(digest);

    size_t expectedLength = SHA256_DIGEST_LENGTH;
    size_t actualLength = parcBuffer_Remaining(digestBuffer);
    assertTrue(actualLength == expectedLength, "Digest unexpected size: got %zu expected %zu", actualLength, expectedLength);

    int fd = open("test_crt_sha256.bin", O_RDONLY);
    uint8_t rawDigest[SHA256_DIGEST_LENGTH];
    ssize_t numBytes = read(fd, rawDigest, SHA256_DIGEST_LENGTH);
    assertTrue(numBytes == SHA256_DIGEST_LENGTH, "Expected to read %d bytes, got %zu", SHA256_DIGEST_LENGTH, numBytes);
    close(fd);

    PARCBuffer *rawBuffer = parcBuffer_Flip(parcBuffer_CreateFromArray(rawDigest, SHA256_DIGEST_LENGTH));

    assertTrue(parcBuffer_Equals(rawBuffer, digestBuffer), "Expected raw binary to equal the computed result.");

    parcBuffer_Release(&rawBuffer);
    parcX509Certificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_X509Certificate_GetDEREncodedCertificate)
{
    char *fileName = "test.pem";
    PARCX509Certificate *certificate = _parcX509Certificate_CreateFromPEMFile(fileName);

    PARCBuffer *digestBuffer = _getDEREncodedCertificate(certificate);

    size_t expectedLength = 517;
    size_t actualLength = parcBuffer_Remaining(digestBuffer);
    assertTrue(actualLength == expectedLength, "Digest unexpected size: got %zu expected %zu", actualLength, expectedLength);

    int fd = open("test_crt_der.bin", O_RDONLY);
    uint8_t rawDigest[expectedLength];
    ssize_t numBytes = read(fd, rawDigest, expectedLength);
    assertTrue(numBytes == expectedLength, "Expected to read %zu bytes, got %zu", expectedLength, numBytes);
    close(fd);

    PARCBuffer *rawBuffer = parcBuffer_Flip(parcBuffer_CreateFromArray(rawDigest, expectedLength));

    assertTrue(parcBuffer_Equals(rawBuffer, digestBuffer), "Expected raw binary to equal the computed result.");

    parcBuffer_Release(&rawBuffer);
    parcX509Certificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_X509Certificate_GetDEREncodedPublicKey)
{
    char *fileName = "test.pem";
    PARCX509Certificate *certificate = _parcX509Certificate_CreateFromPEMFile(fileName);

    PARCBuffer *digestBuffer = _getDEREncodedPublicKey(certificate);

    size_t expectedLength = 162;
    size_t actualLength = parcBuffer_Remaining(digestBuffer);
    assertTrue(actualLength == expectedLength, "Digest unexpected size: got %zu expected %zu", actualLength, expectedLength);

    int fd = open("test_der.bin", O_RDONLY);
    uint8_t rawDigest[expectedLength];
    ssize_t numBytes = read(fd, rawDigest, expectedLength);
    assertTrue(numBytes == expectedLength, "Expected to read %zu bytes, got %zu", expectedLength, numBytes);
    close(fd);

    PARCBuffer *rawBuffer = parcBuffer_Flip(parcBuffer_CreateFromArray(rawDigest, expectedLength));

    assertTrue(parcBuffer_Equals(rawBuffer, digestBuffer), "Expected raw binary to equal the computed result.");

    parcBuffer_Release(&rawBuffer);
    parcX509Certificate_Release(&certificate);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_X509Certificate);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
