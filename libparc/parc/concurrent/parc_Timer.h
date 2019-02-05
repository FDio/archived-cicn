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
 * @file parc_Timer.h
 * @ingroup threading
 * @brief A facility for threads to schedule tasks for future execution in a background thread.
 *
 * Tasks may be scheduled for one-time execution, or for repeated execution at regular intervals.
 *
 * Corresponding to each Timer object is a single background thread that is used to execute all of the timer's tasks,
 * sequentially. Timer tasks should complete quickly.
 * If a timer task takes excessive time to complete, it "hogs" the timer's task execution thread.
 * This can, in turn, delay the execution of subsequent tasks,
 * which may "bunch up" and execute in rapid succession when (and if) the offending task finally completes.
 *
 * After the last live reference to a Timer object goes away and all outstanding tasks have completed execution,
 * the timer's task execution thread terminates gracefully (and becomes subject to garbage collection).
 * However, this can take arbitrarily long to occur.
 * By default, the task execution thread does not run as a daemon thread,
 * so it is capable of keeping an application from terminating.
 * If a caller wants to terminate a timer's task execution thread rapidly, the caller should invoke the timer's cancel method.
 *
 * If the timer's task execution thread terminates unexpectedly,
 * for example, because its stop method is invoked,
 * any further attempt to schedule a task on the timer will result in an IllegalStateException,
 * as if the timer's cancel method had been invoked.
 *
 * This class is thread-safe: multiple threads can share a single Timer object without the need for external synchronization.
 *
 * This class does not offer real-time guarantees: it schedules tasks using the Object.wait(long) method.
 *
 */
#ifndef PARCLibrary_parc_Timer
#define PARCLibrary_parc_Timer
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/concurrent/parc_FutureTask.h>

struct PARCTimer;
typedef struct PARCTimer PARCTimer;

/**
 * Increase the number of references to a `PARCTimer` instance.
 *
 * Note that new `PARCTimer` is not created,
 * only that the given `PARCTimer` reference count is incremented.
 * Discard the reference by invoking `parcTimer_Release`.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     PARCTimer *b = parcTimer_Acquire();
 *
 *     parcTimer_Release(&a);
 *     parcTimer_Release(&b);
 * }
 * @endcode
 */
PARCTimer *parcTimer_Acquire(const PARCTimer *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcTimer_OptionalAssertValid(_instance_)
#else
#  define parcTimer_OptionalAssertValid(_instance_) parcTimer_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCTimer` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     parcTimer_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcTimer_Release(&b);
 * }
 * @endcode
 */
void parcTimer_AssertValid(const PARCTimer *instance);

/**
 * Create an instance of PARCTimer
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCTimer instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     parcTimer_Release(&a);
 * }
 * @endcode
 */
PARCTimer *parcTimer_Create(void);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 * @param [in] other A pointer to a valid PARCTimer instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *     PARCTimer *b = parcTimer_Create();
 *
 *     if (parcTimer_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcTimer_Release(&a);
 *     parcTimer_Release(&b);
 * }
 * @endcode
 *
 * @see parcTimer_Equals
 */
