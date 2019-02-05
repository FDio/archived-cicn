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
#include <sys/errno.h>
#endif

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <parc/assert/parc_Assert.h>
#include <parc/algol/parc_StdlibMemory.h>

static uint32_t _parcStdlibMemory_OutstandingAllocations;

#ifdef PARCLibrary_DISABLE_ATOMICS
static pthread_mutex_t _parcStdlibMemory_Mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void
_parcStdlibMemory_IncrementOutstandingAllocations(void)
{
    pthread_mutex_lock(&_parcStdlibMemory_Mutex);
    _parcStdlibMemory_OutstandingAllocations++;
    pthread_mutex_unlock(&_parcStdlibMemory_Mutex);
}

static inline void
_parcStdlibMemory_DecrementOutstandingAllocations(void)
{
    pthread_mutex_lock(&_parcStdlibMemory_Mutex);
    _parcStdlibMemory_OutstandingAllocations--;
    pthread_mutex_unlock(&_parcStdlibMemory_Mutex);
}
#else

static inline void
_parcStdlibMemory_IncrementOutstandingAllocations(void)
{
    __sync_add_and_fetch(&_parcStdlibMemory_OutstandingAllocations, 1);
}

static inline void
_parcStdlibMemory_DecrementOutstandingAllocations(void)
{
    __sync_sub_and_fetch(&_parcStdlibMemory_OutstandingAllocations, 1);
}
#endif

#ifndef HAVE_REALLOC
static void *
_parcStdlibMemory_rplRealloc(void *oldAlloc, size_t newSize)
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

void *
parcStdlibMemory_Allocate(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    void *result = malloc(size);
    if (result != NULL) {
        _parcStdlibMemory_IncrementOutstandingAllocations();
    }

    return result;
}

void *
parcStdlibMemory_AllocateAndClear(size_t size)
{
    void *pointer = parcStdlibMemory_Allocate(size);
    if (pointer != NULL) {
        memset(pointer, 0, size);
    }
    return pointer;
}

int
parcStdlibMemory_MemAlign(void **pointer, size_t alignment, size_t size)
{
    if (size == 0) {
        return EINVAL;
    }

#ifndef _WIN32
    int failure = posix_memalign(pointer, alignment, size);

    if (failure != 0) {
        return failure;
    }

    if (*pointer == NULL) {
        return ENOMEM;
    }
#else
    *pointer= _aligned_malloc(size, alignment);

    if (*pointer == NULL) {
        return errno;
    }
#endif

    _parcStdlibMemory_IncrementOutstandingAllocations();

    return 0;
}

void
parcStdlibMemory_Deallocate(void **pointer)
{
#ifndef PARCLibrary_DISABLE_VALIDATION
    parcTrapIllegalValueIf(_parcStdlibMemory_OutstandingAllocations == 0,
                       "parcStdlibMemory_Deallocate invoked with nothing left to free (double free somewhere?)\n");
#endif
    free(*pointer);
    *pointer = NULL;

    _parcStdlibMemory_DecrementOutstandingAllocations();
}

#ifdef _WIN32
void
parcStdlibMemory_DeallocateAlign(void **pointer)
{
#ifndef PARCLibrary_DISABLE_VALIDATION
    parcTrapIllegalValueIf(_parcStdlibMemory_OutstandingAllocations == 0,
                       "parcStdlibMemory_DeallocateAlign invoked with nothing left to free (double free somewhere?)\n");
#endif
    _aligned_free(*pointer);
    *pointer = NULL;

    _parcStdlibMemory_DecrementOutstandingAllocations();
}

#endif

void *
parcStdlibMemory_Reallocate(void *pointer, size_t newSize)
{
#ifdef HAVE_REALLOC
    void *result = realloc(pointer, newSize);
#else
    void *result = _parcStdlibMemory_rplRealloc(pointer, newSize);
#endif

    if (pointer == NULL) {
        _parcStdlibMemory_IncrementOutstandingAllocations();
    }
    return result;
}

char *
parcStdlibMemory_StringDuplicate(const char *string, size_t length)
{
    _parcStdlibMemory_IncrementOutstandingAllocations();
    return strndup(string, length);
}

uint32_t
parcStdlibMemory_Outstanding(void)
{
    return _parcStdlibMemory_OutstandingAllocations;
}

PARCMemoryInterface PARCStdlibMemoryAsPARCMemory = {
    .Allocate         = (uintptr_t) parcStdlibMemory_Allocate,
    .AllocateAndClear = (uintptr_t) parcStdlibMemory_AllocateAndClear,
    .MemAlign         = (uintptr_t) parcStdlibMemory_MemAlign,
    .Deallocate       = (uintptr_t) parcStdlibMemory_Deallocate,
#ifdef _WIN32
    .DeallocateAlign  = (uintptr_t) parcStdlibMemory_DeallocateAlign,
#endif
    .Reallocate       = (uintptr_t) parcStdlibMemory_Reallocate,
    .StringDuplicate  = (uintptr_t) parcStdlibMemory_StringDuplicate,
    .Outstanding      = (uintptr_t) parcStdlibMemory_Outstanding
};
