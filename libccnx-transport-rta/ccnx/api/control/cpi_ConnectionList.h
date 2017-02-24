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
 * @file cpi_ConnectionList.h
 * @brief A list of CPIConnection objects
 *
 * <#Detailed Description#>
 *
 */
#ifndef libccnx_cpi_ConnectionList_h
#define libccnx_cpi_ConnectionList_h

struct cpi_connection_list;
typedef struct cpi_connection_list CPIConnectionList;

#include <ccnx/api/control/cpi_Connection.h>

/**
 * Creates an empty list of CPIConnection objects
 *
 *   Each element in the list is reference counted, so the list may persist beyond
 *   what created it.
 *
 * @param <#param1#>
 * @return An allocated list, you must call <code>cpiConnectionList_Destroy()</code>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnectionList *cpiConnectionList_Create(void);

/**
 * Destroys the list and all references in it
 *
 *   Destroys each element in the list, which are reference counted.  Only
 *   on the last destroy of each element is it actually freed.
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void cpiConnectionList_Destroy(CPIConnectionList **listPtr);

/**
 * Adds a iptunnel entry to the list.
 *
 *   Appends <code>entry</code> to the list.  Takes ownership of the entry
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void cpiConnectionList_Append(CPIConnectionList *list, CPIConnection *entry);

/**
 * Determine if two CPIConnectionList instances are equal.
 *
 * Two CPIConnectionList instances are equal if, and only if,they have the same number of objects and
 * the objects -- in order -- are equal.
 *
 * The following equivalence relations on non-null `CPIConnectionList` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIConnectionList_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiConnectionList_Equals(x, y)` must return true if and only if
 *        `cpiConnectionList_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiConnectionList_Equals(x, y)` returns true and
 *        `cpiConnectionList_Equals(y, z)` returns true,
 *        then  `cpiConnectionList_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiConnectionList_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiConnectionList_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIConnectionList` instance.
 * @param b A pointer to a `CPIConnectionList` instance.
 * @return true if the two `CPIConnectionList` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIConnectionList *a = cpiConnectionList_Create();
 *    CPIConnectionList *b = cpiConnectionList_Create();
 *
 *    if (cpiConnectionList_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool cpiConnectionList_Equals(const CPIConnectionList *a, const CPIConnectionList *b);

/**
 * The number of elements in the list
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return The number of elements in the list.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t cpiConnectionList_Length(const CPIConnectionList *list);

/**
 * Returns a reference counted copy of the iptunnel entry.
 *
 *   Caller must destroy the returned value.
 *   Will assert if you go beyond the end of the list.
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnection *cpiConnectionList_Get(CPIConnectionList *list, size_t index);

/**
 * A JSON representation of the list
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *cpiConnectionList_ToJson(const CPIConnectionList *list);

/**
 * Constructs a list
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return An allocated list based on the JSON
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnectionList *cpiConnectionList_FromJson(PARCJSON *json);
#endif // libccnx_cpi_ConnectionList_h
