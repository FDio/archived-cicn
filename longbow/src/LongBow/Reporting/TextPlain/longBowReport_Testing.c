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
#include <assert.h>

#include <LongBow/Reporting/longBowReport_Testing.h>
#include <LongBow/private/longBow_String.h>

static const LongBowTestRunner *
_testRunnerSilent(const LongBowTestRunner *testRunner)
{
    LongBowStatus status = longBowTestRunner_GetStatus(testRunner);

    printf("%s %s\n", longBowTestRunner_GetName(testRunner), longBowStatus_ToString(status));
    return testRunner;
}

static const LongBowTestRunner *
_testRunnerDetail(const LongBowTestRunner *testRunner)
{
    size_t nFixtures = longBowTestRunner_GetFixtureCount(testRunner);

    printf("\r\n");
    printf("%s: %zd fixture%s\r\n", longBowTestRunner_GetName(testRunner), nFixtures, (nFixtures == 1 ? "" : "s"));

    for (size_t i = 0; i < nFixtures; i++) {
        LongBowTestFixture *fixture = longBowTestRunner_GetFixture(testRunner, i);
        longBowReportTesting_TestFixture(fixture);
    }
    return testRunner;
}

const LongBowTestRunner *
longBowReportTesting_TestRunner(const LongBowTestRunner *testRunner)
{
    if (longBowConfig_GetBoolean(longBowTestRunner_GetConfiguration(testRunner), false, "silent")) {
        return _testRunnerSilent(testRunner);
    } else {
        return _testRunnerDetail(testRunner);
    }
}

static unsigned int
_totalSucceeded(const LongBowTestFixtureSummary *summary)
{
    return summary->totalSucceeded + summary->totalWarned + summary->totalTearDownWarned;
}

static unsigned int
_totalWarned(const LongBowTestFixtureSummary *summary)
{
    return summary->totalWarned + summary->totalTearDownWarned;
}

static unsigned int
_totalFailed(const LongBowTestFixtureSummary *summary)
{
    return summary->totalFailed + summary->totalSignalled + summary->totalStopped + summary->totalTearDownFailed;
}

static unsigned int
_totalIncomplete(const LongBowTestFixtureSummary *summary)
{
    return summary->totalSetupFailed + summary->totalSkipped + summary->totalUnimplemented;
}

static void
_reportSummary(const LongBowTestFixture *testFixture)
{
    const LongBowTestFixtureSummary *summary = longBowTestFixture_GetSummary(testFixture);

    char *fixtureString = longBowTestFixture_ToString(testFixture);

    printf("%s: Ran %u test case%s.", fixtureString, summary->totalTested, summary->totalTested == 1 ? "" : "s");
    free(fixtureString);

    if (summary->totalTested > 0) {
        printf(" %d%% (%d) succeeded", _totalSucceeded(summary) * 100 / summary->totalTested, _totalSucceeded(summary));

        if (_totalWarned(summary) > 0) {
            printf(" %d%% (%d) with warnings", _totalWarned(summary) * 100 / _totalSucceeded(summary), _totalWarned(summary));
        }
        if (_totalFailed(summary) != 0) {
            printf(", %d%% (%d) failed", _totalFailed(summary) * 100 / summary->totalTested, _totalFailed(summary));
        }
        if (_totalIncomplete(summary) > 0) {
            printf(", %d%% (%d) incomplete", _totalIncomplete(summary) * 100 / summary->totalTested, _totalIncomplete(summary));
        }
    }
    printf("\n");
}

const LongBowTestFixture *
longBowReportTesting_TestFixture(const LongBowTestFixture *testFixture)
{
    size_t nTestCases = longBowTestFixture_GetTestCaseCount(testFixture);

    _reportSummary(testFixture);

    for (size_t i = 0; i < nTestCases; i++) {
        LongBowTestCase *testCase = longBowTestFixture_GetTestCase(testFixture, i);
        longBowReportTesting_TestCase(testCase);
    }
    return testFixture;
}

const LongBowTestCase *
longBowReportTesting_TestCase(const LongBowTestCase *testCase)
{
    LongBowRuntimeResult *testCaseResult = longBowTestCase_GetActualResult(testCase);

    char *rusageString = longBowReportRuntime_RUsageToString(longBowRuntimeResult_GetRUsage(testCaseResult));

    char *elapsedTimeString = longBowReportRuntime_TimevalToString(longBowRuntimeResult_GetElapsedTime(testCaseResult));

    char *statusString = longBowStatus_ToString(longBowTestCase_GetActualResult(testCase)->status);
    char *testCaseString = longBowTestCase_ToString(testCase);

    LongBowString *string = longBowString_CreateFormat("%10s %s %s %zd %s\n",
                                                       testCaseString,
                                                       elapsedTimeString,
                                                       rusageString,
                                                       longBowRuntimeResult_GetEventEvaluationCount(longBowTestCase_GetActualResult(testCase)),
                                                       statusString);
    longBowString_Write(string, stdout);
    longBowString_Destroy(&string);

    free(testCaseString);
    free(statusString);
    free(elapsedTimeString);
    free(rusageString);

    return testCase;
}

void
longBowReportTesting_DisplayTestCaseResult(const LongBowTestCase *testCase)
{
    const LongBowRuntimeResult *testCaseResult = longBowTestCase_GetActualResult(testCase);

    switch (testCaseResult->status) {
        case LongBowStatus_UNTESTED:       printf("X");    break;
        case LONGBOW_STATUS_SUCCEEDED:      printf(".");    break;
        case LONGBOW_STATUS_SKIPPED:        printf("S");    break;
        case LongBowStatus_WARNED:         printf("W");    break;
        case LONGBOW_STATUS_SETUP_FAILED:   printf("s");    break;
        case LONGBOW_STATUS_TEARDOWN_FAILED: printf("t");    break;
        case LongBowStatus_TEARDOWN_WARNED: printf("w");    break;
        case LONGBOW_STATUS_FAILED:         printf("F");    break;
        case LongBowStatus_STOPPED:        printf("T");    break;
        case LongBowStatus_UNIMPLEMENTED:  printf("U");    break;
        case LongBowStatus_IMPOTENT:       printf("I");    break;
        default:
            if (testCaseResult->status >= LongBowStatus_SIGNALLED) {
                printf("K");
            } else {
                printf("?");
            }
    }
    fflush(stdout);
}

void
longBowReportTesting_Trace(const char *restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *message;
    if (vasprintf(&message, format, ap) == -1) {
        return;
    }
    va_end(ap);

    printf("%s\n", message);
    free(message);
}
