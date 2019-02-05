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
 * @file parc_AtomicUint8.h
 * @ingroup threading
 * @brief An atomically updated 8-bit unsigned integer.
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_AtomicUint8
#define PARCLibrary_parc_AtomicUint8
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_JSON.h>

#ifdef PARCLibrary_DISABLE_ATOMICS
#include <pthread.h>
struct PARCAtomicUint8;
typedef struct PARCAtomicUint8 PARCAtomicUint8;
#else
typedef uint8_t PARCAtomicUint8;
#endif


PARCAtomicUint8 *parcAtomicInteger_CreateUint8(uint8_t value);

uint8_t parcAtomicUint8_AddImpl(PARCAtomicUint8 *value, uint8_t addend);

uint8_t parcAtomicUint8_SubtractImpl(PARCAtomicUint8 *value, uint8_t subtrahend);

bool parcAtomicUint8_CompareAndSwapImpl(PARCAtomicUint8 *value, uint8_t predicate, uint8_t newValue);

#ifdef PARCLibrary_DISABLE_ATOMICS

#define parcAtomicUint8_Add                parcAtomicUint8_AddImpl

#define parcAtomicUint8_Subtract           parcAtomicUint8_SubtractImpl

#define parcAtomicUint8_CompareAndSwap     parcAtomicUint8_CompareAndSwapImpl

#else

#define parcAtomicUint8_Add(_atomic_uint8_, _addend_) \
    __sync_add_and_fetch(_atomic_uint8_, _addend_)

#define parcAtomicUint8_Subtract(_atomic_uint8_, _subtrahend_) \
    __sync_sub_and_fetch(_atomic_uint8_, _subtrahend_)

#define parcAtomicUint8_CompareAndSwap(_atomic_uint8_, _predicate_, _newValue_) \
    __sync_bool_compare_and_swap(_atomic_uint8_, _predicate_, _newValue_)
#endif

#define parcAtomicUint8_Increment(_atomic_uint8_) parcAtomicUint8_Add(_atomic_uint8_, 1)
#define parcAtomicUint8_Decrement(_atomic_uint8_) parcAtomicUint8_Subtract(_atomic_uint8_, 1)

/**
 * Increase the number of references to a `PARCAtomicUint8` instance.
 *
 * Note that new `PARCAtomicUint8` is not created,
 * only that the given `PARCAtomicUint8` reference count is incremented.
 * Discard the reference by invoking `parcAtomicUint8_Release`.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint8 instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *
 *     PARCAtomicUint8 *b = parcAtomicUint8_Acquire();
 *
 *     parcAtomicUint8_Release(&a);
 *     parcAtomicUint8_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint8 *parcAtomicUint8_Acquire(const PARCAtomicUint8 *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcAtomicUint8_OptionalAssertValid(_instance_)
#else
#  define parcAtomicUint8_OptionalAssertValid(_instance_) parcAtomicUint8_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCAtomicUint8` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint8 instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *
 *     parcAtomicUint8_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcAtomicUint8_Release(&b);
 * }
 * @endcode
 */
void parcAtomicUint8_AssertValid(const PARCAtomicUint8 *instance);

/**
 * Create an instance of PARCAtomicUint8
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCAtomicUint8 instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *
 *     parcAtomicUint8_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint8 *parcAtomicUint8_Create(uint8_t);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint8 instance.
 * @param [in] other A pointer to a valid PARCAtomicUint8 instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *     PARCAtomicUint8 *b = parcAtomicUint8_Create();
 *
 *     if (parcAtomicUint8_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint8_Release(&a);
 *     parcAtomicUint8_Release(&b);
 * }
 * @endcode
 *
 * @see parcAtomicUint8_Equals
 */
int parcAtomicUint8_Compare(const PARCAtomicUint8 *instance, const PARCAtomicUint8 *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCAtomicUint8 instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCAtomicUint8` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *
 *     PARCAtomicUint8 *copy = parcAtomicUint8_Copy(&b);
 *
 *     parcAtomicUint8_Release(&b);
 *     parcAtomicUint8_Release(&copy);
 * }
 * @endcode
 */
PARCAtomicUint8 *parcAtomicUint8_Copy(const PARCAtomicUint8 *original);

/**
 * Determine if two `PARCAtomicUint8` instances are equal.
 *
 * The following equivalence relations on non-null `PARCAtomicUint8` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcAtomicUint8_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcAtomicUint8_Equals(x, y)` must return true if and only if
 *        `parcAtomicUint8_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcAtomicUint8_Equals(x, y)` returns true and
 *        `parcAtomicUint8_Equals(y, z)` returns true,
 *        then `parcAtomicUint8_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcAtomicUint8_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcAtomicUint8_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCAtomicUint8 instance.
 * @param [in] y A pointer to a valid PARCAtomicUint8 instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *     PARCAtomicUint8 *b = parcAtomicUint8_Create();
 *
 *     if (parcAtomicUint8_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint8_Release(&a);
 *     parcAtomicUint8_Release(&b);
 * }
 * @endcode
 * @see parcAtomicUint8_HashCode
 */
bool parcAtomicUint8_Equals(const PARCAtomicUint8 *x, const PARCAtomicUint8 *y);

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
 * If two instances are equal according to the {@link parcAtomicUint8_Equals} method,
 * then calling the {@link parcAtomicUint8_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcAtomicUint8_Equals} function,
 * then calling the `parcAtomicUint8_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint8 instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *
 *     PARCHashCode hashValue = parcAtomicUint8_HashCode(buffer);
 *     parcAtomicUint8_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcAtomicUint8_HashCode(const PARCAtomicUint8 *instance);

/**
 * Determine if an instance of `PARCAtomicUint8` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint8 instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *
 *     if (parcAtomicUint8_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcAtomicUint8_Release(&b);
 * }
 * @endcode
 */
bool parcAtomicUint8_IsValid(const PARCAtomicUint8 *instance);

/**
 * Release a previously acquired reference to the given `PARCAtomicUint8` instance,
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
 *     PARCAtomicUint8 *a = parcAtomicUint8_Create();
 *
 *     parcAtomicUint8_Release(&a);
 * }
 * @endcode
 */
void parcAtomicUint8_Release(PARCAtomicUint8 **instancePtr);

/**
 * Get the current value of the given `PARCAtomicUint8` instance.
 *
 * @param [in] instance A pointer to a valid `PARCAtomicUint8` instance.
 *
 * @return the current value of the given `PARCAtomicUint8` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);
 *
 *     uint8_t value = parcAtomicUint8_GetValue(instance);
 *
 *     parcAtomicUint8_Release(&instance);
 * }
 * @endcode
 */
uint8_t parcAtomicUint8_GetValue(const PARCAtomicUint8 *instance);
#endif
