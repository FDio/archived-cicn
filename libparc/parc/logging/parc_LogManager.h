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
 * @file parc_LogManager.h
 * @brief <#Brief Description#>
 *
 */
#ifndef PARC_Library_parc_LogManager_h
#define PARC_Library_parc_LogManager_h

struct PARCLogManager;
typedef struct PARCLogManager PARCLogManager;

/**
 * Create a new PARCLogManager
 *
 * @return non-NULL A pointer to a valid PARCLogManager
 * @return NULL Out of memory.
 *
 * Example:
 * @code
 * {
 *     PARCLogManager *manager = parcLogManager_Create();
 *
 *     parcLogManager_Release(&manager);
 * }
 * @endcode
 */
PARCLogManager *parcLogManager_Create(void);

/**
 * Increase the number of references to a `PARCLogManager` instance.
 *
 * Note that new `PARCLogManager` is not created,
 * only that the given `PARCLogManager` reference count is incremented.
 * Discard the reference by invoking `parcLogManager_Release`.
 *
 * @param [in] instance A pointer to a `PARCLogManager` instance.
 *
 * @return The input `PARCLogManager` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCLogManager *manager = parcLogManager_Create();
 *
 *     PARCLogReporter *x_2 = parcLogManager_Acquire(reporter);
 *
 *     parcLogManager_Release(&manager);
 *     parcLogManager_Release(&x_2);
 * }
 * @endcode
 */
PARCLogManager *parcLogManager_Acquire(const PARCLogManager *instance);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to a `PARCLogManager`.  The parameter is set to zero.
 *
 * Example:
 * @code
 * {
 *     PARCLogManager *manager = parcLogManager_Create();
 *
 *     parcLogManager_Release(&manager);
 * }
 * @endcode
 */
void parcLogManager_Release(PARCLogManager **instancePtr);
#endif
