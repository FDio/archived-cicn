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
#include <assert.h>

#include <LongBow/Reporting/longBowReport_Runtime.h>
#include <LongBow/private/longBow_Memory.h>

LongBowReportConfig *
longBowReportRuntime_Create(int argc, char *argv[argc])
{
    LongBowReportConfig *result = longBowMemory_Allocate(sizeof(LongBowReportConfig));

    return result;
}

void
longBowReportRuntime_Destroy(LongBowReportConfig **reportPtr)
{
    longBowMemory_Deallocate((void **) reportPtr);
}

void
longBowReportRuntime_Event(const LongBowEvent *event)
{
    if (longBowEventType_IsSuppressAlert(longBowEvent_GetEventType(event)) == false) {
        char *location = longBowLocation_ToString(longBowEvent_GetLocation(event));
        printf("%s %s %s %s\r\n",
               longBowEvent_GetName(event), location, longBowEvent_GetKind(event), longBowEvent_GetMessage(event));

        if (longBowEventType_IsSuppressBacktrace(longBowEvent_GetEventType(event)) == false) {
            char **strs = longBowEvent_CreateSymbolicCallstack(event);
            if (strs != NULL) {
                for (size_t i = 0; i < longBowEvent_GetCallStackLength(event); ++i) {
                    fputs(strs[i], stdout);
                    fputs("\r\n", stdout);
                }
                free(strs);
            }
        }
        fflush(stdout);

        free(location);
    }
}

void
longBowReportRuntime_Message(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void
longBowReportRuntime_Warning(const char *format, ...)
{
    printf("WARNING");

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void
longBowReportRuntime_Error(const char *format, ...)
{
    printf("ERROR");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
