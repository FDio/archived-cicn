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
 * This is a substitute for posix_memalign(3) that
 * establishes detectable boundaries around an allocated memory segment,
 * records a stack backtrace for each allocation,
 * detects buffer overruns and underruns by checking the boundaries when the memory is deallocated,
 * and tries to prevent a stray pointer to reference the memory again once it's been deallocated.
 *
 * The allocated memory consists of three contiguous segments: the prefix, the memory usable by the caller, and the suffix.
 * The memory usable by the caller is aligned as specified by the caller.
 * The alignment must be a power of 2 greater than or equal to the size of a {@code void *}.
 * <pre>
 * +--base  +-prefix     +-- memory           +-- suffix aligned on (void *)
 * v        v            v                    v
 * |________|PPPPPPPPPPPP|mmmmmmmmm...mmmm|___|SSSSSSSSS
 *                                         ^
 *                                         +-- variable padding
 * </pre>
 * Where '-' indicates padding, 'P' indicates the prefix data structure, 'm'
 * indicates contiguous memory for use by the caller, and 'S" indicates the suffix data structure.
 *
 */
#include <config.h>

#include <parc/assert/parc_Assert.h>

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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/queue.h>
#include <pthread.h>

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#include <parc/algol/parc_StdlibMemory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

typedef struct memory_backtrace {
    void **callstack;
    int maximumFrameCount;
    int actualFrameCount;
} _MemoryBacktrace;

static const uint32_t _parcSafeMemory_SuffixGuard = 0xcafecafe;
typedef struct memory_suffix {
    uint32_t guard;
} _MemorySuffix;

static const uint64_t _parcSafeMemory_PrefixMagic = 0xfacefacefaceface;
static const uint64_t _parcSafeMemory_Guard = 0xdeaddeaddeaddead;
static const uint64_t _parcSafeMemory_GuardAlreadyFreed = 0xBADDCAFEBADDCAFE;
typedef struct memory_prefix {
    uint64_t magic;               // A magic number indicating the start of this data structure.
    size_t requestedLength;       // The number of bytes the caller requested.
    size_t actualLength;          // The number of bytes >= requestedLength to ensure the right alignment for the suffix.
    size_t alignment;             // The aligment required by the caller.  Must be a power of 2 and >= sizeof(void *).
    _MemoryBacktrace *backtrace;  // A record of the caller's stack trace at the time of allocation.
    uint64_t guard;               // Try to detect underrun of the allocated memory.
} _MemoryPrefix;

typedef void *PARCSafeMemoryOrigin;

typedef void *PARCSafeMemoryUsable;

static PARCMemoryInterface *_parcMemory = &PARCStdlibMemoryAsPARCMemory;

static pthread_mutex_t _parcSafeMemory_Mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * Return true if the given alignment value is greater than or equal to {@code sizeof(void *)} and
 * is a power of 2.
 *
 * @param alignment
 * @return
 */
static bool
_alignmentIsValid(size_t alignment)
{
    return alignment >= sizeof(void *) && (alignment & (~alignment + 1)) == alignment;
}

/**
 * Add two pointers arithmetically.
 */
// Should increment be a ssize_t, instead of size_t?
static void *
_pointerAdd(const void *base, const size_t increment)
{
    void *result = (void *) &((char *) base)[increment];
    return result;
}

static size_t
_computePrefixLength(const size_t alignment)
{
    return (sizeof(_MemoryPrefix) + alignment - 1) & ~(alignment - 1);
}

static size_t
_computeUsableMemoryLength(const size_t requestedLength, const size_t alignment)
{
    return (requestedLength + alignment - 1) & ~(alignment - 1);
}

/**
 * Compute the size of the suffix on an allocated chunk of managed memory that causes the
 * first byte after this size to be aligned according to the given alignment value.
 */
static size_t
_computeSuffixLength(size_t alignment __attribute__((unused)))
{
    return sizeof(_MemorySuffix);
}

/**
 * Compute the total number of bytes necessary to store the entire Safe Memory structure.
 * Given a size number of bytes for use by a client function,
 * produce the total number of bytes necessary to store @p size
 * number of bytes for use by a client function and the `_MemoryPrefix`
 * and `_MemorySuffix` structures.
 */
