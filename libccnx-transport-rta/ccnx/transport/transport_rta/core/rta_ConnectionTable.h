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
 * @file rta_ConnectionTable.h
 * @brief Data structure of connections.  It is managed by rtaFramework.
 *
 */

#ifndef Libccnx_rta_ConnectionTable_h
#define Libccnx_rta_ConnectionTable_h

#include "rta_Connection.h"

struct rta_connection_table;
typedef struct rta_connection_table RtaConnectionTable;

typedef void (TableFreeFunc)(RtaConnection **connection);

/**
 * Create a connection table of the given size.  Whenever a
 * connection is removed, the freefunc is called.  Be sure that
 * does not in turn call back in to the connection table.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaConnectionTable *rtaConnectionTable_Create(size_t elements, TableFreeFunc *freefunc);

/**
 * Destroy the connection table, and it will call freefunc()
 * on each connection in the table.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaConnectionTable_Destroy(RtaConnectionTable **tablePtr);

/**
 * Add a connetion to the table.  Stores the reference provided (does not copy).
 * Returns 0 on success, -1 on error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int rtaConnectionTable_AddConnection(RtaConnectionTable *table, RtaConnection *connection);

/**
 * Lookup a connection.
 * Returns NULL if not found
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaConnection *rtaConnectionTable_GetByApiFd(RtaConnectionTable *table, int api_fd);

/**
 * Lookup a connection.
 * Returns NULL if not found
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaConnection *rtaConnectionTable_GetByTransportFd(RtaConnectionTable *table, int transport_fd);

/**
 * Remove a connection from the table, calling freefunc() on it.
 * Returns 0 on success, -1 if not found (or error)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int rtaConnectionTable_Remove(RtaConnectionTable *table, RtaConnection *connection);

/**
 * Remove all connections in a given stack_id, calling freefunc() on it.
 * Returns 0 on success, -1 if not found (or error)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int rtaConnectionTable_RemoveByStack(RtaConnectionTable *table, int stack_id);
#endif
