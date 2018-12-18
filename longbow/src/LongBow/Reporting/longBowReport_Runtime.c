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

#include <LongBow/Reporting/longBowReport_Runtime.h>

char *
longBowReportRuntime_TimevalToString(const struct timeval time)
{
    char *string = NULL;
    if (asprintf(&string, "%ld.%06lds", time.tv_sec, (long) time.tv_usec) == -1) {
        return NULL;
    }
    return string;
}

char *
longBowReportRuntime_RUsageToString(const struct rusage *rusage)
{
    char *string;

    char *ru_utime = longBowReportRuntime_TimevalToString(rusage->ru_utime);
    char *ru_stime = longBowReportRuntime_TimevalToString(rusage->ru_stime);

    if (asprintf(&string, "%s %s", ru_utime, ru_stime) == -1) {
        return NULL;
    }

    free(ru_utime);
    free(ru_stime);

    return string;
}
