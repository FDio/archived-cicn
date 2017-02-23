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
 * @file parc_TreeMap.h
 * @ingroup datastructures
 * @brief A Red-Black tree containing PARCObject keys and values.
 *
 * The map is sorted according to the natural ordering of its keys,
 * or by a comparator function provided at creation time, depending on which constructor is used.
 *
 */
#ifndef libparc_parc_TreeMap_h
#define libparc_parc_TreeMap_h

#include <stdlib.h>

#include "parc_Object.h"
#include "parc_KeyValue.h"
#include "parc_List.h"
#include "parc_Iterator.h"

struct parc_treemap;
typedef struct parc_treemap PARCTreeMap;

/**
 * Definition of a custom funcion to compare two keys. A function of
 * this signature can be provided to CreateCustom(...) constructor to
 * override the default parcObject_Compare(...) for comparing key
 * objects. This will be used during all internal comparisons.
 *
 * @param [in] key1 The first key to compare
 * @param [in] key2 The second key to compare
 *
 * @return A signum comparison. negative if key1 is smaller than key2,
 * 0 if equal, positive if key1 is bigger.
 *
 * Example:
 * @code
 * {
 *      int _compareKeys(const PARCObject *key1, const PARCObject *key2) {...}
 *
 *      PARCTreeMap * tree1 = parcTreeMap_CreateCustom(_compareKeys);
 *
 *      ...
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
typedef int (PARCTreeMap_CustomCompare)(const PARCObject *key1, const PARCObject *key2);

/**
 * Create a standard `PARCTreeMap` that uses parcObject_Compare for
 * comparisons.
 *
 * @return NULL Error allocating memory
 * @return Non-NULL An initialized TreeMap
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCTreeMap *parcTreeMap_Create(void);

/**
 * Create a `PARCTreeMap` that uses the provided custom compare
 * function for key comparisons.
 *
 * @param [in] customCompare A cusom function to compare keys  (required)
 * @return NULL Error allocating memory
 * @return Non-NULL An initialized RedBlack tree
 *
 * Example:
 * @code
 * {
 *      int _compareKeys(const PARCObject *key1, const PARCObject *key2) {...}
 *
 *      PARCTreeMap * tree1 = parcTreeMap_CreateCustom(_compareKeys);
 *
 *      ...
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCTreeMap *parcTreeMap_CreateCustom(PARCTreeMap_CustomCompare *customCompare);

/**
 * Acquire a reference to a `PARCTreeMap`.
 *
 * @param tree The tree reference to acquire.
 *
 * @return the acquired reference.
 *
 */
PARCTreeMap *parcTreeMap_Acquire(const PARCTreeMap *tree);

