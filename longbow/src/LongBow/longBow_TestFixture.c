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
#include <stdio.h>

#include <LongBow/private/longBow_ArrayList.h>
#include <LongBow/private/longBow_Memory.h>
#include <LongBow/private/longBow_String.h>

#include <LongBow/longBow_TestFixture.h>
#include <LongBow/longBow_TestFixtureConfig.h>
#include <LongBow/longBow_TestCase.h>
#include <LongBow/longBow_RuntimeResult.h>

#include <LongBow/unit-test.h>

/**
 * @struct longbow_fixture
 * @brief The LongBow Test Fixture.
 *
 * This structure is created and initialized by the LongBow Test Runner.
 * When the Test Fixture represented by this structure is executed,
 * this structure is updated with the results of each LongBow Test Case that belongs to this Fixture.
 */
struct longbow_fixture {
    /**
     * The name of this test fixture.
     */
    const char *name;

    /**
     * The fully qualified name of this test fixture.
     */
    char *fullName;

    /**
     *
     */
    LongBowTestFixtureConfig *config;

    /**
     * The Test Runner of this test fixture.
     */
    LongBowTestRunner *runner;

    /**
     * The `LongBowTestFixtureSummary` of this test fixture.
     */
    LongBowTestFixtureSummary summary;

    /**
     * A list of `LongBowTestCase` structures, one for each executed test case.
     */
    LongBowArrayList *testCases;

    /**
     * The function to call to setup the Test Fixture.
     *
     * This function is called before each Test Case is run.
     * @param testCase The LongBow Test Case that is being set-up.
     */
    LongBowTestFixtureSetupFunction *setUp;

    /**
     * The function to call to execute the Test Fixture.
     *
     * @param testFixture A pointer to this `struct longbow_fixture` structure.
     */
    LongBowTestFixtureFunction *fixture;

    /**
     * The function to call to tear-down the Test Fixture.
     *
     * This function is called after each Test Case is run.
     * @param testCase The LongBow Test Case that is being torn-down.
     */
    LongBowTestFixtureTearDownFunction *tearDown;
};

static void
_longBowTestFixture_TestSucceeded(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalSucceeded++;
}

static void
_longBowTestFixture_TestFailed(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalFailed++;
}

static void
_longBowTestFixture_TestLeakedResources(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalFailed++;
}

static void
_longBowTestFixture_TestSkipped(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalSkipped++;
}

static void
_longBowTestFixture_TestSkipTests(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalSkipped++;
}

static void
_longBowTestFixture_TestWarned(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalWarned++;
}

static void
_longBowTestFixture_TestSetupFailed(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalSetupFailed++;
}

static void
_longBowTestFixture_TestSignalled(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalSignalled++;
}

static void
_longBowTestFixture_TestStopped(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalStopped++;
}

static void
_longBowTestFixture_TestUnimplemented(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalUnimplemented++;
}

static void
_longBowTestFixture_TestImpotent(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalUnimplemented++;
}

static void
_longBowTestFixture_Untested(LongBowTestFixtureSummary *summary __attribute__((unused)))
{
}

static void
_longBowTestFixture_TearDownFailed(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalTearDownFailed++;
}

static void
_longBowTestFixture_TearDownWarned(LongBowTestFixtureSummary *summary)
{
    summary->totalTested++;
    summary->totalTearDownWarned++;
}

void
longBowTestFixture_ConfigHelp(void)
{
    printf("Test Fixture options:\n");
    printf("  --set <runnerName>/<fixtureName>/iterations=<integer>  Run the named test fixture <integer> times.\n");
    printf("  --set <runnerName>/<fixtureName>/enable=(true|false) Enable or disable execution of the named test fixture.\n");
}

bool
longBowTestFixture_Config(LongBowConfig *config, const char *parameter)
{
    bool result = false;
    return result;
}

