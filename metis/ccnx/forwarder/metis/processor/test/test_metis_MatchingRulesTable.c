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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_MatchingRulesTable.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

LONGBOW_TEST_RUNNER(metis_MatchingRulesTable)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(HashFunctions);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_MatchingRulesTable)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_MatchingRulesTable)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_Create_Destroy);

    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_Add_ByName);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_Add_ByNameAndKeyId);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_Add_ByNameAndObjectHash);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_AddToAllTables);

    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_Get);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_RemoveFromBest);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_RemoveFromAll);

    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_NoMatch);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_1Table);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_2Tables);
    LONGBOW_RUN_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_3Tables);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_Add_ByName)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    void *data = (void *) 0x01;

    metisMatchingRulesTable_AddToBestTable(rulesTable, interest, data);
    size_t tableLength = parcHashCodeTable_Length(rulesTable->tableByName);

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);

    assertTrue(tableLength == 1, "tableByName wrong length, expected %u got %zu", 1, tableLength);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_Add_ByNameAndKeyId)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 1, 1, logger);
    metisLogger_Release(&logger);
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    void *data = (void *) 0x01;

    metisMatchingRulesTable_AddToBestTable(rulesTable, interest, data);
    size_t tableLength = parcHashCodeTable_Length(rulesTable->tableByNameAndKeyId);

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);

    assertTrue(tableLength == 1, "tableByNameAndKeyId wrong length, expected %u got %zu", 1, tableLength);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_Add_ByNameAndObjectHash)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    metisLogger_Release(&logger);
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    void *data = (void *) 0x01;

    metisMatchingRulesTable_AddToBestTable(rulesTable, interest, data);
    size_t tableLength = parcHashCodeTable_Length(rulesTable->tableByNameAndObjectHash);

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);

    assertTrue(tableLength == 1, "tableByNameAndObjectHash wrong length, expected %u got %zu", 1, tableLength);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_AddToAllTables)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    metisLogger_Release(&logger);
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    void *data = (void *) 0x01;

    metisMatchingRulesTable_AddToAllTables(rulesTable, interest, data);
    size_t tableLength = parcHashCodeTable_Length(rulesTable->tableByNameAndObjectHash);
    assertTrue(tableLength == 1, "tableToAllTables wrong length, expected %u got %zu", 1, tableLength);

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);

}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_Create_Destroy)
{
    size_t baselineMemory = parcMemory_Outstanding();

    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    metisMatchingRulesTable_Destroy(&rulesTable);

    assertTrue(parcMemory_Outstanding() == baselineMemory, "Memory imbalance on create/destroy: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_Get)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    metisLogger_Release(&logger);
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    void *data = (void *) 0x01;

    metisMatchingRulesTable_AddToBestTable(rulesTable, interest, data);
    void *test = metisMatchingRulesTable_Get(rulesTable, interest);

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);

    assertTrue(data == test, "metisMatchingRulesTable_Get returned wrong result, expected %p got %p", data, test);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_RemoveFromAll)
{
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);
    void *data = (void *) 0x01;

    size_t before = parcHashCodeTable_Length(rulesTable->tableByName);
    parcHashCodeTable_Add(rulesTable->tableByName, interest, data);
    metisMatchingRulesTable_RemoveFromAll(rulesTable, interest);
    size_t after = parcHashCodeTable_Length(rulesTable->tableByName);

    metisMessage_Release(&interest);
    metisMatchingRulesTable_Destroy(&rulesTable);

    assertTrue(after == before, "Did not remove interest in HashCodeTable: before %zu after %zu", before, after);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_RemoveFromBest)
{
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);
    void *data = (void *) 0x01;

    size_t before = parcHashCodeTable_Length(rulesTable->tableByName);
    parcHashCodeTable_Add(rulesTable->tableByName, interest, data);
    metisMatchingRulesTable_RemoveFromBest(rulesTable, interest);
    size_t after = parcHashCodeTable_Length(rulesTable->tableByName);

    metisMessage_Release(&interest);
    metisMatchingRulesTable_Destroy(&rulesTable);

    assertTrue(after == before, "Did not remove interest in HashCodeTable: before %zu after %zu", before, after);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_NoMatch)
{
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 1, logger);
    metisLogger_Release(&logger);

    PARCArrayList *list = metisMatchingRulesTable_GetUnion(rulesTable, object);
    assertTrue(parcArrayList_Size(list) == 0, "Incorrect result length, expected %u got %zu", 0, parcArrayList_Size(list));

    parcArrayList_Destroy(&list);
    metisMessage_Release(&object);
    metisMatchingRulesTable_Destroy(&rulesTable);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_1Table)
{
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interestByName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    void *data = (void *) 0x01;

    // add the interest to the table
    metisMatchingRulesTable_AddToBestTable(rulesTable, interestByName, data);

    // now retrieve it with a matching content object
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 4, logger);

    PARCArrayList *list = metisMatchingRulesTable_GetUnion(rulesTable, object);
    assertTrue(parcArrayList_Size(list) == 1, "Incorrect result length, expected %u got %zu", 1, parcArrayList_Size(list));

    metisLogger_Release(&logger);
    parcArrayList_Destroy(&list);
    metisMessage_Release(&object);
    metisMessage_Release(&interestByName);
    metisMatchingRulesTable_Destroy(&rulesTable);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_2Tables)
{
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interestByName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *interestByNameAndKeyId = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 1, 2, logger);
    void *data = (void *) 0x01;

    // add the interest to the tables
    metisMatchingRulesTable_AddToBestTable(rulesTable, interestByName, data);
    metisMatchingRulesTable_AddToBestTable(rulesTable, interestByNameAndKeyId, data);

    // now retrieve it with a matching content object
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 4, logger);

    PARCArrayList *list = metisMatchingRulesTable_GetUnion(rulesTable, object);
    assertTrue(parcArrayList_Size(list) == 2, "Incorrect result length, expected %u got %zu", 2, parcArrayList_Size(list));

    metisLogger_Release(&logger);
    parcArrayList_Destroy(&list);
    metisMessage_Release(&object);
    metisMessage_Release(&interestByName);
    metisMessage_Release(&interestByNameAndKeyId);
    metisMatchingRulesTable_Destroy(&rulesTable);
}

