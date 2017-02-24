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
 * @file ccnx_StackConfig.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef TransportLibrary_ccnx_StackConfig
#define TransportLibrary_ccnx_StackConfig
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>


struct CCNxStackConfig_;
typedef struct CCNxStackConfig_ CCNxStackConfig;

/**
 * Increase the number of references to a `CCNxStackConfig` instance.
 *
 * Note that new `CCNxStackConfig` is not created,
 * only that the given `CCNxStackConfig` reference count is incremented.
 * Discard the reference by invoking `ccnxStackConfig_Release`.
 *
 * @param [in] instance A pointer to a valid CCNxStackConfig instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     CCNxStackConfig *b = ccnxStackConfig_Acquire();
 *
 *     ccnxStackConfig_Release(&a);
 *     ccnxStackConfig_Release(&b);
 * }
 * @endcode
 */
CCNxStackConfig *ccnxStackConfig_Acquire(const CCNxStackConfig *instance);

#ifdef TransportLibrary_DISABLE_VALIDATION
#  define ccnxStackConfig_OptionalAssertValid(_instance_)
#else
#  define ccnxStackConfig_OptionalAssertValid(_instance_) ccnxStackConfig_AssertValid(_instance_)
#endif

/**
 * Assert that the given `CCNxStackConfig` instance is valid.
 *
 * @param [in] instance A pointer to a valid CCNxStackConfig instance.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     ccnxStackConfig_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     ccnxStackConfig_Release(&b);
 * }
 * @endcode
 */
void ccnxStackConfig_AssertValid(const CCNxStackConfig *instance);

/**
 * Create an instance of CCNxStackConfig
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid CCNxStackConfig instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     ccnxStackConfig_Release(&b);
 * }
 * @endcode
 */
CCNxStackConfig *ccnxStackConfig_Create(void);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid CCNxStackConfig instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `CCNxStackConfig` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     CCNxStackConfig *copy = ccnxStackConfig_Copy(&b);
 *
 *     ccnxStackConfig_Release(&b);
 *     ccnxStackConfig_Release(&copy);
 * }
 * @endcode
 */
CCNxStackConfig *ccnxStackConfig_Copy(const CCNxStackConfig *original);

/**
 * Print a human readable representation of the given `CCNxStackConfig`.
 *
 * @param [in] instance A pointer to a valid CCNxStackConfig instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     ccnxStackConfig_Display(a, 0);
 *
 *     ccnxStackConfig_Release(&b);
 * }
 * @endcode
 */
void ccnxStackConfig_Display(const CCNxStackConfig *instance, int indentation);

/**
 * Determine if two `CCNxStackConfig` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxStackConfig` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `ccnxStackConfig_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxStackConfig_Equals(x, y)` must return true if and only if
 *        `ccnxStackConfig_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxStackConfig_Equals(x, y)` returns true and
 *        `ccnxStackConfig_Equals(y, z)` returns true,
 *        then `ccnxStackConfig_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxStackConfig_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxStackConfig_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid CCNxStackConfig instance.
 * @param [in] y A pointer to a valid CCNxStackConfig instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *     CCNxStackConfig *b = ccnxStackConfig_Create();
 *
 *     if (ccnxStackConfig_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     ccnxStackConfig_Release(&a);
 *     ccnxStackConfig_Release(&b);
 * }
 * @endcode
 * @see ccnxStackConfig_HashCode
 */
bool ccnxStackConfig_Equals(const CCNxStackConfig *x, const CCNxStackConfig *y);

/**
 * Determine if an instance of `CCNxStackConfig` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid CCNxStackConfig instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     if (ccnxStackConfig_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     ccnxStackConfig_Release(&b);
 * }
 * @endcode
 *
 */
bool ccnxStackConfig_IsValid(const CCNxStackConfig *instance);

/**
 * Release a previously acquired reference to the given `CCNxStackConfig` instance,
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
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     ccnxStackConfig_Release(&a);
 * }
 * @endcode
 */
void ccnxStackConfig_Release(CCNxStackConfig **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid CCNxStackConfig instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     PARCJSON *json = ccnxStackConfig_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     ccnxStackConfig_Release(&a);
 * }
 * @endcode
 */
PARCJSON *ccnxStackConfig_ToJSON(const CCNxStackConfig *instance);

/**
 * Produce a null-terminated string representation of the specified `CCNxStackConfig`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid CCNxStackConfig instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *a = ccnxStackConfig_Create();
 *
 *     char *string = ccnxStackConfig_ToString(a);
 *
 *     ccnxStackConfig_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see ccnxStackConfig_Display
 */
char *ccnxStackConfig_ToString(const CCNxStackConfig *instance);

PARCJSONValue *ccnxStackConfig_Get(const CCNxStackConfig *config, const char *componentKey);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of the `HashCode` function is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the `HashCode` function must consistently return the same value,
 * provided no information in the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the `Equals` function,
 * then calling the `HashCode` function on each of the two instances must produce the same result.
 *
 * It is not required that if two instances are unequal according to the `Equals` function,
 * then calling the `HashCode` function
 * on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to the `CCNxStackConfig` instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *buffer = ccnxStackConfig_Allocate(10);
 *     PARCHashCode hash = ccnxStackConfig_HashCode(buffer);
 *     ccnxStackConfig_Release(&buffer);
 * }
 * @endcode
 */
PARCHashCode ccnxStackConfig_HashCode(const CCNxStackConfig *instance);

CCNxStackConfig *ccnxStackConfig_Add(CCNxStackConfig *config, const char *componentKey, PARCJSONValue *jsonObject);

PARCJSON *ccnxStackConfig_GetJson(const CCNxStackConfig *config);
#endif
