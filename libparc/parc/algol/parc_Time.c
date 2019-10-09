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

#ifdef __linux__
#if __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
#endif

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <parc/assert/parc_Assert.h>
#include <parc/algol/parc_Time.h>
#include <parc/algol/parc_Memory.h>

int asprintf(char **strp, const char *fmt, ...);

char *
parcTime_TimevalAsString(struct timeval timeval)
{
    char *string;
    int nwritten = asprintf(&string, "%ld.%06ld", timeval.tv_sec, (long) timeval.tv_usec);
    parcAssertTrue(nwritten >= 0, "Error calling asprintf");

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);
    return result;
}

char *
parcTime_TimevalAsRFC3339(const struct timeval *utcTime, char result[64])
{
    char tmbuf[64];
    struct tm *nowtm;

#ifndef _WIN32
    struct tm theTime;
    nowtm = gmtime_r(&utcTime->tv_sec, &theTime);
#else
    struct tm theTime;
    __int64 ltime = utcTime->tv_sec;
    int x = _gmtime64_s(&theTime, &ltime);
    nowtm = &theTime;
#endif

    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%dT%H:%M:%S", nowtm);
    snprintf(result, 64, "%s.%06ldZ", tmbuf, (long)utcTime->tv_usec);
    return result;
}

char *
parcTime_TimevalAsISO8601(const struct timeval *utcTime, char result[64])
{
    char tmbuf[64];
    struct tm theTime;

    struct tm *nowtm;

#ifndef _WIN32
    nowtm = gmtime_r(&utcTime->tv_sec, &theTime);
#else
    __int64 ltime = utcTime->tv_sec;
    _gmtime64_s(&theTime, &ltime);
    nowtm = &theTime;
#endif

    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
    snprintf(result, 64, "%s.%06ldZ", tmbuf, (long) utcTime->tv_usec);
    return result;
}

char *
parcTime_TimeAsRFC3339(const time_t utcTime, char result[64])
{
    struct timeval theTime = { (long)utcTime, 0 };

    return parcTime_TimevalAsRFC3339(&theTime, result);
}

char *
parcTime_NowAsRFC3339(char result[64])
{
    struct timeval theTime;
    gettimeofday(&theTime, NULL);

    return parcTime_TimevalAsRFC3339(&theTime, result);
}

char *
parcTime_TimeAsISO8601(const time_t utcTime, char result[64])
{
    struct timeval theTime = { (long)utcTime, 0 };

    return parcTime_TimevalAsISO8601(&theTime, result);
}

char *
parcTime_NowAsISO8601(char result[64])
{
    struct timeval theTime;
    gettimeofday(&theTime, NULL);

    return parcTime_TimevalAsISO8601(&theTime, result);
}

struct timeval
parcTime_TimevalAdd(const struct timeval *addend1, const struct timeval *addend2)
{
    struct timeval sum;

    sum.tv_sec = addend1->tv_sec + addend2->tv_sec;
    sum.tv_usec = addend1->tv_usec + addend2->tv_usec;
    if (sum.tv_usec >= 1000000) {
        sum.tv_usec -= 1000000;
        sum.tv_sec++;
    }
    return sum;
}

struct timeval
parcTime_TimevalSubtract(const struct timeval *minuend, const struct timeval *subtrahend)
{
    struct timeval result;

    result.tv_sec = minuend->tv_sec - subtrahend->tv_sec;
    result.tv_usec = minuend->tv_usec - subtrahend->tv_usec;
    if (result.tv_usec < 0) {
        result.tv_sec--;
        result.tv_usec += 1000000;
    }
    return result;
}

struct timeval
parcTime_NowTimeval(void)
{
    struct timeval timeval;
    gettimeofday(&timeval, NULL);
    return timeval;
}

uint64_t
parcTime_NowMicroseconds(void)
{
    struct timeval timeval;
    gettimeofday(&timeval, NULL);

    uint64_t result = timeval.tv_sec * 1000000 + timeval.tv_usec;
    return result;
}

uint64_t
parcTime_NowNanoseconds(void)
{
    struct timeval timeval;
    gettimeofday(&timeval, NULL);

    uint64_t result = timeval.tv_sec * 1000000000 + timeval.tv_usec * 1000;
    return result;
}

#ifdef __linux__
#if __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif
#endif
