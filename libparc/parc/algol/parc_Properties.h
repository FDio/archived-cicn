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
 * @file parc_Properties.h
 * @ingroup types
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_Properties
#define PARCLibrary_parc_Properties
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_Iterator.h>

struct PARCProperties;
typedef struct PARCProperties PARCProperties;

/**
 * Increase the number of references to a `PARCProperties` instance.
 *
 * Note that new `PARCProperties` is not created,
 * only that the given `PARCProperties` reference count is incremented.
 * Discard the reference by invoking `parcProperties_Release`.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     PARCProperties *b = parcProperties_Acquire();
 *
 *     parcProperties_Release(&a);
 *     parcProperties_Release(&b);
 * }
 * @endcode
 */
PARCProperties *parcProperties_Acquire(const PARCProperties *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcProperties_OptionalAssertValid(_instance_)
#else
#  define parcProperties_OptionalAssertValid(_instance_) parcProperties_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCProperties` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     parcProperties_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcProperties_Release(&b);
 * }
 * @endcode
 */
void parcProperties_AssertValid(const PARCProperties *instance);

/**
 * Create an instance of PARCProperties
 *
 * @return non-NULL A pointer to a valid PARCProperties instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     parcProperties_Release(&a);
 * }
 * @endcode
 */
PARCProperties *parcProperties_Create(void);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 * @param [in] other A pointer to a valid PARCProperties instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *     PARCProperties *b = parcProperties_Create();
 *
 *     if (parcProperties_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcProperties_Release(&a);
 *     parcProperties_Release(&b);
 * }
 * @endcode
 *
 * @see parcProperties_Equals
 */
