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
 * @file parc_Map.h
 * @ingroup datastructures
 * @brief An object that maps keys to values.
 *
 * A map cannot contain duplicate keys; each key can map to at most one value.
 *
 */
#ifndef libparc_parc_Map_h
#define libparc_parc_Map_h
#include <stdbool.h>

struct parc_map;
typedef struct parc_map PARCMap;

typedef struct parc_map_interface {
    /**
     * Removes all of the mappings from this map.
     *
     * The map will be empty after this call returns.
     *
     * @param [in,out] map The instance of `PARCMap` to be cleared of mappings
     *
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     *
     */
    void (*parcMap_Clear)(PARCMap *map);

    /**
     * Returns true if this map contains a mapping for the specified key.
     *
     * @param [in] map A pointer to the instance of `PARCMap` to check
     * @param [in] key A pointer to the key to check for in @p map
     *
     * @return True if the map cnatins a mapping for the specified key
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     */
    bool (*parcMap_ContainsKey)(PARCMap *map, void *key);

    /**
     * Returns true if this map maps one or more keys to the specified value.
     *
     * @param [in] map A pointer to the instance of `PARCMap` to check
     * @param [in] value A pointer to the value to check for in @p map
     *
     * @return True if the map contains one or more keys that map to @p value.
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     */
    bool (*parcMap_ContainsValue)(PARCMap *map, void *value);

    /**
     * Compares the specified object with this map for equality.
     *
     * @param [in] map A pointer to the instance of `PARCMap` to check
     * @param [in] other A pointer to the other instance of `PARCMap` to compare
     * @return  True is the two maps are equal.
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     *
     */
    bool (*parcMap_Equals)(PARCMap *map, void *other);

    /**
     * Returns the value to which the specified key is mapped, or null if this map contains no mapping for the key.
     *
     * @param [in] map A pointer to the instance of `PARCMap` to check
     * @param [in] key A pointer to the key to check for in @p map
     *
     * @return NULL If the @p key is not present in @p map
     * @return NOT NULL The value to which the @p key is mapped.
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     *
     */
    void *(*parcMap_Get)(PARCMap * map, void *key);

    /**
     * Returns the hash code value for this map.
     *
     * @param [in] map A pointer to the instance of `PARCMap` to hash
     *
     * @return The hash of the instance of `PARCMap`
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     */
    int (*parcMap_HashCode)(PARCMap *map);

    /**
     * Returns true if this map contains no key-value mappings.
     *
     *
     * @param [in] map A pointer to the instance of `PARCMap` to check
     *
     * @return True if the map contains no mappings.
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     */
    bool (*parcMap_IsEmpty)(PARCMap *map);

    /**
     * Associates the specified value with the specified key in this map (optional operation).
     *
     * @param [in,out] map A pointer to the instance of `PARCMap` in which to insert @p value at @p key.
     * @param [in] key A pointer to the key in @p map in which to insert @p value.
     * @param [in] value A pointer to the the value to insert at @p key in @p map.
     *
     * @return
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     */
    void *(*parcMap_Put)(PARCMap * map, void *key, void *value);

    /**
     * Copies all of the mappings from the specified map to this map (optional operation).
     *
     * @param [in,out] map The instance of `PARCMap` to be modified.
     * @param [in] other The instance of `PARCMap` whose mappings should be copied to @p map.
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     */
    void (*parcMap_PutAll)(PARCMap *map, PARCMap *other);

    /**
     * Removes the mapping for a key from this map if it is present (optional operation).
     *
     * @param [in,out] map The instance of `PARCMap` to be modified.
     * @param [in] key The key to the mapping to be removed
     *
     * @return
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     */
    void *(*parcMap_Remove)(PARCMap * map, void *key);

    /**
     * Returns the number of key-value mappings in this map.
     *
     * @param [in,out] map The instance of `PARCMap` to be inspected.
     *
     * @return int The number of mappings in the map
     *
     * Example:
     * @code
     * {
     *     <#example#>
     * }
     * @endcode
     *
     */
    int (*parcMap_Size)(PARCMap *map);
} PARCMapInterface;

