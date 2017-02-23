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
 * @file parc_Stopwatch.h
 * @ingroup developer
 * @brief Measure elapsed time
 *
 * A `PARCStopwatch` measures the time elapsed between the invocation of `parcStopwatch_Start()`
 * and a subsequent invocation of one of the `parcStopwatch_ElapsedTime` functions.
 * The `parcStopwatch_Start()` function may be called for a stopwatch effectively resetting the stopwatch to a new starting time.
 *
 */
#ifndef PARCLibrary_parc_Stopwatch
#define PARCLibrary_parc_Stopwatch
#include <stdbool.h>
#include <stdarg.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

struct PARCStopwatch;
typedef struct PARCStopwatch PARCStopwatch;

/**
 * Increase the number of references to a `PARCStopwatch` instance.
 *
 * Note that new `PARCStopwatch` is not created,
 * only that the given `PARCStopwatch` reference count is incremented.
 * Discard the reference by invoking `parcStopwatch_Release`.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     PARCStopwatch *b = parcStopwatch_Acquire();
 *
 *     parcStopwatch_Release(&a);
 *     parcStopwatch_Release(&b);
 * }
 * @endcode
 */
PARCStopwatch *parcStopwatch_Acquire(const PARCStopwatch *stopwatch);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcStopwatch_OptionalAssertValid(_instance_)
#else
#  define parcStopwatch_OptionalAssertValid(_instance_) parcStopwatch_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCStopwatch` instance is valid.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     parcStopwatch_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcStopwatch_Release(&b);
 * }
 * @endcode
 */
void parcStopwatch_AssertValid(const PARCStopwatch *stopwatch);

/**
 * Create an instance of PARCStopwatch
 *
 * @return non-NULL A pointer to a valid PARCStopwatch instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     parcStopwatch_Release(&a);
 * }
 * @endcode
 */
PARCStopwatch *parcStopwatch_Create(void);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 * @param [in] other A pointer to a valid PARCStopwatch instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *     PARCStopwatch *b = parcStopwatch_Create();
 *
 *     if (parcStopwatch_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcStopwatch_Release(&a);
 *     parcStopwatch_Release(&b);
 * }
 * @endcode
 *
 * @see parcStopwatch_Equals
 */
int parcStopwatch_Compare(const PARCStopwatch *stopwatch, const PARCStopwatch *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCStopwatch instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCStopwatch` instance.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     PARCStopwatch *copy = parcStopwatch_Copy(&b);
 *
 *     parcStopwatch_Release(&b);
 *     parcStopwatch_Release(&copy);
 * }
 * @endcode
 */
PARCStopwatch *parcStopwatch_Copy(const PARCStopwatch *original);

/**
 * Print a human readable representation of the given `PARCStopwatch`.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     parcStopwatch_Display(a, 0);
 *
 *     parcStopwatch_Release(&a);
 * }
 * @endcode
 */
void parcStopwatch_Display(const PARCStopwatch *stopwatch, int indentation);

/**
 * Determine if two `PARCStopwatch` instances are equal.
 *
 * The following equivalence relations on non-null `PARCStopwatch` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcStopwatch_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcStopwatch_Equals(x, y)` must return true if and only if
 *        `parcStopwatch_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcStopwatch_Equals(x, y)` returns true and
 *        `parcStopwatch_Equals(y, z)` returns true,
 *        then `parcStopwatch_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcStopwatch_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcStopwatch_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCStopwatch instance.
 * @param [in] y A pointer to a valid PARCStopwatch instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *     PARCStopwatch *b = parcStopwatch_Create();
 *
 *     if (parcStopwatch_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcStopwatch_Release(&a);
 *     parcStopwatch_Release(&b);
 * }
 * @endcode
 * @see parcStopwatch_HashCode
 */
bool parcStopwatch_Equals(const PARCStopwatch *x, const PARCStopwatch *y);

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
 * If two instances are equal according to the {@link parcStopwatch_Equals} method,
 * then calling the {@link parcStopwatch_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcStopwatch_Equals} function,
 * then calling the `parcStopwatch_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     PARCHashCode hashValue = parcStopwatch_HashCode(buffer);
 *     parcStopwatch_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcStopwatch_HashCode(const PARCStopwatch *stopwatch);

/**
 * Determine if an instance of `PARCStopwatch` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     if (parcStopwatch_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcStopwatch_Release(&a);
 * }
 * @endcode
 *
 */
bool parcStopwatch_IsValid(const PARCStopwatch *stopwatch);

/**
 * Release a previously acquired reference to the given `PARCStopwatch` instance,
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
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     parcStopwatch_Release(&a);
 * }
 * @endcode
 */
void parcStopwatch_Release(PARCStopwatch **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCStopwatch instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     PARCJSON *json = parcStopwatch_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcStopwatch_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcStopwatch_ToJSON(const PARCStopwatch *stopwatch);

/**
 * Produce a null-terminated string representation of the specified `PARCStopwatch`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *a = parcStopwatch_Create();
 *
 *     char *string = parcStopwatch_ToString(a);
 *
 *     parcStopwatch_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcStopwatch_Display
 */
char *parcStopwatch_ToString(const PARCStopwatch *stopwatch);

/**
 * Start one or more PARCStopwatch instances.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 * @param [in] ... A variable argument list consisting of pointers to valid PARCStopwatch instances.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *A = parcStopwatch_Create();
 *     PARCStopwatch *B = parcStopwatch_Create();
 *     PARCStopwatch *C = parcStopwatch_Create();
 *
 *     parcStopwatch_Start(A, B, C);
 *
 * }
 * @endcode
 */
#define parcStopwatch_Start(...) parcStopwatch_StartImpl(__VA_ARGS__, NULL)

/**
 * Start one or more PARCStopwatch instances.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 * @param [in] ... A NULL termianted argument list consisting of pointers to valid PARCStopwatch instances.
 *
 * Example:
 * @code
 * {
 *     PARCStopwatch *stopWatch = parcStopwatch_Create();
 *     parcStopwatch_StartImpl(stopWatch, NULL);
 * }
 * @endcode
 */
void parcStopwatch_StartImpl(PARCStopwatch *stopwatch, ...);

/**
 * Get the number of nanoseconds between the time the PARCStopwatch was started and the time of this function call.
 *
 * The accuracy is dependant upon the operating environment's time resolution.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * @return The difference between the start and stop times nanoseconds.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t parcStopwatch_ElapsedTimeNanos(PARCStopwatch *stopwatch);

/**
 * Get the number of microseconds between the time the PARCStopwatch was started and the time of this function call.
 *
 * The accuracy is dependant upon the operating environment's time resolution.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * @return The difference between the start and stop times in microseconds.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t parcStopwatch_ElapsedTimeMicros(PARCStopwatch *stopwatch);

/**
 * Get the number of milliseconds between the time the PARCStopwatch was started and the time of this function call.
 *
 * The accuracy is dependant upon the operating environment's time resolution.
 *
 * @param [in] stopwatch A pointer to a valid PARCStopwatch instance.
 *
 * @return The difference between the start and stop times in milliseconds.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t parcStopwatch_ElapsedTimeMillis(PARCStopwatch *stopwatch);
#endif
