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
#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Hash.h>

#include <ccnx/forwarder/metis/processor/metis_HashTableFunction.h>
#include <ccnx/forwarder/metis/processor/metis_MatchingRulesTable.h>
#include <LongBow/runtime.h>

struct metis_matching_rules_table {
    // we maintain three hash tables indexed by the different ways
    // one could ask for something.  THis means a content object needs
    // to do three lookups.  We can optimize this later.

    PARCHashCodeTable *tableByName;
    PARCHashCodeTable *tableByNameAndKeyId;
    PARCHashCodeTable *tableByNameAndObjectHash;

    PARCHashCodeTable_Destroyer dataDestroyer;
};

static PARCHashCodeTable *metisMatchingRulesTable_GetTableForMessage(const MetisMatchingRulesTable *pit, const MetisMessage *interestMessage);

// ======================================================================

MetisMatchingRulesTable *
metisMatchingRulesTable_Create(PARCHashCodeTable_Destroyer dataDestroyer)
{
    size_t initialSize = 65535;

    MetisMatchingRulesTable *table = parcMemory_AllocateAndClear(sizeof(MetisMatchingRulesTable));
    assertNotNull(table, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMatchingRulesTable));
    table->dataDestroyer = dataDestroyer;

    // There is not a Key destroyer because we use the message from the MetisPitEntry as the key

    table->tableByName = parcHashCodeTable_Create_Size(metisHashTableFunction_MessageNameEquals,
                                                       metisHashTableFunction_MessageNameHashCode,
                                                       NULL,
                                                       dataDestroyer,
                                                       initialSize);

    table->tableByNameAndKeyId = parcHashCodeTable_Create_Size(metisHashTableFunction_MessageNameAndKeyIdEquals,
                                                               metisHashTableFunction_MessageNameAndKeyIdHashCode,
                                                               NULL,
                                                               dataDestroyer,
                                                               initialSize);

    table->tableByNameAndObjectHash = parcHashCodeTable_Create_Size(metisHashTableFunction_MessageNameAndObjectHashEquals,
                                                                    metisHashTableFunction_MessageNameAndObjectHashHashCode,
                                                                    NULL,
                                                                    dataDestroyer,
                                                                    initialSize);
    return table;
}

void
metisMatchingRulesTable_Destroy(MetisMatchingRulesTable **tablePtr)
{
    assertNotNull(tablePtr, "Parameter must be non-null double pointer");
    assertNotNull(*tablePtr, "Parameter must dereference to non-null pointer");

    MetisMatchingRulesTable *table = *tablePtr;

    parcHashCodeTable_Destroy(&table->tableByNameAndObjectHash);
    parcHashCodeTable_Destroy(&table->tableByNameAndKeyId);
    parcHashCodeTable_Destroy(&table->tableByName);

    parcMemory_Deallocate((void **) &table);
    *tablePtr = NULL;
}

void *
metisMatchingRulesTable_Get(const MetisMatchingRulesTable *rulesTable, const MetisMessage *message)
{
    assertNotNull(rulesTable, "Parameter rulesTable must be non-null");
    assertNotNull(message, "Parameter message must be non-null");

    PARCHashCodeTable *hashTable = metisMatchingRulesTable_GetTableForMessage(rulesTable, message);
    return parcHashCodeTable_Get(hashTable, message);
}

PARCArrayList *
metisMatchingRulesTable_GetUnion(const MetisMatchingRulesTable *table, const MetisMessage *message)
{
    // we can have at most 3 results, so create with that capacity
    PARCArrayList *list = parcArrayList_Create_Capacity(NULL, NULL, 3);

    void *dataByName = parcHashCodeTable_Get(table->tableByName, message);
    if (dataByName) {
        parcArrayList_Add(list, dataByName);
    }

    if (metisMessage_HasKeyId(message)) {
        void *dataByNameAndKeyId = parcHashCodeTable_Get(table->tableByNameAndKeyId, message);
        if (dataByNameAndKeyId) {
            parcArrayList_Add(list, dataByNameAndKeyId);
        }
    }

    if (metisMessage_HasContentObjectHash(message)) {
        void *dataByNameAndObjectHash = parcHashCodeTable_Get(table->tableByNameAndObjectHash, message);
        if (dataByNameAndObjectHash) {
            parcArrayList_Add(list, dataByNameAndObjectHash);
        }
    }

    return list;
}

void
metisMatchingRulesTable_RemoveFromBest(MetisMatchingRulesTable *rulesTable, const MetisMessage *message)
{
    assertNotNull(rulesTable, "Parameter rulesTable must be non-null");
    assertNotNull(message, "Parameter message must be non-null");

    PARCHashCodeTable *hashTable = metisMatchingRulesTable_GetTableForMessage(rulesTable, message);
    parcHashCodeTable_Del(hashTable, message);
}

void
metisMatchingRulesTable_RemoveFromAll(MetisMatchingRulesTable *rulesTable, const MetisMessage *message)
{
    assertNotNull(rulesTable, "Parameter rulesTable must be non-null");
    assertNotNull(message, "Parameter message must be non-null");

    parcHashCodeTable_Del(rulesTable->tableByName, message);

    // not all messages have a keyid any more
    if (metisMessage_HasKeyId(message)) {
        parcHashCodeTable_Del(rulesTable->tableByNameAndKeyId, message);
    }

    if (metisMessage_HasContentObjectHash(message)) {
        parcHashCodeTable_Del(rulesTable->tableByNameAndObjectHash, message);
    }
}

bool
metisMatchingRulesTable_AddToBestTable(MetisMatchingRulesTable *rulesTable, MetisMessage *key, void *data)
{
    assertNotNull(rulesTable, "Parameter rulesTable must be non-null");
    assertNotNull(key, "Parameter key must be non-null");
    assertNotNull(data, "Parameter data must be non-null");

    PARCHashCodeTable *hashTable = metisMatchingRulesTable_GetTableForMessage(rulesTable, key);

    bool success = parcHashCodeTable_Add(hashTable, key, data);

    return success;
}

void
metisMatchingRulesTable_AddToAllTables(MetisMatchingRulesTable *rulesTable, MetisMessage *key, void *data)
{
    assertNotNull(rulesTable, "Parameter rulesTable must be non-null");
    assertNotNull(key, "Parameter key must be non-null");
    assertNotNull(data, "Parameter data must be non-null");

    parcHashCodeTable_Add(rulesTable->tableByName, key, data);

    // not all messages have a keyid any more
    if (metisMessage_HasKeyId(key)) {
        parcHashCodeTable_Add(rulesTable->tableByNameAndKeyId, key, data);
    }

    parcHashCodeTable_Add(rulesTable->tableByNameAndObjectHash, key, data);
}

// ========================================================================================

static PARCHashCodeTable *
metisMatchingRulesTable_GetTableForMessage(const MetisMatchingRulesTable *pit, const MetisMessage *interestMessage)
{
    PARCHashCodeTable *table;
    if (metisMessage_HasContentObjectHash(interestMessage)) {
        table = pit->tableByNameAndObjectHash;
    } else if (metisMessage_HasKeyId(interestMessage)) {
        table = pit->tableByNameAndKeyId;
    } else {
        table = pit->tableByName;
    }

    return table;
}
