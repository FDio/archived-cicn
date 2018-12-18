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
#include <inttypes.h>

#include <LongBow/unit-test.h>

#include <LongBow/longBow_TestRunner.h>
#include <LongBow/longBow_Properties.h>

#include <LongBow/private/longBow_String.h>
#include <LongBow/private/longBow_Memory.h>
#include <LongBow/private/longBow_ArrayList.h>

struct LongBowTestRunner {
    const char *name; /**< The name of this LongBow test runner. */

    LongBowStatus (*testRunnerSetup)(LongBowTestRunner *); /**< The Test Runner Setup function */

    void (*testRunner)(LongBowTestRunner *); /**< The Test Case Runner function */

    LongBowStatus (*testRunnerTearDown)(LongBowTestRunner *); /**< The Test Runner TearDown function */

    LongBowArrayList *fixtures; /**< The  LongBowTestFixtures of this Test Runner */

    LongBowConfig *configuration; /**< The LongBowConfiguration for this Test Runner. */

    /**
     * The clipboard of information shared between the fixture setup,
     * the test case, and the fixture tear-down.
     */
    LongBowClipBoard *clipBoard;
};

void
longBowTestRunner_ConfigHelp(void)
{
    printf("Test Runner options:\n");
    printf("  --set <testRunnerName>/iterations=<count>  Run the named test runner <count> times\n");
}

bool
longBowTestRunner_Config(LongBowConfig *config, const char *parameter)
{
    bool result = false;
    LongBowArrayList *tokens = longBowString_Tokenise(parameter, "-=");
    if (tokens != NULL) {
        if (longBowArrayList_Length(tokens) == 2) {
            result = longBowConfig_SetProperty(config, longBowArrayList_Get(tokens, 0), longBowArrayList_Get(tokens, 1));
        }
        longBowArrayList_Destroy(&tokens);
    }
    return result;
}

const char *
longBowTestRunner_GetName(const LongBowTestRunner *testRunner)
{
    assert(testRunner != NULL);
    return testRunner->name;
}

void
longBowTestRunner_AddFixture(LongBowTestRunner *testRunner, LongBowTestFixture *testFixture)
{
    longBowArrayList_Add(testRunner->fixtures, testFixture);
}

LongBowTestRunner *
longBowTestRunner_Create(const char *name,
                         LongBowTestRunnerSetUp *setup,
                         LongBowTestRunnerRun *runner,
                         LongBowTestRunnerTearDown *tearDown)
{
    LongBowTestRunner *testRunner = longBowMemory_Allocate(sizeof(LongBowTestRunner));

    if (testRunner != NULL) {
        testRunner->name = name;
        testRunner->testRunnerSetup = setup;
        testRunner->testRunner = runner;
        testRunner->testRunnerTearDown = tearDown;
        testRunner->fixtures = longBowArrayList_Create((void (*)(void **))longBowTestFixture_Destroy);
        testRunner->clipBoard = longBowClipBoard_Create();
    }
    return testRunner;
}

void
longBowTestRunner_Destroy(LongBowTestRunner **testRunnerPtr)
{
    if (testRunnerPtr != NULL) {
        LongBowTestRunner *testRunner = *testRunnerPtr;

        longBowArrayList_Destroy(&testRunner->fixtures);
        if (testRunner->clipBoard) {
            longBowClipBoard_Destroy(&testRunner->clipBoard);
        }
        if (testRunner != NULL) {
            longBowMemory_Deallocate((void **) testRunnerPtr);
        }
    }
}

