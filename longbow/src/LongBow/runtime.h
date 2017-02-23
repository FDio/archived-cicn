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
 * @file runtime.h
 * @ingroup runtime
 * @brief LongBow Runtime Support
 *
 */
#ifndef LongBow_runtime_h
#define LongBow_runtime_h

#include <signal.h>

#include <LongBow/longBow_Runtime.h>

/**
 * @def longBowIsFalse
 * @brief Indicate true if a condition is false.
 *
 * @param [in] condition The condition to test.
 */
#if __GNUC__
#    define longBowIsFalse(condition) __builtin_expect(!(condition), 0)
#else
#    define longBowIsFalse(condition) (!(condition))
#endif

#if __STDC_VERSION__ < 199901L
#    define __func__ ((const char *) 0)
#endif

/**
 * @def longBowEvent
 * @brief If the condition is true record the given event and abort the running programme.
 *
 * @param [in] eventPointer
 * @param [in] condition A boolean value that is expected to be true.
 * @param [in] location A pointer to a LongBowLocation instance.
 * @param [in] ... A printf format string following corresponding parameters.
 */
#ifdef LongBow_DISABLE_ASSERTIONS
#define longBowEvent(eventPointer, condition, location, ...) if (0 && condition) for (; false; )
#else
#define longBowEvent(eventPointer, condition, location, ...) \
    if (longBowRuntime_EventEvaluation(eventPointer) && longBowIsFalse(condition) && \
        longBowRuntime_EventTrigger(eventPointer, location, #condition, __VA_ARGS__)) \
        for (; true; longBowRuntime_Abort(), kill(0, SIGTRAP))
#endif

/**
 * @def longBowAssert
 * @brief Assert a condition, abort the running programme recording the given event if the condition is false.
 *
 * @param [in] eventPointer
 * @param [in] condition A boolean value that is expected to be true.
 * @param [in] ... A printf format string following corresponding parameters.
 */

#  define longBowAssert(eventPointer, condition, ...) \
    longBowEvent(eventPointer, condition, longBowLocation_Create(__FILE__, __func__, __LINE__), __VA_ARGS__)

/**
 * @def longBowTrap
 * @brief Abort the running programme recording the given trap.
 *
 * @param [in] eventPointer
 * @param [in] ... A printf format string following corresponding parameters.
 */
#define longBowTrap(eventPointer, ...) \
    longBowRuntime_EventEvaluation(eventPointer); \
    if (longBowRuntime_EventTrigger(eventPointer, \
                                    longBowLocation_Create(__FILE__, __func__, __LINE__), longBowEventType_GetName(eventPointer), __VA_ARGS__), true) \
        for (; true; abort())


#define longBowTrapIf(eventPointer, condition, ...) \
    longBowEvent(eventPointer, (!(condition)), longBowLocation_Create(__FILE__, __func__, __LINE__), __VA_ARGS__)

/**
 * @def longBowTest
 * @brief Terminate a LongBow Test Case signaling the given event if the condition is false.
 *
 * @param [in] testEventPointer
 * @param [in] ... A printf format string following corresponding parameters.
 */
# define longBowTest(testEventPointer, ...) do { \
        longBowRuntime_EventEvaluation(testEventPointer); \
        longBowRuntime_EventTrigger(testEventPointer, \
                                    longBowLocation_Create(__FILE__, __func__, __LINE__), \
                                    "Test", __VA_ARGS__); \
        longjmp(longBowTestCaseAbort, SIGABRT); \
} while (0)

#include <LongBow/assertions.h>
#include <LongBow/debugging.h>
#include <LongBow/traps.h>
#endif // LongBow_runtime_h
