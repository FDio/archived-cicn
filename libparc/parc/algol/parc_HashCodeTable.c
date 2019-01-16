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
 * Implements an open-addressing hash table.  We use linear probing of +1 per step.
 *
 * Table is rehashed when we reach 75% utilization.
 * The table is rehashed if we go more than 10 linear probes without being able to insert.
 *
 * HashCodeTable is a wrapper that holds the key/data management functions.  It also
 * has LinearAddressingHashTable that is the actual hash table.
 *
 * This open-addressing table is inefficient for GET or DEL if the element does not exist.
 * The whole table needs to be
 *
 */
#include <config.h>

#include <parc/assert/parc_Assert.h>

#include <stdio.h>
#include <string.h>

#include <parc/algol/parc_HashCodeTable.h>
#include <parc/algol/parc_Memory.h>

// minimum size if nothing specified
#define MIN_SIZE 256

// when we expand, use this factor
#define EXPAND_FACTOR   2

#define MAX_PROBE_LENGTH 20

typedef enum {
    ADD_OK,     // we added the key
    ADD_DUP,    // the key is a duplicate
    ADD_NOSPACE // ran out of space
} PARCHashCodeTable_AddResult;

typedef struct hashtable_entry {
    // A hashtable entry is in use if the key is non-null
    void *key;
    void *data;
    HashCodeType hashcode;
} HashTableEntry;

typedef struct linear_address_hash_table {
    HashTableEntry  *entries;

    // Number of elements allocated
    size_t tableLimit;

    // Number of elements in use
    size_t tableSize;

    // When the tableSize equals or exceeds this
    // threshold, we should expand and re-hash the table∫
    size_t expandThreshold;
} LinearAddressingHashTable;

struct parc_hashcode_table {
    LinearAddressingHashTable hashtable;

    PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc;
    PARCHashCodeTable_HashCodeFunc keyHashCodeFunc;
    PARCHashCodeTable_Destroyer keyDestroyer;
    PARCHashCodeTable_Destroyer dataDestroyer;

    unsigned expandCount;
};

static bool
_findIndex(PARCHashCodeTable *table, const void *key, size_t *outputIndexPtr)
{
    size_t index, start;
    HashCodeType hashcode;
    LinearAddressingHashTable *innerTable;

    innerTable = &table->hashtable;
    hashcode = table->keyHashCodeFunc(key);
    index = hashcode % innerTable->tableLimit;
    start = index;


    // check until we've gone MAX_PROBE_LENGTH
    unsigned steps = 0;
    do {
        if (innerTable->entries[index].key != NULL) {
            if ((innerTable->entries[index].hashcode == hashcode) && table->keyEqualsFunc(key, innerTable->entries[index].key)) {
                // the key already exists in the table
                *outputIndexPtr = index;
                return true;
            }
        }
        steps++;
        index = index + 1;
        if (index == innerTable->tableLimit) {
            index = 0;
        }
    } while (index != start && steps < MAX_PROBE_LENGTH);

    return false;
}

static PARCHashCodeTable_AddResult
_innerTableAdd(LinearAddressingHashTable *innerTable, PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc,
               HashCodeType hashcode, void *key, void *data)
{
    size_t index = hashcode % innerTable->tableLimit;

    unsigned steps = 0;

    // we know the size < limit, so it will fit eventually
    while (steps < MAX_PROBE_LENGTH) {
        if (innerTable->entries[index].key == NULL) {
            innerTable->entries[index].hashcode = hashcode;
            innerTable->entries[index].key = key;
            innerTable->entries[index].data = data;
            innerTable->tableSize++;
            return ADD_OK;
        }

        if ((innerTable->entries[index].hashcode == hashcode) && keyEqualsFunc(key, innerTable->entries[index].key)) {
            // the key already exists in the table
            return ADD_DUP;
        }

        steps++;
        index = index + 1;
        if (index == innerTable->tableLimit) {
            index = 0;
        }
    }

    return ADD_NOSPACE;
}

static PARCHashCodeTable_AddResult
_rehash(LinearAddressingHashTable *old_table, LinearAddressingHashTable *new_table, PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc)
{
    size_t i;
    for (i = 0; i < old_table->tableLimit; i++) {
        if (old_table->entries[i].key != NULL) {
            PARCHashCodeTable_AddResult result = _innerTableAdd(new_table, keyEqualsFunc, old_table->entries[i].hashcode,
                                                                old_table->entries[i].key, old_table->entries[i].data);
            if (result != ADD_OK) {
                return result;
            }
        }
    }
    return ADD_OK;
}

static void
_expand(PARCHashCodeTable *hashCodeTable)
{
    LinearAddressingHashTable temp_table;
    LinearAddressingHashTable *old_table = &hashCodeTable->hashtable;

    size_t expandby = EXPAND_FACTOR;

    // start with a copy of the current table
    PARCHashCodeTable_AddResult result = ADD_OK;
    do {
        hashCodeTable->expandCount++;

        temp_table.tableSize = 0;
        temp_table.tableLimit = old_table->tableLimit * expandby;
        temp_table.expandThreshold = temp_table.tableLimit - temp_table.tableLimit / 4;
        temp_table.entries = parcMemory_AllocateAndClear(temp_table.tableLimit * sizeof(HashTableEntry));
        parcAssertNotNull(temp_table.entries, "parcMemory_AllocateAndClear(%zu) returned NULL", temp_table.tableLimit * sizeof(HashTableEntry));

        result = _rehash(old_table, &temp_table, hashCodeTable->keyEqualsFunc);
        if (result == ADD_NOSPACE) {
            // could not rehash, so expand by more and try again
            parcMemory_Deallocate((void **) &(temp_table.entries));
            expandby++;
        }
    } while (result == ADD_NOSPACE);

    parcMemory_Deallocate((void **) &old_table->entries);
    hashCodeTable->hashtable = temp_table;
}

