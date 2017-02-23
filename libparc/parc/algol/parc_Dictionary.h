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
 * @file parc_Dictionary.h
 * @ingroup datastructures
 * @brief
 *
 * The `PARCDictionary` is a dictionary of key value tuples.
 *
 */
#ifndef libparc_parc_Dictionary_h
#define libparc_parc_Dictionary_h

#include <parc/algol/parc_ArrayList.h>

#include <stdint.h>

struct parc_dictionary;


/**
 * @typedef PARCDictionary
 * @brief  A PARCDictionary is a dictionary of key-values
 */
typedef struct parc_dictionary PARCDictionary;

/**
 * @typedef `PARCDictionary_CompareKey`
 *
 * @brief Compare two dictionary keys
 *
 * @param [in] key1 First dictionary key
 * @param [in] key2 Second dictionary key
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef int (*PARCDictionary_CompareKey)(const void *key1,
                                         const void *key2);
/**
 * @typedef `PARCDictionary_ValueEquals`
 *
 * @brief Compare two values
 *
 * @param [in] value1 A pointer to the First value
 * @param [in] value2 A pointer to the Second value
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef bool (*PARCDictionary_ValueEquals)(const void *value1, const void *value2);


/**
 * @typedef `PARCDictionary_KeyHashFunc`
 * @brief The key hash function must return a 32 bit hash of the key
 * @param [in] key pointer to the key to be hashed
 * @return uint32 hash
 */

typedef uint32_t (*PARCDictionary_KeyHashFunc)(const void *key);

/**
 * @typedef `PARCDictionary_FreeValue`
 * @brief The free value must free a value.
 * This will be called when a value for a key is changed.
 * It will also be used when the Dictionary is destroyed on any
 * values in the dictionary.
 * @param [in,out] value The pointer to the pointer of the value to be freed.
 */

typedef void (*PARCDictionary_FreeValue)(void **value);

/**
 * @typedef `PARCDictionary_FreeKey`
 * @brief The free key must free a key.
 * This function will be called when the value for a key is removed from the
 * dictionary. It's also called when the dictionary is destroyed.
 * @param [in,out] key  The pointer to the pointer to the key to be freed.
 */

typedef void (*PARCDictionary_FreeKey)(void **key);