static size_t
_computeMemoryTotalLength(size_t requestedLength, size_t alignment)
{
    size_t result =
        _computePrefixLength(alignment)
        + _computeUsableMemoryLength(requestedLength, sizeof(void*))
        + _computeSuffixLength(alignment);

    return result;
}

/**
 * Given the safe memory address, return a pointer to the _MemoryPrefix structure.
 */
static _MemoryPrefix *
_parcSafeMemory_GetPrefix(const PARCSafeMemoryUsable *usable)
{
    _MemoryPrefix *prefix = _pointerAdd(usable, -sizeof(_MemoryPrefix));
    return prefix;
}

/**
 * Given a base address for memory Return a pointer to the {@link _MemorySuffix}
 * structure for the given base pointer to allocated memory.
 *
 * @param base
 * @return
 */
static _MemorySuffix *
_parcSafeMemory_GetSuffix(const PARCSafeMemoryUsable *memory)
{
    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(memory);

    _MemorySuffix *suffix = _pointerAdd(memory, prefix->actualLength);
    return suffix;
}

static PARCSafeMemoryState
_parcSafeMemory_GetPrefixState(const PARCSafeMemoryUsable *usable)
{
    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(usable);

    if (prefix->guard == _parcSafeMemory_GuardAlreadyFreed) {
        return PARCSafeMemoryState_ALREADYFREE;
    }
    if (prefix->guard != _parcSafeMemory_Guard) {
        return PARCSafeMemoryState_UNDERRUN;
    }
    if (prefix->magic != _parcSafeMemory_PrefixMagic) {
        return PARCSafeMemoryState_UNDERRUN;
    }
    if (!_alignmentIsValid(prefix->alignment)) {
        return PARCSafeMemoryState_UNDERRUN;
    }

    return PARCSafeMemoryState_OK;
}

static PARCSafeMemoryOrigin *
_parcSafeMemory_GetOrigin(const PARCSafeMemoryUsable *memory)
{
    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(memory);

    return _pointerAdd(memory, -_computePrefixLength(prefix->alignment));
}

static PARCSafeMemoryState
_parcSafeMemory_GetSuffixState(const PARCSafeMemoryUsable *memory)
{
    PARCSafeMemoryState result = PARCSafeMemoryState_OK;;
    _MemorySuffix *suffix = _parcSafeMemory_GetSuffix(memory);
    if (suffix->guard != _parcSafeMemory_SuffixGuard) {
        result = PARCSafeMemoryState_OVERRUN;
    }
    return result;
}

/**
 * Given a pointer to the base address of an allocated memory segment,
 * compute and return a pointer to the corresponding {@link _MemorySuffix} for that same memory.
 */
static _MemorySuffix *
_parcSafeMemory_FormatSuffix(const PARCSafeMemoryUsable *memory)
{
    _MemorySuffix *suffix = _parcSafeMemory_GetSuffix(memory);

    suffix->guard = _parcSafeMemory_SuffixGuard;
    return suffix;
}

static void
_backtraceReport(const _MemoryBacktrace *backtrace, int outputFd)
{
    if (outputFd != -1) {
        // Ignore the first entry as it points to this function and we just need to start at the calling function.
        backtrace_symbols_fd(&backtrace->callstack[1], backtrace->actualFrameCount - 1, outputFd);
    }
}

// This is a list of all memory allocations that were created by calls to Safe Memory.
// Each element of the list is the pointer to the result returned to the caller of the memory allocation,
// not a pointer to the base.
struct safememory_entry {
    LIST_ENTRY(safememory_entry) entries;      // List
    void *memory;
};
LIST_HEAD(, safememory_entry) head = LIST_HEAD_INITIALIZER(head);
static pthread_mutex_t head_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
_parcSafeMemory_AddAllocation(void *memory)
{
    if (parcSafeMemory_Outstanding() == 0) {
        LIST_INIT(&head); // Initialize the list.
    }

    struct safememory_entry *e = malloc(sizeof(struct safememory_entry)); // Insert this at the head.
    e->memory = memory;

    pthread_mutex_lock(&head_mutex);
    LIST_INSERT_HEAD(&head, e, entries);
    pthread_mutex_unlock(&head_mutex);
}

