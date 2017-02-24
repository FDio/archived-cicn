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
 * @file parc_FutureTask.h
 * @ingroup threading
 * @brief An encapsulated, asynchronous computation.
 *
 * This type associates a function and a pointer to data and provides the functionality
 * to invoke the function supplying the given pointer to data and returning the result.
 *
 * The operations of invoking the function and collecting its return value may be asynchronous from each other
 * in that an attempting to fetch the return value before the function has been invoked
 * will cause the calling thread to block until the function has been invoked and run to completion.
 * This enables the use of PARCFutureTask in a work queue, or thread pool where tasks are run asynchronously
 * from each other and from an originating thread.
 *
 * Each instance of the type may be cancelled,
 * inhibiting a future invocation of the associated function.
 *
 * Typical use is a one time invocation of the associated function, induced by the `parcFutureTask_Get`,
 * but invoking `parcFutureTask_GetAndReset` invokes the associated function and resets the task to the initial state,
 * permitting a future call to `parcFutureTask_Get` or `parcFutureTask_GetAndReset` the run the associated function again.
 *
 */
#ifndef PARCLibrary_parc_FutureTask
#define PARCLibrary_parc_FutureTask
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/concurrent/parc_Timeout.h>
#include <parc/algol/parc_Execution.h>

struct PARCFutureTask;
typedef struct PARCFutureTask PARCFutureTask;

typedef struct PARCFutureTaskResult {
    void *value;
    PARCExecution *execution;
} PARCFutureTaskResult;

/**
 * Increase the number of references to a `PARCFutureTask` instance.
 *
 * Note that new `PARCFutureTask` is not created,
 * only that the given `PARCFutureTask` reference count is incremented.
 * Discard the reference by invoking `parcFutureTask_Release`.
 *
 * @param [in] instance A pointer to a valid PARCFutureTask instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     PARCFutureTask *b = parcFutureTask_Acquire();
 *
 *     parcFutureTask_Release(&a);
 *     parcFutureTask_Release(&b);
 * }
 * @endcode
 */
PARCFutureTask *parcFutureTask_Acquire(const PARCFutureTask *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcFutureTask_OptionalAssertValid(_instance_)
#else
#  define parcFutureTask_OptionalAssertValid(_instance_) parcFutureTask_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCFutureTask` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCFutureTask instance.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     parcFutureTask_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcFutureTask_Release(&b);
 * }
 * @endcode
 */
void parcFutureTask_AssertValid(const PARCFutureTask *instance);

/**
 * Create an instance of `PARCFutureTask`
 *
 * If the parameter is an instance of `PARCObject` a reference to the object will
 * be created and ultimately released via `parcFutureTask_Release`
 *
 * @param [in] function A pointer to a function to call.
 * @param [in] parameter A pointer that will be passed to the function when invoked.
 *
 * @return non-NULL A pointer to a valid `PARCFutureTask` instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create(function, parameter);
 *
 *     parcFutureTask_Release(&a);
 * }
 * @endcode
 */
PARCFutureTask *parcFutureTask_Create(void *(*runnable)(PARCFutureTask *task, void *parameter), void *parameter);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCFutureTask instance.
 * @param [in] other A pointer to a valid PARCFutureTask instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *     PARCFutureTask *b = parcFutureTask_Create();
 *
 *     if (parcFutureTask_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcFutureTask_Release(&a);
 *     parcFutureTask_Release(&b);
 * }
 * @endcode
 *
 * @see parcFutureTask_Equals
 */