/**
 * Release a reference to a `PARCTreeMap` object. If it is the last
 * reference, the object itself is freed and the specified free
 * function is invoked on the stored keys and values.
 *
 * @param [in,out] treePointer A pointer to a pointer to the `PARCTreeMap` to be released.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
void parcTreeMap_Release(PARCTreeMap **treePointer);

/**
 * Insert a value into the `PARCTreeMap`. If the key exists in the
 * tree then the new value will replace the old value. The old key and
 * value will be released by the map and the map will acquire a
 * reference to the new key and value. The key must not be NULL.
 *
 * @param [in, out] tree A pointer to an initialized `PARCTreeMap`.
 * @param [in] key A pointer to a key - This may not be NULL.
 * @param [in] value A pointer to a value.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      PARCObject *someKey = ...;
 *      PARCObject *someValue = ...;
 *
 *      parcTreeMap_Put(tree1, someKey, someValue);
 *
 *      parcObject_Release(&someKey);
 *      parcObject_Release(&someValue);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
void parcTreeMap_Put(PARCTreeMap *tree, const PARCObject *key, const PARCObject *value);

/**
 * Checks to see if a PARCTreeMap already contains a key.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @param [in] key A pointer to a key - This may not be NULL.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      if (parcTreeMap_ContainsKey(tree1, someKey) {
 *         ...
 *      }
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
bool parcTreeMap_ContainsKey(PARCTreeMap *tree, PARCObject *key);

/**
 * Get a value from a `PARCTreeMap`. If the key is not found in the
 * tree NULL is returned.  The returned value will still be owned by
 * the tree.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @param [in] key A pointer to a key  (The tree will not own this).
 * @return A pointer to a value. You do not own this value (it's still in the tree).
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      PARCObject *someValue = parcTreeMap_Get(tree1, someKey);
 *      if (someValue != NULL) {
 *         ...
 *      }
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCObject *parcTreeMap_Get(PARCTreeMap *tree, const PARCObject *key);

/**
 * Get the first (smallest) key from a `PARCTreeMap`. The returned key
 * will still be owned by the tree. If the tree is empty the function
 * will return NULL.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @return A pointer to the smallest key.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = parcTreeMap_GetFirstKey(tree1);
 *      if (someKey != NULL) {
 *         ...
 *      }
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCObject *parcTreeMap_GetFirstKey(const PARCTreeMap *tree);

/**
 * Get the first entry (one with the smallest key) from a
 * `PARCTreeMap`. The returned PARCKeyValue will still be owned by the
 * tree. If the tree is empty the function will return NULL.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @return A pointer to the entry (a PARCKeyValue) with the smallest key.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCKeyValue *someEntry = parcTreeMap_GetFirstEntry(tree1);
 *      if (someEntry != NULL) {
 *         ...
 *      }
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCKeyValue *parcTreeMap_GetFirstEntry(const PARCTreeMap *tree);

/**
 * Get the last (largest) key from a `PARCTreeMap`. The returned key
 * will still be owned by the tree.  If the tree is empty the function
 * will return NULL
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @return A pointer to the largest key.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = parcTreeMap_GetLastKey(tree1);
 *      if (someKey != NULL) {
 *         ...
 *      }
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCObject *parcTreeMap_GetLastKey(const PARCTreeMap *tree);

/**
 * Get the last entry (entry with the largest key) from a
 * `PARCTreeMap`. The returned entry will still be owned by the tree.
 * If the tree is empty the function will return NULL
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @return A pointer to the entry (a PARCKeyValue) with the largest key.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCKeyValue *lastEntry = parcTreeMap_GetLastEntry(tree1);
 *      if (lastEntry != NULL) {
 *         ...
 *      }
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCKeyValue *parcTreeMap_GetLastEntry(const PARCTreeMap *tree);

/**
 * Get the next largest key from a `PARCTreeMap`. The returned key
 * will still be owned by the tree.  If the tree is empty or the
 * supplied key is the largest, the function will return NULL
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @return A pointer to the next key. You do not own this value (it's still in the tree).
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      PARCObject *nextKey = parcTreeMap_GetHigherKey(tree1, someKey);
 *      if (nextKey != NULL) {
 *         ...
 *      }
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCObject *parcTreeMap_GetHigherKey(const PARCTreeMap *tree, const PARCObject *key);

/**
 * Get the entry with the next largest key from a `PARCTreeMap`. The
 * returned entry will still be owned by the tree.  If the tree is
 * empty or the supplied key is the largest, the function will return
 * NULL.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @return A pointer to the next entry (a PARCKeyValue). The caller
 * does not own this return.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      PARCKeyValue *nextEntry = parcTreeMap_GetHigherEntry(tree1, someKey);
 *      if (nextEntry != NULL) {
 *         ...
 *      }
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCKeyValue *parcTreeMap_GetHigherEntry(const PARCTreeMap *tree, const PARCObject *key);

/**
 * Get the previous key from a `PARCTreeMap`. The returned key will
 * still be owned by the tree.  If the tree is empty or the supplied
 * key is the smallest in the tree, the function will return NULL.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @param [in] key A pointer to an key
 * @return A pointer to the previous key. The caller
 * does not own this return.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      PARCObject *previousKey = parcTreeMap_GetLowerKey(tree1, someKey);
 *      if (previousKey != NULL) {
 *         ...
 *      }
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCObject *parcTreeMap_GetLowerKey(const PARCTreeMap *tree, const PARCObject *key);

/**
 * Get the entry with the next smallest key from a `PARCTreeMap`. The returned entry (a PARCKeyValue) will
 * still be owned by the tree.  If the tree is empty or the supplied
 * key is the smallest in the tree, the function will return NULL.
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @param [in] key A pointer to an key
 * @return A pointer to the previous entry (a PARCKeyValue). The caller
 * does not own this return.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      PARCKeyValue *previousEntry = parcTreeMap_GetLowerKey(tree1, someKey);
 *      if (previousEntry != NULL) {
 *         ...
 *      }
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCKeyValue *parcTreeMap_GetLowerEntry(const PARCTreeMap *tree, const PARCObject *key);

/**
 * Remove an entry from a `PARCTreeMap`. The entry will be removed
 * from the tree and the tree's reference to the key will be
 * released. The value will be returned and responsibility for
 * releasing the reference to the value will transfer to the caller.
 * The provided key will not be modified. If the key is not found in
 * the tree, NULL is returned.
 *
 * @param [in,out] tree A pointer to an initialized `PARCTreeMap`.
 * @param [in] key A pointer to a key (The tree will not own this).
 * @return A pointer to a value or NULL. You will now own this value.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      PARCObject *someValue = parcTreeMap_Remove(tree1, someKey);
 *      if (someValue != NULL) {
 *         ...
 *         parcObject_Release(&someValue);
 *      }
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCObject *parcTreeMap_Remove(PARCTreeMap *tree, const PARCObject *key);

/**
 * Remove and destroy an entry from a `PARCTreeMap`. The entry along with
 * it's key & value will be removed and released.
 *
 * @param [in,out] tree A pointer to an initialized `PARCTreeMap`.
 * @param [in] key A pointer to a key (The tree will not own this).
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCObject *someKey = ...;
 *
 *      parcTreeMap_RemoveAndRelease(tree1, someKey);
 *
 *      parcObject_Release(&someKey);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
void parcTreeMap_RemoveAndRelease(PARCTreeMap *tree, const PARCObject *key);

/**
 * Get the size (nuber of elements) of a `PARCTreeMap`.
 *
 *
 * @param [in] tree A pointer to an initialized `PARCTreeMap`.
 * @return size of the tree (number of elements).
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      if (parcTreeMap_Size(tree1) > 0) {
 *         ...
 *      }
 *
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
size_t parcTreeMap_Size(const PARCTreeMap *tree);

/**
 * Get a PARCList of the keys from a `PARCTreeMap`.  All keys will be
 * valid keys in the `PARCTreeMap`.  The caller will own the list of
 * keys and should release it when done.  The caller will not own the
 * keys themselves.  Note that if the tree is modified or destroyed
 * the key pointers might no longer point to valid keys.  The list of
 * keys will be sorted as per the provided or key's default compare function.
 *
 * @param [in] tree A pointer to a `PARCTreeMap`.
 * @return A list of (pointers to) keys.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCList *keys = parcTreeMap_AcquireKeys(tree1);
 *      for (size_t i=0; i < parcList_Size(keys); ++i) {
 *         ...
 *      }
 *
 *      parcList_Release(&keys);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCList *parcTreeMap_AcquireKeys(const PARCTreeMap *tree);

/**
 * Get an PARCList of the values in this `PARCTreeMap`. All of these
 * values will be valid in the `PARCTreeMap`.  The caller will own the
 * list of values and should release it when done.  The list of values
 * will be sorted as per the provided or key's default compare
 * function.
 *
 * @param [in] tree A pointer to a `PARCTreeMap`.
 * @return A list of (pointers to) values.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      PARCList *values = parcTreeMap_AcquireValues(tree1);
 *      for (size_t i=0; i < parcList_Size(values); ++i) {
 *         ...
 *      }
 *
 *      parcList_Release(&keys);
 *      parcTreeMap_Release(&tree1);
 * }
 * @endcode
 */
