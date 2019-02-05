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
 * @file parc_TreeRedBlack.h
 * @ingroup datastructures
 * @brief A red-black tree is a type of self-balancing binary search tree,
 * a data structure used in computer science, typically used to implement associative arrays.
 *
 */
#ifndef libparc_parc_TreeRedBlack_h
#define libparc_parc_TreeRedBlack_h

#include <stdlib.h>
#include <parc/algol/parc_ArrayList.h>

struct parc_tree_redblack;
typedef struct parc_tree_redblack PARCTreeRedBlack;

/**
 * Compare two keys (signum)
 *
 *   This is the definition of a function that takes two keys and compares them.
 *   It is a signum function. A user of the RedBlack tree will provide this function on creation
 *   so the tree can perform it's operations.
 *
 * @param [in] key1 The first key to compare
 * @param [in] key2 The second key to compare
 * @return A signum comparison. negative if key 1 is smaller, 0 if key1 == key2, greater than 0 if key1 is bigger.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef int (PARCTreeRedBlack_KeyCompare)(const void *key1, const void *key2);

/**
 * Compare two values for equality
 *
 * This is a function definiton for a function that compares two values in a `PARCTreeRedBlack`.
 * If the values are equal it will return true. If they are not it will return false.
 * A function of this type will be given on the creation of the `PARCTreeRedBlack`. It will be used by the tree
 * to test equality.
 *
 * @param [in] value1 The first value to compare
 * @param [in] value2 The second value to compare
 * @return TRUE if the values are considered equal, FALSE if they are not
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef bool (PARCTreeRedBlack_ValueEquals)(const void *value1, const void *value2);

/**
 * A function to free a value pointer
 *
 * This is a function definition for a function to free a value in the `PARCTreeRedBlack`. The function should
 * free/destroy the value when called.  A function of this type will be passed to the `PARCTreeRedBlack` on
 * creation. It will be used to destroy values when needed.
 *
 * @param [in,out] value A pointer to a pointer to a value to be freed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef void (PARCTreeRedBlack_ValueFree)(void **value);

/**
 * A function to free a key pointer
 *
 *   This is a function definition for a function to free a key in the `PARCTreeRedBlack`. The function should
 *   free/destroy the key when called.  A function of this type will be passed to the `PARCTreeRedBlack` on
 *   creation. It will be used to destroy keys when needed.
 *
 * @param [in,out] key A pointer to a pointer to a key to be freed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef void (PARCTreeRedBlack_KeyFree)(void **key);

/**
 * Create a copy of a key.
 *
 * This must be a real copy! (allocating new memory)
 *
 * This function should create a copy of the key. The copy must pass an equality test to the original key.
 * If this function retuns NULL, the tree copy will fail (and return NULL)
 *
 * @param [in] key A pointer to the key to be copied
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new key instance
 *
 */
typedef void * (PARCTreeRedBlack_KeyCopy)(const void *key);

/**
 * Create a copy of a value
 *
 * This must be a real copy! (new memory allocated)
 *
 * This function should create a copy of the value. The copy must pass an equality test to the original value.
 * If this function retuns NULL, the tree copy will fail (and return NULL)
 *
 * @param [in] value A pointer to the value to be copied
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new value instance
 *
 */
typedef void * (PARCTreeRedBlack_ValueCopy)(const void *value);