int parcProperties_Compare(const PARCProperties *instance, const PARCProperties *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCProperties instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCProperties` instance.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     PARCProperties *copy = parcProperties_Copy(&b);
 *
 *     parcProperties_Release(&b);
 *     parcProperties_Release(&copy);
 * }
 * @endcode
 */
PARCProperties *parcProperties_Copy(const PARCProperties *original);

/**
 * Print a human readable representation of the given `PARCProperties`.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     parcProperties_Display(a, 0);
 *
 *     parcProperties_Release(&a);
 * }
 * @endcode
 */
void parcProperties_Display(const PARCProperties *instance, int indentation);

/**
 * Determine if two `PARCProperties` instances are equal.
 *
 * The following equivalence relations on non-null `PARCProperties` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcProperties_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcProperties_Equals(x, y)` must return true if and only if
 *        `parcProperties_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcProperties_Equals(x, y)` returns true and
 *        `parcProperties_Equals(y, z)` returns true,
 *        then `parcProperties_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcProperties_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcProperties_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCProperties instance.
 * @param [in] y A pointer to a valid PARCProperties instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *     PARCProperties *b = parcProperties_Create();
 *
 *     if (parcProperties_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcProperties_Release(&a);
 *     parcProperties_Release(&b);
 * }
 * @endcode
 * @see parcProperties_HashCode
 */
bool parcProperties_Equals(const PARCProperties *x, const PARCProperties *y);

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
 * If two instances are equal according to the {@link parcProperties_Equals} method,
 * then calling the {@link parcProperties_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcProperties_Equals} function,
 * then calling the `parcProperties_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     uint32_t hashValue = parcProperties_HashCode(buffer);
 *     parcProperties_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcProperties_HashCode(const PARCProperties *instance);

/**
 * Determine if an instance of `PARCProperties` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     if (parcProperties_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcProperties_Release(&a);
 * }
 * @endcode
 *
 */
bool parcProperties_IsValid(const PARCProperties *instance);

/**
 * Release a previously acquired reference to the given `PARCProperties` instance,
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
 *     PARCProperties *a = parcProperties_Create();
 *
 *     parcProperties_Release(&a);
 * }
 * @endcode
 */
void parcProperties_Release(PARCProperties **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     PARCJSON *json = parcProperties_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcProperties_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcProperties_ToJSON(const PARCProperties *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCProperties`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCProperties instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCProperties *a = parcProperties_Create();
 *
 *     char *string = parcProperties_ToString(a);
 *
 *     parcProperties_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcProperties_Display
 */
char *parcProperties_ToString(const PARCProperties *instance);

/**
 * Append a representation of the specified `PARCProperties` instance to the given `PARCBufferComposer`.
 *
 * @param [in] properties A pointer to a `PARCProperties` instance whose representation should be appended to the @p composer.
 * @param [in,out] composer A pointer to a `PARCBufferComposer` instance to be modified.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The @p composer.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *
 *     parcProperties_BuildString(instance, result);
 *
 *     char *string = parcBufferComposer_ToString(result);
 *     printf("Hello: %s\n", string);
 *     parcMemory_Deallocate(string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcProperties_BuildString(const PARCProperties *properties, PARCBufferComposer *composer);

/**
 * Set the named property to @p value.
 *
 *
 * @param [in] properties A pointer to a valid `PARCProperties` instance.
 * @param [in] name A pointer to a nul-terminate, C string name for the property.
 * @param [in] value A A pointer to a nul-terminate, C string name for the value of the property.
 *
 * @return true <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcProperties_SetProperty(PARCProperties *properties, const char *name, const char *value);


void parcProperties_SetParsedProperty(PARCProperties *properties, char *string);

/**
 * Get the string value of the named property.
 *
 * @param [in] properties A pointer to a valid PARCProperties intance.
 * @param [in] name A pointer to a nul-terminated, C string containing the name of the property.
 *
 * @return non-NULL A nul-terminated C string containing the value of the named property.
 * @return NULL The property was not found.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const char *parcProperties_GetProperty(const PARCProperties *properties, const char *restrict name);

/**
 * Return the string value of the named property, if present.
 * Otherwise return the default value.
 *
 * @param [in] properties A pointer to a valid PARCProperties instance.
 * @param [in] name A pointer to a nul-terminated, C string containing the name of the property.
 * @param [in] defaultValue A pointer to a nul-terminated, C string containing the name of the property.
 *
 * @return non-NULL A nul-terminated C string containing the value of the named property.
 * @return NULL The property was not found, and the default property was NULL.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const char *parcProperties_GetPropertyDefault(const PARCProperties *properties, const char *restrict name, const char *restrict defaultValue);

/**
 * Return the boolean value of the named property, if present.
 * Otherwise return the default value.
 *
 * @param [in] properties A pointer to a valid PARCProperties instance.
 * @param [in] name A pointer to a nul-terminated, C string containing the name of the property.
 * @param [in] defaultValue The default boolean value.
 *
 * @return true A the property is set to true.
 * @return false The property is either not present, or is not set to true.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcProperties_GetAsBoolean(const PARCProperties *properties, const char *name, bool defaultValue);

/**
 * Return the integer value of the named property, if present.
 * Otherwise return the default value.
 *
 * @param [in] properties A pointer to a valid PARCProperties instance.
 * @param [in] name A pointer to a nul-terminated, C string containing the name of the property.
 * @param [in] defaultValue The default integer value.
 *
 * @return The integer value of the named property.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
int64_t parcProperties_GetAsInteger(const PARCProperties *properties, const char *name, int64_t defaultValue);

/**
 * Create a new instance of PARCIterator that iterates through the given PARCProperties.
 * The returned value must be released via {@link parcIterator_Release}.
 *
 * @param [in] list A pointer to a valid `PARCProperties`.
 *
 * @see parcIterator_Release
 *
 * Example:
 * @code
 * {
 *    PARCIterator *iterator = parcLinkedList_CreateIterator(hashMap);
 *
 *    while (parcIterator_HasNext(iterator)) {
 *        PARCObject *object = parcIterator_Next(iterator);
 *    }
 *
 *    parcIterator_Release(&iterator);
 * }
 * @endcode
 */
PARCIterator *parcProperties_CreateIterator(const PARCProperties *properties);
#endif
