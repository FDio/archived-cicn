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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_PIT.c"



#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <parc/logging/parc_LogReporterTextStdout.h>

// ===============================================================================================
// Mock PIT
// These functions just count calls.  The Destroy interface does not actually release memeory, you
// need to call _metisPIT_Release() yourself -- note that this is a static function with leading "_".

typedef struct mock_pit {
    unsigned countRelease;
    unsigned countReceiveInterest;
    unsigned countSatisfyInterest;
    unsigned countRemoveInterest;
    unsigned countGetPitEntry;
} _MockPIT;

static void
_mockPITInterface_Release(MetisPIT **pitPtr)
{
    _MockPIT *mock = metisPIT_Closure(*pitPtr);
    mock->countRelease++;
    *pitPtr = NULL;
}

static MetisPITVerdict
_mockPITInterface_ReceiveInterest(MetisPIT *pit, MetisMessage *interestMessage)
{
    _MockPIT *mock = metisPIT_Closure(pit);
    mock->countReceiveInterest++;
    return MetisPITVerdict_Aggregate;
}

static MetisNumberSet *
_mockPITInterface_SatisfyInterest(MetisPIT *pit, const MetisMessage *objectMessage)
{
    _MockPIT *mock = metisPIT_Closure(pit);
    mock->countSatisfyInterest++;
    return NULL;
}

static void
_mockPITInterface_RemoveInterest(MetisPIT *pit, const MetisMessage *interestMessage)
{
    _MockPIT *mock = metisPIT_Closure(pit);
    mock->countRemoveInterest++;
}

static MetisPitEntry *
_mockPITInterface_GetPitEntry(const MetisPIT *pit, const MetisMessage *interestMessage)
{
    _MockPIT *mock = metisPIT_Closure(pit);
    mock->countGetPitEntry++;
    return NULL;
}

static MetisPIT *
_mockPIT_Create(void)
{
    size_t allocation = sizeof(MetisPIT) + sizeof(_MockPIT);
    MetisPIT *pit = parcMemory_AllocateAndClear(allocation);

    pit->getPitEntry = _mockPITInterface_GetPitEntry;
    pit->receiveInterest = _mockPITInterface_ReceiveInterest;
    pit->release = _mockPITInterface_Release;
    pit->removeInterest = _mockPITInterface_RemoveInterest;
    pit->satisfyInterest = _mockPITInterface_SatisfyInterest;

    pit->closure = (uint8_t *) pit + sizeof(MetisPIT);
    return pit;
}

static void
_metisPIT_Release(MetisPIT **pitPtr)
{
    parcMemory_Deallocate(pitPtr);
}


// ===============================================================================================

LONGBOW_TEST_RUNNER(metis_PIT)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
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
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_Closure);
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_Release);
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_ReceiveInterest);
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_SatisfyInterest);
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_RemoveInterest);
    LONGBOW_RUN_TEST_CASE(Global, metisPIT_GetPitEntry);
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

LONGBOW_TEST_CASE(Global, metisPIT_Closure)
{
    MetisPIT *pit = _mockPIT_Create();
    _MockPIT *mock = metisPIT_Closure(pit);
    assertTrue(mock == pit->closure, "Wrong pointer expected %p got %p", pit->closure, mock);
    _metisPIT_Release(&pit);
}

LONGBOW_TEST_CASE(Global, metisPIT_Release)
{
    MetisPIT *pit = _mockPIT_Create();
    MetisPIT *original = pit;
    _MockPIT *mock = metisPIT_Closure(pit);
    metisPIT_Release(&pit);

    assertTrue(mock->countRelease == 1, "Wrong count expected 1 got %u", mock->countRelease);
    _metisPIT_Release(&original);
}

LONGBOW_TEST_CASE(Global, metisPIT_ReceiveInterest)
{
    MetisPIT *pit = _mockPIT_Create();
    _MockPIT *mock = metisPIT_Closure(pit);
    metisPIT_ReceiveInterest(pit, NULL);

    assertTrue(mock->countReceiveInterest == 1, "Wrong count expected 1 got %u", mock->countReceiveInterest);
    _metisPIT_Release(&pit);
}

LONGBOW_TEST_CASE(Global, metisPIT_SatisfyInterest)
{
    MetisPIT *pit = _mockPIT_Create();
    _MockPIT *mock = metisPIT_Closure(pit);
    metisPIT_SatisfyInterest(pit, NULL);

    assertTrue(mock->countSatisfyInterest == 1, "Wrong count expected 1 got %u", mock->countSatisfyInterest);
    _metisPIT_Release(&pit);
}

LONGBOW_TEST_CASE(Global, metisPIT_RemoveInterest)
{
    MetisPIT *pit = _mockPIT_Create();
    _MockPIT *mock = metisPIT_Closure(pit);
    metisPIT_RemoveInterest(pit, NULL);

    assertTrue(mock->countRemoveInterest == 1, "Wrong count expected 1 got %u", mock->countRemoveInterest);
    _metisPIT_Release(&pit);
}

LONGBOW_TEST_CASE(Global, metisPIT_GetPitEntry)
{
    MetisPIT *pit = _mockPIT_Create();
    _MockPIT *mock = metisPIT_Closure(pit);
    metisPIT_GetPitEntry(pit, NULL);

    assertTrue(mock->countGetPitEntry == 1, "Wrong count expected 1 got %u", mock->countGetPitEntry);
    _metisPIT_Release(&pit);
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
