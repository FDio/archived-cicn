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
 * @header MetisConnectionTable
 * @abstract Records all the current connections and references to them
 * @discussion
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/core/metis_ConnectionTable.h>
#include <ccnx/forwarder/metis/io/metis_AddressPair.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_HashCodeTable.h>
#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_TreeRedBlack.h>

struct metis_connection_table {
    // The main storage table that has a Destroy method.
    // The key is an unsigned int pointer.  We use an unsigned int pointer
    // because we want to be able to lookup by the id alone, and not have to
    // have the MetisIoOperations everywhere.
    PARCHashCodeTable *storageTableById;

    // The key is a MetisAddressPair
    // It does not have a destroy method for the data or key,
    // as they are derived from the storage table.
    PARCHashCodeTable *indexByAddressPair;

    // An iterable stucture organized by connection id.  The keys and
    // values are the same pointers as in storageTableById, so there
    // are no destructors in the tree.
    // The only reason to keep this tree is so we have an iterable list
    // of connections, which the hash table does not give us.
    PARCTreeRedBlack *listById;
};

static bool
metisConnectionTable_ConnectionIdEquals(const void *keyA, const void *keyB)
{
    unsigned idA = *((unsigned  *) keyA);
    unsigned idB = *((unsigned  *) keyB);
    return (idA == idB);
}

static int
metisConnectionTable_ConnectionIdCompare(const void *keyA, const void *keyB)
{
    unsigned idA = *((unsigned  *) keyA);
    unsigned idB = *((unsigned  *) keyB);
    if (idA < idB) {
        return -1;
    }
    if (idA > idB) {
        return +1;
    }
    return 0;
}

static bool
metisConnectionTable_AddressPairEquals(const void *keyA, const void *keyB)
{
    const MetisAddressPair *pairA = (const MetisAddressPair *) keyA;
    const MetisAddressPair *pairB = (const MetisAddressPair *) keyB;

    return metisAddressPair_Equals(pairA, pairB);
}

static HashCodeType
metisConnectionTable_ConnectionIdHashCode(const void *keyA)
{
    unsigned idA = *((unsigned  *) keyA);
    return parcHash32_Int32(idA);
}

static HashCodeType
metisConnectionTable_AddressPairHashCode(const void *keyA)
{
    const MetisAddressPair *pairA = (const MetisAddressPair *) keyA;
    return metisAddressPair_HashCode(pairA);
}


static void
metisConnectionTable_ConnectionIdDestroyer(void **dataPtr)
{
    unsigned *idA = (unsigned *) *dataPtr;
    parcMemory_Deallocate((void **) &idA);
    *dataPtr = NULL;
}

static void
metisConnectionTable_ConnectionDestroyer(void **dataPtr)
{
    metisConnection_Release((MetisConnection **) dataPtr);
}

MetisConnectionTable *
metisConnectionTable_Create()
{
    size_t initialSize = 16384;

    MetisConnectionTable *conntable = parcMemory_AllocateAndClear(sizeof(MetisConnectionTable));
    assertNotNull(conntable, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisConnectionTable));

    conntable->storageTableById = parcHashCodeTable_Create_Size(metisConnectionTable_ConnectionIdEquals,
                                                                metisConnectionTable_ConnectionIdHashCode,
                                                                metisConnectionTable_ConnectionIdDestroyer,
                                                                metisConnectionTable_ConnectionDestroyer,
                                                                initialSize);

    // no key or data destroyer, this is an index into storageByid.
    conntable->indexByAddressPair = parcHashCodeTable_Create_Size(metisConnectionTable_AddressPairEquals,
                                                                  metisConnectionTable_AddressPairHashCode,
                                                                  NULL,
                                                                  NULL,
                                                                  initialSize);

    conntable->listById = parcTreeRedBlack_Create(metisConnectionTable_ConnectionIdCompare,
                                                  NULL,  // key free
                                                  NULL,  // key copy
                                                  NULL,  // value equals
                                                  NULL,  // value free
                                                  NULL); // value copy

    return conntable;
}


void
metisConnectionTable_Destroy(MetisConnectionTable **conntablePtr)
{
    assertNotNull(conntablePtr, "Parameter must be non-null double pointer");
    assertNotNull(*conntablePtr, "Parameter must dereference to non-null pointer");

    MetisConnectionTable *conntable = *conntablePtr;

    parcTreeRedBlack_Destroy(&conntable->listById);
    parcHashCodeTable_Destroy(&conntable->indexByAddressPair);
    parcHashCodeTable_Destroy(&conntable->storageTableById);
    parcMemory_Deallocate((void **) &conntable);
    *conntablePtr = NULL;
}

/**
 * @function metisConnectionTable_Add
 * @abstract Add a connection, takes ownership of memory
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void
metisConnectionTable_Add(MetisConnectionTable *table, MetisConnection *connection)
{
    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(connection, "Parameter connection must be non-null");

    unsigned *connectionIdKey = parcMemory_Allocate(sizeof(unsigned));
    assertNotNull(connectionIdKey, "parcMemory_Allocate(%zu) returned NULL", sizeof(unsigned));
    *connectionIdKey = metisConnection_GetConnectionId(connection);

    if (parcHashCodeTable_Add(table->storageTableById, connectionIdKey, connection)) {
        parcHashCodeTable_Add(table->indexByAddressPair, (void *) metisConnection_GetAddressPair(connection), connection);
        parcTreeRedBlack_Insert(table->listById, connectionIdKey, connection);
    } else {
        trapUnexpectedState("Could not add connection id %u -- is it a duplicate?", *connectionIdKey);
    }
}

/**
 * @function metisConnectionTable_Remove
 * @abstract Removes the connection, calling Destroy on our copy
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void
metisConnectionTable_Remove(MetisConnectionTable *table, const MetisConnection *connection)
{
    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(connection, "Parameter connection must be non-null");

    unsigned connid = metisConnection_GetConnectionId(connection);

    parcTreeRedBlack_Remove(table->listById, &connid);
    parcHashCodeTable_Del(table->indexByAddressPair, metisConnection_GetAddressPair(connection));
    parcHashCodeTable_Del(table->storageTableById, &connid);
}

void
metisConnectionTable_RemoveById(MetisConnectionTable *table, unsigned id)
{
    assertNotNull(table, "Parameter table must be non-null");
    const MetisConnection *connection = metisConnectionTable_FindById(table, id);
    if (connection) {
        metisConnectionTable_Remove(table, connection);
    }
}

const MetisConnection *
metisConnectionTable_FindByAddressPair(MetisConnectionTable *table, const MetisAddressPair *pair)
{
    assertNotNull(table, "Parameter table must be non-null");
    return (MetisConnection *) parcHashCodeTable_Get(table->indexByAddressPair, pair);
}

const MetisConnection *
metisConnectionTable_FindById(MetisConnectionTable *table, unsigned id)
{
    assertNotNull(table, "Parameter table must be non-null");
    return (MetisConnection *) parcHashCodeTable_Get(table->storageTableById, &id);
}


MetisConnectionList *
metisConnectionTable_GetEntries(const MetisConnectionTable *table)
{
    assertNotNull(table, "Parameter table must be non-null");
    MetisConnectionList *list = metisConnectionList_Create();

    PARCArrayList *values = parcTreeRedBlack_Values(table->listById);
    for (size_t i = 0; i < parcArrayList_Size(values); i++) {
        MetisConnection *original = parcArrayList_Get(values, i);
        metisConnectionList_Append(list, original);
    }
    parcArrayList_Destroy(&values);
    return list;
}