int parcFutureTask_Compare(const PARCFutureTask *instance, const PARCFutureTask *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCFutureTask instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCFutureTask` instance.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     PARCFutureTask *copy = parcFutureTask_Copy(&b);
 *
 *     parcFutureTask_Release(&b);
 *     parcFutureTask_Release(&copy);
 * }
 * @endcode
 */
PARCFutureTask *parcFutureTask_Copy(const PARCFutureTask *original);

/**
 * Print a human readable representation of the given `PARCFutureTask`.
 *
 * @param [in] instance A pointer to a valid `PARCFutureTask` instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     parcFutureTask_Display(a, 0);
 *
 *     parcFutureTask_Release(&a);
 * }
 * @endcode
 */
void parcFutureTask_Display(const PARCFutureTask *instance, int indentation);

/**
 * Determine if two `PARCFutureTask` instances are equal.
 *
 * The following equivalence relations on non-null `PARCFutureTask` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcFutureTask_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcFutureTask_Equals(x, y)` must return true if and only if
 *        `parcFutureTask_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcFutureTask_Equals(x, y)` returns true and
 *        `parcFutureTask_Equals(y, z)` returns true,
 *        then `parcFutureTask_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcFutureTask_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcFutureTask_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCFutureTask instance.
 * @param [in] y A pointer to a valid PARCFutureTask instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *     PARCFutureTask *b = parcFutureTask_Create();
 *
 *     if (parcFutureTask_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcFutureTask_Release(&a);
 *     parcFutureTask_Release(&b);
 * }
 * @endcode
 * @see parcFutureTask_HashCode
 */
bool parcFutureTask_Equals(const PARCFutureTask *x, const PARCFutureTask *y);

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
 * If two instances are equal according to the {@link parcFutureTask_Equals} method,
 * then calling the {@link parcFutureTask_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcFutureTask_Equals} function,
 * then calling the `parcFutureTask_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCFutureTask instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     PARCHashCode hashValue = parcFutureTask_HashCode(buffer);
 *     parcFutureTask_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcFutureTask_HashCode(const PARCFutureTask *instance);

/**
 * Determine if an instance of `PARCFutureTask` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCFutureTask instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     if (parcFutureTask_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcFutureTask_Release(&a);
 * }
 * @endcode
 *
 */
bool parcFutureTask_IsValid(const PARCFutureTask *instance);

/**
 * Release a previously acquired reference to the given `PARCFutureTask` instance,
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
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     parcFutureTask_Release(&a);
 * }
 * @endcode
 */
void parcFutureTask_Release(PARCFutureTask **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCFutureTask instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     PARCJSON *json = parcFutureTask_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcFutureTask_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcFutureTask_ToJSON(const PARCFutureTask *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCFutureTask`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCFutureTask instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCFutureTask *a = parcFutureTask_Create();
 *
 *     char *string = parcFutureTask_ToString(a);
 *
 *     parcFutureTask_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcFutureTask_Display
 */
char *parcFutureTask_ToString(const PARCFutureTask *instance);

/**
 * Wakes up a single thread that is waiting on this object (see `parcFutureTask_Wait)`.
 * If any threads are waiting on this object, one of them is chosen to be awakened.
 * The choice is arbitrary and occurs at the discretion of the underlying implementation.
 *
 * The awakened thread will not be able to proceed until the current thread relinquishes the lock on this object.
 * The awakened thread will compete in the usual manner with any other threads that might be actively
 * competing to synchronize on this object;
 * for example, the awakened thread enjoys no reliable privilege or disadvantage in being the next thread to lock this object.
 *
 * @param [in] object A pointer to a valid PARCFutureTask instance.
 *
 * Example:
 * @code
 * {
 *     if (parcFutureTask_Lock(object)) {
 *         parcFutureTask_Notify(object);
 *         parcFutureTask_Unlock(object);
 *     }
 * }
 * @endcode
 */
parcObject_ImplementNotify(parcFutureTask, PARCFutureTask);

/**
 * Wakes up all threads that are waiting on the given object's lock.
 *
 * A thread waits on an object by calling one of the wait methods, `parcFutureTask_Wait`, `parcFutureTask_WaitFor`, `parcFutureTask_WaitUntil`.
 * The awakened threads will proceed after the current thread relinquishes the lock on the given object.
 * The awakened threads will compete in the usual manner with any other threads that might be actively competing
 * to synchronize on this object.
 * Awakened threads have no priority between them in being the next thread to lock this object.
 *
 * This method can only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid `PARCFutureTask` instance.
 *
 * Example:
 * @code
 * {
 *     if (parcFutureTask_Lock(object)) {
 *         parcFutureTask_NotifyAll(object);
 *         parcFutureTask_Unlock(object);
 *     }
 * }
 * @endcode
 */
parcObject_ImplementNotifyAll(parcFutureTask, PARCFutureTask);

/**
 * Causes the calling thread to wait until either another thread invokes the parcFutureTask_Notify() function on the same object.
 *  *
 * @param [in] object A pointer to a valid `PARCFutureTask` instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcFutureTask_Wait(object);
 * }
 * @endcode
 */
parcObject_ImplementWait(parcFutureTask, PARCFutureTask);

/**
 * Obtain the lock on the given `PARCFutureTask` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] object A pointer to a valid `PARCFutureTask` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, or the `PARCFutureTask` is invalid.
 *
 * Example:
 * @code
 * {
 *     if (parcFutureTask_Lock(object)) {
 *
 *     }
 * }
 * @endcode
 */
parcObject_ImplementLock(parcFutureTask, PARCFutureTask);

/**
 * Try to obtain the advisory lock on the given PARCFutureTask instance.
 *
 * Once the lock is obtained, the caller must release the lock as soon as possible.
 *
 * @param [in] object A pointer to a valid PARCFutureTask instance.
 *
 * @return true The PARCFutureTask is locked.
 * @return false The PARCFutureTask is unlocked.
 *
 * Example:
 * @code
 * {
 *     parcFutureTask_TryLock(object);
 * }
 * @endcode
 */
parcObject_ImplementTryLock(parcFutureTask, PARCFutureTask);

/**
 * Try to unlock the advisory lock on the given `PARCFutureTask` instance.
 *
 * @param [in] object A pointer to a valid `PARCFutureTask` instance.
 *
 * @return true The `PARCFutureTask` was locked and now is unlocked.
 * @return false The `PARCFutureTask` was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     parcFutureTask_Unlock(object);
 * }
 * @endcode
 */
parcObject_ImplementUnlock(parcFutureTask, PARCFutureTask);

/**
 * Determine if the advisory lock on the given `PARCFutureTask` instance is locked.
 *
 * @param [in] object A pointer to a valid `PARCFutureTask` instance.
 *
 * @return true The `PARCFutureTask` is locked.
 * @return false The `PARCFutureTask` is unlocked.
 * Example:
 * @code
 * {
 *     if (parcFutureTask_IsLocked(object)) {
 *         ...
 *     }
 * }
 * @endcode
 */
parcObject_ImplementIsLocked(parcFutureTask, PARCFutureTask);

/**
 * Attempt to cancel the execution of this task.
 *
 * This will return `false` if the task is already done or has already been cancelled.
 * Otherswise, if this task has not started when `parcFutureTask_Cancel` is called, this task will never run.
 *
 * If the task is already running, the boolean `mayInterruptIfRunning` may cause the task to be interrupted,
 * otherwise this function will have no effect.
 *
 * After this function returns, subsequent calls to `parcFutureTask_IsDone` will always return true.
 * Subsequent calls to  `parcFutureTask_isCancelled` will always return true if this function returned true.
 *
 * @param [in] object A pointer to a valid `PARCFutureTask` instance.
 *
 * @return true The task was cancelled.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcFutureTask_Cancel(PARCFutureTask *task, bool mayInterruptIfRunning);

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
PARCFutureTaskResult parcFutureTask_Get(const PARCFutureTask *futureTask, const PARCTimeout *timeout);

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
bool parcFutureTask_IsCancelled(const PARCFutureTask *futureTask);

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
bool parcFutureTask_IsDone(const PARCFutureTask *futureTask);

/**
 * Sets this Future to the result of its computation unless it has been cancelled.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return The result returned by the task function.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void *parcFutureTask_Run(PARCFutureTask *futureTask);

/**
 * Executes the computation without setting its result, and then resets this future to initial state, failing to do so if the computation encounters an exception or is cancelled.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return true The task was executed.
 * @retval false The task was not executed because it was previously completed, or it was cancelled.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcFutureTask_RunAndReset(PARCFutureTask *futureTask);

/**
 * Reset the given PARCFutureTask to the intial state, a subsequent ecutes the computation without setting its result, and then resets this future to initial state, failing to do so if the computation encounters an exception or is cancelled.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return true The task was successfully run
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcFutureTask_Reset(PARCFutureTask *task);
#endif