LongBowTestRunner *
longBowTestRunner_Run(LongBowTestRunner *testRunner)
{
    LongBowConfig *configuration = longBowTestRunner_GetConfiguration(testRunner);
    unsigned long iterations = longBowConfig_GetUint32(configuration, 1,
                                                       "%s/iterations", longBowTestRunner_GetName(testRunner));

    if (longBowConfig_IsTrace(configuration)) {
        longBowReportTesting_Trace("%s: setup", longBowTestRunner_GetName(testRunner));
    }
    LongBowStatus setupStatus = (*testRunner->testRunnerSetup)(testRunner);

    if (setupStatus != LONGBOW_STATUS_SETUP_SKIPTESTS) {
        if (!longBowStatus_IsSuccessful(setupStatus)) {
            char *statusString = longBowStatus_ToString(setupStatus);
            printf("Warning: %s setup returned: %s.\n", testRunner->name, statusString);
            free(statusString);
            return testRunner;
        }

        for (unsigned long i = 0; i < iterations; i++) {
            if (longBowConfig_IsTrace(configuration)) {
                longBowReportTesting_Trace("%s: run", longBowTestRunner_GetName(testRunner));
            }
            (*testRunner->testRunner)(testRunner);
        }

        if (longBowConfig_IsTrace(configuration)) {
            longBowReportTesting_Trace("%s: tear-down", longBowTestRunner_GetName(testRunner));
        }

        LongBowStatus tearDownStatus = (*testRunner->testRunnerTearDown)(testRunner);
        if (!longBowStatus_IsSuccessful(tearDownStatus)) {
            char *statusString = longBowStatus_ToString(tearDownStatus);
            printf("Warning: %s tear-down returned: %s.\n", testRunner->name, statusString);
            free(statusString);
            return testRunner;
        }
    }

    return testRunner;
}

char *
longBowTestRunner_ToString(const LongBowTestRunner *runner)
{
    LongBowString *string = longBowString_CreateFormat("%s", longBowTestRunner_GetName(runner));

    char *cString = longBowString_ToString(string);
    longBowString_Destroy(&string);
    return cString;
}

LongBowTestFixture *
longBowTestRunner_GetFixture(const LongBowTestRunner *testRunner, size_t index)
{
    return longBowArrayList_Get(testRunner->fixtures, index);
}

size_t
longBowTestRunner_GetFixtureCount(const LongBowTestRunner *testRunner)
{
    return longBowArrayList_Length(testRunner->fixtures);
}

LongBowConfig *
longBowTestRunner_GetConfiguration(const LongBowTestRunner *testRunner)
{
    return testRunner->configuration;
}

LongBowStatus
longBowTestRunner_GetStatus(const LongBowTestRunner *testRunner)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    // Just return the status of the first non-successful Test Fixture.
    size_t nTestFixtures = longBowTestRunner_GetFixtureCount(testRunner);
    for (size_t i = 0; i < nTestFixtures; i++) {
        LongBowTestFixture *fixture = longBowTestRunner_GetFixture(testRunner, i);
        if (!longBowTestFixture_IsSuccessful(fixture)) {
            result = longBowTestFixture_GetStatus(fixture);
            break;
        }
    }
    return result;
}

bool
longBowTestRunner_IsSuccessful(const LongBowTestRunner *testRunner)
{
    return longBowStatus_IsSuccessful(longBowTestRunner_GetStatus(testRunner));
}

bool
longBowTestRunner_IsFailed(const LongBowTestCase *testCase)
{
    return longBowStatus_IsFailed(longBowTestCase_GetStatus(testCase));
}

bool
longBowTestRunner_IsWarning(const LongBowTestRunner *testCase)
{
    return longBowStatus_IsWarning(longBowTestRunner_GetStatus(testCase));
}

bool
longBowTestRunner_IsIncomplete(const LongBowTestCase *testCase)
{
    return longBowStatus_IsIncomplete(longBowTestCase_GetStatus(testCase));
}

void
longBowTestRunner_SetConfiguration(LongBowTestRunner *testRunner, LongBowConfig *config)
{
    testRunner->configuration = config;
}

LongBowClipBoard *
longBowTestRunner_GetClipBoard(const LongBowTestRunner *testRunner)
{
    return testRunner->clipBoard;
}

bool
longBowTestRunner_SetClipBoardData(const LongBowTestRunner *testRunner, void *shared)
{
    return longBowClipBoard_Set(longBowTestRunner_GetClipBoard(testRunner), "testRunner", shared);
}

void *
longBowTestRunner_GetClipBoardData(const LongBowTestRunner *testRunner)
{
    return longBowClipBoard_Get(longBowTestRunner_GetClipBoard(testRunner), "testRunner");
}
