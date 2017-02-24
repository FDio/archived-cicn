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
 * @ingroup memory
 * @brief A simple pool of uniformly sized PARCBuffer instances.
 *
 * The client uses `parcBufferPool_GetInstance` to obtain instances which are placed attempted to be placed
 * into the pool when the `PARCBuffer_Release` function is called.
 * The pool has a maxmimum number of instances that it will cache.
 *
 */
#ifndef PARCLibrary_parc_BufferPool
#define PARCLibrary_parc_BufferPool
#include <stdbool.h>

#include <parc/algol/parc_Object.h>

parcObject_Declare(PARCBufferPool);

/**
 * Increase the number of references to a `PARCBufferPool` instance.
 *
 * Note that new `PARCBufferPool` is not created,
 * only that the given `PARCBufferPool` reference count is incremented.
 * Discard the reference by invoking `parcBufferPool_Release`.
 *
 * @param [in] instance A pointer to a valid PARCBufferPool instance.
 *
 * @return The same value as the parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcBufferPool_Create(5, 10);
 *
 *     PARCBufferPool *b = parcBufferPool_Acquire(a);
 *
 *     parcBufferPool_Release(&a);
 *     parcBufferPool_Release(&b);
 * }
 * @endcode
 */
PARCBufferPool *parcBufferPool_Acquire(const PARCBufferPool *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcBufferPool_OptionalAssertValid(_instance_)
#else
#  define parcBufferPool_OptionalAssertValid(_instance_) parcBufferPool_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCBufferPool` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCBufferPool instance.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcBufferPool_Create(5, 10);
 *
 *     parcBufferPool_AssertValid(a);
 *
 *     parcBufferPool_Release(&a);
 * }
 * @endcode
 */
void parcBufferPool_AssertValid(const PARCBufferPool *instance);

/**
 * Create an instance of PARCBufferPool containing instances of `PARCBuffer`.
 *
 * This function is equivalent to invoking
 * @code
 *     PARCBufferPool *a = parcBufferPool_CreateExtending(&parcObject_DescriptorName(PARCBuffer), 5, 10);
 * @endcode
 *
 * The value of @p limit is the maximum number of instances that the pool will cache,
 * and @p bufferSize is the size of the `PARCBuffer` instances cached.
 *
 * @return non-NULL A pointer to a valid PARCBufferPool instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcBufferPool_Create(5, 10);
 *
 *     parcBufferPool_Release(&a);
 * }
 * @endcode
 */
PARCBufferPool *parcBufferPool_Create(size_t limit, size_t bufferSize);

/**
 * Create an instance of PARCBufferPool containing instances of object specified by the given `PARCObjectDescriptor`.
 *
 * The value of @p limit is the maximum number of instances that the pool will cache,
 * and @p bufferSize is the size of the `PARCBuffer` instances cached.
 *
 * This function creates a PARCBufferPool that creates and manages instances of PARCBuffer which may have been extended.
 *
 * @return non-NULL A pointer to a valid PARCBufferPool instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcBufferPool_CreateExtending(&parcObject_DescriptorName(MyPARCBuffer), 5, 10);
 *
 *     parcBufferPool_Release(&a);
 * }
 * @endcode
 */
PARCBufferPool *parcBufferPool_CreateExtending(const PARCObjectDescriptor *originalDescriptor, size_t limit, size_t bufferSize);

/**
 * Print a human readable representation of the given `PARCBufferPool`.
 *
 * @param [in] instance A pointer to a valid PARCBufferPool instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcBufferPool_Create(5, 10);
 *
 *     parcBufferPool_Display(a, 0);
 *
 *     parcBufferPool_Release(&a);
 * }
 * @endcode
 */
void parcBufferPool_Display(const PARCBufferPool *instance, int indentation);

/**
 * Determine if an instance of `PARCBufferPool` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCBufferPool instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *a = parcBufferPool_Create(5, 10);
 *
 *     if (parcBufferPool_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcBufferPool_Release(&a);
 * }
 * @endcode
 *
 */
bool parcBufferPool_IsValid(const PARCBufferPool *instance);

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
 *     PARCBufferPool *a = parcBufferPool_Create(5, 10);
 *
 *     parcBufferPool_Release(&a);
 * }
 * @endcode
 */
void parcBufferPool_Release(PARCBufferPool **instancePtr);

