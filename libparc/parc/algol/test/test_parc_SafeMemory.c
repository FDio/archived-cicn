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
#include "../parc_SafeMemory.c"

#include <LongBow/unit-test.h>

#include <fcntl.h>

LONGBOW_TEST_RUNNER(safetyMemory)
{
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(ReportAllocation);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Errors);

    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

LONGBOW_TEST_RUNNER_SETUP(safetyMemory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(safetyMemory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, PARCSafeMemory_Report);
    LONGBOW_RUN_TEST_CASE(Static, _parcSafeMemory_StateToString);
    LONGBOW_RUN_TEST_CASE(Static, _parcSafeMemory_GetPrefixState_OK);
    LONGBOW_RUN_TEST_CASE(Static, _parcSafeMemory_GetPrefixState_ALREADYFREE);
    LONGBOW_RUN_TEST_CASE(Static, _parcSafeMemory_GetPrefixState_UNDERRUN);
    LONGBOW_RUN_TEST_CASE(Static, _parcSafeMemory_FormatPrefix);
    LONGBOW_RUN_TEST_CASE(Static, _computeUsableMemoryLength);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Static, PARCSafeMemory_Report)
{
    size_t expectedSize = 100;
    void *memory = parcSafeMemory_Allocate(expectedSize);
    int fd = open("/dev/null", O_WRONLY);
    _parcSafeMemory_Report(memory, fd);
    close(fd);
    parcSafeMemory_Deallocate(&memory);
}

LONGBOW_TEST_CASE(Static, _parcSafeMemory_StateToString)
{
    assertNotNull(_parcSafeMemory_StateToString(PARCSafeMemoryState_OK),
                  "PARCSafeMemoryState_OK cannot be a NULL string.");
    assertNotNull(_parcSafeMemory_StateToString(PARCSafeMemoryState_MISMATCHED),
                  "PARCSafeMemoryState_MISMATCHED cannot be a NULL string.");
    assertNotNull(_parcSafeMemory_StateToString(PARCSafeMemoryState_UNDERRUN),
                  "PARCSafeMemoryState_UNDERRUN cannot be a NULL string.");
    assertNotNull(_parcSafeMemory_StateToString(PARCSafeMemoryState_OVERRUN),
                  "PARCSafeMemoryState_OVERRUN cannot be a NULL string.");
    assertNotNull(_parcSafeMemory_StateToString(PARCSafeMemoryState_NOTHINGALLOCATED),
                  "PARCSafeMemoryState_NOTHINGALLOCATED cannot be a NULL string.");
    assertNotNull(_parcSafeMemory_StateToString(PARCSafeMemoryState_ALREADYFREE),
                  "PARCSafeMemoryState_ALREADYFREE cannot be a NULL string.");
    assertNotNull(_parcSafeMemory_StateToString(-1),
                  "Garbage cannot be represented by a NULL string.");
}

LONGBOW_TEST_CASE(Static, _parcSafeMemory_GetPrefixState_OK)
{
    size_t expectedLength = 5;
    int expectedAlignment = sizeof(void *);
    char origin[100]; // just some number

    void *memory = _parcSafeMemory_FormatPrefix((PARCSafeMemoryOrigin *) origin, expectedLength, expectedAlignment);
    PARCSafeMemoryState actual = _parcSafeMemory_GetPrefixState(memory);
    assertTrue(actual == PARCSafeMemoryState_OK,
               "Expected PARCSafeMemoryState_OK, actual = %d", actual);
}

LONGBOW_TEST_CASE(Static, _parcSafeMemory_GetPrefixState_ALREADYFREE)
{
    PARCSafeMemoryUsable *usable = parcSafeMemory_Allocate(10);
    PARCSafeMemoryUsable *saved = usable;

    parcSafeMemory_Deallocate((void **) &usable);

    PARCSafeMemoryState actual = _parcSafeMemory_GetPrefixState(saved);
    assertTrue(actual == PARCSafeMemoryState_ALREADYFREE,
               "Expected PARCSafeMemoryState_ALREADYFREE, actual = %d", actual);
}