int parcTimer_Compare(const PARCTimer *instance, const PARCTimer *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCTimer instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCTimer` instance.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     PARCTimer *copy = parcTimer_Copy(&b);
 *
 *     parcTimer_Release(&b);
 *     parcTimer_Release(&copy);
 * }
 * @endcode
 */
PARCTimer *parcTimer_Copy(const PARCTimer *original);

/**
 * Print a human readable representation of the given `PARCTimer`.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     parcTimer_Display(a, 0);
 *
 *     parcTimer_Release(&a);
 * }
 * @endcode
 */
void parcTimer_Display(const PARCTimer *instance, int indentation);

/**
 * Determine if two `PARCTimer` instances are equal.
 *
 * The following equivalence relations on non-null `PARCTimer` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcTimer_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcTimer_Equals(x, y)` must return true if and only if
 *        `parcTimer_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcTimer_Equals(x, y)` returns true and
 *        `parcTimer_Equals(y, z)` returns true,
 *        then `parcTimer_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcTimer_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcTimer_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCTimer instance.
 * @param [in] y A pointer to a valid PARCTimer instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *     PARCTimer *b = parcTimer_Create();
 *
 *     if (parcTimer_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcTimer_Release(&a);
 *     parcTimer_Release(&b);
 * }
 * @endcode
 * @see parcTimer_HashCode
 */
bool parcTimer_Equals(const PARCTimer *x, const PARCTimer *y);

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
 * If two instances are equal according to the {@link parcTimer_Equals} method,
 * then calling the {@link parcTimer_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcTimer_Equals} function,
 * then calling the `parcTimer_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     PARCHashCode hashValue = parcTimer_HashCode(buffer);
 *     parcTimer_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcTimer_HashCode(const PARCTimer *instance);

/**
 * Determine if an instance of `PARCTimer` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     if (parcTimer_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcTimer_Release(&a);
 * }
 * @endcode
 *
 */
bool parcTimer_IsValid(const PARCTimer *instance);

/**
 * Release a previously acquired reference to the given `PARCTimer` instance,
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
 *     PARCTimer *a = parcTimer_Create();
 *
 *     parcTimer_Release(&a);
 * }
 * @endcode
 */
void parcTimer_Release(PARCTimer **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     PARCJSON *json = parcTimer_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcTimer_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcTimer_ToJSON(const PARCTimer *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCTimer`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCTimer instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCTimer *a = parcTimer_Create();
 *
 *     char *string = parcTimer_ToString(a);
 *
 *     parcTimer_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcTimer_Display
 */
char *parcTimer_ToString(const PARCTimer *timer);

/**
 * Terminates this timer, discarding any currently scheduled tasks.
 *
 * Does not interfere with a currently executing task (if it exists).
 * Once a timer has been terminated, its execution thread terminates gracefully,
 * and no more tasks may be scheduled on it.
 *
 * Note that calling this method from within the run method of a timer task that was invoked
 * by this timer absolutely guarantees that the ongoing task execution is the last task execution
 * that will ever be performed by this timer.
 *
 * This method may be called repeatedly; the second and subsequent calls have no effect.
 */
void parcTimer_Cancel(PARCTimer *timer);

/**
 * Removes all cancelled tasks from this timer's task queue.
 *
 * Calling this method has no effect on the behavior of the timer,
 * but eliminates the references to the cancelled tasks from the queue.
 * If there are no external references to these tasks, they become eligible for garbage collection.
 *
 * Most programs will have no need to call this method.
 * It is designed for use by the rare application that cancels a large number of tasks.
 * Calling this method trades time for space: the runtime of the method may be proportional to
 * n + c log n, where n is the number of tasks in the queue and c is the number of cancelled tasks.
 *
 * It is permissible to call this method from within a task scheduled on this timer.
 *
 * @returns the number of tasks removed from the queue.
 */
int parcTimer_Purge(PARCTimer *timer);

/**
 * Schedules the specified task for execution at the specified time.
 */
void parcTimer_ScheduleAtTime(PARCTimer *timer, PARCFutureTask *task, time_t absoluteTime);

/**
 * Schedules the specified task for repeated fixed-delay execution, beginning at the specified time.
 */
void parcTimer_ScheduleAtTimeAndRepeat(PARCTimer *timer, PARCFutureTask *task, time_t firstTime, long period);

/**
 * Schedules the specified task for execution after the specified delay.
 */
void parcTimer_ScheduleAfterDelay(PARCTimer *timer, PARCFutureTask *task, long delay);

/**
 * Schedules the specified task for repeated fixed-delay execution, beginning after the specified delay.
 */
void parcTimer_ScheduleAfterDelayAndRepeat(PARCTimer *timer, PARCFutureTask *task, long delay, long period);
#endif
