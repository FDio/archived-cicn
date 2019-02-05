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
 * @file parc_EWMA.h
 * @ingroup statistics
 * @brief A simple exponential moving average smoothing filter for integers.
 *
 * An exponentially weighted moving average (EWMA) is a type of infinite impulse response filter that
 * applies weighting factors which decrease exponentially. The weighting for each older datum decreases
 * exponentially, never reaching zero.
 *
 */
#ifndef PARCLibrary_parc_EWMA
#define PARCLibrary_parc_EWMA
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

struct PARCEWMA;
typedef struct PARCEWMA PARCEWMA;

/**
 * Increase the number of references to a `PARCEWMA` instance.
 *
 * Note that new `PARCEWMA` is not created,
 * only that the given `PARCEWMA` reference count is incremented.
 * Discard the reference by invoking `parcEWMA_Release`.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     PARCEWMA *b = parcEWMA_Acquire();
 *
 *     parcEWMA_Release(&a);
 *     parcEWMA_Release(&b);
 * }
 * @endcode
 */
PARCEWMA *parcEWMA_Acquire(const PARCEWMA *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcEWMA_OptionalAssertValid(_instance_)
#else
#  define parcEWMA_OptionalAssertValid(_instance_) parcEWMA_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCEWMA` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     parcEWMA_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcEWMA_Release(&b);
 * }
 * @endcode
 */
void parcEWMA_AssertValid(const PARCEWMA *instance);

/**
 * Create an instance of PARCEWMA
 *
 * The coefficient represents  a constant smoothing factor affecting
 * the degree of prior samples to be applied upon each new update.
 * Typically the the coefficient is _0 < coefficient < 1.0_.
 * A higher coefficient discounts older observations faster.
 *
 * @return non-NULL A pointer to a valid PARCEWMA instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     parcEWMA_Release(&a);
 * }
 * @endcode
 */
PARCEWMA *parcEWMA_Create(double coefficient);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 * @param [in] other A pointer to a valid PARCEWMA instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *     PARCEWMA *b = parcEWMA_Create(0.75);
 *
 *     if (parcEWMA_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcEWMA_Release(&a);
 *     parcEWMA_Release(&b);
 * }
 * @endcode
 *
 * @see parcEWMA_Equals
 */
int parcEWMA_Compare(const PARCEWMA *instance, const PARCEWMA *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCEWMA instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCEWMA` instance.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     PARCEWMA *copy = parcEWMA_Copy(&b);
 *
 *     parcEWMA_Release(&b);
 *     parcEWMA_Release(&copy);
 * }
 * @endcode
 */
PARCEWMA *parcEWMA_Copy(const PARCEWMA *original);

/**
 * Print a human readable representation of the given `PARCEWMA`.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     parcEWMA_Display(a, 0);
 *
 *     parcEWMA_Release(&a);
 * }
 * @endcode
 */
void parcEWMA_Display(const PARCEWMA *instance, int indentation);

/**
 * Determine if two `PARCEWMA` instances are equal.
 *
 * The following equivalence relations on non-null `PARCEWMA` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcEWMA_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcEWMA_Equals(x, y)` must return true if and only if
 *        `parcEWMA_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcEWMA_Equals(x, y)` returns true and
 *        `parcEWMA_Equals(y, z)` returns true,
 *        then `parcEWMA_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcEWMA_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcEWMA_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCEWMA instance.
 * @param [in] y A pointer to a valid PARCEWMA instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *     PARCEWMA *b = parcEWMA_Create(0.75);
 *
 *     if (parcEWMA_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcEWMA_Release(&a);
 *     parcEWMA_Release(&b);
 * }
 * @endcode
 * @see parcEWMA_HashCode
 */
bool parcEWMA_Equals(const PARCEWMA *x, const PARCEWMA *y);

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
 * If two instances are equal according to the {@link parcEWMA_Equals} method,
 * then calling the {@link parcEWMA_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcEWMA_Equals} function,
 * then calling the `parcEWMA_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     PARCHashCode hashValue = parcEWMA_HashCode(buffer);
 *     parcEWMA_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcEWMA_HashCode(const PARCEWMA *instance);

/**
 * Determine if an instance of `PARCEWMA` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     if (parcEWMA_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcEWMA_Release(&a);
 * }
 * @endcode
 *
 */
bool parcEWMA_IsValid(const PARCEWMA *instance);

/**
 * Release a previously acquired reference to the given `PARCEWMA` instance,
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
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     parcEWMA_Release(&a);
 * }
 * @endcode
 */
void parcEWMA_Release(PARCEWMA **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     PARCJSON *json = parcEWMA_ToJSON(a);
 *
 *     char *cString = parcJSON_ToString(json);
 *     printf("JSON representation: %s\n", cString);
 *
 *     parcMemory_Deallocate(&string);
 *     parcJSON_Release(&json);
 *
 *     parcEWMA_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcEWMA_ToJSON(const PARCEWMA *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCEWMA`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCEWMA instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *a = parcEWMA_Create(0.75);
 *
 *     char *string = parcEWMA_ToString(a);
 *
 *     parcEWMA_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcEWMA_Display
 */
char *parcEWMA_ToString(const PARCEWMA *instance);

/**
 * Update the given `PARCEWMA` filter.
 *
 * The value of the filter is modified by the input of an updated value
 *
 * @param [in] ewma A pointer to a valid `PARCEWMA` instance.
 *
 * @return The current exponentitally smoothed value of the filter.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *ewma = parcEWMA_Create(0.75);
 *
 *     for (uint64_t i = 0; i < 10; i++) {
 *         parcEWMA_Update(ewma, i);
 *     }
 *
 *     int64_t smoothedValue = parcEWMA_GetValue(ewma);
 *
 *     parcEWMA_Release(&ewma);
 * }
 * @endcode
 */
int64_t parcEWMA_Update(PARCEWMA *ewma, const int64_t value);

/**
 * Get the current exponentitally smoothed value of the filter.
 *
 * @param [in] ewma A pointer to a valid `PARCEWMA` instance.
 *
 * @return The current exponentitally smoothed value of the filter.
 *
 * Example:
 * @code
 * {
 *     PARCEWMA *ewma = parcEWMA_Create(0.75);
 *
 *     for (uint64_t i = 0; i < 10; i++) {
 *         parcEWMA_Update(ewma, i);
 *     }
 *
 *     int64_t smoothedValue = parcEWMA_GetValue(ewma);
 *
 *     parcEWMA_Release(&ewma);
 * }
 * @endcode
 */
int64_t parcEWMA_GetValue(const PARCEWMA *ewma);
#endif
