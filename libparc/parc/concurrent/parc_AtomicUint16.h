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
 * @file parc_AtomicUint16.h
 * @ingroup threading
 * @brief An atomically updated 16-bit unsigned integer.
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_AtomicUint16
#define PARCLibrary_parc_AtomicUint16
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_JSON.h>

#ifdef PARCLibrary_DISABLE_ATOMICS
#include <pthread.h>
struct PARCAtomicUint16;
typedef struct PARCAtomicUint16 PARCAtomicUint16;
#else
typedef uint16_t PARCAtomicUint16;
#endif


PARCAtomicUint16 *parcAtomicInteger_CreateUint16(uint16_t value);

uint16_t parcAtomicUint16_AddImpl(PARCAtomicUint16 *value, uint16_t addend);

uint16_t parcAtomicUint16_SubtractImpl(PARCAtomicUint16 *value, uint16_t subtrahend);

bool parcAtomicUint16_CompareAndSwapImpl(PARCAtomicUint16 *value, uint16_t predicate, uint16_t newValue);

#ifdef PARCLibrary_DISABLE_ATOMICS

#define parcAtomicUint16_Add                parcAtomicUint16_AddImpl

#define parcAtomicUint16_Subtract           parcAtomicUint16_SubtractImpl

#define parcAtomicUint16_CompareAndSwap     parcAtomicUint16_CompareAndSwapImpl

#else

#define parcAtomicUint16_Add(_atomic_uint16_, _addend_) \
    __sync_add_and_fetch(_atomic_uint16_, _addend_)

#define parcAtomicUint16_Subtract(_atomic_uint16_, _subtrahend_) \
    __sync_sub_and_fetch(_atomic_uint16_, _subtrahend_)

#define parcAtomicUint16_CompareAndSwap(_atomic_uint16_, _predicate_, _newValue_) \
    __sync_bool_compare_and_swap(_atomic_uint16_, _predicate_, _newValue_)
#endif

#define parcAtomicUint16_Increment(_atomic_uint16_) parcAtomicUint16_Add(_atomic_uint16_, 1)
#define parcAtomicUint16_Decrement(_atomic_uint16_) parcAtomicUint16_Subtract(_atomic_uint16_, 1)

/**
 * Increase the number of references to a `PARCAtomicUint16` instance.
 *
 * Note that new `PARCAtomicUint16` is not created,
 * only that the given `PARCAtomicUint16` reference count is incremented.
 * Discard the reference by invoking `parcAtomicUint16_Release`.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint16 instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *
 *     PARCAtomicUint16 *b = parcAtomicUint16_Acquire();
 *
 *     parcAtomicUint16_Release(&a);
 *     parcAtomicUint16_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint16 *parcAtomicUint16_Acquire(const PARCAtomicUint16 *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcAtomicUint16_OptionalAssertValid(_instance_)
#else
#  define parcAtomicUint16_OptionalAssertValid(_instance_) parcAtomicUint16_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCAtomicUint16` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint16 instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *
 *     parcAtomicUint16_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcAtomicUint16_Release(&b);
 * }
 * @endcode
 */
void parcAtomicUint16_AssertValid(const PARCAtomicUint16 *instance);

/**
 * Create an instance of PARCAtomicUint16
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCAtomicUint16 instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *
 *     parcAtomicUint16_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint16 *parcAtomicUint16_Create(uint16_t);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint16 instance.
 * @param [in] other A pointer to a valid PARCAtomicUint16 instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *     PARCAtomicUint16 *b = parcAtomicUint16_Create();
 *
 *     if (parcAtomicUint16_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint16_Release(&a);
 *     parcAtomicUint16_Release(&b);
 * }
 * @endcode
 *
 * @see parcAtomicUint16_Equals
 */
int parcAtomicUint16_Compare(const PARCAtomicUint16 *instance, const PARCAtomicUint16 *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCAtomicUint16 instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCAtomicUint16` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *
 *     PARCAtomicUint16 *copy = parcAtomicUint16_Copy(&b);
 *
 *     parcAtomicUint16_Release(&b);
 *     parcAtomicUint16_Release(&copy);
 * }
 * @endcode
 */
PARCAtomicUint16 *parcAtomicUint16_Copy(const PARCAtomicUint16 *original);

/**
 * Determine if two `PARCAtomicUint16` instances are equal.
 *
 * The following equivalence relations on non-null `PARCAtomicUint16` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcAtomicUint16_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcAtomicUint16_Equals(x, y)` must return true if and only if
 *        `parcAtomicUint16_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcAtomicUint16_Equals(x, y)` returns true and
 *        `parcAtomicUint16_Equals(y, z)` returns true,
 *        then `parcAtomicUint16_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcAtomicUint16_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcAtomicUint16_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCAtomicUint16 instance.
 * @param [in] y A pointer to a valid PARCAtomicUint16 instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *     PARCAtomicUint16 *b = parcAtomicUint16_Create();
 *
 *     if (parcAtomicUint16_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint16_Release(&a);
 *     parcAtomicUint16_Release(&b);
 * }
 * @endcode
 * @see parcAtomicUint16_HashCode
 */
bool parcAtomicUint16_Equals(const PARCAtomicUint16 *x, const PARCAtomicUint16 *y);

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
 * If two instances are equal according to the {@link parcAtomicUint16_Equals} method,
 * then calling the {@link parcAtomicUint16_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcAtomicUint16_Equals} function,
 * then calling the `parcAtomicUint16_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint16 instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *
 *     PARCHashCode hashValue = parcAtomicUint16_HashCode(buffer);
 *     parcAtomicUint16_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcAtomicUint16_HashCode(const PARCAtomicUint16 *instance);

/**
 * Determine if an instance of `PARCAtomicUint16` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint16 instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *
 *     if (parcAtomicUint16_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcAtomicUint16_Release(&b);
 * }
 * @endcode
 */
bool parcAtomicUint16_IsValid(const PARCAtomicUint16 *instance);

/**
 * Release a previously acquired reference to the given `PARCAtomicUint16` instance,
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
 *     PARCAtomicUint16 *a = parcAtomicUint16_Create();
 *
 *     parcAtomicUint16_Release(&a);
 * }
 * @endcode
 */
void parcAtomicUint16_Release(PARCAtomicUint16 **instancePtr);

/**
 * Get the current value of the given `PARCAtomicUint16` instance.
 *
 * @param [in] instance A pointer to a valid `PARCAtomicUint16` instance.
 *
 * @return the current value of the given `PARCAtomicUint16` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);
 *
 *     uint16_t value = parcAtomicUint16_GetValue(instance);
 *
 *     parcAtomicUint16_Release(&instance);
 * }
 * @endcode
 */
uint16_t parcAtomicUint16_GetValue(const PARCAtomicUint16 *instance);
#endif
