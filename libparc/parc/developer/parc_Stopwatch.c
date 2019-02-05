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

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <config.h>
#include <inttypes.h>

#if __APPLE__
#include <mach/mach.h>
#include <mach/clock.h>
#include <mach/mach_time.h>
#endif

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/developer/parc_Stopwatch.h>

struct PARCStopwatch {
    uint64_t start;
//    uint64_t stop;
};

static bool
_parcStopwatch_Destructor(PARCStopwatch **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCStopwatch pointer.");

    /* cleanup the instance fields here */
    return true;
}

parcObject_ImplementAcquire(parcStopwatch, PARCStopwatch);

parcObject_ImplementRelease(parcStopwatch, PARCStopwatch);

parcObject_Override(PARCStopwatch, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcStopwatch_Destructor,
                    .release = (PARCObjectRelease *) parcStopwatch_Release,
                    .copy = (PARCObjectCopy *) parcStopwatch_Copy,
                    .toString = (PARCObjectToString *) parcStopwatch_ToString,
                    .equals = (PARCObjectEquals *) parcStopwatch_Equals,
                    .hashCode = (PARCObjectHashCode *) parcStopwatch_HashCode,
                    .toJSON = (PARCObjectToJSON *) parcStopwatch_ToJSON,
                    .display = (PARCObjectDisplay *) parcStopwatch_Display);

void
parcStopwatch_AssertValid(const PARCStopwatch *instance)
{
    parcAssertTrue(parcStopwatch_IsValid(instance),
               "PARCStopwatch is not valid.");
}


PARCStopwatch *
parcStopwatch_Create(void)
{
    PARCStopwatch *result = parcObject_CreateInstance(PARCStopwatch);

    if (result != NULL) {
        result->start = 0;
    }

    return result;
}

PARCStopwatch *
parcStopwatch_Copy(const PARCStopwatch *original)
{
    PARCStopwatch *result = parcStopwatch_Create();
    result->start = original->start;

    return result;
}

void
parcStopwatch_Display(const PARCStopwatch *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCStopwatch@%p { .start=%" PRIu64 " }", instance, instance->start);
}

bool
parcStopwatch_Equals(const PARCStopwatch *x, const PARCStopwatch *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (memcmp(&x->start, &y->start, sizeof(x->start)) == 0) {
            result = true;
        }
    }

    return result;
}

PARCHashCode
parcStopwatch_HashCode(const PARCStopwatch *timer)
{
    PARCHashCode result = parcHashCode_Hash((uint8_t *) timer, sizeof(PARCStopwatch *));
    return result;
}

bool
parcStopwatch_IsValid(const PARCStopwatch *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcStopwatch_ToJSON(const PARCStopwatch *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
        PARCJSON *start = parcJSON_Create();
        parcJSON_AddInteger(start, "nanoseconds", instance->start);

        parcJSON_AddObject(result, "start", start);
        parcJSON_Release(&start);
    }

    return result;
}

char *
parcStopwatch_ToString(const PARCStopwatch *instance)
{
    char *result = parcMemory_Format("PARCStopwatch@%p={ .start=%" PRIu64 " }", instance, instance->start);

    return result;
}

#if __linux__
void
parcStopwatch_StartImpl(PARCStopwatch *timer, ...)
{
    struct timespec theTime;
    clock_gettime(CLOCK_REALTIME_COARSE, &theTime);

    timer->start = (uint64_t) (theTime.tv_sec * 1000000000) + theTime.tv_nsec;

    va_list ap;
    va_start(ap, timer);
    PARCStopwatch *t;

    while ((t = va_arg(ap, PARCStopwatch *)) != NULL) {
        t->start = timer->start;
    }
}

static inline uint64_t
_parcStopwatch_Stop(PARCStopwatch *timer)
{
    struct timespec theTime;
    clock_gettime(CLOCK_REALTIME_COARSE, &theTime);

    uint64_t result = (uint64_t) (theTime.tv_sec * 1000000000) + theTime.tv_nsec;
    return result;
}
#elif __APPLE__

static mach_timebase_info_data_t _parcStopWatch_TimeBaseInfo;

void
parcStopwatch_StartImpl(PARCStopwatch *timer, ...)
{
    if (_parcStopWatch_TimeBaseInfo.denom == 0) {
        mach_timebase_info(&_parcStopWatch_TimeBaseInfo);
    }

    timer->start = mach_absolute_time() * _parcStopWatch_TimeBaseInfo.numer / _parcStopWatch_TimeBaseInfo.denom;

    va_list ap;
    va_start(ap, timer);
    PARCStopwatch *t;

    while ((t = va_arg(ap, PARCStopwatch *)) != NULL) {
        t->start = timer->start;
    }
}

static inline uint64_t
_parcStopwatch_Stop(PARCStopwatch *timer)
{
    uint64_t result = mach_absolute_time() * _parcStopWatch_TimeBaseInfo.numer / _parcStopWatch_TimeBaseInfo.denom;

    return result;
}
#else
void
parcStopwatch_StartImpl(PARCStopwatch *timer, ...)
{
    struct timeval theTime;
    gettimeofday(&theTime, NULL);

    timer->start = (uint64_t) (theTime.tv_sec * 1000000000) + theTime.tv_usec * 1000;
    va_list ap;
    va_start(ap, timer);
    PARCStopwatch *t;

    while ((t = va_arg(ap, PARCStopwatch *)) != NULL) {
        t->start = timer->start;
    }
}

static inline uint64_t
_parcStopwatch_Stop(PARCStopwatch *timer)
{
    struct timeval theTime;
    gettimeofday(&theTime, NULL);

    uint64_t result = theTime.tv_sec * 1000000000 + theTime.tv_usec * 1000;

    return result;
}
#endif

static inline uint64_t
_parcStopWatch_ElapsedTimeNanos(PARCStopwatch *timer)
{
    return (_parcStopwatch_Stop(timer) - timer->start);
}

uint64_t
parcStopwatch_ElapsedTimeNanos(PARCStopwatch *timer)
{
    return _parcStopWatch_ElapsedTimeNanos(timer);
}

uint64_t
parcStopwatch_ElapsedTimeMicros(PARCStopwatch *timer)
{
    return _parcStopWatch_ElapsedTimeNanos(timer) / 1000;
}

uint64_t
parcStopwatch_ElapsedTimeMillis(PARCStopwatch *timer)
{
    return _parcStopWatch_ElapsedTimeNanos(timer) / 1000000;
}
