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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>

#include <LongBow/longBow_Runtime.h>
#include <LongBow/longBow_Backtrace.h>
#include <LongBow/longBow_Location.h>
#include <LongBow/longBow_Event.h>
#include <LongBow/longBow_Config.h>

#include <LongBow/Reporting/longBowReport_Runtime.h>

#include <LongBow/private/longBow_Memory.h>

static unsigned int longBowStackTraceDepth = 128;

struct longbow_runtime {
    LongBowConfig *config;
    LongBowRuntimeResult expectedResult;
    LongBowRuntimeResult actualResult;
};

static LongBowRuntime longBowRuntimeGlobal;

static LongBowRuntime *longBowCurrentRuntime = &longBowRuntimeGlobal;


static char *
_longBowRuntime_FormatErrnoMessage(void)
{
    char *result = NULL;
    if (errno != 0) {
        if (asprintf(&result, "%s: ", strerror(errno)) == -1) {
            return NULL;
        }
    }
    return result;
}

static char *
_longBowRuntime_FormatMessage(const char *format, va_list args)
{
    char *errnoMessage = _longBowRuntime_FormatErrnoMessage();

    char *string;
    if (vasprintf(&string, format, args) == -1) {
        return NULL;
    }

    char *result = NULL;

    if (errnoMessage != NULL) {
        int check = asprintf(&result, "%s%s", errnoMessage, string);
        free(errnoMessage);
        if (check == -1) {
            return NULL;
        }
    } else {
        result = strndup(string, strlen(string));
    }

    free(string);

    return result;
}

LongBowRuntime *
longBowRuntime_Create(const LongBowRuntimeResult *expectedResultTemplate, LongBowConfig *config)
{
    assert(expectedResultTemplate != NULL);

    LongBowRuntime *result = longBowMemory_Allocate(sizeof(LongBowRuntime));
    if (result != NULL) {
        result->expectedResult = *expectedResultTemplate;
        result->config = config;
    }
    return result;
}

size_t
longBowRuntime_GetActualEventEvaluationCount(LongBowRuntime *runtime)
{
    return longBowRuntimeResult_GetEventEvaluationCount(longBowRuntime_GetActualTestCaseResult(runtime));
}

void
longBowRuntime_Destroy(LongBowRuntime **runtimePtr)
{
    longBowMemory_Deallocate((void **) runtimePtr);
}

LongBowRuntimeResult *
longBowRuntime_GetExpectedTestCaseResult(const LongBowRuntime *const runtime)
{
    return (LongBowRuntimeResult *) &(runtime->expectedResult);
}

LongBowRuntimeResult *
longBowRuntime_GetActualTestCaseResult(const LongBowRuntime *runtime)
{
    return (LongBowRuntimeResult *) &(runtime->actualResult);
}

LongBowRuntime *
longBowRuntime_SetCurrentRuntime(LongBowRuntime *runtime)
{
    LongBowRuntime *result = longBowCurrentRuntime;
    longBowCurrentRuntime = runtime;
    return result;
}

LongBowRuntime *
longBowRuntime_GetCurrentRuntime(void)
{
    return longBowCurrentRuntime;
}

LongBowConfig *
longBowRuntime_GetCurrentConfig(void)
{
    return longBowCurrentRuntime->config;
}

void
longBowRuntime_SetCurrentConfig(LongBowConfig *config)
{
    longBowCurrentRuntime->config = config;
}

LongBowEventType *
longBowRuntime_GetActualEventType(const LongBowRuntime *runtime)
{
    return runtime->actualResult.event;
}

LongBowEventType *
longBowRuntime_GetExpectedEventType(const LongBowRuntime *runtime)
{
    return runtime->expectedResult.event;
}

void
longBowRuntime_SetActualEventType(LongBowRuntime *runtime, LongBowEventType *eventType)
{
    runtime->actualResult.event = eventType;
}

bool
longBowRuntime_EventEvaluation(const LongBowEventType *type)
{
    longBowCurrentRuntime->actualResult.eventEvaluationCount++;
    return true;
}

unsigned int
longBowRuntime_SetStackTraceDepth(unsigned int newDepth)
{
    unsigned int previousValue = longBowStackTraceDepth;
    longBowStackTraceDepth = newDepth;
    return previousValue;
}

unsigned int
longBowRuntime_GetStackTraceDepth(void)
{
    return longBowStackTraceDepth;
}

bool
longBowRuntime_EventTrigger(LongBowEventType *eventType, LongBowLocation *location, const char *kind, const char *format, ...)
{
    LongBowRuntime *runtime = longBowRuntime_GetCurrentRuntime();

    longBowRuntime_SetActualEventType(runtime, eventType);

    if (runtime->expectedResult.status == LONGBOW_STATUS_FAILED) {
        return true;
    }

    if (longBowEventType_Equals(longBowRuntime_GetActualEventType(runtime), longBowRuntime_GetExpectedEventType(runtime))) {
        return true;
    }

    va_list args;
    va_start(args, format);

    char *messageString = _longBowRuntime_FormatMessage(format, args);

    va_end(args);

    LongBowBacktrace *stackTrace = longBowBacktrace_Create(longBowRuntime_GetStackTraceDepth(), 2);

    LongBowEvent *event = longBowEvent_Create(eventType, location, kind, messageString, stackTrace);

    free(messageString);
    longBowReportRuntime_Event(event);
    longBowEvent_Destroy(&event);
    return true;
}

void
longBowRuntime_StackTrace(int fileDescriptor)
{
    LongBowBacktrace *backtrace = longBowBacktrace_Create(longBowRuntime_GetStackTraceDepth(), 1);
    char *string = longBowBacktrace_ToString(backtrace);
    fwrite(string, strlen(string), 1, stdout);
    longBowMemory_Deallocate((void **) &string);
    longBowBacktrace_Destroy(&backtrace);
}

bool
longBowRuntime_TestAddressIsAligned(const void *address, size_t alignment)
{
    if ((alignment & (~alignment + 1)) == alignment) {
        return (((uintptr_t) address) % alignment) == 0 ? true : false;
    }
    return false;
}

void
longBowRuntime_CoreDump(void)
{
    struct rlimit coreDumpLimit;

    coreDumpLimit.rlim_cur = RLIM_INFINITY;
    coreDumpLimit.rlim_max = RLIM_INFINITY;

    if (setrlimit(RLIMIT_CORE, &coreDumpLimit) < 0) {
        fprintf(stderr, "setrlimit: %s\n", strerror(errno));
        exit(1);
    }
    kill(0, SIGTRAP);
}

void
longBowRuntime_Abort(void)
{
    bool coreDump = longBowConfig_GetBoolean(longBowRuntime_GetCurrentConfig(), false, "core-dump");
    if (coreDump == false) {
        abort();
    } else {
        longBowRuntime_CoreDump();
    }
}
