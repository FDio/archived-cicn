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

#include <LongBow/private/longBow_Memory.h>

#if HAVE_REALLOC == 0
static void *
_LongBow_rplRealloc(void *oldAlloc, size_t newSize)
{
    if (newSize == 0) {
        newSize = 1;
    }

    char *newAlloc = malloc(newSize);

    if (oldAlloc != NULL) {
        memcpy(newAlloc, oldAlloc, newSize);
        free(oldAlloc);
    }
    return newAlloc;
}
#endif

static uint64_t _outstandingAllocations;

void *
longBowMemory_Allocate(const size_t size)
{
    _outstandingAllocations++;
    return calloc(1, size);
}

void *
longBowMemory_Reallocate(void *oldAllocation, const size_t newSize)
{
#if HAVE_REALLOC
    void *result = realloc(oldAllocation, newSize);
#else
    void *result = _LongBow_rplRealloc(oldAllocation, newSize);
#endif

    if (oldAllocation == NULL) {
        _outstandingAllocations++;
    }

    return result;
}

void
longBowMemory_Deallocate(void **pointerPointer)
{
    free(*pointerPointer);
    _outstandingAllocations--;
    *pointerPointer = NULL;
}

uint64_t
longBowMemory_OutstandingAllocations(void)
{
    return _outstandingAllocations;
}

char *
longBowMemory_StringCopy(const char *string)
{
    char *result = NULL;

    if (string != NULL) {
        size_t length = strlen(string);
        result = longBowMemory_Allocate(length + 1);
        strcpy(result, string);
        result[length] = 0;
    }
    return result;
}
