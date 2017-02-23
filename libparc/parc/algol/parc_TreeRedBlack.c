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

#include <parc/algol/parc_TreeRedBlack.h>

#include <parc/algol/parc_Memory.h>

#define RED   1
#define BLACK 0

#define ASSERT_INVARIANTS

struct redblack_node;
typedef struct redblack_node Node;

struct redblack_node {
    Node *left_child;
    Node *right_child;
    Node *parent;
    void *key;
    void *value;
    int color;
};

struct parc_tree_redblack {
    Node *root;
    Node *nil;
    int size;
    PARCTreeRedBlack_KeyCompare *keyCompare;
    PARCTreeRedBlack_KeyFree *keyFree;
    PARCTreeRedBlack_KeyCopy *keyCopy;
    PARCTreeRedBlack_ValueFree *valueFree;
    PARCTreeRedBlack_ValueEquals *valueEquals;
    PARCTreeRedBlack_ValueCopy *valueCopy;
};

typedef void (rbRecursiveFunc)(Node *node, void *data);

static void
_rbNodeFree(PARCTreeRedBlack *tree, Node *node)
{
    if (tree->keyFree != NULL) {
        tree->keyFree(&(node->key));
    }
    if (tree->valueFree != NULL) {
        tree->valueFree(&(node->value));
    }
    parcMemory_Deallocate((void **) &node);
}

static void
_rbNodeFreeRecursive(PARCTreeRedBlack *tree, Node *node)
{
    if (node->left_child != tree->nil) {
        _rbNodeFreeRecursive(tree, node->left_child);
    }
    if (node->right_child != tree->nil) {
        _rbNodeFreeRecursive(tree, node->right_child);
    }
    // We descended on both branches, now free myself.
    _rbNodeFree(tree, node);
    tree->size--;
}

// Run a function on all nodes in the tree, in order
static void
_rbNodeRecursiveRun(PARCTreeRedBlack *tree, Node *node, rbRecursiveFunc *func, void *data)
{
    if (node->left_child != tree->nil) {
        _rbNodeRecursiveRun(tree, node->left_child, func, data);
    }
    func(node, data);
    if (node->right_child != tree->nil) {
        _rbNodeRecursiveRun(tree, node->right_child, func, data);
    }
}

/**
 * Create a node
 * Set the parent and children to tree->nil.
 * If we are creating the nil node this might leave garbage there (if not preset to NULL).
 */
static Node *
_rbNodeCreate(PARCTreeRedBlack *tree, int color)
{
    Node *node = parcMemory_AllocateAndClear(sizeof(Node));
    assertNotNull(node, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(Node));
    node->color = color;
    node->left_child = tree->nil;
    node->right_child = tree->nil;
    node->parent = tree->nil;
    return node;
}

static void
_rbNodeSetColor(Node *node, uint8_t color)
{
    node->color = color;
}

static int
_rbNodeColor(const Node *node)
{
    return node->color;
}

static int
_rbNodeIsEqual(const PARCTreeRedBlack *tree, const Node *node, const void *key)
{
    return (tree->keyCompare(node->key, key) == 0);
}

static int
_rbNodeIsGreaterThan(const PARCTreeRedBlack *tree, const Node *node, const void *key)
{
    return (tree->keyCompare(node->key, key) > 0);
}

static void
_rbNodeUpdate(PARCTreeRedBlack *tree, Node *treeNode, Node *newNode)
{
    // Free old values
    if (tree->keyFree != NULL) {
        tree->keyFree(&(treeNode->key));
    }
    if (tree->valueFree != NULL) {
        tree->valueFree(&(treeNode->value));
    }
    treeNode->key = newNode->key;
    treeNode->value = newNode->value;
    parcMemory_Deallocate((void **) &newNode);
}

static void
_rbNodeRotateLeft(PARCTreeRedBlack *tree, Node *node)
{
    Node *subroot = node->right_child;
    node->right_child = subroot->left_child;
    if (node->right_child != tree->nil) {
        node->right_child->parent = node;
    }

    subroot->parent = node->parent;
    if (tree->root == node) {
        tree->root = subroot;
    } else {
        if (subroot->parent->left_child == node) {
            // node was a left child
            subroot->parent->left_child = subroot;
        } else {
            // node was a right child
            subroot->parent->right_child = subroot;
        }
    }

    subroot->left_child = node;
    node->parent = subroot;
}

