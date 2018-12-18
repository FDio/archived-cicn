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
#include <config.h>
#include <inttypes.h>
#include <stdio.h>
#include <inttypes.h>

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Buffer.c"

LONGBOW_TEST_RUNNER(parcBuffer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateDestroy);
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(GettersSetters);
    LONGBOW_RUN_TEST_FIXTURE(CreateDestroyErrors);
    LONGBOW_RUN_TEST_FIXTURE(Errors);
    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parcBuffer)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parcBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateDestroy)
{
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_Allocate);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_Allocate_0);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_Allocate_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_Allocate_SIZE_MAX);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_Wrap);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_Wrap_NULL);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_Wrap_WithOffset);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcBuffer_AllocateCString);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateDestroy)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateDestroy)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_Allocate)
{
    PARCBuffer *actual = parcBuffer_Allocate(10);
    assertTrue(parcBuffer_Position(actual) == 0, "Expected initial position to be 0.");
    assertTrue(parcBuffer_Limit(actual) == 10, "Expected initial limit to be 10.");
    assertTrue(_markIsDiscarded(actual), "Expected initial mark to be discarded.");

    parcBuffer_Release(&actual);
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_Allocate_0)
{
    PARCBuffer *actual = parcBuffer_Allocate(0);
    assertTrue(parcBuffer_Position(actual) == 0, "Expected initial position to be 0.");
    assertTrue(parcBuffer_Limit(actual) == 0, "Expected initial limit to be 10.");
    assertTrue(_markIsDiscarded(actual), "Expected initial mark to be discarded.");

    parcBuffer_Release(&actual);
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_Allocate_SIZE_MAX)
{
    PARCBuffer *actual = parcBuffer_Allocate(SIZE_MAX);
    assertNull(actual, "Expected parcBuffer_Allocate to return NULL");
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_Wrap_NULL)
{
    PARCBuffer *actual = parcBuffer_Wrap(NULL, 10, 0, 10);
    assertNull(actual, "Expected parcBuffer_Wrap to return NULL");
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_Wrap)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *actual = parcBuffer_Wrap(array, 10, 0, 10);
    assertTrue(parcBuffer_Position(actual) == 0, "Expected initial position to be 0.");
    assertTrue(parcBuffer_Limit(actual) == sizeof(array) / sizeof(array[0]), "Expected initial limit to be 10.");
    assertTrue(_markIsDiscarded(actual), "Expected initial mark to be discarded.");

    parcBuffer_Release(&actual);
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_Wrap_WithOffset)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *actual = parcBuffer_Wrap(array, 10, 3, 10);
    assertTrue(parcBuffer_Capacity(actual) == 10, "Expected initial capacity to be 3.");
    assertTrue(parcBuffer_Limit(actual) == 10, "Expected initial limit to be 3.");
    assertTrue(parcBuffer_Position(actual) == 3, "Expected initial position to be 0.");
    assertTrue(_markIsDiscarded(actual), "Expected initial mark to be discarded.");

    parcBuffer_Release(&actual);
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_AllocateCString)
{
    PARCBuffer *buffer = parcBuffer_AllocateCString("Hello World");
    assertNotNull(buffer, "Expected parcBuffer_AllocateCString to return non-null value");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(CreateDestroy, parcBuffer_Allocate_AcquireRelease)
{
    PARCBuffer *expected = parcBuffer_Allocate(10);
    PARCBuffer *actual = parcBuffer_Acquire(expected);

    assertTrue(expected == actual, "Expected %p, actual %p", (void *) expected, (void *) actual);

    parcBuffer_Release(&expected);
    assertTrue(expected == NULL, "Expected parcBuffer_Release to NULL the pointer.");
    parcBuffer_Release(&actual);
    assertTrue(actual == NULL, "Expected parcBuffer_Release to NULL the pointer.");
}

LONGBOW_TEST_FIXTURE(CreateDestroyErrors)
{
    LONGBOW_RUN_TEST_CASE(CreateDestroyErrors, parcBuffer_Allocate_AcquireRelease_TooMany);
    LONGBOW_RUN_TEST_CASE(CreateDestroyErrors, parcBuffer_WrapByteArray_limit_exceeds_capacity);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateDestroyErrors)
{
    PARCByteArray *array = parcByteArray_Allocate(10);
    longBowTestCase_SetClipBoardData(testCase, array);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateDestroyErrors)
{
    PARCByteArray *array = longBowTestCase_GetClipBoardData(testCase);
    parcByteArray_Release(&array);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(CreateDestroyErrors, parcBuffer_Allocate_AcquireRelease_TooMany, .event = &LongBowTrapIllegalValue)
{
    PARCBuffer *expected = parcBuffer_Allocate(10);
    PARCBuffer *actual = parcBuffer_Acquire(expected);
    PARCBuffer *alias = actual;

    parcBuffer_Release(&expected);
    parcBuffer_Release(&actual);
    parcBuffer_Release(&alias); // this must fail.
}

LONGBOW_TEST_CASE_EXPECTS(CreateDestroyErrors, parcBuffer_WrapByteArray_limit_exceeds_capacity, .event = &LongBowAssertEvent)
{
    PARCByteArray *array = longBowTestCase_GetClipBoardData(testCase);

    PARCBuffer *buffer = parcBuffer_WrapByteArray(array, 0, parcByteArray_Capacity(array) + 1);

    assertNotNull(buffer, "Expected NULL");
}

LONGBOW_TEST_FIXTURE(Global)
{
//    LONGBOW_RUN_TEST_CASE(Global, HELPME);
//    LONGBOW_RUN_TEST_CASE(Global, HELPME2);

    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Array);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_ArrayOffset);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Clear);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Clone);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Duplicate);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Clone_WithOffset);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Equals_ZeroLength);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Equals_Bug80);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Flip);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_GetByte);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_GetBytes);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_GetBytes_Incremental);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_HasRemaining);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_HashCode_ZeroRemaining);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Mark);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Resize_Growing);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Resize_Growing_AtLimit);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Resize_Shrinking);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Resize_Shrinking_AtLimit);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Resize_Example);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Resize_Slice);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Overlay);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Position);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_PutBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_PutBuffer_ZeroLength_operand);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_PutByte);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_PutBytes);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_PutIndex);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_PutUint16);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_PutCString);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Remaining);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Rewind);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_SetLimit);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_SetLimit_TruncatePosition);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_SetPosition);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_Slice);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_ToString);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_ToString_ZeroRemaining);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_SkipOver);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_SkipOver_NotFound);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_SkipTo_NotFound);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_SkipTo);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_FindUint8);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_FindUint8_NotFound);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_IsValid_True);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_ParseNumeric_Decimal);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_ParseNumeric_Hexadecimal);

    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_ParseHexString);
    LONGBOW_RUN_TEST_CASE(Global, parcBuffer_CreateFromArray);
}

