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
#include <stdio.h>
#include <sys/time.h>

#include <LongBow/longBow_MeasureTime.h>
#include <LongBow/private/longBow_Memory.h>

struct longBowMeasureTime {
    struct timeval start;
    struct timeval stop;
    unsigned int iterations;
};

LongBowMeasureTime *
longBowMeasureTime_Start(unsigned int iterations)
{
    LongBowMeasureTime *result = longBowMemory_Allocate(sizeof(LongBowMeasureTime));

    if (result != NULL) {
        gettimeofday(&result->start, NULL);
        result->iterations = iterations;
        result->stop = (struct timeval) { .tv_sec = 0, .tv_usec = 0 };
    }

    return result;
}

LongBowMeasureTime *
longBowMeasureTime_Stop(LongBowMeasureTime *measure)
{
    if (measure != NULL) {
        gettimeofday(&measure->stop, NULL);
    }

    return measure;
}

uint64_t
longBowMeasureTime_GetMicroseconds(const LongBowMeasureTime *measure)
{
    struct timeval result;
    timersub(&(measure->stop), &(measure->start), &result);

    return ((uint64_t) result.tv_sec * 1000000ULL) + ((uint64_t) result.tv_usec);
}

uint64_t
longBowMeasureTime_GetNanoseconds(const LongBowMeasureTime *measure)
{
    struct timeval result;
    timersub(&(measure->stop), &(measure->start), &result);

    return ((uint64_t) result.tv_sec * 1000000000ULL) + ((uint64_t) result.tv_usec * 1000);
}

unsigned int
longBowMeasureTime_CountDown(LongBowMeasureTime *measure)
{
    return measure->iterations--;
}

bool
longBowMeasureTime_Report(LongBowMeasureTime *measure, const char *file, const char *function, unsigned int line)
{
    struct timeval result;
    if (measure->stop.tv_sec == 0 && measure->stop.tv_usec == 0) {
        longBowMeasureTime_Stop(measure);
    }

    timersub(&(measure->stop), &(measure->start), &result);
    printf("%s %s %d %ld.%06ld\n", file, function, line, result.tv_sec, (long) result.tv_usec);
    return true;
}

void
longBowMeasureTime_Destroy(LongBowMeasureTime **instancePtr)
{
    longBowMemory_Deallocate((void **) instancePtr);
}
