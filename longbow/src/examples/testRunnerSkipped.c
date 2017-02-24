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
 */

#include <stdio.h>

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(Example1)
{
    LONGBOW_RUN_TEST_FIXTURE(FixtureA);
}

LONGBOW_TEST_RUNNER_SETUP(Example1)
{
    return LONGBOW_STATUS_SETUP_SKIPTESTS;
}

LONGBOW_TEST_RUNNER_TEARDOWN(Example1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(FixtureA)
{
    LONGBOW_RUN_TEST_CASE(FixtureA, alwaysSucceed);
}

LONGBOW_TEST_FIXTURE_SETUP(FixtureA)
{
    assertTrue(0, "This should have been skipped and never be called.");
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(FixtureA)
{
    assertTrue(0, "This should have been skipped and never be called.");
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(FixtureA, alwaysSucceed)
{
    assertTrue(0, "This should have been skipped and never be called.");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(Example1);
    int status = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);

    exit(status);
}
