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
 * @file longBow_Runtime.h
 * @ingroup runtime
 * @brief The LongBow Runtime support.
 *
 */
#ifndef LongBow_longBow_Runtime_h
#define LongBow_longBow_Runtime_h

#include <LongBow/longBow_Event.h>
#include <LongBow/longBow_RuntimeResult.h>
#include <LongBow/longBow_Config.h>

struct longbow_runtime;
typedef struct longbow_runtime LongBowRuntime;

/**
 * Create and return a new `LongBowRuntime` instance with the specified `LongBowRuntimeResult` instance.
 *
 * @param [in] expectedResultTemplate A `LongBowRuntimeResult` instance.
 *
 * @return A pointer to an allocated LongBowRuntime instance that must be deallocated via longBowRuntime_Destroy
 */
LongBowRuntime *longBowRuntime_Create(const LongBowRuntimeResult *expectedResultTemplate, LongBowConfig *config);

/**
 * Destroy the `LongBowRuntime` instance.
 *
 * @param [in,out] runtimePtr A pointer to a `LongBowRuntime` instance.
 */
void longBowRuntime_Destroy(LongBowRuntime **runtimePtr);

/**
 * Get the expected test case result from the `LongBowRuntime` instance.
 *
 * @param [in] runtime A `LongBowRuntime` instance.
 *
 * @return A pointer to the expected LongBowRuntimeResult.
 */
LongBowRuntimeResult *longBowRuntime_GetExpectedTestCaseResult(const LongBowRuntime *runtime);

/**
 * Get the actual test case result from the `LongBowRuntime` instance.
 *
 * @param [in] runtime A `LongBowRuntime` instance.
 *
 * @return A pointer to the actual LongBowRuntimeResult.
 */
LongBowRuntimeResult *longBowRuntime_GetActualTestCaseResult(const LongBowRuntime *runtime);

/**
 * Get the number of events that were evalutated.
 *
 * @param [in] runtime A `LongBowRuntime` instance.
 *
 * @return The number of events that were evalutated.
 */
size_t longBowRuntime_GetActualEventEvaluationCount(LongBowRuntime *runtime);

/**
 * Get the LongBowEventType of the given LongBowRuntime.
 *
 * @param [in] runtime A `LongBowRuntime` instance.
 *
 * @return The LongBowEventType of the given LongBowRuntime.
 */
LongBowEventType *longBowRuntime_GetActualEventType(const LongBowRuntime *runtime);

/**
 * Get the expected EventType from the given LongBowRuntime.
 *
 * When testing, a test may set a LongBowEventType that is expected to be triggered.
 * This function simply gets the expected LongBowEventType from the given LongBowRuntime instance.
 *
 * @param [in] runtime A pointer to a LongBowRuntime instance.
 *
 * @return The expected EventType in the LongBowRuntime.
 */
LongBowEventType *longBowRuntime_GetExpectedEventType(const LongBowRuntime *runtime);

/**
 * Set the "actual" LongBowEventType of the given LongBowRuntime.
 *
 * @param [in] runtime A `LongBowRuntime` instance.
 * @param [in] eventType A `LongBowEventType` instance.
 */
void longBowRuntime_SetActualEventType(LongBowRuntime *runtime, LongBowEventType *eventType);

/**
 * Set the current `LongBowRuntime`.
 *
 * @param [in] runtime A `LongBowRuntime` instance.
 *
 * @return The previous LongBowRuntime
 */
LongBowRuntime *longBowRuntime_SetCurrentRuntime(LongBowRuntime *runtime);

/**
 * Retrieve the current `LongBowRuntime`.
 *
 * @return The current global LongBowRuntime.
 */
LongBowRuntime *longBowRuntime_GetCurrentRuntime(void);

/**
 * Retrieve the `LongBowConfig` instance of the current global runtime.
 *
 * @return The `LongBowConfig` instance of the current global runtime.
 */
LongBowConfig *longBowRuntime_GetCurrentConfig(void);

/**
 * Set the `LongBowConfig` instance of the current global runtime.
 *
 * @param [in] config The new `LongBowConfig` instance of the current global runtime.
 */
void longBowRuntime_SetCurrentConfig(LongBowConfig *config);

/**
 * Trigger a LongBow event.
 *
 * The event will be reported via the longBowReport_Event.
 *
 * @param [in] eventType The type of event.
 * @param [in] location The LongBowLocation of the event (this will be destroyed).
 * @param [in] kind A string indicating the kind of event this is triggering.
 * @param [in] format A printf format string.
 * @param [in] ... Parameters associated with the printf format string.
 *
 * @return true Always return true.
 */
bool longBowRuntime_EventTrigger(LongBowEventType *eventType,
                                 LongBowLocation *location,
                                 const char *kind,
                                 const char *format, ...) __attribute__((__format__(__printf__, 4, 5)));
/**
 * Record an event evaluation.
 *
 * This only records the fact of the evaluation, not the results of the evaluation.
 *
 * @param [in] type A pointer to the LongBowEventType being evaluated.
 *
 * @return true Always returns true.
 *
 * Example:
 * @code
 * {
 *     longBowRuntime_EventEvaluation(&LongBowAssertEvent);
 * }
 * @endcode
 */
bool longBowRuntime_EventEvaluation(const LongBowEventType *type);

/**
 * Set the current value for the depth of  printed stack trace.
 *
 *   If the depth is less than 1, no stack trace is displayed.
 *
 * @param [in] newDepth The new value to set.
 *
 * @return The previous value.
 */
unsigned int longBowRuntime_SetStackTraceDepth(unsigned int newDepth);

/**
 * Get the current value for the depth of printed stack trace.
 *
 * @return The current stack-trace depth.
 */
unsigned int longBowRuntime_GetStackTraceDepth(void);

/**
 * Print a formatted stack trace to the current output file descriptor.
 *
 * @param [in] fileDescriptor A valid file descriptor.
 */
void longBowRuntime_StackTrace(int fileDescriptor);

/**
 * Abort the running process using the current runtime environment.
 * If the configuration is set to produce a core dump, this function simply returns.
 * This permits the caller to use the form:
 * <code>
 * longBowRuntime_Abort(), kill(0, SIGTRACE)
 * </code>
 * To generate a core image.
 *
 * See the assertion macros for how this is used.
 */
void longBowRuntime_Abort(void);
#endif // LongBow_longBow_Runtime_h
