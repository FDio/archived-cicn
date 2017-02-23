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
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_AtomicInteger.c"

#include <inttypes.h>
#include <stdio.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

LONGBOW_TEST_RUNNER(test_parc_AtomicInteger)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Threaded);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_AtomicInteger)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_AtomicInteger)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicInteger_Uint32Increment);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicInteger_Uint32Decrement);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicInteger_Uint64Increment);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicInteger_Uint64Decrement);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcAtomicInteger_Uint32Increment)
{
    uint32_t value = 0;
    parcAtomicInteger_Uint32Increment(&value);
    assertTrue(value == 1, "Expected 1, actual, %u", value);
}

LONGBOW_TEST_CASE(Global, parcAtomicInteger_Uint32Decrement)
{
    uint32_t value = 0;
    parcAtomicInteger_Uint32Increment(&value);
    assertTrue(value == 1, "Expected 1, actual, %u", value);
}

LONGBOW_TEST_CASE(Global, parcAtomicInteger_Uint64Increment)
{
    uint64_t value = 0;
    parcAtomicInteger_Uint64Increment(&value);
    assertTrue(value == 1, "Expected 1, actual, %" PRIu64 "", value);
}

LONGBOW_TEST_CASE(Global, parcAtomicInteger_Uint64Decrement)
{
    uint64_t value = 0;
    parcAtomicInteger_Uint64Increment(&value);
    assertTrue(value == 1, "Expected 1, actual, %" PRIu64 "", value);
}

LONGBOW_TEST_FIXTURE(Threaded)
{
    LONGBOW_RUN_TEST_CASE(Threaded, collaborative);
}

LONGBOW_TEST_FIXTURE_SETUP(Threaded)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Threaded)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

static void *
collaborator_A(void *data)
{
    uint32_t *valuePointer = (uint32_t *) data;
    uint32_t contribution = 0;
    while (*valuePointer < 1000000) {
        parcAtomicInteger_Uint32Increment(valuePointer);
        contribution++;
    }
    printf("A contribution %d\n", contribution);
    pthread_exit((void *) NULL);
}

static void *
collaborator_B(void *data)
{
    uint32_t *valuePointer = (uint32_t *) data;

    uint32_t contribution = 0;
    while (*valuePointer < 1000000) {
        parcAtomicInteger_Uint32Increment(valuePointer);
        contribution++;
    }

    printf("B contribution %d\n", contribution);
    pthread_exit((void *) NULL);
}

LONGBOW_TEST_CASE(Threaded, collaborative)
{
    uint32_t value = 0;

    pthread_t thread_A;
    pthread_t thread_B;

    pthread_create(&thread_A, NULL, collaborator_A, &value);
    pthread_create(&thread_B, NULL, collaborator_B, &value);

    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_AtomicInteger);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