static size_t _longBowGlobal_Global_outstanding;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    size_t allocationsLeaked = parcMemory_Outstanding() - _longBowGlobal_Global_outstanding;

    if (allocationsLeaked) {
        printf("%s leaks memory by %zd allocations\n", longBowTestCase_GetName(testCase), allocationsLeaked);
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }
    return result;
}

LONGBOW_TEST_CASE(Global, HELPME)
{
    uint8_t decodeBytes[] = { 0x00, 0x02, 0x00, 0x0A, 'b', 'r', 'a', 'n', 'd', 'y', 'w', 'i', 'n', 'e' };
    PARCBuffer *b1 = parcBuffer_Wrap(decodeBytes, sizeof(decodeBytes), 0, sizeof(decodeBytes));

    // b1 is a buffer wrapping a byte array.
    // This will take 2 allocations: 1 for the buffer and 1 for the wrapper around the byte array.

    PARCBuffer *s2 = parcBuffer_Slice(b1);

    // s2 is another buffer referencing the wrapper created in the original buffer.
    // This will increase the allocations by 1 for the buffer making it 3.

    // **** DO NOT RELEASE s2

    parcBuffer_Release(&b1);
    // This releases the b1 buffer, deallocating it.  The wrapper around the original byte array still has a reference to it from s2.
    // The number of allocations is reduced by 1, making it 2 (1 for s2, and 1 for the wrapper it references)

    assertTrue(parcMemory_Outstanding() == 2, "memory imbalance");

    parcBuffer_Release(&s2);

    assertTrue(parcMemory_Outstanding() == 0, "memory imbalance must be 0, actual %d", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, HELPME2)
{
    uint8_t decodeBytes[] = { 0x00, 0x02, 0x00, 0x0A, 'b', 'r', 'a', 'n', 'd', 'y', 'w', 'i', 'n', 'e' };
    PARCBuffer *b1 = parcBuffer_Allocate(sizeof(decodeBytes));
    // This will create a buffer, a wrapper, and an allocated array of bytes to wrap.
    // The number of allocations is 3.

    parcBuffer_PutArray(b1, sizeof(decodeBytes), decodeBytes);
    parcBuffer_Flip(b1);

    PARCBuffer *s2 = parcBuffer_Slice(b1);
    // The number of allocations is 4.

    // **** DO NOT RELEASE s2

    parcBuffer_Release(&b1);
    // The number of allocations is now 3, the slice buffer, the wrapper, and the allocated array of bytes.

    // This will now correctly assert
    assertTrue(parcMemory_Outstanding() == 3, "memory imbalance");

    parcBuffer_Release(&s2);
    assertTrue(parcMemory_Outstanding() == 0, "memory imbalance");
}

LONGBOW_TEST_CASE(Global, parcBuffer_Equals)
{
    PARCBuffer *x = parcBuffer_Wrap((uint8_t [10])  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);
    PARCBuffer *y = parcBuffer_Wrap((uint8_t [10])  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);
    PARCBuffer *z = parcBuffer_Wrap((uint8_t [10])  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);

    // _Pragma("GCC diagnostic push")
    // _Pragma("GCC diagnostic ignored \"-Wzero-length-array\"")
    // _Pragma("GCC diagnostic ignored \"-Wgnu-empty-initializer\"")
    struct hack {
        uint8_t dummy;
        uint8_t empty[];
    };
    struct hack h = { 0 };
    PARCBuffer *u0 = parcBuffer_Wrap(h.empty, 0, 0, 0);
    // _Pragma("GCC diagnostic pop")

    PARCBuffer *u1 = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }, 10, 0, 10);
    PARCBuffer *u2 = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 9, 0, 9);
    PARCBuffer *u3 = parcBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9);
    PARCBuffer *u4 = parcBuffer_SetPosition(parcBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9), 2);
    PARCBuffer *u5 = parcBuffer_SetPosition(parcBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9), 9);
    PARCBuffer *u6 = parcBuffer_SetPosition(parcBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, 9, 0, 9), 9);
    PARCBuffer *u7 = parcBuffer_Wrap((uint8_t [9])  { 0 }, 0, 0, 0);

    parcObjectTesting_AssertEqualsFunction(parcBuffer_Equals, x, y, z, u0, u1, u2, u3, u4, u5, u6, u7, NULL);

    parcBuffer_Release(&x);
    parcBuffer_Release(&y);
    parcBuffer_Release(&z);
    parcBuffer_Release(&u0);
    parcBuffer_Release(&u1);
    parcBuffer_Release(&u2);
    parcBuffer_Release(&u3);
    parcBuffer_Release(&u4);
    parcBuffer_Release(&u5);
    parcBuffer_Release(&u6);
    parcBuffer_Release(&u7);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Equals_ZeroLength)
{
    // _Pragma("GCC diagnostic push")
    // _Pragma("GCC diagnostic ignored \"-Wzero-length-array\"")
    // _Pragma("GCC diagnostic ignored \"-Wgnu-empty-initializer\"")

    struct hack {
        uint8_t dummy;
        uint8_t empty[];
    };
    struct hack h = { 0 };
    PARCBuffer *x = parcBuffer_Wrap(h.empty, 0, 0, 0);
    PARCBuffer *y = parcBuffer_Wrap(h.empty, 0, 0, 0);
    PARCBuffer *z = parcBuffer_Wrap(h.empty, 0, 0, 0);

    // _Pragma("GCC diagnostic pop")

    PARCBuffer *u1 = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }, 10, 0, 10);
    PARCBuffer *u2 = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 9, 0, 9);

    parcObjectTesting_AssertEqualsFunction(parcBuffer_Equals, x, y, z, u1, u2, NULL);

    parcBuffer_Release(&x);
    parcBuffer_Release(&y);
    parcBuffer_Release(&z);
    parcBuffer_Release(&u1);
    parcBuffer_Release(&u2);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Equals_Bug80)
{
    PARCBuffer *x = parcBuffer_WrapCString("a");
    PARCBuffer *y = parcBuffer_WrapCString("a");
    PARCBuffer *z = parcBuffer_WrapCString("a");
    PARCBuffer *u1 = parcBuffer_WrapCString("b");
    PARCBuffer *u2 = parcBuffer_WrapCString("");
    PARCBuffer *u3 = parcBuffer_WrapCString("ab");

    parcObjectTesting_AssertEqualsFunction(parcBuffer_Equals, x, y, z, u1, u2, u3);

    parcBuffer_Release(&x);
    parcBuffer_Release(&y);
    parcBuffer_Release(&z);
    parcBuffer_Release(&u1);
    parcBuffer_Release(&u2);
    parcBuffer_Release(&u3);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Compare)
{
    PARCBuffer *x = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);
    PARCBuffer *y = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);

    PARCBuffer *equivalent[] = {
        x,
        y,
        NULL
    };
    PARCBuffer *lesser[] = {
        parcBuffer_Wrap((uint8_t [9])  { 0 }, 0,  0, 0),
        parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 8}, 10, 0, 10),
        parcBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 5, 7, 8,}, 9,  0, 9),
        NULL
    };
    PARCBuffer *greater[] = {
        parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }, 10, 0, 10),
        parcBuffer_Wrap((uint8_t [11]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 11, 0, 11),
        NULL
    };

    parcObjectTesting_AssertCompareTo(parcBuffer_Compare, x, equivalent, lesser, greater);

    parcBuffer_Release(&x);
    parcBuffer_Release(&y);

    for (int i = 0; lesser[i] != NULL; i++) {
        parcBuffer_Release(&lesser[i]);
    }
    for (int i = 0; greater[i] != NULL; i++) {
        parcBuffer_Release(&greater[i]);
    }
}

