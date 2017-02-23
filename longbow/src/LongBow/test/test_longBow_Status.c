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

#include <LongBow/testing.h>

LONGBOW_TEST_RUNNER(longBow_Status)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(longBow_Status)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(longBow_Status)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, longBowStatus_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, longBowStatus_ToString)
{
    char *expected;
    char *actual;

    expected = "Succeeded";
    actual = longBowStatus_ToString(LONGBOW_STATUS_SUCCEEDED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Warning";
    actual = longBowStatus_ToString(LongBowStatus_WARNED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Tear Down Warning";
    actual = longBowStatus_ToString(LongBowStatus_TEARDOWN_WARNED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Skipped";
    actual = longBowStatus_ToString(LONGBOW_STATUS_SKIPPED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Unimplemented";
    actual = longBowStatus_ToString(LongBowStatus_UNIMPLEMENTED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Impotent";
    actual = longBowStatus_ToString(LongBowStatus_IMPOTENT);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Failed";
    actual = longBowStatus_ToString(LONGBOW_STATUS_FAILED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Stopped";
    actual = longBowStatus_ToString(LongBowStatus_STOPPED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Tear Down Failed";
    actual = longBowStatus_ToString(LONGBOW_STATUS_TEARDOWN_FAILED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Setup Failed";
    actual = longBowStatus_ToString(LONGBOW_STATUS_SETUP_FAILED);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    expected = "Memory Leak";
    actual = longBowStatus_ToString(LONGBOW_STATUS_MEMORYLEAK);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);
    free(actual);

    actual = longBowStatus_ToString(LongBowStatus_SIGNALLED + 1);
    assertNotNull(actual, "Expected longBowStatus_ToString to return non-null value");
    free(actual);
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
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(longBow_Status);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
