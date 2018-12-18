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


#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include "../parc_TreeMap.c"


typedef struct {
    int value;
} _Int;

static _Int *
_int_Copy(const _Int *source);

static bool
_int_Equals(const _Int *a, const _Int *b)
{
    return a->value == b->value;
}

static int
_int_Compare(const _Int *a, const _Int *b)
{
    int result = 0;
    if (a->value > b->value) {
        result = 1;
    } else if (a->value < b->value) {
        result = -1;
    }

    return result;
}

parcObject_ExtendPARCObject(_Int, NULL, _int_Copy, NULL, _int_Equals, _int_Compare, NULL, NULL);

static
parcObject_ImplementRelease(_int, _Int);

static _Int *
_int_Create(const int value)
{
    _Int *newObj = parcObject_CreateInstance(_Int);
    assertNotNull(newObj, "parcMemory_Allocate(%zu) returned NULL", sizeof(int));
    newObj->value = value;
    return newObj;
}

static _Int *
_int_Copy(const _Int *source)
{
    return _int_Create(source->value);
}

static _Int *
_int_Set(_Int *obj, const int value)
{
    obj->value = value;
    return obj;
}
static PARCBuffer*
strBuf(char *key)
{
    return parcBuffer_WrapCString(key);
}


LONGBOW_TEST_RUNNER(PARC_TreeMap)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(Stress);
}

LONGBOW_TEST_RUNNER_SETUP(PARC_TreeMap)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    int seed = (int) time(NULL);
    srandom(seed);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(PARC_TreeMap)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Create);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_Ordered);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Put_Release);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Put_Ordered);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Put_OutOfOrder);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Put_Overwrite);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_ReleaseTillEmpty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Size_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Size);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Size_Overwrite);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Get_EmptyTree);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Get_NonExistent);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Get_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Get);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Get_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Get_Biggest);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Get_Smallest);

    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_FirstKey);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_FirstEntry);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_LastKey);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_LastEntry);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_FirstKey_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_LastKey_Empty);

    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_LowerEntry);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_LowerKey);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_HigherEntry);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_HigherKey);

    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_NonExistent);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease_NonExistent);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_WithSuccessorNonRoot);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_LeftChildRightChild);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Keys);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Values);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Equals_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Equals);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Equals_DifferentLength);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Equals_Not_Values);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Equals_Not_Keys);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Copy);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Copy_Direct);

    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Iterator);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_ValueIterator);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_KeyIterator);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_Using_Iterator);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeMap_Remove_Element_Using_Iterator);
}

#define N_TEST_ELEMENTS 42

typedef struct {
    PARCTreeMap *testMap1;
    PARCTreeMap *testMap2;
    _Int *k[N_TEST_ELEMENTS];
    _Int *v[N_TEST_ELEMENTS];
} TestData;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->testMap1 = parcTreeMap_Create();
    data->testMap2 = parcTreeMap_CreateCustom((PARCTreeMap_CustomCompare *) _int_Compare);

    for (int i = 0; i < N_TEST_ELEMENTS; ++i) {
        data->k[i] = _int_Create(i);
        data->v[i] = _int_Create(i + 1000);
    }
    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    parcTreeMap_Release(&data->testMap1);
    parcTreeMap_Release(&data->testMap2);

    for (int i = 0; i < N_TEST_ELEMENTS; ++i) {
        _int_Release(&(data->k[i]));
        _int_Release(&(data->v[i]));
    }

    parcMemory_Deallocate((void **) &data);

    /*
     * uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
     * if (outstandingAllocations != 0) {
     *    printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
     *    return LONGBOW_STATUS_MEMORYLEAK;
     * }
     */
    return LONGBOW_STATUS_SUCCEEDED;
}

static
int
recursiveCheckBlackDepth(const PARCTreeMap *tree, const _RBNode *node)
{
    assertNotNull(tree, "Null tree?\n");
    assertNotNull(node, "Null node?\n");
    if (node == tree->nil) {
        return 0;
    }
    int right_depth = recursiveCheckBlackDepth(tree, node->rightChild);
    int left_depth = recursiveCheckBlackDepth(tree, node->leftChild);
    assertTrue(right_depth == left_depth, "Wrong depth!!\n");
    if (_rbNodeColor(node) == BLACK) {
        return right_depth + 1;
    }
    return right_depth;
}

