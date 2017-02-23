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

#include <LongBow/runtime.h>

#include <stdio.h>

#include "parc_TreeMap.h"
#include "parc_ArrayList.h"
#include "parc_KeyValue.h"

#include <parc/algol/parc_Memory.h>

#define RED   1
#define BLACK 0

#define ASSERT_INVARIANTS

struct treemap_node;
typedef struct treemap_node _RBNode;

struct treemap_node {
    _RBNode *leftChild;
    _RBNode *rightChild;
    _RBNode *parent;
    PARCKeyValue *element;
    int color;
};

struct parc_treemap {
    _RBNode *root;
    _RBNode *nil;
    int size;
    PARCTreeMap_CustomCompare *customCompare;
};

typedef void (rbRecursiveFunc)(_RBNode *node, PARCObject *data);

static void
_rbNodeFree(_RBNode *node)
{
    if (node->element != NULL) {
        parcKeyValue_Release(&(node->element));
    }
    parcMemory_Deallocate((void **) &node);
}

static void
_rbNodeFreeRecursive(PARCTreeMap *tree, _RBNode *node)
{
    if (node->leftChild != tree->nil) {
        _rbNodeFreeRecursive(tree, node->leftChild);
    }
    if (node->rightChild != tree->nil) {
        _rbNodeFreeRecursive(tree, node->rightChild);
    }
    // We descended on both branches, now free myself.
    _rbNodeFree(node);
    tree->size--;
}

// Run a function on all nodes in the tree, in order
static void
_rbNodeRecursiveRun(PARCTreeMap *tree, _RBNode *node, rbRecursiveFunc *func, PARCObject *data)
{
    if (node->leftChild != tree->nil) {
        _rbNodeRecursiveRun(tree, node->leftChild, func, data);
    }
    func(node, data);
    if (node->rightChild != tree->nil) {
        _rbNodeRecursiveRun(tree, node->rightChild, func, data);
    }
}


static _RBNode *
_rbMinRelativeNode(const PARCTreeMap *tree, _RBNode *startNode)
{
    _RBNode *searchNode = startNode;

    // Let's get to the bottom left
    while (searchNode->leftChild != tree->nil) {
        searchNode = searchNode->leftChild;
    }

    return searchNode;
}

static _RBNode *
_rbMaxRelativeNode(const PARCTreeMap *tree, _RBNode *startNode)
{
    _RBNode *searchNode = startNode;

    // Let's get to the bottom left
    while (searchNode->rightChild != tree->nil) {
        searchNode = searchNode->rightChild;
    }

    return searchNode;
}

static _RBNode *
_rbNextNode(const PARCTreeMap *tree, _RBNode *node)
{
    _RBNode *searchNode = node;
    if (searchNode->rightChild != tree->nil) {
        searchNode = _rbMinRelativeNode(tree, searchNode->rightChild);
    } else {
        _RBNode *parent = searchNode->parent;
        while (parent != tree->nil) {
            if (parent->leftChild == searchNode) {
                break;
            }
            searchNode = parent;
            parent = searchNode->parent;
        }
        searchNode = parent;
    }

    return searchNode;
}

static _RBNode *
_rbPreviousNode(const PARCTreeMap *tree, _RBNode *node)
{
    _RBNode *searchNode = node;
    if (searchNode->leftChild != tree->nil) {
        searchNode = _rbMaxRelativeNode(tree, searchNode->leftChild);
    } else {
        _RBNode *parent = searchNode->parent;
        while (parent != tree->nil) {
            if (parent->rightChild == searchNode) {
                break;
            }
            searchNode = parent;
            parent = searchNode->parent;
        }
        searchNode = parent;
    }

    return searchNode;
}


/**
 * Create a node
 * Set the parent and children to tree->nil.
 * If we are creating the nil node this might leave garbage there (if not preset to NULL).
 */
static _RBNode *
_rbNodeCreate(PARCTreeMap *tree, int color)
{
    _RBNode *node = parcMemory_AllocateAndClear(sizeof(_RBNode));
    assertNotNull(node, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_RBNode));
    node->color = color;
    node->leftChild = tree->nil;
    node->rightChild = tree->nil;
    node->parent = tree->nil;
    return node;
}

