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
 * These tests were written before MetisMatchRulesTable was broken out of the PIT.
 * So, many of the tests "cheat" by looking directly in a constiuent table in MetisMatchingRulesTable.
 * They should be re-written to use the MetisMatchingRulesTable API.
 */

// Include this so we can step the clock forward without waiting real time
#include "../../core/metis_Forwarder.c"

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_StandardPIT.c"

// so we can directly test the underlying tables
#include "../metis_MatchingRulesTable.c"

// test data set
#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <parc/logging/parc_LogReporterTextStdout.h>

MetisForwarder *metis;

LONGBOW_TEST_RUNNER(metis_PIT)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_PIT)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_PIT)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ===============================================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisPit_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisPit_ReceiveInterest_NewEntry);
    LONGBOW_RUN_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingExpired);
    LONGBOW_RUN_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingExpired_VerifyTable);
    LONGBOW_RUN_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingCurrentSameReversePath);
    LONGBOW_RUN_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingCurrentNewReversePath);
    LONGBOW_RUN_TEST_CASE(Global, metisPit_SatisfyInterest);
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_RemoveInterest);
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_AddEgressConnectionId);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    metis = metisForwarder_Create(NULL);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    metisForwarder_Destroy(&metis);
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisPit_Create_Destroy)
{
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    size_t baselineMemory = parcMemory_Outstanding();

    MetisPIT *pit = metisStandardPIT_Create(metis);
    metisPIT_Release(&pit);

    assertTrue(parcMemory_Outstanding() == baselineMemory, "Memory imbalance on create/destroy: %u", parcMemory_Outstanding());
}

/**
 * Receive an interest that is not already in the table
 */
LONGBOW_TEST_CASE(Global, metisPit_ReceiveInterest_NewEntry)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisPITVerdict verdict = metisPIT_ReceiveInterest(generic, interest);
    size_t table_length = parcHashCodeTable_Length(pit->table->tableByName);

    metisMessage_Release(&interest);

    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(table_length == 1, "tableByName wrong length, expected %u got %zu", 1, table_length);
    assertTrue(verdict == MetisPITVerdict_Forward, "New entry did not return PIT_VERDICT_NEW_ENTRY, got %d", verdict);
}

/**
 * Receive an interest that is in the table, but expired
 */
LONGBOW_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingExpired)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *interest_1 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    MetisMessage *interest_2 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 2, 2, logger);

    // stuff in the first interest
    _metisPIT_StoreInTable(pit, interest_1);

    // we need to crank the clock forward over 4 seconds, so add 5 seconds to the clock
    metis->clockOffset = metisForwarder_NanosToTicks(5000000000ULL);

    // now do the operation we're testing.  The previous entry should show as expired
    MetisPITVerdict verdict_2 = metisPIT_ReceiveInterest(generic, interest_2);

    size_t table_length = parcHashCodeTable_Length(pit->table->tableByName);

    metisMessage_Release(&interest_1);
    metisMessage_Release(&interest_2);

    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(table_length == 1, "tableByName wrong length, expected %u got %zu", 1, table_length);
    assertTrue(verdict_2 == MetisPITVerdict_Forward, "New entry did not return PIT_VERDICT_NEW_ENTRY, got %d", verdict_2);
}

/**
 * Receive an interest that is in the table, but expired.
 * In this test, retrieve the interest from the table and make sure its the 2nd one.
 */
LONGBOW_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingExpired_VerifyTable)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *interest_1 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    MetisMessage *interest_2 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 2, 2, logger);

    // stuff in the first interest
    _metisPIT_StoreInTable(pit, interest_1);

    // we need to crank the clock forward over 4 seconds, so add 5 seconds to the clock
    metis->clockOffset = metisForwarder_NanosToTicks(5000000000ULL);

    // now do the operation we're testing.  The previous entry should show as expired
    metisPIT_ReceiveInterest(generic, interest_2);

    MetisPitEntry *entry = parcHashCodeTable_Get(pit->table->tableByName, interest_2);
    const MetisNumberSet *ingressSet = metisPitEntry_GetIngressSet(entry);
    bool containsTwo = metisNumberSet_Contains(ingressSet, 2);

    metisMessage_Release(&interest_1);
    metisMessage_Release(&interest_2);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(containsTwo, "Got wrong ingressId, does not contain %u", 2);
}

