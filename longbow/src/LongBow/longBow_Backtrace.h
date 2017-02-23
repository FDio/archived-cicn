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
 * @file longBow_Backtrace.h
 * @ingroup internals
 * @brief Support for Stack Traces
 *
 */
#ifndef LongBow_longBow_Backtrace_h
#define LongBow_longBow_Backtrace_h
#include <stdint.h>

struct longbow_backtrace;
/**
 * @typedef LongBowBacktrace
 * @brief A Backtrace representation.
 */
typedef struct longbow_backtrace LongBowBacktrace;

/**
 * Create a `LongBowBacktrace`.
 *
 * The backtrace includes depth number of elements from the stack.
 *
 * @param [in] depth  The number of elements from the stack to include.
 * @param [in] offset The offset of the stack to start the Backtrace.
 * @return A pointer to an allocated LongBowBacktrace instance.
 */
LongBowBacktrace *longBowBacktrace_Create(uint32_t depth, uint32_t offset);

/**
 * Get the array of symbols from the given `LongBowBacktrace instance`.
 *
 * @param [in] backtrace A pointer to a valid LongBowBacktrace instance.
 *
 * @return An array of nul-terminated, C strings each containing a symbolic representatino of the corresponding stack frame.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
char **longBowBacktrace_Symbols(const LongBowBacktrace *backtrace);
/**
 * Get the number of frames in the given LongBowBacktrace instance.
 *
 * @param [in] backtrace A pointer to a valid LongBowBacktrace instance.
 *
 * @return The number of frames in the LongBowBacktrace.
 */
unsigned int longBowBacktrace_GetFrameCount(const LongBowBacktrace *backtrace);

/**
 *
 * @param [in,out] backtracePtr A pointer to a pointer to a valid LongBowBacktrace instance.
 */
void longBowBacktrace_Destroy(LongBowBacktrace **backtracePtr);

/**
 *
 * @param [in] backtrace A pointer to a valid LongBowBacktrace instance.
 * @return An allocated C string that must be deallocated via longBowMemory_Deallocate().
 */
char *longBowBacktrace_ToString(const LongBowBacktrace *backtrace);
#endif
