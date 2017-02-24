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
 * @file unit-test.h
 * @ingroup testing
 * @brief LongBow Unit Test Support.
 *
 * Every LongBow Test module must include this file as the first included file <em>after</em>
 * including the files necessary for the functions under test.
 *
 */
#ifndef UNIT_TEST_H_
#define UNIT_TEST_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>

#include <LongBow/runtime.h>

#include <LongBow/longBow_Compiler.h>
#include <LongBow/longBow_UnitTest.h>
#include <LongBow/longBow_Status.h>
#include <LongBow/longBow_Config.h>
#include <LongBow/longBow_UnitTesting.h>
#include <LongBow/longBow_Runtime.h>
#include <LongBow/longBow_Main.h>
#include <LongBow/longBow_TestRunner.h>
#include <LongBow/longBow_TestFixture.h>
#include <LongBow/longBow_TestCase.h>
#include <LongBow/longBow_SubProcess.h>
#include <LongBow/longBow_TestCaseMetaData.h>
#include <LongBow/Reporting/longBowReport_Testing.h>

#define longBowUnused(declaration) declaration __attribute__((unused))

/**
 * Test Runner setup function called before the invocation of the Test Fixtures associated with this Test Runner.
 *
 * Every Test Runner has a set-up and tear-down function that are invoked
 * just before and just after the execution of each test case function.
 * This function performs setup common for all fixtures to use.
 * If this function must return a valid `LongBowStatus` value.
 *
 * @param [in] runnerName A valid identifier for the Test Runner.
 *
 * @return A LongBowStatus indicating the status of the setup.
 *
 * Example:
 * @code
 * // An example of a minimal Test Runner Setup function.
 *
 * LONGBOW_TEST_RUNNER_SETUP(LongBow)
 * {
 *     return LONGBOW_STATUS_SUCCEEDED;
 * }
 * @endcode
 *
 * @see LONGBOW_TEST_RUNNER_TEARDOWN
 */
#define LONGBOW_TEST_RUNNER_SETUP(runnerName) \
    LongBowStatus longBowUnitTest_RunnerSetupName(runnerName) (LongBowTestRunner * testRunner); \
    LongBowStatus longBowUnitTest_RunnerSetupName(runnerName) (longBowUnused(LongBowTestRunner * testRunner))

/**
 * @brief The post-processing for a Test Runner called after all fixtures have been run.
 *
 * The Test Runner calls this function once after all the Test Fixtures are run.
 *
 * This function is responsible for resetting or restoring external resources previously setup by the
 * Test Runner Setup function and which may have been modified by any test fixture or test case.
 * If this function must return a valid LongBowStatus value.
 *
 * @see LONGBOW_TEST_RUNNER_SETUP
 * @param [in] longBowRunnerName A valid identifier for the Test Runner.
 */
#define LONGBOW_TEST_RUNNER_TEARDOWN(longBowRunnerName) \
    LongBowStatus longBowUnitTest_RunnerTearDownName(longBowRunnerName) (LongBowTestRunner * testRunner); \
    LongBowStatus longBowUnitTest_RunnerTearDownName(longBowRunnerName) (longBowUnused(LongBowTestRunner * testRunner))

/**
 * @brief Define a Test Case Runner with the given name.
 * The name must be valid syntax for a C identifier and will be used to compose a longer C function name.
 *
 * The resulting function that this macro defines specifies the parameter `LongBowTestRunner *testRunner`
 * which is a pointer to a LongBowTestRunner instance for the Test Runner.
 *
 * @param [in] testRunnerName A valid identifier for the Test Runner.
 */
#define LONGBOW_TEST_RUNNER(testRunnerName) \
    void longBowUnitTest_RunnerName(testRunnerName) (const LongBowTestRunner * testRunner); \
    void longBowUnitTest_RunnerName(testRunnerName) (const LongBowTestRunner * testRunner)

/**
 * @brief Create an allocated instance of a LongBowTestRunner that must be destroyed via `longBowTestRunner_Destroy`
 *
 * @param [in] testRunnerName A valid identifier for the Test Runner. (see LONGBOW_TEST_RUNNER)
 * @return The return value from longBowTestRunner_Create
 */
#define LONGBOW_TEST_RUNNER_CREATE(testRunnerName) \
    longBowTestRunner_Create(#testRunnerName, \
                             longBowUnitTest_RunnerSetupName(testRunnerName), \
                             (LongBowTestRunnerRun *) longBowUnitTest_RunnerName(testRunnerName), \
                             longBowUnitTest_RunnerTearDownName(testRunnerName))

#define LongBowTestRunner_Create(_runnerName_) \
    longBowTestRunner_Create(#_runnerName_, longBowUnitTest_RunnerSetupName(_runnerName_), \
                             longBowUnitTest_RunnerName(_runnerName_), longBowUnitTest_RunnerTearDownName(_runnerName_))