static void
_rbNodeSetColor(_RBNode *node, uint8_t color)
{
    node->color = color;
}

static int
_rbNodeColor(const _RBNode *node)
{
    return node->color;
}

static bool
_rbNodeIsEqual(const PARCTreeMap *tree, const _RBNode *node, const PARCObject *key)
{
    bool result = false;
    if (node->element != NULL) {
        if (tree->customCompare != NULL) {
            result = (tree->customCompare(parcKeyValue_GetKey(node->element), key) == 0);
        } else {
            result = parcObject_Equals(parcKeyValue_GetKey(node->element), key);
        }
    }
    return result;
}

static bool
_rbNodeIsGreaterThan(const PARCTreeMap *tree, const _RBNode *node, const PARCObject *key)
{
    bool result = false;
    if (node->element != NULL) {
        if (tree->customCompare != NULL) {
            result = (tree->customCompare(parcKeyValue_GetKey(node->element), key) > 0);
        } else {
            result = (parcObject_Compare(parcKeyValue_GetKey(node->element), key) > 0);
        }
    }
    return result;
}

static _RBNode *
_rbFindNode(const PARCTreeMap *tree, _RBNode *startNode, const PARCObject *key)
{
    _RBNode *result = NULL;
    _RBNode *node = startNode;

    // Let's get to the bottom of the tree to insert.
    while (node != tree->nil) {
        if (_rbNodeIsEqual(tree, node, key)) {
            result = node;
            break;
        } else {
            if (_rbNodeIsGreaterThan(tree, node, key)) {
                node = node->leftChild;
            } else {
                node = node->rightChild;
            }
        }
    }
    return result;
}


static void
_rbNodeUpdate(_RBNode *treeNode, _RBNode *newNode)
{
    // Free old values
    if (treeNode->element != NULL) {
        parcKeyValue_Release(&treeNode->element);
    }

    treeNode->element = parcKeyValue_Acquire(newNode->element);
    _rbNodeFree(newNode);
}

static void
_rbNodeRotateLeft(PARCTreeMap *tree, _RBNode *node)
{
    _RBNode *subroot = node->rightChild;
    node->rightChild = subroot->leftChild;
    if (node->rightChild != tree->nil) {
        node->rightChild->parent = node;
    }

    subroot->parent = node->parent;
    if (tree->root == node) {
        tree->root = subroot;
    } else {
        if (subroot->parent->leftChild == node) {
            // node was a left child
            subroot->parent->leftChild = subroot;
        } else {
            // node was a right child
            subroot->parent->rightChild = subroot;
        }
    }

    subroot->leftChild = node;
    node->parent = subroot;
}

static void
_rbNodeRotateRight(PARCTreeMap *tree, _RBNode *node)
{
    _RBNode *subroot = node->leftChild;
    node->leftChild = subroot->rightChild;
    if (node->leftChild != tree->nil) {
        node->leftChild->parent = node;
    }

    subroot->parent = node->parent;
    if (tree->root == node) {
        tree->root = subroot;
    } else {
        if (subroot->parent->leftChild == node) {
            // node was a left child
            subroot->parent->leftChild = subroot;
        } else {
            // node was a right child
            subroot->parent->rightChild = subroot;
        }
    }

    subroot->rightChild = node;
    node->parent = subroot;
}

static void
_rbNodeFix(PARCTreeMap *tree, _RBNode *startNode)
{
    _RBNode *node = startNode;
    _RBNode *uncle;
    while (_rbNodeColor(node->parent) == RED) {
        if (node->parent->parent->leftChild == node->parent) {
            uncle = node->parent->parent->rightChild;
            if (_rbNodeColor(uncle) == RED) {
                // My dad and uncle are red. Switch dad to black.
                // Switch grandpa to red and start there.
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(uncle, BLACK);
                _rbNodeSetColor(node->parent->parent, RED);
                node = node->parent->parent;
            } else {
                if (node->parent->rightChild == node) {
                    node = node->parent;
                    _rbNodeRotateLeft(tree, node);
                }
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(node->parent->parent, RED);
                _rbNodeRotateRight(tree, node->parent->parent);
            }
        } else {
            uncle = node->parent->parent->leftChild;
            if (_rbNodeColor(uncle) == RED) {
                // My dad and uncle are red. Switch dad to black.
                // Switch grandpa to red and start there.
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(uncle, BLACK);
                _rbNodeSetColor(node->parent->parent, RED);
                node = node->parent->parent;
            } else {
                if (node->parent->leftChild == node) {
                    node = node->parent;
                    _rbNodeRotateRight(tree, node);
                }
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(node->parent->parent, RED);
                _rbNodeRotateLeft(tree, node->parent->parent);
            }
        }
    }
    _rbNodeSetColor(tree->root, BLACK);
}

