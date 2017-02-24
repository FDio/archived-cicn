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
 * @ingroup threading
 *
 */
#ifndef parc_Timeout_h
#define parc_Timeout_h
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t PARCTimeout;

/**
 * @def PARCTimeout_Never
 * This represents a timeout that will never happen.
 */
#define PARCTimeout_Never NULL

/*
 * @def PARCTimeout_Immediate
 * This represents a timeout that immediately happens.
 * Equivalent to parcTimeout_NanoSeconds(0)
 */
#define PARCTimeout_Immediate (&(PARCTimeout) { 0 })

/*
 * @def PARCTimeout_NanoSeconds
 * This represents a timeout that will occur in the specified number of nanoseconds.
 */
#define parcTimeout_NanoSeconds(_nsec_) (&(PARCTimeout) { _nsec_ })

/*
 * @def PARCTimeout_MicroSeconds
 * This represents a timeout that will occur in the specified number of microseconds.
 */
#define parcTimeout_MicroSeconds(_usec_) parcTimeout_NanoSeconds(_usec_ * 1000)

/*
 * @def PARCTimeout_MilliSeconds
 * This represents a timeout that will occur in the specified number of microseconds.
 */
#define parcTimeout_MilliSeconds(_msec_) parcTimeout_MicroSeconds(_msec_ * 1000)

/**
 * Determine if two `PARCTimeout` instances are equal.
 *
 * The following equivalence relations on non-null `PARCTimeout` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcTimeout_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcTimeout_Equals(x, y)` must return true if and only if
 *        `parcTimeout_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcTimeout_Equals(x, y)` returns true and
 *        `parcTimeout_Equals(y, z)` returns true,
 *        then `parcTimeout_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcTimeout_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcTimeout_Equals(x, NULL)` must return false.
 *
 * @param [in] x A valid PARCTimeout instance.
 * @param [in] y A valid PARCTimeout instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCTimeout *a = parcTimeout_Create();
 *     PARCTimeout *b = parcTimeout_Create();
 *
 *     if (parcTimeout_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcTimeout_Release(&a);
 *     parcTimeout_Release(&b);
 * }
 * @endcode
 * @see parcTimeout_HashCode
 */
bool parcTimeout_Equals(PARCTimeout x, PARCTimeout y);

/**
 * Predicate function returning true if the given timeout represents an infinite delay.
 *
 * @param [in] timeout A pointer to a valid PARCTimeout value.
 *
 * @return true  The given timeout represents an infinite delay.
 */
static inline bool
parcTimeout_IsNever(const PARCTimeout *timeout)
{
    return (timeout == PARCTimeout_Never);
}

/**
 * Predicate function returning true if the given timeout represents an immediate, no-delay timeout.
 *
 * @param [in] timeout A pointer to a valid PARCTimeout value.
 *
 * @return true  The given timeout represents an immediate, no-delay timeout.
 */
static inline bool
parcTimeout_IsImmediate(const PARCTimeout *timeout)
{
    return parcTimeout_IsNever(timeout) ? false : (*timeout == 0);
}

/**
 * Return the number of nano-seconds in the given PARCTimeout instance.
 *
 * If the PARCTimeout is never (`parcTimeout_IsNever` returns `true`), the returned value is UINT64_MAX.
 *
 * @param [in] timeout A pointer to a valid PARCTimeout value.
 *
 * @return The number of nano-seconds in the given PARCTimeout instance, UINT64_MAX.
 */
static inline uint64_t
parcTimeout_InNanoSeconds(const PARCTimeout *timeout)
{
    return parcTimeout_IsNever(timeout) ? UINT64_MAX : *timeout;
}
#endif /* parc_Timeout_h */
