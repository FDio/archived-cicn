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
 * test_LongBow.c
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/errno.h>

#include "../unit-test.h"


LONGBOW_TEST_RUNNER(LongBow)
{
    LONGBOW_RUN_TEST_FIXTURE(LongBowFixture);
}

LONGBOW_TEST_RUNNER_SETUP(LongBow)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(LongBow)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(LongBowFixture)
{
    LONGBOW_RUN_TEST_CASE(LongBowFixture, alwaysSucceed);
    LONGBOW_RUN_TEST_CASE(LongBowFixture, alwaysUnimplemented);
    LONGBOW_RUN_TEST_CASE(LongBowFixture, alwaysImpotent);
}

LONGBOW_TEST_FIXTURE_SETUP(LongBowFixture)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(LongBowFixture)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(LongBowFixture, testErrno)
{
    errno = ENOENT;
    assertTrue(0, "Errno test");
}

LONGBOW_TEST_CASE(LongBowFixture, alwaysSucceed)
{
    assertTrue(1, "alwaysSucceed");
}

LONGBOW_TEST_CASE(LongBowFixture, alwaysImpotent)
{
}

LONGBOW_TEST_CASE_EXPECTS(LongBowFixture, testEvent, .event = &LongBowAssertEvent)
{
    assertTrue(0, "testEvent");
}

LONGBOW_TEST_CASE_EXPECTS(LongBowFixture, alwaysFail, .status = LONGBOW_STATUS_FAILED, .event = &LongBowAssertEvent)
{
    assertTrue(0, "alwaysFail");
}

LONGBOW_TEST_CASE_EXPECTS(LongBowFixture, alwaysSigTERM, .status = LONGBOW_STATUS_SIGNAL(SIGTERM))
{
    kill(getpid(), SIGTERM);
}

LONGBOW_TEST_CASE_EXPECTS(LongBowFixture, alwaysSEGV, .event = &LongBowEventSIGSEGV)
{
    int *p = 0;
    int i = *p;
    printf("not used %d\n", i);
}

LONGBOW_TEST_CASE(LongBowFixture, alwaysUnimplemented)
{
    testUnimplemented("alwaysUnimplemented");
}

LONGBOW_TEST_CASE(LongBowFixture, alwaysWarn)
{
    testWarn("alwaysWarn");
}

LONGBOW_TEST_CASE_EXPECTS(LongBowFixture, alwaysSkip, .event = &LongBowTestSkippedEvent)
{
    testSkip("alwaysSkip");
}

LONGBOW_TEST_CASE_EXPECTS(LongBowFixture, alwaysTrap, .status = LONGBOW_STATUS_FAILED)
{
    trapNotImplemented("alwaysTrap");
}

LONGBOW_TEST_FIXTURE(TestTearDownWarning)
{
    LONGBOW_RUN_TEST_CASE(TestTearDownWarning, alwaysFail);
    LONGBOW_RUN_TEST_CASE(TestTearDownWarning, alwaysSucceed);
}

LONGBOW_TEST_FIXTURE_SETUP(TestTearDownWarning)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(TestTearDownWarning)
{
    return LONGBOW_STATUS_TEARDOWN_WARNED;
}

LONGBOW_TEST_CASE(TestTearDownWarning, alwaysFail)
{
    assertTrue(0, "alwaysFail");
}

LONGBOW_TEST_CASE(TestTearDownWarning, alwaysSucceed)
{
    assertTrue(1, "alwaysSucceed");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(LongBow);
    int status = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);

    exit(status);
}