PARCList *parcTreeMap_AcquireValues(const PARCTreeMap *tree);

/**
 * Calculate if two `PARCTreeMap`s are equal.
 *
 * Two trees are equal if they have the same keys associated with the
 * same values. The keys & values will be compared using the
 * parcObject_Equals(...);
 *
 * @param [in] tree1 A pointer to a `PARCTreeMap`.
 * @param [in] tree2 A pointer to another `PARCTreeMap`.
 * @return true if the trees are equal, zero otherwise
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * tree1 = parcTreeMap_Create(.....);
 *      PARCTreeMap * tree2 = parcTreeMap_Create(.....);
 *
 *      ...
 *
 *      if (parcTreeMap_Equals(tree1, tree2)) {
 *         ...
 *      }
 *
 *      parcTreeMap_Release(&tree1);
 *      parcTreeMap_Release(&tree2);
 * }
 * @endcode
 */
bool parcTreeMap_Equals(const PARCTreeMap *tree1, const PARCTreeMap *tree2);

/**
 * Copy a TreeMap.
 *
 * This will create a completely new tree. It will copy every key and
 * every value using parcObject_Copy(...).
 *
 * @param [in] sourceTree A pointer to a `PARCTreeMap` to be copied
 * @return NULL Error copying the tree.
 * @return Non-NULL A copy of the `PARCTreeMap`.
 *
 * Example:
 * @code
 * {
 *      PARCTreeMap * source_tree = parcTreeMap_Create(.....);
 *
 *      ... operations on tree ...
 *
 *      PARCTreeMap * tree_copy = parcTreeMap_Copy(source_tree);
 *
 *      ... operations on tree ...
 *
 *      parcTreeMap_Release(&source_tree);
 *      parcTreeMap_Release(&tree_copy);
 * }
 * @endcode
 */
