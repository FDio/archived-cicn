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
 * @file parc_Timing.h
 * @ingroup developer
 * @brief Macros for timing code
 *
 * These macros allow the developer to measure time spent in sections of code.
 *    On Intel platforms (i386 or x86_64), the timing is done with the TSC counter, so it will be measured in CPU cycles.
 *    On non-Intel Linux platforms, it will be be done with the nano-second oscillator clock (CLOCK_MONOTONIC_RAW).
 *    On Darwin, it will use the nano-second SYSTEM_CLOCK.
 *    Otherwise, uses gettimeofday(), which will be micro-second timing.
 *
 * This set of headers will define several macros for timing:
 *    parcTiming_Init(prefix)
 *    parcTiming_Fini(prefix)
 *    parcTiming_Start(prefix)
 *    parcTiming_Stop(prefix)
 *    (uint64_t) parcTiming_Delta(prefix)
 *
 * The units returned from parcTiming_Delta() will be consistent, but not necessarily
 * related to wall clock time, real time, or any discernable time unit.  For example, they
 * may be in CPU instruction cycles or raw oscillator ticks or nano-seconds.
 *
 * These macros only work if the user defines PARCTIMING_ENABLE.  Otherwise, they do not
 * generate any instructions and parcTiming_Delta will always return 0.
 *
 * @code
 * #define PARCTIMING_ENABLE 1
 * #include <parc/developer/parc_Timing.h>
 *
 * void
 * foo(void)
 * {
 *    parcTiming_Init(_foo);
 *    // ... other stuff ..
 *    parcTiming_Start(_foo);
 *    // ... stuff to measure ...
 *    parcTiming_Stop(_foo);
 *
 *    uint64_t delta = parcTiming_Delta(_foo);
 *    parcTiming_Fini(_foo)
 * }
 * @endcode
 *
 */
#ifndef libparc_parc_Timing_h
#define libparc_parc_Timing_h

#if defined(PARCTIMING_ENABLE)
// begin platform detection
#if defined(__i386__) || defined(__x86_64__)
#define PARCTIMING_INTEL
#include <parc/developer/parc_TimingIntel.h>
#elif defined(__APPLE__)
#define PARCTIMING_DARWIN
#include <parc/developer/parc_TimingDarwin.h>
#elif defined(__linux__)
#define PARCTIMING_LINUX
#include <parc/developer/parc_TimingLinux.h>
#else
#define PARCTIMING_GENERIC
#include <parc/developer/parc_TimingGeneric.h>
#endif // platform detection
#else // PARCTIMING_ENABLE
#define _private_parcTiming_Init(prefix)
#define _private_parcTiming_Start(prefix)
#define _private_parcTiming_Stop(prefix)
#define _private_parcTiming_Delta(prefix) ((uint64_t) 0)
#define _private_parcTiming_Fini(prefix)
#endif // PARCTIMING_ENABLE

/**
 * Initialize the timing facility for namespace prefix `prefix`
 *
 * Used inside a code block, this macro will define several variables prefixed with
 * the name `prefix`.  It may also allocate memory or system resources.  It must be used
 * within a function scope, not at a global scope.
 *
 * You must use parcTiming_Fini(prefix) when done with the timing facility.
 *
 * If `PARCTIMING_ENABLE` is not defined before including the header file, this function is a no-op.
 *
 * @param [in] prefix The namespace prefix used for variables in scope.
 *
 * Example:
 * @code
 * #define PARCTIMING_ENABLE 1
 * #include <parc/developer/parc_Timing.h>
 *
 * void
 * foo(void)
 * {
 *    parcTiming_Init(_foo);
 *    parcTiming_Start(_foo);
 *    // ... stuff to measure ...
 *    parcTiming_Stop(_foo);
 *
 *    uint64_t delta = parcTiming_Delta(_foo);
 *    parcTiming_Fini(_foo)
 * }
 * @endcode
 */
#define parcTiming_Init(prefix) _private_parcTiming_Init(prefix)

/**
 * Marks the time in the start variable
 *
 * Records the current time in the start variable for use by parcTiming_Delta().
 *
 * If PARCTIMING_ENABLE is not defined before including the header file, this function is a no-op.
 *
 * @param [in] prefix The namespace prefix used for variables in scope.
 *
 * Example:
 * @code
 * #define PARCTIMING_ENABLE 1
 * #include <parc/developer/parc_Timing.h>
 *
 * void
 * foo(void)
 * {
 *    parcTiming_Init(_foo);
 *    parcTiming_Start(_foo);
 *    // ... stuff to measure ...
 *    parcTiming_Stop(_foo);
 *
 *    uint64_t delta = parcTiming_Delta(_foo);
 *    parcTiming_Fini(_foo)
 * }
 * @endcode
 */
#define parcTiming_Start(prefix) _private_parcTiming_Start(prefix)

/**
 * Marks the time in the stop variable
 *
 * Records the current time in the stop variable for use by parcTiming_Delta().
 *
 * If PARCTIMING_ENABLE is not defined before including the header file, this function is a no-op.
 *
 * @param [in] prefix The namespace prefix used for variables in scope.
 *
 * Example:
 * @code
 * #define PARCTIMING_ENABLE 1
 * #include <parc/developer/parc_Timing.h>
 *
 * void
 * foo(void)
 * {
 *    parcTiming_Init(_foo);
 *    parcTiming_Start(_foo);
 *    // ... stuff to measure ...
 *    parcTiming_Stop(_foo);
 *
 *    uint64_t delta = parcTiming_Delta(_foo);
 *    parcTiming_Fini(_foo)
 * }
 * @endcode
 */
#define parcTiming_Stop(prefix) _private_parcTiming_Stop(prefix)

/**
 * Returns the number of ticks between calls to start and stop.
 *
 * The delta will be in whatever units the best clock and provide.  It may be CPU cycles
 * or oscillator ticks, or nanos, or in the worst case micros.
 *
 * If PARCTIMING_ENABLE is not defined before including the header file, this function is a no-op.
 *
 * @param [in] prefix The namespace prefix used for variables in scope.
 *
 * @return uint64_t The number of ticks between start and stop
 *
 * Example:
 * @code
 * #define PARCTIMING_ENABLE 1
 * #include <parc/developer/parc_Timing.h>
 *
 * void
 * foo(void)
 * {
 *    parcTiming_Init(_foo);
 *    parcTiming_Start(_foo);
 *    // ... stuff to measure ...
 *    parcTiming_Stop(_foo);
 *
 *    uint64_t delta = parcTiming_Delta(_foo);
 *    parcTiming_Fini(_foo)
 * }
 * @endcode
 */
#define parcTiming_Delta(prefix) _private_parcTiming_Delta(prefix)

/**
 * Finalized the timing, releasing any system resources or memory
 *
 * Must be called when done with each namespace initialized by parcTiming_Init().
 *
 * If PARCTIMING_ENABLE is not defined before including the header file, this function is a no-op.
 *
 * @param [in] prefix The namespace prefix used for variables in scope.
 *
 * Example:
 * @code
 * #define PARCTIMING_ENABLE 1
 * #include <parc/developer/parc_Timing.h>
 *
 * void
 * foo(void)
 * {
 *    parcTiming_Init(_foo);
 *    parcTiming_Start(_foo);
 *    // ... stuff to measure ...
 *    parcTiming_Stop(_foo);
 *
 *    uint64_t delta = parcTiming_Delta(_foo);
 *    parcTiming_Fini(_foo)
 * }
 * @endcode
 */
#define parcTiming_Fini(prefix) _private_parcTiming_Fini(prefix)
#endif // libparc_parc_Timing_h

