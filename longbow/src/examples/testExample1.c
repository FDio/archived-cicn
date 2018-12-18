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

#include <stdio.h>
#include "../unit-test.h"

#include "example1.c"

LONGBOW_TEST_RUNNER(example1)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

LONGBOW_TEST_RUNNER_SETUP(example1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(example1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, strlen);
    LONGBOW_RUN_TEST_CASE(Global, alwaysFail);
    LONGBOW_RUN_TEST_CASE(Global, alwaysSignalled);
    LONGBOW_RUN_TEST_CASE(Global, alwaysSucceed);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, alwaysFail)
{
    assertTrue(alwaysFalse(), "This test must always fail.")
    {
        printf("And this is extra code that is executed when the assertion fails");
    }
}

LONGBOW_TEST_CASE(Global, alwaysSignalled)
{
    kill(getpid(), SIGTERM);
}

LONGBOW_TEST_CASE(Global, alwaysSucceed)
{
    assertTrue(alwaysTrue(), "This test must always succeed.");
}

LONGBOW_TEST_CASE(Global, strlen)
{
    assertNotNull(NULL, "Parameter must be a non-null char pointer.");
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(example1);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