static void
_parcSafeMemory_RemoveAllocation(void *memory)
{
    struct safememory_entry *e;
    pthread_mutex_lock(&head_mutex);
    LIST_FOREACH(e, &head, entries)
    {
        if (e->memory == memory) {
            LIST_REMOVE(e, entries);
            free(e);
            pthread_mutex_unlock(&head_mutex);
            return;
        }
    }

    pthread_mutex_unlock(&head_mutex);
    fprintf(stderr, "parcSafeMemory_RemoveAllocation: Destroying memory (%p) which is NOT in the allocated memory record. Double free?\n", memory);
}

static PARCSafeMemoryState
_parcSafeMemory_GetState(const PARCSafeMemoryUsable *memory)
{
    PARCSafeMemoryState prefixState = _parcSafeMemory_GetPrefixState(memory);
    if (prefixState != PARCSafeMemoryState_OK) {
        return prefixState;
    }
    return _parcSafeMemory_GetSuffixState(memory);
}

static const char *
_parcSafeMemory_StateToString(PARCSafeMemoryState status)
{
    switch (status) {
        case PARCSafeMemoryState_OK:                return "OK";
        case PARCSafeMemoryState_MISMATCHED:        return "MISMATCHED";
        case PARCSafeMemoryState_UNDERRUN:          return "UNDERRUN";
        case PARCSafeMemoryState_OVERRUN:           return "OVERRUN";
        case PARCSafeMemoryState_NOTHINGALLOCATED:  return "NOTHINGALLOCATED";
        case PARCSafeMemoryState_ALREADYFREE:       return "ALREADYFREE";
    }
    return "?";
}

static void
_parcSafeMemory_Report(const PARCSafeMemoryUsable *safeMemory, int outputFd)
{
    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(safeMemory);

    if (outputFd != -1) {
        int charactersPrinted = dprintf(outputFd, "Memory %p (base %p) %s\n",
                                        (void *) safeMemory,
                                        (void *) prefix,
                                        _parcSafeMemory_StateToString(_parcSafeMemory_GetState(safeMemory)));
        parcTrapUnexpectedStateIf(charactersPrinted < 0, "Cannot write to file descriptor %d", outputFd);
    }
    _backtraceReport(prefix->backtrace, outputFd);
}

uint32_t
parcSafeMemory_ReportAllocation(int outputFd)
{
    uint32_t index = 0;
    struct safememory_entry *e;

    pthread_mutex_lock(&head_mutex);
    LIST_FOREACH(e, &head, entries)
    {
        _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(e->memory);
        if (outputFd != -1) {
            int charactersPrinted = dprintf(outputFd,
                                            "\n%u SafeMemory@%p: %p={ .requestedLength=%zd, .actualLength=%zd, .alignment=%zd }\n",
                                            index, e->memory, (void *) prefix, prefix->requestedLength, prefix->actualLength, prefix->alignment);
            parcTrapUnexpectedStateIf(charactersPrinted < 0, "Cannot write to file descriptor %d", outputFd)
            {
                pthread_mutex_unlock(&head_mutex);
            }
        }
        _parcSafeMemory_Report(e->memory, outputFd);
        index++;
    }
    pthread_mutex_unlock(&head_mutex);
    return parcSafeMemory_Outstanding();
}

static void
_backtraceDestroy(_MemoryBacktrace **backtrace)
{
    free((*backtrace)->callstack);
    free(*backtrace);
    *backtrace = 0;
}

static PARCSafeMemoryState
_parcSafeMemory_Destroy(void **memoryPointer)
{
    pthread_mutex_lock(&_parcSafeMemory_Mutex);

    if (parcSafeMemory_Outstanding() == 0) {
        pthread_mutex_unlock(&_parcSafeMemory_Mutex);
        return PARCSafeMemoryState_NOTHINGALLOCATED;
    }

    _parcSafeMemory_RemoveAllocation(*memoryPointer);

    PARCSafeMemoryUsable *memory = *memoryPointer;

    PARCSafeMemoryState state = _parcSafeMemory_GetState(memory);
    parcTrapUnexpectedStateIf(state != PARCSafeMemoryState_OK,
                          "Expected PARCSafeMemoryState_OK, actual %s (see parc_SafeMemory.h)",
                          _parcSafeMemory_StateToString(state))
    {
        pthread_mutex_unlock(&_parcSafeMemory_Mutex);
    }

    _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(memory);
    _backtraceDestroy(&prefix->backtrace);

    PARCSafeMemoryOrigin *base = _parcSafeMemory_GetOrigin(memory);

    memset(base, 0, _computeMemoryTotalLength(prefix->requestedLength, prefix->alignment));
    prefix->guard = _parcSafeMemory_GuardAlreadyFreed;

    ((PARCMemoryDeallocate *) _parcMemory->Deallocate)((void **) &base);

    *memoryPointer = 0;

    pthread_mutex_unlock(&_parcSafeMemory_Mutex);
    return PARCSafeMemoryState_OK;
}

