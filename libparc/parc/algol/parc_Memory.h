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
 * @file parc_Memory.h
 * @ingroup memory
 * @brief A Facade to memory allocation features.
 *
 * PARC Memory provides an interface implementing many regularly available memory allocation functions.
 * This interface is a Facade that software implementors may use to substitute different kinds of underlying
 * Interfaces of these allocation fucntions.
 * Notable examples are PARC Safe Memory and PARC Stdlib Memory.
 *
 */
#ifndef libparc_parc_Memory_h
#define libparc_parc_Memory_h

#include <stdlib.h>
#include <stdint.h>

/**
 * @typedef PARCMemoryAllocate
 * @brief Function signature for memory allocator.
 *
 */
typedef void *(PARCMemoryAllocate)(size_t size);

typedef void *(PARCMemoryAllocateAndClear)(size_t size);

typedef int (PARCMemoryMemAlign)(void **pointer, size_t alignment, size_t size);

typedef void (PARCMemoryDeallocate)(void **pointer);

typedef void *(PARCMemoryReallocate)(void *pointer, size_t newSize);

typedef char *(PARCMemoryStringDuplicate)(const char *string, size_t length);

typedef uint32_t (PARCMemoryOutstanding)(void);

/**
 * @typedef PARCMemoryInterface
 * @brief A structure containing pointers to functions that implement a PARC Memory manager.
 *
 * A 'PARC Memory' manager is a collection of inter-dependant functions that perform memory allocation,
 * re-allocation, deallocation, and housekeeping.
 *
 * PARC Memory managers are cascadable, where one Interface may call other Interface in a chain.
 * This permits the design and Interface of PARC Memory managers that specialise in fixed length memory sizes,
 * reference counting, debugging and so forth.
 */
typedef struct parc_memory_interface {
    /**
     * A pointer to a function that allocates @p size bytes of memory
     * and returns the allocation in the return value.
     *
     * @param [in] size The number of bytes to allocate.
     *
     * @return A `void *` pointer indicating the address of the allocated memory.
     * @return NULL Memory allocation error.
     *
     * @see AllocateAndClear
     * @see Reallocate
     */
    uintptr_t Allocate;

    /**
     * Performs the same operation as `Allocate` and then sets each byte of the allocated memory to zero.
     *
     * @param [in] size The number of bytes to allocate.
     *
     * @return A `void *` pointer indicating the address of the allocated memory.
     * @return NULL Memory allocation error.
     *
     * @see Allocate
     * @see Reallocate
     */
    uintptr_t AllocateAndClear;

    /**
     * A pointer to a function that allocates @p size bytes of memory such that the allocation's
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
     * @see posix_memalign
     */
    uintptr_t MemAlign;

    /**
     * Deallocate memory previously allocated via `Allocate` or `AllocateAndClear`.
     *
     * @param [in,out] pointer A pointer to a `void *` pointer to the address of the allocated memory that will be set to zero.
     *
     * @see AllocateAndClear
     * @see Allocate
     * @see Reallocate
     */
    uintptr_t Deallocate;

    /**
     * Try to change the size of the allocation pointed to by @p pointer to @p newSize, and returns ptr.
     * If there is not enough room to enlarge the memory allocation pointed to by @p pointer,
     * create a new allocation,
     * copy as much of the old data pointed to by @p pointer as will fit to the new allocation,
     * deallocate the old allocation,
     * and return a pointer to the allocated memory.
     *
     * If @p pointer is `NULL`,
     * simply invoke the `Allocate` function to allocate memory aligned to the value of `sizeof(void *)` of @p newSize bytes.
     * If @p newSize is zero and @p pointer is not NULL,
     * a new, minimum sized object is allocated and the original object is freed.
     *
     * When extending a region previously allocated with `AllocateAndClear`,
     * the additional memory is not guaranteed to be zero-filled.
     *
     * @param [in] pointer A pointer to previously allocated memory, or NULL.
     * @param [in] newSize The size of the allocated memory.
     *
     * @return non-NULL A pointer to allocated memory.
     * @return NULL A an error occurred.
     *
     * @see Deallocate
     * @see AllocateAndClear
     * @see Allocate
     */
    uintptr_t Reallocate;

    /**
     * Allocate sufficient memory for a copy of the string @p string,
     * copy at most n characters from the string @p string into the allocated memory,
     * and return the pointer to allocated memory.
     *
     * The copied string is always null-terminated.
     *
     * @param [in] string A pointer to a null-terminated string.
     * @param [length] The maximum allows length of the resulting copy.
     *
     * @return non-NULL A pointer to allocated memory.
     * @return NULL A an error occurred.
     */
    uintptr_t StringDuplicate;

    /**
     * Return the number of allocations outstanding. That is, the numbe of allocations
     * that have been made, but not yet freed.
     *
     * @return The number of outstanding allocations known to this `PARCMemoryInterface`.
     */
    uintptr_t Outstanding;
} PARCMemoryInterface;