/**
 * Get an instance of a `PARCBuffer`.
 *
 * If the PARCBufferPool contains a cached instance, it will be returned.
 * Otherwise a new instance will be created.
 *
 * Any `PARCBuffer` instance which is later released, will be a candidate for caching by the given `PARCBufferPool`.
 *
 * @param [in] bufferPool A pointer to a valid PARCBufferPool instance.
 *
 * @return non-NULL A pointer to a valid `PARCBuffer`.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *
 *     PARCBufferPool *pool = parcBufferPool_Create(5, 10);
 *     ...
 *
 *     parcBufferPool_Release(&pool);
 * }
 * @endcode
 */
PARCBuffer *parcBufferPool_GetInstance(PARCBufferPool *bufferPool);

/**
 * Set the largest number of buffers the pool will cache.
 *
 * If the new limit is less than the current limit, and the current pool size is greater than the new limit,
 * the number of buffers the pool will cache will decay as they are obtained and released from the pool during use.
 *
 * @param [in] bufferPool A pointer to a valid PARCBufferPool instance.
 * @param [in] limit the largest number of buffers the pool will cache.
 *
 * @return The previous value of the largest number of buffers the pool cached.
 *
 * Example:
 * @code
 * {
 *
 *     PARCBufferPool *pool = parcBufferPool_Create(5, 10);
 *     ...
 *
 *     size_t limit = parcBufferPool_SetLimit(pool, 3);
 *
 *     parcBufferPool_Release(&pool);
 * }
 * @endcode
 */
size_t parcBufferPool_SetLimit(PARCBufferPool *bufferPool, size_t limit);

/**
 * Get the largest number of buffers the pool will cache.
 *
 * @param [in] bufferPool A pointer to a valid PARCBufferPool instance.
 *
 * @return The value of the largest number of buffers the pool will cache.
 *
 * Example:
 * @code
 * {
 *
 *     PARCBufferPool *pool = parcBufferPool_Create(5, 10);
 *     ...
 *
 *     size_t limit = parcBufferPool_GetLimit(pool);
 *
 *     parcBufferPool_Release(&pool);
 * }
 * @endcode
 */
size_t parcBufferPool_GetLimit(const PARCBufferPool *bufferPool);

/**
 * Get the current number of buffers the pool has cached.
 *
 * The value is always greater than or equal to 0 and less than or equal to the limit.
 *
 * @param [in] bufferPool A pointer to a valid PARCBufferPool instance.
 *
 * @return the largest number of buffers the pool has ever cached.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *pool = parcBufferPool_Create(5, 10);
 *     ...
 *
 *     size_t poolSize = parcBufferPool_GetCurrentPoolSize(pool);
 *
 *     parcBufferPool_Release(&pool);
 * }
 * @endcode
 */
size_t parcBufferPool_GetCurrentPoolSize(const PARCBufferPool *bufferPool);

/**
 * Get the largest number of buffers the pool has ever cached.
 *
 * The value is always greater than or equal to 0 and less than or equal to the limit.
 *
 * @param [in] bufferPool A pointer to a valid PARCBufferPool instance.
 *
 * @return the largest number of buffers the pool has ever cached.
 *
 * Example:
 * @code
 * {
 *     PARCBufferPool *pool = parcBufferPool_Create(5, 10);
 *     ...
 *
 *     size_t allTimeHigh = parcBufferPool_GetLargestPoolSize(pool);
 *
 *     parcBufferPool_Release(&pool);
 * }
 * @endcode
 */
size_t parcBufferPool_GetLargestPoolSize(const PARCBufferPool *bufferPool);

/**
 * Forcibly drain the PARCBufferPool of an excess (more than the pool's limit) `PARCBuffer` instances.
 *
 * The number of PARCBuffer instances can exceed the PARCBufferPool's limit if `parcBufferPool_SetLimit` is used to set the limit
 * to less than Pool's current pool size.
 *
 * @param [in] bufferPool A pointer to a valid PARCBufferPool instance.
 *
 * @return the largest number of buffers released from the Pool's cache.
 *
 * Example:
 * @code
 * {
 *
 *     PARCBufferPool *pool = parcBufferPool_Create(5, 10);
 *     ...
 *
 *     size_t limit = parcBufferPool_SetLimit(pool, 3);
 *     size_t drained = parcBufferPool_Drain(pool);
 *
 *     parcBufferPool_Release(&pool);
 * }
 * @endcode
 */
size_t parcBufferPool_Drain(PARCBufferPool *bufferPool);
#endif
