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

#include <config.h>

#define PARCTIMING_ENABLE 1
#include "../parc_Timing.h"

#include <inttypes.h>
#include <stdio.h>
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_Timing)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Timing)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Timing)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcTiming_One);
    LONGBOW_RUN_TEST_CASE(Global, parcTiming_Two);
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

static void
_delay(void)
{
    int count = 0;
    for (int i = 0; i < 100000; i++) {
        count++;
    }
}

LONGBOW_TEST_CASE(Global, parcTiming_One)
{
    parcTiming_Init(foo);
    parcTiming_Start(foo);
    _delay();
    parcTiming_Stop(foo);

    uint64_t delta = parcTiming_Delta(foo);

    assertTrue(delta > 0, "Did not measure a delta, expected positive");
    parcTiming_Fini(foo);
}

/*
 * Test two clocks at the same time
 */
LONGBOW_TEST_CASE(Global, parcTiming_Two)
{
    parcTiming_Init(outer);
    parcTiming_Init(inner);

    parcTiming_Start(outer);
    _delay();

    parcTiming_Start(inner);
    _delay();
    parcTiming_Stop(inner);

    parcTiming_Stop(outer);

    uint64_t deltaOuter = parcTiming_Delta(outer);
    uint64_t deltaInner = parcTiming_Delta(inner);

    assertTrue(deltaOuter > deltaInner,
               "expected the outer timer to be greater than the inner timer: outer %" PRIu64 ", inner %" PRIu64,
               deltaOuter, deltaInner);

    printf("outer %" PRIu64 ", inner %" PRIu64, deltaOuter, deltaInner);

    parcTiming_Fini(outer);
    parcTiming_Fini(inner);
}

// ===============================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Timing);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
