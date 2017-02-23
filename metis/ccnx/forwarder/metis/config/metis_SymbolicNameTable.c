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


#include <config.h>
#include <ctype.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_HashCodeTable.h>
#include <parc/algol/parc_Hash.h>

#include <ccnx/forwarder/metis/config/metis_SymbolicNameTable.h>

struct metis_symblic_name_table {
    PARCHashCodeTable *symbolicNameTable;
};

// ========================================================================================
// symbolic name table functions

static bool
_symbolicNameEquals(const void *keyA, const void *keyB)
{
    return (strcasecmp((const char *) keyA, (const char *) keyB) == 0);
}

static HashCodeType
_symbolicNameHash(const void *keyA)
{
    const char *str = (const char *) keyA;
    size_t length = strlen(str);
    return parcHash32_Data(str, length);
}

// ========================================================================================

MetisSymbolicNameTable *
metisSymbolicNameTable_Create(void)
{
    MetisSymbolicNameTable *table = parcMemory_Allocate(sizeof(MetisSymbolicNameTable));

    if (table) {
        // key = char *
        // value = uint32_t *
        table->symbolicNameTable = parcHashCodeTable_Create(_symbolicNameEquals, _symbolicNameHash, parcMemory_DeallocateImpl, parcMemory_DeallocateImpl);
    }

    return table;
}

void
metisSymbolicNameTable_Destroy(MetisSymbolicNameTable **tablePtr)
{
    MetisSymbolicNameTable *table = *tablePtr;
    parcHashCodeTable_Destroy(&table->symbolicNameTable);
    parcMemory_Deallocate((void **) &table);
    *tablePtr = NULL;
}

static char *
_createKey(const char *symbolicName)
{
    char *key = parcMemory_StringDuplicate(symbolicName, strlen(symbolicName));

    // convert key to upper case
    char *p = key;

    // keeps looping until the first null
    while ((*p = toupper(*p))) {
        p++;
    }
    return key;
}

bool
metisSymbolicNameTable_Exists(MetisSymbolicNameTable *table, const char *symbolicName)
{
    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(symbolicName, "Parameter symbolicName must be non-null");

    char *key = _createKey(symbolicName);
    bool found = (parcHashCodeTable_Get(table->symbolicNameTable, key) != NULL);
    parcMemory_Deallocate((void **) &key);
    return found;
}

void
metisSymbolicNameTable_Remove(MetisSymbolicNameTable *table, const char *symbolicName)
{
    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(symbolicName, "Parameter symbolicName must be non-null");
   
    char *key = _createKey(symbolicName); 
    parcHashCodeTable_Del(table->symbolicNameTable, key);
    parcMemory_Deallocate((void **) &key);
    
}

bool
metisSymbolicNameTable_Add(MetisSymbolicNameTable *table, const char *symbolicName, unsigned connid)
{
    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(symbolicName, "Parameter symbolicName must be non-null");
    assertTrue(connid < UINT32_MAX, "Parameter connid must be less than %u", UINT32_MAX);

    char *key = _createKey(symbolicName);

    uint32_t *value = parcMemory_Allocate(sizeof(uint32_t));
    *value = connid;

    bool success = parcHashCodeTable_Add(table->symbolicNameTable, key, value);
    if (!success) {
        parcMemory_Deallocate((void **) &key);
        parcMemory_Deallocate((void **) &value);
    }

    return success;
}

unsigned
metisSymbolicNameTable_Get(MetisSymbolicNameTable *table, const char *symbolicName)
{
    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(symbolicName, "Parameter symbolicName must be non-null");

    unsigned connid = UINT32_MAX;

    char *key = _createKey(symbolicName);

    uint32_t *value = parcHashCodeTable_Get(table->symbolicNameTable, key);
    if (value) {
        connid = *value;
    }

    parcMemory_Deallocate((void **) &key);
    return connid;
}

