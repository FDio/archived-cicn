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
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Object.h>

#include <ccnx/common/ccnx_TimeStamp.h>

#include <parc/algol/parc_Memory.h>

struct ccnx_timestamp {
    struct timespec timespec;
};

static void
_Destroy(CCNxTimeStamp **timeStampPtr)
{
}

parcObject_ExtendPARCObject(CCNxTimeStamp, _Destroy, ccnxTimeStamp_Copy,
                            ccnxTimeStamp_ToString, ccnxTimeStamp_Equals, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxTimeStamp, CCNxTimeStamp);

parcObject_ImplementRelease(ccnxTimeStamp, CCNxTimeStamp);

void
ccnxTimeStamp_AssertValid(const CCNxTimeStamp *timeStamp)
{
    assertNotNull(timeStamp, "Parameter must be a pointer to a CCNxTimeStamp structure.");
}

CCNxTimeStamp *
ccnxTimeStamp_CreateFromCurrentUTCTime(void)
{
    struct timeval timeval;
    gettimeofday(&timeval, NULL);

    struct timespec timespec;
    timespec.tv_sec = timeval.tv_sec;
    timespec.tv_nsec = timeval.tv_usec * 1000;

    return ccnxTimeStamp_CreateFromTimespec(&timespec);
}

CCNxTimeStamp *
ccnxTimeStamp_CreateFromTimespec(const struct timespec *timespec)
{
    CCNxTimeStamp *result = parcObject_CreateInstance(CCNxTimeStamp);
    result->timespec.tv_sec = timespec->tv_sec;
    result->timespec.tv_nsec = timespec->tv_nsec;

    return result;
}

struct timespec
ccnxTimeStamp_AsTimespec(const CCNxTimeStamp *timeStamp)
{
    ccnxTimeStamp_AssertValid(timeStamp);
    return timeStamp->timespec;
}

CCNxTimeStamp *
ccnxTimeStamp_CreateFromNanosecondsSinceEpoch(uint64_t nanos)
{
    struct timespec timespec = { .tv_sec = nanos / 1000000000, .tv_nsec = (nanos % 1000000000) };

    return ccnxTimeStamp_CreateFromTimespec(&timespec);
}

CCNxTimeStamp *
ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(uint64_t millis)
{
    struct timespec timespec = { .tv_sec = millis / 1000, .tv_nsec = (millis % 1000) * 1000000 };
    return ccnxTimeStamp_CreateFromTimespec(&timespec);
}

bool
ccnxTimeStamp_Equals(const CCNxTimeStamp *timeStampA, const CCNxTimeStamp *timeStampB)
{
    if (timeStampA == timeStampB) {
        return true;
    }
    if (timeStampA == NULL || timeStampB == NULL) {
        return false;
    }
    if (timeStampA->timespec.tv_sec == timeStampB->timespec.tv_sec) {
        if (timeStampA->timespec.tv_nsec == timeStampB->timespec.tv_nsec) {
            return true;
        }
    }
    return false;
}

uint64_t
ccnxTimeStamp_AsNanoSeconds(const CCNxTimeStamp *timeStamp)
{
    ccnxTimeStamp_AssertValid(timeStamp);
    uint64_t result = timeStamp->timespec.tv_sec * 1000000000ULL + timeStamp->timespec.tv_nsec;
    return result;
}

char *
ccnxTimeStamp_ToString(const CCNxTimeStamp *timeStamp)
{
    if (timeStamp == NULL) {
        return parcMemory_StringDuplicate("NULL", 4);
    }

    char *string;
    int failure = asprintf(&string, "%ld.%ld", timeStamp->timespec.tv_sec, timeStamp->timespec.tv_nsec);
    assertTrue(failure > -1, "Error asprintf");

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);
    return result;
}

CCNxTimeStamp *
ccnxTimeStamp_Copy(const CCNxTimeStamp *timeStamp)
{
    ccnxTimeStamp_AssertValid(timeStamp);
    return ccnxTimeStamp_CreateFromTimespec(&timeStamp->timespec);
}