static
void
rbCheckTree(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);
    if (tree->size > 0) {
        recursiveCheckBlackDepth(tree, tree->root);
    }
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_Ordered)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;

    for (int i = 0; i < 16; i++) {
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    for (int i = 0; i < 14; i++) {
        //rbPrintTreeString(tree1);
        PARCObject *value = parcTreeMap_Remove(tree1, data->k[i]);
        assertNotNull(value, "Data is null!");
        assertTrue(_int_Equals(value, data->v[i]), "Expect the ordered value.");
        parcObject_Release(&value);
    }
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Create)
{
    PARCTreeMap *map = parcTreeMap_Create();
    parcTreeMap_Release(&map);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Acquire)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *map = parcTreeMap_Acquire(data->testMap1);
    parcTreeMap_Release(&map);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Put_Release)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    PARCBuffer *value1 = strBuf("value 1");
    PARCBuffer *key1 = strBuf("1");
    PARCBuffer *value2 = strBuf("value 2");
    PARCBuffer *key2 = strBuf("2");
    PARCBuffer *value3 = strBuf("value 3");
    PARCBuffer *key3 = strBuf("3");

    parcTreeMap_Put(tree, key1, value1);
    parcTreeMap_Put(tree, key2, value2);
    parcTreeMap_Put(tree, key3, value3);

    parcBuffer_Release(&key1);
    parcBuffer_Release(&value1);
    parcBuffer_Release(&key2);
    parcBuffer_Release(&value2);
    parcBuffer_Release(&key3);
    parcBuffer_Release(&value3);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Put_Overwrite)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    parcTreeMap_Put(tree, data->k[1], data->v[1]);
    parcTreeMap_Put(tree, data->k[2], data->v[2]);
    parcTreeMap_Put(tree, data->k[3], data->v[3]);
    parcTreeMap_Put(tree, data->k[3], data->v[4]);
    parcTreeMap_Put(tree, data->k[3], data->v[5]);

    assertTrue(3 == parcTreeMap_Size(tree), "Wrong size of tree should stay at 3");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Put_Ordered)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    parcTreeMap_Put(tree, data->k[1], data->v[1]);
    parcTreeMap_Put(tree, data->k[2], data->v[2]);
    parcTreeMap_Put(tree, data->k[3], data->v[3]);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Put_OutOfOrder)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;

    parcTreeMap_Put(tree1, data->k[4], data->v[4]);
    parcTreeMap_Put(tree1, data->k[2], data->v[2]);
    parcTreeMap_Put(tree1, data->k[3], data->v[3]);
    parcTreeMap_Put(tree1, data->k[1], data->v[1]);

    PARCTreeMap *tree2 = data->testMap2;
    parcTreeMap_Put(tree2, data->k[1], data->v[1]);
    parcTreeMap_Put(tree2, data->k[3], data->v[3]);
    parcTreeMap_Put(tree2, data->k[2], data->v[2]);
    parcTreeMap_Put(tree2, data->k[4], data->v[4]);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Expect trees to be Equal");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Size_Empty)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    assertTrue(0 == parcTreeMap_Size(tree), "Wrong size of tree - empty, start");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Size)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    parcTreeMap_Put(tree, data->k[4], data->v[4]);
    parcTreeMap_Put(tree, data->k[2], data->v[2]);
    parcTreeMap_Put(tree, data->k[3], data->v[3]);

    assertTrue(3 == parcTreeMap_Size(tree), "Wrong size of tree after add 3");

    parcTreeMap_Put(tree, data->k[1], data->v[1]);

    assertTrue(4 == parcTreeMap_Size(tree), "Wrong size of tree after add 1 more");

    parcTreeMap_RemoveAndRelease(tree, data->k[2]);

    size_t size = parcTreeMap_Size(tree);

    assertTrue(3 == size, "Wrong size of tree after 1 delete (%zu instead of 3)", size);

    parcTreeMap_Put(tree, data->k[7], data->v[7]);

    assertTrue(4 == parcTreeMap_Size(tree), "Wrong size of tree after add 1 more");

    parcTreeMap_RemoveAndRelease(tree, data->k[3]);
    assertTrue(3 == parcTreeMap_Size(tree), "Wrong size of tree after del 1 more - 3");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_ReleaseTillEmpty)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    // This order of puts are removes exercises code paths
    // in TreeMap not exercised in any other place.
    int idx1a[7] = { 4, 2, 3, 1, 5, 7, 6 };
    int idx1b[7] = { 3, 1, 4, 2, 6, 5, 7 };
    int idx2a[7] = { 4, 6, 5, 7, 3, 1, 2 };
    int idx2b[7] = { 5, 7, 4, 6, 2, 3, 1 };

    for (int i = 0; i < 7; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1a[i]], data->v[idx1a[i]]);
        parcTreeMap_Put(tree2, data->k[idx2a[i]], data->v[idx2a[i]]);
    }

    for (int i = 0; i < 7; ++i) {
        parcTreeMap_RemoveAndRelease(tree1, data->k[idx1b[i]]);
        parcTreeMap_RemoveAndRelease(tree2, data->k[idx2b[i]]);
    }

    size_t size;
    size = parcTreeMap_Size(tree1);

    assertTrue(0 == size, "Wrong size of tree - empty (got %zu)", size);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Size_Overwrite)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    parcTreeMap_Put(tree, data->k[4], data->v[4]);
    parcTreeMap_Put(tree, data->k[2], data->v[2]);
    parcTreeMap_Put(tree, data->k[3], data->v[3]);

    // Size is 3 here, we'll insert the same number now..

    parcTreeMap_Put(tree, data->k[3], data->v[23]);

    assertTrue(3 == parcTreeMap_Size(tree), "Wrong size of tree after overwrite");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Get_EmptyTree)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    PARCObject *value = parcTreeMap_Get(tree, data->k[1]);

    assertTrue(NULL == value, "Object did not exist, must return NULL");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Get_NonExistent)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree = data->testMap1;

    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Get(tree, data->k[23]);

    assertTrue(NULL == value, "Object did not exist, must return NULL");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Get_First)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 1; i < 4; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Get(tree, data->k[1]);

    assertTrue(_int_Equals(data->v[1], value), "Wrong value, got %ld", (long) value);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Get)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Get(tree, data->k[4]);

    assertTrue(_int_Equals(data->v[4], value), "Wrong value, got %ld", (long) value);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Get_Last)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Get(tree, data->k[9]);

    assertTrue(_int_Equals(data->v[9], value), "Wrong value, got %ld", (long) value);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Get_Smallest)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }


    PARCObject *value = parcTreeMap_Get(tree, data->k[1]);

    assertTrue(_int_Equals(data->v[1], value), "Wrong value, got %ld", (long) value);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Get_Biggest)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Get(tree, data->k[39]);

    assertTrue(_int_Equals(data->v[39], value), "Wrong value, got %ld", (long) value);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_FirstEntry)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCKeyValue *entry = parcTreeMap_GetFirstEntry(tree);

    assertTrue(_int_Equals(data->k[1], parcKeyValue_GetKey(entry)),
               "Wrong value, got %ld", (long) parcKeyValue_GetKey(entry));
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_FirstKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCObject *key = parcTreeMap_GetFirstKey(tree);

    assertTrue(_int_Equals(data->k[1], key), "Wrong value, got %ld", (long) key);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_FirstKey_Empty)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    PARCObject *key = parcTreeMap_GetFirstKey(tree);

    assertNull(key, "Should get NULL on empty tree");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_LastKey_Empty)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    PARCObject *key = parcTreeMap_GetLastKey(tree);

    assertNull(key, "Should get NULL on empty tree");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_LastEntry)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCKeyValue *entry = parcTreeMap_GetLastEntry(tree);

    assertTrue(_int_Equals(data->k[39], parcKeyValue_GetKey(entry)),
               "Wrong value, got %ld", (long) parcKeyValue_GetKey(entry));
}