/**
 *
 */
extern PARCMemoryInterface PARCMemoryAsPARCMemory;

/**
 * Set the current memory allocation interface.
 *
 * The previous interface is returned.
 *
 * @param [in] memoryProvider A pointer to a {@link PARCMemoryInterface} instance.
 *
 * @return A pointer to the previous `PARCMemoryInterface` instance.
 *
 * Example:
 * @code
 * {
 *     PARCMemoryInterface *previousInterface = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
 * }
 * @endcode
 *
 * @see PARCSafeMemoryAsPARCMemory
 * @see PARCMemoryAsPARCMemory
 * @see PARCStdlibMemoryAsPARCMemory
 */
const PARCMemoryInterface *parcMemory_SetInterface(const PARCMemoryInterface *memoryProvider);

/**
 * Allocate memory.
 *
 * Allocates @p size bytes of memory and returns the allocation in the return value.
 *
 * Memory that is allocated can be used as an argument in subsequent call `Reallocate`.
 *
 * @param [in] size The number of bytes to allocate.
 *
 * @return A `void *` pointer set to the address of the allocated memory.
 * @return NULL Memory allocation error.
 *
 * Example:
 * @code
 * {
 *     void *allocatedMemory;
 *
 *     allocateMemory = parcMemory_Allocate(100);
 *     if (allocatedMemory == NULL) {
 *         // allocation failed.
 *     }
 * }
 * @endcode
 */
void *parcMemory_Allocate(const size_t size);

/**
 * Performs the same operation as `Allocate` and then sets each byte of the allocated memory to zero.
 *
 * @param [in] size The number of bytes to allocate.
 *
 * @return A `void *` pointer set to the address of the allocated memory.
 * @return NULL Memory allocation error.
 *
 * Example:
 * @code
 * {
 *     void *allocatedMemory;
 *
 *     allocatedMemory = parcMemory_AllocateAndClear(100);
 *     if (allocatedMemory == NULL)
 *         // allocation failed
 *     }
 * }
 * @endcode
 *
 * @see parcMemory_Allocate
 */
void *parcMemory_AllocateAndClear(const size_t size);

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
 *     int failure = parcMemory_MemAlign(&allocatedMemory, sizeof(void *), 100);
 *     if (failure == 0) {
 *         parcMemory_Deallocate(&allocatedMemory);
 *         // allocatedMemory is now equal to zero.
 *     }
 * }
 * @endcode
 * @see `posix_memalign`
 */
int parcMemory_MemAlign(void **pointer, const size_t alignment, const size_t size);

/**
 * Deallocate memory previously allocated via `Allocate` or `AllocateAndClear`.
 *
 * @param [in,out] pointer A pointer to a `void *` pointer to the address of the allocated memory that will be set to zero.
 *
 * Example:
 * @code
 * {
 *     void *allocatedMemory;
 *
 *     allocatedMemory = parcMemory_Allocate(100);
 *     if (allocatedMemory == NULL) {
 *         // allocation failed
 *     }
 * }
 * @endcode
 *
 * @see parcMemory_Allocate
 * @see parcMemory_AllocateAndClear
 */
void parcMemory_DeallocateImpl(void **pointer);

#define parcMemory_Deallocate(_pointer_) parcMemory_DeallocateImpl((void **) _pointer_)