__attribute__((unused))
static void
_parcSafeMemory_DeallocateAll(void)
{
    struct safememory_entry *e;

    pthread_mutex_lock(&head_mutex);
    LIST_FOREACH(e, &head, entries)
    {
        _parcSafeMemory_Destroy(&e->memory);
    }
    pthread_mutex_unlock(&head_mutex);
}

static _MemoryBacktrace *
_backtraceCreate(int maximumFrameCount)
{
    _MemoryBacktrace *result = malloc(sizeof(_MemoryBacktrace));
    result->maximumFrameCount = maximumFrameCount;
    result->callstack = calloc(result->maximumFrameCount, sizeof(void *));

    result->actualFrameCount = backtrace(result->callstack, result->maximumFrameCount);

    return result;
}

/**
 * Format memory with a MemoryPrefix structure.
 *
 * @param origin The origin of the allocated memory (which is not the same as the start of usable memory).
 * @param requestedLength The length of the extent of memory for general purpose use by the caller.
 * @param alignment A power of 2 greater than or equal to {@code sizeof(void *)}.
 * @return The pointer to the first address suitable for general purpose use by the caller.
 */
static PARCSafeMemoryUsable *
_parcSafeMemory_FormatPrefix(PARCSafeMemoryOrigin *origin, size_t requestedLength, size_t alignment)
{
    int backTraceDepth = 20;

    if (!_alignmentIsValid(alignment)) {
        return NULL;
    }
    size_t prefixSize = _computePrefixLength(alignment);

    // This abuts the prefix to the user memory, it does not start at the beginning
    // of the aligned prefix region.
    _MemoryPrefix *prefix = (_MemoryPrefix *) (origin + (prefixSize - sizeof(_MemoryPrefix)));

    prefix->magic = _parcSafeMemory_PrefixMagic;
    prefix->requestedLength = requestedLength;
    prefix->actualLength = _computeUsableMemoryLength(requestedLength, sizeof(void*));
    prefix->alignment = alignment;
    prefix->backtrace = _backtraceCreate(backTraceDepth);
    prefix->guard = _parcSafeMemory_Guard;

    PARCSafeMemoryUsable *result = _pointerAdd(origin, prefixSize);

    parcAssertAligned(result, alignment, "Return value is not properly aligned to %zu", alignment);
    return result;
}

/**
 * Given a pointer to allocated memory and the length of bytes that will be used by the caller,
 * format the prefix and suffix structures returning a pointer to the first properly aligned
 * byte available to the client function.
 */
static void *
_parcSafeMemory_FormatMemory(PARCSafeMemoryOrigin *origin, size_t length, size_t alignment)
{
    PARCSafeMemoryUsable *memory = _parcSafeMemory_FormatPrefix(origin, length, alignment);
    if (memory != NULL) {
        _parcSafeMemory_FormatSuffix(memory);
    }

    return memory;
}

int
parcSafeMemory_MemAlign(void **memptr, size_t alignment, size_t requestedSize)
{
    if (!_alignmentIsValid(alignment)) {
        return EINVAL;
    }
    if (requestedSize == 0) {
        return EINVAL;
    }

    size_t totalSize = _computeMemoryTotalLength(requestedSize, alignment);
    if (totalSize < requestedSize) {
        return ERANGE;
    }

    pthread_mutex_lock(&_parcSafeMemory_Mutex);

    void *base;
    int failure = ((PARCMemoryMemAlign *) _parcMemory->MemAlign)(&base, alignment, totalSize);

    if (failure != 0 || base == NULL) {
        pthread_mutex_unlock(&_parcSafeMemory_Mutex);
        return ENOMEM;
    }

    *memptr = _parcSafeMemory_FormatMemory(base, requestedSize, alignment);

    _parcSafeMemory_AddAllocation(*memptr);
    pthread_mutex_unlock(&_parcSafeMemory_Mutex);

    return 0;
}

