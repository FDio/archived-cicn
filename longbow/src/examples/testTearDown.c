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

/**
 *
 */
#include <stdio.h>
#include <string.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

LONGBOW_TEST_RUNNER(testTearDown)
{
    LONGBOW_RUN_TEST_FIXTURE(Succeeded);
    LONGBOW_RUN_TEST_FIXTURE(Warned);
}

LONGBOW_TEST_RUNNER_SETUP(testTearDown)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(testTearDown)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Succeeded)
{
    LONGBOW_RUN_TEST_CASE(Succeeded, testTearDown);
}

LONGBOW_TEST_FIXTURE_SETUP(Succeeded)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Succeeded)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Succeeded, testTearDown)
{
    assertTrue(true, "");
}


LONGBOW_TEST_FIXTURE(Warned)
{
    LONGBOW_RUN_TEST_CASE(Warned, alwaysWarn);
}

LONGBOW_TEST_FIXTURE_SETUP(Warned)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Warned)
{
    return LONGBOW_STATUS_TEARDOWN_WARNED;
}

LONGBOW_TEST_CASE(Warned, alwaysWarn)
{
    assertTrue(true, "");
}


int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(testTearDown);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
