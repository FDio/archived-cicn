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

#include "../parc_TreeRedBlack.c"


void *
keyNewInt(int key)
{
    int *newKey = parcMemory_Allocate(sizeof(int));
    assertNotNull(newKey, "parcMemory_Allocate(%zu) returned NULL", sizeof(int));
    *newKey = key;
    return newKey;
}

void *
valueNewInt(int value)
{
    int *newValue = parcMemory_Allocate(sizeof(int));
    assertNotNull(newValue, "parcMemory_Allocate(%zu) returned NULL", sizeof(int));
    *newValue = value;
    return newValue;
}

void *
keyCopy(const void *key)
{
    return keyNewInt(*(int *) key);
}

void *
valueCopy(const void *value)
{
    return valueNewInt(*(int *) value);
}

void *
keyNew(char *key)
{
    return parcMemory_StringDuplicate(key, strlen(key));
}

void *
valueNew(char *value)
{
    return parcMemory_StringDuplicate(value, strlen(value));
}

int
intComp(const void *key1, const void *key2)
{
    if (*(int *) key1 < *(int *) key2) {
        return -1;
    }
    if (*(int *) key1 == *(int *) key2) {
        return 0;
    }
    return 1;
}

bool
intEquals(const void *int1, const void *int2)
{
    return intComp(int1, int2) == 0;
}

int
pointerComp(const void *key1, const void *key2)
{
    if (key1 < key2) {
        return -1;
    }
    if (key1 == key2) {
        return 0;
    }
    return 1;
}

int
stringComp(const void *key1, const void *key2)
{
    // We assume all keys are strings.
    return strcmp(key1, key2);
}

void
keyFree(void **value)
{
    parcMemory_Deallocate((void **) value);
    *value = NULL;
}

void
valueFree(void **key)
{
    parcMemory_Deallocate((void **) key);
    *key = NULL;
}


LONGBOW_TEST_RUNNER(PARC_TreeRedBlack)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(Stress);
}

LONGBOW_TEST_RUNNER_SETUP(PARC_TreeRedBlack)
{
    int seed = (int) time(NULL);
    printf("Seed = %u\n", seed);
    srandom(seed);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(PARC_TreeRedBlack)
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
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Remove_Ordered);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Create);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Insert_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Insert_Ordered);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Insert_OutOfOrder);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Insert_Overwrite);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_DestroyTillEmpty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Size_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Size);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Size_Overwrite);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_EmptyTree);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_NonExistent);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_Biggest);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_Smallest);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_FirstKey);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_LastKey);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_FirstKey_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Get_LastKey_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Remove_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Remove);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Remove_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Remove_NonExistent);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy_First);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy_Last);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy_NonExistent);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Remove_WithSuccessorNonRoot);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Remove_LeftChildRightChild);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Keys);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Values);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Empty);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Equals);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Func);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Equals_DifferentLength);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Not_Values);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Not_Values_Func);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Not_Keys);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Copy);
    LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_Copy_Direct);
    //LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_ExerciseRandom);
    //LONGBOW_RUN_TEST_CASE(Global, PARC_TreeRedBlack_ExerciseRootFailure);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
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
strComp(const void *s1, const void *s2)
{
    return strcmp((char *) s1, (char *) s2);
}

static
bool
strEquals(const void  *s1, const void *s2)
{
    return strcmp(s1, s2) == 0;
}

static
int
recursiveCheckBlackDepth(const PARCTreeRedBlack *tree, const Node *node)
{
    assertNotNull(tree, "Null tree?\n");
    assertNotNull(node, "Null node?\n");
    if (node == tree->nil) {
        return 0;
    }
    int right_depth = recursiveCheckBlackDepth(tree, node->right_child);
    int left_depth = recursiveCheckBlackDepth(tree, node->left_child);
    assertTrue(right_depth == left_depth, "Wrong depth!!\n");
    if (_rbNodeColor(node) == BLACK) {
        return right_depth + 1;
    }
    return right_depth;
}

