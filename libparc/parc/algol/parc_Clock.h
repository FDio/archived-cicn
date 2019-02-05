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
 * @file parc_Clock.h
 * @ingroup datastructures
 * @brief Generic API to a clock
 *
 * Interface over clock providers.  We provide two system clocks, a Wallclock that tracks the
 * real time clock and a monotonic clock that will not skew or go backwards.
 * @see parcClock_Monotonic()
 * and
 * @see parcClock_Wallclock()
 * Also provided is a counting clock.
 * @see parcClock_Counter()
 *
 * Below is a complete example of a simple custom clock that implements an atomic counter for
 * the time.  Each call to getTime or getTimeval will increment the counter.  This clock is
 * available as parcClock_Counter().
 *
 * @code
 *   typedef struct counter_clock {
 *      uint64_t counter;
 *   } _CounterClock;
 *
 *   parcObject_ExtendPARCObject(_CounterClock, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
 *   static parcObject_ImplementAcquire(_counterClock, _CounterClock);
 *
 *   static PARCClock *
 *   _counterClock_AcquireInterface(const PARCClock *clock)
 *   {
 *      _CounterClock *cc = (_CounterClock *) clock->closure;
 *      _counterClock_Acquire(cc);
 *      return (PARCClock *) clock;
 *   }
 *
 *   static void
 *   _counterClock_ReleaseInterface(PARCClock **clockPtr)
 *   {
 *      PARCClock *clock = *clockPtr;
 *      _CounterClock *cc = (_CounterClock *) clock->closure;
 *
 *      PARCReferenceCount refcount = parcObject_Release((void **) &cc);
 *      if (refcount == 0) {
 *         parcMemory_Deallocate((void **) clockPtr);
 *      } else {
 *         *clockPtr = NULL;
 *      }
 *   }
 *
 *   static void
 *   _counterClock_GetTimeval(const PARCClock *clock, struct timeval *output)
 *   {
 *      _CounterClock *cc = (_CounterClock *) clock->closure;
 *      uint64_t value = parcAtomicInteger_Uint64Increment(&cc->counter);
 *      // put 19 bits in the micro-seconds so it is never larger than 1E+6
 *      output->tv_sec = value >> 19;
 *      output->tv_usec = value & 0x7FFFF;
 *   }
 *
 *   static uint64_t
 *   _counterClock_GetTime(const PARCClock *clock)
 *   {
 *      _CounterClock *cc = (_CounterClock *) clock->closure;
 *      return parcAtomicInteger_Uint64Increment(&cc->counter);
 *   }
 *
 *   PARCClock *
 *   parcClock_Counter(void)
 *   {
 *      _CounterClock *cc = parcObject_CreateInstance(_CounterClock);
 *      cc->counter = 0;
 *
 *      PARCClock *clock = parcMemory_Allocate(sizeof(PARCClock));
 *      clock->closure = cc;
 *      clock->acquire = _counterClock_AcquireInterface;
 *      clock->release = _counterClock_ReleaseInterface;
 *      clock->getTime = _counterClock_GetTime;
 *      clock->getTimeval = _counterClock_GetTimeval;
 *      return clock;
 *   }
 * @encode
 *
 */

#ifndef PARC_parc_Clock_h
#define PARC_parc_Clock_h

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <inttypes.h>

struct parc_clock;
typedef struct parc_clock PARCClock;

struct parc_clock {
    /**
     * Opaque parameter set by the clock provider
     */
    void *closure;

    /**
     * Gets the clock time
     *
     * The resolution and epoch of the clock are determined by the clock provider
     *
     * @param [in] clock An allocated PARCClock
     *
     * @retval number The clock's time as a uint64_t
     *
     * Example:
     * @code
     * {
     *     PARCClock *clock = parcClock_Monotonic();
     *     uint64_t t = parcClock_GetTime(clock);
     *     parcClock_Release(&clock);
     * }
     * @endcode
     */
    uint64_t (*getTime)(const PARCClock *clock);

    /**
     * Gets the clock time as a struct timeval
     *
     * The resolution and epoch of the clock are determined by the clock provider.
     * There may be an arbitrary mapping to the struct timeval as per the clock
     * provider, so the units of 'seconds' and 'micro seconds' need to be interpreted
     * as per the clock provider.
     *
     * @param [in] clock An allocated PARCClock
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    void (*getTimeval)(const PARCClock *clock, struct timeval *output);

    /**
     * Increase the number of references to a `PARCClock`.
     *
     * Note that new `PARCClock` is not created,
     * only that the given `PARCClock` reference count is incremented.
     * Discard the reference by invoking `parcClock_Release`.
     *
     * @param [in] clock A pointer to a `PARCClock` instance.
     *
     * @return The input `PARCClock` pointer.
     *
     * Example:
     * @code
     * {
     *     PARCClock *clock = parcClock_Counter();
     *     PARCClock *copy = parcClock_Acquire(clock);
     *     parcClock_Release(&copy);
     *     parcClock_Release(&clock);
     *
     * }
     * @endcode
     */
    PARCClock * (*acquire)(const PARCClock *clock);

    /**
     * Release a previously acquired reference to the specified instance,
     * decrementing the reference count for the instance.
     *
     * The pointer to the instance is set to NULL as a side-effect of this function.
     *
     * If the invocation causes the last reference to the instance to be released,
     * the instance is deallocated and the instance's implementation will perform
     * additional cleanup and release other privately held references.
     *
     * @param [in,out] clockPtr A pointer to a pointer to the instance to release.
     *
     * Example:
     * @code
     * {
     *     PARCClock *clock = parcClock_Counter();
     *     PARCClock *copy = parcClock_Acquire(clock);
     *     parcClock_Release(&copy);
     *     parcClock_Release(&clock);
     *
     * }
     * @endcode
     */

    void (*release)(PARCClock **clockPtr);
};

/**
 * A clock provider for the wall clock time
 *
 * This will use clock_gettime(CLOCK_REALTIME_COARSE) on linux or
 * host_get_clock_service with CALENDAR_CLOCK on Mac
 *
 * @retval non-null An allocated PARCClock, which must be released via parcClock_Release.
 *
 * Example:
 * @code
 * {
 *     PARCClock *clock = parcClock_Wallclock();
 *     uint64_t t = parcClock_GetTime(clock);
 *     parcClock_Release(&clock);
 * }
 * @endcode
 */
PARCClock *parcClock_Wallclock(void);

/**
 * A monotonic clock that will not normally adjust for time changes
 *
 * On linux, this uses the CLOCK_MONOTONIC_RAW.  On Darwin, it uses
 * the SYSTEM_CLOCK from <mach/mach_time.h>.
 *
 * @retval non-null An allocated PARCClock, which must be released via parcClock_Release.
 *
 * Example:
 * @code
 * {
 *     PARCClock *clock = parcClock_Monotonic();
 *     uint64_t t = parcClock_GetTime(clock);
 *     parcClock_Release(&clock);
 * }
 * @endcode
 */
PARCClock *parcClock_Monotonic(void);


/**
 * The counter clock begins at 0 and increments for every call to getTime or getTimeval
 *
 * Each allocated counter clock will begin at zero.  Copies made via parcClock_Acquire() will
 * share the same counter and use atomic updates.
 *
 * getTime() will return the counter.
 *
 * getTimeval() will return the lower 19 bits in tv_usec (so it does not overflow the concept
 * of micro-second) and the upper 45 bits are in tv_sec.  On some platforms, that may overflow.
 *
 * @retval non-null An allocated PARCClock, which must be released via parcClock_Release.
 *
 * Example:
 * @code
 * {
 *     PARCClock *clock = parcClock_Counter();
 *     uint64_t t = parcClock_GetTime(clock);
 *     parcClock_Release(&clock);
 * }
 * @endcode
 */
PARCClock *parcClock_Counter(void);

/**
 * Returns the clock provider's idea of the current time as a uint64
 *
 * Returns the clock provider's idea of the current time, which may or
 * may not be a wall clock time.
 *
 * @param [in] clock A clock provider
 *
 * @retval number The current time (depends on provider)
 *
 * Example:
 * @code
 * {
 *     PARCClock *clock = parcClock_Counter();
 *     uint64_t t = parcClock_GetTime(clock);
 *     parcClock_Release(&clock);
 * }
 * @endcode
 */
uint64_t parcClock_GetTime(const PARCClock *clock);

/**
 * Returns the clock provider's idea of the current time as a timeval
 *
 * Returns the clock provider's idea of the current time, which may or
 * may not be a wall clock time.
 *
 * @param [in] clock A clock provider
 * @param [in] output The structure to fill in with the current time
 *
 * Example:
 * @code
 * {
 *     struct timeval t;
 *     PARCClock *clock = parcClock_Counter();
 *     parcClock_GetTimeval(clock, &t);
 *     parcClock_Release(&clock);
 * }
 * @endcode
 */
void parcClock_GetTimeval(const PARCClock *clock, struct timeval *output);

/**
 * Increase the number of references to a `PARCClock`.
 *
 * Note that new `PARCClock` is not created,
 * only that the given `PARCClock` reference count is incremented.
 * Discard the reference by invoking `parcClock_Release`.
 *
 * @param [in] clock A pointer to a `PARCClock` instance.
 *
 * @return The input `PARCClock` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCClock *clock = parcClock_Counter();
 *     PARCClock *copy = parcClock_Acquire(clock);
 *     parcClock_Release(&copy);
 *     parcClock_Release(&clock);
 *
 * }
 * @endcode
 */
PARCClock *parcClock_Acquire(const PARCClock *clock);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] clockPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCClock *clock = parcClock_Counter();
 *     PARCClock *copy = parcClock_Acquire(clock);
 *     parcClock_Release(&copy);
 *     parcClock_Release(&clock);
 *
 * }
 * @endcode
 */
void parcClock_Release(PARCClock **clockPtr);
#endif // PARC_parc_Clock_h