LongBowTestFixture *
longBowTestFixture_Create(const LongBowTestRunner *testRunner,
                          const char *fixtureName,
                          LongBowTestFixtureSetupFunction *setup,
                          LongBowTestFixtureFunction *fixtureFunction,
                          LongBowTestFixtureTearDownFunction *tearDown)
{
    LongBowTestFixture *fixture = longBowMemory_Allocate(sizeof(LongBowTestFixture));
    if (fixture != NULL) {
        fixture->runner = (LongBowTestRunner *) testRunner;
        fixture->name = fixtureName;

        int status = asprintf(&fixture->fullName, "%s/%s", longBowTestRunner_GetName(testRunner), fixtureName);
        if (status == -1) {
            return NULL;
        }

        fixture->testCases = longBowArrayList_Create((void (*)(void **))longBowTestCase_Destroy);
        fixture->setUp = setup;
        fixture->fixture = fixtureFunction;
        fixture->tearDown = tearDown;
    }

    return fixture;
}

void
longBowTestFixture_Destroy(LongBowTestFixture **fixturePtr)
{
    LongBowTestFixture *fixture = *fixturePtr;

    free(fixture->fullName);

    longBowArrayList_Destroy(&fixture->testCases);
    longBowMemory_Deallocate((void **) fixturePtr);
}

const char *
longBowTestFixture_GetFullName(const LongBowTestFixture *testFixture)
{
    return testFixture->fullName;
}

