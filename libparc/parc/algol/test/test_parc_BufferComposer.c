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
#include "../parc_BufferComposer.c"

#include <inttypes.h>
#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

typedef struct {
    uint32_t setupAllocations;
    PARCBufferComposer *composer;
} TestData;

static TestData*
commonSetup()
{
    uint32_t outstanding = parcSafeMemory_Outstanding();
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear of %zu bytes returned NULL.", sizeof(TestData));
    data->setupAllocations = outstanding;
    data->composer = parcBufferComposer_Create();
    return data;
}

static uint32_t
commonTearDown(TestData *data)
{
    uint32_t setupAllocations = data->setupAllocations;

    // NOTE: The `parcBufferComposer_AssertValid_IncrementSize` invalidates this object, so we must
    // restore it to a good state in order for all memory to be released correctly.
    (data->composer)->incrementHeuristic = sizeof(void *);

    parcBufferComposer_Release(&(data->composer));
    parcMemory_Deallocate((void **) &data);
    return parcSafeMemory_ReportAllocation(STDOUT_FILENO) - setupAllocations;
}

LONGBOW_TEST_RUNNER(parc_BufferComposer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_BufferComposer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_BufferComposer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_Allocate);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_Allocate_SizeMax);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_AssertValid);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_AssertValid_NULL);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_AssertValid_IncrementSize);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutArray);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutUint16);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutUint32);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutUint64);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutUint8);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutString);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutStrings);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_Format);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutChar);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_GetBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_CreateBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_ProduceBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_PutString_Extend);
    LONGBOW_RUN_TEST_CASE(Global, parcBufferComposer_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = commonSetup();
    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    uint32_t outstandingAllocations = commonTearDown(data);

    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_Acquire)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    PARCBufferComposer *handle = parcBufferComposer_Acquire(composer);

    assertNotNull(handle, "Acquired PARCBufferComposer handle should be non-NULL.");
    assertTrue(parcBufferComposer_Equals(composer, handle), "PARCBufferComposer instances should be equal");

    parcBufferComposer_Release(&handle);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_Allocate)
{
    size_t size = 10;
    PARCBufferComposer *composer = parcBufferComposer_Allocate(size);
    PARCBuffer *buffer = parcBufferComposer_GetBuffer(composer);

    assertNotNull(composer, "PARCBufferComposer instance should be non-NULL.");
    assertTrue(parcBuffer_Limit(buffer) == size,
               "PARCBufferComposer instance's internal PARCBuffer limit must be %zu: %zu", size, parcBuffer_Limit(buffer));

    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_Allocate_SizeMax)
{
    size_t size = SIZE_MAX;
    PARCBufferComposer *composer = parcBufferComposer_Allocate(size);

    assertNull(composer, "PARCBufferComposer instance should be NULL.");
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_AssertValid)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    assertNotNull(composer, "PARCBufferComposer instance should be non-NULL.");
    parcBufferComposer_AssertValid(composer);

    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcBufferComposer_AssertValid_NULL, .event = &LongBowTrapIllegalValue)
{
    parcBufferComposer_AssertValid(NULL);
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcBufferComposer_AssertValid_IncrementSize, .event = &LongBowTrapIllegalValue)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    (data->composer)->incrementHeuristic = 0; // must be >= sizeof(void *)
    parcBufferComposer_AssertValid(data->composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_Create)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    assertNotNull(composer, "PARCBufferComposer instance should be non-NULL.");

    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_Equals)
{
    PARCBufferComposer *x = parcBufferComposer_Create();
    PARCBufferComposer *y = parcBufferComposer_Create();
    PARCBufferComposer *z = parcBufferComposer_Create();
    PARCBufferComposer *u = parcBufferComposer_Allocate(10);

    parcObjectTesting_AssertEqualsFunction(parcBufferComposer_Equals, x, y, z, u);

    u->incrementHeuristic = 0;
    assertFalse(parcBufferComposer_Equals(x, u), "PARCBufferComposer instances should be unequal due to size increments.");
    assertFalse(parcBufferComposer_Equals(x, NULL), "Equals should return false since the second parameter is NULL.");
    u->incrementHeuristic = sizeof(void *);

    parcBufferComposer_Release(&x);
    parcBufferComposer_Release(&y);
    parcBufferComposer_Release(&z);
    parcBufferComposer_Release(&u);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_ToString)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_Format(composer, "hello %s", "world");

    char *string = parcBufferComposer_ToString(composer);

    assertNotNull(string, "Expected non-NULL result from parcBufferComposer_ToString");

    parcMemory_Deallocate((void **) &string);

    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_GetBuffer)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    PARCBuffer *buffer = parcBufferComposer_GetBuffer(composer);

    assertNotNull(composer, "PARCBufferComposer instance should be non-NULL.");
    assertNotNull(buffer, "PARCBufferComposer instance's internal PARCBuffer should not be NULL");

    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutArray)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    uint8_t string[6] = { 'h', 'e', 'l', 'l', 'o', '\0' };
    parcBufferComposer_PutArray(composer, string, 6);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);

    char expected[5] = "hello";
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(expected, actual, 5) == 0, "Expected strings to match. Got %s, expected %s", actual, expected);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutBuffer)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    PARCBuffer *insertee = parcBuffer_WrapCString("hello world");
    parcBufferComposer_PutBuffer(composer, insertee);
    parcBuffer_Release(&insertee);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);

    char expected[11] = "hello world";
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(expected, actual, 11) == 0, "Expected strings to match. Got %s, expected %s", actual, expected);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutUint16)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    uint16_t val = 0x0101;
    parcBufferComposer_PutUint16(composer, val);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    assertTrue(parcBuffer_GetUint16(buffer) == val, "Expected inserted uint16_t to be equal to %x, got %x", val, parcBuffer_GetUint16(buffer));

    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutUint32)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    uint32_t val = 0x0101FFFF;
    parcBufferComposer_PutUint32(composer, val);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    assertTrue(parcBuffer_GetUint32(buffer) == val, "Expected inserted uint32_t to be equal to %x, got %x", val, parcBuffer_GetUint32(buffer));

    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutUint64)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    uint64_t val = 0x0101FFFFABCD0123;
    parcBufferComposer_PutUint64(composer, val);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    assertTrue(parcBuffer_GetUint64(buffer) == val,
               "Expected inserted uint64_t to be equal to %" PRIu64 ", got %" PRIu64 "", val, parcBuffer_GetUint64(buffer));

    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutUint8)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    uint8_t byte = 0x01;
    parcBufferComposer_PutUint8(composer, byte);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    assertTrue(parcBuffer_GetUint8(buffer) == byte, "Expected inserted byte to be equal to %x, got %x", byte, parcBuffer_GetUint8(buffer));

    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutString)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    char string[14] = "Hello, World!";
    parcBufferComposer_PutString(composer, string);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(actual, string, strlen(string)) == 0,
               "Expected inserted string to be equal to %s, got %s", string, actual);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutStrings)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    char string[14] = "Hello, World!";
    parcBufferComposer_PutStrings(composer, "Hello", ", ", "World!", NULL);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(actual, string, strlen(string)) == 0, "Expected inserted string to be equal to %s, got %s", string, actual);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_Format)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_Format(composer, "hello %s", "world");

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);

    char expected[11] = "hello world";
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(expected, actual, 11) == 0, "Expected strings to match. Got %s, expected %s", actual, expected);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutChar)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    char byte = 'a';
    parcBufferComposer_PutChar(composer, byte);

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);
    assertTrue(parcBuffer_GetUint8(buffer) == byte, "Expected inserted char to be equal to %c, got %c", byte, parcBuffer_GetUint8(buffer));

    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_CreateBuffer)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "hello world");

    PARCBuffer *buffer = parcBufferComposer_CreateBuffer(composer);
    parcBuffer_Flip(buffer); // flip the buffer since it was returned in write mode

    char expected[11] = "hello world";
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(expected, actual, 11) == 0, "Expected strings to match. Got %s, expected %s", actual, expected);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_ProduceBuffer)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "hello world");

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);

    char expected[11] = "hello world";
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(expected, actual, 11) == 0, "Expected strings to match. Got %s, expected %s", actual, expected);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, parcBufferComposer_PutString_Extend)
{
    PARCBufferComposer *composer = parcBufferComposer_Allocate(4);
    parcBufferComposer_PutString(composer, "hello world");

    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);

    char expected[11] = "hello world";
    char *actual = parcBuffer_ToString(buffer);
    assertTrue(strncmp(expected, actual, 11) == 0, "Expected strings to match. Got %s, expected %s", actual, expected);

    parcMemory_Deallocate((void **) &actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_BufferComposer);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
