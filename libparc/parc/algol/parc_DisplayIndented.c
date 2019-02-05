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
#include <unistd.h>
#endif

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <parc/assert/parc_Assert.h>
#include <parc/algol/parc_DisplayIndented.h>

static char *_spaces = "                                                                                                           ";

static size_t _indentationFactor = 2;

static size_t
_indent(int indentation)
{
    size_t result = 0;

    if (indentation > 0) {
        result = write(1, _spaces, (unsigned int)(indentation * _indentationFactor));
        parcAssertTrue(result == (indentation * _indentationFactor),
                   "Write(2) failed to write %zd bytes.", indentation * _indentationFactor);
    }
    return result;
}

static void
_parcDisplayIndented_Print(int indentation, char *string)
{
    char *start = string;
    char *end = strchr(start, '\n');

    while (start != NULL) {
        _indent(indentation);
        if (end != NULL) {
            ssize_t nwritten = write(1, start, (unsigned int)(end - start + 1));
            parcAssertTrue(nwritten >= 0, "Error calling write");
            start = end + 1;
        } else {
            ssize_t nwritten = write(1, start, (unsigned int)strlen(start));
            parcAssertTrue(nwritten >= 0, "Error calling write");
            break;
        }
        end = strchr(start, '\n');
    }
}

void
parcDisplayIndented_PrintLine(int indentation, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    char *cString;
    int length = vasprintf(&cString, format, ap);
    parcAssertTrue(length >= 0, "Error in vasprintf");

    va_end(ap);

    _parcDisplayIndented_Print(indentation, cString);

    ssize_t nwritten = write(1, "\n", 1);
    parcAssertTrue(nwritten >= 0, "Error calling write");

    free(cString);
}

void
parcDisplayIndented_PrintMemory(int indentation, size_t length, const char *memory)
{
    int bytesPerLine = 16;
    char *accumulator = (char *)malloc(sizeof(char)*(bytesPerLine + 1));
    memset(accumulator, ' ', bytesPerLine);
    accumulator[bytesPerLine] = 0;

    char *cString;
    for (size_t offset = 0; offset < length; /**/) {
        int nwritten = asprintf(&cString, "%p=[", &memory[offset]);
        parcAssertTrue(nwritten >= 0, "Error calling asprintf");
        _parcDisplayIndented_Print(indentation, cString);
        free(cString);

        size_t bytesInLine = (length - offset) < bytesPerLine ? (length - offset) : bytesPerLine;
        for (size_t i = 0; i < bytesInLine; i++) {
            char c = memory[offset + i];
            printf("0x%02x, ", c & 0xFF);
            accumulator[i] = isprint(c) ? c : '.';
        }
        offset += bytesInLine;
    }
    printf("  %s]\n", accumulator);
    free(accumulator);
}