/**
 * Create a PARCMap instance.
 *
 * Create an instance of `PARCMap` wrapping the given pointer to a base map
 * interface and the {@ link PARCMapInterface} structure containing pointers
 * to functions performing the actual Map operations.
 *
 * @param [in] map A pointer to the structure for the new instance of `PARCMap`
 * @param [in] interface A pointer to the instance of `PARCMapInterface`
 * @return A new instance of `PARCMap`
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCMap *parcMap_Create(void *map, PARCMapInterface *interface);

/**
 * Removes all of the mappings from this map.
 *
 * The map will be empty after this call returns.
 *
 * @param [in,out] map A pointer to the instance of `PARCMap` to be cleared.
 *
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcMap_Clear(PARCMap *map);

/**
 * Returns true if this map contains a mapping for the specified key.
 *
 * @param [in] map A pointer to the instance of `PARCMap` to be checked.
 * @param [in] key A pointer to the key to be checked for in @p map
 *
 * @return True if the specified key is found in the map.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcMap_ContainsKey(PARCMap *map, void *key);

/**
 * Returns true if this map maps one or more keys to the specified value.
 *
 * @param [in] map A pointer to the instance of `PARCMap` to be checked.
 * @param [in] value A pointer to the value to be checked for in @p map
 *
 * @return True if the specified value has one or more keys pointing to it.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcMap_ContainsValue(PARCMap *map, void *value);

/**
 * Determine if two `PARCMap` instances are equal.
 *
 * Two `PARCMap` instances are equal if, and only if, the maps have the same
 * number of elements, all of the keys are equal and the values to which they point are equal
 *
 * The following equivalence relations on non-null `PARCMap` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `PARCMap_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `parcMap_Equals(x, y)` must return true if and only if
 *        `parcMap_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcMap_Equals(x, y)` returns true and
 *        `parcMap_Equals(y, z)` returns true,
 *        then  `parcMap_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `parcMap_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `parcMap_Equals(x, NULL)` must
 *      return false.
 *
 * @param [in] map A pointer to a `PARCMap` instance.
 * @param [in] other A pointer to a `PARCMap` instance.
 * @return true if the two `PARCMap` instances are equal.
 *
 * Example:
 * @code
 * {
 *    PARCMap *a = parcMap_Create();
 *    PARCMap *b = parcMap_Create();
 *
 *    if (parcMap_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool parcMap_Equals(PARCMap *map, void *other);

/**
 * Returns the value to which the specified key is mapped, or null if this map contains no mapping for the key.
 *
 * @param [in] map A pointer to the instance of `PARCMap` to be checked.
 * @param [in] key A pointer to the key to be checked for which the value is to be returned.
 *
 * @return Null if no mapping for @p key exists
 * @return Non Null A pointer to the value for @p key
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void *parcMap_Get(PARCMap *map, void *key);

/**
 * Returns the hash code value for this map.
 *
 * @param [in] map A pointer to the instance of `PARCMap` to be hashed.
 *
 * @return The hash value for the @p map
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
int parcMap_HashCode(PARCMap *map);

/**
 * Returns true if this map contains no key-value mappings.
 *
 * @param [in] map A pointer to the instance of `PARCMap` to be checked.
 *
 * @return True if the @p map is empty. else false.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcMap_IsEmpty(PARCMap *map);

/**
 * Associates the specified value with the specified key in this map (optional operation).
 *
 * @param [in,out] map A pointer to the instance of `PARCMap` into which the key,value pair should be inserted.
 * @param [in] key A pointer to the key to be inserted in @p map
 * @param [in] value A pointer to the value to be inserted in @p map at @p key
 *
 * @return The previous value at @p key if one exists, else NULL
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void *parcMap_Put(PARCMap *map, void *key, void *value);

/**
 * Copies all of the mappings from the specified map to this map (optional operation).
 *
 * @param [in,out] map The map into which all the mappings from @p other should be copied.
 * @param [in] other The instance of `PARCMap` whose mappings should be copied into @p map
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcMap_PutAll(PARCMap *map, PARCMap *other);

/**
 * Removes the mapping for a key from this map if it is present (optional operation).
 *
 *
 * @param [in,out] map The instance of `PARCMap` in which @p key should be removed if present.
 * @param [in] key The pointer to the key representing the mapping that should be removed from @p map.
 *
 * @return A pointer to the value previously mapped to @p key, if @p key exists.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void *parcMap_Remove(PARCMap *map, void *key);

/**
 * Returns the number of key-value mappings in this map.
 *
 * @param [in,out] map The instance of `PARCMap` to be measured
 *
 * @return  The number of mappings in @p map.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
int parcMap_Size(PARCMap *map);
#endif // libparc_parc_Map_h
