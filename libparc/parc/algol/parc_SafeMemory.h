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
 * @file parc_SafeMemory.h
 * @ingroup developer
 *
 * @brief Facade to memory functions to make calls safer.
 *
 * This is a facade to interpose between an application and the standard C library functions posix_memalign(3),
 * malloc(3), calloc(3), realloc(3) and free(3) that establishes detectable boundaries around an allocated memory segment,
 * records a stack backtrace for each allocation,
 * detects buffer overruns and underruns by checking the boundaries when the memory is deallocated,
 * and tries to prevent a stray pointer to reference the memory again once it's been deallocated.
 *
 * The allocated memory consists of three contiguous segments: the prefix, the memory usable by the caller, and the suffix.
 * The memory usable by the caller is aligned as specified by the caller.
 * The alignment must be a power of 2 greater than or equal to the size of a {@code void *}.
 * <pre>
 * +--base  +-prefix     +-- aligned memory   +-- suffix aligned on (void *)
 * v        v            v                    v
 * |________|PPPPPPPPPPPP|mmmmmmmmm...mmmm|___|SSSSSSSSS
 *                                         ^
 *                                         +-- variable padding
 * </pre>
 * Where '-' indicates padding, 'P' indicates the prefix data structure, 'm'
 * indicates contiguous memory for use by the caller, and 'S" indicates the suffix data structure.
 *
 * To enable this facade, you must include the following line in your execution before any allocations are performed.
 *
 * @code
 * {
 *     parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
 * }
 * @endcode
 *
 */
#ifndef libparc_parc_SafeMemory_h
#define libparc_parc_SafeMemory_h

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <parc/algol/parc_Memory.h>

typedef enum parc_safety_memory_state {
    PARCSafeMemoryState_OK = 0,
    PARCSafeMemoryState_MISMATCHED = 1,
    PARCSafeMemoryState_UNDERRUN = 2,
    PARCSafeMemoryState_OVERRUN = 3,
    PARCSafeMemoryState_NOTHINGALLOCATED = 4,
    PARCSafeMemoryState_ALREADYFREE = 5
} PARCSafeMemoryState;

/**
 * Generate a readable, null-terminated C string representation
 * for the specified `PARCSafeMemoryState` type.
 *
 * @param [in] status A `PARCSafeMemoryState` value.
 *
 * @return A null-terminated C string that must be freed when done.
 *
 * Example:
 * @code
 * {
 *     size_t size = 100;
 *     uint32_t alignment = sizeof(void *);
 *     void *memory;
 *
 *     memory = parcSafeMemory_Allocate(size);
 *     PARCSafeMemoryState state = parcSafeMemory_Destroy(&memory);
 *
 *     printf("SafeMemoryState = %s\n", parcSafeMemoryState_ToString(state));
 * }
 * @endcode
 */
const char *parcSafeMemoryState_ToString(PARCSafeMemoryState status);

/**
 * Memory operations defined by {@link PARCMemoryInterface}
 * and implemented by the Safe Memory functions.
 */
extern PARCMemoryInterface PARCSafeMemoryAsPARCMemory;

/**
 * Allocate Safe Memory.
 *
 * Allocate memory through the configured memory allocator, setting the environment to track this memory.
 *
 * @param [in] size The number of bytes necessary.
 *
 * @return non-NULL A pointer to allocated memory.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 *     size_t size = 100;
 *     void *memory = parcSafeMemory_Allocate(size);
 * }
 * @endcode
 */
void *parcSafeMemory_Allocate(size_t size);

/**
 * Allocate Safe Memory.
 *
 * Allocate memory through the configured memory allocator, setting the environment to track this memory.
 *
 * @param [in] requestedSize The number of bytes necessary.
 *
 * @return non-NULL A pointer to allocated memory.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 *     size_t size = 100;
 *     void *memory = parcSafeMemory_Allocate(size);
 * }
 * @endcode
 */
void *parcSafeMemory_AllocateAndClear(size_t requestedSize);

/**
 * Allocate aligned memory.
 *
 * Allocates @p size bytes of memory such that the allocation's
 * base address is an exact multiple of alignment,
 * and returns the allocation in the value pointed to by @p pointer.
 *
 * The requested alignment must be a power of 2 greater than or equal to `sizeof(void *)`.
 *
 * Memory that is allocated can be used as an argument in subsequent call `Reallocate`, however
 * `Reallocate` is not guaranteed to preserve the original alignment.
 *
 * @param [out] pointer A pointer to a `void *` pointer that will be set to the address of the allocated memory.
 * @param [in] alignment A power of 2 greater than or equal to `sizeof(void *)`
 * @param [in] size The number of bytes to allocate.
 *
 * @return 0 Successful
 * @return EINVAL The alignment parameter is not a power of 2 at least as large as sizeof(void *)
 * @return ENOMEM Memory allocation error.
 *
 * Example:
 * @code
 * {
 *     void *allocatedMemory;
 *
 *     int failure = parcSafeMemory_MemAlign(&allocatedMemory, sizeof(void *), 100);
 *     if (failure == 0) {
 *         parcSafeMemory_Deallocate(&allocatedMemory);
 *         // allocatedMemory is now equal to zero.
 *     }
 * }
 * @endcode
 * @see `posix_memalign`
 */
