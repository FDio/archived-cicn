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
 */

#include <config.h>
#include <time.h>
#include <parc/algol/parc_Clock.h>

#if __APPLE__
#include <mach/mach.h>
#include <mach/clock.h>
#include <mach/mach_time.h>
#endif

// These are used by the counter Clock
#include <parc/algol/parc_AtomicInteger.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
// ----

typedef struct counter_clock {
    uint64_t counter;
} _CounterClock;

parcObject_ExtendPARCObject(_CounterClock, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
static parcObject_ImplementAcquire(_counterClock, _CounterClock);

static PARCClock *
_counterClock_AcquireInterface(const PARCClock *clock)
{
    _CounterClock *cc = (_CounterClock *) clock->closure;
    _counterClock_Acquire(cc);
    return (PARCClock *) clock;
}

static void
_counterClock_ReleaseInterface(PARCClock **clockPtr)
{
    PARCClock *clock = *clockPtr;
    _CounterClock *cc = (_CounterClock *) clock->closure;

    PARCReferenceCount refcount = parcObject_Release((void **) &cc);
    if (refcount == 0) {
        parcMemory_Deallocate((void **) clockPtr);
    } else {
        *clockPtr = NULL;
    }
}

static void
_counterClock_GetTimeval(const PARCClock *clock, struct timeval *output)
{
    _CounterClock *cc = (_CounterClock *) clock->closure;
    uint64_t value = parcAtomicInteger_Uint64Increment(&cc->counter);

    // put 19 bits in the micro-seconds so it is never larger than 1E+6
    output->tv_sec = value >> 19;
    output->tv_usec = value & 0x7FFFF;
}

static uint64_t
_counterClock_GetTime(const PARCClock *clock)
{
    _CounterClock *cc = (_CounterClock *) clock->closure;
    return parcAtomicInteger_Uint64Increment(&cc->counter);
}

PARCClock *
parcClock_Counter(void)
{
    _CounterClock *cc = parcObject_CreateInstance(_CounterClock);
    cc->counter = 0;

    PARCClock *clock = parcMemory_Allocate(sizeof(PARCClock));
    clock->closure = cc;
    clock->acquire = _counterClock_AcquireInterface;
    clock->release = _counterClock_ReleaseInterface;
    clock->getTime = _counterClock_GetTime;
    clock->getTimeval = _counterClock_GetTimeval;
    return clock;
}

// ===========
// Wallclock

static void
_wallclock_GetTimeval(const PARCClock *dummy __attribute__((unused)), struct timeval *output)
{
#if __linux__
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    output->tv_sec = ts.tv_sec;
    output->tv_usec = ts.tv_nsec / 1000;
#else
    clock_serv_t clockService;
    mach_timespec_t mts;

    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &clockService);
    clock_get_time(clockService, &mts);
    mach_port_deallocate(mach_task_self(), clockService);

    output->tv_sec = mts.tv_sec;
    output->tv_usec = mts.tv_nsec / 1000;
#endif
}

static uint64_t
_wallclock_GetTime(const PARCClock *clock)
{
    struct timeval tv;
    _wallclock_GetTimeval(clock, &tv);
    uint64_t t = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return t;
}


static PARCClock *
_wallclock_Acquire(const PARCClock *clock)
{
    return (PARCClock *) clock;
}

static void
_wallclock_Release(PARCClock **clockPtr)
{
    *clockPtr = NULL;
}

static PARCClock _wallclock = {
    .closure    = NULL,
    .getTime    = _wallclock_GetTime,
    .getTimeval = _wallclock_GetTimeval,
    .acquire    = _wallclock_Acquire,
    .release    = _wallclock_Release
};

PARCClock *
parcClock_Wallclock(void)
{
    return &_wallclock;
}


// ==========================
// Monotonic clock

static void
_monoclock_GetTimeval(const PARCClock *dummy __attribute__((unused)), struct timeval *output)
{
#if __linux__
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    output->tv_sec = ts.tv_sec;
    output->tv_usec = ts.tv_nsec / 1000;
#else
    clock_serv_t clockService;
    mach_timespec_t mts;

    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clockService);
    clock_get_time(clockService, &mts);
    mach_port_deallocate(mach_task_self(), clockService);

    output->tv_sec = mts.tv_sec;
    output->tv_usec = mts.tv_nsec / 1000;
#endif
}

static uint64_t
_monoclock_GetTime(const PARCClock *clock)
{
    struct timeval tv;
    _monoclock_GetTimeval(clock, &tv);
    uint64_t t = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return t;
}

static PARCClock *
_monoclock_Acquire(const PARCClock *clock)
{
    return (PARCClock *) clock;
}

static void
_monoclock_Release(PARCClock **clockPtr)
{
    *clockPtr = NULL;
}

static PARCClock _monoclock = {
    .closure    = NULL,
    .getTime    = _monoclock_GetTime,
    .getTimeval = _monoclock_GetTimeval,
    .acquire    = _monoclock_Acquire,
    .release    = _monoclock_Release
};

PARCClock *
parcClock_Monotonic(void)
{
    return &_monoclock;
}

// ===========================
// Facade API

uint64_t
parcClock_GetTime(const PARCClock *clock)
{
    return clock->getTime(clock);
}

void
parcClock_GetTimeval(const PARCClock *clock, struct timeval *output)
{
    clock->getTimeval(clock, output);
}

PARCClock *
parcClock_Acquire(const PARCClock *clock)
{
    return clock->acquire(clock);
}

void
parcClock_Release(PARCClock **clockPtr)
{
    (*clockPtr)->release(clockPtr);
}