static void
_rbNodeAssertNodeInvariants(_RBNode *node, PARCObject *data)
{
    PARCTreeMap *tree = (PARCTreeMap *) data;
    assertNotNull(node->parent, "Node has NULL parent");
    assertNotNull(node->leftChild, "Left child NULL");
    assertNotNull(node->rightChild, "Richt child NULL");
    if (node != tree->root) {
        assertTrue(node->parent != tree->nil, "Paren't can't be nill for node!");
        // Don't need to compare to parent, they compared to us
    }
    assertNotNull(node->element, "We have a null element!!");
    assertNotNull(parcKeyValue_GetKey(node->element), "We have a null key!!");
    assertNotNull(parcKeyValue_GetValue(node->element), "We have a null value!!");
    if (node->leftChild != tree->nil) {
        if (tree->customCompare != NULL) {
            assertTrue(tree->customCompare(parcKeyValue_GetKey(node->element), parcKeyValue_GetKey(node->leftChild->element)) > 0, "Left child not smaller?");
        } else {
            assertTrue(parcObject_Compare(parcKeyValue_GetKey(node->element), parcKeyValue_GetKey(node->leftChild->element)) > 0, "Left child not smaller?");
        }
    }
    if (node->rightChild != tree->nil) {
        if (tree->customCompare != NULL) {
            assertTrue(tree->customCompare(parcKeyValue_GetKey(node->element), parcKeyValue_GetKey(node->rightChild->element)) < 0, "Right child not bigger?");
        } else {
            assertTrue(parcObject_Compare(parcKeyValue_GetKey(node->element), parcKeyValue_GetKey(node->rightChild->element)) < 0, "Right child not bigger?");
        }
    }
}

static
void
_rbNodeAssertTreeInvariants(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree is null!");
    assertTrue(tree->size >= 0, "Tree has negative size");
    if (tree->size != 0) {
        assertTrue(tree->root != tree->nil, "Tree size = %d > 0 but root is nil", tree->size);
        assertNotNull(tree->root, "Tree size > 0 but root is NULL");
#ifdef ASSERT_INVARIANTS
        _rbNodeRecursiveRun((PARCTreeMap *) tree, tree->root, _rbNodeAssertNodeInvariants, (PARCObject *) tree);
#endif
    }
}

static void
_rbNodeFixDelete(PARCTreeMap *tree, _RBNode *node)
{
    _RBNode *fixNode;

    while ((node != tree->root) && (_rbNodeColor(node) == BLACK)) {
        _rbNodeAssertTreeInvariants(tree);
        if (node == node->parent->leftChild) {
            fixNode = node->parent->rightChild;
            if (_rbNodeColor(fixNode) == RED) {
                _rbNodeSetColor(fixNode, BLACK);
                _rbNodeSetColor(node->parent, RED);
                _rbNodeRotateLeft(tree, node->parent);
                fixNode = node->parent->rightChild;
            }
            if ((_rbNodeColor(fixNode->leftChild) == BLACK) &&
                (_rbNodeColor(fixNode->rightChild) == BLACK)) {
                _rbNodeSetColor(fixNode, RED);
                node = node->parent;
            } else {
                if (_rbNodeColor(fixNode->rightChild) == BLACK) {
                    _rbNodeSetColor(fixNode->leftChild, BLACK);
                    _rbNodeSetColor(fixNode, RED);
                    _rbNodeRotateRight(tree, fixNode);
                    fixNode = node->parent->rightChild;
                }
                _rbNodeSetColor(fixNode, _rbNodeColor(node->parent));
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(fixNode->rightChild, BLACK);
                _rbNodeRotateLeft(tree, node->parent);
                node = tree->root;
            }
        } else {
            fixNode = node->parent->leftChild;
            if (_rbNodeColor(fixNode) == RED) {
                _rbNodeSetColor(fixNode, BLACK);
                _rbNodeSetColor(node->parent, RED);
                _rbNodeRotateRight(tree, node->parent);
                fixNode = node->parent->leftChild;
            }
            if ((_rbNodeColor(fixNode->leftChild) == BLACK) &&
                (_rbNodeColor(fixNode->rightChild) == BLACK)) {
                _rbNodeSetColor(fixNode, RED);
                node = node->parent;
            } else {
                if (_rbNodeColor(fixNode->leftChild) == BLACK) {
                    _rbNodeSetColor(fixNode->rightChild, BLACK);
                    _rbNodeSetColor(fixNode, RED);
                    _rbNodeRotateLeft(tree, fixNode);
                    fixNode = node->parent->leftChild;
                }
                _rbNodeSetColor(fixNode, _rbNodeColor(node->parent));
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(fixNode->leftChild, BLACK);
                _rbNodeRotateRight(tree, node->parent);
                node = tree->root;
            }
        }
    }

    _rbNodeSetColor(node, BLACK);
}

