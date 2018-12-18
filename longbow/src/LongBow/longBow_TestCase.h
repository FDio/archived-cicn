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
 * @file longBow_TestCase.h
 * @ingroup internals
 * @brief The interface and supporting functionality of a LongBow Test Case.
 *
 */
#ifndef LONGBOWTESTCASE_H_
#define LONGBOWTESTCASE_H_

#include <setjmp.h>

struct longbow_testcase;
typedef struct longbow_testcase LongBowTestCase;

#include <LongBow/longBow_TestCaseClipBoard.h>
#include <LongBow/longBow_ClipBoard.h>

#include <LongBow/longBow_TestRunner.h>
#include <LongBow/longBow_TestFixture.h>
#include <LongBow/longBow_RuntimeResult.h>
#include <LongBow/longBow_TestCaseMetaData.h>

typedef void (LongBowTestCaseFunction)(const LongBowTestRunner *, const LongBowTestFixture *, const LongBowTestCase *, const LongBowClipBoard *c, jmp_buf);

/**
 * Get the name of the given LongBowTestCase.
 *
 * @param [in] testCase A pointer to a `LongBowTestCase` instance.
 *
 * @return A pointer to an immutable C string.
 *
 * Example:
 * @code
 * const char *name = longBowTestCase_GetName(testCase);
 * @endcode
 */
const char *longBowTestCase_GetName(const LongBowTestCase *testCase);

/**
 * Get the corresponding LongBowTestFixture for the given LongBowTestCase.
 *
 * @param [in] testCase A pointer to a LongBowTestCase instance.
 *
 * @return A pointer to the corresponding LongBowTestFixture.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
LongBowTestFixture *longBowTestCase_GetFixture(const LongBowTestCase *testCase);

/**
 * Create a LongBowTestCase instance.
 *
 * @param [in] testCaseName A nul-terminated C string of the test case name.
 * @param [in] fixture A pointer to a valid LongBowTestFixture instance.
 * @param [in] testCase A pointer to a test case function.
 * @param [in] metaData A pointer to a LongBowTestCaseMetaData instance.
 * @return A pointer to an allocated LongBowTestCase instance, that must be destroyed via longBowTestCase_Destroy().
 */
LongBowTestCase *longBowTestCase_Create(const char *testCaseName,
                                        const LongBowTestFixture *fixture,
                                        LongBowTestCaseFunction *testCase,
                                        const LongBowTestCaseMetaData  *metaData);

/**
 * Print command line and configuration help applicable to a Long Bow Test Case.
 *
 */
void longBowTestCase_ConfigHelp(void);

/**
 * @param [in,out] testCasePtr A pointer to a pointer to a LongBowTestCase instance.
 */
void longBowTestCase_Destroy(LongBowTestCase **testCasePtr);

/**
 * Get the fully qualified name of the given `LongBowTestCase`.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 *
 * @return A constant nul-terminated, C string.
 */
const char *longBowTestCase_GetFullName(const LongBowTestCase *testCase);

/**
 *
 * @param [in] testCaseName A nul-terminated C string of the test case name.
 * @param [in] fixture A pointer to a valid LongBowTestFixture instance.
 * @param [in] testCase A pointer to a test case function.
 * @param [in] testCaseMetaData A pointer to a LongBowTestCaseMetaData instance.
 * @return return
 */
LongBowTestCase *longBowTestCase_Run(const char *testCaseName,
                                     const LongBowTestFixture *fixture,
                                     LongBowTestCaseFunction *testCase,
                                     const LongBowTestCaseMetaData *testCaseMetaData);

/**
 * Get the string representation of the given LongBowTestCase.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return An allocated, nul-terminated C string that must be deallocated via free(3)
 */
char *longBowTestCase_ToString(const LongBowTestCase *testCase);

/**
 * Return the LongBowStatus of the given test case.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return The LongBowStatus of the given test case.
 */
LongBowStatus longBowTestCase_GetStatus(const LongBowTestCase *testCase);