LONGBOW_TEST_CASE(Static, _parcSafeMemory_GetPrefixState_UNDERRUN)
{
    char *usable = parcSafeMemory_Allocate(10);

    char savedByte = usable[-1];
    usable[-1] = 0;

    PARCSafeMemoryState actual = _parcSafeMemory_GetPrefixState((PARCSafeMemoryUsable *) usable);
    assertTrue(actual == PARCSafeMemoryState_UNDERRUN,
               "Expected PARCSafeMemoryState_UNDERRUN, actual = %d", actual);
    usable[-1] = savedByte;
    parcSafeMemory_Deallocate((void **) &usable);
}

LONGBOW_TEST_CASE(Static, _parcSafeMemory_FormatPrefix)
{
    size_t expectedLength = 5;
    int expectedAlignment = sizeof(void *) - 1;
    char base[100]; // just some number
    void *memory = _parcSafeMemory_FormatPrefix((PARCSafeMemoryOrigin *) base, expectedLength, expectedAlignment);

    assertNull(memory,
               "Expected _parcSafeMemory_FormatPrefix to return NULL for bad alignment specification.");
}

LONGBOW_TEST_CASE(Static, _computeUsableMemoryLength)
{
    size_t actual = _computeUsableMemoryLength(100, sizeof(void *));

    // The result must be >= to the requested length and an even multiple of sizeof(void *)
    assertTrue(actual >= 100 && (actual % sizeof(void *)) == 0,
               "Expected the result to be >= to the requested length and an even multiple of sizeof(void *)");
}

LONGBOW_TEST_FIXTURE(ReportAllocation)
{
    LONGBOW_RUN_TEST_CASE(ReportAllocation, parcSafeMemory_ReportAllocation_Empty);
    LONGBOW_RUN_TEST_CASE(ReportAllocation, parcSafeMemory_ReportAllocation_One);
    LONGBOW_RUN_TEST_CASE(ReportAllocation, parcSafeMemory_ReportAllocation_Deallocated);
}

LONGBOW_TEST_CASE(ReportAllocation, parcSafeMemory_ReportAllocation_Empty)
{
    _parcSafeMemory_DeallocateAll();
    int fd = open("/dev/null", O_WRONLY);
    size_t result = parcSafeMemory_ReportAllocation(fd);
    close(fd);
    assertTrue(result == 0, "Expected 0, was %zd", result);
}

LONGBOW_TEST_CASE(ReportAllocation, parcSafeMemory_ReportAllocation_One)
{
    void *memory;
    size_t size = 100;

    memory = parcSafeMemory_Allocate(size);

    int fd = open("/dev/null", O_WRONLY);
    size_t result = parcSafeMemory_ReportAllocation(fd);
    close(fd);
    assertTrue(result == 1, "Expected 1, was %zd", result);

    parcSafeMemory_Deallocate(&memory);
}

LONGBOW_TEST_CASE(ReportAllocation, parcSafeMemory_ReportAllocation_Deallocated)
{
    size_t size = 100;
    void *memory = parcSafeMemory_Allocate(size);
    assertTrue(parcSafeMemory_Outstanding() != 0, "No memory allocated!");
    PARCSafeMemoryState state = _parcSafeMemory_GetState(memory);
    parcSafeMemory_Deallocate(&memory);
    assertTrue(state == PARCSafeMemoryState_OK, "Expected uncorrupted memory.");

    int fd = open("/dev/null", O_WRONLY);
    size_t result = parcSafeMemory_ReportAllocation(fd);
    close(fd);

    assertTrue(result == 0, "Expected 0, was %zd", result);
}

