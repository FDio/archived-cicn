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
 * @file longBow_RuntimeResult.h
 * @ingroup internals
 * @brief LongBow Test Case Results
 *
 * LongBow Test Cases have expected and actual results.
 * The expected results are typically a statically created instance of {@link LongBowRuntimeResult}
 * which is used when the Test Case is executed to compare with the actual results.
 * This permits, for example, a Test Case to indicate that it is expected to induce a specific LongBowEvent.
 * In which case, the actual LongBowEvent must equal the expected event for the Test Case to be considered a success.
 *
 */
#ifndef LongBow_longBow_CaseResult_h
#define LongBow_longBow_CaseResult_h

#include <stdlib.h>
#include <stdint.h>
#include <sys/resource.h>

#include <LongBow/longBow_Status.h>
#include <LongBow/longBow_Event.h>

struct longbow_testcase_result;

/**
 * @typedef LongBowRuntimeResult
 * @brief The expected and actual result of a LongBow Test.
 */
typedef struct longbow_testcase_result LongBowRuntimeResult;

/**
 * @struct longbow_testcase_result
 * @brief The expected and actual result of a LongBow Test.
 */
struct longbow_testcase_result {
    /**
     * The number of event evaluations performed.
     * These asserts and traps with conditions that were evaluated.
     */
    size_t eventEvaluationCount;
    LongBowStatus status;           /**< The resulting status of the test case. */
    struct timeval elapsedTime;     /**< The elapsed time of the test case. */
    struct rusage resources;        /**< The resulting resource usage of the test case. */
    LongBowEventType *event;        /**< The expected or actual event. */
};

/**
 * Return the event evaluation count associated with the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 *
 * @return The number of Event evaluations.
 */
size_t longBowRuntimeResult_GetEventEvaluationCount(const LongBowRuntimeResult *testCaseResult);

/**
 * Retrieve the event type associated with the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 *
 * @return The LongBowEventType for the given LongBowRuntimeResult.
 */
LongBowEventType *longBowRuntimeResult_GetEvent(const LongBowRuntimeResult *testCaseResult);

/**
 * Set the event type associated with the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 * @param [in] eventType A new `LongBowEventType` instance.
 */
void longBowRuntimeResult_SetEvent(LongBowRuntimeResult *testCaseResult, LongBowEventType *eventType);

/**
 * Get the LongBowStatus type from the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 *
 * @return The LongBowStatus of the given LongBowRuntimeResult.
 */
LongBowStatus longBowRuntimeResult_GetStatus(const LongBowRuntimeResult *testCaseResult);

/**
 * Set the LongBowStatus type for the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 * @param [in] status A `LongBowStatus` value.
 */
void longBowRuntimeResult_SetStatus(LongBowRuntimeResult *testCaseResult, LongBowStatus status);

/**
 * Set the elapsed time for the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 * @param [in] elapsedTime A `struct timeval` instance.
 */
void longBowRuntimeResult_SetElapsedTime(LongBowRuntimeResult *testCaseResult, struct timeval *elapsedTime);

/**
 * Get the elapsed time associated with the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 *
 * @return A copy of the timeval of the given LongBowRuntimeResult.
 */
struct timeval longBowRuntimeResult_GetElapsedTime(const LongBowRuntimeResult *testCaseResult);

/**
 * Retrieve the RUsage struct from the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 *
 * @return A pointer to the struct rusage instance in the given LongBowRuntimeResult.
 */
struct rusage *longBowRuntimeResult_GetRUsage(LongBowRuntimeResult *testCaseResult);

/**
 * Set the RUsage struct for the given `LongBowRuntimeResult` instance.
 *
 * @param [in] testCaseResult A `LongBowRuntimeResult` instance.
 * @param [in] rusage A `struct rusage` instance.
 */
void longBowRuntimeResult_SetRUsage(LongBowRuntimeResult *testCaseResult, struct rusage *rusage);
#endif