static void
_rbNodeRotateRight(PARCTreeRedBlack *tree, Node *node)
{
    Node *subroot = node->left_child;
    node->left_child = subroot->right_child;
    if (node->left_child != tree->nil) {
        node->left_child->parent = node;
    }

    subroot->parent = node->parent;
    if (tree->root == node) {
        tree->root = subroot;
    } else {
        if (subroot->parent->left_child == node) {
            // node was a left child
            subroot->parent->left_child = subroot;
        } else {
            // node was a right child
            subroot->parent->right_child = subroot;
        }
    }

    subroot->right_child = node;
    node->parent = subroot;
}

static void
_rbNodeFix(PARCTreeRedBlack *tree, Node *startNode)
{
    Node *node = startNode;
    Node *uncle;
    while (_rbNodeColor(node->parent) == RED) {
        if (node->parent->parent->left_child == node->parent) {
            uncle = node->parent->parent->right_child;
            if (_rbNodeColor(uncle) == RED) {
                // My dad and uncle are red. Switch dad to black.
                // Switch grandpa to red and start there.
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(uncle, BLACK);
                _rbNodeSetColor(node->parent->parent, RED);
                node = node->parent->parent;
            } else {
                if (node->parent->right_child == node) {
                    node = node->parent;
                    _rbNodeRotateLeft(tree, node);
                }
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(node->parent->parent, RED);
                _rbNodeRotateRight(tree, node->parent->parent);
            }
        } else {
            uncle = node->parent->parent->left_child;
            if (_rbNodeColor(uncle) == RED) {
                // My dad and uncle are red. Switch dad to black.
                // Switch grandpa to red and start there.
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(uncle, BLACK);
                _rbNodeSetColor(node->parent->parent, RED);
                node = node->parent->parent;
            } else {
                if (node->parent->left_child == node) {
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
_rbNodeAssertNodeInvariants(Node *node, void *data)
{
    PARCTreeRedBlack *tree = (PARCTreeRedBlack *) data;
    assertNotNull(node->parent, "Node has NULL parent");
    assertNotNull(node->left_child, "Left child NULL");
    assertNotNull(node->right_child, "Richt child NULL");
    if (node != tree->root) {
        assertTrue(node->parent != tree->nil, "Paren't can't be nill for node!");
        // Don't need to compare to parent, they compared to us
    }
    assertNotNull(node->key, "We have a null key!!");
    assertNotNull(node->value, "We have a null value!!");
    if (node->left_child != tree->nil) {
        assertTrue(tree->keyCompare(node->key, node->left_child->key) > 0, "Left child not smaller?");
    }
    if (node->right_child != tree->nil) {
        assertTrue(tree->keyCompare(node->key, node->right_child->key) < 0, "Right child not bigger?");
    }
}

static
void
_rbNodeAssertTreeInvariants(const PARCTreeRedBlack *tree)
{
    assertNotNull(tree, "Tree is null!");
    assertTrue(tree->size >= 0, "Tree has negative size");
    assertNotNull(tree->keyCompare, "No key compare function");
    if (tree->size != 0) {
        assertTrue(tree->root != tree->nil, "Tree size = %d > 0 but root is nil", tree->size);
        assertNotNull(tree->root, "Tree size > 0 but root is NULL");
#ifdef ASSERT_INVARIANTS
        _rbNodeRecursiveRun((PARCTreeRedBlack *) tree, tree->root, _rbNodeAssertNodeInvariants, (void *) tree);
#endif
    }
}

static void
_rbNodeFixDelete(PARCTreeRedBlack *tree, Node *node)
{
    Node *fixNode;

    while ((node != tree->root) && (_rbNodeColor(node) == BLACK)) {
        _rbNodeAssertTreeInvariants(tree);
        if (node == node->parent->left_child) {
            fixNode = node->parent->right_child;
            if (_rbNodeColor(fixNode) == RED) {
                _rbNodeSetColor(fixNode, BLACK);
                _rbNodeSetColor(node->parent, RED);
                _rbNodeRotateLeft(tree, node->parent);
                fixNode = node->parent->right_child;
            }
            if ((_rbNodeColor(fixNode->left_child) == BLACK) &&
                (_rbNodeColor(fixNode->right_child) == BLACK)) {
                _rbNodeSetColor(fixNode, RED);
                node = node->parent;
            } else {
                if (_rbNodeColor(fixNode->right_child) == BLACK) {
                    _rbNodeSetColor(fixNode->left_child, BLACK);
                    _rbNodeSetColor(fixNode, RED);
                    _rbNodeRotateRight(tree, fixNode);
                    fixNode = node->parent->right_child;
                }
                _rbNodeSetColor(fixNode, _rbNodeColor(node->parent));
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(fixNode->right_child, BLACK);
                _rbNodeRotateLeft(tree, node->parent);
                node = tree->root;
            }
        } else {
            fixNode = node->parent->left_child;
            if (_rbNodeColor(fixNode) == RED) {
                _rbNodeSetColor(fixNode, BLACK);
                _rbNodeSetColor(node->parent, RED);
                _rbNodeRotateRight(tree, node->parent);
                fixNode = node->parent->left_child;
            }
            if ((_rbNodeColor(fixNode->left_child) == BLACK) &&
                (_rbNodeColor(fixNode->right_child) == BLACK)) {
                _rbNodeSetColor(fixNode, RED);
                node = node->parent;
            } else {
                if (_rbNodeColor(fixNode->left_child) == BLACK) {
                    _rbNodeSetColor(fixNode->right_child, BLACK);
                    _rbNodeSetColor(fixNode, RED);
                    _rbNodeRotateLeft(tree, fixNode);
                    fixNode = node->parent->left_child;
                }
                _rbNodeSetColor(fixNode, _rbNodeColor(node->parent));
                _rbNodeSetColor(node->parent, BLACK);
                _rbNodeSetColor(fixNode->left_child, BLACK);
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
_rbNodeRemove(PARCTreeRedBlack *tree, Node *node)
{
    _rbNodeAssertTreeInvariants(tree);
    Node *fixupNode;
    int deleteNodeColor = _rbNodeColor(node);
    if (node->left_child == tree->nil) {
        if (node->right_child == tree->nil) {
            // ---- We have no children ----
            if (tree->root == node) {
                tree->root = tree->nil;
            } else {
                if (node->parent->left_child == node) {
                    node->parent->left_child = tree->nil;
                } else {
                    node->parent->right_child = tree->nil;
                }
            }
            fixupNode = tree->nil;
            fixupNode->parent = node->parent;
        } else {
            // ---- We only have right child, move up ----
            if (tree->root == node) {
                tree->root = node->right_child;
            } else {
                if (node->parent->left_child == node) {
                    node->parent->left_child = node->right_child;
                } else {
                    node->parent->right_child = node->right_child;
                }
            }
            fixupNode = node->right_child;
            node->right_child->parent = node->parent;
        }
    } else {
        if (node->right_child == tree->nil) {
            // ---- We only have left child, move up ----
            if (tree->root == node) {
                tree->root = node->left_child;
            } else {
                if (node->parent->left_child == node) {
                    node->parent->left_child = node->left_child;
                } else {
                    node->parent->right_child = node->left_child;
                }
            }
            node->left_child->parent = node->parent;
            fixupNode = node->left_child;
        } else {
            // ---- We have 2 children, move our successor to our location ----
            Node *successor = node->right_child;
            while (successor->left_child != tree->nil) {
                successor = successor->left_child;
            }
            deleteNodeColor = _rbNodeColor(successor);

            // Remove successor, it has no left child
            if (successor == successor->parent->left_child) {
                successor->parent->left_child = successor->right_child;
            } else {
                successor->parent->right_child = successor->right_child;
            }
            successor->right_child->parent = successor->parent;

            fixupNode = successor->right_child;

            if (node->parent == tree->nil) {
                tree->root = successor;
            } else if (node->parent->left_child == node) {
                node->parent->left_child = successor;
            } else {
                node->parent->right_child = successor;
            }
            successor->parent = node->parent;
            successor->left_child = node->left_child;
            node->left_child->parent = successor;
            successor->right_child = node->right_child;
            node->right_child->parent = successor;

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


PARCTreeRedBlack *
parcTreeRedBlack_Create(PARCTreeRedBlack_KeyCompare *keyCompare,
                        PARCTreeRedBlack_KeyFree *keyFree,
                        PARCTreeRedBlack_KeyCopy *keyCopy,
                        PARCTreeRedBlack_ValueEquals *valueEquals,
                        PARCTreeRedBlack_ValueFree *valueFree,
                        PARCTreeRedBlack_ValueCopy *valueCopy)
{
    assertNotNull(keyCompare, "We need a key compare function");
    PARCTreeRedBlack *tree = parcMemory_AllocateAndClear(sizeof(PARCTreeRedBlack));
    assertNotNull(tree, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCTreeRedBlack));
    tree->nil = _rbNodeCreate(tree, BLACK);
    tree->nil->left_child = tree->nil;
    tree->nil->right_child = tree->nil;
    tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->keyCompare = keyCompare;
    tree->keyFree = keyFree;
    tree->keyCopy = keyCopy;
    tree->valueEquals = valueEquals;
    tree->valueFree = valueFree;
    tree->valueCopy = valueCopy;
    tree->size = 0;
    return tree;
}

void
parcTreeRedBlack_Destroy(PARCTreeRedBlack **treePointer)
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

    parcMemory_Deallocate((void **) treePointer);
    *treePointer = NULL;
}

void
parcTreeRedBlack_Insert(PARCTreeRedBlack *tree, void *key, void *value)
{
    assertNotNull(tree, "Tree can't be NULL");
    assertNotNull(key, "Key can't be NULL");
    assertNotNull(value, "Value can't be NULL");

    Node *newNode = _rbNodeCreate(tree, RED);
    Node *parent = tree->nil;
    Node *node;

    // Set the value for the created node
    newNode->key = key;
    newNode->value = value;

    // Start at the top
    node = tree->root;

    // Let's get to the bottom of the tree to insert.
    while (node != tree->nil) {
        parent = node;
        if (_rbNodeIsEqual(tree, node, key)) {
            // We're trying to insert the same value
            _rbNodeUpdate(tree, node, newNode);
            return;
        } else {
            if (_rbNodeIsGreaterThan(tree, node, key)) {
                // The key is smaller
                node = node->left_child;
            } else {
                node = node->right_child;
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
            parent->left_child = newNode;
        } else {
            parent->right_child = newNode;
        }
    }

    // We have inserted one node.
    tree->size++;

    // We have a correct tree. But we need to regain the red-black property.
    _rbNodeFix(tree, newNode);

    _rbNodeAssertTreeInvariants(tree);
}

// Return value
void *
parcTreeRedBlack_Get(PARCTreeRedBlack *tree, const void *key)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    Node *node = tree->root;

    // Let's get to the bottom of the tree to insert.
    while (node != tree->nil) {
        if (_rbNodeIsEqual(tree, node, key)) {
            // We found the node, return
            return node->value;
        } else {
            if (_rbNodeIsGreaterThan(tree, node, key)) {
                node = node->left_child;
            } else {
                node = node->right_child;
            }
        }
    }
    return NULL;
}

// Return value, remove from tree
void *
parcTreeRedBlack_Remove(PARCTreeRedBlack *tree, const void *key)
{
    assertNotNull(tree, "Tree can't be NULL");
    assertNotNull(key, "Key can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    Node *node = tree->root;

    // Let's get to the bottom of the tree to insert.
    while (node != tree->nil) {
        if (_rbNodeIsEqual(tree, node, key)) {
            _rbNodeRemove(tree, node);
            void *value = node->value;
            if (tree->keyFree != NULL) {
                tree->keyFree(&node->key);
            }
            parcMemory_Deallocate((void **) &node);
            _rbNodeAssertTreeInvariants(tree);
            return value;
        } else {
            if (_rbNodeIsGreaterThan(tree, node, key)) {
                node = node->left_child;
            } else {
                node = node->right_child;
            }
        }
    }

    // We didn't find the node

    _rbNodeAssertTreeInvariants(tree);
    return NULL;
}

// remove from tree and destroy
void
parcTreeRedBlack_RemoveAndDestroy(PARCTreeRedBlack *tree, const void *key)
{
    assertNotNull(tree, "Tree can't be NULL");
    assertNotNull(key, "Key can't be NULL");

    Node *node = tree->root;
    // Let's get to the bottom of the tree to insert.
    while (node != tree->nil) {
        if (_rbNodeIsEqual(tree, node, key)) {
            _rbNodeRemove(tree, node);
            _rbNodeFree(tree, node);
            _rbNodeAssertTreeInvariants(tree);
            return;
        } else {
            if (_rbNodeIsGreaterThan(tree, node, key)) {
                node = node->left_child;
            } else {
                node = node->right_child;
            }
        }
    }
    _rbNodeAssertTreeInvariants(tree);
}

void *
parcTreeRedBlack_LastKey(const PARCTreeRedBlack *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);
    Node *node = tree->root;

    if (tree->size == 0) {
        // We don't have any entries
        return NULL;
    }

    // Let's get to the bottom right of the tree to find the largest
    while (node->right_child != tree->nil) {
        node = node->right_child;
    }

    return node->key;
}

void *
parcTreeRedBlack_FirstKey(const PARCTreeRedBlack *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);
    Node *node = tree->root;


    if (tree->size == 0) {
        // We don't have any entries
        return NULL;
    }

    // Let's get to the bottom left of the tree to find the smallest
    while (node->left_child != tree->nil) {
        node = node->left_child;
    }

    return node->key;
}

size_t
parcTreeRedBlack_Size(const PARCTreeRedBlack *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    return tree->size;
}

static void
_rbGetKeys(Node *node, void *data)
{
    PARCArrayList *list = (PARCArrayList *) data;
    parcArrayList_Add(list, node->key);
}

PARCArrayList *
parcTreeRedBlack_Keys(const PARCTreeRedBlack *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    PARCArrayList *keys = parcArrayList_Create(NULL);

    if (tree->size > 0) {
        _rbNodeRecursiveRun((PARCTreeRedBlack *) tree, tree->root, _rbGetKeys, keys);
    }
    return keys;
}

static void
_rbGetValues(Node *node, void *data)
{
    PARCArrayList *list = (PARCArrayList *) data;
    parcArrayList_Add(list, node->value);
}

PARCArrayList *
parcTreeRedBlack_Values(const PARCTreeRedBlack *tree)
{
    assertNotNull(tree, "Tree can't be NULL");
    _rbNodeAssertTreeInvariants(tree);

    PARCArrayList *values = parcArrayList_Create(NULL);

    if (tree->size > 0) {
        _rbNodeRecursiveRun((PARCTreeRedBlack *) tree, tree->root, _rbGetValues, values);
    }
    return values;
}

int
parcTreeRedBlack_Equals(const PARCTreeRedBlack *tree1, const PARCTreeRedBlack *tree2)
{
    _rbNodeAssertTreeInvariants(tree1);
    _rbNodeAssertTreeInvariants(tree2);
    assertNotNull(tree1, "Tree can't be NULL");
    assertNotNull(tree2, "Tree can't be NULL");

    int ret = 1;

    PARCArrayList *keys1 = parcTreeRedBlack_Keys(tree1);
    PARCArrayList *keys2 = parcTreeRedBlack_Keys(tree2);
    size_t length1 = parcArrayList_Size(keys1);
    size_t length2 = parcArrayList_Size(keys2);

    if (length1 == length2) {
        for (size_t i = 0; i < length1; i++) {
            if (tree1->keyCompare(parcArrayList_Get(keys1, i), parcArrayList_Get(keys2, i)) != 0) {
                ret = 0;
                break;
            }
        }
        if (ret != 0) {
            PARCArrayList *values1 = parcTreeRedBlack_Values(tree1);
            PARCArrayList *values2 = parcTreeRedBlack_Values(tree2);
            size_t length1 = parcArrayList_Size(values1);
            size_t length2 = parcArrayList_Size(values2);
            if (length1 == length2) {
                for (size_t i = 0; i < length1; i++) {
                    void *value1 = parcArrayList_Get(values1, i);
                    void *value2 = parcArrayList_Get(values2, i);
                    if (tree1->valueEquals != NULL) {
                        if (tree1->valueEquals(value1, value2) == 0) {
                            ret = 0;
                            break;
                        }
                    } else {
                        if (value1 != value2) {
                            ret = 0;
                            break;
                        }
                    }
                }
            }
            parcArrayList_Destroy(&values1);
            parcArrayList_Destroy(&values2);
        }
    } else {
        ret = 0;
    }

    parcArrayList_Destroy(&keys1);
    parcArrayList_Destroy(&keys2);
    return ret;
}


/*
 * This is a simple implementation of Copy that goes through the list of keys and values.
 */
PARCTreeRedBlack *
parcTreeRedBlack_Copy(const PARCTreeRedBlack *source_tree)
{
    _rbNodeAssertTreeInvariants(source_tree);
    assertNotNull(source_tree, "Tree can't be NULL");

    void *key_source;
    void *key_copy;
    void *value_source;
    void *value_copy;

    PARCTreeRedBlack *tree_copy = parcTreeRedBlack_Create(source_tree->keyCompare,
                                                          source_tree->keyFree,
                                                          source_tree->keyCopy,
                                                          source_tree->valueEquals,
                                                          source_tree->valueFree,
                                                          source_tree->valueCopy);

    PARCArrayList *keys = parcTreeRedBlack_Keys(source_tree);
    PARCArrayList *values = parcTreeRedBlack_Values(source_tree);

    size_t total_keys = parcArrayList_Size(keys);

    for (size_t i = 0; i < total_keys; i++) {
        key_source = parcArrayList_Get(keys, i);
        value_source = parcArrayList_Get(values, i);

        if (source_tree->keyCopy != NULL) {
            key_copy = source_tree->keyCopy(key_source);
        } else {
            key_copy = key_source;
        }

        if (source_tree->valueCopy != NULL) {
            value_copy = source_tree->valueCopy(value_source);
        } else {
            value_copy = value_source;
        }

        parcTreeRedBlack_Insert(tree_copy, key_copy, value_copy);
    }

    parcArrayList_Destroy(&keys);
    parcArrayList_Destroy(&values);

    return tree_copy;
}
