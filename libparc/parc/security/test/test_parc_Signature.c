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
#include "../parc_Signature.c"
#include <inttypes.h>

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_Signature)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Signature)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Signature)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_GetHashType);
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_GetSignature);
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_GetSigningAlgorithm);
    LONGBOW_RUN_TEST_CASE(Global, parcSignature_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcSignature_Create)
{
    PARCBuffer *bits = parcBuffer_Allocate(10); // arbitrary buffer size -- not important
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, bits);
    parcBuffer_Release(&bits);

    assertNotNull(signature, "Expected non-NULL PARCSignature");
    PARCReferenceCount referenceCount = parcObject_GetReferenceCount(signature);
    assertTrue(referenceCount == 1,
               "Expected reference count to be equal to 1, got %" PRIu64 "",
               referenceCount);

    parcSignature_Release(&signature);
}

LONGBOW_TEST_CASE(Global, parcSignature_Acquire)
{
    PARCBuffer *bits = parcBuffer_Allocate(10); // arbitrary buffer size -- not important
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, bits);
    PARCSignature *handle = parcSignature_Acquire(signature);
    parcBuffer_Release(&bits);

    assertNotNull(signature, "Expected non-NULL PARCSignature");
    assertNotNull(handle, "Expected non-NULL PARCSignature after acquisition");
    PARCReferenceCount referenceCount = parcObject_GetReferenceCount(handle);
    assertTrue(referenceCount == 2,
               "Expected reference count to be equal to 2, got %" PRIu64 "",
               referenceCount);

    parcSignature_Release(&signature);
    parcSignature_Release(&handle);
}

LONGBOW_TEST_CASE(Global, parcSignature_Release)
{
    PARCBuffer *bits = parcBuffer_Allocate(10); // arbitrary bufer size -- not important
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, bits);
    PARCSignature *handle = parcSignature_Acquire(signature);
    parcBuffer_Release(&bits);

    assertNotNull(signature, "Expected non-NULL PARCSignature");
    assertNotNull(handle, "Expected non-NULL PARCSignature after acquisition");
    PARCReferenceCount referenceCount = parcObject_GetReferenceCount(handle);
    assertTrue(referenceCount == 2,
               "Expected reference count to be equal to 2, got %" PRIu64 "",
               referenceCount);

    parcSignature_Release(&signature);
    parcSignature_Release(&handle);

    assertNull(signature, "Expected NULL PARCSignature");
}

LONGBOW_TEST_CASE(Global, parcSignature_Equals)
{
    PARCBuffer *bits = parcBuffer_Allocate(10); // arbitrary bufer size -- not important
    PARCBuffer *otherBits = parcBuffer_Allocate(strlen("hello"));
    parcBuffer_PutArray(otherBits, strlen("hello"), (uint8_t *) "hello");

    PARCSignature *x = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, bits);
    PARCSignature *y = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, bits);
    PARCSignature *z = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, bits);
    PARCSignature *unequal1 = parcSignature_Create(PARCSigningAlgorithm_HMAC, PARCCryptoHashType_SHA256, bits);
    PARCSignature *unequal2 = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_CRC32C, bits);
    PARCSignature *unequal3 = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, otherBits);

    parcObjectTesting_AssertEqualsFunction(parcSignature_Equals, x, y, z, unequal1, unequal2, unequal3, NULL);

    parcSignature_Release(&x);
    parcSignature_Release(&y);
    parcSignature_Release(&z);
    parcSignature_Release(&unequal1);
    parcSignature_Release(&unequal2);
    parcSignature_Release(&unequal3);

    parcBuffer_Release(&bits);
    parcBuffer_Release(&otherBits);
}

LONGBOW_TEST_CASE(Global, parcSignature_GetHashType)
{
    PARCBuffer *bits = parcBuffer_Allocate(strlen("Hello"));
    parcBuffer_PutArray(bits, strlen("Hello"), (uint8_t *) "Hello");
    PARCCryptoHashType expected = PARCCryptoHashType_SHA256;
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_ECDSA, expected, bits);
    parcBuffer_Release(&bits);

    PARCCryptoHashType actual = parcSignature_GetHashType(signature);

    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
    parcSignature_Release(&signature);
}

LONGBOW_TEST_CASE(Global, parcSignature_GetSignature)
{
    PARCBuffer *expected = parcBuffer_Allocate(strlen("Hello"));
    parcBuffer_PutArray(expected, strlen("Hello"), (uint8_t *) "Hello");
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_ECDSA, PARCCryptoHashType_SHA256, expected);

    PARCBuffer *actual = parcSignature_GetSignature(signature);

    assertTrue(parcBuffer_Equals(expected, actual), "Expected the original signature bits to be equal to the actual bits");
    parcSignature_Release(&signature);
    parcBuffer_Release(&expected);
}

LONGBOW_TEST_CASE(Global, parcSignature_GetSigningAlgorithm)
{
    PARCBuffer *signatureBits = parcBuffer_Allocate(strlen("Hello"));
    parcBuffer_PutArray(signatureBits, strlen("Hello"), (uint8_t *) "Hello");
    PARCSigningAlgorithm expected = PARCSigningAlgorithm_ECDSA;
    PARCSignature *signature = parcSignature_Create(expected, PARCCryptoHashType_SHA256, signatureBits);

    PARCSigningAlgorithm actual = parcSignature_GetSigningAlgorithm(signature);

    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
    parcSignature_Release(&signature);
    parcBuffer_Release(&signatureBits);
}

LONGBOW_TEST_CASE(Global, parcSignature_ToString)
{
    PARCBuffer *signatureBits = parcBuffer_Allocate(strlen("Hello"));
    parcBuffer_PutArray(signatureBits, strlen("Hello"), (uint8_t *) "Hello");
    PARCSigningAlgorithm expected = PARCSigningAlgorithm_ECDSA;
    PARCSignature *signature = parcSignature_Create(expected, PARCCryptoHashType_SHA256, signatureBits);

    char *string = parcSignature_ToString(signature);

    assertNotNull(string, "Expected non-NULL result from parcSignature_ToString");

    parcMemory_Deallocate((void **) &string);
    parcSignature_Release(&signature);
    parcBuffer_Release(&signatureBits);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Signature);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