// Remove the node from the tree.
// The node must be part of a tree (with parents and children)
static void
_rbNodeRemove(PARCTreeMap *tree, _RBNode *node)
{
    _rbNodeAssertTreeInvariants(tree);
    _RBNode *fixupNode;
    int deleteNodeColor = _rbNodeColor(node);
    if (node->leftChild == tree->nil) {
        if (node->rightChild == tree->nil) {
            // ---- We have no children ----
            if (tree->root == node) {
                tree->root = tree->nil;
            } else {
                if (node->parent->leftChild == node) {
                    node->parent->leftChild = tree->nil;
                } else {
                    node->parent->rightChild = tree->nil;
                }
            }
            fixupNode = tree->nil;
            fixupNode->parent = node->parent;
        } else {
            // ---- We only have right child, move up ----
            if (tree->root == node) {
                tree->root = node->rightChild;
            } else {
                if (node->parent->leftChild == node) {
                    node->parent->leftChild = node->rightChild;
                } else {
                    node->parent->rightChild = node->rightChild;
                }
            }
            fixupNode = node->rightChild;
            node->rightChild->parent = node->parent;
        }
    } else {
        if (node->rightChild == tree->nil) {
            // ---- We only have left child, move up ----
            if (tree->root == node) {
                tree->root = node->leftChild;
            } else {
                if (node->parent->leftChild == node) {
                    node->parent->leftChild = node->leftChild;
                } else {
                    node->parent->rightChild = node->leftChild;
                }
            }
            node->leftChild->parent = node->parent;
            fixupNode = node->leftChild;
        } else {
            // ---- We have 2 children, move our successor to our location ----
            _RBNode *successor = node->rightChild;
            while (successor->leftChild != tree->nil) {
                successor = successor->leftChild;
            }
            deleteNodeColor = _rbNodeColor(successor);

            // Remove successor, it has no left child
            if (successor == successor->parent->leftChild) {
                successor->parent->leftChild = successor->rightChild;
            } else {
                successor->parent->rightChild = successor->rightChild;
            }
            successor->rightChild->parent = successor->parent;

            fixupNode = successor->rightChild;

            if (node->parent == tree->nil) {
                tree->root = successor;
            } else if (node->parent->leftChild == node) {
                node->parent->leftChild = successor;
            } else {
                node->parent->rightChild = successor;
            }
            successor->parent = node->parent;
            successor->leftChild = node->leftChild;
            node->leftChild->parent = successor;
            successor->rightChild = node->rightChild;
            node->rightChild->parent = successor;

            _rbNodeSetColor(successor, _rbNodeColor(node));

            if (successor->parent == tree->nil) {
                tree->root = successor;
            }
        }
    }
    tree->size--;

    // Fix the red-blackness
    _rbNodeAssertTreeInvariants(tree);
    if (deleteNodeColor == BLACK) {
        _rbNodeFixDelete(tree, fixupNode);
    }
    _rbNodeAssertTreeInvariants(tree);
}

