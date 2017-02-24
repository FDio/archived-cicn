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
 * @file metis_ConnectionList.h
 * @brief A typesafe list of MetisConnection objects
 *
 * <#Detailed Description#>
 *
 */

#ifndef Metis_metis_ConnectionList_h
#define Metis_metis_ConnectionList_h

struct metis_connection_list;
typedef struct metis_connection_list MetisConnectionList;

#include <ccnx/forwarder/metis/core/metis_Connection.h>

/**
 * Creates a lis of MetisConnection
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-null An allocated list
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisConnectionList *metisConnectionList_Create(void);

/**
 * Destroys the list and all objects inside it
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisConnectionList_Destroy(MetisConnectionList **listPtr);

/**
 * @function metisConnectionList_Append
 * @abstract Adds a connection entry to the list.
 * @discussion
 *   Acquires a reference to the passed entry and stores it in the list.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisConnectionList_Append(MetisConnectionList *list, MetisConnection *entry);

/**
 * Returns the number of items on the list
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] list The allocated list to check
 *
 * @return number The number of items on the list
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t metisConnectionList_Length(const MetisConnectionList *list);

/**
 * @function metisConnectionList_Get
 * @abstract Returns the connection entry.
 * @discussion
 *   Caller must not destroy the returned value.  If you will store the
 *   entry in your own data structure, you should acquire your own reference.
 *   Will assert if you go beyond the end of the list.
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisConnection *metisConnectionList_Get(MetisConnectionList *list, size_t index);
#endif // Metis_metis_ConnectionList_h
