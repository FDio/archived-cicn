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
 * @file parc_AtomicUint32.h
 * @ingroup threading
 * @brief An atomically updated 32-bit unsigned integer.
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_AtomicUint32
#define PARCLibrary_parc_AtomicUint32
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_JSON.h>

#ifdef PARCLibrary_DISABLE_ATOMICS
#include <pthread.h>
struct PARCAtomicUint32;
typedef struct PARCAtomicUint32 PARCAtomicUint32;
#else
typedef uint32_t PARCAtomicUint32;
#endif


PARCAtomicUint32 *parcAtomicInteger_CreateUint32(uint32_t value);

uint32_t parcAtomicUint32_AddImpl(PARCAtomicUint32 *value, uint32_t addend);

uint32_t parcAtomicUint32_SubtractImpl(PARCAtomicUint32 *value, uint32_t subtrahend);

bool parcAtomicUint32_CompareAndSwapImpl(PARCAtomicUint32 *value, uint32_t predicate, uint32_t newValue);

#ifdef PARCLibrary_DISABLE_ATOMICS

#define parcAtomicUint32_Add                parcAtomicUint32_AddImpl

#define parcAtomicUint32_Subtract           parcAtomicUint32_SubtractImpl

#define parcAtomicUint32_CompareAndSwap     parcAtomicUint32_CompareAndSwapImpl

#else

#define parcAtomicUint32_Add(_atomic_uint32_, _addend_) \
    __sync_add_and_fetch(_atomic_uint32_, _addend_)

#define parcAtomicUint32_Subtract(_atomic_uint32_, _subtrahend_) \
    __sync_sub_and_fetch(_atomic_uint32_, _subtrahend_)

#define parcAtomicUint32_CompareAndSwap(_atomic_uint32_, _predicate_, _newValue_) \
    __sync_bool_compare_and_swap(_atomic_uint32_, _predicate_, _newValue_)
#endif

#define parcAtomicUint32_Increment(_atomic_uint32_) parcAtomicUint32_Add(_atomic_uint32_, 1)
#define parcAtomicUint32_Decrement(_atomic_uint32_) parcAtomicUint32_Subtract(_atomic_uint32_, 1)

/**
 * Increase the number of references to a `PARCAtomicUint32` instance.
 *
 * Note that new `PARCAtomicUint32` is not created,
 * only that the given `PARCAtomicUint32` reference count is incremented.
 * Discard the reference by invoking `parcAtomicUint32_Release`.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint32 instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *
 *     PARCAtomicUint32 *b = parcAtomicUint32_Acquire();
 *
 *     parcAtomicUint32_Release(&a);
 *     parcAtomicUint32_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint32 *parcAtomicUint32_Acquire(const PARCAtomicUint32 *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcAtomicUint32_OptionalAssertValid(_instance_)
#else
#  define parcAtomicUint32_OptionalAssertValid(_instance_) parcAtomicUint32_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCAtomicUint32` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint32 instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *
 *     parcAtomicUint32_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcAtomicUint32_Release(&b);
 * }
 * @endcode
 */
void parcAtomicUint32_AssertValid(const PARCAtomicUint32 *instance);

/**
 * Create an instance of PARCAtomicUint32
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCAtomicUint32 instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *
 *     parcAtomicUint32_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint32 *parcAtomicUint32_Create(uint32_t);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint32 instance.
 * @param [in] other A pointer to a valid PARCAtomicUint32 instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *     PARCAtomicUint32 *b = parcAtomicUint32_Create();
 *
 *     if (parcAtomicUint32_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint32_Release(&a);
 *     parcAtomicUint32_Release(&b);
 * }
 * @endcode
 *
 * @see parcAtomicUint32_Equals
 */
int parcAtomicUint32_Compare(const PARCAtomicUint32 *instance, const PARCAtomicUint32 *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCAtomicUint32 instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCAtomicUint32` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *
 *     PARCAtomicUint32 *copy = parcAtomicUint32_Copy(&b);
 *
 *     parcAtomicUint32_Release(&b);
 *     parcAtomicUint32_Release(&copy);
 * }
 * @endcode
 */
PARCAtomicUint32 *parcAtomicUint32_Copy(const PARCAtomicUint32 *original);

/**
 * Determine if two `PARCAtomicUint32` instances are equal.
 *
 * The following equivalence relations on non-null `PARCAtomicUint32` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcAtomicUint32_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcAtomicUint32_Equals(x, y)` must return true if and only if
 *        `parcAtomicUint32_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcAtomicUint32_Equals(x, y)` returns true and
 *        `parcAtomicUint32_Equals(y, z)` returns true,
 *        then `parcAtomicUint32_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcAtomicUint32_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcAtomicUint32_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCAtomicUint32 instance.
 * @param [in] y A pointer to a valid PARCAtomicUint32 instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *     PARCAtomicUint32 *b = parcAtomicUint32_Create();
 *
 *     if (parcAtomicUint32_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint32_Release(&a);
 *     parcAtomicUint32_Release(&b);
 * }
 * @endcode
 * @see parcAtomicUint32_HashCode
 */
bool parcAtomicUint32_Equals(const PARCAtomicUint32 *x, const PARCAtomicUint32 *y);

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
 * If two instances are equal according to the {@link parcAtomicUint32_Equals} method,
 * then calling the {@link parcAtomicUint32_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcAtomicUint32_Equals} function,
 * then calling the `parcAtomicUint32_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint32 instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *
 *     PARCHashCode hashValue = parcAtomicUint32_HashCode(buffer);
 *     parcAtomicUint32_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcAtomicUint32_HashCode(const PARCAtomicUint32 *instance);

/**
 * Determine if an instance of `PARCAtomicUint32` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint32 instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *
 *     if (parcAtomicUint32_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcAtomicUint32_Release(&b);
 * }
 * @endcode
 */
bool parcAtomicUint32_IsValid(const PARCAtomicUint32 *instance);

/**
 * Release a previously acquired reference to the given `PARCAtomicUint32` instance,
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
 *     PARCAtomicUint32 *a = parcAtomicUint32_Create();
 *
 *     parcAtomicUint32_Release(&a);
 * }
 * @endcode
 */
void parcAtomicUint32_Release(PARCAtomicUint32 **instancePtr);

/**
 * Get the current value of the given `PARCAtomicUint32` instance.
 *
 * @param [in] instance A pointer to a valid `PARCAtomicUint32` instance.
 *
 * @return the current value of the given `PARCAtomicUint32` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint32 *instance = parcAtomicUint32_Create(7);
 *
 *     uint32_t value = parcAtomicUint32_GetValue(instance);
 *
 *     parcAtomicUint32_Release(&instance);
 * }
 * @endcode
 */
uint32_t parcAtomicUint32_GetValue(const PARCAtomicUint32 *instance);
#endif
