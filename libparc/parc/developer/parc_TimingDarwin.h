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
 * @file parc_TimingDarwin.h
 * @brief Macros for timing code
 *
 * On linux will use clock_gettime with the MONOTONIC RAW clock, which does not speed up or slow down based on adj_time().
 *
 */
#ifndef libparc_parc_TimingDarwin_h
#define libparc_parc_TimingDarwin_h

#ifdef PARCTIMING_DARWIN
#include <stdint.h>
#include <time.h>
#include <mach/mach.h>
#include <mach/clock.h>
#include <mach/mach_time.h>

/*
 * This allocates the clock service which must be released in the Fini macro
 */
#define _private_parcTiming_Init(prefix) \
    clock_serv_t prefix ## _clockService; \
    mach_timespec_t prefix ## _ts0, prefix ## _ts1; \
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &(prefix ## _clockService));

#define _private_parcTiming_Start(prefix) \
    clock_get_time((prefix ## _clockService), &(prefix ## _ts0));

#define _private_parcTiming_Stop(prefix) \
    clock_get_time((prefix ## _clockService), &(prefix ## _ts1));

static inline uint64_t
_parcTiming_Delta(const mach_timespec_t *t0, const mach_timespec_t *t1)
{
    // SUB_MACH_TIMESPEC(t1, t2) => t1 -= t2

    mach_timespec_t delta;
    memcpy(&delta, t1, sizeof(delta));
    SUB_MACH_TIMESPEC(&delta, t0);

    return (uint64_t) delta.tv_sec * 1000000000ULL + delta.tv_nsec;
}

#define _private_parcTiming_Delta(prefix) _parcTiming_Delta(&(prefix ## _ts0), &(prefix ## _ts1))

#define _private_parcTiming_Fini(prefix) \
    mach_port_deallocate(mach_task_self(), &(prefix ## _clockService));

#endif // PARCTIMING_DARWIN
#endif // libparc_parc_TimingDarwin_h

