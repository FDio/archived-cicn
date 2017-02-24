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

#include "../cpi_NameRouteType.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(cpi_NameRouteType)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_NameRouteType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_NameRouteType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiNameRouteType_ToString);
    LONGBOW_RUN_TEST_CASE(Global, cpiNameRouteType_FromString);
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

LONGBOW_TEST_CASE(Global, cpiNameRouteType_ToString)
{
    int numEntries = sizeof(nameRouteTypeString) / sizeof(nameRouteTypeString[0]);

    for (int i = 0; i < numEntries; i++) {
        if (nameRouteTypeString[i].type != 0) {
            assertTrue(cpiNameRouteType_ToString(nameRouteTypeString[i].type) != NULL, "Expected non-NULL type name");
        }
    }
}

LONGBOW_TEST_CASE(Global, cpiNameRouteType_FromString)
{
    int numEntries = sizeof(nameRouteTypeString) / sizeof(nameRouteTypeString[0]);

    for (int i = 0; i < numEntries; i++) {
        if (nameRouteTypeString[i].str != NULL) {
            CPINameRouteType type = cpiNameRouteType_FromString(nameRouteTypeString[i].str);
            assertTrue(type == cpiNameRouteType_DEFAULT || type == cpiNameRouteType_EXACT_MATCH || type == cpiNameRouteType_LONGEST_MATCH, "unexpected route type");
        }
    }
    // Test a name that doesn't exist. (need to catch the trap) Case 1034
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_NameRouteType);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