PARCHashCodeTable *
parcHashCodeTable_Create_Size(PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc,
                              PARCHashCodeTable_HashCodeFunc keyHashCodeFunc,
                              PARCHashCodeTable_Destroyer keyDestroyer,
                              PARCHashCodeTable_Destroyer dataDestroyer,
                              size_t minimumSize)
{
    PARCHashCodeTable *table = parcMemory_AllocateAndClear(sizeof(PARCHashCodeTable));
    parcAssertNotNull(table, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCHashCodeTable));

    parcAssertNotNull(keyEqualsFunc, "keyEqualsFunc must be non-null");
    parcAssertNotNull(keyHashCodeFunc, "keyHashCodeFunc must be non-null");
    parcAssertTrue(minimumSize > 0, "minimumSize must be greater than zero");

    table->keyEqualsFunc = keyEqualsFunc;
    table->keyHashCodeFunc = keyHashCodeFunc;
    table->keyDestroyer = keyDestroyer;
    table->dataDestroyer = dataDestroyer;

    table->hashtable.entries = parcMemory_AllocateAndClear(minimumSize * sizeof(HashTableEntry));
    parcAssertNotNull(table->hashtable.entries, "parcMemory_AllocateAndClear(%zu) returned NULL", minimumSize * sizeof(HashTableEntry));
    table->hashtable.tableLimit = minimumSize;
    table->hashtable.tableSize = 0;

    memset(table->hashtable.entries, 0, minimumSize * sizeof(HashTableEntry));

    // expand at 75% utilization
    table->hashtable.expandThreshold = minimumSize - minimumSize / 4;

    return table;
}

PARCHashCodeTable *
parcHashCodeTable_Create(PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc,
                         PARCHashCodeTable_HashCodeFunc keyHashCodeFunc,
                         PARCHashCodeTable_Destroyer keyDestroyer,
                         PARCHashCodeTable_Destroyer dataDestroyer)
{
    return parcHashCodeTable_Create_Size(keyEqualsFunc, keyHashCodeFunc, keyDestroyer, dataDestroyer, MIN_SIZE);
}

void
parcHashCodeTable_Destroy(PARCHashCodeTable **tablePtr)
{
    parcAssertNotNull(tablePtr, "Parameter must be non-null double pointer");
    parcAssertNotNull(*tablePtr, "Parameter must dereference to non-null pointer");
    PARCHashCodeTable *table = *tablePtr;
    size_t i;

    for (i = 0; i < table->hashtable.tableLimit; i++) {
        if (table->hashtable.entries[i].key != NULL) {
            if (table->keyDestroyer) {
                table->keyDestroyer(&table->hashtable.entries[i].key);
            }

            if (table->dataDestroyer) {
                table->dataDestroyer(&table->hashtable.entries[i].data);
            }
        }
    }

    parcMemory_Deallocate((void **) &(table->hashtable.entries));
    parcMemory_Deallocate((void **) &table);
    *tablePtr = NULL;
}

bool
parcHashCodeTable_Add(PARCHashCodeTable *table, void *key, void *data)
{
    parcAssertNotNull(table, "Parameter table must be non-null");
    parcAssertNotNull(key, "Parameter key must be non-null");
    parcAssertNotNull(data, "Parameter data must be non-null");

    if (table->hashtable.tableSize >= table->hashtable.expandThreshold) {
        _expand(table);
    }

    HashCodeType hashcode = table->keyHashCodeFunc(key);

    PARCHashCodeTable_AddResult result = ADD_OK;
    do {
        result = _innerTableAdd(&table->hashtable, table->keyEqualsFunc, hashcode, key, data);
        if (result == ADD_NOSPACE) {
            _expand(table);
        }
    } while (result == ADD_NOSPACE);

    return (result == ADD_OK);
}

void
parcHashCodeTable_Del(PARCHashCodeTable *table, const void *key)
{
    size_t index;
    bool found;

    parcAssertNotNull(table, "Parameter table must be non-null");
    parcAssertNotNull(key, "parameter key must be non-null");

    found = _findIndex(table, key, &index);

    if (found) {
        parcAssertTrue(table->hashtable.tableSize > 0, "Illegal state: found entry in a hash table with 0 size");

        if (table->keyDestroyer) {
            table->keyDestroyer(&table->hashtable.entries[index].key);
        }

        if (table->dataDestroyer) {
            table->dataDestroyer(&table->hashtable.entries[index].data);
        }

        memset(&table->hashtable.entries[index], 0, sizeof(HashTableEntry));

        table->hashtable.tableSize--;
    }
}

void *
parcHashCodeTable_Get(PARCHashCodeTable *table, const void *key)
{
    size_t index;

    parcAssertNotNull(table, "Parameter table must be non-null");
    parcAssertNotNull(key, "parameter key must be non-null");

    bool found = _findIndex(table, key, &index);

    if (found) {
        return table->hashtable.entries[index].data;
    }

    return NULL;
}

size_t
parcHashCodeTable_Length(const PARCHashCodeTable *table)
{
    parcAssertNotNull(table, "Parameter table must be non-null");
    return table->hashtable.tableSize;
}