/**
 * Receive an interest that is in the table, and not expired, and from an existing reverse path.
 * This should cause the interest to be forwarded.
 */
LONGBOW_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingCurrentSameReversePath)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *interest_1 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    MetisMessage *interest_2 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);

    // stuff in the first interest
    _metisPIT_StoreInTable(pit, interest_1);

    // now do the operation we're testing
    MetisPITVerdict verdict_2 = metisPIT_ReceiveInterest(generic, interest_2);
    size_t table_length = parcHashCodeTable_Length(pit->table->tableByName);

    metisMessage_Release(&interest_1);
    metisMessage_Release(&interest_2);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(table_length == 1, "tableByName wrong length, expected %u got %zu", 1, table_length);
    assertTrue(verdict_2 == MetisPITVerdict_Forward, "New entry did not return MetisPITVerdict_Forward, got %d", verdict_2);
}

/*
 * Receive an interest that exists in the PIT but from a new reverse path.  this should be
 * aggregated as an existing entry.
 */
LONGBOW_TEST_CASE(Global, metisPit_ReceiveInterest_ExistingCurrentNewReversePath)
{
    printf("THE TEST\n");
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *interest_1 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    MetisMessage *interest_2 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 2, 2, logger);

    // stuff in the first interest
    _metisPIT_StoreInTable(pit, interest_1);

    // now do the operation we're testing
    MetisPITVerdict verdict_2 = metisPIT_ReceiveInterest(generic, interest_2);
    size_t table_length = parcHashCodeTable_Length(pit->table->tableByName);

    metisMessage_Release(&interest_1);
    metisMessage_Release(&interest_2);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(table_length == 1, "tableByName wrong length, expected %u got %zu", 1, table_length);
    assertTrue(verdict_2 == MetisPITVerdict_Aggregate, "New entry did not return MetisPITVerdict_Aggregate, got %d", verdict_2);
}


LONGBOW_TEST_CASE(Global, metisPit_SatisfyInterest)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);

    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName_objecthash, sizeof(metisTestDataV0_InterestWithName_objecthash), 1, 1, logger);
    MetisMessage *contentObjectMessage = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 1, logger);

    // we manually stuff it in to the proper table, then call the public API, which will
    // figure out the right table then remove it.
    size_t before = parcHashCodeTable_Length(pit->table->tableByName);
    _metisPIT_StoreInTable(pit, interest);
    MetisNumberSet *ingressSetUnion = metisPIT_SatisfyInterest(generic, contentObjectMessage);
    metisPIT_RemoveInterest(generic, interest);
    assertTrue(metisNumberSet_Length(ingressSetUnion) == 1, "Unexpected satisfy interest return set size (%zu)",
               metisNumberSet_Length(ingressSetUnion));
    size_t after = parcHashCodeTable_Length(pit->table->tableByName);

    metisNumberSet_Release(&ingressSetUnion);
    metisMessage_Release(&interest);
    metisMessage_Release(&contentObjectMessage);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(after == before, "Did not remove interest in HashCodeTable: before %zu after %zu", before, after);
}

LONGBOW_TEST_CASE(Global, metisPIT_RemoveInterest)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);

    // we manually stuff it in to the proper table, then call the public API, which will
    // figure out the right table then remove it.
    size_t before = parcHashCodeTable_Length(pit->table->tableByName);
    _metisPIT_StoreInTable(pit, interest);
    metisPIT_RemoveInterest(generic, interest);
    size_t after = parcHashCodeTable_Length(pit->table->tableByName);

    metisMessage_Release(&interest);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(after == before, "Did not remove interest in HashCodeTable: before %zu after %zu", before, after);
}

