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
 * @file longBow_UnitTest.h
 * @ingroup testing
 * @brief Unit Testing Support.
 *
 */
#ifndef __LongBow__longBow_UnitTest__
#define __LongBow__longBow_UnitTest__

/**
 * Compose the C function name prefix for a Test Runner.
 *
 * @param [in] _testRunnerName_ A valid identifier for the Test Runner.
 */
#define longBowUnitTest_RunnerName(_testRunnerName_) \
    LongBowTestRunner_ ## _testRunnerName_

/**
 * Compose the C function name for a Test Runner Setup function.
 *
 * @param [in] _testRunnerName_ A valid identifier for the Test Runner.
 */
#define longBowUnitTest_RunnerSetupName(_testRunnerName_) \
    LongBowTestRunner_ ## _testRunnerName_ ## _Setup

/**
 * @brief Compose the C function name for a Test Runner Tear-Down function.
 *
 * @param [in] _testRunnerName_ A valid identifier for the Test Runner.
 */
#define longBowUnitTest_RunnerTearDownName(_testRunnerName_) \
    LongBowTestRunner_ ## _testRunnerName_ ## _TearDown

/**
 * @brief Compose the C function name for a Test Runner function.
 *
 * @param [in] _testRunnerName_ A valid identifier for the Test Runner.
 */
#define longBowUnitTest_RunnerRunName(_testRunnerName_) \
    LongBowTestRunner_ ## _testRunnerName_ ## _Run

/**
 * @defined longBowUnitTest_FixtureName
 * @brief Compose a Test Fixture name.
 * @param [in] _testFixtureName_ A valid identifier fragment for the Test Fixture.
 */
#define longBowUnitTest_FixtureName(_testFixtureName_) \
    LongBowTestFixture_ ## _testFixtureName_

/**
 * @brief Compose the C function name for a Test Fixture function.
 *
 * @param [in] _testFixtureName_ A valid identifier for the Test Fixture.
 */
#define longBowUnitTest_FixtureRunName(_testFixtureName_) \
    LongBowTestFixture_ ## _testFixtureName_ ## _Run

/**
 * Compose the C function name for a Test Fixture Setup function.
 *
 * @param [in] _testFixtureName_ A valid identifier for the Test Fixture.
 */
#define longBowUnitTest_FixtureSetupName(_testFixtureName_) \
    LongBowTestFixture_ ## _testFixtureName_ ## _Setup

/**
 * @brief Compose the C function name for a Test Fixture Tear-Down function.
 *
 * @param [in] _testFixtureName_ A valid identifier for the Test Fixture.
 */
#define longBowUnitTest_FixtureTearDownName(_testFixtureName_) \
    LongBowTestFixture_ ## _testFixtureName_ ## _TearDown

/**
 * @brief Compose the C name for a Test Fixture Configuration structure.
 *
 * @param [in] _testFixtureName_ A valid identifier for the Test Fixture.
 */
#define longBowUnitTest_FixtureConfigName(_testFixtureName_) \
    LongBowTestFixture_ ## _testFixtureName_ ## _Config

/**
 *
 */
#define longBowUnitTest_CaseName(_testFixtureName_, _testCaseName_) \
    LongBowTestCase_ ## _testFixtureName_ ## _ ## _testCaseName_

#endif /* defined(__LongBow__longBow_UnitTest__) */
