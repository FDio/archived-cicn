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
 * @file parc_ScheduledTask.h
 * @ingroup threading
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_ScheduledTask
#define PARCLibrary_parc_ScheduledTask
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/concurrent/parc_FutureTask.h>
#include <parc/concurrent/parc_Timeout.h>

struct PARCScheduledTask;
typedef struct PARCScheduledTask PARCScheduledTask;

/**
 * Increase the number of references to a `PARCScheduledTask` instance.
 *
 * Note that new `PARCScheduledTask` is not created,
 * only that the given `PARCScheduledTask` reference count is incremented.
 * Discard the reference by invoking `parcScheduledTask_Release`.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     PARCScheduledTask *b = parcScheduledTask_Acquire();
 *
 *     parcScheduledTask_Release(&a);
 *     parcScheduledTask_Release(&b);
 * }
 * @endcode
 */
PARCScheduledTask *parcScheduledTask_Acquire(const PARCScheduledTask *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcScheduledTask_OptionalAssertValid(_instance_)
#else
#  define parcScheduledTask_OptionalAssertValid(_instance_) parcScheduledTask_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCScheduledTask` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     parcScheduledTask_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcScheduledTask_Release(&b);
 * }
 * @endcode
 */
void parcScheduledTask_AssertValid(const PARCScheduledTask *instance);

/**
 * Create an instance of PARCScheduledTask
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCScheduledTask instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     parcScheduledTask_Release(&a);
 * }
 * @endcode
 */
PARCScheduledTask *parcScheduledTask_Create(PARCFutureTask *task, uint64_t executionTime);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 * @param [in] other A pointer to a valid PARCScheduledTask instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *     PARCScheduledTask *b = parcScheduledTask_Create();
 *
 *     if (parcScheduledTask_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcScheduledTask_Release(&a);
 *     parcScheduledTask_Release(&b);
 * }
 * @endcode
 *
 * @see parcScheduledTask_Equals
 */
int parcScheduledTask_Compare(const PARCScheduledTask *instance, const PARCScheduledTask *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCScheduledTask instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCScheduledTask` instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     PARCScheduledTask *copy = parcScheduledTask_Copy(&b);
 *
 *     parcScheduledTask_Release(&b);
 *     parcScheduledTask_Release(&copy);
 * }
 * @endcode
 */
PARCScheduledTask *parcScheduledTask_Copy(const PARCScheduledTask *original);

/**
 * Print a human readable representation of the given `PARCScheduledTask`.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     parcScheduledTask_Display(a, 0);
 *
 *     parcScheduledTask_Release(&a);
 * }
 * @endcode
 */
void parcScheduledTask_Display(const PARCScheduledTask *instance, int indentation);

/**
 * Determine if two `PARCScheduledTask` instances are equal.
 *
 * The following equivalence relations on non-null `PARCScheduledTask` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcScheduledTask_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcScheduledTask_Equals(x, y)` must return true if and only if
 *        `parcScheduledTask_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcScheduledTask_Equals(x, y)` returns true and
 *        `parcScheduledTask_Equals(y, z)` returns true,
 *        then `parcScheduledTask_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcScheduledTask_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcScheduledTask_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCScheduledTask instance.
 * @param [in] y A pointer to a valid PARCScheduledTask instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *     PARCScheduledTask *b = parcScheduledTask_Create();
 *
 *     if (parcScheduledTask_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcScheduledTask_Release(&a);
 *     parcScheduledTask_Release(&b);
 * }
 * @endcode
 * @see parcScheduledTask_HashCode
 */
bool parcScheduledTask_Equals(const PARCScheduledTask *x, const PARCScheduledTask *y);

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
 * If two instances are equal according to the {@link parcScheduledTask_Equals} method,
 * then calling the {@link parcScheduledTask_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcScheduledTask_Equals} function,
 * then calling the `parcScheduledTask_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     PARCHashCode hashValue = parcScheduledTask_HashCode(buffer);
 *     parcScheduledTask_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcScheduledTask_HashCode(const PARCScheduledTask *instance);

/**
 * Determine if an instance of `PARCScheduledTask` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     if (parcScheduledTask_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcScheduledTask_Release(&a);
 * }
 * @endcode
 *
 */
bool parcScheduledTask_IsValid(const PARCScheduledTask *instance);

/**
 * Release a previously acquired reference to the given `PARCScheduledTask` instance,
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
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     parcScheduledTask_Release(&a);
 * }
 * @endcode
 */
void parcScheduledTask_Release(PARCScheduledTask **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     PARCJSON *json = parcScheduledTask_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcScheduledTask_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcScheduledTask_ToJSON(const PARCScheduledTask *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCScheduledTask`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCScheduledTask instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledTask *a = parcScheduledTask_Create();
 *
 *     char *string = parcScheduledTask_ToString(a);
 *
 *     parcScheduledTask_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcScheduledTask_Display
 */
char *parcScheduledTask_ToString(const PARCScheduledTask *instance);

/**
 * Returns the remaining delay associated with this object.
 *
 * @param [in] task A pointer to a valid PARCScheduledTask instance.
 *
 * @return the remaining delay; zero or negative values indicate that the delay has already elapsed
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t parcScheduledTask_GetExecutionTime(const PARCScheduledTask *task);

/**
 * Attempts to cancel execution of this task.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
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
bool parcScheduledTask_Cancel(PARCScheduledTask *task, bool mayInterruptIfRunning);

/**
 * Waits if necessary for at most the given time for the computation to complete, and then retrieves its result, if available.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
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
PARCFutureTaskResult parcScheduledTask_Get(const PARCScheduledTask *task, const PARCTimeout *timeout);


void *parcScheduledTask_Run(const PARCScheduledTask *task);


/**
 * Get the `PARCFutureTask` instance for the given `PARCScheduledTask`
 *
 * @param [in] task A pointer to a valid `PARCScheduledTask` instance.
 *
 * @return the `PARCFutureTask` instance for the given `PARCScheduledTask`
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCFutureTask *parcScheduledTask_GetTask(const PARCScheduledTask *task);

/**
 * Returns true if this task was cancelled before it completed normally.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
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
bool parcScheduledTask_IsCancelled(const PARCScheduledTask *task);

/**
 * Returns true if this task completed.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
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
bool parcScheduledTask_IsDone(const PARCScheduledTask *task);
#endif
