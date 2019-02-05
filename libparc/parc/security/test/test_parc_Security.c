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
#include "../parc_Security.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(parc_Security)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Security)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Security)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSecurity_Fini);
    LONGBOW_RUN_TEST_CASE(Global, parcSecurity_Init);
    LONGBOW_RUN_TEST_CASE(Global, parcSecurity_Init_Multiple);
    LONGBOW_RUN_TEST_CASE(Global, parcSecurity_IsInitialized);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcSecurity_Fini)
{
    parcSecurity_Init();
    parcSecurity_AssertIsInitialized();
    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(Global, parcSecurity_Init)
{
    parcSecurity_Init();
    parcSecurity_AssertIsInitialized();
    parcSecurity_Fini();
}

LONGBOW_TEST_CASE(Global, parcSecurity_Init_Multiple)
{
    parcSecurity_Init();
    parcSecurity_Init();
    parcSecurity_Init();
    parcSecurity_Init();
    parcSecurity_AssertIsInitialized();
    parcSecurity_Fini();
    parcSecurity_Fini();
    parcSecurity_Fini();
    parcSecurity_Fini();
    assertFalse(parcSecurity_IsInitialized(), "parcSecurity_IsInitialized should be false now");
}

LONGBOW_TEST_CASE(Global, parcSecurity_IsInitialized)
{
    parcSecurity_Init();
    parcSecurity_AssertIsInitialized();
    parcSecurity_Fini();
    assertFalse(parcSecurity_IsInitialized(), "parcSecurity_IsInitialized should be false now");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Security);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