/**
 * @brief Run the LongBow test fixture with the specified `fixtureName`.
 *
 * @param [in] LongBowTestFixtureName A valid C identifier
 * @see LONGBOW_TEST_FIXTURE(fixtureName)
 */
#define LONGBOW_RUN_TEST_FIXTURE(LongBowTestFixtureName) \
    do { \
        extern LONGBOW_TEST_FIXTURE_SETUP(LongBowTestFixtureName); \
        extern LONGBOW_TEST_FIXTURE_TEARDOWN(LongBowTestFixtureName); \
        extern LongBowUnitTest_FixtureDeclaration(LongBowTestFixtureName); \
        extern LongBowTestFixtureConfig longBowUnitTest_FixtureConfigName(LongBowTestFixtureName); \
        longBowTestFixture_Run(testRunner, \
                               #LongBowTestFixtureName, \
                               &longBowUnitTest_FixtureConfigName(LongBowTestFixtureName), \
                               longBowUnitTest_FixtureSetupName(LongBowTestFixtureName), \
                               longBowUnitTest_FixtureName(LongBowTestFixtureName), \
                               longBowUnitTest_FixtureTearDownName(LongBowTestFixtureName)); \
    } while (0)



/**
 * @brief Define a test fixture with the given `fixtureName`.
 *
 * The value for `fixtureName` must be a valid syntax for a C identifier.
 *
 * @param [in] fixtureName  A valid syntax for a C identifier.
 */
#define LongBowUnitTest_FixtureDeclaration(_fixtureName_) \
    void longBowUnitTest_FixtureName(_fixtureName_)(longBowUnused(const LongBowTestRunner * testRunner), longBowUnused(const LongBowTestFixture * testFixture))

/**
 * @brief The default value for the expected result of a LongBow Fixture.
 */
#define LONGBOW_TEST_FIXTURE_CONFIG_DEFAULT .enabled = true

/**
 * @brief Define a test fixture with the given `fixtureName`.
 *
 * The value for `fixtureName` must be valid syntax for a C identifier.
 *
 * The resulting function that this macro defines specifies the parameter `LongBowTestFixture *LongBowTestFixture`
 * which is a pointer to a `LongBowTestFixture` instance for the Test Fixture.
 *
 * @param [in] fixtureName
 */
#define LONGBOW_TEST_FIXTURE(fixtureName) \
    LONGBOW_TEST_FIXTURE_OPTIONS(fixtureName, LONGBOW_TEST_FIXTURE_CONFIG_DEFAULT)

#define LONGBOW_TEST_FIXTURE_OPTIONS(fixtureName, ...) \
    LongBowCompiler_IgnoreInitializerOverrides \
    LongBowTestFixtureConfig longBowUnitTest_FixtureConfigName(fixtureName) = { LONGBOW_TEST_FIXTURE_CONFIG_DEFAULT, __VA_ARGS__ }; \
    LongBowCompiler_WarnInitializerOverrides \
    extern LONGBOW_TEST_FIXTURE_SETUP(fixtureName); \
    extern LONGBOW_TEST_FIXTURE_TEARDOWN(fixtureName); \
    LongBowUnitTest_FixtureDeclaration(fixtureName); \
    LongBowUnitTest_FixtureDeclaration(fixtureName)


/**
 * The pre-processing for a test fixture called before each invocation of a test case in the same fixture.
 *
 * This function is responsible for setting up the common programmatic state for each test case in the same fixture.
 * If this function returns `false` the corresponding test case is skipped.
 *
 * The resulting function that this macro defines specifies the parameters
 * `LongBowTestFixture *testFixture` and `LongBowTestCase *testCase`
 *
 * @param [in] fixtureName
 *
 * @see LONGBOW_TEST_FIXTURE_TEARDOWN
 */
#define LONGBOW_TEST_FIXTURE_SETUP(fixtureName) \
    LongBowStatus longBowUnitTest_FixtureSetupName(fixtureName) (longBowUnused(const LongBowTestRunner * testRunner), longBowUnused(const LongBowTestFixture * testFixture), longBowUnused(const LongBowTestCase * testCase), longBowUnused(LongBowClipBoard * testClipBoard))

/**
 * The post-processing for a test fixture called after each invocation of a test case in the same fixture.
 *
 * This function is responsible for resetting the common programmatic state for each test case in the same fixture.
 * If this function returns `false` the corresponding test case is considered passed, but warned.
 *
 * The resulting function that this macro defines specifies the parameters
 * `LongBowTestFixture *testFixture` and `LongBowTestCase *testCase`
 *
 * @param [in] fixtureName
 *
 * @see `LONGBOW_TEST_FIXTURE_SETUP`
 */
