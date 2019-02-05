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
 * @file test_parc_ByteArray.c
 *
 */
#include "../parc_ByteArray.c"
#include <stdio.h>

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>
#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(PARCByteArray)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(PARCByteArray)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(PARCByteArray)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Acquire_destroyoriginal);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Allocate);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Allocate_ZeroLength);

    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Wrap_NULL);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Wrap_ZeroLength);

    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Wrap);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Array);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_AddressOfIndex);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Capacity);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Copy_Allocated);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Copy_Wrapped);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_PutBytes);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_CopyOut);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_CopyInByteArray);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Get);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Put);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcByteArray_Display);
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

LONGBOW_TEST_CASE(Global, parcByteArray_Allocate)
{
    PARCByteArray *actual = parcByteArray_Allocate(10);

    parcByteArray_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Allocate_ZeroLength)
{
    PARCByteArray *actual = parcByteArray_Allocate(0);
    assertNotNull(actual, "parcByteArray_Allocate(0) must not return NULL.");
    assertTrue(parcByteArray_Capacity(actual) == 0, "Expected capacity to be 0");

    parcByteArray_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Wrap)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *actual = parcByteArray_Wrap(10, buffer);

    parcByteArray_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Wrap_NULL)
{
    PARCByteArray *actual = parcByteArray_Wrap(10, NULL);

    assertNull(actual, "Expected NULL return value from parcByteArray_Wrap()");
}

LONGBOW_TEST_CASE(Global, parcByteArray_Wrap_ZeroLength)
{
    PARCByteArray *actual = parcByteArray_Wrap(0, (uint8_t[1]) { 0 });

    assertNotNull(actual, "Expected non-NULL return value from parcByteArray_Wrap()");
    assertTrue(parcByteArray_Capacity(actual) == 0, "Expected capacity to be zero.");
    parcByteArray_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Array)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *actual = parcByteArray_Wrap(10, buffer);
    assertTrue(buffer == parcByteArray_Array(actual), "Expected the array to be the wrapped array.");

    parcByteArray_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcByteArray_AddressOfIndex)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *actual = parcByteArray_Wrap(10, buffer);
    uint8_t *address = parcByteArray_AddressOfIndex(actual, 3);

    assertTrue(buffer[3] == *address,
               "Expected %d, actual %d", buffer[3], *address);

    parcByteArray_Release(&actual);
}


LONGBOW_TEST_CASE(Global, parcByteArray_Release)
{
    PARCByteArray *actual = parcByteArray_Allocate(10);

    parcByteArray_Release(&actual);
    assertNull(actual, "Expected the pointer to be NULL after parcByteArray_Release");
}

LONGBOW_TEST_CASE(Global, parcByteArray_Copy_Allocated)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *original = parcByteArray_Allocate(sizeof(buffer));
    parcByteArray_PutBytes(original, 0, sizeof(buffer), buffer);

    PARCByteArray *clone = parcByteArray_Copy(original);

    assertTrue(original != clone, "Expected clone to be a different instance that original.");

    assertTrue(parcByteArray_Equals(original, clone), "Expected the clone to be equal to the original.");

    parcByteArray_Release(&original);
    parcByteArray_Release(&clone);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Copy_Wrapped)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *original = parcByteArray_Wrap(sizeof(buffer), buffer);

    PARCByteArray *clone = parcByteArray_Copy(original);

    assertTrue(original != clone, "Expected clone to be a different instance that original.");

    assertTrue(parcByteArray_Equals(original, clone), "Expected the clone to be equal to the original.");

    parcByteArray_Release(&original);
    parcByteArray_Release(&clone);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Compare)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *x = parcByteArray_Wrap(sizeof(buffer), buffer);

    PARCByteArray **equivalents = (PARCByteArray *[]) {
        parcByteArray_Wrap(sizeof(buffer), buffer),
        NULL
    };
    PARCByteArray **lessers = (PARCByteArray *[]) {
        parcByteArray_Wrap(sizeof(buffer) - 1, buffer),
        parcByteArray_Wrap(sizeof(buffer) - 1, (uint8_t[]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 8 }),
        NULL
    };
    PARCByteArray **greaters = (PARCByteArray *[]) {
        parcByteArray_Wrap(11, (uint8_t[]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }),
        parcByteArray_Wrap(10, (uint8_t[]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }),
        NULL
    };
    /*
     * (a - b)
     */
    parcObjectTesting_AssertCompareTo(parcByteArray_Compare, x, equivalents, lessers, greaters);

    parcByteArray_Release(&x);

    for (int i = 0; equivalents[i] != NULL; i++) {
        parcByteArray_Release(&equivalents[i]);
    }
    for (int i = 0; lessers[i] != NULL; i++) {
        parcByteArray_Release(&lessers[i]);
    }
    for (int i = 0; greaters[i] != NULL; i++) {
        parcByteArray_Release(&greaters[i]);
    }
}

