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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <parc/assert/parc_Assert.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_StdlibMemory.h>

static const PARCMemoryInterface *parcMemory = &PARCStdlibMemoryAsPARCMemory;

const PARCMemoryInterface *
parcMemory_SetInterface(const PARCMemoryInterface *memoryProvider)
{
    parcAssertFalse(memoryProvider == &PARCMemoryAsPARCMemory,
                "You cannot use PARCMemoryAsPARCMemory as a memory provider for parcMemory.");
    const PARCMemoryInterface *result = parcMemory;
    parcMemory = memoryProvider;

    return result;
}

size_t
parcMemory_RoundUpToCacheLine(const size_t size)
{
    return parcMemory_RoundUpToMultiple(size, LEVEL1_DCACHE_LINESIZE);
}

size_t
parcMemory_RoundUpToMultiple(const size_t size, const size_t multiple)
{
    if (size == 0) {
        return multiple;
    }

    if (multiple == 0) {
        return size;
    }

    size_t remainder = size % multiple;
    if (remainder == 0) {
        return size;
    }
    return size + multiple - remainder;
}

void *
parcMemory_Allocate(const size_t size)
{
    return ((PARCMemoryAllocate *) parcMemory->Allocate)(size);
}

void *
parcMemory_AllocateAndClear(const size_t size)
{
    return ((PARCMemoryAllocateAndClear *) parcMemory->AllocateAndClear)(size);
}

int
parcMemory_MemAlign(void **pointer, const size_t alignment, const size_t size)
{
    return ((PARCMemoryMemAlign *) parcMemory->MemAlign)(pointer, alignment, size);
}

void
parcMemory_DeallocateImpl(void **pointer)
{
    ((PARCMemoryDeallocate *) parcMemory->Deallocate)(pointer);
}

#ifdef _WIN32
void
parcMemory_DeallocateAlignImpl(void **pointer)
{
    ((PARCMemoryDeallocateAlign *) parcMemory->DeallocateAlign)(pointer);
}
#endif

void *
parcMemory_Reallocate(void *pointer, size_t newSize)
{
    return ((PARCMemoryReallocate *) parcMemory->Reallocate)(pointer, newSize);
}

char *
parcMemory_StringDuplicate(const char *string, const size_t length)
{
    return ((PARCMemoryStringDuplicate *) parcMemory->StringDuplicate)(string, length);
}

uint32_t
parcMemory_Outstanding(void)
{
    return ((PARCMemoryOutstanding *) parcMemory->Outstanding)();
}

char *
parcMemory_Format(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    va_list copy;
    va_copy(copy, ap);
    int length = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    char *result = NULL;
    if (length >= 0) {
        result = parcMemory_Allocate(length + 1);

        if (result != NULL) {
            vsprintf(result, format, ap);
        }
    }
    return result;
}

PARCMemoryInterface PARCMemoryAsPARCMemory = {
    .Allocate         = (uintptr_t) parcMemory_Allocate,
    .AllocateAndClear = (uintptr_t) parcMemory_AllocateAndClear,
    .MemAlign         = (uintptr_t) parcMemory_MemAlign,
    .Deallocate       = (uintptr_t) parcMemory_DeallocateImpl,
#ifdef _WIN32
    .DeallocateAlign  = (uintptr_t)parcMemory_DeallocateAlignImpl,
#endif
    .Reallocate       = (uintptr_t) parcMemory_Reallocate,
    .StringDuplicate  = (uintptr_t) parcMemory_StringDuplicate,
    .Outstanding      = (uintptr_t) parcMemory_Outstanding
};
