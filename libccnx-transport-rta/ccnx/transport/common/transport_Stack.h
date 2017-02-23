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
 * @file transport_Stack.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef libccnx_ccnx_transport_Stack
#define libccnx_ccnx_transport_Stack
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

struct TransportStack;
typedef struct TransportStack TransportStack;

/**
 * Increase the number of references to a `TransportStack` instance.
 *
 * Note that new `TransportStack` is not created,
 * only that the given `TransportStack` reference count is incremented.
 * Discard the reference by invoking `transportStack_Release`.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     TransportStack *b = transportStack_Acquire();
 *
 *     transportStack_Release(&a);
 *     transportStack_Release(&b);
 * }
 * @endcode
 */
TransportStack *transportStack_Acquire(const TransportStack *instance);

#ifdef LIBRTA_DISABLE_VALIDATION
#  define transportStack_OptionalAssertValid(_instance_)
#else
#  define transportStack_OptionalAssertValid(_instance_) transportStack_AssertValid(_instance_)
#endif

/**
 * Assert that the given `TransportStack` instance is valid.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     transportStack_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     transportStack_Release(&b);
 * }
 * @endcode
 */
void transportStack_AssertValid(const TransportStack *instance);

/**
 * Create an instance of TransportStack
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid TransportStack instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     transportStack_Release(&a);
 * }
 * @endcode
 */
TransportStack *transportStack_Create(void);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 * @param [in] other A pointer to a valid TransportStack instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *     TransportStack *b = transportStack_Create();
 *
 *     if (transportStack_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     transportStack_Release(&a);
 *     transportStack_Release(&b);
 * }
 * @endcode
 *
 * @see transportStack_Equals
 */
int transportStack_Compare(const TransportStack *instance, const TransportStack *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid TransportStack instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `TransportStack` instance.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     TransportStack *copy = transportStack_Copy(&b);
 *
 *     transportStack_Release(&b);
 *     transportStack_Release(&copy);
 * }
 * @endcode
 */
TransportStack *transportStack_Copy(const TransportStack *original);

/**
 * Print a human readable representation of the given `TransportStack`.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     transportStack_Display(a, 0);
 *
 *     transportStack_Release(&a);
 * }
 * @endcode
 */
void transportStack_Display(const TransportStack *instance, int indentation);

/**
 * Determine if two `TransportStack` instances are equal.
 *
 * The following equivalence relations on non-null `TransportStack` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `transportStack_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `transportStack_Equals(x, y)` must return true if and only if
 *        `transportStack_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `transportStack_Equals(x, y)` returns true and
 *        `transportStack_Equals(y, z)` returns true,
 *        then `transportStack_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `transportStack_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `transportStack_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid TransportStack instance.
 * @param [in] y A pointer to a valid TransportStack instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *     TransportStack *b = transportStack_Create();
 *
 *     if (transportStack_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     transportStack_Release(&a);
 *     transportStack_Release(&b);
 * }
 * @endcode
 * @see transportStack_HashCode
 */
bool transportStack_Equals(const TransportStack *x, const TransportStack *y);

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
 * If two instances are equal according to the {@link transportStack_Equals} method,
 * then calling the {@link transportStack_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link transportStack_Equals} function,
 * then calling the `transportStack_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     PARCHashCode hashValue = transportStack_HashCode(buffer);
 *     transportStack_Release(&a);
 * }
 * @endcode
 */
PARCHashCode transportStack_HashCode(const TransportStack *instance);

/**
 * Determine if an instance of `TransportStack` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     if (transportStack_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     transportStack_Release(&a);
 * }
 * @endcode
 *
 */
bool transportStack_IsValid(const TransportStack *instance);

/**
 * Release a previously acquired reference to the given `TransportStack` instance,
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
 *     TransportStack *a = transportStack_Create();
 *
 *     transportStack_Release(&a);
 * }
 * @endcode
 */
void transportStack_Release(TransportStack **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     PARCJSON *json = transportStack_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     transportStack_Release(&a);
 * }
 * @endcode
 */
PARCJSON *transportStack_ToJSON(const TransportStack *instance);

/**
 * Produce a null-terminated string representation of the specified `TransportStack`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid TransportStack instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     TransportStack *a = transportStack_Create();
 *
 *     char *string = transportStack_ToString(a);
 *
 *     transportStack_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see transportStack_Display
 */
char *transportStack_ToString(const TransportStack *instance);
#endif