LONGBOW_TEST_CASE(Global, parcBuffer_Array)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(expected, 10, 0, 10);

    PARCByteArray *array = parcBuffer_Array(buffer);
    uint8_t *actual = parcByteArray_Array(array);

    parcBuffer_Release(&buffer);

    assertTrue(expected == actual,
               "Expected %p, actual %p", (void *) expected, (void *) actual);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Resize_Growing)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCBuffer *buffer = parcBuffer_Allocate(12);
    parcBuffer_PutArray(buffer, sizeof(expected), expected);

    parcBuffer_SetPosition(buffer, 5);
    parcBuffer_SetLimit(buffer, 11);
    parcBuffer_Mark(buffer);

    parcBuffer_Resize(buffer, 20);

    assertTrue(buffer->position == 5, "Expected position at %d, actual %zd", 5, buffer->position);
    assertTrue(buffer->mark == 5, "Expected mark at %d, actual %zd", 5, buffer->mark);
    assertTrue(buffer->limit == 11, "Expected limit at %d, actual %zd", 11, buffer->limit);
    assertTrue(parcBuffer_Capacity(buffer) == 20, "Expected capacity at %d, actual %zd", 20, parcBuffer_Capacity(buffer));

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Resize_Growing_AtLimit)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCBuffer *buffer = parcBuffer_Allocate(12);
    parcBuffer_PutArray(buffer, sizeof(expected), expected);

    parcBuffer_SetPosition(buffer, 5);
    parcBuffer_Mark(buffer);

    parcBuffer_Resize(buffer, 20);

    assertTrue(buffer->position == 5, "Expected position at %d, actual %zd", 5, buffer->position);
    assertTrue(buffer->mark == 5, "Expected mark at %d, actual %zd", 5, buffer->mark);
    assertTrue(buffer->limit == 20, "Expected limit at %d, actual %zd", 20, buffer->limit);
    assertTrue(parcBuffer_Capacity(buffer) == 20, "Expected capacity at %d, actual %zd", 20, parcBuffer_Capacity(buffer));

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Resize_Shrinking)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, sizeof(expected), expected);

    parcBuffer_SetPosition(buffer, 3);
    parcBuffer_SetLimit(buffer, 4);
    parcBuffer_Mark(buffer);

    parcBuffer_Resize(buffer, 5);

    assertTrue(buffer->position == 3, "Expected position at %d, actual %zd", 3, buffer->position);
    assertTrue(buffer->mark == 3, "Expected mark to be 3");
    assertTrue(buffer->limit == 4, "Expected limit at %d, actual %zd", 4, buffer->limit);
    assertTrue(parcBuffer_Capacity(buffer) == 5, "Expected capacity at %d, actual %zd", 5, parcBuffer_Capacity(buffer));

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Resize_Shrinking_AtLimit)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, sizeof(expected), expected);

    parcBuffer_SetPosition(buffer, 5);
    parcBuffer_SetLimit(buffer, 5);
    parcBuffer_Mark(buffer);

    parcBuffer_Resize(buffer, 3);

    assertTrue(buffer->position == 3, "Expected position at %d, actual %zd", 3, buffer->position);
    assertTrue(_markIsDiscarded(buffer), "Expected mark to be discarded");
    assertTrue(buffer->limit == 3, "Expected limit at %d, actual %zd", 3, buffer->limit);
    assertTrue(parcBuffer_Capacity(buffer) == 3, "Expected capacity at %d, actual %zd", 3, parcBuffer_Capacity(buffer));

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Resize_Example)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
    parcBuffer_Resize(buffer, 4);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Resize_Slice)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
    parcBuffer_SetPosition(buffer, 5);
    PARCBuffer *slice = parcBuffer_Slice(buffer);

    parcBuffer_Resize(slice, 4);

    parcBuffer_Release(&buffer);
    parcBuffer_Release(&slice);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Flip)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, 10, expected);
    parcBuffer_Flip(buffer);
    assertTrue(parcBuffer_Position(buffer) == 0, "Expected position to be 0.");
    assertTrue(parcBuffer_Limit(buffer) == 10, "Expected limit to be 10.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Clear)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, 10, expected);
    parcBuffer_Clear(buffer);
    assertTrue(parcBuffer_Position(buffer) == 0, "Expected position to be 0.");
    assertTrue(parcBuffer_Limit(buffer) == 10, "Expected limit to be 10.");
    assertTrue(buffer->mark >= parcBuffer_Capacity(buffer), "Expected the mark to be unset.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_ArrayOffset)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    size_t expected = 5;
    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, expected, 10);

    size_t actual = parcBuffer_ArrayOffset(buffer);
    parcBuffer_Release(&buffer);

    assertTrue(0 == actual,
               "Expected offset to be 0, actual %zd", actual);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Position)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 5;
    parcBuffer_SetPosition(buffer, expected);

    size_t actual = parcBuffer_Position(buffer);

    assertTrue(expected == actual,
               "Expected position to be 0, actual %zd", actual);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Overlay)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t expected[5] = { 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    size_t position = 5;
    parcBuffer_SetPosition(buffer, position);
    uint8_t *actual = parcBuffer_Overlay(buffer, sizeof(array) - position);

    assertTrue(memcmp(expected, actual, sizeof(expected)) == 0,
               "Array contents should not be different.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Clone)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *original = parcBuffer_Wrap(array, 10, 0, 10);

    PARCBuffer *clone = parcBuffer_Copy(original);

    assertTrue(clone != original, "Expected the clone to be a different instance.");

    assertTrue(parcBuffer_Equals(original, clone), "Expected clone to be equal to the original.");

    parcBuffer_Release(&original);
    assertNull(original, "Expected the parcBuffer_Release function to NULL the pointer.");

    parcBuffer_Release(&clone);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Clone_WithOffset)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *original = parcBuffer_Wrap(array, 10, 0, 10);
    parcBuffer_SetLimit(original, 9);
    parcBuffer_SetPosition(original, 1);
    PARCBuffer *range = parcBuffer_Slice(original);

    PARCBuffer *clone = parcBuffer_Copy(range);

    assertTrue(clone != original, "Expected the clone to be a different instance.");

    assertTrue(parcBuffer_Equals(range, clone), "Expected clone to be equal to the original.");

    parcBuffer_Release(&clone);
    parcBuffer_Release(&range);
    parcBuffer_Release(&original);
}

