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
#include <config.h>
#include <inttypes.h>

#include "../parc_PriorityQueue.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_PriorityQueue)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_PriorityQueue)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_PriorityQueue)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Add);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Add_Expand);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Clear);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Clear_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_ParcFreeDestroyer);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Peek);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Poll);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Peek_Empty);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Poll_Empty);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Size);
    LONGBOW_RUN_TEST_CASE(Global, parcPriorityQueue_Uint64CompareTo);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Add)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 60, 70, 50, 71, 72, 55 };
    size_t count = 6;

    for (int i = 0; i < count; i++) {
        parcPriorityQueue_Add(queue, &data[i]);
    }

    assertTrue(parcPriorityQueue_Size(queue) == count, "Wrong size got %zu expected %zu", parcPriorityQueue_Size(queue), count);
    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Add_Expand)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    size_t capacity = queue->capacity;
    for (int i = 0; i <= capacity; i++) {
        parcPriorityQueue_Add(queue, &capacity);
    }

    assertTrue(capacity < queue->capacity, "Did not expand queue before %zu after %zu", capacity, queue->capacity);
    parcPriorityQueue_Destroy(&queue);
}


LONGBOW_TEST_CASE(Global, parcPriorityQueue_Clear)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 60, 70, 50, 71, 72, 55 };
    size_t count = 6;

    for (int i = 0; i < count; i++) {
        parcPriorityQueue_Add(queue, &data[i]);
    }

    parcPriorityQueue_Clear(queue);

    assertTrue(parcPriorityQueue_Size(queue) == 0, "Wrong size got %zu expected %d", parcPriorityQueue_Size(queue), 0);
    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Clear_Destroy)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, parcPriorityQueue_ParcFreeDestroyer);
    uint64_t *value = parcMemory_Allocate(sizeof(uint64_t));
    assertNotNull(value, "parcMemory_Allocate(%zu) returned NULL", sizeof(uint64_t));
    *value = 1;
    parcPriorityQueue_Add(queue, value);

    parcPriorityQueue_Clear(queue);

    assertTrue(parcPriorityQueue_Size(queue) == 0, "Wrong size got %zu expected %d", parcPriorityQueue_Size(queue), 0);
    parcPriorityQueue_Destroy(&queue);

    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance after clear with destroy: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Create)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_ParcFreeDestroyer)
{
    size_t before_balance = parcMemory_Outstanding();
    uint64_t *a = parcMemory_Allocate(sizeof(uint64_t));
    assertNotNull(a, "parcMemory_Allocate(%zu) returned NULL", sizeof(uint64_t));
    *a = 1;
    parcPriorityQueue_ParcFreeDestroyer((void **) &a);
    size_t after_balance = parcMemory_Outstanding();
    assertTrue(a == NULL, "Did not null double pointer");
    assertTrue(before_balance == after_balance, "Memory imbalance after destroy: before %zu after %zu", before_balance, after_balance);
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Peek)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 60, 70, 50, 71, 72, 55 };
    size_t count = 6;

    for (int i = 0; i < count; i++) {
        parcPriorityQueue_Add(queue, &data[i]);
    }

    uint64_t *test = parcPriorityQueue_Peek(queue);

    assertTrue(*test == 50, "Wrong head element, expected 50 got %" PRIu64 "", *test);
    assertTrue(parcPriorityQueue_Size(queue) == count, "Queue should not have shunk, size %zu expected %zu", parcPriorityQueue_Size(queue), count);
    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Poll)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 60, 70, 50, 71, 72, 55 };
    size_t count = 6;

    for (int i = 0; i < count; i++) {
        parcPriorityQueue_Add(queue, &data[i]);
    }

    uint64_t *test = parcPriorityQueue_Poll(queue);

    assertTrue(*test == 50, "Wrong head element, expected 50 got %" PRIu64 "", *test);
    assertTrue(queue->size == count - 1, "Queue should have shunk, size %zu expected %zu", queue->size, count - 1);
    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Peek_Empty)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t *test = parcPriorityQueue_Peek(queue);
    assertNull(test, "Peek on empty queue should return null, got %p", (void *) test);
    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Poll_Empty)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t *test = parcPriorityQueue_Poll(queue);
    assertNull(test, "Poll on empty queue should return null, got %p", (void *) test);
    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Size)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcPriorityQueue_Uint64CompareTo)
{
    testUnimplemented("");
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_BubbleUp_True);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_BubbleUp_False);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_Expand);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_LeftChildIndex);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_ParentIndex);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_RightChildIndex);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_Swap);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_TrickleDown);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_TrickleLeftChild_True);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_TrickleLeftChild_False);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_TrickleRightChild_Case1_True);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_TrickleRightChild_Case2_True);
    LONGBOW_RUN_TEST_CASE(Local, parcPriorityQueue_TrickleRightChild_Case1_False);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, parcPriorityQueue_BubbleUp_True)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 50, 6 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->size = 2;

    _bubbleUp(queue, 1);
    assertTrue(queue->array[0].data == &data[1], "Element 6 did not make it to the root");

    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Local, parcPriorityQueue_BubbleUp_False)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 50, 60 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->size = 2;

    _bubbleUp(queue, 1);
    assertTrue(queue->array[0].data == &data[0], "Element 60 did not stay as child");

    parcPriorityQueue_Destroy(&queue);
}