static
void
rbCheckTree(const PARCTreeRedBlack *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    //printf("--- TREE ---\n");
    _rbNodeAssertTreeInvariants(tree);
    if (tree->size > 0) {
        //_rbNodeRecursiveRun((PARCTreeRedBlack *)tree,tree->root,rbCheckNode,(void *)tree);
        recursiveCheckBlackDepth(tree, tree->root);
    }
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Remove_Ordered)
{
    char *insertList[16] = {
        "01",
        "02",
        "03",
        "04",
        "05",
        "06",
        "07",
        "08",
        "09",
        "10",
        "11",
        "12",
        "13",
        "14",
        "15",
        "16"
    };

    PARCTreeRedBlack *tree1;

    tree1 = parcTreeRedBlack_Create(strComp, NULL, NULL, strEquals, NULL, NULL);

    for (int i = 0; i < 16; i++) {
        parcTreeRedBlack_Insert(tree1, insertList[i], insertList[i]);
    }

    for (int i = 0; i < 14; i++) {
        //rbPrintTreeString(tree1);
        void *data = parcTreeRedBlack_Remove(tree1, insertList[i]);
        assertNotNull(data, "Data is null!");
    }

    parcTreeRedBlack_Destroy(&tree1);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Create)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(stringComp, NULL, NULL, NULL, NULL, NULL);

    parcTreeRedBlack_Destroy(&tree);

    tree = parcTreeRedBlack_Create(stringComp, keyFree, NULL, NULL, valueFree, NULL);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Insert_Destroy)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(stringComp, keyFree, NULL, NULL, valueFree, NULL);

    void *value1 = valueNew("value 1");
    void *key1 = keyNew("1");
    void *value2 = valueNew("value 2");
    void *key2 = keyNew("2");
    void *value3 = valueNew("value 3");
    void *key3 = keyNew("3");

    parcTreeRedBlack_Insert(tree, key1, value1);
    parcTreeRedBlack_Insert(tree, key2, value2);
    parcTreeRedBlack_Insert(tree, key3, value3);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Insert_Overwrite)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(stringComp, keyFree, NULL, NULL, valueFree, NULL);

    parcTreeRedBlack_Insert(tree, keyNew("1"), valueNew("v1"));
    parcTreeRedBlack_Insert(tree, keyNew("2"), valueNew("v2"));
    parcTreeRedBlack_Insert(tree, keyNew("3"), valueNew("v3"));
    parcTreeRedBlack_Insert(tree, keyNew("3"), valueNew("v4"));
    parcTreeRedBlack_Insert(tree, keyNew("3"), valueNew("v5"));

    assertTrue(3 == parcTreeRedBlack_Size(tree), "Wrong size of tree should stay at 3");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Insert_Ordered)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    parcTreeRedBlack_Insert(tree, (void *) 1, (void *) 1001);
    parcTreeRedBlack_Insert(tree, (void *) 2, (void *) 1002);
    parcTreeRedBlack_Insert(tree, (void *) 3, (void *) 1003);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Insert_OutOfOrder)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    parcTreeRedBlack_Insert(tree, (void *) 4, (void *) 1004);
    parcTreeRedBlack_Insert(tree, (void *) 2, (void *) 1002);
    parcTreeRedBlack_Insert(tree, (void *) 3, (void *) 1003);
    parcTreeRedBlack_Insert(tree, (void *) 1, (void *) 1001);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Size_Empty)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    assertTrue(0 == parcTreeRedBlack_Size(tree), "Wrong size of tree - empty, start");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Size)
{
    PARCTreeRedBlack *tree;
    size_t size;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    parcTreeRedBlack_Insert(tree, (void *) 4, (void *) 1004);
    parcTreeRedBlack_Insert(tree, (void *) 2, (void *) 1002);
    parcTreeRedBlack_Insert(tree, (void *) 3, (void *) 1003);

    assertTrue(3 == parcTreeRedBlack_Size(tree), "Wrong size of tree after add 3");

    parcTreeRedBlack_Insert(tree, (void *) 1, (void *) 1001);

    assertTrue(4 == parcTreeRedBlack_Size(tree), "Wrong size of tree after add 1 more");

    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 2);

    size = parcTreeRedBlack_Size(tree);

    assertTrue(3 == size, "Wrong size of tree after 1 delete (%zu instead of 3)", size);

    parcTreeRedBlack_Insert(tree, (void *) 7, (void *) 1007);

    assertTrue(4 == parcTreeRedBlack_Size(tree), "Wrong size of tree after add 1 more");

    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 3);
    assertTrue(3 == parcTreeRedBlack_Size(tree), "Wrong size of tree after del 1 more - 3");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_DestroyTillEmpty)
{
    PARCTreeRedBlack *tree;
    size_t size;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    parcTreeRedBlack_Insert(tree, (void *) 4, (void *) 1004);
    parcTreeRedBlack_Insert(tree, (void *) 2, (void *) 1002);
    parcTreeRedBlack_Insert(tree, (void *) 3, (void *) 1003);
    parcTreeRedBlack_Insert(tree, (void *) 1, (void *) 1001);
    parcTreeRedBlack_Insert(tree, (void *) 5, (void *) 1001);
    parcTreeRedBlack_Insert(tree, (void *) 7, (void *) 1001);
    parcTreeRedBlack_Insert(tree, (void *) 6, (void *) 1001);

    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 3);
    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 1);
    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 4);
    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 2);
    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 6);
    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 5);
    parcTreeRedBlack_RemoveAndDestroy(tree, (void *) 7);

    size = parcTreeRedBlack_Size(tree);

    assertTrue(0 == size, "Wrong size of tree - empty (got %zu)", size);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Size_Overwrite)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    parcTreeRedBlack_Insert(tree, (void *) 4, (void *) 1004);
    parcTreeRedBlack_Insert(tree, (void *) 2, (void *) 1002);
    parcTreeRedBlack_Insert(tree, (void *) 3, (void *) 1003);

    // Size is 3 here, we'll insert the same number now..

    parcTreeRedBlack_Insert(tree, (void *) 3, (void *) 1033);

    assertTrue(3 == parcTreeRedBlack_Size(tree), "Wrong size of tree after overwrite");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_EmptyTree)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    void *value = parcTreeRedBlack_Get(tree, (void *) 1);

    assertTrue(NULL == value, "Object did not exist, must return NULL");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_NonExistent)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }

    void *value = parcTreeRedBlack_Get(tree, (void *) 100);

    assertTrue(NULL == value, "Object did not exist, must return NULL");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_First)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < 4; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }

    void *value = parcTreeRedBlack_Get(tree, (void *) 1);

    assertTrue((1 << 8) == (long) value, "Wrong value, got %ld", (long) value);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }

    void *value = parcTreeRedBlack_Get(tree, (void *) 4);

    assertTrue((4 << 8) == (long) value, "Wrong value, got %ld", (long) value);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_Last)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }

    void *value = parcTreeRedBlack_Get(tree, (void *) 9);

    assertTrue((9 << 8) == (long) value, "Wrong value, got %ld", (long) value);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_Smallest)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }


    void *value = parcTreeRedBlack_Get(tree, (void *) 1);

    assertTrue((1 << 8) == (long) value, "Wrong value, got %ld", (long) value);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_Biggest)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }


    void *value = parcTreeRedBlack_Get(tree, (void *) 39);

    assertTrue((39 << 8) == (long) value, "Wrong value, got %ld", (long) value);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_FirstKey)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }

    void *key = parcTreeRedBlack_FirstKey(tree);

    assertTrue(1 == (long) key, "Wrong value, got %ld", (long) key);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_FirstKey_Empty)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    void *key = parcTreeRedBlack_FirstKey(tree);

    assertNull(key, "Should get NULL on empty tree");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_LastKey_Empty)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    void *key = parcTreeRedBlack_LastKey(tree);

    assertNull(key, "Should get NULL on empty tree");

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Get_LastKey)
{
    PARCTreeRedBlack *tree;

    tree = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree, (void *) i, (void *) (i << 8));
    }

    void *key = parcTreeRedBlack_LastKey(tree);

    assertTrue(39 == (long) key, "Wrong value, got %ld", (long) key);

    parcTreeRedBlack_Destroy(&tree);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Remove_First)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(intComp, keyFree, NULL, intEquals, valueFree, NULL);
    tree2 = parcTreeRedBlack_Create(intComp, keyFree, NULL, intEquals, valueFree, NULL);

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }

    parcTreeRedBlack_Insert(tree1, keyNewInt(1), valueNewInt(1 << 8));

    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }

    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }

    int searchKey = 1;

    void *data = parcTreeRedBlack_Remove(tree1, &searchKey);

    valueFree(&data);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Remove)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(intComp, keyFree, NULL, intEquals, valueFree, NULL);
    tree2 = parcTreeRedBlack_Create(intComp, keyFree, NULL, intEquals, valueFree, NULL);

    for (int i = 31; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }

    parcTreeRedBlack_Insert(tree1, keyNewInt(30), valueNewInt(31 << 8));

    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }

    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }

    int searchKey = 30;

    void *data = parcTreeRedBlack_Remove(tree1, &searchKey);

    valueFree(&data);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Remove_Last)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(intComp, keyFree, NULL, intEquals, valueFree, NULL);
    tree2 = parcTreeRedBlack_Create(intComp, keyFree, NULL, intEquals, valueFree, NULL);

    for (int i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }
    parcTreeRedBlack_Insert(tree1, keyNewInt(100), valueNewInt(100 << 8));
    for (int i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }
    for (int i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, keyNewInt(i), valueNewInt(i << 8));
        parcTreeRedBlack_Insert(tree2, keyNewInt(i), valueNewInt(i << 8));
    }

    int searchKey = 100;

    void *data = parcTreeRedBlack_Remove(tree1, &searchKey);

    valueFree(&data);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy_First)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }

    parcTreeRedBlack_Insert(tree1, (void *) 1, (void *) (1 << 8));

    for (long i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }

    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 1);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 31; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }

    parcTreeRedBlack_Insert(tree1, (void *) 30, (void *) (30 << 8));

    for (long i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }

    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 30);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Remove_NonExistent)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    for (long i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }

    void *element = parcTreeRedBlack_Remove(tree1, (void *) 100);

    assertNull(element, "Return value must be NULL on non existing element");
    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy_NonExistent)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    for (long i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }

    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 100);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Remove_WithSuccessorNonRoot)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    long insert1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };
    long insert2[13] = { 8, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);



    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) insert1[i], (void *) (insert1[i] << 8));
    }

    for (int i = 0; i < 13; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree2, (void *) insert2[i], (void *) (insert2[i] << 8));
    }

    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 4);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 12);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Remove_LeftChildRightChild)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    long insert1[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };
    long insert2[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);



    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) insert1[i], (void *) (insert1[i] << 8));
    }

    for (int i = 0; i < 15; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree2, (void *) insert2[i], (void *) (insert2[i] << 8));
    }

    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 13);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 7);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 14);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 6);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 15);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 12);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 11);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 10);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 9);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 8);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 5);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 4);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 3);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 2);
    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 1);

    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 1);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 2);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 3);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 4);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 5);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 6);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 7);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 8);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 9);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 10);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 11);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 12);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 13);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 14);
    parcTreeRedBlack_RemoveAndDestroy(tree2, (void *) 15);

    //assertTrue(parcTreeRedBlack_Equals(tree1,tree2),"Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_RemoveAndDestroy_Last)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 30; i < 40; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    parcTreeRedBlack_Insert(tree1, (void *) 100, (void *) (100 << 8));
    for (long i = 2; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }
    for (long i = 20; i < 30; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2, (void *) i, (void *) (i << 8));
    }

    parcTreeRedBlack_RemoveAndDestroy(tree1, (void *) 100);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Trees dont match after remove");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Keys)
{
    PARCTreeRedBlack *tree1;
    PARCArrayList *list;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    list = parcArrayList_Create(NULL);

    // Insert in tree out of order
    for (long i = 10; i < 20; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
    }

    // Insert in list in order
    for (long i = 1; i < 20; i++) {
        // Add some elements to the tree
        parcArrayList_Add(list, (void *) i);
    }

    PARCArrayList *keys = parcTreeRedBlack_Keys(tree1);

    assertTrue(parcArrayList_Equals(list, keys), "Key list doesnt' match");

    parcArrayList_Destroy(&keys);
    parcArrayList_Destroy(&list);
    parcTreeRedBlack_Destroy(&tree1);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Values)
{
    PARCTreeRedBlack *tree1;
    PARCArrayList *list;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    list = parcArrayList_Create(NULL);

    // Insert in tree out of order
    for (long i = 10; i < 20; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
    }
    for (long i = 1; i < 10; i++) {
        // Add some elements to the tree
        parcTreeRedBlack_Insert(tree1, (void *) i, (void *) (i << 8));
    }

    // Insert in list in order
    for (long i = 1; i < 20; i++) {
        // Add some elements to the tree
        parcArrayList_Add(list, (void *) (i << 8));
    }

    PARCArrayList *values = parcTreeRedBlack_Values(tree1);

    assertTrue(parcArrayList_Equals(list, values), "Key list doesnt' match");

    parcArrayList_Destroy(&values);
    parcArrayList_Destroy(&list);
    parcTreeRedBlack_Destroy(&tree1);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Empty)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Empty lists are not equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Equals_DifferentLength)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 100;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < compareInserts; i++) {
        parcTreeRedBlack_Insert(tree1,
                                (void *) i,
                                (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2,
                                (void *) (compareInserts - i),
                                (void *) ((compareInserts - i) << 8));
    }
    parcTreeRedBlack_Insert(tree2, (void *) (1000), (void *) ((12304) << 8));

    assertFalse(parcTreeRedBlack_Equals(tree1, tree2), "Lists are equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Not_Values)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 100;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < compareInserts; i++) {
        parcTreeRedBlack_Insert(tree1,
                                (void *) i,
                                (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2,
                                (void *) (compareInserts - i),
                                (void *) ((compareInserts + i) << 8));
    }

    assertFalse(parcTreeRedBlack_Equals(tree1, tree2), "Lists are equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Not_Values_Func)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 100;

    tree1 = parcTreeRedBlack_Create(pointerComp, keyFree, keyCopy, intEquals, valueFree, valueCopy);
    tree2 = parcTreeRedBlack_Create(pointerComp, keyFree, keyCopy, intEquals, valueFree, valueCopy);

    for (int i = 1; i < compareInserts; i++) {
        parcTreeRedBlack_Insert(tree1,
                                keyNewInt(i),
                                valueNewInt(i + 1000));
        parcTreeRedBlack_Insert(tree2,
                                keyNewInt(i),
                                valueNewInt(i + 2000));
    }

    assertFalse(parcTreeRedBlack_Equals(tree1, tree2), "Lists are equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Not_Keys)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 100;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < compareInserts; i++) {
        parcTreeRedBlack_Insert(tree1,
                                (void *) i,
                                (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2,
                                (void *) (compareInserts + i),
                                (void *) ((compareInserts - i) << 8));
    }

    assertFalse(parcTreeRedBlack_Equals(tree1, tree2), "Lists are equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Equals)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 100;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);
    tree2 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < compareInserts; i++) {
        parcTreeRedBlack_Insert(tree1,
                                (void *) i,
                                (void *) (i << 8));
        parcTreeRedBlack_Insert(tree2,
                                (void *) (compareInserts - i),
                                (void *) ((compareInserts - i) << 8));
    }

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Lists are not equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Equals_Func)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 100;

    tree1 = parcTreeRedBlack_Create(intComp, keyFree, keyCopy, intEquals, valueFree, valueCopy);
    tree2 = parcTreeRedBlack_Create(intComp, keyFree, keyCopy, intEquals, valueFree, valueCopy);

    for (int i = 1; i < compareInserts; i++) {
        parcTreeRedBlack_Insert(tree1,
                                keyNewInt(i),
                                valueNewInt(i + 1000));
        parcTreeRedBlack_Insert(tree2,
                                keyNewInt(i),
                                valueNewInt(i + 1000));
    }

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Lists are not equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Copy)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 20;

    tree1 = parcTreeRedBlack_Create(intComp, keyFree, keyCopy, intEquals, valueFree, valueCopy);

    for (int i = 1; i < compareInserts; i++) {
        void *key = keyNewInt(i);
        void *value = valueNewInt(i + 1000);
        //value = (void *) &((*(int*)value) + 100);
        parcTreeRedBlack_Insert(tree1, key, value);
    }

    tree2 = parcTreeRedBlack_Copy(tree1);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Lists are not equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_CASE(Global, PARC_TreeRedBlack_Copy_Direct)
{
    PARCTreeRedBlack *tree1;
    PARCTreeRedBlack *tree2;

    int compareInserts = 20;

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    for (long i = 1; i < compareInserts; i++) {
        parcTreeRedBlack_Insert(tree1,
                                (void *) i,
                                (void *) (i << 8));
    }

    tree2 = parcTreeRedBlack_Copy(tree1);

    assertTrue(parcTreeRedBlack_Equals(tree1, tree2), "Lists are not equal");

    parcTreeRedBlack_Destroy(&tree1);
    parcTreeRedBlack_Destroy(&tree2);
}

LONGBOW_TEST_FIXTURE(Local)
{
    //LONGBOW_RUN_TEST_CASE(Local, PARC_TreeRedBlack_EnsureRemaining_NonEmpty);
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
        LONGBOW_RUN_TEST_CASE(Stress, PARC_TreeRedBlack_ExerciseRandomSeededSmall);
        LONGBOW_RUN_TEST_CASE(Stress, PARC_TreeRedBlack_ExerciseRandomSeeded);
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

LONGBOW_TEST_CASE(Stress, PARC_TreeRedBlack_ExerciseRandomSeededSmall)
{
    PARCTreeRedBlack *tree1;
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

        printf("Random seed %u\n", seed);

        srandom(seed);
        tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

        int inserts = 0;
        int deletes = 0;

        for (int i = 0; i < 100; i++) {
            intptr_t item = 1 + (random() % 100);
            int operation = random() % 1000;
            if (operation < 400) {
                inserts++;
                parcTreeRedBlack_Insert(tree1, (void *) item, (void *) (item << 8));
            } else {
                deletes++;
                parcTreeRedBlack_Remove(tree1, (void *) item);
            }
            rbCheckTree(tree1);
        }

        parcTreeRedBlack_Destroy(&tree1);
    }
}

LONGBOW_TEST_CASE(Stress, PARC_TreeRedBlack_ExerciseRandomSeeded)
{
    PARCTreeRedBlack *tree1;
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
    printf("Random seed %u\n", seed);

    srandom(seed);

    tree1 = parcTreeRedBlack_Create(pointerComp, NULL, NULL, NULL, NULL, NULL);

    int inserts = 0;
    int deletes = 0;

    for (int i = 0; i < 100000; i++) {
        intptr_t item = 1 + (random() % 10000);
        int operation = random() % 1000;
        if (operation < 400) {
            inserts++;
            parcTreeRedBlack_Insert(tree1, (void *) item, (void *) (item << 8));
        } else {
            deletes++;
            parcTreeRedBlack_Remove(tree1, (void *) item);
        }
        rbCheckTree(tree1);
    }

    parcTreeRedBlack_Destroy(&tree1);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARC_TreeRedBlack);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
