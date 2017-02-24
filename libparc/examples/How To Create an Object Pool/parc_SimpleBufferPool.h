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
 * @file parc_BufferPool.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_BufferPool
#define PARCLibrary_parc_BufferPool
#include <stdbool.h>

#include <parc/algol/parc_Object.h>

parcObject_Declare(PARCSimpleBufferPool);

/**
 * Increase the number of references to a `PARCBufferPool` instance.
 *
 * Note that new `PARCBufferPool` is not created,
 * only that the given `PARCBufferPool` reference count is incremented.
 * Discard the reference by invoking `parcSimpleBufferPool_Release`.
 *
 * @param [in] instance A pointer to a valid PARCBufferPool instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcSimpleBufferPool_Create();
 *
 *     PARCBufferPool *b = parcSimpleBufferPool_Acquire();
 *
 *     parcSimpleBufferPool_Release(&a);
 *     parcSimpleBufferPool_Release(&b);
 * }
 * @endcode
 */
PARCSimpleBufferPool *parcSimpleBufferPool_Acquire(const PARCSimpleBufferPool *instance);

/**
 * Create an instance of PARCBufferPool
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCBufferPool instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcSimpleBufferPool_Create();
 *
 *     parcSimpleBufferPool_Release(&a);
 * }
 * @endcode
 */
PARCSimpleBufferPool *parcSimpleBufferPool_Create(size_t highWater, size_t bufferSize);

/**
 * Release a previously acquired reference to the given `PARCBufferPool` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCSimpleBufferPool *a = parcSimpleBufferPool_Create();
 *
 *     parcSimpleBufferPool_Release(&a);
 * }
 * @endcode
 */
void parcSimpleBufferPool_Release(PARCSimpleBufferPool **instancePtr);

PARCBuffer *parcSimpleBufferPool_GetInstance(PARCSimpleBufferPool *bufferPool);

#endif
