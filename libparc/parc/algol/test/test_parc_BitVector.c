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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_BitVector.c"

#include <stdio.h>
#include <parc/algol/parc_SafeMemory.h>
#include <limits.h>

LONGBOW_TEST_RUNNER(parc_BitVector)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_BitVector)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_BitVector)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Create_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_SetClear);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_SetVector);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Reset);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_ClearVector);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_NextBitSet);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Get);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_ToString);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Contains);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Set);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_And);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Or);
    LONGBOW_RUN_TEST_CASE(Global, parcBitVector_Shift);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcBitVector_Create_Release)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");
    PARCBitVector *reference = parcBitVector;
    parcBitVector_Acquire(reference);
    parcBitVector_Release(&parcBitVector);
    parcBitVector_Release(&reference);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Set)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Create created a non-empty vector");

    parcBitVector_Set(parcBitVector, 0);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 1, "Expect number of bits set to be 1");
    assertTrue(parcBitVector->firstBitSet == 0, "Expect first bit set to be 0");
    assertTrue(parcBitVector->bitLength == 8, "Expect the bitLength to be 8");
    assertTrue(parcBitVector->bitArray[0] == (uint8_t) 1, "Expect the bitArray as a unsigned char to be = 1");

    parcBitVector_Set(parcBitVector, 7);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 2, "Expect number of bits set to be 2");
    assertTrue(parcBitVector->firstBitSet == 0, "Expect first bit set to be 0");
    assertTrue(parcBitVector->bitLength == 8, "Expect the bitLength to be 8");
    assertTrue(parcBitVector->bitArray[0] == (uint8_t) 0x81, "Expect the bitArray as a unsigned char to be = 0x81");

    parcBitVector_Set(parcBitVector, 8);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 3, "Expect number of bits set to be 3");
    assertTrue(parcBitVector->firstBitSet == 0, "Expect first bit set to be 0");
    assertTrue(parcBitVector->bitLength == 16, "Expect the bitLength to be 16");
    assertTrue(parcBitVector->bitArray[0] == (uint8_t) 0x81, "Expect the bitArray as a unsigned char to be = 0x81");
    assertTrue(parcBitVector->bitArray[1] == (uint8_t) 0x1, "Expect the bitArray as a unsigned char to be = 0x1");

    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_And)
{
    PARCBitVector *vector1 = parcBitVector_Create();
    PARCBitVector *vector2 = parcBitVector_Create();

    parcBitVector_Set(vector1, 1);
    parcBitVector_Set(vector1, 2);
    parcBitVector_Set(vector1, 10);
    parcBitVector_Set(vector2, 2);
    parcBitVector_Set(vector2, 1);
    parcBitVector_Set(vector2, 20);

    PARCBitVector *result = parcBitVector_And(vector1, vector2);

    assertTrue(parcBitVector_NumberOfBitsSet(result) == 2, "AND vector not equal to expected results");
    parcBitVector_Release(&result);

    result = parcBitVector_And(vector1, NULL);
    assertTrue(parcBitVector_NumberOfBitsSet(result) == 0, "AND vector not equal to expected results");
    parcBitVector_Release(&result);

    result = parcBitVector_And(NULL, vector2);
    assertTrue(parcBitVector_NumberOfBitsSet(result) == 0, "AND vector not equal to expected results");
    parcBitVector_Release(&result);

    result = parcBitVector_And(NULL, NULL);
    assertTrue(parcBitVector_NumberOfBitsSet(result) == 0, "AND vector not equal to expected results");
    parcBitVector_Release(&result);

    parcBitVector_Release(&vector1);
    parcBitVector_Release(&vector2);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Or)
{
    PARCBitVector *vector1 = parcBitVector_Create();
    PARCBitVector *vector2 = parcBitVector_Create();

    parcBitVector_Set(vector1, 1);
    parcBitVector_Set(vector1, 2);
    parcBitVector_Set(vector1, 10);
    parcBitVector_Set(vector2, 2);
    parcBitVector_Set(vector2, 1);
    parcBitVector_Set(vector2, 20);

    PARCBitVector *result = parcBitVector_Or(vector1, vector2);

    assertTrue(parcBitVector_Contains(result, vector1), "Vector contents not included in OR operation results");
    assertTrue(parcBitVector_Contains(result, vector2), "Vector contents not included in OR operation results");
    assertTrue(parcBitVector_NumberOfBitsSet(result) == 4, "OR vector not equal to expected results");
    parcBitVector_Release(&result);

    result = parcBitVector_Or(vector1, NULL);
    assertTrue(parcBitVector_Equals(result, vector1), "OR vector not equal to expected results");
    parcBitVector_Release(&result);

    result = parcBitVector_Or(NULL, vector2);
    assertTrue(parcBitVector_Equals(result, vector2), "OR vector not equal to expected results");
    parcBitVector_Release(&result);

    result = parcBitVector_Or(NULL, NULL);
    assertTrue(parcBitVector_NumberOfBitsSet(result) == 0, "OR vector not equal to expected results");
    parcBitVector_Release(&result);

    parcBitVector_Release(&vector1);
    parcBitVector_Release(&vector2);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Shift)
{
    PARCBitVector *vector = parcBitVector_Create();

    parcBitVector_Set(vector, 0); // should drop off on left shift
    parcBitVector_Set(vector, 11);
    parcBitVector_Set(vector, 12);
    parcBitVector_Set(vector, 13);
    parcBitVector_Set(vector, 22);
    parcBitVector_LeftShift(vector, 10);
    parcBitVector_RightShift(vector, 10);
    assertTrue(parcBitVector_NextBitSet(vector, 0) == 11, "Shift operations failed");
    assertTrue(parcBitVector_NextBitSet(vector, 12) == 12, "Shift operations failed");
    assertTrue(parcBitVector_NextBitSet(vector, 14) == 22, "Shift operations failed");
    assertTrue(parcBitVector_NumberOfBitsSet(vector) == 4, "Shift operations failed to drop first bit on left shift");
    parcBitVector_Release(&vector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_SetClear)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Create created a non-empty vector");

    parcBitVector_Set(parcBitVector, 10);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 1, "parcBitVector_Set failed");

    parcBitVector_Clear(parcBitVector, 10);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Clear failed");

    parcBitVector_Clear(parcBitVector, 20);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Clear failed");

    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_SetVector)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");
    PARCBitVector *setVector = parcBitVector_Create();
    parcBitVector_Set(parcBitVector, 1);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 1, "parcBitVector_Set failed");

    parcBitVector_Set(setVector, 20);
    parcBitVector_SetVector(parcBitVector, setVector);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 2, "parcBitVector_SetVector failed");
    assertTrue(parcBitVector_NextBitSet(parcBitVector, 0) == 1, "parcBitVector_Set failed to set bit 1");
    assertTrue(parcBitVector_NextBitSet(parcBitVector, 2) == 20, "parcBitVector_SetVector failed to set bit 20");

    parcBitVector_Set(setVector, 10);
    parcBitVector_SetVector(parcBitVector, setVector);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 3, "parcBitVector_SetVector failed");
    parcBitVector_Release(&parcBitVector);
    parcBitVector_Release(&setVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Reset)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");

    // Reset and empty vector test
    parcBitVector_Reset(parcBitVector);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Reset failed");

    parcBitVector_Set(parcBitVector, 1);
    parcBitVector_Set(parcBitVector, 42);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 2, "parcBitVector_Set failed");
    assertTrue(parcBitVector->bitLength == 48, "Expected a bitLength of 48");

    parcBitVector_Reset(parcBitVector);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Reset failed");
    assertTrue(parcBitVector->bitLength == 48, "Expected a bitLength of 48");

    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_ClearVector)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");

    PARCBitVector *setVector = parcBitVector_Create();
    parcBitVector_Set(parcBitVector, 1);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 1, "parcBitVector_Set failed to set bit");

    parcBitVector_Set(setVector, 1);
    parcBitVector_Set(setVector, 20);
    parcBitVector_ClearVector(parcBitVector, setVector);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_ClearVector failed to clear vector");

    parcBitVector_Set(parcBitVector, 12);
    parcBitVector_Set(parcBitVector, 17);
    parcBitVector_ClearVector(parcBitVector, parcBitVector);
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_ClearVector failed to clear vector");

    parcBitVector_Release(&parcBitVector);
    parcBitVector_Release(&setVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_NextBitSet)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Create created a non-empty vector");

    int nextBit = parcBitVector_NextBitSet(parcBitVector, 0);
    assertTrue(nextBit == -1, "parcBitVector_NextBitSet should have failed (%d)", nextBit);

    parcBitVector_Set(parcBitVector, 10);
    nextBit = parcBitVector_NextBitSet(parcBitVector, 0);
    assertTrue(nextBit == 10, "parcBitVector_NextBitSet failed (%d)", nextBit);

    nextBit = parcBitVector_NextBitSet(parcBitVector, 20);
    assertTrue(nextBit == -1, "parcBitVector_NextBitSet read past end of vector (%d)", nextBit);

    nextBit = parcBitVector_NextBitSet(parcBitVector, 10);
    assertTrue(nextBit == 10, "parcBitVector_NextBitSet failed (%d)", nextBit);

    nextBit = parcBitVector_NextBitSet(parcBitVector, 11);
    assertTrue(nextBit == -1, "parcBitVector_NextBitSet should have failed (%d)", nextBit);

    parcBitVector_Set(parcBitVector, 20);
    nextBit = parcBitVector_NextBitSet(parcBitVector, 11);
    assertTrue(nextBit == 20, "parcBitVector_NextBitSet failed (%d)", nextBit);
    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Get)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");
    assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "parcBitVector_Create created a non-empty vector");

    parcBitVector_Set(parcBitVector, 10);
    int bitValue = parcBitVector_Get(parcBitVector, 10);
    assertTrue(bitValue == 1, "parcBitVector_Get returned wrong value (%d)", bitValue);

    bitValue = parcBitVector_Get(parcBitVector, 11);
    assertTrue(bitValue == 0, "parcBitVector_Get returned wrong value (%d)", bitValue);

    bitValue = parcBitVector_Get(parcBitVector, 100);
    assertTrue(bitValue == -1, "parcBitVector_NextBitSet should have failed (%d)", bitValue);

    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_ToString)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");

    char *string = parcBitVector_ToString(parcBitVector);
    assertTrue(strcmp(string, "[ ]") == 0, "parcBitVector_ToString returned unexpected representation (%s != [ ])", string);
    parcMemory_Deallocate(&string);

    parcBitVector_Set(parcBitVector, 10);
    parcBitVector_Set(parcBitVector, 1);
    string = parcBitVector_ToString(parcBitVector);
    assertTrue(strcmp(string, "[ 1 10 ]") == 0, "parcBitVector_ToString returned unexpected representation (%s != [ 1 10 ])", string);
    parcMemory_Deallocate(&string);

    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Copy)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");

    parcBitVector_Set(parcBitVector, 10);
    PARCBitVector *copy = parcBitVector_Copy(parcBitVector);
    assertTrue(parcBitVector_NumberOfBitsSet(copy) == 1, "parcBitVector_Copy failed to copy set bit");
    assertTrue(parcBitVector_NextBitSet(copy, 0) == 10, "parcBitVector_Copy failed to copy correct bit");

    parcBitVector_Release(&copy);
    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Equals)
{
    PARCBitVector *parcBitVector = parcBitVector_Create();
    assertTrue(parcBitVector, "parcBitVector_Create returned a NULL pointer");

    parcBitVector_Set(parcBitVector, 10);
    PARCBitVector *copy = parcBitVector_Copy(parcBitVector);
    assertTrue(parcBitVector_Equals(parcBitVector, copy), "Duplicate vector found unequal");

    parcBitVector_Set(copy, 9);
    assertFalse(parcBitVector_Equals(parcBitVector, copy), "Unequal vector found equal");

    parcBitVector_Clear(copy, 9);
    parcBitVector_Set(copy, 29);
    assertFalse(parcBitVector_Equals(parcBitVector, copy), "Unequal long vector found equal");

    parcBitVector_Clear(copy, 29);
    assertTrue(parcBitVector_Equals(parcBitVector, copy), "Equal long vector found unequal");
    assertTrue(parcBitVector_Equals(copy, parcBitVector), "Equal long vector found unequal");

    parcBitVector_Release(&copy);
    parcBitVector_Release(&parcBitVector);
}

LONGBOW_TEST_CASE(Global, parcBitVector_Contains)
{
    PARCBitVector *supersetVector = parcBitVector_Create();

    parcBitVector_Set(supersetVector, 10);
    parcBitVector_Set(supersetVector, 11);

    PARCBitVector *testVector = parcBitVector_Create();
    parcBitVector_Set(testVector, 10);
    assertTrue(parcBitVector_Contains(supersetVector, testVector), "Expect superset to contain testVector");

    parcBitVector_Set(testVector, 12);
    assertFalse(parcBitVector_Contains(supersetVector, testVector), "Expect superset to not contain testVector");

    parcBitVector_Release(&supersetVector);
    parcBitVector_Release(&testVector);
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
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_BitVector);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