LONGBOW_TEST_CASE(Global, metisPIT_AddEgressConnectionId)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);

    _metisPIT_StoreInTable(pit, interest);
    _metisPIT_AddEgressConnectionId(generic, interest, 6);

    MetisPitEntry *entry = metisPIT_GetPitEntry(generic, interest);
    const MetisNumberSet *egressSet = metisPitEntry_GetEgressSet(entry);

    size_t egress_length = metisNumberSet_Length(egressSet);
    bool contains_6 = metisNumberSet_Contains(egressSet, 6);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&interest);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(egress_length == 1, "Wrong egress_set length, expected %u got %zu", 1, egress_length);
    assertTrue(contains_6, "Wrong egress_set match, did not contain %u", 6);
}

// ===============================================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisPit_PitEntryDestroyer);
    LONGBOW_RUN_TEST_CASE(Local, _metisPIT_StoreInTable);
    LONGBOW_RUN_TEST_CASE(Local, metisPit_StoreInTable_IngressSetCheck);
    LONGBOW_RUN_TEST_CASE(Local, _metisPIT_CalculateLifetime_WithLifetime);
    LONGBOW_RUN_TEST_CASE(Local, _metisPIT_CalculateLifetime_DefaultLifetime);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    metis = metisForwarder_Create(NULL);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    metisForwarder_Destroy(&metis);
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, metisPit_PitEntryDestroyer)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, _metisPIT_StoreInTable)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);

    size_t before = parcHashCodeTable_Length(pit->table->tableByName);
    _metisPIT_StoreInTable(pit, interest);
    size_t after = parcHashCodeTable_Length(pit->table->tableByName);

    metisMessage_Release(&interest);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(after == before + 1, "Did not store interest in HashCodeTable: before %zu after %zu", before, after);
}

LONGBOW_TEST_CASE(Local, metisPit_StoreInTable_IngressSetCheck)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    unsigned connid = 99;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), connid, 1, logger);
    metisLogger_Release(&logger);

    _metisPIT_StoreInTable(pit, interest);
    MetisPitEntry *entry = parcHashCodeTable_Get(pit->table->tableByName, interest);
    const MetisNumberSet *ingressSet = metisPitEntry_GetIngressSet(entry);
    bool containsIngressId = metisNumberSet_Contains(ingressSet, connid);

    metisMessage_Release(&interest);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(containsIngressId, "PIT entry did not have the ingress id in its ingress set");
}

/*
 * Use an interest with a lifetime
 */
LONGBOW_TEST_CASE(Local, _metisPIT_CalculateLifetime_WithLifetime)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_SecondInterest, sizeof(metisTestDataV0_SecondInterest), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisTicks now = metisForwarder_GetTicks(metis);
    MetisTicks lifetime = _metisPIT_CalculateLifetime(pit, interest);

    metisMessage_Release(&interest);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    uint64_t value = 32000;
    assertTrue(lifetime >= value + now, "Wrong lifetime, should be at least %" PRIu64 ", got %" PRIu64, now + value, lifetime);
}

/*
 * Use an interest without a Lifetime, should return with the default 4s lifetime
 */
LONGBOW_TEST_CASE(Local, _metisPIT_CalculateLifetime_DefaultLifetime)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisPIT *generic = metisStandardPIT_Create(metis);
    MetisStandardPIT *pit = metisPIT_Closure(generic);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 1, logger);
    metisLogger_Release(&logger);

    MetisTicks now = metisForwarder_GetTicks(metis);
    MetisTicks lifetime = _metisPIT_CalculateLifetime(pit, interest);

    metisMessage_Release(&interest);
    metisPIT_Release(&generic);
    metisForwarder_Destroy(&metis);

    assertTrue(lifetime >= 4000 + now, "Wrong lifetime, should be at least %" PRIu64 ", got %" PRIu64, now + 4000, lifetime);
}



// ===============================================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_PIT);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