LONGBOW_TEST_FIXTURE_SETUP(ReportAllocation)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(ReportAllocation)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Allocate);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_MemAlign);

    LONGBOW_RUN_TEST_CASE(Global, PARCSafeMemory_Realloc_Larger);
    LONGBOW_RUN_TEST_CASE(Global, PARCSafeMemory_Realloc_Smaller);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Reallocate_Zero);
    LONGBOW_RUN_TEST_CASE(Global, PARCSafeMemory_Validate);

    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Allocate_BadAlignment);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Allocate_BadSize);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_AllocateAndClear);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Reallocate);

    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Deallocate_NothingAllocated);

    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_IsValid_True);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_IsValid_False);

    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_Display_NULL);

    LONGBOW_RUN_TEST_CASE(Global, compute_prefix_length);
    LONGBOW_RUN_TEST_CASE(Global, _parcSafeMemory_FormatMemory);
    LONGBOW_RUN_TEST_CASE(Global, memory_prefix_format);
    LONGBOW_RUN_TEST_CASE(Global, memory_prefix_validate);
    LONGBOW_RUN_TEST_CASE(Global, memory_suffix_format);
    LONGBOW_RUN_TEST_CASE(Global, memory_suffix_validate);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_StringDuplicate);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_StringDuplicate_Long);
    LONGBOW_RUN_TEST_CASE(Global, parcSafeMemory_StringDuplicate_Short);
    LONGBOW_RUN_TEST_CASE(Global, validateAlignment);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    assertTrue(parcSafeMemory_Outstanding() == 0, "Expected 0 outstanding allocations")
    {
        printf("Leaking test case: %s", longBowTestCase_GetName(testCase));
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_CASE(Global, validateAlignment)
{
    assertTrue(_alignmentIsValid(sizeof(void *)),
               "Expected alignment of sizeof(void *) failed.");
    assertTrue(_alignmentIsValid(16),
               "Expected alignment of 16 failed.");
}

LONGBOW_TEST_CASE(Global, compute_prefix_length)
{
    // Test that the result is a multiple of the alignment value and greater than the size of _MemoryPrefix.
    for (int i = 0; i < 9; i++) {
        size_t alignment = 1 << i;
        size_t actual = _computePrefixLength(alignment);
        assertTrue((actual & (alignment - 1)) == 0,
                   "Alignment needs to be a multiple of %zd", alignment);
    }
}

LONGBOW_TEST_CASE(Global, memory_prefix_format)
{
    size_t expectedLength = 5;
    int expectedAlignment = sizeof(void *);
    char base[100];
    void *memory = _parcSafeMemory_FormatPrefix((PARCSafeMemoryOrigin *) base, expectedLength, expectedAlignment);

    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(memory);

    assertAligned(prefix, sizeof(void *),
                  "prefix address %p is not aligned to %d",
                  memory, expectedAlignment);
    assertAligned(memory, expectedAlignment,
                  "memory address %p is not aligned to %d",
                  memory, expectedAlignment);

    assertTrue((void *) prefix >= (void *) base,
               "Expected >= %p, actual %p", (void *) base, (void *) prefix);
    assertTrue(_parcSafeMemory_PrefixMagic == prefix->magic,
               "Prefix magic is wrong.");
    assertTrue(expectedLength == prefix->requestedLength,
               "Expected length %zd, actual %zd", expectedLength, prefix->requestedLength);
    assertTrue(expectedAlignment == prefix->alignment,
               "Expected alignment %d, actual %zu",
               expectedAlignment, prefix->alignment);
    assertTrue(_parcSafeMemory_Guard == prefix->guard,
               "Prefix guard is wrong.");
}

LONGBOW_TEST_CASE(Global, memory_suffix_format)
{
    size_t expectedLength = 5;
    int expectedAlignment = sizeof(void *);
    char base[100];
    void *memory = _parcSafeMemory_FormatPrefix((PARCSafeMemoryOrigin *) base, expectedLength, expectedAlignment);

    _MemorySuffix *suffix = _parcSafeMemory_FormatSuffix(memory);
    assertAligned(suffix, sizeof(void *), "suffix pointer is not aligned to %zu", sizeof(void*));
}

LONGBOW_TEST_CASE(Global, memory_suffix_validate)
{
    size_t expectedLength = 5;
    int expectedAlignment = sizeof(void *);
    char base[100];
    void *memory = _parcSafeMemory_FormatPrefix((PARCSafeMemoryOrigin *) base, expectedLength, expectedAlignment);

    _MemorySuffix *suffix = _parcSafeMemory_FormatSuffix(memory);
    assertAligned(suffix, sizeof(void *),
                  "suffix pointer is not aligned to %zu", sizeof(void*));

    PARCSafeMemoryState suffixState = _parcSafeMemory_GetSuffixState(memory);
    assertTrue(suffixState == PARCSafeMemoryState_OK,
               "Expected PARCSafeMemoryState_OK suffix state, actual %s", _parcSafeMemory_StateToString(suffixState));
}

LONGBOW_TEST_CASE(Global, memory_prefix_validate)
{
    size_t expectedLength = 5;
    int expectedAlignment = sizeof(void *);
    char base[100];

    void *memory = _parcSafeMemory_FormatPrefix((PARCSafeMemoryOrigin *) base, expectedLength, expectedAlignment);
    PARCSafeMemoryState actual = _parcSafeMemory_GetPrefixState(memory);
    assertTrue(actual == PARCSafeMemoryState_OK,
               "Expected valid prefix, actual = %d", actual);
}

LONGBOW_TEST_CASE(Global, _parcSafeMemory_FormatMemory)
{
    size_t expectedLength = 5;
    int expectedAlignment = sizeof(void *);
    char base[100];

    void *memory = _parcSafeMemory_FormatMemory((PARCSafeMemoryOrigin *) base, expectedLength, expectedAlignment);

    PARCSafeMemoryState state = _parcSafeMemory_GetState(memory);

    assertTrue(state == PARCSafeMemoryState_OK,
               "Memory did not validate. Actual %d", state);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Allocate)
{
    void *memory;
    size_t size = 100;

    memory = parcSafeMemory_Allocate(size);

    assertTrue(memory != NULL,
               "Expected non-NULL return.");

    assertTrue((_parcSafeMemory_GetPrefixState(memory)) == PARCSafeMemoryState_OK,
               "Prefix did not validate.");
    parcSafeMemory_Deallocate(&memory);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_MemAlign)
{
    void *memory;
    size_t size = 100;

    int failure = parcSafeMemory_MemAlign(&memory, sizeof(void *), size);
    assertTrue(failure == 0,
               "parcSafeMemory_MemAlign failed: %d", failure);

    assertTrue(memory != NULL,
               "Expected non-NULL return.");

    assertTrue((_parcSafeMemory_GetPrefixState(memory)) == PARCSafeMemoryState_OK,
               "Prefix did not validate.");
    parcSafeMemory_Deallocate(&memory);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_ReportAllocation)
{
    void *memory;
    size_t size = 100;

    memory = parcSafeMemory_Allocate(size);
    assertTrue(memory != NULL, "Expected non-NULL return.");
    PARCSafeMemoryState prefixState = _parcSafeMemory_GetPrefixState(memory);
    assertTrue(prefixState == PARCSafeMemoryState_OK,
               "Prefix did not validate.");

    parcSafeMemory_ReportAllocation(1);
}

LONGBOW_TEST_CASE(Global, PARCSafeMemory_Validate)
{
    void *memory;
    size_t size = 100;

    memory = parcSafeMemory_Allocate(size);

    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_OK,
               "Memory did not validate.");
    parcSafeMemory_Deallocate(&memory);
}

