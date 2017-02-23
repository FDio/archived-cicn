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
#include "../parc_Chunker.c"

#include <stdio.h>

#include <parc/algol/parc_Object.h>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

typedef struct {
    int val;
    int dir;
    bool atEnd;
} _DummyChunkerState;

typedef struct {
    int start;
    int end;
    bool released;
    size_t chunkSize;
} _DummyChunker;

static void *
_InitForward(_DummyChunker *chunker)
{
    _DummyChunkerState *state = parcMemory_Allocate(sizeof(_DummyChunkerState));

    state->val = 0;
    state->dir = 1;
    state->atEnd = false;

    return state;
}

static bool
_hasNext(_DummyChunker *chunker, void *voidstate)
{
    _DummyChunkerState *state = (_DummyChunkerState *) voidstate;
    return !state->atEnd;
}

static void *
_next(_DummyChunker *chunker, void *voidstate)
{
    _DummyChunkerState *state = (_DummyChunkerState *) voidstate;
    state->val++;

    if (state->val == chunker->end) {
        state->atEnd = true;
    }

    return state;
}

static void *
_get(_DummyChunker *chunker, void *voidstate)
{
    _DummyChunkerState *state = (_DummyChunkerState *) voidstate;
    return &(state->val);
}

static void
_finish(_DummyChunker *chunker, void *state)
{
    _DummyChunkerState *thestate = (_DummyChunkerState *) state;
    parcMemory_Deallocate(&thestate);
}

static PARCIterator *
_mock_ForwardIterator(const void *chunker)
{
    PARCIterator *iterator = parcIterator_Create((void *) chunker,
                                                 (void *(*)(PARCObject *))_InitForward,
                                                 (bool (*)(PARCObject *, void *))_hasNext,
                                                 (void *(*)(PARCObject *, void *))_next,
                                                 NULL,
                                                 (void *(*)(PARCObject *, void *))_get,
                                                 (void (*)(void *, void *))_finish,
                                                 NULL);

    return iterator;
}

static PARCIterator *
_mock_ReverseIterator(const void *chunker)
{
    PARCIterator *iterator = parcIterator_Create((void *) chunker,
                                                 (void *(*)(PARCObject *))_InitForward,
                                                 (bool (*)(PARCObject *, void *))_hasNext,
                                                 (void *(*)(PARCObject *, void *))_next,
                                                 NULL,
                                                 (void *(*)(PARCObject *, void *))_get,
                                                 (void (*)(void *, void *))_finish,
                                                 NULL);

    return iterator;
}

static size_t
_mock_GetChunkSize(const void *chunker)
{
    _DummyChunker *dummy = (_DummyChunker *) chunker;
    return dummy->chunkSize;
}

static void
_dummyDestroy(_DummyChunker **chunkerP)
{
    // pass...
}

PARCChunkerInterface *_MockChunker = &(PARCChunkerInterface) {
    .ForwardIterator = (void *(*)(const void *))_mock_ForwardIterator,
    .ReverseIterator = (void *(*)(const void *))_mock_ReverseIterator,
    .GetChunkSize = (size_t (*)(const void *))_mock_GetChunkSize
};

parcObject_ExtendPARCObject(_DummyChunker, _dummyDestroy, NULL, NULL, NULL, NULL, NULL, NULL);
parcObject_ImplementAcquire(_dummy, _DummyChunker);
parcObject_ImplementRelease(_dummy, _DummyChunker);

static _DummyChunker *
_dummy_Create(int val)
{
    _DummyChunker *chunker = (_DummyChunker *) parcObject_CreateAndClearInstance(_DummyChunker);
    chunker->start = 0;
    chunker->end = val;
    chunker->released = false;
    chunker->chunkSize = val;
    return chunker;
}

LONGBOW_TEST_RUNNER(parc_Chunker)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Chunker)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Chunker)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_Chunker_Create);
    LONGBOW_RUN_TEST_CASE(Global, parc_Chunker_ForwardIterator);
    LONGBOW_RUN_TEST_CASE(Global, parc_Chunker_ReverseIterator);
    LONGBOW_RUN_TEST_CASE(Global, parc_Chunker_GetChunkSize);
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

LONGBOW_TEST_CASE(Global, parc_Chunker_Create)
{
    _DummyChunker *dummy = _dummy_Create(10);
    PARCChunker *chunker = parcChunker_Create(dummy, _MockChunker);
    _dummy_Release(&dummy);

    assertNotNull(chunker, "Expected non-NULL PARCChunker to be created from the dummy MockChunker");
    PARCChunker *copy = parcChunker_Acquire(chunker);
    assertNotNull(copy, "Expected non-NULL copy of the PARCChunker");

    parcChunker_Release(&chunker);
    parcChunker_Release(&copy);
}

LONGBOW_TEST_CASE(Global, parc_Chunker_ForwardIterator)
{
    int n = 10;

    _DummyChunker *dummy = _dummy_Create(n);
    PARCChunker *chunker = parcChunker_Create(dummy, _MockChunker);
    _dummy_Release(&dummy);
    PARCIterator *itr = parcChunker_ForwardIterator(chunker);

    int targetSum = (n * (n + 1)) / 2;
    int sum = 0;
    while (parcIterator_HasNext(itr)) {
        int *val = parcIterator_Next(itr);
        sum += *val;
    }
    assertTrue(targetSum == sum, "Expected the iterator to walk the chunker as needed\n");

    parcIterator_Release(&itr);
    parcChunker_Release(&chunker);
}

LONGBOW_TEST_CASE(Global, parc_Chunker_ReverseIterator)
{
    int n = 10;

    _DummyChunker *dummy = _dummy_Create(n);
    PARCChunker *chunker = parcChunker_Create(dummy, _MockChunker);
    _dummy_Release(&dummy);
    PARCIterator *itr = parcChunker_ReverseIterator(chunker);

    int targetSum = (n * (n + 1)) / 2;
    int sum = 0;
    while (parcIterator_HasNext(itr)) {
        int *val = parcIterator_Next(itr);
        sum += *val;
    }
    assertTrue(targetSum == sum, "Expected the iterator to walk the chunker as needed\n");

    parcIterator_Release(&itr);
    parcChunker_Release(&chunker);
}

LONGBOW_TEST_CASE(Global, parc_Chunker_GetChunkSize)
{
    int n = 10;

    _DummyChunker *dummy = _dummy_Create(n);
    PARCChunker *chunker = parcChunker_Create(dummy, _MockChunker);
    _dummy_Release(&dummy);

    size_t chunkSize = parcChunker_GetChunkSize(chunker);
    assertTrue(chunkSize == n, "Expected the chunk size to be %d, got %zu\n", n, chunkSize);

    parcChunker_Release(&chunker);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Chunker);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
