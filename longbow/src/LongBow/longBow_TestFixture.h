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
 * @file longBow_TestFixture.h
 * @ingroup internals
 * @brief Manage the execution of Test Cases.
 *
 * A Test Fixture manages the individual execution of Test Cases.
 *
 */
#ifndef LongBowTestFixture_H_
#define LongBowTestFixture_H_

#include <string.h>

struct longbow_fixture;

/**
 * @typedef LongBowTestFixture
 * @brief The LongBow Test Fixture.
 *
 * This structure is created and initialized by the LongBow Test Runner.
 *      When the Test Fixture represented by this structure is executed,
 *      this structure is updated with the results of each LongBow Test Case that belongs to this Fixture.
 */
typedef struct longbow_fixture LongBowTestFixture;

#include <LongBow/longBow_ClipBoard.h>
#include <LongBow/longBow_TestRunner.h>
#include <LongBow/longBow_TestCase.h>
#include <LongBow/longBow_TestFixtureConfig.h>

typedef void (LongBowTestFixtureFunction)(const LongBowTestRunner *testRunner, const LongBowTestFixture *testFixture);

typedef int (LongBowTestFixtureSetupFunction)(const LongBowTestRunner *, const LongBowTestFixture *, const LongBowTestCase *, LongBowClipBoard *);

typedef int (LongBowTestFixtureTearDownFunction)(const LongBowTestRunner *, const LongBowTestFixture *, const LongBowTestCase *, LongBowClipBoard *);

/**
 * @typedef LongBowTestFixtureSummary
 * @brief The summary for a test fixture.
 */
typedef struct longbow_fixture_summary {
    /**
     * The total number of test cases executed.
     */
    unsigned int totalTested;
    /**
     * The number of test cases that succeeded.
     */
    unsigned int totalSucceeded;
    /**
     * The number of test cases that failed.
     */
    unsigned int totalFailed;
    /**
     * The number of test cases that were skipped.
     */
    unsigned int totalSkipped;
    /**
     * The number of test cases that issued a warning.
     */
    unsigned int totalWarned;
    /**
     * The number of test cases that setup failed.
     */
    unsigned int totalSetupFailed;
    /**
     * The number of test cases that failed due to an signal.
     */
    unsigned int totalSignalled;
    /**
     * The number of test cases that failed due to a stop signal.
     */
    unsigned int totalStopped;
    /**
     * The number of test cases in which the tear-down failed.
     */
    unsigned int totalTearDownFailed;
    /**
     * The number of test cases in which the tear-down issued a warning.
     */
    unsigned int totalTearDownWarned;
    /**
     * The number of test cases that existed but indicated they were unimplemented.
     */
    unsigned int totalUnimplemented;
} LongBowTestFixtureSummary;

/**
 * Update the summary information from the given LongBowTestCase.
 *
 * @param testCase A pointer to a valid LongBowTestCase instance.
 */
void longBowTestFixture_UpdateSummary(LongBowTestCase *testCase);

/**
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @param testCase A pointer to a valid LongBowTestCase instance.
 */
void longBowTestFixture_AddTestCase(const LongBowTestFixture *testFixture, LongBowTestCase *testCase);

/**
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @param index The index of the instance to get.
 * @return A pointer to the LongBowTestCase instance at the given index.
 */
LongBowTestCase *longBowTestFixture_GetTestCase(const LongBowTestFixture *testFixture, size_t index);

/**
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @return A pointer ot the LongBowTestRunner instance of the given LongBowTestFixture.
 */
LongBowTestRunner *longBowTestFixture_GetRunner(const LongBowTestFixture *testFixture);

/**
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @return A pointer to the C string of the name of the given LongBowTestFixture.
 */
const char *longBowTestFixture_GetName(const LongBowTestFixture *testFixture);

/**
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @return The number of LongBow Test Cases in the given LongBowTestFixture.
 */
size_t longBowTestFixture_GetTestCaseCount(const LongBowTestFixture *testFixture);

/**
 * Initialise a {@link LongBowTestFixture} structure with the given parameters.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @param [in] fixtureName      The name of this Test Fixture.
 * @param [in] setup            A pointer to the function to call before invoking the first Test Case.
 * @param [in] fixtureFunction	A pointer to the function to call that will run each Test Case.
 * @param [in] tearDown             A pointer to the function to call after invoking the last Test Case.
 * @return A pointer to an allocated LongBowTestFixture instance that must be deallocated via LongBowTestFixture_Destroy.
 */
