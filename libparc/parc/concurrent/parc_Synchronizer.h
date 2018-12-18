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
 * @file parc_Synchronizer.h
 * @ingroup threading
 * @brief A simple mutual exclusive synchronization implementation.
 *
 * Detailed Description
 *
 */
#ifndef PARCLibrary_parc_Barrier
#define PARCLibrary_parc_Barrier
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>

struct PARCSynchronizer;
typedef struct PARCSynchronizer PARCSynchronizer;

/**
 * Increase the number of references to a `PARCSynchronizer` instance.
 *
 * Note that new `PARCSynchronizer` is not created,
 * only that the given `PARCSynchronizer` reference count is incremented.
 * Discard the reference by invoking `parcSynchronizer_Release`.
 *
 * @param [in] instance A pointer to a valid PARCSynchronizer instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     PARCSynchronizer *b = parcSynchronizer_Acquire();
 *
 *     parcSynchronizer_Release(&a);
 *     parcSynchronizer_Release(&b);
 * }
 * @endcode
 */
PARCSynchronizer *parcSynchronizer_Acquire(const PARCSynchronizer *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcSynchronizer_OptionalAssertValid(_instance_)
#else
#  define parcSynchronizer_OptionalAssertValid(_instance_) parcSynchronizer_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCSynchronizer` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCSynchronizer instance.
 *
 * Example:
 * @code
 * {
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     parcSynchronizer_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcSynchronizer_Release(&b);
 * }
 * @endcode
 */
void parcSynchronizer_AssertValid(const PARCSynchronizer *instance);

/**
 * Create an instance of PARCSynchronizer
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCSynchronizer instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     parcSynchronizer_Release(&a);
 * }
 * @endcode
 */
PARCSynchronizer *parcSynchronizer_Create(void);

/**
 * Print a human readable representation of the given `PARCSynchronizer`.
 *
 * @param [in] instance A pointer to a valid PARCSynchronizer instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     parcSynchronizer_Display(a, 0);
 *
 *     parcSynchronizer_Release(&a);
 * }
 * @endcode
 */
void parcSynchronizer_Display(const PARCSynchronizer *instance, int indentation);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the `HashCode` function must consistently return the same value,
 * provided no information used in a corresponding comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link parcSynchronizer_Equals} method,
 * then calling the {@link parcSynchronizer_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcSynchronizer_Equals} function,
 * then calling the `parcSynchronizer_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCSynchronizer instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     uint32_t hashValue = parcSynchronizer_HashCode(buffer);
 *     parcSynchronizer_Release(&a);
 * }
 * @endcode
 */
int parcSynchronizer_HashCode(const PARCSynchronizer *instance);

/**
 * Determine if an instance of `PARCSynchronizer` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCSynchronizer instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     if (parcSynchronizer_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcSynchronizer_Release(&a);
 * }
 * @endcode
 *
 */
bool parcSynchronizer_IsValid(const PARCSynchronizer *instance);

/**
 * Release a previously acquired reference to the given `PARCSynchronizer` instance,
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
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     parcSynchronizer_Release(&a);
 * }
 * @endcode
 */
void parcSynchronizer_Release(PARCSynchronizer **instancePtr);

/**
 * Attempt to lock the given PARCSynchronizer.
 *
 * If the synchronizer is already locked, this function returns `false`.
 * Otherwise, the lock is established and this function returns `true`.
 *
 * @param [in] barrier A pointer to a valid PARCSynchronizer instance.
 *
 * @return `true` The PARCSynchronizer was successfully set.
 * @return `false` The PARCSynchronizer could not be set.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcSynchronizer_TryLock(PARCSynchronizer *barrier);

/**
 * Lock the given PARCSynchronizer.
 *
 * If the synchronizer is already locked, this function blocks the caller until it is able to acquire the lock.
 *
 * @param [in] barrier A pointer to a valid PARCSynchronizer instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcSynchronizer_Lock(PARCSynchronizer *barrier);

/**
 * Unlock the given PARCSynchronizer.
 *
 * @param [in] barrier A pointer to a valid PARCSynchronizer instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcSynchronizer_Unlock(PARCSynchronizer *barrier);

/**
 * Check if a PARCSynchronizer is locked.
 *
 * @param [in] synchronizer A pointer to a valid PARCSynchronizer instance.
 *
 * @return true The specified synchronizer is currently locked.
 *
 * Example:
 * @code
 * {
 *     PARCSynchronizer *a = parcSynchronizer_Create();
 *
 *     if (parcSynchronizer_IsLocked(a) == true) {
 *         printf("A PARCSynchronizer cannot be created in the locked state.\n");
 *     }
 *
 *     parcSynchronizer_Release(&a);
 * }
 * @endcode
 */
bool parcSynchronizer_IsLocked(const PARCSynchronizer *synchronizer);
#endif