LONGBOW_TEST_CASE(Global, PARC_TreeMap_LastKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree = data->testMap1;

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree, data->k[i], data->v[i]);
    }

    PARCObject *key = parcTreeMap_GetLastKey(tree);

    assertTrue(_int_Equals(data->k[39], key), "Wrong value, got %ld", (long) key);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_First)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_Put(tree1, data->k[1], data->v[1]);

    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Remove(tree1, data->k[1]);
    parcObject_Release(&value);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 31; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_Put(tree1, data->k[30], data->v[30]);

    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Remove(tree1, data->k[30]);
    parcObject_Release(&value);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_Last)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_Put(tree1, data->k[41], data->v[41]);

    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    PARCObject *value = parcTreeMap_Remove(tree1, data->k[41]);
    assertNotNull(value, "Expected to find some object.");
    assertTrue(_int_Equals(data->v[41], value), "Expected value 41 in return");
    parcObject_Release(&value);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees don't match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease_First)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_Put(tree1, data->k[1], data->v[1]);

    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_RemoveAndRelease(tree1, data->k[1]);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 31; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_Put(tree1, data->k[30], data->v[30]);

    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_RemoveAndRelease(tree1, data->k[30]);
    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");

    for (int i = 20; i < 30; i++) {
        parcTreeMap_RemoveAndRelease(tree1, data->k[i]);
        parcTreeMap_RemoveAndRelease(tree2, data->k[49 - i]);
    }
    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");

    for (int i = 2; i < 10; i++) {
        parcTreeMap_RemoveAndRelease(tree1, data->k[i]);
        parcTreeMap_RemoveAndRelease(tree2, data->k[11 - i]);
    }
    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");

    for (int i = 31; i < 40; i++) {
        parcTreeMap_RemoveAndRelease(tree1, data->k[i]);
        parcTreeMap_RemoveAndRelease(tree2, data->k[70 - i]);
    }
    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_NonExistent)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    PARCObject *element = parcTreeMap_Remove(tree1, data->k[0]);

    assertNull(element, "Return value must be NULL on non existing element");
    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease_NonExistent)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_RemoveAndRelease(tree1, data->k[0]);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_WithSuccessorNonRoot)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    int idx1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };
    int idx2[13] = { 8, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1[i]], data->v[idx1[i]]);
    }

    for (int i = 0; i < 13; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree2, data->k[idx2[i]], data->v[idx2[i]]);
    }

    _Int *key = _int_Create(4);
    parcTreeMap_RemoveAndRelease(tree1, key);
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 12));
    _int_Release(&key);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_LeftChildRightChild)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    int idx1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1[i]], data->v[idx1[i]]);
        parcTreeMap_Put(tree2, data->k[idx1[i]], data->v[idx1[i]]);
    }

    _Int *key = _int_Create(0);
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 13));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 7));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 14));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 6));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 15));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 12));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 11));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 10));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 9));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 8));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 5));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 4));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 3));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 2));
    parcTreeMap_RemoveAndRelease(tree1, _int_Set(key, 1));

    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 1));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 2));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 3));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 4));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 5));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 6));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 7));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 8));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 9));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 10));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 11));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 12));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 13));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 14));
    parcTreeMap_RemoveAndRelease(tree2, _int_Set(key, 15));
    _int_Release(&key);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_RemoveAndRelease_Last)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    parcTreeMap_Put(tree1, data->k[41], data->v[41]);
    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_RemoveAndRelease(tree1, data->k[41]);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Trees dont match after remove");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_LowerEntry)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    // Empty Tree
    PARCKeyValue *kv = parcTreeMap_GetLowerEntry(tree1, data->k[23]);
    assertNull(kv, "Expected a NULL return for LowerEntry() on empty tree");

    // Fill Tree
    int max = N_TEST_ELEMENTS - 1;
    for (int i = 21; i <= max; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }
    for (int i = 1; i < 21; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    // Using lowest key in tree
    kv = parcTreeMap_GetLowerEntry(tree1, data->k[1]);
    assertNull(kv, "Expected a NULL return for no lower entry");

    // On all entries except the lowest tree
    for (int i = max; i > 1; --i) {
        kv = parcTreeMap_GetLowerEntry(tree1, data->k[i]);
        assertNotNull(kv, "Expected a lower entry to exist");
        _Int *key = (_Int *) parcKeyValue_GetKey(kv);
        assertTrue(_int_Equals(key, data->k[i - 1]),
                   "Expected entry with key %d, got %d",
                   data->k[i - 1]->value, key->value);
    }
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_LowerKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    // Empty Tree
    _Int *key = (_Int *) parcTreeMap_GetLowerKey(tree1, data->k[23]);
    assertNull(key, "Expected a NULL return for LowerEntry() on empty tree");

    int max = N_TEST_ELEMENTS - 1;
    for (int i = 21; i <= max; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }
    for (int i = 1; i < 21; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    // Using lowest key in tree
    key = (_Int *) parcTreeMap_GetLowerKey(tree1, data->k[1]);
    assertNull(key, "Expected a NULL return for no lower entry");

    // On all entries except the lowest tree
    for (int i = max; i > 1; --i) {
        key = parcTreeMap_GetLowerKey(tree1, data->k[i]);
        assertNotNull(key, "Expected a lower entry to exist");
        assertTrue(_int_Equals(key, data->k[i - 1]),
                   "Expected entry with key %d, got %d",
                   data->k[i - 1]->value, key->value);
    }
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_HigherEntry)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    // Empty Tree
    PARCKeyValue *kv = parcTreeMap_GetHigherEntry(tree1, data->k[23]);
    assertNull(kv, "Expected a NULL return for HigherEntry() on empty tree");

    int max = N_TEST_ELEMENTS - 2;
    for (int i = 21; i <= max; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }
    for (int i = 1; i < 21; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    // Using highest key in tree
    kv = parcTreeMap_GetHigherEntry(tree1, data->k[max]);
    assertNull(kv, "Expected a NULL return for no higher entry");

    // On all entries except the lowest tree
    for (int i = 1; i < max; ++i) {
        kv = parcTreeMap_GetHigherEntry(tree1, data->k[i]);
        assertNotNull(kv, "Expected a higher entry to exist");
        _Int *key = (_Int *) parcKeyValue_GetKey(kv);
        assertTrue(_int_Equals(key, data->k[i + 1]),
                   "Expected entry with key %d, got %d",
                   data->k[i + 1]->value, key->value);
    }
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_HigherKey)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    // Empty Tree
    _Int *key = (_Int *) parcTreeMap_GetHigherKey(tree1, data->k[23]);
    assertNull(key, "Expected a NULL return for LowerEntry() on empty tree");

    int max = N_TEST_ELEMENTS - 2;
    for (int i = 21; i <= max; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }
    for (int i = 1; i < 21; ++i) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    key = (_Int *) parcTreeMap_GetHigherEntry(tree1, data->k[max]);
    assertNull(key, "Expected a NULL return for no higher entry");

    for (int i = 1; i < max; ++i) {
        key = (_Int *) parcTreeMap_GetHigherKey(tree1, data->k[i]);
        assertNotNull(key, "Expected a higher entry to exist");
        assertTrue(_int_Equals(key, data->k[i + 1]),
                   "Expected entry with key %d, got %d",
                   data->k[i + 1]->value, key->value);
    }
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Keys)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;

    PARCList *list = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);

    // Insert in tree out of order
    for (int i = 10; i < 20; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }
    for (int i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    // Insert in list in order
    for (int i = 1; i < 20; i++) {
        // Add some elements to the tree
        parcList_Add(list, data->k[i]);
    }

    PARCList *keys = parcTreeMap_AcquireKeys(tree1);

    assertTrue(parcList_Equals(list, keys), "Key list doesnt' match");

    parcList_Release(&keys);
    parcList_Release(&list);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Values)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;

    PARCList *list = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);

    // Insert in tree out of order
    for (int i = 10; i < 20; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }
    for (int i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    // Insert in list in order
    for (int i = 1; i < 20; i++) {
        // Add some elements to the tree
        parcList_Add(list, data->v[i]);
    }

    PARCList *values = parcTreeMap_AcquireValues(tree1);

    assertTrue(parcList_Equals(list, values), "Key list doesnt' match");

    parcList_Release(&values);
    parcList_Release(&list);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Equals_Empty)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Empty lists are not equal");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Equals_DifferentLength)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;


    for (int i = 1; i < 20; i++) {
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[i]);
    }

    parcTreeMap_Put(tree2, data->k[41], data->v[41]);

    assertFalse(parcTreeMap_Equals(tree1, tree2), "Lists are equal");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Equals_Not_Values)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 1; i < 20; i++) {
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i], data->v[20 - i]);
    }

    assertFalse(parcTreeMap_Equals(tree1, tree2), "Lists are equal");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Equals_Not_Keys)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 1; i < 20; i++) {
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[i + 1], data->v[i]);
    }
    assertTrue(parcTreeMap_Size(tree1) == parcTreeMap_Size(tree2), "Expect trees to have the same size.");

    assertFalse(parcTreeMap_Equals(tree1, tree2), "Lists should not be equal");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Equals)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap2;

    for (int i = 1; i < 40; i++) {
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
        parcTreeMap_Put(tree2, data->k[40 - i], data->v[40 - i]);
    }

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Lists are not equal");
}