static void
_parcTreeMap_Destroy(PARCTreeMap **treePointer)
{
    assertNotNull(treePointer, "pointer to pointer to tree can't be null");
    assertNotNull(*treePointer, "pointer to tree can't be null");
    _rbNodeAssertTreeInvariants(*treePointer);

    if ((*treePointer)->size > 0) {
        // If we have any elements in the tree, free them
        _rbNodeFreeRecursive(*treePointer, (*treePointer)->root);
    }

    // Free the nil element
    parcMemory_Deallocate((void **) &((*treePointer)->nil));
}


parcObject_ExtendPARCObject(PARCTreeMap, _parcTreeMap_Destroy, parcTreeMap_Copy, NULL, parcTreeMap_Equals, NULL, NULL, NULL);

parcObject_ImplementAcquire(parcTreeMap, PARCTreeMap);

parcObject_ImplementRelease(parcTreeMap, PARCTreeMap);

PARCTreeMap *
parcTreeMap_CreateCustom(PARCTreeMap_CustomCompare *customCompare)
{
    PARCTreeMap *tree = parcObject_CreateInstance(PARCTreeMap);
    assertNotNull(tree, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCTreeMap));
    tree->nil = _rbNodeCreate(tree, BLACK);
    tree->nil->leftChild = tree->nil;
    tree->nil->rightChild = tree->nil;
    tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->customCompare = customCompare;
    tree->size = 0;
    return tree;
}

PARCTreeMap *
parcTreeMap_Create(void)
{
    return parcTreeMap_CreateCustom(NULL);
}

void
parcTreeMap_Put(PARCTreeMap *tree, const PARCObject *key, const PARCObject *value)
{
    assertNotNull(tree, "Tree can't be NULL");
    assertNotNull(key, "Key can't be NULL");
    assertNotNull(value, "Value can't be NULL");

    _RBNode *newNode = _rbNodeCreate(tree, RED);
    _RBNode *parent = tree->nil;
    _RBNode *node;

    // Set the value for the created node
    PARCKeyValue *element = parcKeyValue_Create(key, value);
    newNode->element = element;

    // Start at the top
    node = tree->root;

    // Let's get to the bottom of the tree to insert.
    while (node != tree->nil) {
        parent = node;
        if (_rbNodeIsEqual(tree, node, key)) {
            // We're trying to insert the same value
            _rbNodeUpdate(node, newNode);
            return;
        } else {
            if (_rbNodeIsGreaterThan(tree, node, key)) {
                // The key is smaller
                node = node->leftChild;
            } else {
                node = node->rightChild;
            }
        }
    }

    // We're at the bottom.
    // node is nil (a leaf)
    newNode->parent = parent;
    if (parent == tree->nil) {
        // nil is our parent, we are the root
        tree->root = newNode;
    } else {
        if (_rbNodeIsGreaterThan(tree, parent, key)) {
            parent->leftChild = newNode;
        } else {
            parent->rightChild = newNode;
        }
    }

    // We have inserted one node.
    tree->size++;

    // We have a correct tree. But we need to regain the red-black property.
    _rbNodeFix(tree, newNode);

    _rbNodeAssertTreeInvariants(tree);
}

PARCObject *
parcTreeMap_Get(PARCTreeMap *tree, const PARCObject *key)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    PARCObject *result = NULL;

    _RBNode *node = _rbFindNode(tree, tree->root, key);

    if (node != NULL) {
        result = parcKeyValue_GetValue(node->element);
    }

    return result;
}

// Return value, remove from tree
PARCObject *
parcTreeMap_Remove(PARCTreeMap *tree, const PARCObject *key)
{
    assertNotNull(tree, "Tree can't be NULL");
    assertNotNull(key, "Key can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    PARCObject *result = NULL;

    _RBNode *node = _rbFindNode(tree, tree->root, key);

    if (node != NULL) {
        _rbNodeRemove(tree, node);
        result = parcObject_Acquire(parcKeyValue_GetValue(node->element));
        _rbNodeFree(node);
    }

    // We didn't find the node

    _rbNodeAssertTreeInvariants(tree);

    return result;
}

// remove from tree and destroy
void
parcTreeMap_RemoveAndRelease(PARCTreeMap *tree, const PARCObject *key)
{
    assertNotNull(tree, "Tree can't be NULL");
    assertNotNull(key, "Key can't be NULL");

    _RBNode *node = _rbFindNode(tree, tree->root, key);

    if (node != NULL) {
        _rbNodeRemove(tree, node);
        _rbNodeFree(node);
    }

    _rbNodeAssertTreeInvariants(tree);
}

PARCKeyValue *
parcTreeMap_GetLastEntry(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);
    _RBNode *node = tree->root;

    if (tree->size == 0) {
        // We don't have any entries
        return NULL;
    }

    // Let's get to the bottom right of the tree to find the largest
    while (node->rightChild != tree->nil) {
        node = node->rightChild;
    }

    return node->element;
}

