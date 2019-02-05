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
 * @file parc_ThreadPool.h
 * @ingroup threading
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_ThreadPool
#define PARCLibrary_parc_ThreadPool
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/concurrent/parc_Timeout.h>
#include <parc/concurrent/parc_FutureTask.h>

struct PARCThreadPool;
typedef struct PARCThreadPool PARCThreadPool;

/**
 * Increase the number of references to a `PARCThreadPool` instance.
 *
 * Note that new `PARCThreadPool` is not created,
 * only that the given `PARCThreadPool` reference count is incremented.
 * Discard the reference by invoking `parcThreadPool_Release`.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     PARCThreadPool *b = parcThreadPool_Acquire();
 *
 *     parcThreadPool_Release(&a);
 *     parcThreadPool_Release(&b);
 * }
 * @endcode
 */
PARCThreadPool *parcThreadPool_Acquire(const PARCThreadPool *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcThreadPool_OptionalAssertValid(_instance_)
#else
#  define parcThreadPool_OptionalAssertValid(_instance_) parcThreadPool_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCThreadPool` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     parcThreadPool_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcThreadPool_Release(&b);
 * }
 * @endcode
 */
void parcThreadPool_AssertValid(const PARCThreadPool *instance);

/**
 * Create an instance of PARCThreadPool
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCThreadPool instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     parcThreadPool_Release(&a);
 * }
 * @endcode
 */
PARCThreadPool *parcThreadPool_Create(int poolSize);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 * @param [in] other A pointer to a valid PARCThreadPool instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *     PARCThreadPool *b = parcThreadPool_Create();
 *
 *     if (parcThreadPool_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcThreadPool_Release(&a);
 *     parcThreadPool_Release(&b);
 * }
 * @endcode
 *
 * @see parcThreadPool_Equals
 */