LONGBOW_TEST_CASE(Global, PARCSafeMemory_Realloc_Larger)
{
    void *memory = parcSafeMemory_Allocate(100);

    for (unsigned char i = 0; i < 100; i++) {
        ((unsigned char *) memory)[i] = i;
    }

    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_OK,
               "Expected memory to be OK.");

    size_t expectedLength = 100 + 1;
    unsigned char *newMemory = parcSafeMemory_Reallocate(memory, expectedLength);

    assertTrue(_parcSafeMemory_GetState((PARCSafeMemoryUsable *) memory) != PARCSafeMemoryState_OK,
               "Expected original memory to NOT be OK.");
    assertTrue(_parcSafeMemory_GetState((PARCSafeMemoryUsable *) newMemory) == PARCSafeMemoryState_OK,
               "Expected new memory to be OK.");

    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix((PARCSafeMemoryUsable *) newMemory);
    assertTrue(prefix->requestedLength == expectedLength,
               "Prefix Expected length %zd, actual %zd", expectedLength, prefix->requestedLength);

    for (int i = 0; i < 100; i++) {
        assertTrue(((unsigned char *) newMemory)[i] == i,
                   "PARCSafeMemory_Realloc did not copy old memory correctly");
    }

    assertTrue(parcSafeMemory_Outstanding() != 0,
               "No memory allocated!");
    PARCSafeMemoryState state = _parcSafeMemory_GetState((PARCSafeMemoryUsable *) newMemory);
    parcSafeMemory_Deallocate((void **) &newMemory);
    assertTrue(state == PARCSafeMemoryState_OK,
               "Expected PARCSafeMemory_Deallocate of new memory to be OK, actual =%d", state);
    assertTrue(_parcSafeMemory_GetState(memory) != PARCSafeMemoryState_OK,
               "Expected old memory to be invalid.");
}