void *
parcSafeMemory_Allocate(size_t requestedSize)
{
    void *result = NULL;

    if (requestedSize != 0) {
        size_t totalSize = _computeMemoryTotalLength(requestedSize, sizeof(void *));

        if (totalSize >= requestedSize) {
            pthread_mutex_lock(&_parcSafeMemory_Mutex);

            void *base = ((PARCMemoryAllocate *) _parcMemory->Allocate)(totalSize);
            if (base != NULL) {
                result = _parcSafeMemory_FormatMemory(base, requestedSize, sizeof(void *));

                _parcSafeMemory_AddAllocation(result);
            }
            pthread_mutex_unlock(&_parcSafeMemory_Mutex);
        }
    }
    return result;
}

void *
parcSafeMemory_AllocateAndClear(size_t requestedSize)
{
    void *memptr = parcSafeMemory_Allocate(requestedSize);
    if (memptr != NULL) {
        memset(memptr, 0, requestedSize);
    }
    return memptr;
}

bool
parcSafeMemory_IsValid(const void *memory)
{
    bool result = true;

    PARCSafeMemoryState state = _parcSafeMemory_GetState(memory);
    if (state != PARCSafeMemoryState_OK) {
        return false;
    }

    return result;
}


uint32_t
parcSafeMemory_Outstanding(void)
{
    return ((PARCMemoryOutstanding *) _parcMemory->Outstanding)();
}

void *
parcSafeMemory_Reallocate(void *original, size_t newSize)
{
    void *result;

    result = parcSafeMemory_Allocate(newSize);

    if (original == NULL) {
        return result;
    }

    if (result != NULL) {
        _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(original);
        size_t originalSize = prefix->requestedLength;

        memcpy(result, original, originalSize);
        parcSafeMemory_Deallocate(&original);
    }
    return result;
}

char *
parcSafeMemory_StringDuplicate(const char *string, size_t length)
{
    size_t actualLength = strlen(string);
    if (length < actualLength) {
        actualLength = length;
    }

    char *result = parcSafeMemory_Allocate(actualLength + 1);
    if (result != NULL) {
        memcpy(result, string, actualLength);
        result[actualLength] = 0;
    }
    return result;
}

void
parcSafeMemory_Deallocate(void **pointer)
{
    _parcSafeMemory_Destroy(pointer);
}

void
parcSafeMemory_Display(const void *memory, int indentation)
{
    if (memory == NULL) {
        parcDisplayIndented_PrintLine(indentation, "PARCSafeMemory@NULL");
    } else {
        _MemoryPrefix *prefix = _parcSafeMemory_GetPrefix(memory);

        parcDisplayIndented_PrintLine(indentation, "PARCSafeMemory@%p {", (void *) memory);
        parcDisplayIndented_PrintLine(indentation + 1,
                                      "%p=[ magic=0x%" PRIx64 " requestedLength=%zd, actualLength=%zd, alignment=%zd, guard=0x%" PRIx64 "]",
                                      _parcSafeMemory_GetOrigin(memory),
                                      prefix->magic,
                                      prefix->requestedLength,
                                      prefix->actualLength,
                                      prefix->alignment,
                                      prefix->guard);

        parcDisplayIndented_PrintMemory(indentation + 1, prefix->requestedLength, memory);

        parcDisplayIndented_PrintLine(indentation, "}");
    }
}

PARCMemoryInterface PARCSafeMemoryAsPARCMemory = {
    .Allocate         = (uintptr_t) parcSafeMemory_Allocate,
    .AllocateAndClear = (uintptr_t) parcSafeMemory_AllocateAndClear,
    .MemAlign         = (uintptr_t) parcSafeMemory_MemAlign,
    .Deallocate       = (uintptr_t) parcSafeMemory_Deallocate,
    .Reallocate       = (uintptr_t) parcSafeMemory_Reallocate,
    .Outstanding      = (uintptr_t) parcSafeMemory_Outstanding,
    .StringDuplicate  = (uintptr_t) parcSafeMemory_StringDuplicate
};
