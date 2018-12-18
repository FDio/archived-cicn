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
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_ReadOnlyBuffer.c"

#include <stdio.h>
#include <inttypes.h>

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_ReadableBuffer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Getters);
    LONGBOW_RUN_TEST_FIXTURE(CreateDestroy);
    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_ReadableBuffer)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_ReadableBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}
LONGBOW_TEST_FIXTURE(CreateDestroy)
{
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Create);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Allocate_AcquireRelease);
//    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Allocate_AcquireRelease_TooMany);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Wrap);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Wrap_NULL);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Wrap_WithOffset);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateDestroy)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateDestroy)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Create)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Create(buffer);

    assertTrue(parcReadOnlyBuffer_Position(actual) == 0, "Expected initial position to be 0.");
    assertTrue(parcReadOnlyBuffer_Limit(actual) == 10, "Expected initial limit to be 10.");

    parcReadOnlyBuffer_Release(&actual);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Wrap_NULL)
{
    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Create(parcBuffer_Wrap(NULL, 10, 0, 10));
    assertNull(actual, "Expected parcReadOnlyBuffer_Wrap to return NULL");
}

LONGBOW_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Wrap)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Create(buffer);
    assertTrue(parcReadOnlyBuffer_Position(actual) == 0, "Expected initial position to be 0.");
    assertTrue(parcReadOnlyBuffer_Limit(actual) == sizeof(array) / sizeof(array[0]), "Expected initial limit to be 10.");

    parcReadOnlyBuffer_Release(&actual);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Wrap_WithOffset)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 3, 10);

    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Create(buffer);
    parcBuffer_Release(&buffer);
    assertTrue(parcReadOnlyBuffer_Capacity(actual) == 10, "Expected initial capacity to be 3.");
    assertTrue(parcReadOnlyBuffer_Limit(actual) == 10, "Expected initial limit to be 3.");
    assertTrue(parcReadOnlyBuffer_Position(actual) == 3, "Expected initial position to be 0.");

    parcReadOnlyBuffer_Release(&actual);
}

LONGBOW_TEST_CASE(CreateDestroy, parcReadOnlyBuffer_Allocate_AcquireRelease)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);

    PARCReadOnlyBuffer *expected = parcReadOnlyBuffer_Create(buffer);
    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Acquire(expected);

    assertTrue(expected == actual, "Expected %p, actual %p", (void *) expected, (void *) actual);

    parcReadOnlyBuffer_Release(&expected);
    assertTrue(expected == NULL, "Expected parcReadOnlyBuffer_Release to NULL the pointer.");
    parcReadOnlyBuffer_Release(&actual);
    assertTrue(actual == NULL, "Expected parcReadOnlyBuffer_Release to NULL the pointer.");
    parcBuffer_Release(&buffer);
}