LONGBOW_TEST_CASE(Global, PARCSafeMemory_Realloc_Smaller)
{
    void *memory = parcSafeMemory_Allocate(100);
    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_OK,
               "Memory did not validate.");

    for (unsigned char i = 0; i < 100; i++) {
        ((unsigned char *) memory)[i] = i;
    }

    size_t expectedLength = 100 - 1;
    unsigned char *newMemory = parcSafeMemory_Reallocate(memory, expectedLength);

    assertTrue(_parcSafeMemory_GetState(memory) != PARCSafeMemoryState_OK,
               "Expected original memory to NOT be OK.");
    assertTrue(_parcSafeMemory_GetState((PARCSafeMemoryUsable *) newMemory) == PARCSafeMemoryState_OK,
               "Expected new memory to be OK.");

    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix((PARCSafeMemoryUsable *) newMemory);
    assertTrue(prefix->requestedLength == expectedLength,
               "Prefix Expected length %zd, actual %zd", expectedLength, prefix->requestedLength);

    for (int i = 0; i < expectedLength; i++) {
        assertTrue(((unsigned char *) newMemory)[i] == i,
                   "PARCSafeMemory_Realloc did not copy correctly");
    }

    assertTrue(parcSafeMemory_Outstanding() != 0,
               "No memory allocated!");
    PARCSafeMemoryState state = _parcSafeMemory_GetState((PARCSafeMemoryUsable *) newMemory);
    parcSafeMemory_Deallocate((void **) &newMemory);
    assertTrue(state == PARCSafeMemoryState_OK,
               "Expected PARCSafeMemory_Deallocate of new memory to be OK, actual =%d", state);
    assertTrue(_parcSafeMemory_GetState(memory) != PARCSafeMemoryState_OK,
               "Expected old memory to be invalid.");
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Reallocate_Zero)
{
    void *memory = parcSafeMemory_Allocate(100);
    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_OK,
               "Memory did not validate.");

    for (unsigned char i = 0; i < 100; i++) {
        ((unsigned char *) memory)[i] = i;
    }

    size_t expectedLength = 0;
    unsigned char *newMemory = parcSafeMemory_Reallocate(memory, expectedLength);

    assertTrue(newMemory == NULL,
               "Expected NULL, actual %p", (void *) newMemory);

    parcSafeMemory_Deallocate(&memory);
    assertNull(memory,
               "Expected memory pointer to be NULL.");
}

