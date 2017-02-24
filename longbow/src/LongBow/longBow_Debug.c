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

#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include <LongBow/longBow_Debug.h>
#include <LongBow/Reporting/longBowReport_Runtime.h>

/** @cond */
struct longbow_debug_criteria {
    bool enabled;
};
/** @endcond */

static struct longbow_debug_criteria LongBowDebug_StaticCriteria = {
    .enabled = true
};

static LongBowDebugCriteria *LongBowDebug_CurrentCriteria = &LongBowDebug_StaticCriteria;

LongBowDebugCriteria *
longBowDebug_CurrentCriteria(void)
{
    return LongBowDebug_CurrentCriteria;
}

static void
_longBowDebug_MemoryDumpLine(const char *memory, size_t offset, size_t length)
{
    int bytesPerLine = 16;
    char accumulator[bytesPerLine + 1];
    memset(accumulator, ' ', bytesPerLine);
    accumulator[bytesPerLine] = 0;

    printf("%5zd: ", offset);

    for (size_t i = 0; i < bytesPerLine; i++) {
        if (offset + i >= length) {
            printf("   ");
            accumulator[i] = ' ';
        } else {
            char c = memory[offset + i];
            printf("%02x ", c & 0xFF);
            if (isprint(c)) {
                accumulator[i] = c;
            } else {
                accumulator[i] = '.';
            }
        }
    }
    printf("   %s\n", accumulator);
}

void
longBowDebug_MemoryDump(const char *memory, size_t length)
{
    size_t bytesPerLine = 16;

    for (size_t offset = 0; offset < length; offset += bytesPerLine) {
        _longBowDebug_MemoryDumpLine(memory, offset, length);
    }
}

void
longBowDebug_Message(LongBowDebugCriteria *criteria, const LongBowLocation *location, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    char *message;
    int status = vasprintf(&message, format, ap);
    va_end(ap);
    if (status != -1) {
#if 1
        char *locationString = longBowLocation_ToString(location);
        longBowReportRuntime_Message("%s %s\r\n", locationString, message);

        free(locationString);
#else
        longBowReportRuntime_Message(location, message);
#endif
        free(message);
    }
}

ssize_t
longBowDebug_WriteFile(const char *fileName, const char *data, size_t length)
{
    ssize_t result = 0;

    int fd = open(fileName, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd == -1) {
        perror(fileName);
    } else {
        result = write(fd, data, length);
    }
    close(fd);
    return result;
}

ssize_t
longBowDebug_ReadFile(const char *fileName, char **data)
{
    ssize_t result = -1;
    struct stat statbuf;
    char *buffer;

    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        perror(fileName);
    } else {
        if (fstat(fd, &statbuf) != -1) {
            buffer = malloc((unsigned long) statbuf.st_size + 1);
            result = read(fd, buffer, (unsigned long) statbuf.st_size);
            buffer[statbuf.st_size] = 0;
            *data = buffer;
        }
    }
    return result;
}