void
longBowTestFixture_UpdateSummary(LongBowTestCase *longBowTestCase)
{
    switch (longBowRuntimeResult_GetStatus(longBowTestCase_GetActualResult(longBowTestCase))) {
        case LONGBOW_STATUS_SUCCEEDED:
            _longBowTestFixture_TestSucceeded(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LONGBOW_STATUS_SKIPPED:
            _longBowTestFixture_TestSkipped(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LongBowStatus_WARNED:
            _longBowTestFixture_TestWarned(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LONGBOW_STATUS_SETUP_FAILED:
            _longBowTestFixture_TestSetupFailed(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LONGBOW_STATUS_SETUP_SKIPTESTS:
            _longBowTestFixture_TestSkipTests(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LONGBOW_STATUS_TEARDOWN_FAILED:
            _longBowTestFixture_TearDownFailed(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LongBowStatus_TEARDOWN_WARNED:
            _longBowTestFixture_TearDownWarned(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LONGBOW_STATUS_FAILED:
            _longBowTestFixture_TestFailed(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LongBowStatus_STOPPED:
            _longBowTestFixture_TestStopped(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LongBowStatus_UNIMPLEMENTED:
            _longBowTestFixture_TestUnimplemented(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LongBowStatus_IMPOTENT:
            _longBowTestFixture_TestImpotent(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LongBowStatus_UNTESTED:
            _longBowTestFixture_Untested(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        case LONGBOW_STATUS_MEMORYLEAK:
            _longBowTestFixture_TestLeakedResources(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            break;

        default:
            if (longBowTestCase_GetActualResult(longBowTestCase)->status >= LongBowStatus_SIGNALLED) {
                _longBowTestFixture_TestSignalled(&longBowTestCase_GetFixture(longBowTestCase)->summary);
            } else {
                printf("longBowTestFixture_UpdateSummary: unhandled status %d\n",
                       longBowTestCase_GetActualResult(longBowTestCase)->status);
            }
    }
}

void
longBowTestFixture_AddTestCase(const LongBowTestFixture *fixture, LongBowTestCase *longBowTestCase)
{
    longBowTestFixture_UpdateSummary(longBowTestCase);

    longBowArrayList_Add(fixture->testCases, longBowTestCase);
}

LongBowTestCase *
longBowTestFixture_GetTestCase(const LongBowTestFixture *fixture, size_t index)
{
    LongBowTestCase *testCase = longBowArrayList_Get(fixture->testCases, index);

    return testCase;
}

const char *
longBowTestFixture_GetName(const LongBowTestFixture *fixture)
{
    return fixture->name;
}

const LongBowTestFixtureSummary *
longBowTestFixture_GetSummary(const LongBowTestFixture *fixture)
{
    return &(fixture->summary);
}

size_t
longBowTestFixture_GetTestCaseCount(const LongBowTestFixture *fixture)
{
    return longBowArrayList_Length(fixture->testCases);
}

LongBowStatus
longBowTestFixture_Setup(LongBowTestFixture *fixture, LongBowTestCase *testCase)
{
    if (longBowConfig_IsTrace(longBowTestRunner_GetConfiguration(longBowTestFixture_GetRunner(fixture)))) {
        longBowReportTesting_Trace("    %s/%s: setup",
                                   longBowTestRunner_GetName(longBowTestFixture_GetRunner(fixture)),
                                   longBowTestFixture_GetName(fixture));
    }
    LongBowClipBoard *testClipBoard = longBowTestFixture_GetClipBoard(fixture);
    LongBowStatus result = (longBowTestCase_GetFixture(testCase)->setUp)(longBowTestFixture_GetRunner(fixture), fixture, testCase, testClipBoard);
    return result;
}

LongBowStatus
longBowTestFixture_TearDown(LongBowTestFixture *fixture, LongBowTestCase *testCase)
{
    if (longBowConfig_IsTrace(longBowTestRunner_GetConfiguration(longBowTestFixture_GetRunner(fixture)))) {
        longBowReportTesting_Trace("    %s/%s: tearDown",
                                   longBowTestRunner_GetName(longBowTestFixture_GetRunner(fixture)),
                                   longBowTestFixture_GetName(fixture));
    }
    LongBowClipBoard *testClipBoard = longBowTestFixture_GetClipBoard(fixture);
    LongBowStatus result = (longBowTestCase_GetFixture(testCase)->tearDown)(longBowTestFixture_GetRunner(fixture), fixture, testCase, testClipBoard);
    return result;
}

LongBowStatus
longBowTestFixture_GetStatus(const LongBowTestFixture *fixture)
{
    size_t nTestCases = longBowTestFixture_GetTestCaseCount(fixture);

    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    // Just return the status of the first non-successful test case.
    for (size_t i = 0; i < nTestCases; i++) {
        LongBowTestCase *testCase = longBowTestFixture_GetTestCase(fixture, i);
        if (!longBowTestCase_IsSuccessful(testCase)) {
            result = longBowTestCase_GetStatus(testCase);
            break;
        }
    }
    return result;
}

bool
longBowTestFixture_IsSuccessful(const LongBowTestFixture *testFixture)
{
    return longBowStatus_IsSuccessful(longBowTestFixture_GetStatus(testFixture));
}

char *
longBowTestFixture_ToString(const LongBowTestFixture *fixture)
{
    char *runnerString = longBowTestRunner_ToString(longBowTestFixture_GetRunner(fixture));

    char *string;
    if (asprintf(&string, "%s/%s", runnerString, longBowTestFixture_GetName(fixture)) == -1) {
        return NULL;
    }

    free(runnerString);

    return string;
}

LongBowTestFixture *
longBowTestFixture_Run(const LongBowTestRunner *testRunner,
                       const char *fixtureName,
                       const LongBowTestFixtureConfig *config,
                       LongBowTestFixtureSetupFunction *setup,
                       LongBowTestFixtureFunction *fixtureRun,
                       LongBowTestFixtureTearDownFunction *tearDown)
{
    LongBowTestFixture *testFixture = longBowTestFixture_Create(testRunner, fixtureName, setup, fixtureRun, tearDown);

    LongBowConfig *configuration = longBowTestRunner_GetConfiguration(testRunner);

    bool enabled = longBowConfig_GetBoolean(configuration, config->enabled, "%s/enabled", longBowTestFixture_GetFullName(testFixture));
    unsigned long iterations = longBowConfig_GetUint32(configuration, 1, "%s/iterations", longBowTestFixture_GetFullName(testFixture));

    if (enabled) {
        for (unsigned long i = 0; i < iterations; i++) {
            (*testFixture->fixture)(testRunner, testFixture);
        }
        longBowTestRunner_AddFixture(testFixture->runner, testFixture);
    }

    return testFixture;
}

LongBowTestRunner *
longBowTestFixture_GetRunner(const LongBowTestFixture *fixture)
{
    return fixture->runner;
}

LongBowClipBoard *
longBowTestFixture_GetClipBoard(const LongBowTestFixture *fixture)
{
    return longBowTestRunner_GetClipBoard(fixture->runner);
}