PARCObject *
parcTreeMap_GetLastKey(const PARCTreeMap *tree)
{
    PARCObject *result = NULL;

    PARCKeyValue *entry = parcTreeMap_GetLastEntry(tree);
    if (entry != NULL) {
        result = parcKeyValue_GetKey(entry);
    }

    return result;
}

PARCKeyValue *
parcTreeMap_GetFirstEntry(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    if (tree->size == 0) {
        // We don't have any entries
        return NULL;
    }

    _RBNode *node = _rbMinRelativeNode(tree, tree->root);

    return node->element;
}

PARCObject *
parcTreeMap_GetFirstKey(const PARCTreeMap *tree)
{
    PARCObject *result = NULL;

    PARCKeyValue *entry = parcTreeMap_GetFirstEntry(tree);
    if (entry != NULL) {
        result = parcKeyValue_GetKey(entry);
    }

    return result;
}

PARCKeyValue *
parcTreeMap_GetHigherEntry(const PARCTreeMap *tree, const PARCObject *key)
{
    PARCKeyValue *result = NULL;

    _RBNode *node = _rbFindNode(tree, tree->root, key);
    if (node != NULL) {
        node = _rbNextNode(tree, node);
        if (node != NULL) {
            result = node->element;
        }
    }

    return result;
}

PARCObject *
parcTreeMap_GetHigherKey(const PARCTreeMap *tree, const PARCObject *key)
{
    PARCObject *result = NULL;

    PARCKeyValue *kv = parcTreeMap_GetHigherEntry(tree, key);
    if (kv != NULL) {
        result = parcKeyValue_GetKey(kv);
    }

    return result;
}

PARCKeyValue *
parcTreeMap_GetLowerEntry(const PARCTreeMap *tree, const PARCObject *key)
{
    PARCKeyValue *result = NULL;

    _RBNode *node = _rbFindNode(tree, tree->root, key);
    if (node != NULL) {
        node = _rbPreviousNode(tree, node);
        if (node != NULL) {
            result = node->element;
        }
    }

    return result;
}

PARCObject *
parcTreeMap_GetLowerKey(const PARCTreeMap *tree, const PARCObject *key)
{
    PARCObject *result = NULL;

    PARCKeyValue *kv = parcTreeMap_GetLowerEntry(tree, key);
    if (kv != NULL) {
        result = parcKeyValue_GetKey(kv);
    }

    return result;
}


size_t
parcTreeMap_Size(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    return tree->size;
}

static void
_rbAddKeyToList(_RBNode *node, PARCList *list)
{
    parcList_Add(list, parcObject_Acquire(parcKeyValue_GetKey(node->element)));
}

static void
_rbAddalueToList(_RBNode *node, PARCList *list)
{
    parcList_Add(list, parcObject_Acquire(parcKeyValue_GetValue(node->element)));
}

static void
_rbAddElementToList(_RBNode *node, PARCList *list)
{
    parcList_Add(list, parcObject_Acquire(node->element));
}

PARCList *
parcTreeMap_AcquireKeys(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    PARCList *keys = parcList(parcArrayList_Create_Capacity((bool (*)(void *x, void *y))parcObject_Equals,
                                                            (void (*)(void **))parcObject_Release, tree->size),
                              PARCArrayListAsPARCList);

    if (tree->size > 0) {
        _rbNodeRecursiveRun((PARCTreeMap *) tree, tree->root, (rbRecursiveFunc *) _rbAddKeyToList, keys);
    }
    return keys;
}