LONGBOW_TEST_CASE(Global, PARCSafeMemory_DoubleFree)
{
    size_t expectedSize = 100;

    void *memory = parcSafeMemory_Allocate(expectedSize);
    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_OK,
               "Memory did not validate.");

    for (unsigned char i = 0; i < expectedSize; i++) {
        ((unsigned char *) memory)[i] = i;
    }

    assertTrue(parcSafeMemory_Outstanding() != 0,
               "No memory allocated!");
    PARCSafeMemoryState state = _parcSafeMemory_GetState(memory);
    parcSafeMemory_Deallocate(&memory);
    assertTrue(state == PARCSafeMemoryState_OK,
               "Expected memory to validate");
    assertTrue(parcSafeMemory_Outstanding() != 0,
               "No memory allocated!");
    state = _parcSafeMemory_GetState(memory);
    parcSafeMemory_Deallocate(&memory);
    assertTrue(state == PARCSafeMemoryState_UNDERRUN,
               "Expected memory to be underrun (double free).");
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_StringDuplicate)
{
    char *string = "hello world";
    char *actual = parcSafeMemory_StringDuplicate(string, strlen(string));

    assertTrue(strcmp(string, actual) == 0,
               "Expected %s, actual %s", string, actual);
    parcSafeMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_StringDuplicate_Long)
{
    char *string = "hello world";
    char *actual = parcSafeMemory_StringDuplicate(string, SIZE_MAX);

    assertTrue(strcmp(string, actual) == 0,
               "Expected %s, actual %s", string, actual);
    parcSafeMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_StringDuplicate_Short)
{
    char *string = "hello world";
    char *expected = "hello";
    char *actual = parcSafeMemory_StringDuplicate(string, 5);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcSafeMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Allocate_BadAlignment)
{
    void *result;
    size_t alignment = 3;
    size_t size = 100;

    int failure = parcSafeMemory_MemAlign(&result, alignment, size);
    assertTrue(failure == EINVAL,
               "parcSafeMemory_MemAlign failed to report bad aligment specification");
    assertTrue(parcSafeMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcSafeMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Allocate_BadSize)
{
    void *result;
    size_t alignment = sizeof(void *);
    size_t size = 0;

    int failure = parcSafeMemory_MemAlign(&result, alignment, size);
    assertTrue(failure == EINVAL,
               "parcSafeMemory_MemAlign failed to report bad aligment specification");
    assertTrue(parcSafeMemory_Outstanding() == 0,
               "Expected 0 outstanding allocation, actual %d", parcSafeMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_AllocateAndClear)
{
    void *result;
    size_t size = 100;

    result = parcSafeMemory_AllocateAndClear(size);
    assertNotNull(result,
                  "parcSafeMemory_AllocateAndClear failed: NULL result.");

    for (size_t i = 0; i < size; i++) {
        assertTrue(((char *) result)[i] == 0,
                   "parcSafeMemory_AllocateAndClear failed to zero memory at index %zd", i);
    }
    assertTrue(parcSafeMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcSafeMemory_Outstanding());
    parcSafeMemory_Deallocate(&result);
    assertTrue(parcSafeMemory_Outstanding() == 0,
               "Expected 0 outstanding allocation, actual %d", parcSafeMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Reallocate)
{
    size_t size = 100;

    void *result = parcSafeMemory_AllocateAndClear(size);
    assertNotNull(result,
                  "parcSafeMemory_Allocate failed: NULL.");

    for (size_t i = 0; i < size; i++) {
        assertTrue(((char *) result)[i] == 0,
                   "parcSafeMemory_AllocateAndClear failed to zero memory at index %zd", i);
    }

    result = parcSafeMemory_Reallocate(result, size * 2);

    assertTrue(parcSafeMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcSafeMemory_Outstanding());
    parcSafeMemory_Deallocate(&result);
    assertTrue(parcSafeMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcSafeMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Deallocate_NothingAllocated)
{
    void *result = 0;

    parcSafeMemory_Deallocate(&result);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_IsValid_True)
{
    void *result = parcSafeMemory_AllocateAndClear(5);

    assertTrue(parcSafeMemory_IsValid(result), "Expected properly allocated memory to be valid PARC Safe Memory.");

    parcSafeMemory_Deallocate(&result);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_IsValid_False)
{
    char *memory[10];

    assertFalse(parcSafeMemory_IsValid(memory), "Expected improperly allocated memory to be invalid PARC Safe Memory.");
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Display)
{
    void *result = parcSafeMemory_AllocateAndClear(5);

    parcSafeMemory_Display(result, 0);
    parcSafeMemory_Deallocate(&result);
}

LONGBOW_TEST_CASE(Global, parcSafeMemory_Display_NULL)
{
    parcSafeMemory_Display(NULL, 0);
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, parcSafeMemory_Reallocate_NULL);
    LONGBOW_RUN_TEST_CASE(Errors, PARCSafeMemory_Deallocate_Overrun);
    LONGBOW_RUN_TEST_CASE(Errors, PARCSafeMemory_Deallocate_Underrun);
}

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    // The tests purposefully wreck the various per-allocation accounting structures for the allocated memory in order to test for
    // properly catching the overrun, underrun or other damage.
    // As a result any cleanup of allocated memory is not possible, so these tests leak allocated memory.
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Errors, parcSafeMemory_Reallocate_NULL)
{
    void *result = NULL;
    size_t size = 100;

    result = parcSafeMemory_Reallocate(result, size * 2);

    assertTrue(parcSafeMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcSafeMemory_Outstanding());
    parcSafeMemory_Deallocate(&result);
    assertTrue(parcSafeMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcSafeMemory_Outstanding());
}

LONGBOW_TEST_CASE_EXPECTS(Errors, PARCSafeMemory_Deallocate_Underrun, .event = &LongBowTrapUnexpectedStateEvent)
{
    size_t expectedSize = sizeof(void *) * 2;
    void *memory = parcSafeMemory_Allocate(expectedSize);
    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_OK,
               "Memory did not validate.");

    for (int i = -2; i < (int) expectedSize; i++) {
        ((unsigned char *) memory)[i] = (unsigned char) i;
    }

    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_UNDERRUN,
               "Memory did not underrun.");

    assertTrue(parcSafeMemory_Outstanding() != 0,
               "No memory allocated!");
    PARCSafeMemoryState state = _parcSafeMemory_GetState(memory);
    parcSafeMemory_Deallocate(&memory);

    assertTrue(state == PARCSafeMemoryState_UNDERRUN,
               "Expected memory to be underrun");
}

