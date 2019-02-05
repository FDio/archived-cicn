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
 * @file parc_TimingLinux.h
 * @brief Macros for timing code
 *
 * On linux will use clock_gettime with the MONOTONIC RAW clock, which does not speed up or slow down based on adj_time().
 *
 */
#ifndef libparc_parc_TimingLinux_h
#define libparc_parc_TimingLinux_h

#ifdef PARCTIMING_LINUX
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define _private_parcTiming_Init(prefix) \
    struct timespec prefix ## _ts0, prefix ## _ts1;

#define _private_parcTiming_Start(prefix) \
    clock_gettime(CLOCK_MONOTONIC_RAW, &(prefix ## _ts0));

#define _private_parcTiming_Stop(prefix) \
    clock_gettime(CLOCK_MONOTONIC_RAW, &(prefix ## _ts1));

static inline uint64_t
_parcTimingLinux_Delta(const struct timespec *t0, const struct timespec *t1)
{
    struct timespec delta;

    delta.tv_sec = t1->tv_sec - t0->tv_sec;
    delta.tv_nsec = t1->tv_nsec - t0->tv_nsec;
    if (delta.tv_nsec < 0) {
        --delta.tv_sec;
        delta.tv_nsec += 1000000000;
    }

    return (uint64_t) delta.tv_sec * 1000000000ULL + delta.tv_nsec;
}

#define _private_parcTiming_Delta(prefix) _parcTimingLinux_Delta(&(prefix ## _ts0), &(prefix ## _ts1))

// No teardown work that we need to do
#define _private_parcTiming_Fini(prefix)

#endif // PARCTIMING_LINUX
#endif // libparc_parc_TimingLinux_h