LONGBOW_TEST_CASE(Global, metisMatchingRulesTable_GetUnion_3Tables)
{
    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interestByName = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *interestByNameAndKeyId = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 1, 2, logger);
    MetisMessage *interestByNameAndObjectHash = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 2, logger);
    void *data = (void *) 0x01;

    // add the interest to the tables
    assertTrue(metisMatchingRulesTable_AddToBestTable(rulesTable, interestByName, data), "Cannot add interestByName");
    assertTrue(metisMatchingRulesTable_AddToBestTable(rulesTable, interestByNameAndKeyId, data), "Cannot add interestByNameAndKeyId");
    assertTrue(metisMatchingRulesTable_AddToBestTable(rulesTable, interestByNameAndObjectHash, data), "Cannot add interestByNameAndObjectHash");

    // now retrieve it with a matching content object
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 4, logger);

    PARCArrayList *list = metisMatchingRulesTable_GetUnion(rulesTable, object);
    assertTrue(parcArrayList_Size(list) == 3, "Incorrect result length, expected %u got %zu", 3, parcArrayList_Size(list));

    metisLogger_Release(&logger);
    parcArrayList_Destroy(&list);
    metisMessage_Release(&object);
    metisMessage_Release(&interestByName);
    metisMessage_Release(&interestByNameAndKeyId);
    metisMessage_Release(&interestByNameAndObjectHash);
    metisMatchingRulesTable_Destroy(&rulesTable);
}

