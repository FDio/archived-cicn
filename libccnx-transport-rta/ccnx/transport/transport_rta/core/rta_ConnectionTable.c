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

/*
 * Uses a linked list right now, but should be hash tables on the keys we use.
 */
#include <config.h>
#include <stdio.h>
#include <sys/queue.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_Services.h>
#include <ccnx/transport/transport_rta/core/rta_ConnectionTable.h>

#define DEBUG_OUTPUT 0

typedef struct rta_connection_entry {
    RtaConnection *connection;

    TAILQ_ENTRY(rta_connection_entry) list;
} RtaConnectionEntry;

struct rta_connection_table {
    size_t max_elements;
    size_t count_elements;
    TableFreeFunc *freefunc;
    TAILQ_HEAD(, rta_connection_entry) head;
};


/**
 * Create a connection table of the given size
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaConnectionTable *
rtaConnectionTable_Create(size_t elements, TableFreeFunc *freefunc)
{
    RtaConnectionTable *table = parcMemory_AllocateAndClear(sizeof(RtaConnectionTable));
    assertNotNull(table, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(RtaConnectionTable));
    TAILQ_INIT(&table->head);
    table->max_elements = elements;
    table->count_elements = 0;
    table->freefunc = freefunc;
    return table;
}

/**
 * Destroy the connection table, and it will call rtaConnection_Destroy()
 * on each connection in the table.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
rtaConnectionTable_Destroy(RtaConnectionTable **tablePtr)
{
    RtaConnectionTable *table;

    assertNotNull(tablePtr, "Called with null parameter");
    table = *tablePtr;
    assertNotNull(table, "Called with parameter that dereferences to null");

    while (!TAILQ_EMPTY(&table->head)) {
        RtaConnectionEntry *entry = TAILQ_FIRST(&table->head);
        TAILQ_REMOVE(&table->head, entry, list);
        if (table->freefunc) {
            table->freefunc(&entry->connection);
        }
        parcMemory_Deallocate((void **) &entry);
    }

    parcMemory_Deallocate((void **) &table);
    *tablePtr = NULL;
}

/**
 * Add a connetion to the table.  Stores the reference provided (does not copy).
 * Returns 0 on success, -1 on error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaConnectionTable_AddConnection(RtaConnectionTable *table, RtaConnection *connection)
{
    assertNotNull(table, "Called with null parameter RtaConnectionTable");
    assertNotNull(connection, "Called with null parameter RtaConnection");

    if (table->count_elements < table->max_elements) {
        table->count_elements++;
        RtaConnectionEntry *entry = parcMemory_AllocateAndClear(sizeof(RtaConnectionEntry));
        assertNotNull(entry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(RtaConnectionEntry));
        entry->connection = connection;
        TAILQ_INSERT_TAIL(&table->head, entry, list);
        return 0;
    }
    return -1;
}

/**
 * Lookup a connection.
 * Returns NULL if not found
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaConnection *
rtaConnectionTable_GetByApiFd(RtaConnectionTable *table, int api_fd)
{
    assertNotNull(table, "Called with null parameter RtaConnectionTable");

    RtaConnectionEntry *entry;
    TAILQ_FOREACH(entry, &table->head, list)
    {
        if (rtaConnection_GetApiFd(entry->connection) == api_fd) {
            return entry->connection;
        }
    }
    return NULL;
}

/**
 * Lookup a connection.
 * Returns NULL if not found
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaConnection *
rtaConnectionTable_GetByTransportFd(RtaConnectionTable *table, int transport_fd)
{
    assertNotNull(table, "Called with null parameter RtaConnectionTable");

    RtaConnectionEntry *entry;
    TAILQ_FOREACH(entry, &table->head, list)
    {
        if (rtaConnection_GetTransportFd(entry->connection) == transport_fd) {
            return entry->connection;
        }
    }
    return NULL;
}


/**
 * Remove a connection from the table, calling rtaConnection_Destroy() on it.
 * Returns 0 on success, -1 if not found (or error)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaConnectionTable_Remove(RtaConnectionTable *table, RtaConnection *connection)
{
    assertNotNull(table, "Called with null parameter RtaConnectionTable");
    assertNotNull(connection, "Called with null parameter RtaConnection");

    RtaConnectionEntry *entry;
    TAILQ_FOREACH(entry, &table->head, list)
    {
        if (entry->connection == connection) {
            assertTrue(table->count_elements > 0, "Invalid state, found an entry, but count_elements is zero");
            table->count_elements--;
            TAILQ_REMOVE(&table->head, entry, list);
            if (table->freefunc) {
                table->freefunc(&entry->connection);
            }
            parcMemory_Deallocate((void **) &entry);
            return 0;
        }
    }
    return -1;
}

/**
 * Remove all connections in a given stack_id, calling rtaConnection_Destroy() on it.
 * Returns 0 on success, -1 if not found (or error)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaConnectionTable_RemoveByStack(RtaConnectionTable *table, int stack_id)
{
    assertNotNull(table, "Called with null parameter RtaConnectionTable");

    RtaConnectionEntry *entry = TAILQ_FIRST(&table->head);
    while (entry != NULL) {
        RtaConnectionEntry *temp = TAILQ_NEXT(entry, list);
        if (rtaConnection_GetStackId(entry->connection) == stack_id) {
            assertTrue(table->count_elements > 0, "Invalid state, found an entry, but count_elements is zero");
            table->count_elements--;

            if (DEBUG_OUTPUT) {
                printf("%9" PRIu64 "%s stack_id %d conn %p\n",
                       rtaFramework_GetTicks(rtaConnection_GetFramework(entry->connection)),
                       __func__,
                       stack_id,
                       (void *) entry->connection);
            }

            TAILQ_REMOVE(&table->head, entry, list);
            if (table->freefunc) {
                table->freefunc(&entry->connection);
            }

            if (DEBUG_OUTPUT) {
                printf("%9s %s FREEFUNC RETURNS\n",
                       " ", __func__);
            }

            parcMemory_Deallocate((void **) &entry);
        }
        entry = temp;
    }
    return 0;
}
