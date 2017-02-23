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
#include "../parc_Iterator.c"

#include <inttypes.h>
#include <stdio.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_Iterator)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Iterator)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Iterator)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, parcIterator_CreateAcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s", longBowTestCase_GetName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

static uint64_t _state;

static void *
init(PARCObject *object __attribute__((unused)))
{
    _state = 0;
    return &_state;
}

static bool
hasNext(PARCObject *object __attribute__((unused)), void *state)
{
    uint64_t *value = (uint64_t *) state;
    return (*value < 5);
}

static void *
next(PARCObject *object __attribute__((unused)), void *state)
{
    uint64_t *value = (uint64_t *) state;

    (*value)++;
    return state;
}

static void
removex(PARCObject *object __attribute__((unused)), void **state)
{
}

static void *
getElement(PARCObject *object __attribute__((unused)), void *state)
{
    uint64_t *value = (uint64_t *) state;
    return (void *) *value;
}

static void
fini(PARCObject *object __attribute__((unused)), void *state __attribute__((unused)))
{
}

static void
assertValid(const void *state __attribute__((unused)))
{
}

LONGBOW_TEST_CASE(CreateAcquireRelease, parcIterator_CreateAcquireRelease)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);

    PARCIterator *iterator = parcIterator_Create(buffer, init, hasNext, next, removex, getElement, fini, assertValid);

    parcObjectTesting_AssertAcquireReleaseContract(parcIterator_Acquire, iterator);
    parcBuffer_Release(&buffer);
    parcIterator_Release(&iterator);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcIterator_HasNext);
    LONGBOW_RUN_TEST_CASE(Global, parcIterator_Next);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    bool leaked = parcMemoryTesting_ExpectedOutstanding(0, "%s leaks memory \n", longBowTestCase_GetName(testCase)) != true;
    if (leaked) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcIterator_HasNext)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);

    PARCIterator *iterator = parcIterator_Create(buffer, init, hasNext, next, removex, getElement, fini, assertValid);

    while (parcIterator_HasNext(iterator)) {
        uint64_t value = (uint64_t) parcIterator_Next(iterator);
        printf("%" PRIu64 "\n", value);
    }
    parcBuffer_Release(&buffer);
    parcIterator_Release(&iterator);
}

LONGBOW_TEST_CASE(Global, parcIterator_Next)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);

    PARCIterator *iterator = parcIterator_Create(buffer, init, hasNext, next, removex, getElement, fini, assertValid);

    while (parcIterator_HasNext(iterator)) {
        uint64_t value = (uint64_t) parcIterator_Next(iterator);
        printf("%" PRIu64 "\n", value);
    }
    parcBuffer_Release(&buffer);
    parcIterator_Release(&iterator);
}

LONGBOW_TEST_CASE(Local, _finalize)
{
    testUnimplemented("");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Iterator);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
