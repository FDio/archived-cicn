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
 * @file longBowReport_Testing.h
 * @ingroup reporting
 * @brief The LongBow Test Report Generator.
 *
 *    This header specifies the interface for an implementation of a LongBow Test Report generator.
 *    Different implementations of a Test Report generator are used to connect to external environments to hook in
 *	  LongBow unit tests within a larger framework like an IDE or continuous integration system.
 *
 */
#ifndef LONGBOW_REPORT_TESTING_H_
#define LONGBOW_REPORT_TESTING_H_

#include <LongBow/Reporting/longBowReport_Runtime.h>

#include <LongBow/runtime.h>
#include <LongBow/unit-test.h>

/**
 * Produce a summary report for the given LongBowTestRunner.
 *
 * @param [in] testRunner A pointer to a valid LongBowTestRunner instance.
 * @return The given LongBowTestRunner.
 */
const LongBowTestRunner *longBowReportTesting_TestRunner(const LongBowTestRunner *testRunner);

/**
 * Produce a summary report for the given LongBowTestFixture.
 *
 * @param [in] testFixture A pointer to a LongBowTestFixture instance.
 * @return The given LongBowTestFixture.
 */
const LongBowTestFixture *longBowReportTesting_TestFixture(const LongBowTestFixture *testFixture);

/**
 * Produce a summary report for the given LongBowTestCase.
 *
 * @param [in] testCase A pointer to a LongBowTestCase instance.
 * @return The pointer to the given LongBowTestCase.
 */
const LongBowTestCase *longBowReportTesting_TestCase(const LongBowTestCase *testCase);

/**
 * Produce a single character displaying the status of an individual test case.
 *
 * @param [in] testCase A pointer to a LongBowTestCase instance.
 */
void longBowReportTesting_DisplayTestCaseResult(const LongBowTestCase *testCase);

/**
 * Make a trace report.
 *
 * @param [in] format A printf-style format string.
 */
void longBowReportTesting_Trace(const char *restrict format, ...);
#endif // LONGBOW_REPORT_TESTING_H_