PARCTreeMap *parcTreeMap_Copy(const PARCTreeMap *sourceTree);


/**
 * Create a new instance of PARCIterator that iterates through the keys of the specified `PARCTreeMap`.
 * The returned iterator must be released via {@link parcIterator_Release}.
 *
 * @param [in] hashMap A pointer to a valid `PARCTreeMap`.
 *
 * @see parcIterator_Release
 * Example:
 * @code
 * {
 *    PARCIterator *iterator = parcTreeMap_CreateKeyIterator(myTreeMap);
 *
 *    while (parcIterator_HasNext(iterator)) {
 *        PARCObject *object = parcIterator_Next(iterator);
 *    }
 *
 *    parcIterator_Release(&iterator);
 * }
 * @endcode
 */
PARCIterator *parcTreeMap_CreateKeyIterator(PARCTreeMap *tree);

/**
 * Create a new instance of PARCIterator that iterates through the values of the specified `PARCTreeMap`.
 * The returned iterator must be released via {@link parcIterator_Release}.
 *
 * @param [in] hashMap A pointer to a valid `PARCTreeMap`.
 *
 * @see parcIterator_Release
 * Example:
 * @code
 * {
 *    PARCIterator *iterator = parcTreeMap_CreateValueIterator(myTreeMap);
 *
 *    while (parcIterator_HasNext(iterator)) {
 *        PARCObject *object = parcIterator_Next(iterator);
 *    }
 *
 *    parcIterator_Release(&iterator);
 * }
 * @endcode
 */
PARCIterator *parcTreeMap_CreateValueIterator(PARCTreeMap *tree);

/**
 * Create a new instance of PARCIterator that iterates through the KeyValue elements of the specified `PARCTreeMap`.
 * The returned iterator must be released via {@link parcIterator_Release}.
 *
 * @param [in] hashMap A pointer to a valid `PARCTreeMap`.
 *
 * @see parcIterator_Release
 * Example:
 * @code
 * {
 *    PARCIterator *iterator = parcTreeMap_CreateKeyValueIterator(myTreeMap);
 *
 *    while (parcIterator_HasNext(iterator)) {
 *        PARCObject *object = parcIterator_Next(iterator);
 *    }
 *
 *    parcIterator_Release(&iterator);
 * }
 * @endcode
 */
PARCIterator *parcTreeMap_CreateKeyValueIterator(PARCTreeMap *tree);
#endif // libparc_parc_TreeMap_h