LONGBOW_TEST_CASE(Local, parcPriorityQueue_Expand)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    size_t before_capacity = queue->capacity;
    _expand(queue);
    size_t after_capacity = queue->capacity;

    assertTrue(before_capacity < after_capacity, "Expected after capacity %zu to be larger than before %zu", after_capacity, before_capacity);
    parcPriorityQueue_Destroy(&queue);
}

LONGBOW_TEST_CASE(Local, parcPriorityQueue_LeftChildIndex)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, parcPriorityQueue_ParentIndex)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, parcPriorityQueue_RightChildIndex)
{
    testUnimplemented("");
}

/**
 * Swaps two elements
 */
LONGBOW_TEST_CASE(Local, parcPriorityQueue_Swap)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 50, 6 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->size = 2;

    _swap(queue, 0, 1);
    assertTrue(queue->array[0].data == &data[1], "array[0] does not equal data[1]: %p != %p",
               (void *) queue->array[0].data, (void *) &data[1]);
    assertTrue(queue->array[1].data == &data[0], "array[1] does not equal data[0]: %p != %p",
               (void *) queue->array[1].data, (void *) &data[0]);

    parcPriorityQueue_Destroy(&queue);
}

/**
 * Tests each case in TrickleDown:
 * - right child exists, then
 * - no right child, only left child, then
 * - no child
 *
 *        60                      50
 *      /    \                  /    \
 *     70      50    ====>    70      55
 *    /  \    /  \           /  \    /  \
 *   71 72  55   x          71  72  60   x
 */
LONGBOW_TEST_CASE(Local, parcPriorityQueue_TrickleDown)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 60, 70, 50, 71, 72, 55 };

    queue->size = 6;
    for (int i = 0; i < queue->size; i++) {
        queue->array[i].data = &data[i];
    }

    _trickleDown(queue, 0);
    assertTrue(*((uint64_t *) queue->array[0].data) == 50,
               "Root not 50, got %" PRIu64 "\n",
               (uint64_t) *((uint64_t *) queue->array[0].data));
    assertTrue(*((uint64_t *) queue->array[2].data) == 55,
               "Right not 55, got %" PRIu64 "\n",
               (uint64_t) *((uint64_t *) queue->array[2].data));
    assertTrue(*((uint64_t *) queue->array[5].data) == 60,
               "Last not 60, got %" PRIu64 "\n",
               (uint64_t) *((uint64_t *) queue->array[5].data));

    parcPriorityQueue_Destroy(&queue);
}

