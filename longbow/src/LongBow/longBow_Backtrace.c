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
#include <stdlib.h>
#include <string.h>

#include <LongBow/longBow_Backtrace.h>
#include <LongBow/private/longBow_Memory.h>

#if defined(_WIN64)
#  define backtrace(...) (0)
#  define backtrace_symbols(...) 0
#  define backtrace_symbols_fd(...) ((void) 0)
#elif defined(_WIN32)
#  define backtrace(...) (0)
#  define backtrace_symbols(...) 0
#  define backtrace_symbols_fd(...) ((void) 0)
#elif defined(__ANDROID__)
#  define backtrace(...) (0)
#  define backtrace_symbols(...) 0
#  define backtrace_symbols_fd(...) ((void) 0)
#elif defined(__APPLE__)
#  include <execinfo.h>
#elif defined(__linux)
#  include <execinfo.h>
#elif defined(__unix) // all unices not caught above
#  define backtrace(...) (0)
#  define backtrace_symbols(...) 0
#  define backtrace_symbols_fd(...) ((void) 0)
#elif defined(__posix)
#  include <execinfo.h>
#endif

struct longbow_backtrace {
    void *callstack;
    unsigned int frames;
    unsigned int offset;
};

LongBowBacktrace *
longBowBacktrace_Create(uint32_t maximumFrames, uint32_t offset)
{
    LongBowBacktrace *result = longBowMemory_Allocate(sizeof(LongBowBacktrace));

    if (maximumFrames > 0) {
        void **stackTrace = longBowMemory_Allocate(sizeof(stackTrace[0]) * ((size_t) maximumFrames + offset));

        unsigned int frames = (unsigned int) backtrace(stackTrace, (int) (maximumFrames + offset));
        if (frames > offset) {
            unsigned int actualFrames = frames - offset;

            if (actualFrames > maximumFrames) {
                actualFrames = maximumFrames;
            }

            // Shift out the first 'offset' number of frames in the stack trace.
            memmove(&stackTrace[0], &stackTrace[offset], actualFrames * sizeof(stackTrace[0]));

            result->callstack = stackTrace;
            result->frames = actualFrames;
            result->offset = 0;
        }
    }

    return result;
}

unsigned int
longBowBacktrace_GetFrameCount(const LongBowBacktrace *backtrace)
{
    return backtrace->frames;
}

void
longBowBacktrace_Destroy(LongBowBacktrace **backtracePtr)
{
    LongBowBacktrace *backtrace = *backtracePtr;
    longBowMemory_Deallocate(&backtrace->callstack);
    longBowMemory_Deallocate((void **) backtracePtr);
}

char **
longBowBacktrace_Symbols(const LongBowBacktrace *backtrace)
{
    char **result = NULL;
    if (backtrace != NULL) {
        result = backtrace_symbols(backtrace->callstack, (int) backtrace->frames);
    }

    return result;
}

char *
longBowBacktrace_ToString(const LongBowBacktrace *backtrace)
{
    char *result = NULL;

    char **lines = longBowBacktrace_Symbols(backtrace);

    if (lines == NULL) {
        result = longBowMemory_StringCopy("(backtrace symbols not supported)");
    } else {
        size_t sum = 0;
        for (int i = 0; i < backtrace->frames; i++) {
            sum += strlen(lines[i]) + 1;
        }
        result = longBowMemory_Allocate(sum + 1 * sizeof(char));

        char *offset = result;
        for (int i = 0; i < backtrace->frames; i++) {
            strcpy(offset, lines[i]);
            offset += strlen(lines[i]);
            *offset++ = '\n';
        }
        *offset = 0;
    }

    return result;
}