LONGBOW_TEST_CASE(Global, parcByteArray_Equals)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *x = parcByteArray_Wrap(10, buffer);
    PARCByteArray *y = parcByteArray_Wrap(10, buffer);
    PARCByteArray *z = parcByteArray_Wrap(10, buffer);
    PARCByteArray *u1 = parcByteArray_Wrap(5, buffer);
    PARCByteArray *u2 = parcByteArray_Allocate(5);

    parcObjectTesting_AssertEqualsFunction(parcByteArray_Equals, x, y, z, u1, u2, NULL);

    parcByteArray_Release(&x);
    parcByteArray_Release(&y);
    parcByteArray_Release(&z);
    parcByteArray_Release(&u1);
    parcByteArray_Release(&u2);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Capacity)
{
    size_t expected = 10;

    PARCByteArray *actual = parcByteArray_Allocate(expected);
    assertTrue(expected == parcByteArray_Capacity(actual), "Expected %zd, actual %zd", expected, parcByteArray_Capacity(actual));

    parcByteArray_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcByteArray_CopyOut)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t actual[10];

    PARCByteArray *original = parcByteArray_Wrap(10, expected);
    parcByteArray_GetBytes(original, 0, sizeof(actual), actual);

    assertTrue(memcmp(expected, actual, sizeof(actual)) == 0,
               "Expected parcByteArray_CopyOut to copy the orginal data");

    parcByteArray_Release(&original);
}

LONGBOW_TEST_CASE(Global, parcByteArray_PutBytes)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t actual[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    PARCByteArray *original = parcByteArray_Wrap(10, expected);
    parcByteArray_PutBytes(original, 0, 10, actual);

    assertTrue(memcmp(expected, actual, 10) == 0,
               "Expected parcByteArray_CopyOut to copy the orginal data");

    parcByteArray_Release(&original);
}

LONGBOW_TEST_CASE(Global, parcByteArray_CopyInByteArray)
{
    uint8_t array1[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t array2[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t expected[10] = { 0, 1, 2, 0, 0, 0, 6, 7, 8, 9 };

    PARCByteArray *a1 = parcByteArray_Wrap(10, array1);
    PARCByteArray *a2 = parcByteArray_Wrap(10, array2);
    parcByteArray_ArrayCopy(a1, 3, a2, 0, 3);

    assertTrue(memcmp(expected, parcByteArray_Array(a1), 10) == 0,
               "Expected parcByteArray_CopyOut to copy the orginal data");

    parcByteArray_Release(&a1);
    parcByteArray_Release(&a2);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Get)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *original = parcByteArray_Wrap(10, buffer);

    for (uint8_t index = 0; index < 10; index++) {
        uint8_t actual = parcByteArray_GetByte(original, index);
        assertTrue(index == actual, "Expected %d, actual %d", index, actual);
    }
    parcByteArray_Release(&original);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Put)
{
    uint8_t buffer[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    PARCByteArray *original = parcByteArray_Wrap(10, buffer);

    for (uint8_t index = 0; index < 10; index++) {
        parcByteArray_PutByte(original, index, index);
    }

    for (uint8_t index = 0; index < 10; index++) {
        uint8_t actual = parcByteArray_GetByte(original, index);
        assertTrue(index == actual, "Expected %d, actual %d", index, actual);
    }

    parcByteArray_Release(&original);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Acquire)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *actual = parcByteArray_Wrap(10, buffer);
    PARCByteArray *reference = parcByteArray_Acquire(actual);

    assertTrue(reference == actual, "Expected the new reference to be equal to the original.");

    PARCByteArray *new1 = parcByteArray_Acquire(actual);
    assertTrue(new1 == actual, "Expected new to be the same as actual");

    PARCByteArray *new2 = parcByteArray_Acquire(actual);
    assertTrue(new2 == actual, "Expected new to be the same as actual");

    parcByteArray_Release(&new1);
    assertNull(new1, "Expected destroy to null the pointer");
    assertNotNull(actual, "Expected destroy to NOT null the original pointer");

    parcByteArray_Release(&new2);
    assertNull(new1, "Expected destroy to null the pointer");
    assertNotNull(actual, "Expected destroy to NOT null the original pointer");

    parcByteArray_Release(&reference);
    parcByteArray_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Acquire_destroyoriginal)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *original = parcByteArray_Wrap(10, buffer);

    PARCByteArray *ref1 = parcByteArray_Acquire(original);
    assertTrue(ref1 == original, "Expected new to be the same as original");

    parcByteArray_Release(&original);
    assertNull(original, "Expected destroy to null the pointer");
    assertNotNull(ref1, "Expected destroy to NOT null the new reference");

    parcByteArray_Release(&ref1);
}

