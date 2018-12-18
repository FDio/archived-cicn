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

/** *
 */
#include "../parc_StdlibMemory.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(test_parc_StdlibMemory)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Threads);
    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_StdlibMemory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_StdlibMemory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_Allocate);
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_MemAlign_BadAlignment);
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_MemAlign_BadSize);
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_AllocateAndClear);
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_AllocateAndClear_BadSize);
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_Reallocate);
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_Reallocate_NULL);
    LONGBOW_RUN_TEST_CASE(Global, parcStdlibMemory_StringDuplicate);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaks allocations.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcStdlibMemory_Allocate)
{
    size_t size = 100;

    void *result = parcStdlibMemory_Allocate(size);

    assertNotNull(result, "parcStdlibMemory_Allocate failed: NULL result.");
    assertTrue(parcStdlibMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcStdlibMemory_Outstanding());
    parcStdlibMemory_Deallocate(&result);
}

LONGBOW_TEST_CASE(Global, parcStdlibMemory_MemAlign_BadAlignment)
{
    void *result;
    size_t alignment = 3;
    size_t size = 1200;

    int failure = parcStdlibMemory_MemAlign(&result, alignment, size);
    assertTrue(failure == EINVAL,
               "parcStdlibMemory_Allocate failed to report bad aligment specification");
    assertTrue(parcStdlibMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcStdlibMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcStdlibMemory_MemAlign_BadSize)
{
    void *result;
    size_t alignment = sizeof(void *);
    size_t size = 0;

    int failure = parcStdlibMemory_MemAlign(&result, alignment, size);
    assertTrue(failure == EINVAL,
               "parcStdlibMemory_Allocate failed to report bad aligment specification");
    assertTrue(parcStdlibMemory_Outstanding() == 0,
               "Expected 0 outstanding allocation, actual %d", parcStdlibMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcStdlibMemory_AllocateAndClear)
{
    size_t size = 1200;

    void *result = parcStdlibMemory_AllocateAndClear(size);
    assertNotNull(result, "parcStdlibMemory_Allocate failed: NULL result.");

    for (size_t i = 0; i < size; i++) {
        assertTrue(((char *) result)[i] == 0,
                   "parcStdlibMemory_AllocateAndClear failed to zero memory at index %zd", i);
    }
    assertTrue(parcStdlibMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcStdlibMemory_Outstanding());
    parcStdlibMemory_Deallocate(&result);
    assertTrue(parcStdlibMemory_Outstanding() == 0,
               "Expected 0 outstanding allocation, actual %d", parcStdlibMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcStdlibMemory_AllocateAndClear_BadSize)
{
    void *result;
    size_t alignment = sizeof(void *);
    size_t size = 0;

    int failure = parcStdlibMemory_MemAlign(&result, alignment, size);
    assertTrue(failure == EINVAL,
               "parcStdlibMemory_Allocate failed to report bad aligment specification");
    assertTrue(parcStdlibMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcStdlibMemory_Outstanding());
}

static void
_test_SetMemory(unsigned char *memory, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        memory[i] = i;
    }
}

static void
_test_CheckMemory(unsigned char *memory, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        assertTrue(memory[i] == (i % 256), "memory failed to check at index %zd", i);
    }
}


LONGBOW_TEST_CASE(Global, parcStdlibMemory_Reallocate)
{
    size_t size = 1200;

    void *result = parcStdlibMemory_AllocateAndClear(size);
    assertNotNull(result,
                  "parcStdlibMemory_Allocate failed: NULL result.");

    _test_SetMemory(result, size);
    _test_CheckMemory(result, size);

    result = parcStdlibMemory_Reallocate(result, size * 2);

    _test_CheckMemory(result, size);

    assertTrue(parcStdlibMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcStdlibMemory_Outstanding());
    parcStdlibMemory_Deallocate(&result);
    assertTrue(parcStdlibMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcStdlibMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcStdlibMemory_Reallocate_NULL)
{
    void *result = NULL;
    size_t size = 1200;

    result = parcStdlibMemory_Reallocate(result, size * 2);
    _test_SetMemory(result, size * 2);
    _test_CheckMemory(result, size * 2);

    assertTrue(parcStdlibMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcStdlibMemory_Outstanding());
    parcStdlibMemory_Deallocate(&result);
    assertTrue(parcStdlibMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcStdlibMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcStdlibMemory_StringDuplicate)
{
    char *expected = "Hello World";
    char *actual = parcStdlibMemory_StringDuplicate(expected, strlen(expected));

    assertTrue(expected != actual,
               "Expected a distinct pointer unequal to the original string");
    assertTrue(strcmp(expected, actual) == 0,
               "Expected strings to be equal. '%s' vs '%s'", expected, actual);

    assertTrue(parcStdlibMemory_Outstanding() == 1,
               "Expected 1 outstanding allocation, actual %d", parcStdlibMemory_Outstanding());
    parcStdlibMemory_Deallocate((void **) &actual);
    assertTrue(parcStdlibMemory_Outstanding() == 0,
               "Expected 0 outstanding allocations, actual %d", parcStdlibMemory_Outstanding());
}

LONGBOW_TEST_FIXTURE(Threads)
{
    LONGBOW_RUN_TEST_CASE(Threads, Threads1000);
}

LONGBOW_TEST_FIXTURE_SETUP(Threads)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Threads)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

void *
allocator(void *unused)
{
    for (int i = 0; i < 1000; i++) {
        void *memory = parcStdlibMemory_Allocate(10);
        parcStdlibMemory_Deallocate(&memory);
    }
    return 0;
}

LONGBOW_TEST_CASE(Threads, Threads1000)
{
#define NTHREADS 1000
    pthread_t thread[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        pthread_create(&thread[i], NULL, allocator, NULL);
    }
    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(thread[0], NULL);
    }
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcStdlibMemory_AllocateDeallocate_Forward);
    LONGBOW_RUN_TEST_CASE(Performance, parcStdlibMemory_AllocateDeallocate_Reverse);
    LONGBOW_RUN_TEST_CASE(Performance, parcStdlibMemory_MemAlignDeallocate_Forward);
    LONGBOW_RUN_TEST_CASE(Performance, parcStdlibMemory_MemAlignDeallocate_Reverse);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

#define ELEMENT_COUNT 1000000
#define ELEMENT_SIZE 151
void *memory[ELEMENT_COUNT];

LONGBOW_TEST_CASE(Performance, parcStdlibMemory_AllocateDeallocate_Forward)
{
    for (int i = 0; i < ELEMENT_COUNT; i++) {
        memory[i] = parcStdlibMemory_Allocate(ELEMENT_SIZE);
    }
    for (int i = 0; i < ELEMENT_COUNT; i++) {
        parcStdlibMemory_Deallocate(&memory[i]);
    }
}

LONGBOW_TEST_CASE(Performance, parcStdlibMemory_AllocateDeallocate_Reverse)
{
    for (int i = 0; i < ELEMENT_COUNT; i++) {
        memory[i] = parcStdlibMemory_Allocate(ELEMENT_SIZE);
    }

    int i = ELEMENT_COUNT;
    do {
        i--;
        parcStdlibMemory_Deallocate(&memory[i]);
    } while (i > 0);
}

LONGBOW_TEST_CASE(Performance, parcStdlibMemory_MemAlignDeallocate_Forward)
{
    for (int i = 0; i < ELEMENT_COUNT; i++) {
        parcStdlibMemory_MemAlign(&memory[i], sizeof(void *), ELEMENT_SIZE);
    }
    for (int i = 0; i < ELEMENT_COUNT; i++) {
        parcStdlibMemory_Deallocate(&memory[i]);
    }
}

LONGBOW_TEST_CASE(Performance, parcStdlibMemory_MemAlignDeallocate_Reverse)
{
    for (int i = 0; i < ELEMENT_COUNT; i++) {
        parcStdlibMemory_MemAlign(&memory[i], sizeof(void *), ELEMENT_SIZE);
    }

    int i = ELEMENT_COUNT;
    do {
        i--;
        parcStdlibMemory_Deallocate(&memory[i]);
    } while (i > 0);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_StdlibMemory);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