LONGBOW_TEST_CASE(Global, PARC_TreeMap_Copy)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *treeCopy = parcTreeMap_Copy(tree1);

    for (int i = 1; i < 10; i++) {
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    assertFalse(parcTreeMap_Equals(tree1, treeCopy), "Lists are not equal");

    parcTreeMap_Release(&treeCopy);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Copy_Direct)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    for (int i = 1; i < 20; i++) {
        parcTreeMap_Put(tree1, data->k[i], data->v[i]);
    }

    PARCTreeMap *treeCopy = parcTreeMap_Copy(tree1);

    assertTrue(parcTreeMap_Equals(tree1, treeCopy), "Lists are not equal");

    parcTreeMap_Release(&treeCopy);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_ValueIterator)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    int idx1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1[i]], data->v[idx1[i]]);
    }

    PARCIterator *it = parcTreeMap_CreateValueIterator(tree1);

    for (int idx = 1; parcIterator_HasNext(it); ++idx) {
        _Int *value = (_Int *) parcIterator_Next(it);
        assertTrue(_int_Equals(value, data->v[idx]), "Expected value %d got %d", data->v[idx]->value, value->value);
    }

    parcIterator_Release(&it);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_KeyIterator)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    int idx1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1[i]], data->v[idx1[i]]);
    }

    PARCIterator *it = parcTreeMap_CreateKeyIterator(tree1);

    for (int idx = 1; parcIterator_HasNext(it); ++idx) {
        _Int *key = (_Int *) parcIterator_Next(it);
        assertTrue(_int_Equals(key, data->k[idx]),
                   "Expected value %d got %d",
                   data->k[idx]->value,
                   key->value);
    }

    parcIterator_Release(&it);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Iterator)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    int idx1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1[i]], data->v[idx1[i]]);
    }

    PARCIterator *it = parcTreeMap_CreateKeyValueIterator(tree1);

    for (int idx = 1; parcIterator_HasNext(it); ++idx) {
        PARCKeyValue *kv = (PARCKeyValue *) parcIterator_Next(it);
        assertTrue(_int_Equals((_Int *) parcKeyValue_GetKey(kv), data->k[idx]),
                   "Expected value %d got %d",
                   data->k[idx]->value,
                   ((_Int *) parcKeyValue_GetKey(kv))->value);
    }

    parcIterator_Release(&it);
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_Using_Iterator)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;

    int idx1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1[i]], data->v[idx1[i]]);
    }

    PARCIterator *it = parcTreeMap_CreateKeyValueIterator(tree1);
    for (int idx = 1; parcIterator_HasNext(it); ++idx) {
        parcIterator_Next(it);
        parcIterator_Remove(it);
    }
    parcIterator_Release(&it);

    assertTrue(parcTreeMap_Size(tree1) == 0, "Expect the tree to be empty after removes.");
}

