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
 * @file parc_ScheduledThreadPool.h
 * @ingroup threading
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_ScheduledThreadPool
#define PARCLibrary_parc_ScheduledThreadPool
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_SortedList.h>
#include <parc/concurrent/parc_FutureTask.h>
#include <parc/concurrent/parc_ScheduledTask.h>
#include <parc/concurrent/parc_Timeout.h>

struct PARCScheduledThreadPool;
typedef struct PARCScheduledThreadPool PARCScheduledThreadPool;

/**
 * Increase the number of references to a `PARCScheduledThreadPool` instance.
 *
 * Note that new `PARCScheduledThreadPool` is not created,
 * only that the given `PARCScheduledThreadPool` reference count is incremented.
 * Discard the reference by invoking `parcScheduledThreadPool_Release`.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     PARCScheduledThreadPool *b = parcScheduledThreadPool_Acquire();
 *
 *     parcScheduledThreadPool_Release(&a);
 *     parcScheduledThreadPool_Release(&b);
 * }
 * @endcode
 */
PARCScheduledThreadPool *parcScheduledThreadPool_Acquire(const PARCScheduledThreadPool *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcScheduledThreadPool_OptionalAssertValid(_instance_)
#else
#  define parcScheduledThreadPool_OptionalAssertValid(_instance_) parcScheduledThreadPool_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCScheduledThreadPool` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     parcScheduledThreadPool_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcScheduledThreadPool_Release(&b);
 * }
 * @endcode
 */
void parcScheduledThreadPool_AssertValid(const PARCScheduledThreadPool *instance);

/**
 * Create an instance of PARCScheduledThreadPool
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCScheduledThreadPool instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     parcScheduledThreadPool_Release(&a);
 * }
 * @endcode
 */
PARCScheduledThreadPool *parcScheduledThreadPool_Create(int poolSize);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 * @param [in] other A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *     PARCScheduledThreadPool *b = parcScheduledThreadPool_Create();
 *
 *     if (parcScheduledThreadPool_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcScheduledThreadPool_Release(&a);
 *     parcScheduledThreadPool_Release(&b);
 * }
 * @endcode
 *
 * @see parcScheduledThreadPool_Equals
 */