LONGBOW_TEST_CASE(Global, parcBuffer_SetPosition)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 2;
    parcBuffer_SetPosition(buffer, expected);
    size_t actual = parcBuffer_Position(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_SetLimit)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 2;
    parcBuffer_SetLimit(buffer, expected);
    size_t actual = parcBuffer_Limit(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_SetLimit_TruncatePosition)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    parcBuffer_SetPosition(buffer, 5);
    parcBuffer_Mark(buffer);

    size_t expected = 2;
    parcBuffer_SetLimit(buffer, expected);
    size_t actual = parcBuffer_Limit(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Slice)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);
    parcBuffer_GetUint8(buffer);

    PARCBuffer *actual = parcBuffer_Slice(buffer);
    assertTrue(parcBuffer_Position(actual) == 0,
               "Expected position to be 0");
    assertTrue(parcBuffer_Limit(actual) == parcBuffer_Remaining(buffer),
               "Expected position to be %zd", parcBuffer_Remaining(buffer));
    assertTrue(_markIsDiscarded(actual), "Expected the mark to be discarded.");

    parcBuffer_Release(&buffer);
    parcBuffer_Release(&actual);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Remaining)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 2;
    parcBuffer_SetLimit(buffer, expected);
    size_t actual = parcBuffer_Limit(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_HasRemaining)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);
    bool actual = parcBuffer_HasRemaining(buffer);

    assertTrue(actual, "Expected true");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Rewind)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);
    parcBuffer_SetPosition(buffer, 4);
    size_t actual = parcBuffer_Position(buffer);
    assertTrue(actual == 4, "Expected position to be at 4.");

    parcBuffer_Rewind(buffer);

    actual = parcBuffer_Position(buffer);
    assertTrue(actual == 0, "Expected position to be at 0.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Duplicate)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);
    parcBuffer_SetPosition(buffer, 4);

    PARCBuffer *buffer2 = parcBuffer_Duplicate(buffer);

    assertTrue(buffer != buffer2, "Expected distinct pointers to the different buffers.");
    assertTrue(parcBuffer_Position(buffer) == parcBuffer_Position(buffer2), "Expected equal position values.");
    assertTrue(parcBuffer_Limit(buffer) == parcBuffer_Limit(buffer2), "Expected equal limit values.");
    assertTrue(parcBuffer_Capacity(buffer) == parcBuffer_Capacity(buffer2), "Expected equal capacity values.");

    parcBuffer_Rewind(buffer);
    assertFalse(parcBuffer_Position(buffer) == parcBuffer_Position(buffer2), "Expected unequal position values.");

    parcBuffer_Release(&buffer);
    parcBuffer_Release(&buffer2);
}

