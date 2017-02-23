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
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <LongBow/private/longBow_String.h>
#include <LongBow/private/longBow_Memory.h>

struct longbow_string {
    char *buffer;
    size_t cursor; // always the index of the nul terminating byte of the stored string.
    size_t end; // always the index of the very last byte in buffer;
};

static size_t
_longBowString_RemainingSpace(const LongBowString *string)
{
    size_t result = string->end - string->cursor;

    return result;
}

LongBowString *
longBowString_Create(const size_t initialSize)
{
    LongBowString *result = longBowMemory_Allocate(sizeof(LongBowString));
    result->end = initialSize;
    result->buffer = longBowMemory_Allocate(initialSize);
    result->cursor = 0;

    return result;
}

LongBowString *
longBowString_CreateString(const char *string)
{
    LongBowString *result = longBowMemory_Allocate(sizeof(LongBowString));
    result->end = strlen(string) + 1;
    result->buffer = longBowMemory_StringCopy(string);
    result->cursor = result->end - 1;
    result->buffer[result->cursor] = 0;

    return result;
}

LongBowString *
longBowString_CreateFormat(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *cString;
    if (vasprintf(&cString, format, ap) == -1) {
        return NULL;
    }
    va_end(ap);

    LongBowString *string = longBowString_CreateString(cString);

    free(cString);

    return string;
}

void
longBowString_Destroy(LongBowString **stringPtr)
{
    LongBowString *string = *stringPtr;
    if (string != NULL) {
        longBowMemory_Deallocate((void **) &string->buffer);
        longBowMemory_Deallocate((void **) stringPtr);
    }
}

LongBowString *
longBowString_Append(LongBowString *string, const char *value)
{
    size_t length = strlen(value) + 1;

    if (_longBowString_RemainingSpace(string) < length) {
        size_t size = string->end + length;
        string->buffer = longBowMemory_Reallocate(string->buffer, size);
        string->end = size - 1;
    }
    strcpy(&string->buffer[string->cursor], value);
    string->cursor += (length - 1);
    string->buffer[string->cursor] = 0;

    return string;
}

LongBowString *
longBowString_Format(LongBowString *string, const char *format, ...)
{
    LongBowString *result = NULL;

    va_list ap;
    va_start(ap, format);

    char *cString;
    int status = vasprintf(&cString, format, ap);
    va_end(ap);
    if (status != -1) {
        result = longBowString_Append(string, cString);
        free(cString);
    } else {
        result = NULL;
    }

    return result;
}

char *
longBowString_ToString(const LongBowString *string)
{
    char *result = strndup(string->buffer, string->end);
    return result;
}

bool
longBowString_StartsWith(const char *string, const char *prefix)
{
    bool result = strncmp(string, prefix, strlen(prefix)) == 0;
    return result;
}

bool
longBowString_Equals(const char *string, const char *other)
{
    return strcmp(string, other) == 0;
}

bool
longBowString_Write(const LongBowString *string, FILE *fp)
{
    bool result = false;
    size_t nwrite = string->end;

    if (fwrite(string->buffer, sizeof(char), string->end, fp) == nwrite) {
        result = true;
    }

    return result;
}

LongBowArrayList *
longBowString_Tokenise(const char *string, const char *separators)
{
    LongBowArrayList *result = longBowArrayList_Create(longBowMemory_Deallocate);
    if (string != NULL) {
        char *workingCopy = longBowMemory_StringCopy(string);

        char *p = strtok(workingCopy, separators);
        while (p) {
            longBowArrayList_Add(result, longBowMemory_StringCopy(p));
            p = strtok(NULL, separators);
        }

        longBowMemory_Deallocate((void **) &workingCopy);
    }

    return result;
}