// ============================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisMatchingRulesTable_GetTableForMessage_TableByNameAndObjectHash);
    LONGBOW_RUN_TEST_CASE(Local, metisMatchingRulesTable_GetTableForMessage_TableByNameAndKeyId);
    LONGBOW_RUN_TEST_CASE(Local, metisMatchingRulesTable_GetTableForMessage_TableByName);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * Use an interest with only a name, should select tableByName
 */
LONGBOW_TEST_CASE(Local, metisMatchingRulesTable_GetTableForMessage_TableByName)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCHashCodeTable *table = metisMatchingRulesTable_GetTableForMessage(rulesTable, interest);

    assertTrue(table == rulesTable->tableByName,
               "Chose wrong table, expected TableByName, got %s",
               (table == rulesTable->tableByNameAndKeyId) ? "tableByNameAndKeyId" :
               (table == rulesTable->tableByNameAndObjectHash) ? "tableByNameAndObjectHash" : "unknown");

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);
}

/**
 * Use an interest with a name and keyid, should select tableByNameAndKeyId
 */
LONGBOW_TEST_CASE(Local, metisMatchingRulesTable_GetTableForMessage_TableByNameAndKeyId)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCHashCodeTable *table = metisMatchingRulesTable_GetTableForMessage(rulesTable, interest);

    assertTrue(table == rulesTable->tableByNameAndKeyId,
               "Chose wrong table, expected TableByNameAndKeyId, got %s",
               (table == rulesTable->tableByName) ? "tableByName" :
               (table == rulesTable->tableByNameAndObjectHash) ? "tableByNameAndObjectHash" : "unknown");

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);
}

/**
 * Use an interest with a name and objecthash, should select tableByNameAndObjectHash
 */
LONGBOW_TEST_CASE(Local, metisMatchingRulesTable_GetTableForMessage_TableByNameAndObjectHash)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisMatchingRulesTable *rulesTable = metisMatchingRulesTable_Create(NULL);
    PARCHashCodeTable *table = metisMatchingRulesTable_GetTableForMessage(rulesTable, interest);

    assertTrue(table == rulesTable->tableByNameAndObjectHash,
               "Chose wrong table, expected TableByName, got %s",
               (table == rulesTable->tableByNameAndKeyId) ? "tableByNameAndKeyId" :
               (table == rulesTable->tableByName) ? "tableByName" : "unknown");

    metisMatchingRulesTable_Destroy(&rulesTable);
    metisMessage_Release(&interest);
}

// ============================================================================

LONGBOW_TEST_FIXTURE(HashFunctions)
{
    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameAndKeyIdEquals_IsEqual);
    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameAndKeyIdEquals_IsNotEqual);
    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameAndKeyIdHashCode);

    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameAndObjectHashEquals_IsEqual);
    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameAndObjectHashEquals_IsNotEqual);
    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameAndObjectHashHashCode);

    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameEquals_IsEqual);
    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameEquals_IsNotEqual);
    LONGBOW_RUN_TEST_CASE(HashFunctions, hashTableFunction_NameHashCode);
}

LONGBOW_TEST_FIXTURE_SETUP(HashFunctions)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(HashFunctions)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * Test an interest and content object that match on (Name, KeyId)
 */
LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameAndKeyIdEquals_IsEqual)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 1, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 1, logger);
    metisLogger_Release(&logger);

    bool success = metisHashTableFunction_MessageNameAndKeyIdEquals(a, b);

    metisMessage_Release(&a);
    metisMessage_Release(&b);

    assertTrue(success, "Two equal names and keyids did not compare equal");
}

/**
 * Test two interests that do not match on (Name, KeyId)
 */
LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameAndKeyIdEquals_IsNotEqual)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 1, 1, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid2, sizeof(metisTestDataV0_InterestWithName_keyid2), 1, 1, logger);
    metisLogger_Release(&logger);

    bool success = metisHashTableFunction_MessageNameAndKeyIdEquals(a, b);

    metisMessage_Release(&a);
    metisMessage_Release(&b);

    assertFalse(success, "Two unequal names compared equal");
}

LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameAndKeyIdHashCode)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisTlvName *name = metisMessage_GetName(interest);
    uint32_t name_hash = metisTlvName_HashCode(name);
    uint32_t keyid_hash;
    metisMessage_GetKeyIdHash(interest, &keyid_hash);

    HashCodeType truth_hash = parcHash32_Data_Cumulative(&keyid_hash, sizeof(keyid_hash), name_hash);

    // the function to test
    HashCodeType test_hash = metisHashTableFunction_MessageNameAndKeyIdHashCode(interest);

    metisMessage_Release(&interest);

    assertTrue(test_hash == truth_hash, "Got wrong hash, expected %08"PRIX64 " got %08"PRIX64, truth_hash, test_hash);
}

LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameAndObjectHashEquals_IsEqual)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 1, logger);
    metisLogger_Release(&logger);

    bool success = metisHashTableFunction_MessageNameAndObjectHashEquals(a, b);

    metisMessage_Release(&a);
    metisMessage_Release(&b);

    assertTrue(success, "Two equal names and hashes did not compare equal");
}

LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameAndObjectHashEquals_IsNotEqual)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 1, logger);
    metisLogger_Release(&logger);

    bool success = metisHashTableFunction_MessageNameAndObjectHashEquals(a, b);

    metisMessage_Release(&a);
    metisMessage_Release(&b);

    assertFalse(success, "Two unequal names and hashes compared equal");
}

LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameAndObjectHashHashCode)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisTlvName *name = metisMessage_GetName(interest);
    uint32_t name_hash = metisTlvName_HashCode(name);
    uint32_t object_hash;
    metisMessage_GetContentObjectHashHash(interest, &object_hash);

    HashCodeType truth_hash = parcHash32_Data_Cumulative(&object_hash, sizeof(object_hash), name_hash);

    // the function we actually want to test
    HashCodeType test_hash = (HashCodeType)metisHashTableFunction_MessageNameAndObjectHashHashCode(interest);

    metisMessage_Release(&interest);

    assertTrue(test_hash == truth_hash, "Got wrong hash, expected %08"PRIX64 " got %08"PRIX64 , truth_hash, test_hash);
}

/**
 * Takes two MetisMessage and compares their names for equality
 */
LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameEquals_IsEqual)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_keyid, sizeof(metisTestDataV0_InterestWithName_keyid), 1, 1, logger);
    metisLogger_Release(&logger);

    bool success = metisHashTableFunction_MessageNameEquals(a, b);

    metisMessage_Release(&a);
    metisMessage_Release(&b);

    assertTrue(success, "Two equal names did not compare equal");
}

/**
 * test two interests with different names
 */
LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameEquals_IsNotEqual)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_InterestWithOtherName, sizeof(metisTestDataV0_InterestWithOtherName), 1, 1, logger);
    metisLogger_Release(&logger);

    bool success = metisHashTableFunction_MessageNameEquals(a, b);

    metisMessage_Release(&a);
    metisMessage_Release(&b);

    assertFalse(success, "Two unequal names compared equal");
}

/**
 * Used on a MetisMessage key type, should return the HashCode
 * of the message's name
 */
LONGBOW_TEST_CASE(HashFunctions, hashTableFunction_NameHashCode)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisTlvName *name = metisMessage_GetName(interest);
    HashCodeType truth_hash = (HashCodeType)metisTlvName_HashCode(name);
    HashCodeType test_hash = metisHashTableFunction_MessageNameHashCode(interest);

    metisMessage_Release(&interest);

    assertTrue(test_hash == truth_hash, "Got wrong hash, expected %08"PRIX64 " got %08"PRIX64, truth_hash, test_hash);
}

// ============================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_MatchingRulesTable);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