/**
 * Create a `PARCTreeRedBlack`
 *
 *   Create a RedBlack Tree. You must provide a key compare function.
 *   The valueEquals function will be used to compare 2 trees.  If you do not provide such a function
 *   the values will be compared directly.
 *   The keyFree and valueFree functions are optional but highly encouraged.  They will be used on
 *   destruction of the tree on any remaining elements. They are also used on deletion of elements.
 *
 * @param [in] keyCompare A function to compare keys  (required)
 * @param [in] keyFree A function to free Keys  (may be NULL)
 * @param [in] keyCopy A function to copy Keys (may be NULL)
 * @param [in] valueEquals A function to test value equality (may be NULL)
 * @param [in] valueFree A function to free Values (may be NULL)
 * @param [in] valueCopy A function to free Keys (may be NULL)
 * @return NULL Error allocating memory
 * @return Non-NULL An initialized RedBlack tree
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCTreeRedBlack *parcTreeRedBlack_Create(PARCTreeRedBlack_KeyCompare *keyCompare,
                                          PARCTreeRedBlack_KeyFree *keyFree,
                                          PARCTreeRedBlack_KeyCopy *keyCopy,
                                          PARCTreeRedBlack_ValueEquals *valueEquals,
                                          PARCTreeRedBlack_ValueFree *valueFree,
                                          PARCTreeRedBlack_ValueCopy *valueCopy);

/**
 * Destroy a `PARCTreeRedBlack`
 *
 *   Destroy a `PARCTreeRedBlack`, free all the memory.
 *   All remaining keys and values will be freed with the respective free functions.
 *
 * @param [in,out] treePointer A pointer to a pointer to the `PARCTreeRedBlack` to be destroyed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcTreeRedBlack_Destroy(PARCTreeRedBlack **treePointer);

/**
 * Insert a value into the `PARCTreeRedBlack`.
 *
 *   Insert a value into the `PARCTreeRedBlack`.
 *   If the key exists in the tree then the new value will replace the old value.
 *   The old key and value will be freed using the provided free functions.
 *   The tree will take ownership of the key and value. They can't be NULL.
 *
 * @param [in,out] tree A pointer to an initialized `PARCTreeRedBlack`.
 * @param [in] key A pointer to a key - This may not be NULL.
 * @param [in] value A pointer to a value - This may not be NULL.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcTreeRedBlack_Insert(PARCTreeRedBlack *tree, void *key, void *value);

/**
 * Get a value into the `PARCTreeRedBlack`.
 *
 *   Get a value from a `PARCTreeRedBlack`.
 *   If the key is not found in the tree NULL is returned.
 *   The returned value will still be owned by the tree.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeRedBlack`.
 * @param [in] key A pointer to a key  (The tree will not own this).
 * @return A pointer to a value. You do not own this value (it's still in the tree).
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcTreeRedBlack_Get(PARCTreeRedBlack *tree, const void *key);

/**
 * Get the first (smallest) key from a `PARCTreeRedBlack`.
 *
 *   Get the first (smallest) key from a `PARCTreeRedBlack`.
 *   The returned key will still be owned by the tree.
 *   If the tree is empty the function will return NULL.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeRedBlack`.
 * @return A pointer to the smallest key. You do not own this value (it's still in the tree).
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcTreeRedBlack_FirstKey(const PARCTreeRedBlack *tree);

/**
 * Get the last (largest) key from a `PARCTreeRedBlack`.
 *
 *   Get the last (largest) key from a `PARCTreeRedBlack`.
 *   The returned key will still be owned by the tree.
 *   If the tree is empty the function will return NULL
 *
 * @param [in] tree A pointer to an initialized `PARCTreeRedBlack`.
 * @return A pointer to the smallest key. You do not own this value (it's still in the tree).
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcTreeRedBlack_LastKey(const PARCTreeRedBlack *tree);

/**
 * Remove a value (and key) from a `PARCTreeRedBlack`.
 *
 *   Remove a value from a `PARCTreeRedBlack`.  The value (and key) will no longer be in the tree.
 *   The (internal) key will be freed.  The provided key will not be modified.
 *   The value associated with the key will be returned. You will own this value.
 *   If the key is not found in the tree NULL is returned.
 *
 * @param [in,out] tree A pointer to an initialized `PARCTreeRedBlack`.
 * @param [in] key A pointer to a key (The tree will not own this).
 * @return A pointer to a value. You will now own this value.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcTreeRedBlack_Remove(PARCTreeRedBlack *tree, const void *key);

/**
 * Remove and destroy a value (and key) from a `PARCTreeRedBlack`.
 *
 *   Delete a value from a `PARCTreeRedBlack`.  The value (and key) will no longer be in the tree.
 *   The value and key will be freed by the tree. The provided key will not be modified.
 *
 * @param [in,out] tree A pointer to an initialized `PARCTreeRedBlack`.
 * @param [in] key A pointer to a key (The tree will not own this).
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcTreeRedBlack_RemoveAndDestroy(PARCTreeRedBlack *tree, const void *key);

/**
 * Get the size of a `PARCTreeRedBlack`.
 *
 *   Return the size of the tree (number of elements).
 *
 * @param [in] tree A pointer to an initialized `PARCTreeRedBlack`.
 * @return size of the tree (number of elements).
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcTreeRedBlack_Size(const PARCTreeRedBlack *tree);

/**
 * Get an ArrayList of the keys in this tree
 *
 *   Get a list of the keys from this `PARCTreeRedBlack`.  All of these keys will be valid keys in the `PARCTreeRedBlack`.
 *   The caller will own the list of keys and should destroy it when done.  The caller will not own
 *   the keys themselves. Destroying the {@link PARCArrayList} should be enough.
 *   Note that if the tree is modified or destroyed the key pointers might no longer point to valid keys.
 *   The list of keys will be sorted (as defined by the key compare function, smallest first)
 *
 * @param [in] tree A pointer to a `PARCTreeRedBlack`.
 * @return A list of (pointers to) keys.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCArrayList *parcTreeRedBlack_Keys(const PARCTreeRedBlack *tree);

/**
 * Get an ArrayList of the values in this `PARCTreeRedBlack`.
 *
 *   Get a list of the values from this tree.  All of these values will be valid in the `PARCTreeRedBlack`.
 *   The caller will own the list of values and should destroy it when done.  The caller will not own
 *   the values themselves. Destroying the {@link PARCArrayList} should be enough.
 *   Note that if the tree is modified or destroyed the value pointers might no longer point to valid values.
 *   The list of values will be sorted by key (as defined by the key compare function, smallest first).
 *
 * @param [in] tree A pointer to a `PARCTreeRedBlack`.
 * @return A list of (pointers to) values.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCArrayList *parcTreeRedBlack_Values(const PARCTreeRedBlack *tree);

/**
 * Calculate if two `PARCTreeRedBlack`s are equal
 *
 *   Compare 2 `PARCTreeRedBlack` for equality.
 *   Two trees are equal if they have the same keys associated with the same values. The values
 *   will be compared using the valueEquals function provided on create. If this function was not
 *   provided it will compare the actual values.
 *
 * @param [in] tree1 A pointer to a `PARCTreeRedBlack`.
 * @param [in] tree2 A pointer to another `PARCTreeRedBlack`.
 * @return true if the trees are equal, zero otherwise
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int parcTreeRedBlack_Equals(const PARCTreeRedBlack *tree1, const PARCTreeRedBlack *tree2);

/**
 * Copy a RedBlack Tree
 *
 * Crete a copy of a RedBlack Tree.
 * This will create a completely new tree. It will copy every key and every value using the Copy functions
 * provided at tree creation.  If these functions are NULL then the numeric values will be copied directly.
 *
 * @param [in] source_tree A pointer to a `PARCTreeRedBlack` to be copied
 * @return NULL Error copying the tree.
 * @return Non-NULL A copy of the `PARCTreeRedBlack`.
 *
 * Example:
 * @code
 * {
 *      PARCTreeRedBlack * source_tree = parcTreeRedBlack_Create(.....);
 *
 *      ... operations on tree ...
 *
 *      PARCTreeRedBlack * tree_copy = parcTreeRedBlack_Copy(source_tree);
 *
 *      ... operations on tree ...
 *
 *      parcTreeRedBlack_Destroy(&source_tree);
 *      parcTreeRedBlack_Destroy(&tree_copy);
 * }
 * @endcode
 */
PARCTreeRedBlack *parcTreeRedBlack_Copy(const PARCTreeRedBlack *source_tree);
#endif // libparc_parc_TreeRedBlack_h