//LONGBOW_TEST_CASE_EXPECTS(CreateDestroy, parcReadOnlyBuffer_Allocate_AcquireRelease_TooMany, .event = &LongBowTrapIllegalValue)
//{
//    PARCBuffer *buffer = parcBuffer_Allocate(10);
//    PARCReadOnlyBuffer *expected = parcReadOnlyBuffer_Create(buffer);
//    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Acquire(expected);
//    PARCReadOnlyBuffer *alias = actual;
//
//    parcBuffer_Release(&buffer);
//    parcReadOnlyBuffer_Release(&expected);
//    parcReadOnlyBuffer_Release(&actual);
//    parcReadOnlyBuffer_Release(&alias); // this must fail.
//}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Array);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_ArrayOffset);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Clear);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Flip);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_GetByte);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_GetArray);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_HasRemaining);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Mark);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Overlay);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Position);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Remaining);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Rewind);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_SetLimit);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_SetLimit_TruncatePosition);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_SetPosition);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_ToString);
    LONGBOW_RUN_TEST_CASE(Global, parcReadOnlyBuffer_Display);
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

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Equals)
{
    PARCReadOnlyBuffer *x = parcReadOnlyBuffer_Wrap((uint8_t [10])  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);
    PARCReadOnlyBuffer *y = parcReadOnlyBuffer_Wrap((uint8_t [10])  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);
    PARCReadOnlyBuffer *z = parcReadOnlyBuffer_Wrap((uint8_t [10])  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);
    PARCReadOnlyBuffer *u1 = parcReadOnlyBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }, 10, 0, 10);
    PARCReadOnlyBuffer *u2 = parcReadOnlyBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 9, 0, 9);
    PARCReadOnlyBuffer *u3 = parcReadOnlyBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9);
    PARCReadOnlyBuffer *u4 = parcReadOnlyBuffer_SetPosition(parcReadOnlyBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9), 2);
    PARCReadOnlyBuffer *u5 = parcReadOnlyBuffer_SetPosition(parcReadOnlyBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9), 9);
    PARCReadOnlyBuffer *u6 = parcReadOnlyBuffer_SetPosition(parcReadOnlyBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9), 9);

    parcObjectTesting_AssertEqualsFunction(parcReadOnlyBuffer_Equals, x, y, z, u1, u2, u3, u4, u5, u6, NULL);

    parcReadOnlyBuffer_Release(&x);
    parcReadOnlyBuffer_Release(&y);
    parcReadOnlyBuffer_Release(&z);
    parcReadOnlyBuffer_Release(&u1);
    parcReadOnlyBuffer_Release(&u2);
    parcReadOnlyBuffer_Release(&u3);
    parcReadOnlyBuffer_Release(&u4);
    parcReadOnlyBuffer_Release(&u5);
    parcReadOnlyBuffer_Release(&u6);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Array)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *readWriteBuffer = parcBuffer_Wrap(expected, 10, 0, 10);
    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Create(readWriteBuffer);
    parcBuffer_Release(&readWriteBuffer);

    PARCByteArray *array = parcReadOnlyBuffer_Array(buffer);
    uint8_t *actual = parcByteArray_Array(array);

    parcReadOnlyBuffer_Release(&buffer);

    assertTrue(expected == actual,
               "Expected %p, actual %p",
               (void *) expected, (void *) actual);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Flip)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_PutArray(parcBuffer_Allocate(10), 10, expected);

    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Create(buffer);

    parcReadOnlyBuffer_Flip(actual);
    assertTrue(parcReadOnlyBuffer_Position(actual) == 0,
               "Expected position to be 0.");
    assertTrue(parcReadOnlyBuffer_Limit(actual) == 10,
               "Expected limit to be 10.");

    parcReadOnlyBuffer_Release(&actual);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Copy)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_PutArray(parcBuffer_Allocate(10), 10, expected);

    PARCReadOnlyBuffer *original = parcReadOnlyBuffer_Create(buffer);

    PARCReadOnlyBuffer *copy = parcReadOnlyBuffer_Copy(original);

    assertTrue(parcReadOnlyBuffer_Equals(original, copy), "Expected the copy to be equal to the original.");

    parcReadOnlyBuffer_Release(&copy);
    parcReadOnlyBuffer_Release(&original);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Clear)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_PutArray(parcBuffer_Allocate(10), 10, expected);

    PARCReadOnlyBuffer *actual = parcReadOnlyBuffer_Create(buffer);
    assertTrue(parcReadOnlyBuffer_Position(actual) == 10, "Expected position to be 10.");
    assertTrue(parcReadOnlyBuffer_Limit(actual) == 10, "Expected limit to be 10.");

    parcReadOnlyBuffer_Clear(actual);
    assertTrue(parcReadOnlyBuffer_Position(actual) == 0, "Expected position to be 0.");
    assertTrue(parcReadOnlyBuffer_Limit(actual) == 10, "Expected limit to be 10.");

    parcBuffer_Release(&buffer);
    parcReadOnlyBuffer_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_ArrayOffset)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    size_t expected = 5;
    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, expected, 10);

    size_t actual = parcReadOnlyBuffer_ArrayOffset(buffer);
    parcReadOnlyBuffer_Release(&buffer);

    assertTrue(0 == actual,
               "Expected offset to be 0, actual %zu", actual);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Position)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 5;
    parcReadOnlyBuffer_SetPosition(buffer, expected);

    size_t actual = parcReadOnlyBuffer_Position(buffer);

    assertTrue(expected == actual,
               "Expected position to be 0, actual %zu", actual);
    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Overlay)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t expected[5] = { 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    size_t position = 5;
    parcReadOnlyBuffer_SetPosition(buffer, position);
    uint8_t *actual = parcReadOnlyBuffer_Overlay(buffer, sizeof(array) - position);

    assertTrue(memcmp(expected, actual, sizeof(expected)) == 0,
               "Array contents should not be different.");
    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_SetPosition)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 2;
    parcReadOnlyBuffer_SetPosition(buffer, expected);
    size_t actual = parcReadOnlyBuffer_Position(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);

    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_SetLimit)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 2;
    parcReadOnlyBuffer_SetLimit(buffer, expected);
    size_t actual = parcReadOnlyBuffer_Limit(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);

    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_SetLimit_TruncatePosition)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    parcReadOnlyBuffer_SetPosition(buffer, 5);
    parcReadOnlyBuffer_Mark(buffer);

    size_t expected = 2;
    parcReadOnlyBuffer_SetLimit(buffer, expected);
    size_t actual = parcReadOnlyBuffer_Limit(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Remaining)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 10;
    size_t actual = parcReadOnlyBuffer_Remaining(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_HasRemaining)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);
    bool actual = parcReadOnlyBuffer_HasRemaining(buffer);

    assertTrue(actual, "Expected true");
    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Rewind)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);
    parcReadOnlyBuffer_SetPosition(buffer, 4);
    size_t actual = parcReadOnlyBuffer_Position(buffer);
    assertTrue(actual == 4, "Expected position to be at 4.");

    parcReadOnlyBuffer_Rewind(buffer);

    actual = parcReadOnlyBuffer_Position(buffer);
    assertTrue(actual == 0, "Expected position to be at 0.");
    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Mark)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 2;
    parcReadOnlyBuffer_SetPosition(buffer, expected);
    parcReadOnlyBuffer_Mark(buffer);
    parcReadOnlyBuffer_SetPosition(buffer, 4);
    parcReadOnlyBuffer_Reset(buffer);
    size_t actual = parcReadOnlyBuffer_Position(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_GetByte)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    uint8_t actual = parcReadOnlyBuffer_GetUint8(buffer);

    assertTrue(array[0] == actual,
               "Expected %d, actual %d", array[0], actual);

    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_GetArray)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    uint8_t actual[10];

    parcReadOnlyBuffer_GetArray(buffer, actual, sizeof(actual));

    assertTrue(memcmp(array, actual, sizeof(actual)) == 0,
               "Expected arrays to be equal.");

    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_HashCode)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCBuffer *referenceBuffer = parcBuffer_Wrap(array, 10, 0, 10);

    PARCReadOnlyBuffer *buffer1 = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    PARCReadOnlyBuffer *buffer2 = parcReadOnlyBuffer_Wrap(array, 10, 0, 10);

    uint32_t hashX = parcReadOnlyBuffer_HashCode(buffer1);
    uint32_t hashY = parcReadOnlyBuffer_HashCode(buffer2);
    uint32_t referenceHash = parcBuffer_HashCode(referenceBuffer);

    assertTrue(hashX == hashY, "Expected %u, actual %u", hashX, hashY);
    assertTrue(hashX == referenceHash, "Expected %u, actual %u", hashX, hashY);

    parcReadOnlyBuffer_Release(&buffer2);
    parcReadOnlyBuffer_Release(&buffer1);
    parcBuffer_Release(&referenceBuffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_ToString)
{
    uint8_t array[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 'x' };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, sizeof(array), 0, sizeof(array));

    char *actual = parcReadOnlyBuffer_ToString(buffer);

    assertTrue(strcmp("hello worldx", actual) == 0, "Expected 'hello world', actual %s", actual);

    parcMemory_Deallocate((void **) &actual);

    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcReadOnlyBuffer_Display)
{
    uint8_t array[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 'x' };

    PARCReadOnlyBuffer *buffer = parcReadOnlyBuffer_Wrap(array, sizeof(array), 0, sizeof(array));

    parcReadOnlyBuffer_Display(buffer, 0);

    parcReadOnlyBuffer_Release(&buffer);
}