#define LONGBOW_TEST_FIXTURE_TEARDOWN(fixtureName) \
    LongBowStatus longBowUnitTest_FixtureTearDownName(fixtureName) (longBowUnused(const LongBowTestRunner * testRunner), longBowUnused(const LongBowTestFixture * testFixture), longBowUnused(const LongBowTestCase * testCase), longBowUnused(LongBowClipBoard * testClipBoard))

/**
 * @brief Defines the canonical name of a LongBow Test Case meta data.
 *
 * @param fixtureName The name of the Test Fixture that the Test Case belongs to.
 * @param testCaseName The name of the Test Case.
 */
#define longBowTestCase_MetaDataDeclaration(_testFixtureName_, _testCaseName_) \
    TestCase ## _testFixtureName_ ## _testCaseName_ ## MetaData

/**
 * @brief Forward declare a LongBow Test Case.
 *
 * @param fixtureName The name of the Test Fixture that the Test Case belongs to.
 * @param testCaseName The name of the Test Case.
 */
#define longBowUnitTest_TestCaseDeclaration(fixtureName, testCaseName) \
    void longBowUnitTest_CaseName(fixtureName, testCaseName) (longBowUnused(const LongBowTestRunner * testRunner), longBowUnused(const LongBowTestFixture * testFixture), longBowUnused(const LongBowTestCase * testCase), longBowUnused(const LongBowClipBoard * testClipBoard), longBowUnused(jmp_buf longBowTestCaseAbort))

/**
 * @brief The default value for the expected result of a LongBow Test Case.
 */
#define LongBowUnitTest_TestCaseDefaultExpectedResult .event = NULL, .status = LongBowStatus_DONTCARE

/**
 * Define a test case with the given `fixtureName` and `testCaseName`.
 *
 * The `fixtureName` must be the name of a defined fixture, see `LONGBOW_TEST_FIXTURE`.
 * The `testCaseName` must be valid syntax for a C identifier.
 *
 * @param fixtureName
 * @param testCaseName
 *
 * @code
 * LONGBOW_TEST_CASE(MyFixture, MyTestCase)
 * {
 *     assertTrue(true, "It lied to me!");
 * }
 */
#define LONGBOW_TEST_CASE(fixtureName, testCaseName) \
    LONGBOW_TEST_CASE_EXPECTS(fixtureName, testCaseName, LongBowUnitTest_TestCaseDefaultExpectedResult)

/**
 * Define a test case with the given `fixtureName` and `testCaseName` with an explictly specified status.
 *
 * The `fixtureName` must be the name of a defined fixture, see `LONGBOW_TEST_FIXTURE`.
 * The `testCaseName` is the name of the Test Case and must be valid syntax for a C identifier.
 * The variable number of subsequent arguments are a comma separated list of structure initialisers for the
 * LongBowRuntimeResult structure.
 *
 * For example, the construct
 * @code
 * LONGBOW_TEST_CASE_EXPECTS(MyFixture, alwaysWarn, .event = &LongBowAssertEvent)
 * @endcode
 * defines the Long Bow Test case `alwaysWarn` that is within the Test Fixture named `MyFixture`.
 * The expected termination is expected to be a `LongBowAssertEvent`, which means an assertion was triggered.
 *
 * If the termination status of the test must is equal to the specified status,
 * the test will be signaled as `LONGBOW_STATUS_SUCCESS, otherwise it will be LONGBOW_STATUS_FAILED.
 *
 * @param fixtureName
 * @param testCaseName
 */
#define LONGBOW_TEST_CASE_EXPECTS(fixtureName, testCaseName, ...) \
    LongBowCompiler_IgnoreInitializerOverrides \
    LongBowTestCaseMetaData longBowTestCase_MetaDataDeclaration(fixtureName, testCaseName) = \
    { .fileName           = __FILE__,                                      .functionName = #testCaseName, .lineNumber = __LINE__, \
      .expectedResult = { LongBowUnitTest_TestCaseDefaultExpectedResult, __VA_ARGS__ } }; \
    LongBowCompiler_WarnInitializerOverrides \
    longBowUnitTest_TestCaseDeclaration(fixtureName, testCaseName); \
    longBowUnitTest_TestCaseDeclaration(fixtureName, testCaseName)


/**
 * @brief Run a test case defined for the named <code>fixtureName</code> and <code>testCaseName</code>.
 *
 * This is executed within a LongBow test fixture function and references the function's
 * <code>LongBowTestFixture</code> parameter.
 *
 * @param [in] fixtureName
 * @param [in] testCaseName
 *
 * @see LONGBOW_TEST_CASE
 */
#define LONGBOW_RUN_TEST_CASE(fixtureName, testCaseName) \
    extern longBowUnitTest_TestCaseDeclaration(fixtureName, testCaseName); \
    extern LongBowTestCaseMetaData longBowTestCase_MetaDataDeclaration(fixtureName, testCaseName); \
    longBowTestCase_Run(#testCaseName, testFixture, longBowUnitTest_CaseName(fixtureName, testCaseName), \
                        &longBowTestCase_MetaDataDeclaration(fixtureName, testCaseName))