int parcSafeMemory_MemAlign(void **pointer, size_t alignment, size_t size);

/**
 * Deallocate memory previously allocated with {@link parcSafeMemory_Allocate}
 *
 * The value pointed to by @p pointer will be set to NULL.
 *
 * @param [in,out] pointer A pointer to a pointer to the allocated memory.
 *
 * Example:
 * @code
 * {
 *     size_t size = 100;
 *     void *memory = parcSafeMemory_Allocate(size);
 *
 *     parcSafeMemory_Deallocate(&memory);
 * }
 * @endcode
 */
void parcSafeMemory_Deallocate(void **pointer);

/**
 * A (mostly) suitable replacement for realloc(3).
 * The primary difference is that it is an error if newSize is zero.
 * If the newSize is equal to the old size, then NULL is returned.
 *
 * @param [in] original Pointer to the original memory
 * @param [in] newSize The size of the newly re-allocated memory.
 *
 * @return Non-NULL A pointer to the newly allocated memory
 * @return NULL An error occurred (newSize == oldSize, or newSize == 0)
 *
 * Example:
 * @code
 * {
 *     void *memory = parcSafeMemory_Allocate(100);
 *
 *     size_t newLength = 0;
 *     unsigned char *newMemory = parcSafeMemory_Reallocate(memory, newLength);
 *
 *     parcAssertTrue(newMemory == NULL, "Expected NULL, actual %p", newMemory);
 * }
 * @endcode
 */
void *parcSafeMemory_Reallocate(void *original, size_t newSize);

/**
 * Duplicate the given null-terminated C string.
 *
 * @param [in] string A pointer to a null-terminated C string.
 * @param [in] length The length of the string, not including the terminating null character.
 *
 * @return non-NULL Allocated Safe Memory containing the duplicate string. This must be freed via `parcSafeMemory_Deallocate`.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 *     char *string = "hello world";
 *     char *actual = parcSafeMemory_StringDuplicate(string, strlen(string));
 *     ...
 * }
 * @endcode
 *
 * @see parcSafeMemory_Deallocate
 */
char *parcSafeMemory_StringDuplicate(const char *string, size_t length);

/**
 * Return the number of outstanding allocations.
 *
 * In practice, every allocation should be matched with a corresponding deallocation.
 * This return the number of allocations that have not been deallocated.
 *
 * @return The number of outstanding allocations.
 *
 */
uint32_t parcSafeMemory_Outstanding(void);

/**
 * Display information about outstanding memory allocations.
 *
 * To enable this function, you must include the following line in your execution before any allocations are performed.
 *
 * @code
 * {
 *     parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
 * }
 * @endcode
 *
 * @param [in] outputFd Output file descriptor.
 *
 * @return The number of currenly outstanding allocations.
 *
 * Example:
 * @code
 * {
 *     parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
 *
 *     ...
 *
 *     FILE *fd = fopen ("log.txt", "w");
 *     size_t outstandingAllocations = parcSafeMemory_ReportAllocation(fd);
 * }
 * @endcode
 */
uint32_t parcSafeMemory_ReportAllocation(int outputFd);

/**
 * Determine if a pointer to Safe Memory is valid.
 *
 * Invalid indicates the memory is overrun or underrun.
 *
 * To enable this function, you must include the following line in your execution before any allocations are performed.
 *
 * @code
 * {
 *     parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
 * }
 * @endcode
 *
 * @param [in] memory A pointer to previously allocated Safe Memory.
 *
 * @return true The memory is valid;
 * @return false The memory is invalid;
 *
 * Example:
 * @code
 * {
 *     void *memory = parcSafeMemory_Allocate(100);
 *     if (parcSafeMemory_IsValid(memory) == false) {
 *         printf("Memory is invalid\n");
 *     }
 * @endcode
 */
bool parcSafeMemory_IsValid(const void *memory);

/**
 * Print a human readable representation of the given PARC Safe Memory array.
 *
 * To enable this function, you must include the following line in your execution before any allocations are performed.
 *
 * @code
 * {
 *     parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
 * }
 * @endcode
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] memory A pointer to the memory to display.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *instance = parcBuffer_Create();
 *
 *     parcBuffer_Display(instance, 0);
 *
 *     parcBuffer_Release(&instance);
 * }
 * @endcode
 */
void parcSafeMemory_Display(const void *memory, int indentation);
#endif // libparc_parc_SafeMemory_h