/**
 * Get the LongBowStatus value for the expected status of the given LongBowTestCase.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return A LongBowStatus value for the expected status of the given LongBowTestCase.
 */
LongBowStatus longBowTestCase_GetExpectedStatus(const LongBowTestCase *testCase);

/**
 * Get a pointer to the expected LongBowRuntimeResult value for the given LongBowTestCase.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return A LongBowStatus value for the expected status of the given LongBowTestCase.
 */
LongBowRuntimeResult *longBowTestCase_GetExpectedResult(const LongBowTestCase *testCase);

/**
 * Get the count of event evaluations performed during the execution of a LongBow Test Case.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 *
 * @return The count of event evaluations performed during the execution of a LongBow Test Case.
 */
size_t longBowTestCase_GetEventEvaluationCount(const LongBowTestCase *testCase);

/**
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return A LongBowRuntimeResult pointer.
 */
LongBowRuntimeResult *longBowTestCase_GetActualResult(const LongBowTestCase *testCase);

/**
 * Return <code>true</code> if the given test case was successful.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return <code>true</code> if the given test case succeeded.
 */
bool longBowTestCase_IsSuccessful(const LongBowTestCase *testCase);

/**
 * Return <code>true</code> if the given test case issued a warning.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return <code>true</code> if the given test case issued a warning.
 */
bool longBowTestCase_IsWarning(const LongBowTestCase *testCase);

/**
 * Return <code>true</code> if the given test case was incomplete.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return <code>true</code> if the given test case was incomplete.
 */
bool longBowTestCase_IsIncomplete(const LongBowTestCase *testCase);

/**
 *  Return <code>true</code> if the given test case failed.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 * @return <code>true</code> if the given test case failed.
 */
bool longBowTestCase_IsFailed(const LongBowTestCase *testCase);

///**
// * Place a value on the Test Case "clipboard"
// *
// * Every Test Case has an associated "clipboard" which is shared between the Test Fixture Setup,
// * Test Case, and Test Fixture Tear Down.
// *
// * @param [in] testCase A pointer to a valid LongBowTestCase instance.
// * @param [in] data The value to share on the clipboard.
// * @return The previous value on the "clipboard", or NULL if no previous value was set.
// */
//void *longBowTestCase_SetClipBoard(const LongBowTestCase *testCase, const char *name, void *data);

///**
// * Get the named clipboard data from the given `LongBowTestCase`.
// *
// * @param [in] testCase A pointer to a valid LongBowTestCase instance.
// * @param [in] name A nul-terminate, C string of the name of the clipboard entry.
// */
//void *longBowTestCase_GetClipBoard(const LongBowTestCase *testCase, const char *name);

/**
 * Get the LongBowConfig instance for the given LongBowTestCase instance.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 *
 * @return non-NULL The LongBowConfig instance for the given LongBowTestCase instance.
 * @return NULL No LongBowConfig instance for the given LongBowTestCase.
 */
LongBowConfig *longBowTestCase_GetConfiguration(const LongBowTestCase *testCase);

/**
 * Get the clipboard data from the given `LongBowTestCase`.
 *
 * NOTE: Use `longBowTestCase_GetClipBoard`.
 *
 * @param [in] testCase A pointer to a valid LongBowTestCase instance.
 */
void *longBowTestCase_GetClipBoardData(const LongBowTestCase *testCase);

void *longBowTestCase_SetClipBoardData(const LongBowTestCase *testCase, void *data);

void *longBowTestCase_Set(const LongBowTestCase *testCase, const char *name, void *value);

void *longBowTestCase_Get(const LongBowTestCase *testCase, const char *name);

char *longBowClipBoard_GetCString(const LongBowTestCase *testCase, const char *name);

void *longBowTestCase_SetInt(const LongBowTestCase *testCase, const char *name, int value);

void *longBowTestCase_SetCString(const LongBowTestCase *testCase, const char *name, char *value);

int longBowTestCase_GetInt(const LongBowTestCase *testCase, const char *name);
#endif // LONGBOWTESTCASE_H_