LONGBOW_TEST_CASE(Global, parcBuffer_Mark)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);

    size_t expected = 2;
    parcBuffer_SetPosition(buffer, expected);
    parcBuffer_Mark(buffer);
    parcBuffer_SetPosition(buffer, 4);
    parcBuffer_Reset(buffer);
    size_t actual = parcBuffer_Position(buffer);

    assertTrue(expected == actual, "Expected %zd, actual %zd", expected, actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_PutByte)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);

    uint8_t expectedValue = 1;
    parcBuffer_PutUint8(buffer, expectedValue);

    size_t expectedPosition = 1;
    size_t actualPosition = parcBuffer_Position(buffer);

    parcBuffer_SetPosition(buffer, 0);
    uint8_t actualValue = parcBuffer_GetAtIndex(buffer, 0);
    parcBuffer_Release(&buffer);

    assertTrue(expectedValue == actualValue,
               "Expected %d, actual %d", expectedValue, actualValue);
    assertTrue(expectedPosition == actualPosition,
               "Expected %zu, actual %zu", expectedPosition, actualPosition);
}

LONGBOW_TEST_CASE(Global, parcBuffer_PutCString)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);

    char *expectedValue = "abcdefg";
    parcBuffer_PutCString(buffer, expectedValue);

    size_t expectedPosition = 8;
    size_t actualPosition = parcBuffer_Position(buffer);

    uint8_t zero = parcBuffer_GetAtIndex(buffer, 7);

    assertTrue(zero == 0, "Expected zero, actual %d", zero);

    assertTrue(expectedPosition == actualPosition,
               "Expected %zu, actual %zu", expectedPosition, actualPosition);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_PutUint16)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);

    uint16_t expectedValue = 0x1234;
    parcBuffer_PutUint16(buffer, expectedValue);

    size_t expectedPosition = 2;
    size_t actualPosition = parcBuffer_Position(buffer);
    assertTrue(expectedPosition == actualPosition,
               "Expected position %zd, actual %zd", expectedPosition, actualPosition);

    parcBuffer_Flip(buffer);
    uint16_t actualValue = parcBuffer_GetUint16(buffer);

    actualPosition = parcBuffer_Position(buffer);

    parcBuffer_Release(&buffer);

    assertTrue(expectedValue == actualValue,
               "Expected %d, actual %d", expectedValue, actualValue);
    assertTrue(expectedPosition == actualPosition,
               "Expected %zu, actual %zu", expectedPosition, actualPosition);
}