LONGBOW_TEST_FIXTURE(Getters)
{
    LONGBOW_RUN_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint8);
    LONGBOW_RUN_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint16);
    LONGBOW_RUN_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint32);
    LONGBOW_RUN_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint64);
    LONGBOW_RUN_TEST_CASE(Getters, parcReadOnlyBuffer_GetAtIndex);
}

LONGBOW_TEST_FIXTURE_SETUP(Getters)
{
    PARCBuffer *buffer = parcBuffer_Allocate(100);

    longBowTestCase_SetClipBoardData(testCase, buffer);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Getters)
{
    PARCBuffer *buffer = longBowTestCase_GetClipBoardData(testCase);
    parcBuffer_Release(&buffer);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Getters, parcReadOnlyBuffer_GetAtIndex)
{
    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(uint8_t));
    uint8_t expected = 0x12;

    parcBuffer_PutUint8(buffer, expected);
    parcBuffer_Flip(buffer);

    PARCReadOnlyBuffer *readOnly = parcReadOnlyBuffer_Create(buffer);
    uint8_t actual = parcReadOnlyBuffer_GetAtIndex(readOnly, 0);

    parcReadOnlyBuffer_Release(&readOnly);
    parcBuffer_Release(&buffer);
    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint8)
{
    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(uint8_t));
    uint8_t expected = 0x12;

    parcBuffer_PutUint8(buffer, expected);
    parcBuffer_Flip(buffer);

    PARCReadOnlyBuffer *readOnly = parcReadOnlyBuffer_Create(buffer);
    uint8_t actual = parcReadOnlyBuffer_GetUint8(readOnly);

    parcReadOnlyBuffer_Release(&readOnly);
    parcBuffer_Release(&buffer);
    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint16)
{
    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(uint16_t));
    uint16_t expected = 0x1234;

    parcBuffer_PutUint16(buffer, expected);
    parcBuffer_Flip(buffer);

    PARCReadOnlyBuffer *readOnly = parcReadOnlyBuffer_Create(buffer);
    uint16_t actual = parcReadOnlyBuffer_GetUint16(readOnly);

    parcReadOnlyBuffer_Release(&readOnly);
    parcBuffer_Release(&buffer);
    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint32)
{
    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(uint32_t));
    uint32_t expected = 0x12345678;

    parcBuffer_PutUint32(buffer, expected);
    parcBuffer_Flip(buffer);

    PARCReadOnlyBuffer *readOnly = parcReadOnlyBuffer_Create(buffer);
    uint32_t actual = parcReadOnlyBuffer_GetUint32(readOnly);

    parcReadOnlyBuffer_Release(&readOnly);
    parcBuffer_Release(&buffer);
    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(Getters, parcReadOnlyBuffer_GetUint64)
{
    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(uint64_t));
    uint64_t expected = 0x1234567812345678;

    parcBuffer_PutUint64(buffer, expected);
    parcBuffer_Flip(buffer);

    PARCReadOnlyBuffer *readOnly = parcReadOnlyBuffer_Create(buffer);
    uint64_t actual = parcReadOnlyBuffer_GetUint64(readOnly);

    parcReadOnlyBuffer_Release(&readOnly);
    parcBuffer_Release(&buffer);
    assertTrue(expected == actual, "Expected %" PRIu64 ", actual %" PRIu64 "", expected, actual);
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, parcReadOnlyBuffer_GetByte_Underflow);
    LONGBOW_RUN_TEST_CASE(Errors, parcReadOnlyBuffer_Mark_mark_exceeds_position);
}

