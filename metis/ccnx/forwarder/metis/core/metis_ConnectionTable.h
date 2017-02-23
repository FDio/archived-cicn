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

#ifndef Metis_metis_ConnectionTable_h
#define Metis_metis_ConnectionTable_h

#include <ccnx/forwarder/metis/core/metis_Connection.h>
#include <ccnx/forwarder/metis/core/metis_ConnectionList.h>
#include <ccnx/forwarder/metis/io/metis_IoOperations.h>
#include <ccnx/forwarder/metis/io/metis_AddressPair.h>

struct metis_connection_table;
typedef struct metis_connection_table MetisConnectionTable;

/**
 * Creates an empty connection table
 *
 * <#Paragraphs Of Explanation#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisConnectionTable *metisConnectionTable_Create(void);

/**
 * Destroys the connection table
 *
 * This will release the reference to all connections stored in the connection table.
 *
 * @param [in,out] conntablePtr Pointer to the allocated connection table, will be NULL'd
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisConnectionTable_Destroy(MetisConnectionTable **conntablePtr);

/**
 * @function metisConnectionTable_Add
 * @abstract Add a connection, takes ownership of memory
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void metisConnectionTable_Add(MetisConnectionTable *table, MetisConnection *connection);

/**
 * @function metisConnectionTable_Remove
 * @abstract Removes the connection, calling Destroy on our copy
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void metisConnectionTable_Remove(MetisConnectionTable *table, const MetisConnection *connection);

/**
 * Removes a connection from the connection table
 *
 * Looks up a connection by its connection ID and removes it from the connection table.
 * Removing the connection will call metisConnection_Release() on the connection object.
 *
 * @param [in] table The allocated connection table
 * @param [in] id The connection ID
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisConnectionTable_RemoveById(MetisConnectionTable *table, unsigned id);

/**
 * Lookup a connection by the (local, remote) addres pair
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] table The allocated connection table
 * @param [in] pair The address pair to match, based on the inner values of the local and remote addresses
 *
 * @retval non-null The matched conneciton
 * @retval null No match found or error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const MetisConnection *metisConnectionTable_FindByAddressPair(MetisConnectionTable *table, const MetisAddressPair *pair);

/**
 * @function metisConnectionTable_FindById
 * @abstract Find a connection by its numeric id.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return NULL if not found
 */
const MetisConnection *metisConnectionTable_FindById(MetisConnectionTable *table, unsigned id);

/**
 * @function metisConnectionTable_GetEntries
 * @abstract Returns a list of connections.  They are reference counted copies from the table.
 * @discussion
 *   An allocated list of connections in the table.  Each list entry is a reference counted
 *   copy of the connection in the table, thus they are "live" objects.
 *
 * @param <#param1#>
 * @return An allocated list, which you must destroy
 */
MetisConnectionList *metisConnectionTable_GetEntries(const MetisConnectionTable *table);
#endif // Metis_metis_ConnectionTable_h
