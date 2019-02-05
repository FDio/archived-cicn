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
 * @file parc_BasicStats.h
 * @ingroup statistics
 * @brief A basic descriptive statistics implementation for time-series data.
 *
 */
#ifndef PARCLIbrary_parc_IntegerStats
#define PARCLIbrary_parc_IntegerStats
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

struct PARCBasicStats;
typedef struct PARCBasicStats PARCBasicStats;

/**
 * Increase the number of references to a `PARCBasicStats` instance.
 *
 * Note that new `PARCBasicStats` is not created,
 * only that the given `PARCBasicStats` reference count is incremented.
 * Discard the reference by invoking `parcBasicStats_Release`.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     PARCBasicStats *b = parcBasicStats_Acquire();
 *
 *     parcBasicStats_Release(&a);
 *     parcBasicStats_Release(&b);
 * }
 * @endcode
 */
PARCBasicStats *parcBasicStats_Acquire(const PARCBasicStats *instance);

#ifdef PARCLIbrary_DISABLE_VALIDATION
#  define parcBasicStats_OptionalAssertValid(_instance_)
#else
#  define parcBasicStats_OptionalAssertValid(_instance_) parcBasicStats_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCBasicStats` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     parcBasicStats_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcBasicStats_Release(&b);
 * }
 * @endcode
 */
void parcBasicStats_AssertValid(const PARCBasicStats *instance);

/**
 * Create an instance of PARCBasicStats
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCBasicStats instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     parcBasicStats_Release(&a);
 * }
 * @endcode
 */
PARCBasicStats *parcBasicStats_Create(void);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 * @param [in] other A pointer to a valid PARCBasicStats instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *     PARCBasicStats *b = parcBasicStats_Create();
 *
 *     if (parcBasicStats_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcBasicStats_Release(&a);
 *     parcBasicStats_Release(&b);
 * }
 * @endcode
 *
 * @see parcBasicStats_Equals
 */
int parcBasicStats_Compare(const PARCBasicStats *instance, const PARCBasicStats *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCBasicStats instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCBasicStats` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     PARCBasicStats *copy = parcBasicStats_Copy(&b);
 *
 *     parcBasicStats_Release(&b);
 *     parcBasicStats_Release(&copy);
 * }
 * @endcode
 */
PARCBasicStats *parcBasicStats_Copy(const PARCBasicStats *original);

/**
 * Print a human readable representation of the given `PARCBasicStats`.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     parcBasicStats_Display(a, 0);
 *
 *     parcBasicStats_Release(&a);
 * }
 * @endcode
 */
void parcBasicStats_Display(const PARCBasicStats *instance, int indentation);

/**
 * Determine if two `PARCBasicStats` instances are equal.
 *
 * The following equivalence relations on non-null `PARCBasicStats` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcBasicStats_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcBasicStats_Equals(x, y)` must return true if and only if
 *        `parcBasicStats_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcBasicStats_Equals(x, y)` returns true and
 *        `parcBasicStats_Equals(y, z)` returns true,
 *        then `parcBasicStats_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcBasicStats_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcBasicStats_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCBasicStats instance.
 * @param [in] y A pointer to a valid PARCBasicStats instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *     PARCBasicStats *b = parcBasicStats_Create();
 *
 *     if (parcBasicStats_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcBasicStats_Release(&a);
 *     parcBasicStats_Release(&b);
 * }
 * @endcode
 * @see parcBasicStats_HashCode
 */
bool parcBasicStats_Equals(const PARCBasicStats *x, const PARCBasicStats *y);

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
 * If two instances are equal according to the {@link parcBasicStats_Equals} method,
 * then calling the {@link parcBasicStats_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcBasicStats_Equals} function,
 * then calling the `parcBasicStats_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     PARCHashCode hashValue = parcBasicStats_HashCode(buffer);
 *     parcBasicStats_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcBasicStats_HashCode(const PARCBasicStats *instance);

/**
 * Determine if an instance of `PARCBasicStats` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     if (parcBasicStats_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcBasicStats_Release(&a);
 * }
 * @endcode
 *
 */
bool parcBasicStats_IsValid(const PARCBasicStats *instance);

/**
 * Release a previously acquired reference to the given `PARCBasicStats` instance,
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
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     parcBasicStats_Release(&a);
 * }
 * @endcode
 */
void parcBasicStats_Release(PARCBasicStats **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     PARCJSON *json = parcBasicStats_ToJSON(a);
 *
 *     char *cString = parcJSON_ToString(json);
 *     printf("JSON representation: %s\n", cString);
 *
 *     parcMemory_Deallocate(&cString);
 *     parcJSON_Release(&json);
 *
 *     parcBasicStats_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcBasicStats_ToJSON(const PARCBasicStats *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCBasicStats`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCBasicStats instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCBasicStats *a = parcBasicStats_Create();
 *
 *     char *string = parcBasicStats_ToString(a);
 *
 *     parcBasicStats_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcBasicStats_Display
 */
char *parcBasicStats_ToString(const PARCBasicStats *instance);

/**
 * Add a value to the observed set of values.
 *
 * @param [in] stats A pointer to a valid `PARCBasicStats` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcBasicStats_Update(PARCBasicStats *stats, double value);

/**
 * The arithmetic mean of the set of observed values.
 *
 * @param [in] stats A pointer to a valid `PARCBasicStats` instance.
 *
 * @return The arithmetic mean of the set of observed values.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
double parcBasicStats_Mean(const PARCBasicStats *stats);

/**
 * The variance of the set of observed values.
 *
 * @param [in] stats A pointer to a valid `PARCBasicStats` instance.
 *
 * @return  The variance of the set of observed values.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
double parcBasicStats_Variance(const PARCBasicStats *stats);

/**
 * The standard deviation of the set of observed values.
 *
 * @param [in] stats A pointer to a valid `PARCBasicStats` instance.
 *
 * @return  The standard deviation of the set of observed values.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
double parcBasicStats_StandardDeviation(const PARCBasicStats *stats);

/**
 * The maximum value of the set of observed values.
 *
 * @param [in] stats A pointer to a valid `PARCBasicStats` instance.
 *
 * @return The maximum value of the set of observed values.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
double parcBasicStats_Maximum(const PARCBasicStats *stats);

/**
 * The minimum value of the set of observed values.
 *
 * @param [in] stats A pointer to a valid `PARCBasicStats` instance.
 *
 * @return The minimum value of the set of observed values.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
double parcBasicStats_Minimum(const PARCBasicStats *stats);

/**
 * The arithmetic range of the observed set of values.
 *
 * @param [in] stats A pointer to a valid `PARCBasicStats` instance.
 *
 * @return The arithmetic range of the observed set of values.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
double parcBasicStats_Range(const PARCBasicStats *stats);
#endif
