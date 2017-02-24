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
#include <stdarg.h>
#include <assert.h>

#include <LongBow/Reporting/ANSITerm/longBowReport_Runtime.h>
#include <LongBow/private/longBow_Memory.h>

static const char *ansiRed = "\x1b[31m";
static const char *ansiGreen = "\x1b[32m";
static const char *ansiYellow = "\x1b[33m";
static const char *ansiMagenta = "\x1b[35m";
static const char *ansiReset = "\x1b[0m";

static void
_printGreen(void)
{
    printf("%s", ansiGreen);
}

static void
_printYellow(void)
{
    printf("%s", ansiYellow);
}

static void
_printMagenta(void)
{
    printf("%s", ansiMagenta);
}

static void
_printReset(void)
{
    printf("%s", ansiReset);
    fflush(stdout);
}

static void
_longBowReportRuntime_RedPrintf(const char *format, va_list args)
{
    longBowReportRuntime_PrintRed();
    vprintf(format, args);
    _printReset();
}

static void
_longBowReportRuntime_YellowPrintf(const char *format, va_list args)
{
    _printYellow();
    vprintf(format, args);
    _printReset();
}

void
longBowReportRuntime_PrintRed(void)
{
    printf("%s", ansiRed);
}

void
longBowReportRuntime_RedPrintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    _longBowReportRuntime_RedPrintf(format, args);
    va_end(args);
}

void
longBowReportRuntime_GreenPrintf(const char *format, ...)
{
    _printGreen();
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    _printReset();
}

void
longBowReportRuntime_MagentaPrintf(const char *format, ...)
{
    _printMagenta();
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    _printReset();
}

void
longBowReportRuntime_YellowPrintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    _longBowReportRuntime_YellowPrintf(format, args);

    va_end(args);
}

void
longBowReportRuntime_ParseSuppress(LongBowReportConfig *result, const char *key)
{
    for (size_t i = 0; i < strlen(key); i++) {
        if (*key == 'X') {
            result->suppress_report.untested = 1;
        } else if (*key == '.') {
            result->suppress_report.succeeded = 1;
        } else if (*key == 'S') {
            result->suppress_report.skipped = 1;
        } else if (*key == 'W') {
            result->suppress_report.warned = 1;
        } else if (*key == 's') {
            result->suppress_report.setup_failed = 1;
        } else if (*key == 't') {
            result->suppress_report.teardown_failed = 1;
        } else if (*key == 'w') {
            result->suppress_report.teardown_warned = 1;
        } else if (*key == 'F') {
            result->suppress_report.failed = 1;
        } else if (*key == 'T') {
            result->suppress_report.stopped = 1;
        } else if (*key == 'U') {
            result->suppress_report.unimplemented = 1;
        } else {
            printf("Unknown suppression key '%c'\n", *key);
        }
    }
}

LongBowReportConfig *
longBowReportRuntime_Create(int argc, char *argv[])
{
    static const char *prefix = "--report";
    size_t prefixLength = strlen(prefix);

    LongBowReportConfig *result = longBowMemory_Allocate(sizeof(LongBowReportConfig));

    for (int i = 0; i < argc; i++) {
        if (strncmp(prefix, argv[i], prefixLength) == 0) {
            if (strcmp("--report-suppress", argv[i]) == 0) {
                longBowReportRuntime_ParseSuppress(result, argv[i + 1]);
                i++;
            }
        } else if (strcmp("--help", argv[i]) == 0) {
            printf("Options for LongBow Report ANSI Terminal\n");
            printf("  --report-suppress [STFU.XWstw] Suppress the display of specific reports.\n");
            printf("     S - suppress the report of a skipped test.\n");
            printf("     T - suppress the report of a stopped test.\n");
            printf("     F - suppress the report of a failed test.\n");
            printf("     U - suppress the report of an unimplemented test.\n");
            printf("     . - suppress the report of a successful test.\n");
            printf("     X - suppress the report of an untested test.\n");
            printf("     W - suppress the report of a warned test.\n");
            printf("     s - suppress the report of a setup failure.\n");
            printf("     t - suppress the report of a tear-down failure.\n");
            printf("     w - suppress the report of a tear-down warning.\n");
            free(result);
            return NULL;
        }
    }

    return result;
}

void
longBowReportRuntime_Destroy(LongBowReportConfig **reportPtr)
{
    longBowMemory_Deallocate((void **) reportPtr);
}

static void
_EventPrint(const LongBowEvent *event)
{
    if (longBowEventType_IsSuppressAlert(longBowEvent_GetEventType(event)) == false) {
        char *location = longBowLocation_ToString(longBowEvent_GetLocation(event));
        printf("%s %s %s %s\r\n",
               longBowEvent_GetName(event), location, longBowEvent_GetKind(event), longBowEvent_GetMessage(event));

        if (longBowEventType_IsSuppressBacktrace(longBowEvent_GetEventType(event)) == false) {
            char **strs = longBowEvent_CreateSymbolicCallstack(event);
            if (strs != NULL) {
                for (size_t i = 0; i < longBowEvent_GetCallStackLength(event); ++i) {
                    printf("%s\r\n", strs[i]);
                }
                free(strs);
            }
        }
        fflush(stdout);
        free(location);
    }
}

void
longBowReportRuntime_Event(const LongBowEvent *event)
{
    LongBowStatus status = longBowEventType_GetStatus(longBowEvent_GetEventType(event));
    switch (status) {
        case LongBowStatus_DONTCARE:
        case LongBowStatus_UNTESTED:
            break;

        /* successful */
        case LONGBOW_STATUS_SUCCEEDED:
            // If this happens, there is an error in the encoding of the LongBowEventType.
            longBowReportRuntime_PrintRed();
            _EventPrint(event);
            _printReset();
            break;

        case LongBowStatus_WARNED:
        case LongBowStatus_TEARDOWN_WARNED:
        case LONGBOW_STATUS_SKIPPED:
        case LongBowStatus_UNIMPLEMENTED:
        case LongBowStatus_IMPOTENT:
        case LONGBOW_STATUS_MEMORYLEAK:
        case LONGBOW_STATUS_SETUP_SKIPTESTS:
            _printYellow();
            _EventPrint(event);
            _printReset();
            break;

        /* failure */
        case LONGBOW_STATUS_FAILED:
        case LongBowStatus_STOPPED:
        case LONGBOW_STATUS_TEARDOWN_FAILED:
        case LONGBOW_STATUS_SETUP_FAILED:
        case LongBowStatus_SIGNALLED:
            longBowReportRuntime_PrintRed();
            _EventPrint(event);
            _printReset();
            break;

        case LongBowStatus_LIMIT: // fall through
        default:
            _printYellow();
            _EventPrint(event);
            _printReset();
            break;
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
    va_list args;
    va_start(args, format);
    longBowReportRuntime_YellowPrintf("WARNING ");
    _longBowReportRuntime_YellowPrintf(format, args);
    va_end(args);
}

void
longBowReportRuntime_Error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    longBowReportRuntime_RedPrintf("FAILURE ");
    _longBowReportRuntime_RedPrintf(format, args);
    va_end(args);
}