/**
 * Tests the TRUE case of this condition
 *
 * Case 3: Left child exists (right does not) and l.value < n.value
 *   In this case, swap(n.index, l.index) and set n.index = l.index
 *       50                6
 *      /  \     ===>     / \
 *     6    x            50  x
 */
LONGBOW_TEST_CASE(Local, parcPriorityQueue_TrickleLeftChild_True)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 50, 6 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->size = 2;

    size_t nextElementIndex = _trickleLeftChild(queue, 0, 1);
    assertTrue(nextElementIndex == 1, "nextElementIndex should have been left child 1, got %zu\n", nextElementIndex);

    parcPriorityQueue_Destroy(&queue);
}

/**
 * Tests the FALSE case of this condition
 *
 * Case 3: Left child exists (right does not) and l.value < n.value
 *   In this case, swap(n.index, l.index) and set n.index = l.index
 *       50                6
 *      /  \     ===>     / \
 *     6    x            50  x
 */
LONGBOW_TEST_CASE(Local, parcPriorityQueue_TrickleLeftChild_False)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 6, 50 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->size = 2;

    size_t nextElementIndex = _trickleLeftChild(queue, 0, 1);
    assertTrue(nextElementIndex == 0, "nextElementIndex should have been root 0, got %zu\n", nextElementIndex);

    parcPriorityQueue_Destroy(&queue);
}


/**
 * Tests the TRUE case
 *
 * Case 1: Right child exists and r.value < n.value && r.value < l.value
 *   In this case, swap(n.index, r.index) and set n.index = r.index.
 *       50                6
 *      /  \     ===>     / \
 *     9    6            9   50
 */
LONGBOW_TEST_CASE(Local, parcPriorityQueue_TrickleRightChild_Case1_True)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 50, 9, 6 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->array[2].data = &data[2];
    queue->size = 3;

    size_t nextElementIndex = _trickleRightChild(queue, 0, 1, 2);
    assertTrue(nextElementIndex == 2, "nextElementIndex should have been right 2, got %zu\n", nextElementIndex);

    parcPriorityQueue_Destroy(&queue);
}

/**
 * Tests the FALSE case
 *
 * Case 1: Right child exists and r.value < n.value && r.value < l.value
 *   In this case, swap(n.index, r.index) and set n.index = r.index.
 *       50                6
 *      /  \     ===>     / \
 *     9    6            9   50
 */
LONGBOW_TEST_CASE(Local, parcPriorityQueue_TrickleRightChild_Case1_False)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);

    // r.value not < n.value
    uint64_t data[] = { 6, 9, 50 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->array[2].data = &data[2];
    queue->size = 3;

    size_t nextElementIndex = _trickleRightChild(queue, 0, 1, 2);
    assertTrue(nextElementIndex == 0, "nextElementIndex should have been root 0, got %zu\n", nextElementIndex);

    parcPriorityQueue_Destroy(&queue);
}

/**
 * Tests the TRUE case
 *
 * Case 2: Right child exists and r.value < n.value && l.value <= r.value
 *   In this case swap(n.index, l.index) and set n.index = l.index
 *   This makes sense by transitivity that l <= r < n, so swap(n,l) satisfies the invariant.
 *       50                6
 *      /  \     ===>     / \
 *     6    9            50  9
 */
LONGBOW_TEST_CASE(Local, parcPriorityQueue_TrickleRightChild_Case2_True)
{
    PARCPriorityQueue *queue = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, NULL);
    uint64_t data[] = { 50, 6, 9 };

    queue->array[0].data = &data[0];
    queue->array[1].data = &data[1];
    queue->array[2].data = &data[2];
    queue->size = 3;

    size_t nextElementIndex = _trickleRightChild(queue, 0, 1, 2);
    assertTrue(nextElementIndex == 1, "nextElementIndex should have been left 1, got %zu\n", nextElementIndex);

    parcPriorityQueue_Destroy(&queue);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_PriorityQueue);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