LONGBOW_TEST_CASE(Global, parcBuffer_PutIndex)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);

    uint8_t expected = 1;
    parcBuffer_PutAtIndex(buffer, 0, expected);
    uint8_t actual = parcBuffer_GetAtIndex(buffer, 0);

    parcBuffer_Release(&buffer);

    assertTrue(expected == actual,
               "Expected %" PRIu8 ", actual %" PRIu8 "", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcBuffer_PutBytes)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, 10, array);

    size_t expected = parcBuffer_Limit(buffer);
    size_t actual = parcBuffer_Position(buffer);

    assertTrue(expected == actual, "Expected position to be at the limit.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_PutBuffer)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer1 = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer1, 5, array);

    PARCBuffer *buffer2 = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer2, 5, &array[5]);
    parcBuffer_Flip(buffer2);

    parcBuffer_PutBuffer(buffer1, buffer2);

    size_t expected = parcBuffer_Limit(buffer1);
    size_t actual = parcBuffer_Position(buffer1);

    assertTrue(expected == actual, "Expected position to be at the limit. Expected %zd, actual %zd", expected, actual);
    assertTrue(memcmp(array, parcByteArray_Array(buffer1->array), sizeof(array)) == 0,
               "Array content differs.");

    parcBuffer_Release(&buffer1);
    parcBuffer_Release(&buffer2);
}

LONGBOW_TEST_CASE(Global, parcBuffer_GetByte)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, sizeof(expected), expected);
    parcBuffer_Flip(buffer);

    uint8_t actual = parcBuffer_GetUint8(buffer);

    assertTrue(expected[0] == actual,
               "Expected %d, actual %d", expected[0], actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_GetBytes)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t actual[10];

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, sizeof(expected), expected);
    parcBuffer_Flip(buffer);

    parcBuffer_GetBytes(buffer, sizeof(actual), actual);

    assertTrue(memcmp(expected, actual, sizeof(actual)) == 0,
               "Expected arrays to be equal.");

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_GetBytes_Incremental)
{
    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t actual[10];

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer, sizeof(expected), expected);
    parcBuffer_Flip(buffer);

    parcBuffer_GetBytes(buffer, 1, actual);
    assertTrue(parcBuffer_Position(buffer) == 1, "Expected position to be %d\n", 1);
    assertTrue(actual[0] == expected[0], "Expected %d, actual %d", expected[0], actual[0]);
    parcBuffer_GetBytes(buffer, 1, actual);
    assertTrue(parcBuffer_Position(buffer) == 2, "Expected position to be 2, actual %zd\n", parcBuffer_Position(buffer));
    assertTrue(actual[0] == expected[1], "Expected %d, actual %d", expected[1], actual[0]);
    parcBuffer_GetBytes(buffer, 1, actual);
    assertTrue(parcBuffer_Position(buffer) == 3, "Expected position to be 3, actual %zd\n", parcBuffer_Position(buffer));
    assertTrue(actual[0] == expected[2], "Expected %d, actual %d", expected[2], actual[0]);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_PutBuffer_ZeroLength_operand)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer1 = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer1, 10, array);

    PARCBuffer *buffer2 = parcBuffer_Allocate(0);
    parcBuffer_PutBuffer(buffer1, buffer2);

    size_t expected = parcBuffer_Limit(buffer1);
    size_t actual = parcBuffer_Position(buffer1);

    assertTrue(expected == actual, "Expected position to be at the limit.");

    parcBuffer_Release(&buffer1);
    parcBuffer_Release(&buffer2);
}

LONGBOW_TEST_CASE(Global, parcBuffer_HashCode)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer1 = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer1, 10, array);
    parcBuffer_Flip(buffer1);

    PARCBuffer *buffer2 = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer2, 10, array);
    parcBuffer_Flip(buffer2);

    PARCHashCode hashX = parcBuffer_HashCode(buffer1);
    PARCHashCode hashY = parcBuffer_HashCode(buffer2);

    assertTrue(hashX == hashY, "Expected %" PRIPARCHashCode ", actual %" PRIPARCHashCode, hashX, hashY);

    parcBuffer_Release(&buffer1);
    parcBuffer_Release(&buffer2);
}

LONGBOW_TEST_CASE(Global, parcBuffer_HashCode_ZeroRemaining)
{
    uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    PARCBuffer *buffer1 = parcBuffer_Allocate(10);
    parcBuffer_PutArray(buffer1, 10, array);

    PARCHashCode hashX = parcBuffer_HashCode(buffer1);

    assertTrue(hashX == 0, "Expected 0, actual %" PRIPARCHashCode, hashX);

    parcBuffer_Release(&buffer1);
}

LONGBOW_TEST_CASE(Global, parcBuffer_ToString)
{
    uint8_t array[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 'x' };

    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(array) - 1);
    parcBuffer_PutArray(buffer, sizeof(array) - 1, array);
    parcBuffer_Flip(buffer);

    char *actual = parcBuffer_ToString(buffer);

    assertTrue(strcmp("hello world", actual) == 0, "Expected 'hello world', actual %s", actual);

    parcMemory_Deallocate((void **) &actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_ToString_ZeroRemaining)
{
    uint8_t array[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 'x' };

    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(array) - 1);
    parcBuffer_PutArray(buffer, sizeof(array) - 1, array);