LongBowTestFixture *longBowTestFixture_Create(const LongBowTestRunner *testRunner,
                                              const char *fixtureName,
                                              LongBowTestFixtureSetupFunction *setup,
                                              LongBowTestFixtureFunction *fixtureFunction,

                                              LongBowTestFixtureTearDownFunction *tearDown);

/**
 * Destroy a LongBowTestFixture structure.
 *
 * @param fixturePtr A pointer to a LongBowTestFixture structure pointer.
 */
void longBowTestFixture_Destroy(LongBowTestFixture **fixturePtr);

/**
 * Get the fully qualified name of the given `LongBowTestFixture`.
 *
 * @param [in] testFixture A pointer to a valid LongBowTestCase instance.
 *
 * @return A constant nul-terminated, C string.
 */
const char *longBowTestFixture_GetFullName(const LongBowTestFixture *testFixture);

void longBowTestFixture_ConfigHelp(void);

bool longBowTestFixture_Config(LongBowConfig *config, const char *parameter);

/**
 * Execute a LongBow Test Fixture.
 *
 * The Test Fixture will execute Test Cases in the order they appear in the Test Fixture function.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @param [in] fixtureName The name of this Test Fixture.
 * @param [in] config A pointer to a LongBowTestFixtureConfig to use when running the Test Fixture.
 * @param [in] setup A pointer to the function to call before invoking the first Test Case.
 * @param [in] fixtureRun A pointer to the function to call that will run each Test Case.
 * @param [in] tearDown A pointer to the function to call after invoking the last Test Case.
 * @return An allocated structure representing this Test Fixture.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see LONGBOW_TEST_FIXTURE
 */
LongBowTestFixture *longBowTestFixture_Run(const LongBowTestRunner *testRunner,
                                           const char *fixtureName,
                                           const LongBowTestFixtureConfig *config,
                                           LongBowTestFixtureSetupFunction *setup,
                                           LongBowTestFixtureFunction *fixtureRun,
                                           LongBowTestFixtureTearDownFunction *tearDown);

/**
 * Get the status of the given LongBow Test Fixture.
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @return The `LongBowStatus` of the given Test Fixture.
 */
LongBowStatus longBowTestFixture_GetStatus(const LongBowTestFixture *testFixture);

/**
 * Return <code>true</code> if the given test case was successful.
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @return `true` if the given test case was successful.
 */
bool longBowTestFixture_IsSuccessful(const LongBowTestFixture *testFixture);

/**
 * Get a pointer to {@link LongBowTestFixtureSummary} for the given Test Fixture.
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @return A pointer to {@link LongBowTestFixtureSummary} for the given Test Fixture.
 */
const LongBowTestFixtureSummary *longBowTestFixture_GetSummary(const LongBowTestFixture *testFixture);

/**
 * Perform a test case setup.
 *
 * @param [in] testFixture A pointer to a valid LongBowTestFixture instance.
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return The `LongBowStatus` returned from the setup function.
 */
LongBowStatus longBowTestFixture_Setup(LongBowTestFixture *testFixture, LongBowTestCase *testCase);

/**
 * Perform a test case teardown.
 *
 * @param testFixture A pointer to a valid LongBowTestFixture instance.
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return The `LongBowStatus` returned from the teardown function.
 */
LongBowStatus longBowTestFixture_TearDown(LongBowTestFixture *testFixture, LongBowTestCase *testCase);

/**
 * Get the clipboard that belongs to the given Test Fixture.
 *
 * Every LongBow test fixure has an associated "clipboard" that is specific to that fixture and
 * is shared between the test runner setup and tear down functions.
 * Test Runner setup may store things on the test fixture's clipboard,
 * the test fixture may access them and the tear down destroy them.
 *
 * @param [in] testFixture A pointer to a valid LongBowTestFixture instance.
 * @return The `LongBowTestCaseClipBoard` for the given `LongBowTestFixture`.
 *
 * @see longBowTestCase_GetClipBoard
 */
LongBowClipBoard *longBowTestFixture_GetClipBoard(const LongBowTestFixture *testFixture);

/**
 * Compose a C string representing the current state of the given `LongBowTestFixture`.
 *
 * @param [in] testFixture A pointer to a valid LongBowTestFixture instance.
 * @return An allocated, nul-terminated C string that must be deallocated via free(3)
 */
char *longBowTestFixture_ToString(const LongBowTestFixture *testFixture);
#endif // LongBowTestFixture_H_
