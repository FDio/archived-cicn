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
#include "../metis_PitEntry.c"
#include <LongBow/unit-test.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

#define __STDC_FORMAT_MACROS
#include <stdint.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

LONGBOW_TEST_RUNNER(metis_PitEntry)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_PitEntry)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_PitEntry)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// =====================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_AddEgressId);
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_AddIngressId);
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_Copy);
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_GetExpiryTime);
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_SetExpiryTime);
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_GetIngressSet);
    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_GetEgressSet);

    LONGBOW_RUN_TEST_CASE(Global, metisPitEntry_GetMessage);
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

LONGBOW_TEST_CASE(Global, metisPitEntry_AddEgressId)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);

    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), 10000, 0);

    metisPitEntry_AddEgressId(entry, 10);
    metisPitEntry_AddEgressId(entry, 11);

    size_t set_length = metisNumberSet_Length(entry->egressIdSet);
    bool contains_10 = metisNumberSet_Contains(entry->egressIdSet, 10);
    bool contains_11 = metisNumberSet_Contains(entry->egressIdSet, 11);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&interest);

    assertTrue(set_length == 2, "Wrong set length, expected %u got %zu", 2, set_length);
    assertTrue(contains_10, "Set did not contain 10");
    assertTrue(contains_11, "Set did not contain 11");
}

LONGBOW_TEST_CASE(Global, metisPitEntry_AddIngressId)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);

    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), 10000, 0);

    metisPitEntry_AddIngressId(entry, 10);
    metisPitEntry_AddIngressId(entry, 11);

    size_t set_length = metisNumberSet_Length(entry->ingressIdSet);

    // #1 is from the original interest
    bool contains_1 = metisNumberSet_Contains(entry->ingressIdSet, 1);
    bool contains_10 = metisNumberSet_Contains(entry->ingressIdSet, 10);
    bool contains_11 = metisNumberSet_Contains(entry->ingressIdSet, 11);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&interest);

    assertTrue(set_length == 3, "Wrong set length, expected %u got %zu", 2, set_length);
    assertTrue(contains_1, "Set did not contain 1");
    assertTrue(contains_10, "Set did not contain 10");
    assertTrue(contains_11, "Set did not contain 11");
}

LONGBOW_TEST_CASE(Global, metisPitEntry_Copy)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);

    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), 40000, 0);
    unsigned refCountBeforeCopy = entry->refcount;

    MetisPitEntry *copy = metisPitEntry_Acquire(entry);
    unsigned refCountAfterCopy = entry->refcount;

    metisPitEntry_Release(&entry);
    unsigned refCountAfterDestroy = copy->refcount;
    metisPitEntry_Release(&copy);
    metisMessage_Release(&interest);

    assertTrue(refCountAfterCopy == refCountBeforeCopy + 1, "Refcount after copy not 1 larger: expected %u got %u", refCountBeforeCopy + 1, refCountAfterCopy);
    assertTrue(refCountAfterDestroy == refCountBeforeCopy, "Refcount after destroy not same as before copy: expected %u got %u", refCountBeforeCopy, refCountAfterDestroy);
}

LONGBOW_TEST_CASE(Global, metisPitEntry_Create_Destroy)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);
    size_t baselineMemory = parcMemory_Outstanding();
    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), 40000, 0);
    metisPitEntry_Release(&entry);
    size_t testMemory = parcMemory_Outstanding();

    metisMessage_Release(&interest);

    assertTrue(testMemory == baselineMemory, "Memory imbalance on create/destroy: expected %zu got %zu", baselineMemory, testMemory);
}

LONGBOW_TEST_CASE(Global, metisPitEntry_GetExpiryTime)
{
    MetisTicks expiry = 40000;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);
    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), expiry, 0);

    MetisTicks test = metisPitEntry_GetExpiryTime(entry);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&interest);

    assertTrue(expiry == test, "Got wrong expiry time, expected %" PRIu64 ", got %" PRIu64, expiry, test);
}

LONGBOW_TEST_CASE(Global, metisPitEntry_SetExpiryTime)
{
    MetisTicks expiry = 40000;
    MetisTicks expiry2 = 80000;

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);
    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), expiry, 0);

    metisPitEntry_SetExpiryTime(entry, expiry2);

    MetisTicks test = metisPitEntry_GetExpiryTime(entry);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&interest);

    assertTrue(expiry2 == test, "Got wrong expiry time, expected %" PRIu64 ", got %" PRIu64, expiry2, test);
}

LONGBOW_TEST_CASE(Global, metisPitEntry_GetIngressSet)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);
    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), 10000, 0);

    unsigned truth_set[] = { 1, 2, 3, 4, 0 };

    MetisNumberSet *truth = metisNumberSet_Create();

    for (int i = 0; truth_set[i] != 0; i++) {
        metisNumberSet_Add(truth, truth_set[i]);
        metisPitEntry_AddIngressId(entry, truth_set[i]);
    }

    const MetisNumberSet *ingressSet = metisPitEntry_GetIngressSet(entry);
    bool equals = metisNumberSet_Equals(truth, ingressSet);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&interest);
    metisNumberSet_Release(&truth);

    assertTrue(equals, "Number set returned by metisPitEntry_GetIngressSet did not equal truth set");
}

LONGBOW_TEST_CASE(Global, metisPitEntry_GetEgressSet)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);
    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), 10000, 0);

    unsigned truth_set[] = { 1, 2, 3, 4, 0 };

    MetisNumberSet *truth = metisNumberSet_Create();

    for (int i = 0; truth_set[i] != 0; i++) {
        metisNumberSet_Add(truth, truth_set[i]);
        metisPitEntry_AddEgressId(entry, truth_set[i]);
    }

    const MetisNumberSet *egressSet = metisPitEntry_GetEgressSet(entry);
    bool equals = metisNumberSet_Equals(truth, egressSet);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&interest);
    metisNumberSet_Release(&truth);

    assertTrue(equals, "Number set returned by metisPitEntry_GetIngressSet did not equal truth set");
}

LONGBOW_TEST_CASE(Global, metisPitEntry_GetMessage)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);

    MetisPitEntry *entry = metisPitEntry_Create(metisMessage_Acquire(interest), 10000, 0);
    MetisMessage *copy = metisPitEntry_GetMessage(entry);

    assertTrue(copy == interest, "Returned message not equal, expected %p got %p", (void *) interest, (void *) entry);

    metisPitEntry_Release(&entry);
    metisMessage_Release(&copy);
    metisMessage_Release(&interest);
}

// =====================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_PitEntry);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