PARCList *
parcTreeMap_AcquireValues(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    PARCList *values = parcList(parcArrayList_Create_Capacity((bool (*)(void *x, void *y))parcObject_Equals,
                                                              (void (*)(void **))parcObject_Release, tree->size),
                                PARCArrayListAsPARCList);

    if (tree->size > 0) {
        _rbNodeRecursiveRun((PARCTreeMap *) tree, tree->root, (rbRecursiveFunc *) _rbAddalueToList, values);
    }
    return values;
}

static PARCList *
_parcTreeMap_Elements(const PARCTreeMap *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    PARCList *elements = parcList(parcArrayList_Create_Capacity((bool (*)(void *x, void *y))parcObject_Equals,
                                                                (void (*)(void **))parcObject_Release, tree->size),
                                  PARCArrayListAsPARCList);

    if (tree->size > 0) {
        _rbNodeRecursiveRun((PARCTreeMap *) tree, tree->root, (rbRecursiveFunc *) _rbAddElementToList, elements);
    }
    return elements;
}

bool
parcTreeMap_Equals(const PARCTreeMap *tree1, const PARCTreeMap *tree2)
{
    _rbNodeAssertTreeInvariants(tree1);
    _rbNodeAssertTreeInvariants(tree2);
    assertNotNull(tree1, "Tree can't be NULL");
    assertNotNull(tree2, "Tree can't be NULL");

    bool result = false;

    PARCList *keys1 = parcTreeMap_AcquireKeys(tree1);
    PARCList *keys2 = parcTreeMap_AcquireKeys(tree2);
    size_t length1 = parcList_Size(keys1);
    size_t length2 = parcList_Size(keys2);

    if (length1 == length2) {
        result = true;
        for (size_t i = 0; i < length1; i++) {
            if (!parcObject_Equals(parcList_GetAtIndex(keys1, i), parcList_GetAtIndex(keys2, i))) {
                result = false;
                break;
            }
        }
        if (result) {
            PARCList *values1 = parcTreeMap_AcquireValues(tree1);
            PARCList *values2 = parcTreeMap_AcquireValues(tree2);
            size_t s1 = parcList_Size(values1);
            size_t s2 = parcList_Size(values2);
            if (s1 == s2) {
                for (size_t i = 0; i < s1; i++) {
                    PARCObject *value1 = parcList_GetAtIndex(values1, i);
                    PARCObject *value2 = parcList_GetAtIndex(values2, i);
                    if (!parcObject_Equals(value1, value2)) {
                        result = false;
                        break;
                    }
                }
            }
            parcList_Release(&values1);
            parcList_Release(&values2);
        }
    }

    parcList_Release(&keys1);
    parcList_Release(&keys2);
    return result;
}


/*
 * This is a simple implementation of Copy that goes through the list of keys and values.
 */
PARCTreeMap *
parcTreeMap_Copy(const PARCTreeMap *sourceTree)
{
    _rbNodeAssertTreeInvariants(sourceTree);
    assertNotNull(sourceTree, "Tree can't be NULL");

    PARCObject *keySource;
    PARCObject *keyCopy;
    PARCObject *valueSource;
    PARCObject *valueCopy;

    PARCTreeMap *treeCopy = parcTreeMap_CreateCustom(sourceTree->customCompare);

    PARCList *keys = parcTreeMap_AcquireKeys(sourceTree);
    PARCList *values = parcTreeMap_AcquireValues(sourceTree);

    size_t total_keys = parcList_Size(keys);

    for (size_t i = 0; i < total_keys; i++) {
        keySource = parcList_GetAtIndex(keys, i);
        valueSource = parcList_GetAtIndex(values, i);

        keyCopy = parcObject_Copy(keySource);
        valueCopy = parcObject_Copy(valueSource);

        parcTreeMap_Put(treeCopy, keyCopy, valueCopy);
        parcObject_Release(&keyCopy);
        parcObject_Release(&valueCopy);
    }

    parcList_Release(&keys);
    parcList_Release(&values);

    return treeCopy;
}

////// Iterator Support //////

typedef struct {
    PARCTreeMap *map;
    PARCList *list;
    size_t currentIndex;
    PARCKeyValue *currentElement;
} _PARCTreeMapIterator;