/**
 * Try to change the size of the allocation pointed to by @p pointer to @p newSize, and returns ptr.
 * If there is not enough room to enlarge the memory allocation pointed to by @p pointer,
 * create a new allocation,
 * copy as much of the old data pointed to by @p pointer as will fit to the new allocation,
 * deallocate the old allocation,
 * and return a pointer to the allocated memory.
 *
 * If @p pointer is `NULL`,
 * simply invoke the {@link parcMemory_Allocate} function to allocate memory aligned to the value of `sizeof(void *)` of @p newSize bytes.
 * If @p newSize is zero and @p pointer is not NULL,
 * a new, minimum sized object is allocated and the original object is freed.
 *
 * When extending a region previously allocated with `AllocateAndClear`,
 * the additional memory is not guaranteed to be zero-filled.
 *
 * @param [in] pointer A pointer to previously allocated memory, or NULL.
 * @param [in] newSize The size of the allocated memory.
 *
 * @return non-NULL A pointer to allocated memory.
 * @return NULL A an error occurred.
 *
 * Example:
 * @code
 * {
 *     void *allocatedMemory;
 *
 *     allocateMemory = parcMemory_Allocate(100);
 *
 *     allocatedMemory = parcMemory_Reallocate(allocatedMemory, sizeof(void *), 200);
 *
 *     parcMemory_Deallocate(&allocatedMemory);
 * }
 * @endcode
 *
 * @see parcMemory_Deallocate
 * @see parcMemory_Allocate
 */
void *parcMemory_Reallocate(void *pointer, size_t newSize);

/**
 * Allocate sufficient memory for a copy of the string @p string,
 * copy at most n characters from the string @p string into the allocated memory,
 * and return the pointer to allocated memory.
 *
 * The copied string is always null-terminated.
 *
 * @param [in] string A pointer to a null-terminated string.
 * @param [in] length  The maximum allowed length of the resulting copy.
 *
 * @return non-NULL A pointer to allocated memory.
 * @return NULL A an error occurred.
 *
 * Example:
 * @code
 * {
 *     char *string = "this is a string";
 *     char *copy = parcMemory_StringDuplicate(string, strlen(string));
 *
 *     if (copy != NULL) {
 *         . . .
 *         parcMemory_Deallocate(&copy);
 *     }
 * }
 * @endcode
 *
 * @see {@link parcMemory_Deallocate()}
 */
char *parcMemory_StringDuplicate(const char *string, const size_t length);

/**
 * Return the number of outstanding allocations managed by this allocator.
 *
 * When you allocate memory, this count goes up by one. When you deallocate, it goes down by one.
 * A well-behaved program will terminate with a call to parcMemory_Outstanding() returning 0.
 *
 * @return The number of memory allocations still outstanding (remaining to be deallocated).
 *
 * Example:
 * @code
 * {
 *     uint32_t numberOfAllocations = parcMemory_Outstanding();
 * }
 * @endcode
 */
uint32_t parcMemory_Outstanding(void);

/**
 * Round up a given number of bytes to be a multiple of the cache line size on the target computer.
 *
 * @param [in] size The number of bytes to round up.
 *
 * @return The number of bytes that are a multiple of the cache line size on the target computer.
 *
 * Example:
 * @code
 * {
 *     size_t size = parcMemory_RoundUpToCacheLine(14);
 * }
 * @endcode
 *
 * @see parcMemory_RoundUpToMultiple
 */
size_t parcMemory_RoundUpToCacheLine(const size_t size);

/**
 * Round up a given number of bytes to be an even multiple.
 *
 * @param [in] size The number of bytes to round up.
 * @param [in] multiple The number of bytes to round up.
 *
 * @return The number of bytes that are an even multiple of @p multiple.
 *
 * Example:
 * @code
 * {
 *     size_t size = parcMemory_RoundUp(14, 20);
 * }
 * @endcode
 *
 * @see parcMemory_RoundUpToCacheLine
 */
size_t parcMemory_RoundUpToMultiple(size_t size, size_t multiple);

/**
 * @def parcMemory_SafeFree
 *
 * Free only non-null pointers to memory
 *
 * @param memory A pointer to allocated memory
 *
 */
#define parcMemory_SafeFree(memory) do { if (memory != NULL) { parcMemory_Deallocate(& (memory)); } } while (0)

/**
 * Allocate a printf(3) style formatted string.
 * The result must be deallocated via `parcMemory_Deallocate`
 *
 * This function is equivalent to the `asprintf(3)` function in the standard library.
 *
 * @param [in] format A pointer to nul-terminated C string containing a printf style format specification.
 *
 * @return non-NULL A pointer to allocated memory containing the formatted string.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     char *string = parcMemory_Format("Hello %s", "World");
 *
 *     parcMemory_Deallocated(&string);
 * }
 * @endcode
 */
char *parcMemory_Format(const char *format, ...);
#endif // libparc_parc_Memory_h