//    parcBuffer_Flip(buffer);

    char *actual = parcBuffer_ToString(buffer);

    assertTrue(strcmp("", actual) == 0, "Expected '', actual %s", actual);

    parcMemory_Deallocate((void **) &actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_SkipOver)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
    uint8_t skipOverBytes[] = { 'H', 0 };

    bool actual = parcBuffer_SkipOver(buffer, 1, skipOverBytes);

    assertTrue(actual, "Expected parcBuffer_SkipOver to return true.");

    uint8_t peekByte = parcBuffer_PeekByte(buffer);

    assertTrue(peekByte == 'e', "Expected buffer to point to 'e', actual '%c'", peekByte);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_SkipOver_NotFound)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");

    bool actual = parcBuffer_SkipOver(buffer, 8, (uint8_t *) "Helo Wrd");

    assertFalse(actual, "Expected parcBuffer_SkipOver to return false.");

    assertTrue(parcBuffer_Remaining(buffer) == 0,
               "Expected buffer to have no remaining bytes. Actual %zd", parcBuffer_Remaining(buffer));

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_SkipTo)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
    uint8_t skipToBytes[] = { 'l', 0 };

    bool actual = parcBuffer_SkipTo(buffer, 1, skipToBytes);

    assertTrue(actual, "Expected parcBuffer_SkipOver to return true.");

    uint8_t peekByte = parcBuffer_PeekByte(buffer);

    assertTrue(peekByte == 'l', "Expected buffer to point to 'l', actual '%c'", peekByte);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_SkipTo_NotFound)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");

    bool actual = parcBuffer_SkipTo(buffer, 1, (uint8_t *) "x");

    assertFalse(actual, "Expected parcBuffer_SkipOver to return false.");
    assertTrue(parcBuffer_Remaining(buffer) == 0,
               "Expected buffer to have no remaining bytes. Actual %zd", parcBuffer_Remaining(buffer));

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_FindUint8)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
    size_t index = parcBuffer_FindUint8(buffer, 'e');
    assertTrue(index == 1, "Expected index to be 1, actual %zu", index);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_FindUint8_NotFound)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
    size_t index = parcBuffer_FindUint8(buffer, 'z');
    assertTrue(index == SIZE_MAX, "Expected index to be SIZE_MAX, actual %zu", index);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_IsValid_True)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
    bool actual = parcBuffer_IsValid(buffer);
    assertTrue(actual, "Expected PARCBuffer to be valid");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_ParseNumeric_Decimal)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("123abc");

    uint64_t actual = parcBuffer_ParseNumeric(buffer);

    assertTrue(actual == 123, "Expected 123, actual %" PRIu64 "", actual);
    assertTrue(parcBuffer_Position(buffer) == 3, "Expected position to be 3, actual %zd", parcBuffer_Position(buffer));
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_ParseNumeric_Hexadecimal)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("0x123xyz");

    uint64_t actual = parcBuffer_ParseNumeric(buffer);

    assertTrue(actual == 0x123, "Expected 0x123, actual %" PRIx64 "", actual);
    assertTrue(parcBuffer_Position(buffer) == 5, "Expected position to be 5, actual %zd", parcBuffer_Position(buffer));
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_ParseHexString)
{
    char *expected = "00";
    PARCBuffer *buffer = parcBuffer_ParseHexString("3030");
    parcBuffer_Flip(buffer);
    char *actual = parcBuffer_ToString(buffer);

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);

    parcMemory_Deallocate(&actual);

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, parcBuffer_CreateFromArray)
{
    char *expected = "0123456789ABCDEF";
    PARCBuffer *buffer = parcBuffer_CreateFromArray(expected, strlen(expected));

    assertTrue(parcBuffer_Position(buffer) == 16, "Expected position to be at 15, actual %zd", parcBuffer_Position(buffer));

    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_FIXTURE(GettersSetters)
{
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcPutGetUint8);
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcPutGetUint16);
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcPutGetUint32);
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcPutGetUint64);
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcBuffer_ToHexString);
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcBuffer_ToHexString_NULLBuffer);
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcBuffer_Display);
    LONGBOW_RUN_TEST_CASE(GettersSetters, parcBuffer_Display_NULL);
}

