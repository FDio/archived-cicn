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
 * @file longBow_TestRunner.h
 * @ingroup testing
 * @brief LongBow Test Runner Support.
 *
 */
#ifndef LONGBOWRUNNER_H_
#define LONGBOWRUNNER_H_

#include <stdbool.h>

struct LongBowTestRunner;
/**
 * @typedef LongBowRunner
 * @brief The LongBow Test Runner
 */
typedef struct LongBowTestRunner LongBowRunner;

typedef struct LongBowTestRunner LongBowTestRunner;

#include <LongBow/longBow_Config.h>
#include <LongBow/longBow_Status.h>
#include <LongBow/longBow_TestCase.h>
#include <LongBow/longBow_TestFixture.h>

/**
 * The function prototype for a LongBow Test Runner set-up function.
 *
 * @param testRunner A pointer to the associated LongBowTestRunner.
 * @return A LongBowStatus value indicating the result of the set-up.
 */
typedef LongBowStatus (LongBowTestRunnerSetUp)(LongBowTestRunner *testRunner);

/**
 * The function prototype for a LongBow Test Runner function.
 *
 * @param testRunner A pointer to the associated LongBowTestRunner.
 */
typedef void (LongBowTestRunnerRun)(LongBowTestRunner *testRunner);

/**
 * The function prototype for a LongBow Test Runner tear-down function.
 *
 * @param  testRunner A pointer to the associated LongBowTestRunner.
 * @return A LongBowStatus value indicating the result of the tear-down.
 */
typedef LongBowStatus (LongBowTestRunnerTearDown)(LongBowTestRunner *testRunner);

/**
 * Allocate and initialise a LongBowTestRunner structure with the given parameters.
 *
 * See <code>longBowTestRunner_Destroy</code> to destroy the result.
 *
 * The parameters may be static values (ie. not allocated) supplied by the caller
 * and they are not subject to free in <code>longBowTestRunner_Destroy</code>.
 *
 * @param [in] name The name of the LongBow test runner as a null-terminated string.
 * @param [in] setup The setup function for the Test Runner
 * @param [in] testRunner The function to call to execute the Test Runner.
 * @param [in] tearDown The function to call to tear-down the Test Runner.
 * @return An allocated LongBowTestRunner structure representing a Test Runner.
 */
LongBowTestRunner *longBowTestRunner_Create(const char *name,
                                            LongBowTestRunnerSetUp *setup,
                                            LongBowTestRunnerRun *testRunner,
                                            LongBowTestRunnerTearDown *tearDown);

/**
 * Destroy a previously allocated LongBowTestRunner structure.
 *
 * @param [in,out] testRunnerPtr A pointer to a previously allocated LongBowTestRunner structure.
 */
void longBowTestRunner_Destroy(LongBowTestRunner **testRunnerPtr);

/**
 * Get the name of the given LongBowTestRunner.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return The name of the Test Runner.
 */
const char *longBowTestRunner_GetName(const LongBowTestRunner *testRunner);

/**
 * Get the `LongBowClipBoard` for the given LongBowTestRunner.
 *
 * Every LongBow test case has an associated "clipboard" that is specific to a test case
 * and is shared between the fixture setup and tear down functions.
 * Fixture setup may store things on the clipboard,
 * the test case may access them and the tear down destroy them.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return The LongBowClipBoard for the given LongBowTestCase.
 */
LongBowClipBoard *longBowTestRunner_GetClipBoard(const LongBowTestRunner *testRunner);

/**
 * Place a value on the Test Runner "clipboard"
 *
 * Every Test Runner has an associated "clipboard" which is shared between the Test Runner
 * Setup and Test Runner Tear Down and is accessible to all Test Fixtures and Test Cases.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @param [in] shared The value to share on the clipboard.
 * @return True if the value was set.
 */
bool longBowTestRunner_SetClipBoardData(const LongBowTestRunner *testRunner, void *shared);

/**
 * Get the clipboard data from the given `LongBowTestRunner`.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 */
void *longBowTestRunner_GetClipBoardData(const LongBowTestRunner *testRunner);

/**
 * Execute a LongBow Test Runner.
 *
 * The Test Runner will have zero or more Test Fixtures executed in the order
 * they are specified in the Test Runner function.
 * See LONGBOW_TEST_RUNNER()
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return The given LongBowTestRunner
 */
