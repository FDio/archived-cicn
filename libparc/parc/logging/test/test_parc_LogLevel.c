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
// This permits internal static functions to be visible to this Test Runner.
#include "../parc_LogLevel.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_LogLevel)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_LogLevel)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_LogLevel)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcLogLevel_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcLogLevel_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcLogLevel_ToString);
    LONGBOW_RUN_TEST_CASE(Global, parcLogLevel_ToString_All);
    LONGBOW_RUN_TEST_CASE(Global, parcLogLevel_ToString_Off);
    LONGBOW_RUN_TEST_CASE(Global, parcLogLevel_FromString_Debug);
    LONGBOW_RUN_TEST_CASE(Global, parcLogLevel_FromString_All);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcLogLevel_Compare)
{
    assertTrue(parcLogLevel_Compare(PARCLogLevel_Off, PARCLogLevel_All) < 0, "Expected PARCLogLevel_Off to be less that All");
}

LONGBOW_TEST_CASE(Global, parcLogLevel_Equals)
{
    assertTrue(parcLogLevel_Equals(PARCLogLevel_Emergency, PARCLogLevel_Emergency), "Expected equality");
    assertFalse(parcLogLevel_Equals(PARCLogLevel_Emergency, PARCLogLevel_Debug), "Expected inequality");
}

LONGBOW_TEST_CASE(Global, parcLogLevel_ToString)
{
    char *expected = "Debug";
    const char *actual = parcLogLevel_ToString(PARCLogLevel_Debug);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcLogLevel_ToString_Off)
{
    char *expected = "Off";
    const char *actual = parcLogLevel_ToString(PARCLogLevel_Off);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcLogLevel_ToString_All)
{
    char *expected = "All";
    const char *actual = parcLogLevel_ToString(PARCLogLevel_All);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcLogLevel_FromString_Debug)
{
    PARCLogLevel expected = PARCLogLevel_Debug;
    PARCLogLevel actual = parcLogLevel_FromString("DEBUG");

    assertTrue(expected == actual, "Expected '%d', actual '%d'", expected, actual);
}

LONGBOW_TEST_CASE(Global, parcLogLevel_FromString_All)
{
    PARCLogLevel expected = PARCLogLevel_All;
    PARCLogLevel actual = parcLogLevel_FromString("AlL");

    assertTrue(expected == actual, "Expected '%d', actual '%d'", expected, actual);
}


LONGBOW_TEST_FIXTURE(Static)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_LogLevel);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
