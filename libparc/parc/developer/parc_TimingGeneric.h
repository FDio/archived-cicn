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
 * @file parc_TimingGeneric.h
 * @brief Macros for timing code
 *
 * We cannot do any better than gettimeofday
 *
 */
#ifndef libparc_parc_TimingLinux_h
#define libparc_parc_TimingLinux_h

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <stdint.h>
#include <time.h>

#ifdef PARCTIMING_GENERIC

#define _private_parcTiming_Init(prefix) \
    struct timeval prefix ## _ts0, prefix ## _ts1, prefix ## _delta;

#define _private_parcTiming_Start(prefix) \
    gettimeofday(&(prefix ## _ts0), NULL);

#define _private_parcTiming_Stop(prefix) \
    gettimeofday(&(prefix ## _ts1), NULL);

static inline uint64_t
_parcTimingGeneric_Delta(const struct timeval *t0, const struct timeval *t1)
{
    struct timeval delta;
    timersub(t1, t0, &delta);
    return (uint64_t) delta.tv_sec * 1000000ULL + delta.tv_usec;
}

#define _private_parcTiming_Delta(prefix) _parcTimingGeneric_Delta(&(prefix ## _ts0), &(prefix ## _ts1))

// No teardown work that we need to do
#define _private_parcTiming_Fini(prefix)

#endif // PARCTIMING_GENERIC
#endif // libparc_parc_TimingLinux_h