LONGBOW_TEST_FIXTURE_SETUP(GettersSetters)
{
    PARCBuffer *buffer = parcBuffer_Allocate(100);

    longBowTestCase_SetClipBoardData(testCase, buffer);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(GettersSetters)
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

LONGBOW_TEST_CASE(GettersSetters, parcPutGetUint8)
{
    PARCBuffer *buffer = longBowTestCase_GetClipBoardData(testCase);

    uint8_t expected = 0x12;
    parcBuffer_PutUint8(buffer, expected);
    parcBuffer_Flip(buffer);
    uint8_t actual = parcBuffer_GetUint8(buffer);

    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(GettersSetters, parcPutGetUint16)
{
    PARCBuffer *buffer = longBowTestCase_GetClipBoardData(testCase);

    uint16_t expected = 0x1234;
    parcBuffer_PutUint16(buffer, expected);
    parcBuffer_Flip(buffer);
    uint16_t actual = parcBuffer_GetUint16(buffer);

    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(GettersSetters, parcPutGetUint32)
{
    PARCBuffer *buffer = longBowTestCase_GetClipBoardData(testCase);

    uint32_t expected = 0x12345678;
    parcBuffer_PutUint32(buffer, expected);
    parcBuffer_Flip(buffer);
    uint32_t actual = parcBuffer_GetUint32(buffer);

    assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
}

LONGBOW_TEST_CASE(GettersSetters, parcPutGetUint64)
{
    PARCBuffer *buffer = longBowTestCase_GetClipBoardData(testCase);

    uint64_t expected = 0x1234567812345678;
    parcBuffer_PutUint64(buffer, expected);
    parcBuffer_Flip(buffer);
    uint64_t actual = parcBuffer_GetUint64(buffer);

    assertTrue(expected == actual, "Expected %" PRIu64 ", actual %" PRIu64 "", expected, actual);
}

LONGBOW_TEST_CASE(GettersSetters, parcBuffer_ToHexString)
{
    PARCBuffer *buffer = longBowTestCase_GetClipBoardData(testCase);

    uint64_t expected = 0x1234567812345678;
    parcBuffer_PutUint64(buffer, expected);
    parcBuffer_Flip(buffer);
    char *hexString = parcBuffer_ToHexString(buffer);

    assertTrue(strcmp("1234567812345678", hexString) == 0, "Expected 1234567812345678, actual %s", hexString);
    parcMemory_Deallocate((void **) &hexString);
}

LONGBOW_TEST_CASE(GettersSetters, parcBuffer_ToHexString_NULLBuffer)
{
    char *hexString = parcBuffer_ToHexString(NULL);

    assertTrue(strcmp("null", hexString) == 0, "Expected null, actual %s", hexString);
    parcMemory_Deallocate((void **) &hexString);
}

LONGBOW_TEST_CASE(GettersSetters, parcBuffer_Display)
{
    PARCBuffer *buffer = longBowTestCase_GetClipBoardData(testCase);

    uint64_t expected = 0x1234567812345678;
    parcBuffer_PutUint64(buffer, expected);
    parcBuffer_Flip(buffer);
    parcBuffer_Display(buffer, 0);
}

LONGBOW_TEST_CASE(GettersSetters, parcBuffer_Display_NULL)
{
    parcBuffer_Display(NULL, 0);
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, parcBuffer_GetByte_Underflow);
    LONGBOW_RUN_TEST_CASE(Errors, parcBuffer_Mark_mark_exceeds_position);
}

typedef struct parc_buffer_longbow_clipboard {
    PARCBuffer *buffer;
} parcBuffer_LongBowClipBoard;

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    parcBuffer_LongBowClipBoard *testData = calloc(1, sizeof(parcBuffer_LongBowClipBoard));
    testData->buffer = parcBuffer_Allocate(10);

    longBowTestCase_SetClipBoardData(testCase, testData);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    parcBuffer_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    parcBuffer_Release(&testData->buffer);
    free(testData);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcBuffer_GetByte_Underflow, .event = &LongBowTrapOutOfBounds)
{
    parcBuffer_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = testData->buffer;

    uint8_t expected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    parcBuffer_PutArray(buffer, 1, expected);
    parcBuffer_Flip(buffer);

    parcBuffer_GetUint8(buffer);
    parcBuffer_GetUint8(buffer); // this will fail.
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcBuffer_Mark_mark_exceeds_position, .event = &LongBowAssertEvent)
{
    parcBuffer_LongBowClipBoard *testData = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *buffer = testData->buffer;

    size_t expected = 2;
    parcBuffer_SetPosition(buffer, expected);
    parcBuffer_Mark(buffer);
    parcBuffer_SetPosition(buffer, 0);
    parcBuffer_Reset(buffer);
}


LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, _digittoint);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    size_t allocationsLeaked = parcMemory_Outstanding() - _longBowGlobal_Global_outstanding;

    if (allocationsLeaked) {
        printf("%s leaks memory by %zd allocations\n", longBowTestCase_GetName(testCase), allocationsLeaked);
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }
    return result;
}

LONGBOW_TEST_CASE(Static, _digittoint)
{
    char *base10 = "0123456789";

    for (size_t i = 0; i < strlen(base10); i++) {
        int expected = (int) i;
        int actual = _digittoint(base10[i]);
        assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
    }

    char *base16 = "0123456789abcdef";

    for (size_t i = 0; i < strlen(base16); i++) {
        int expected = (int) i;
        int actual = _digittoint(base16[i]);
        assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
    }

    base16 = "0123456789ABCDEF";

    for (size_t i = 0; i < strlen(base16); i++) {
        int expected = (int) i;
        int actual = _digittoint(base16[i]);
        assertTrue(expected == actual, "Expected %d, actual %d", expected, actual);
    }
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcBuffer_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    size_t allocationsLeaked = parcMemory_Outstanding() - _longBowGlobal_Global_outstanding;

    if (allocationsLeaked) {
        printf("%s leaks memory by %zd allocations\n", longBowTestCase_GetName(testCase), allocationsLeaked);
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }
    return result;
}

LONGBOW_TEST_CASE(Performance, parcBuffer_Create)
{
    for (size_t i = 0; i < 1000000; i++) {
        PARCBuffer *buffer = parcBuffer_Allocate(1200);
        parcBuffer_Release(&buffer);
    }
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parcBuffer);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
