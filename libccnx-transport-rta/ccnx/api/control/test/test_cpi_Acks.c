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

#include "../cpi_Acks.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>


LONGBOW_TEST_RUNNER(cpi_Acks)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_Acks)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_Acks)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiAck_CreateAck);
    LONGBOW_RUN_TEST_CASE(Global, cpiAck_CreateNack);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, cpiAck_CreateAck)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(name);
    PARCJSON *request = cpiForwarding_CreateAddRouteRequest(route);

    PARCJSON *actual = cpiAcks_CreateAck(request);

    assertTrue(cpiAcks_IsAck(actual), "Expected cpiAcks_IsAck to return true.");

    parcJSON_Release(&actual);
    parcJSON_Release(&request);
    cpiRouteEntry_Destroy(&route);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, cpiAck_CreateNack)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
    CPIRouteEntry *route = cpiRouteEntry_CreateRouteToSelf(name);
    PARCJSON *request = cpiForwarding_CreateAddRouteRequest(route);

    PARCJSON *actual = cpiAcks_CreateNack(request);

    assertFalse(cpiAcks_IsAck(actual), "Expected cpiAcks_IsAck to return false.");

    parcJSON_Release(&actual);
    parcJSON_Release(&request);
    cpiRouteEntry_Destroy(&route);
    ccnxName_Release(&name);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_Acks);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