LongBowTestRunner *longBowTestRunner_Run(LongBowTestRunner *testRunner);

/**
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return An allocated, nul-terminated C string that must be deallocated via free(3)
 */
char *longBowTestRunner_ToString(const LongBowTestRunner *testRunner);

/**
 * Add the supplied LongBowTestFixture to the list of Test Fixtures for the given Test Runner.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @param [in] testFixture A pointer to a valid LongBowTestFixture instance.
 */
void longBowTestRunner_AddFixture(LongBowTestRunner *testRunner, LongBowTestFixture *testFixture);

/**
 * Get the number of Test Fixtures in the given Test Runner.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return the number of Test Fixtures in the given Test Runner.
 */
size_t longBowTestRunner_GetFixtureCount(const LongBowTestRunner *testRunner);

/**
 * Get a pointer to the Test Fixture indexed by <code>index</code> in this Test Runner.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @param [in] index The index of the LongBowTestFixture to get.
 * @return The corresponding LongBowTestFixture.
 */
LongBowTestFixture *longBowTestRunner_GetFixture(const LongBowTestRunner *testRunner, size_t index);

/**
 * Get the status of the given LongBow Test Fixture.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return The LongBowStatus of the given LongBowTestRunner
 */
LongBowStatus longBowTestRunner_GetStatus(const LongBowTestRunner *testRunner);

/**
 * Get a pointer to the LongBowConfig structure for the given Test Runner.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return A pointer to the LongBowConfig structure for the given runner.
 */
LongBowConfig *longBowTestRunner_GetConfiguration(const LongBowTestRunner *testRunner);

/**
 * Return `true` if the given Test Runner was successful.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return true if the status of the LongBowTestRunner was successful.
 */
bool longBowTestRunner_IsSuccessful(const LongBowTestRunner *testRunner);

/**
 * Return `true` if the given Test Runner was a failure.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return true if the status of the LongBowTestRunner is as a failure.
 */
bool longBowTestRunner_IsFailed(const LongBowTestCase *testCase);

/**
 * Return `true` if the given Test Runner was a warning.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return true if the status of the LongBowTestRunner was a warning.
 */
bool longBowTestRunner_IsWarning(const LongBowTestRunner *testRunner);

/**
 * Return `true` if the given Test Runner was incomplete.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return true if the status of the LongBowTestRunner was incomplete.
 */
bool longBowTestRunner_IsIncomplete(const LongBowTestCase *testCase);

/**
 * Get the LongBowClipBoard for the given LongBowTestRunner.
 *
 * Every LongBow test runner has an associated "clipboard" that is specific to a test runner
 * and is shared between the runner setup and tear down functions.
 * The Test Runner setup may store things on the clipboard,
 * the test fixtures and test cases may access them and the test runner tear down destroy them.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return The LongBowClipBoard for the given LongBowTestRunner.
 */
LongBowClipBoard *longBowTestRunner_GetClipBoard(const LongBowTestRunner *testRunner);

/**
 * Set the configuration for the given LongBowTestRunner.
 *
 * The <code>LongBowConfig</code> pointed to by <code>configuration</code> is borrowed.
 * If it is deallocated, reused, or overwritten results are unpredicable.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @param [in] configuration A pointer to a valid LongBowConfig instance.
 */
void longBowTestRunner_SetConfiguration(LongBowTestRunner *testRunner, LongBowConfig *configuration);

/**
 * Get the clipboard data from the given `LongBowTestRunner`.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 */
void *longBowTestRunner_GetClipBoardData(const LongBowTestRunner *testRunner);

/**
 * Update the LongBowConfig instance with information indicated by @p parameter.
 *
 * @param [in] config A pointer to a valid LongBowConfig instance.
 * @param [in] parameter A pointer to nul-terminated C string.
 *
 * @return true A valid parameter.
 * @return false The value of @p parameter is invalid.
 */
bool longBowTestRunner_Config(LongBowConfig *config, const char *parameter);

/**
 * Print command line and configuration help applicable to a Long Bow Test Runner.
 *
 */
void longBowTestRunner_ConfigHelp(void);
#endif /* LONGBOWRUNNER_H_ */
