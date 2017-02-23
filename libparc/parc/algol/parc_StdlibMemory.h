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
 * @file parc_StdlibMemory.h
 * @ingroup datastructures
 *
 * @brief Standard library memory mangement functions wrapped up to be suitable for use by parc_Memory.[ch]
 *
 */
#ifndef libparc_parc_StdlibMemory_h
#define libparc_parc_StdlibMemory_h

#include <parc/algol/parc_Memory.h>

extern PARCMemoryInterface PARCStdlibMemoryAsPARCMemory;

/**
 * Allocate memory.
 *
 * @param [in] size The size of memory to allocate
 *
 * @return A pointer to the allocated memory.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcStdlibMemory_Allocate(size_t size);

/**
 * Allocate memory of size @p size and clear it.
 *
 * @param [in] size Size of memory to allocate
 *
 * @return A pointer to the allocated memory
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcStdlibMemory_AllocateAndClear(size_t size);

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
 *     int failure = parcStdlibMemory_MemAlign(&allocatedMemory, sizeof(void *), 100);
 *     if (failure == 0) {
 *         parcStdlibMemory_Deallocate(&allocatedMemory);
 *         // allocatedMemory is now equal to zero.
 *     }
 * }
 * @endcode
 * @see {@link parcMemory_MemAlign}
 */
int parcStdlibMemory_MemAlign(void **pointer, size_t alignment, size_t size);

/**
 * Deallocate the memory pointed to by @p pointer
 *
 * @param [in,out] pointer A pointer to a pointer to the memory to be deallocated
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcStdlibMemory_Deallocate(void **pointer);

/**
 * Resizes previously allocated memory at @p pointer to @p newSize. If necessary,
 * new memory is allocated and the content copied from the old memory to the
 * new memory and the old memory is deallocated.
 *
 * @param [in,out] pointer A pointer to the memory to be reallocated.
 * @param [in] newSize The size that the memory to be resized to.
 *
 * @return A pointer to the memory
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcStdlibMemory_Reallocate(void *pointer, size_t newSize);

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
 *     char *copy = parcStdlibMemory_StringDuplicate(string, strlen(string));
 *
 *     if (copy != NULL) {
 *         . . .
 *         parcStdLibMemory_Deallocate(&copy);
 *     }
 * }
 * @endcode
 *
 * @see {@link parcStdlibMemory_Deallocate()}
 */
char *parcStdlibMemory_StringDuplicate(const char *string, size_t length);

/**
 * Return the number of outstanding allocations managed by this allocator.
 *
 * When you allocate memory, this count goes up by one. When you deallocate, it goes down by one.
 * A well-behaved program will terminate with a call to `parcStdlibMemory_Outstanding()` returning 0.
 *
 * @return The number of memory allocations still outstanding (remaining to be deallocated).
 *
 * Example:
 * @code
 * {
 *     uint32_t numberOfAllocations = parcStdlibMemory_Outstanding();
 * }
 * @endcode
 */
uint32_t parcStdlibMemory_Outstanding(void);


/**
 * Replacement function for realloc(3).
 *
 * The standard `realloc()` function tries to change the size of the allocation pointed to by @p oldAlloc to @p newSize,
 * and returns @p oldAlloc.  If there is not enough room to enlarge the memory allocation pointed to by @p oldAlloc,
 * realloc() creates a new allocation, copies as much of the old data pointed to by @p oldAlloc as will fit to the new allocation,
 * frees the old allocation, and returns a pointer to the allocated memory.
 *
 * If @p oldAlloc is `NULL`, `realloc()` is identical to a call to `malloc()` for size bytes.
 * If @p newSize is zero and @p oldAlloc is not NULL, a new, minimum sized object is allocated and the original object is freed.
 * When extending a region allocated with `calloc(3)`, `realloc(3)` does not guarantee that the additional memory
 * is also zero-filled.
 *
 * If the realloc(3) function is not compatible with the above constraints
 * (i.e., ‘realloc(NULL, 0)’ returns an invalid pointer), then autoconf tools will define
 * `realloc` to `rpl_realloc` so that the native realloc is not used in the main project.
 *
 * @param [in] oldAlloc A previously allocated
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void *rpl_realloc(void *oldAlloc, size_t newSize);
#endif // libparc_parc_StdlibMemory_h