LONGBOW_TEST_CASE(Global, parcByteArray_HashCode)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCByteArray *x = parcByteArray_Wrap(10, buffer);
    PARCByteArray *y = parcByteArray_Wrap(10, buffer);

    PARCHashCode hashX = parcByteArray_HashCode(x);
    PARCHashCode hashY = parcByteArray_HashCode(y);

    assertTrue(hashX == hashY,
               "Expected %" PRIPARCHashCode ", actual %" PRIPARCHashCode, hashX, hashY);

    parcByteArray_Release(&x);
    parcByteArray_Release(&y);
}

LONGBOW_TEST_CASE(Global, parcByteArray_Display)
{
    uint8_t buffer[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };

    PARCByteArray *x = parcByteArray_Wrap(sizeof(buffer), buffer);

    parcByteArray_Display(x, 0);

    parcByteArray_Release(&x);
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, parcByteArray_Put_overrun);
    LONGBOW_RUN_TEST_CASE(Errors, parcByteArray_Get_overrun);
    LONGBOW_RUN_TEST_CASE(Errors, parcByteArray_CopyIn_overrun);
    LONGBOW_RUN_TEST_CASE(Errors, parcByteArray_CopyOut_overrun);
}

typedef struct parc_byte_array_longbow_clipboard {
    PARCByteArray *byteArray;
} parcByteArray_LongBowClipBoard;

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    parcByteArray_LongBowClipBoard *clipboardData = calloc(1, sizeof(parcByteArray_LongBowClipBoard));
    clipboardData->byteArray = parcByteArray_Allocate(10);

    longBowTestCase_SetClipBoardData(testCase, clipboardData);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    parcByteArray_LongBowClipBoard *clipboardData = longBowTestCase_GetClipBoardData(testCase);
    parcByteArray_Release(&clipboardData->byteArray);
    free(clipboardData);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcByteArray_Put_overrun, .event = &LongBowTrapOutOfBounds)
{
    parcByteArray_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCByteArray *original = testData->byteArray;

    for (uint8_t index = 0; index < 10 + 1; index++) {
        parcByteArray_PutByte(original, index, index); // This will fail.
    }
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcByteArray_CopyIn_overrun, .event = &LongBowTrapOutOfBounds)
{
    uint8_t actual[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    parcByteArray_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCByteArray *original = testData->byteArray;
    parcByteArray_GetBytes(original, 1, 10, actual); // This will fail.
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcByteArray_CopyOut_overrun, .event = &LongBowTrapOutOfBounds)
{
    uint8_t actual[10];

    parcByteArray_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCByteArray *original = testData->byteArray;
    parcByteArray_GetBytes(original, 1, 10, actual); // This will fail.
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcByteArray_Get_overrun, .event = &LongBowTrapOutOfBounds)
{
    uint8_t buffer[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    parcByteArray_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCByteArray *original = testData->byteArray;
    parcByteArray_PutBytes(original, 0, 10, buffer);

    for (uint8_t index = 0; index < 10 + 1; index++) {
        uint8_t actual = parcByteArray_GetByte(original, index); // this will fail.
        assertTrue(index == actual, "Expected %d, actual %d", index, actual);
    }
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARCByteArray);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