int parcThreadPool_Compare(const PARCThreadPool *instance, const PARCThreadPool *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCThreadPool instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCThreadPool` instance.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     PARCThreadPool *copy = parcThreadPool_Copy(&b);
 *
 *     parcThreadPool_Release(&b);
 *     parcThreadPool_Release(&copy);
 * }
 * @endcode
 */
PARCThreadPool *parcThreadPool_Copy(const PARCThreadPool *original);

/**
 * Print a human readable representation of the given `PARCThreadPool`.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     parcThreadPool_Display(a, 0);
 *
 *     parcThreadPool_Release(&a);
 * }
 * @endcode
 */
void parcThreadPool_Display(const PARCThreadPool *instance, int indentation);

/**
 * Determine if two `PARCThreadPool` instances are equal.
 *
 * The following equivalence relations on non-null `PARCThreadPool` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcThreadPool_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcThreadPool_Equals(x, y)` must return true if and only if
 *        `parcThreadPool_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcThreadPool_Equals(x, y)` returns true and
 *        `parcThreadPool_Equals(y, z)` returns true,
 *        then `parcThreadPool_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcThreadPool_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcThreadPool_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCThreadPool instance.
 * @param [in] y A pointer to a valid PARCThreadPool instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *     PARCThreadPool *b = parcThreadPool_Create();
 *
 *     if (parcThreadPool_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcThreadPool_Release(&a);
 *     parcThreadPool_Release(&b);
 * }
 * @endcode
 * @see parcThreadPool_HashCode
 */
bool parcThreadPool_Equals(const PARCThreadPool *x, const PARCThreadPool *y);

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
 * If two instances are equal according to the {@link parcThreadPool_Equals} method,
 * then calling the {@link parcThreadPool_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcThreadPool_Equals} function,
 * then calling the `parcThreadPool_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     PARCHashCode hashValue = parcThreadPool_HashCode(buffer);
 *     parcThreadPool_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcThreadPool_HashCode(const PARCThreadPool *instance);

/**
 * Determine if an instance of `PARCThreadPool` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     if (parcThreadPool_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcThreadPool_Release(&a);
 * }
 * @endcode
 *
 */
bool parcThreadPool_IsValid(const PARCThreadPool *instance);

/**
 * Release a previously acquired reference to the given `PARCThreadPool` instance,
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
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     parcThreadPool_Release(&a);
 * }
 * @endcode
 */
void parcThreadPool_Release(PARCThreadPool **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     PARCJSON *json = parcThreadPool_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcThreadPool_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcThreadPool_ToJSON(const PARCThreadPool *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCThreadPool`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCThreadPool instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCThreadPool *a = parcThreadPool_Create();
 *
 *     char *string = parcThreadPool_ToString(a);
 *
 *     parcThreadPool_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcThreadPool_Display
 */
char *parcThreadPool_ToString(const PARCThreadPool *instance);

/**
 * Wakes up a single thread that is waiting on this object (see `parcThreadPool_Wait)`.
 * If any threads are waiting on this object, one of them is chosen to be awakened.
 * The choice is arbitrary and occurs at the discretion of the underlying implementation.
 *
 * The awakened thread will not be able to proceed until the current thread relinquishes the lock on this object.
 * The awakened thread will compete in the usual manner with any other threads that might be actively
 * competing to synchronize on this object;
 * for example, the awakened thread enjoys no reliable privilege or disadvantage in being the next thread to lock this object.
 *
 * @param [in] object A pointer to a valid PARCThreadPool instance.
 *
 * Example:
 * @code
 * {
 *     if (parcThreadPool_Lock(object)) {
 *         parcThreadPool_Notify(object);
 *         parcThreadPool_Unlock(object);
 *     }
 * }
 * @endcode
 */
parcObject_ImplementNotify(parcThreadPool, PARCThreadPool);

/**
 * Wakes up all threads that are waiting on the given object's lock.
 *
 * A thread waits on an object by calling one of the wait methods, `parcThreadPool_Wait`, `parcThreadPool_WaitFor`, `parcThreadPool_WaitUntil`.
 * The awakened threads will proceed after the current thread relinquishes the lock on the given object.
 * The awakened threads will compete in the usual manner with any other threads that might be actively competing
 * to synchronize on this object.
 * Awakened threads have no priority between them in being the next thread to lock this object.
 *
 * This method can only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid `PARCThreadPool` instance.
 *
 * Example:
 * @code
 * {
 *     if (parcThreadPool_Lock(object)) {
 *         parcThreadPool_NotifyAll(object);
 *         parcThreadPool_Unlock(object);
 *     }
 * }
 * @endcode
 */
parcObject_ImplementNotifyAll(parcThreadPool, PARCThreadPool);

/**
 * Causes the calling thread to wait until either another thread invokes the parcThreadPool_Notify() function on the same object.
 *  *
 * @param [in] object A pointer to a valid `PARCThreadPool` instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcThreadPool_Wait(object);
 * }
 * @endcode
 */
parcObject_ImplementWait(parcThreadPool, PARCThreadPool);

/**
 * Obtain the lock on the given `PARCThreadPool` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] object A pointer to a valid `PARCThreadPool` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, or the `PARCThreadPool` is invalid.
 *
 * Example:
 * @code
 * {
 *     if (parcThreadPool_Lock(object)) {
 *
 *     }
 * }
 * @endcode
 */
parcObject_ImplementLock(parcThreadPool, PARCThreadPool);

/**
 * Try to obtain the advisory lock on the given PARCThreadPool instance.
 *
 * Once the lock is obtained, the caller must release the lock as soon as possible.
 *
 * @param [in] object A pointer to a valid PARCThreadPool instance.
 *
 * @return true The PARCThreadPool is locked.
 * @return false The PARCThreadPool is unlocked.
 *
 * Example:
 * @code
 * {
 *     parcThreadPool_TryLock(object);
 * }
 * @endcode
 */
parcObject_ImplementTryLock(parcThreadPool, PARCThreadPool);

/**
 * Try to unlock the advisory lock on the given `PARCThreadPool` instance.
 *
 * @param [in] object A pointer to a valid `PARCThreadPool` instance.
 *
 * @return true The `PARCThreadPool` was locked and now is unlocked.
 * @return false The `PARCThreadPool` was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     parcThreadPool_Unlock(object);
 * }
 * @endcode
 */
parcObject_ImplementUnlock(parcThreadPool, PARCThreadPool);

/**
 * Determine if the advisory lock on the given `PARCThreadPool` instance is locked.
 *
 * @param [in] object A pointer to a valid `PARCThreadPool` instance.
 *
 * @return true The `PARCThreadPool` is locked.
 * @return false The `PARCThreadPool` is unlocked.
 * Example:
 * @code
 * {
 *     if (parcThreadPool_IsLocked(object)) {
 *         ...
 *     }
 * }
 * @endcode
 */
parcObject_ImplementIsLocked(parcThreadPool, PARCThreadPool);

/**
 * Sets the policy governing whether core threads may time out and terminate if no tasks arrive within the keep-alive time, being replaced if needed when new tasks arrive.
 */
void parcThreadPool_SetAllowCoreThreadTimeOut(PARCThreadPool *pool, bool value);

/**
 * Returns true if this pool allows core threads to time out and terminate if no tasks arrive within the keepAlive time, being replaced if needed when new tasks arrive.
 */
bool parcThreadPool_GetAllowsCoreThreadTimeOut(const PARCThreadPool *pool);

/**
 * Blocks until all tasks have completed execution after a shutdown request, or the timeout occurs, whichever happens first.
 */
bool parcThreadPool_AwaitTermination(PARCThreadPool *pool, PARCTimeout *timeout);

/**
 * Executes the given task sometime in the future.
 */
bool parcThreadPool_Execute(PARCThreadPool *pool, PARCFutureTask *task);

/**
 * Returns the approximate number of threads that are actively executing tasks.
 */
int parcThreadPool_GetActiveCount(const PARCThreadPool *pool);

/**
 * Returns the approximate total number of tasks that have completed execution.
 */
uint64_t parcThreadPool_GetCompletedTaskCount(const PARCThreadPool *pool);

/**
 * Returns the core number of threads.
 */
int parcThreadPool_GetCorePoolSize(const PARCThreadPool *pool);

/**
 * Returns the thread keep-alive time, which is the amount of time that threads in excess of the core pool size may remain idle before being terminated.
 */
PARCTimeout *parcThreadPool_GetKeepAliveTime(const PARCThreadPool *pool);

/**
 * Returns the largest number of threads that have ever simultaneously been in the pool.
 */
int parcThreadPool_GetLargestPoolSize(const PARCThreadPool *pool);

/**
 * Returns the maximum allowed number of threads.
 */
int parcThreadPool_GetMaximumPoolSize(const PARCThreadPool *pool);

/**
 * Returns the current number of threads in the pool.
 */
int parcThreadPool_GetPoolSize(const PARCThreadPool *pool);

/**
 * Returns the task queue used by this executor.
 */
PARCLinkedList *parcThreadPool_GetQueue(const PARCThreadPool *pool);

/**
 * Returns the approximate total number of tasks that have ever been scheduled for execution.
 */
long parcThreadPool_GetTaskCount(const PARCThreadPool *pool);

/**
 * Returns true if this executor has been shut down.
 */
bool parcThreadPool_IsShutdown(const PARCThreadPool *pool);

/**
 * Returns true if all tasks have completed following shut down.
 */
bool parcThreadPool_IsTerminated(const PARCThreadPool *pool);

/**
 * Returns true if this executor is in the process of terminating after shutdown() or shutdownNow() but has not completely terminated.
 */
bool parcThreadPool_IsTerminating(const PARCThreadPool *pool);

/**
 * Starts all core threads, causing them to idly wait for work.
 */
int parcThreadPool_PrestartAllCoreThreads(PARCThreadPool *pool);

/**
 * Starts a core thread, causing it to idly wait for work.
 */
bool parcThreadPool_PrestartCoreThread(PARCThreadPool *pool);

/**
 * Tries to remove from the work queue all Future tasks that have been cancelled.
 */
void parcThreadPool_Purge(PARCThreadPool *pool);

/**
 * Removes this task from the executor's internal queue if it is present, thus causing it not to be run if it has not already started.
 */
bool parcThreadPool_Remove(PARCThreadPool *pool, PARCFutureTask *task);

/**
 * Sets the core number of threads.
 */
void parcThreadPool_SetCorePoolSize(PARCThreadPool *pool, int corePoolSize);

/**
 * Sets the time limit for which threads may remain idle before being terminated.
 */
void parcThreadPool_SetKeepAliveTime(PARCThreadPool *pool, PARCTimeout *timeout);

/**
 * Sets the maximum allowed number of threads.
 */
void parcThreadPool_SetMaximumPoolSize(PARCThreadPool *pool, int maximumPoolSize);

/**
 * Initiates an orderly shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
 */
void parcThreadPool_Shutdown(PARCThreadPool *pool);

/**
 * Attempts to stop all actively executing tasks, halts the processing of waiting tasks, and returns a list of the tasks that were awaiting execution.
 */
PARCLinkedList *parcThreadPool_ShutdownNow(PARCThreadPool *pool);
#endif