int parcScheduledThreadPool_Compare(const PARCScheduledThreadPool *instance, const PARCScheduledThreadPool *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCScheduledThreadPool` instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     PARCScheduledThreadPool *copy = parcScheduledThreadPool_Copy(&b);
 *
 *     parcScheduledThreadPool_Release(&b);
 *     parcScheduledThreadPool_Release(&copy);
 * }
 * @endcode
 */
PARCScheduledThreadPool *parcScheduledThreadPool_Copy(const PARCScheduledThreadPool *original);

/**
 * Print a human readable representation of the given `PARCScheduledThreadPool`.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     parcScheduledThreadPool_Display(a, 0);
 *
 *     parcScheduledThreadPool_Release(&a);
 * }
 * @endcode
 */
void parcScheduledThreadPool_Display(const PARCScheduledThreadPool *instance, int indentation);

/**
 * Determine if two `PARCScheduledThreadPool` instances are equal.
 *
 * The following equivalence relations on non-null `PARCScheduledThreadPool` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcScheduledThreadPool_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcScheduledThreadPool_Equals(x, y)` must return true if and only if
 *        `parcScheduledThreadPool_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcScheduledThreadPool_Equals(x, y)` returns true and
 *        `parcScheduledThreadPool_Equals(y, z)` returns true,
 *        then `parcScheduledThreadPool_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcScheduledThreadPool_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcScheduledThreadPool_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCScheduledThreadPool instance.
 * @param [in] y A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *     PARCScheduledThreadPool *b = parcScheduledThreadPool_Create();
 *
 *     if (parcScheduledThreadPool_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcScheduledThreadPool_Release(&a);
 *     parcScheduledThreadPool_Release(&b);
 * }
 * @endcode
 * @see parcScheduledThreadPool_HashCode
 */
bool parcScheduledThreadPool_Equals(const PARCScheduledThreadPool *x, const PARCScheduledThreadPool *y);

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
 * If two instances are equal according to the {@link parcScheduledThreadPool_Equals} method,
 * then calling the {@link parcScheduledThreadPool_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcScheduledThreadPool_Equals} function,
 * then calling the `parcScheduledThreadPool_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     PARCHashCode hashValue = parcScheduledThreadPool_HashCode(buffer);
 *     parcScheduledThreadPool_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcScheduledThreadPool_HashCode(const PARCScheduledThreadPool *instance);

/**
 * Determine if an instance of `PARCScheduledThreadPool` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     if (parcScheduledThreadPool_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcScheduledThreadPool_Release(&a);
 * }
 * @endcode
 *
 */
bool parcScheduledThreadPool_IsValid(const PARCScheduledThreadPool *instance);

/**
 * Release a previously acquired reference to the given `PARCScheduledThreadPool` instance,
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
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     parcScheduledThreadPool_Release(&a);
 * }
 * @endcode
 */
void parcScheduledThreadPool_Release(PARCScheduledThreadPool **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     PARCJSON *json = parcScheduledThreadPool_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcScheduledThreadPool_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcScheduledThreadPool_ToJSON(const PARCScheduledThreadPool *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCScheduledThreadPool`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCScheduledThreadPool instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCScheduledThreadPool *a = parcScheduledThreadPool_Create();
 *
 *     char *string = parcScheduledThreadPool_ToString(a);
 *
 *     parcScheduledThreadPool_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcScheduledThreadPool_Display
 */
char *parcScheduledThreadPool_ToString(const PARCScheduledThreadPool *instance);

/**
 * Executes command with zero required delay.
 */
void parcScheduledThreadPool_Execute(PARCScheduledThreadPool *pool, PARCFutureTask *command);

/**
 * Gets the policy on whether to continue executing existing periodic tasks even when this executor has been shutdown.
 */
bool parcScheduledThreadPool_GetContinueExistingPeriodicTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool);

/**
 * Gets the policy on whether to execute existing delayed tasks even when this executor has been shutdown.
 */
bool parcScheduledThreadPool_GetExecuteExistingDelayedTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool);

/**
 * Returns the task queue used by this executor.
 */
PARCSortedList *parcScheduledThreadPool_GetQueue(const PARCScheduledThreadPool *pool);

/**
 * Gets the policy on whether cancelled tasks should be immediately removed from the work queue at time of cancellation.
 */
bool parcScheduledThreadPool_GetRemoveOnCancelPolicy(const PARCScheduledThreadPool *pool);

/**
 * Creates and executes a one-shot action that becomes enabled after the given delay.
 */
PARCScheduledTask *parcScheduledThreadPool_Schedule(PARCScheduledThreadPool *pool, PARCFutureTask *task, const PARCTimeout *delay);

/**
 * Creates and executes a periodic action that becomes enabled first after the given initial delay, and subsequently with the given period; that is executions will commence after initialDelay then initialDelay+period, then initialDelay + 2 * period, and so on.
 */
PARCScheduledTask *parcScheduledThreadPool_ScheduleAtFixedRate(PARCScheduledThreadPool *pool, PARCFutureTask *task, PARCTimeout initialDelay, PARCTimeout period);

/**
 * Creates and executes a periodic action that becomes enabled first after the given initial delay, and subsequently with the given delay between the termination of one execution and the commencement of the next.
 */
PARCScheduledTask *parcScheduledThreadPool_ScheduleWithFixedDelay(PARCScheduledThreadPool *pool, PARCFutureTask *task, PARCTimeout initialDelay, PARCTimeout delay);

/**
 * Sets the policy on whether to continue executing existing periodic tasks even when this executor has been shutdown.
 */
void parcScheduledThreadPool_SetContinueExistingPeriodicTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool, bool value);

/**
 * Sets the policy on whether to execute existing delayed tasks even when this executor has been shutdown.
 */
void parcScheduledThreadPool_SetExecuteExistingDelayedTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool, bool value);

/**
 * Sets the policy on whether cancelled tasks should be immediately removed from the work queue at time of cancellation.
 */
void parcScheduledThreadPool_SetRemoveOnCancelPolicy(PARCScheduledThreadPool *pool, bool value);

/**
 * Initiates an orderly shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
 */
void parcScheduledThreadPool_Shutdown(PARCScheduledThreadPool *pool);

/**
 * Attempts to stop all actively executing tasks, halts the processing of waiting tasks, and returns a list of the tasks that were awaiting execution.
 */
PARCList *parcScheduledThreadPool_ShutdownNow(PARCScheduledThreadPool *pool);

/**
 * Submits a PARCFutureTask task for execution and returns the PARCFutureTask representing that task.
 */
PARCScheduledTask *parcScheduledThreadPool_Submit(PARCScheduledThreadPool *pool, PARCFutureTask *task);
#endif