typedef struct parc_buffer_longbow_clipboard {
    PARCReadOnlyBuffer *buffer;
} parcReadOnlyBuffer_LongBowClipBoard;

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    parcReadOnlyBuffer_LongBowClipBoard *testData = calloc(1, sizeof(parcReadOnlyBuffer_LongBowClipBoard));
    testData->buffer = parcReadOnlyBuffer_Wrap(array, sizeof(array), 0, sizeof(array));

    longBowTestCase_SetClipBoardData(testCase, testData);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    parcReadOnlyBuffer_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    parcReadOnlyBuffer_Release(&testData->buffer);
    free(testData);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcReadOnlyBuffer_GetByte_Underflow, .event = &LongBowTrapOutOfBounds)
{
    parcReadOnlyBuffer_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCReadOnlyBuffer *buffer = testData->buffer;

    parcReadOnlyBuffer_SetPosition(buffer, 10);
    parcReadOnlyBuffer_GetUint8(buffer); // this will fail.
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcReadOnlyBuffer_Mark_mark_exceeds_position, .event = &LongBowAssertEvent)
{
    parcReadOnlyBuffer_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCReadOnlyBuffer *buffer = testData->buffer;

    size_t expected = 2;
    parcReadOnlyBuffer_SetPosition(buffer, expected);
    parcReadOnlyBuffer_Mark(buffer);
    parcReadOnlyBuffer_SetPosition(buffer, 0);
    parcReadOnlyBuffer_Reset(buffer);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_ReadableBuffer);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