LONGBOW_TEST_CASE(Global, PARC_TreeMap_Remove_Element_Using_Iterator)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCTreeMap *tree1 = data->testMap1;
    PARCTreeMap *tree2 = data->testMap1;

    int idx1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 15, 13 }; //Missing 11

    for (int i = 0; i < 14; i++) {
        // Add some elements to the tree
        parcTreeMap_Put(tree1, data->k[idx1[i]], data->v[idx1[i]]);
        parcTreeMap_Put(tree2, data->k[idx1[i]], data->v[idx1[i]]);
    }

    parcTreeMap_Put(tree1, data->k[11], data->v[11]);


    PARCIterator *it = parcTreeMap_CreateKeyValueIterator(tree1);
    for (int idx = 1; parcIterator_HasNext(it); ++idx) {
        parcIterator_Next(it);
        if (idx == 11) {
            parcIterator_Remove(it);
        }
    }
    parcIterator_Release(&it);

    assertTrue(parcTreeMap_Equals(tree1, tree2), "Expect the trees to be equal after remove.");
}

LONGBOW_TEST_FIXTURE(Local)
{
    //LONGBOW_RUN_TEST_CASE(Local, PARC_TreeMap_EnsureRemaining_NonEmpty);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Stress)
{
    // LongBow could use a command line option to enable/disable tests
    // See LongBow issue #5
    if (getenv("LongBowStress")) {
        LONGBOW_RUN_TEST_CASE(Stress, PARC_TreeMap_ExerciseRandomSeededSmall);
        LONGBOW_RUN_TEST_CASE(Stress, PARC_TreeMap_ExerciseRandomSeeded);
    }
}

LONGBOW_TEST_FIXTURE_SETUP(Stress)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Stress)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Stress, PARC_TreeMap_ExerciseRandomSeededSmall)
{
    unsigned seed;
    char *seedString;

    seedString = getenv("RBSeed");
    if (seedString) {
        seed = (unsigned) atol(seedString);
    } else {
        seed = 4179329122; // known to fail
    }

    for (int j = 0; j < 1; j++) {
        // this test case should obtain a seed and number of iterations from a
        // command line option once LongBow has that feature available

        srandom(seed);
        PARCTreeMap *tree = parcTreeMap_Create();

        int inserts = 0;
        int deletes = 0;

        for (int i = 0; i < 100; i++) {
            intptr_t item = 1 + (random() % 100);
            int operation = random() % 1000;
            if (operation < 400) {
                inserts++;
                parcTreeMap_Put(tree, (void *) item, (void *) (item << 8));
            } else {
                deletes++;
                parcTreeMap_Remove(tree, (void *) item);
            }
            rbCheckTree(tree);
        }

        parcTreeMap_Release(&tree);
    }
}

LONGBOW_TEST_CASE(Stress, PARC_TreeMap_ExerciseRandomSeeded)
{
    PARCTreeMap *tree1;
    unsigned seed;
    char *seedString;

    // this test case should obtain a seed and number of iterations from a
    // command line option once LongBow has that feature available
    seedString = getenv("RBSeed");
    if (seedString) {
        seed = (unsigned) atol(seedString);
    } else {
        seed = 4179329122; // known to fail
    }

    srandom(seed);

    tree1 = parcTreeMap_Create();

    int inserts = 0;
    int deletes = 0;

    for (int i = 0; i < 100000; i++) {
        intptr_t item = 1 + (random() % 10000);
        int operation = random() % 1000;
        if (operation < 400) {
            inserts++;
            parcTreeMap_Put(tree1, (void *) item, (void *) (item << 8));
        } else {
            deletes++;
            parcTreeMap_Remove(tree1, (void *) item);
        }
        rbCheckTree(tree1);
    }

    parcTreeMap_Release(&tree1);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARC_TreeMap);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