LONGBOW_TEST_CASE_EXPECTS(Errors, PARCSafeMemory_Deallocate_Overrun, .event = &LongBowTrapUnexpectedStateEvent)
{
    size_t expectedSize = 100;
    void *memory = parcSafeMemory_Allocate(expectedSize);
    assertTrue(_parcSafeMemory_GetState(memory) == PARCSafeMemoryState_OK,
               "Memory did not validate.");

    for (unsigned char i = 0; i < expectedSize + 5; i++) {
        ((unsigned char *) memory)[i] = i;
    }

    assertTrue(parcSafeMemory_Outstanding() != 0,
               "No memory allocated!");
    assertTrue(_parcSafeMemory_GetState(memory) != PARCSafeMemoryState_OK,
               "Expected  not OK, actual %s", _parcSafeMemory_StateToString(_parcSafeMemory_GetState(memory)));
    // this is expected to fail
    parcSafeMemory_Deallocate(&memory);
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcSafeMemory_AllocateDeallocate_1000000_WorstCase);
    LONGBOW_RUN_TEST_CASE(Performance, parcSafeMemory_AllocateDeallocate_1000000_BestCase);

    LONGBOW_RUN_TEST_CASE(Performance, _computeUsableMemoryLength);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

void *memory[1000000];

LONGBOW_TEST_CASE(Performance, parcSafeMemory_AllocateDeallocate_1000000_WorstCase)
{
    size_t size = 100;

    for (int i = 0; i < sizeof(memory) / sizeof(memory[0]); i++) {
        memory[i] = parcSafeMemory_Allocate(size);
    }
    for (int i = 0; i < sizeof(memory) / sizeof(memory[0]); i++) {
        parcSafeMemory_Deallocate(&memory[i]);
    }
}

LONGBOW_TEST_CASE(Performance, parcSafeMemory_AllocateDeallocate_1000000_BestCase)
{
    size_t size = 100;

    for (int i = 0; i < sizeof(memory) / sizeof(memory[0]); i++) {
        memory[i] = parcSafeMemory_Allocate(size);
    }

    int i = sizeof(memory) / sizeof(memory[0]);
    do {
        i--;
        parcSafeMemory_Deallocate(&memory[i]);
    } while (i > 0);
}

LONGBOW_TEST_CASE(Performance, _computeUsableMemoryLength)
{
    for (int i = 0; i < 100000000; i++) {
        size_t alignment = sizeof(void *);
        size_t requestedLength = 10;
        _computeUsableMemoryLength(requestedLength, alignment);
    }
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(safetyMemory);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