/**
 * Create a Dictionary.
 * You MUST set the function to compare keys and hash keys.
 * You can give NULL as the free function of the key and the data,
 * but why would you do that? :-)
 *
 * @param [in] keyCompareFunction The function that compares 2 keys (can't be NULL)
 * @param [in] keyHashFunction The function to hash the keys to 32 bit values (can't be NULL)
 * @param [in] keyFreeFunction The function to free the key (can be NULL)
 * @param [in] valueEqualsFunction The function to know that values are equal. If NULL then values won't be compared on equality.
 * @param [in] valueFreeFunction The function to free the values (can be NULL)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCDictionary *parcDictionary_Create(PARCDictionary_CompareKey keyCompareFunction,
                                      PARCDictionary_KeyHashFunc keyHashFunction,
                                      PARCDictionary_FreeKey keyFreeFunction,
                                      PARCDictionary_ValueEquals valueEqualsFunction,
                                      PARCDictionary_FreeValue valueFreeFunction);

/**
 * Destroy a Dictionary. If the Free functions were passed to the constructor and are not NULL
 * they will be called for every element.
 *
 * @param [in,out] dictionaryPointer A pointer to the pointer to the instance of `PARCDictionary` to be destroyed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcDictionary_Destroy(PARCDictionary **dictionaryPointer);

/**
 * Set a value for a key.
 * Both key and value will be referenced by the Dictionary. No memory copies will happen.
 *
 * If the key does not exist a new dictionary entry will be added pointing to the value.
 *
 * If the key does exists then the old value will be freed using the valueFree function (if not NULL).
 * The old key will also be freed using the keyFree function (if not NULL).
 *
 * @param [in,out] dictionary A pointer to the specified `PARCDictionary`
 * @param [in] key   The key to insert in the dictionary. Can't be NULL.
 * @param [in] value The value to associate with that key. Can't be NULL.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

void parcDictionary_SetValue(PARCDictionary *dictionary, void *key, void *value);

/**
 * Get a value from the dictionary associated with a specific key.
 *
 * @param [in] dictionary A PARCDictionary
 * @param [in] key A pointer to the key. It must be valid as a parameter to the key compare function of the
 *   dictionary
 * @return A pointer to the value.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcDictionary_GetValue(PARCDictionary *dictionary, const void *key);

/**
 * Remove a value from the dictionary.  This will return the value referenced by the key. It will remove it
 *   in the process.  The key will be freed with the keyFree function (if not NULL)
 *
 * @param [in,out] dictionary A `PARCDictionary`
 * @param [in] key A pointer to the key. It must be valid as a parameter to the key compare function of the dictionary.
 * @return A pointer to the value.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcDictionary_RemoveValue(PARCDictionary *dictionary, const void *key);

/**
 * Delete an entry from the dictionary.
 * This will remove an entry from the dictionary and free both the key and the value using the functions
 *   provided at the creation of the dictionary.
 *
 * @param [in,out] dictionary A pointer to an instance of `PARCDictionary`
 * @param [in] key A pointer to the key. It must be valid as a parameter to the key compare function of the dictionary.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcDictionary_RemoveAndDestroyValue(PARCDictionary *dictionary, const void *key);

/**
 * Get a list of the keys from this dictionary.
 *
 * @param [in] dictionary A `PARCDictionary`
 * @return A pointer to a {@link PARCArrayList} of (pointers to) keys.  All of these keys will be valid keys in the dictionary.
 * The caller will own the list of keys and should destroy it when done.  The caller will not own
 * the key themselves. (Destroying the `PARCArrayList` should be enough).
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCArrayList *parcDictionary_Keys(const PARCDictionary *dictionary);

/**
 * Get a list of the values from this dictionary.
 * The caller will own the list of values and should destroy it when done.  The caller will not own
 * the values themselves. Destroying the {@link PARCArrayList} should be enough.
 *
 * Note that if the Dictionary is destroyed the value pointers might no longer point to valid values.
 *
 * @param [in] dictionary A pointer to an instance of `PARCDictionary`
 * @return A pointer to a `PARCArrayList` of (pointers to) values.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCArrayList *parcDictionary_Values(const PARCDictionary *dictionary);

/**
 * Return the number of entries in the dictionary.
 *
 * @param [in] dictionary A pointer to an instance of `PARCDictionary`
 * @return The number of keys in the dictionary.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcDictionary_Size(const PARCDictionary *dictionary);


/**
 * Determine if two `PARCDictionary` instances are equal.
 *
 * Two `PARCDictionary` instances are equal if, and only if, the trees are equal
 *
 * The following equivalence relations on non-null `PARCDictionary` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `PARCDictionary_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `parcDictionary_Equals(x, y)` must return true if and only if
 *        `parcDictionary_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcDictionary_Equals(x, y)` returns true and
 *        `parcDictionary_Equals(y, z)` returns true,
 *        then  `parcDictionary_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `parcDictionary_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `parcDictionary_Equals(x, NULL)` must
 *      return false.
 *
 * @param [in] dictionary1 A pointer to a `PARCDictionary` instance.
 * @param [in] dictionary2 A pointer to a `PARCDictionary` instance.
 * @return true if the two `PARCDictionary` instances are equal.
 *
 * Example:
 * @code
 * {
 *    PARCDictionary *a = parcDictionary_Create();
 *    PARCDictionary *b = parcDictionary_Create();
 *
 *    if (parcDictionary_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
int parcDictionary_Equals(const PARCDictionary *dictionary1, const PARCDictionary *dictionary2);
#endif // libparc_parc_Dictionary_h