static _PARCTreeMapIterator *
_parcTreeMapIterator_Init(PARCTreeMap *map)
{
    _PARCTreeMapIterator *state = parcMemory_AllocateAndClear(sizeof(_PARCTreeMapIterator));

    if (state != NULL) {
        state->map = map;
        state->list = _parcTreeMap_Elements(map);
        state->currentIndex = 0;
        state->currentElement = parcList_GetAtIndex(state->list, 0);
        trapOutOfMemoryIf(state->list == NULL, "Cannot create parcList");
    }

    return state;
}

static bool
_parcTreeMapIterator_Fini(PARCTreeMap *map __attribute__((unused)), _PARCTreeMapIterator *state __attribute__((unused)))
{
    parcList_Release(&state->list);
    parcMemory_Deallocate(&state);
    return true;
}

static _PARCTreeMapIterator *
_parcTreeMapIterator_Next(PARCTreeMap *map __attribute__((unused)), _PARCTreeMapIterator *state)
{
    state->currentElement = parcList_GetAtIndex(state->list, state->currentIndex);
    ++state->currentIndex;
    return state;
}

static void
_parcTreeMapIterator_Remove(PARCTreeMap *map, _PARCTreeMapIterator **statePtr)
{
    _PARCTreeMapIterator *state = *statePtr;

    parcTreeMap_RemoveAndRelease(map, parcKeyValue_GetKey(state->currentElement));
}

static bool
_parcTreeMapIterator_HasNext(PARCTreeMap *map __attribute__((unused)), _PARCTreeMapIterator *state)
{
    return (parcList_Size(state->list) > state->currentIndex);
}

static PARCObject *
_parcTreeMapIterator_Element(PARCTreeMap *map __attribute__((unused)), const _PARCTreeMapIterator *state)
{
    return state->currentElement;
}

static PARCObject *
_parcTreeMapIterator_ElementValue(PARCTreeMap *map __attribute__((unused)), const _PARCTreeMapIterator *state)
{
    return parcKeyValue_GetValue(state->currentElement);
}

static PARCObject *
_parcTreeMapIterator_ElementKey(PARCTreeMap *map __attribute__((unused)), const _PARCTreeMapIterator *state)
{
    return parcKeyValue_GetKey(state->currentElement);
}

PARCIterator *
parcTreeMap_CreateValueIterator(PARCTreeMap *treeMap)
{
    PARCIterator *iterator = parcIterator_Create(treeMap,
                                                 (void *(*)(PARCObject *))_parcTreeMapIterator_Init,
                                                 (bool (*)(PARCObject *, void *))_parcTreeMapIterator_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcTreeMapIterator_Next,
                                                 (void (*)(PARCObject *, void **))_parcTreeMapIterator_Remove,
                                                 (void *(*)(PARCObject *, void *))_parcTreeMapIterator_ElementValue,
                                                 (void (*)(PARCObject *, void *))_parcTreeMapIterator_Fini,
                                                 NULL);

    return iterator;
}


PARCIterator *
parcTreeMap_CreateKeyIterator(PARCTreeMap *treeMap)
{
    PARCIterator *iterator = parcIterator_Create(treeMap,
                                                 (void *(*)(PARCObject *))_parcTreeMapIterator_Init,
                                                 (bool (*)(PARCObject *, void *))_parcTreeMapIterator_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcTreeMapIterator_Next,
                                                 (void (*)(PARCObject *, void **))_parcTreeMapIterator_Remove,
                                                 (void *(*)(PARCObject *, void *))_parcTreeMapIterator_ElementKey,
                                                 (void (*)(PARCObject *, void *))_parcTreeMapIterator_Fini,
                                                 NULL);

    return iterator;
}

PARCIterator *
parcTreeMap_CreateKeyValueIterator(PARCTreeMap *treeMap)
{
    PARCIterator *iterator = parcIterator_Create(treeMap,
                                                 (void *(*)(PARCObject *))_parcTreeMapIterator_Init,
                                                 (bool (*)(PARCObject *, void *))_parcTreeMapIterator_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcTreeMapIterator_Next,
                                                 (void (*)(PARCObject *, void **))_parcTreeMapIterator_Remove,
                                                 (void *(*)(PARCObject *, void *))_parcTreeMapIterator_Element,
                                                 (void (*)(PARCObject *, void *))_parcTreeMapIterator_Fini,
                                                 NULL);

    return iterator;
}
