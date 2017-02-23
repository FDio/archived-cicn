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
 * @file parc_String.h
 * @ingroup types
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_String
#define PARCLibrary_parc_String
#include <stdbool.h>
#include <string.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

typedef struct PARCString PARCString;
extern parcObjectDescriptor_Declaration(PARCString);

/**
 * Increase the number of references to a `PARCString` instance.
 *
 * Note that new `PARCString` is not created,
 * only that the given `PARCString` reference count is incremented.
 * Discard the reference by invoking `parcString_Release`.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     PARCString *b = parcString_Acquire();
 *
 *     parcString_Release(&a);
 *     parcString_Release(&b);
 * }
 * @endcode
 */
PARCString *parcString_Acquire(const PARCString *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcString_OptionalAssertValid(_instance_)
#else
#  define parcString_OptionalAssertValid(_instance_) parcString_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCString` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     parcString_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcString_Release(&b);
 * }
 * @endcode
 */
void parcString_AssertValid(const PARCString *instance);

/**
 * Create an instance of PARCString
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCString instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     parcString_Release(&a);
 * }
 * @endcode
 */
PARCString *parcString_Create(const char *);

/**
 * Create an instance of PARCString from the content of a given PARCBuffer.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCString instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     parcString_Release(&a);
 * }
 * @endcode
 */
PARCString *parcString_CreateFromBuffer(const PARCBuffer *buffer);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 * @param [in] other A pointer to a valid PARCString instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *     PARCString *b = parcString_Create();
 *
 *     if (parcString_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcString_Release(&a);
 *     parcString_Release(&b);
 * }
 * @endcode
 *
 * @see parcString_Equals
 */
int parcString_Compare(const PARCString *instance, const PARCString *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCString instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCString` instance.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     PARCString *copy = parcString_Copy(&b);
 *
 *     parcString_Release(&b);
 *     parcString_Release(&copy);
 * }
 * @endcode
 */
PARCString *parcString_Copy(const PARCString *original);

/**
 * Print a human readable representation of the given `PARCString`.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     parcString_Display(a, 0);
 *
 *     parcString_Release(&a);
 * }
 * @endcode
 */
void parcString_Display(const PARCString *instance, int indentation);

/**
 * Determine if two `PARCString` instances are equal.
 *
 * The following equivalence relations on non-null `PARCString` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcString_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcString_Equals(x, y)` must return true if and only if
 *        `parcString_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcString_Equals(x, y)` returns true and
 *        `parcString_Equals(y, z)` returns true,
 *        then `parcString_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcString_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcString_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCString instance.
 * @param [in] y A pointer to a valid PARCString instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *     PARCString *b = parcString_Create();
 *
 *     if (parcString_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcString_Release(&a);
 *     parcString_Release(&b);
 * }
 * @endcode
 * @see parcString_HashCode
 */
bool parcString_Equals(const PARCString *x, const PARCString *y);

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
 * If two instances are equal according to the {@link parcString_Equals} method,
 * then calling the {@link parcString_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcString_Equals} function,
 * then calling the `parcString_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     PARCHashCode hashValue = parcString_HashCode(buffer);
 *     parcString_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcString_HashCode(const PARCString *instance);

/**
 * Determine if an instance of `PARCString` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     if (parcString_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcString_Release(&a);
 * }
 * @endcode
 *
 */
bool parcString_IsValid(const PARCString *instance);

/**
 * Release a previously acquired reference to the given `PARCString` instance,
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
 *     PARCString *a = parcString_Create();
 *
 *     parcString_Release(&a);
 * }
 * @endcode
 */
void parcString_Release(PARCString **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     PARCJSON *json = parcString_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcString_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcString_ToJSON(const PARCString *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCString`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCString instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCString *a = parcString_Create();
 *
 *     char *string = parcString_ToString(a);
 *
 *     parcString_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcString_Display
 */
char *parcString_ToString(const PARCString *instance);

/**
 * Get a pointer to the underlying nul-terminated sequence of bytes containing the string's value.
 *
 * @param [in] string A pointer to a valid PARCString instance.
 *
 * @return A pointer to the underlying nul-terminated sequence of bytes
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const char *parcString_GetString(const PARCString *string);
#endif