/**
 * @brief Configure and run a set of LongBowTestRunner instances
 *
 * @param [in] argc The number of elements in the @p argv array.
 * @param [in] argv An array of nul-terminated C-strings.
 *
 * Example Usage:
 * @code
 * int
 * main(int argc, char *argv[argc])
 * {
 *   LongBowTestRunner *testRunner1 = LONGBOW_TEST_RUNNER_CREATE(MyTestRunner);
 *   LongBowTestRunner *testRunner2 = LONGBOW_TEST_RUNNER_CREATE(MyOtherTestRunner);
 *
 *   int exitStatus = longBowMain(argc, argv, testRunner, testRunner2);
 *
 *   longBowTestRunner_Destroy(&testRunner1);
 *   longBowTestRunner_Destroy(&testRunner2);
 *   exit(exitStatus);
 * }
 * @endcode
 */
#ifdef LongBow_DISABLE_ASSERTIONS
/* 77 is the exit status for a skipped test when run with automake generated Makefiles */
#  define longBowMain(argc, argv, ...) \
    77
#else
#  define longBowMain(argc, argv, ...) \
    longBowMain_Impl(argc, argv, __VA_ARGS__, NULL)
#endif

/**
 * @brief Configure and run a set of `LongBowTestRunner` instances
 *
 * @param [in] argc The number of elements in the @p argv array.
 * @param [in] argv An NULL terminated list of pointers to `LongBowTestRunner` instances.
 *
 * Example Usage:
 * @code
 * int
 * main(int argc, char *argv[argc])
 * {
 *   LongBowTestRunner *testRunner1 = LONGBOW_TEST_RUNNER_CREATE(MyTestRunner);
 *   LongBowTestRunner *testRunner2 = LONGBOW_TEST_RUNNER_CREATE(MyOtherTestRunner);
 *
 *   int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, testRunner2);
 *
 *   longBowTestRunner_Destroy(&testRunner1);
 *   longBowTestRunner_Destroy(&testRunner2);
 *   exit(exitStatus);
 * }
 * @endcode
 */
#define LONGBOW_TEST_MAIN(argc, argv, ...) \
    longBowMain(argc, argv, __VA_ARGS__, NULL)

/**
 * @brief Skip this test case
 *
 * @param [in] ... A printf format string followed by a variable number of parameters corresponding to the format string.
 */
#define testSkip(...) do { \
        longBowTest(&LongBowTestSkippedEvent, "Skipped " __VA_ARGS__); \
} while (0)

/**
 * @brief Terminate the test indicating that the test is unimplemented.
 *
 * @param [in] ... A printf format string followed by a variable number of parameters corresponding to the format string.
 */
#define testUnimplemented(...) do { \
        longBowTest(&LongBowTestUnimplementedEvent, "Unimplemented test " __VA_ARGS__); \
} while (0)

/**
 * @brief Issue a warning and terminate with the `LONGBOW_TESTCASE_WARNED` code.
 *
 * @param [in] ... A printf format string followed by a variable number of parameters corresponding to the format string.
 */
#define testWarn(...) \
    do { longBowTest(&LongBowTestEvent, "Warning " __VA_ARGS__); } while (0)

/**
 * @brief assert the Equals Contract for the given function.
 *
 * @param [in] function The function under test.
 * @param [in] x The pivotal value which must not be NULL.
 * @param [in] y A value that must be perfectly equal to x and z, but neither x nor z.
 * @param [in] z A value that must be perfectly equal to x and y, but neither x nor y.
 * @param [in] ... A variable number of values that are unequal to x.
 */
#define assertEqualsContract(function, x, y, z, ...) \
    assertTrue(longBowUnitTesting_AssertEqualsContract((bool (*)(void *, void *))function, x, y, z, __VA_ARGS__, NULL), "Failed Equals Contract");

/**
 * @brief assert the Compare To Contract for the given function.
 *
 * @param [in] function The function under test.
 * @param [in] value The pivotal value under test.
 * @param [in] equality A NULL terminated array of values that are all equivalent to `value`.
 * @param [in] lesser A NULL terminated array of values that are all less than `value`.
 * @param [in] greater A NULL terminated array of values that are all greater than `value`.
 * @return `true` if the evalutation is successful.
 * @see assertCompareToContract()
 */
#define assertCompareToContract(function, value, equality, lesser, greater) \
    assertTrue(longBowUnitTesting_AssertCompareToContract((int (*)(const void *, const void *))function, value, (void *) equality, (void *) lesser, (void *) greater), "Failed CompareTo Contract");
#endif // UNIT_TEST_H_
