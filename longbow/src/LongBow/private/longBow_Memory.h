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
 * @file longBow_Memory.c
 * @ingroup internal
 * @brief Memory allocation and deallocation support.
 *
 */
#ifndef LongBow_longBow_Memory_h
#define LongBow_longBow_Memory_h

#include <stdint.h>

/**
 * Allocate `size` bytes of memory.
 *
 * @param [in] size The number of bytes to allocate
 *
 * @return non-NULL A pointer to allocated memory that must be deallocated via `longBowMemory_Deallocate`
 * @return NULL Memory could not be allocated.
 * @see longBowMemory_Deallocate
 */
void *longBowMemory_Allocate(const size_t size);

/**
 * Reallocate memory adjusting to a new size.
 *
 * @param [in] oldAllocation A pointer to memory previously allocated by `longBowMemory_Allocate` or `longBowMemory_Reallocate`
 * @param [in] newSize The number of bytes to allocate
 *
 * @return non-NULL A pointer to allocated memory that must be deallocated via `longBowMemory_Deallocate`
 * @return NULL Memory could not be allocated.
 * @see longBowMemory_Allocate
 */
void *longBowMemory_Reallocate(void *oldAllocation, const size_t newSize);

/**
 * Deallocate previously allocated memory.
 *
 * @param [in,out] pointerPointer A pointer to a pointer to allocated memory that will set to NULL.
 *
 * @see longBowMemory_Allocate
 */
void longBowMemory_Deallocate(void **pointerPointer);

/**
 * Duplicate a nul-terminated C string in allocated memory.
 *
 * @param [in] string The nul-terminated string to duplicate
 *
 * @return non-NULL A pointer to allocated memory that must be deallocated via `longBowMemory_Deallocate`
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 *     char *copy = longBowMemory_StringCopy("Hello World");
 *
 *     longBowMemory_Deallocate((void **) &copy);
 * }
 * @endcode
 *
 * @see longBowMemory_Deallocate
 */
char *longBowMemory_StringCopy(const char *string);

/**
 * Get the count of outstanding memory allocations.
 *
 * @return The number of outstanding memory allocations.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t longBowMemory_OutstandingAllocations(void);
#endif
