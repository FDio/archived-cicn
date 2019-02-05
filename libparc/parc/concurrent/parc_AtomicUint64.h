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
 * @file parc_AtomicUint64.h
 * @ingroup threading
 * @brief An atomically updated 64-bit unsigned integer.
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_AtomicUint64
#define PARCLibrary_parc_AtomicUint64
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_JSON.h>

#ifdef PARCLibrary_DISABLE_ATOMICS
#include <pthread.h>
struct PARCAtomicUint64;
typedef struct PARCAtomicUint64 PARCAtomicUint64;
#else
typedef uint64_t PARCAtomicUint64;
#endif


PARCAtomicUint64 *parcAtomicInteger_CreateUint64(uint64_t value);

uint64_t parcAtomicUint64_AddImpl(PARCAtomicUint64 *value, uint64_t addend);

uint64_t parcAtomicUint64_SubtractImpl(PARCAtomicUint64 *value, uint64_t subtrahend);

bool parcAtomicUint64_CompareAndSwapImpl(PARCAtomicUint64 *value, uint64_t predicate, uint64_t newValue);

#ifdef PARCLibrary_DISABLE_ATOMICS

#define parcAtomicUint64_Add                parcAtomicUint64_AddImpl

#define parcAtomicUint64_Subtract           parcAtomicUint64_SubtractImpl

#define parcAtomicUint64_CompareAndSwap     parcAtomicUint64_CompareAndSwapImpl

#else

#define parcAtomicUint64_Add(_atomic_uint64_, _addend_) \
    __sync_add_and_fetch(_atomic_uint64_, _addend_)

#define parcAtomicUint64_Subtract(_atomic_uint64_, _subtrahend_) \
    __sync_sub_and_fetch(_atomic_uint64_, _subtrahend_)

#define parcAtomicUint64_CompareAndSwap(_atomic_uint64_, _predicate_, _newValue_) \
    __sync_bool_compare_and_swap(_atomic_uint64_, _predicate_, _newValue_)
#endif

#define parcAtomicUint64_Increment(_atomic_uint64_) parcAtomicUint64_Add(_atomic_uint64_, 1)
#define parcAtomicUint64_Decrement(_atomic_uint64_) parcAtomicUint64_Subtract(_atomic_uint64_, 1)

/**
 * Increase the number of references to a `PARCAtomicUint64` instance.
 *
 * Note that new `PARCAtomicUint64` is not created,
 * only that the given `PARCAtomicUint64` reference count is incremented.
 * Discard the reference by invoking `parcAtomicUint64_Release`.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint64 instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *
 *     PARCAtomicUint64 *b = parcAtomicUint64_Acquire();
 *
 *     parcAtomicUint64_Release(&a);
 *     parcAtomicUint64_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint64 *parcAtomicUint64_Acquire(const PARCAtomicUint64 *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcAtomicUint64_OptionalAssertValid(_instance_)
#else
#  define parcAtomicUint64_OptionalAssertValid(_instance_) parcAtomicUint64_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCAtomicUint64` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint64 instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *
 *     parcAtomicUint64_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcAtomicUint64_Release(&b);
 * }
 * @endcode
 */
void parcAtomicUint64_AssertValid(const PARCAtomicUint64 *instance);

/**
 * Create an instance of PARCAtomicUint64
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCAtomicUint64 instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *
 *     parcAtomicUint64_Release(&b);
 * }
 * @endcode
 */
PARCAtomicUint64 *parcAtomicUint64_Create(uint64_t);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint64 instance.
 * @param [in] other A pointer to a valid PARCAtomicUint64 instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *     PARCAtomicUint64 *b = parcAtomicUint64_Create();
 *
 *     if (parcAtomicUint64_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint64_Release(&a);
 *     parcAtomicUint64_Release(&b);
 * }
 * @endcode
 *
 * @see parcAtomicUint64_Equals
 */
int parcAtomicUint64_Compare(const PARCAtomicUint64 *instance, const PARCAtomicUint64 *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCAtomicUint64 instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCAtomicUint64` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *
 *     PARCAtomicUint64 *copy = parcAtomicUint64_Copy(&b);
 *
 *     parcAtomicUint64_Release(&b);
 *     parcAtomicUint64_Release(&copy);
 * }
 * @endcode
 */
PARCAtomicUint64 *parcAtomicUint64_Copy(const PARCAtomicUint64 *original);

/**
 * Determine if two `PARCAtomicUint64` instances are equal.
 *
 * The following equivalence relations on non-null `PARCAtomicUint64` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcAtomicUint64_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcAtomicUint64_Equals(x, y)` must return true if and only if
 *        `parcAtomicUint64_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcAtomicUint64_Equals(x, y)` returns true and
 *        `parcAtomicUint64_Equals(y, z)` returns true,
 *        then `parcAtomicUint64_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcAtomicUint64_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcAtomicUint64_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCAtomicUint64 instance.
 * @param [in] y A pointer to a valid PARCAtomicUint64 instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *     PARCAtomicUint64 *b = parcAtomicUint64_Create();
 *
 *     if (parcAtomicUint64_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcAtomicUint64_Release(&a);
 *     parcAtomicUint64_Release(&b);
 * }
 * @endcode
 * @see parcAtomicUint64_HashCode
 */
bool parcAtomicUint64_Equals(const PARCAtomicUint64 *x, const PARCAtomicUint64 *y);

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
 * If two instances are equal according to the {@link parcAtomicUint64_Equals} method,
 * then calling the {@link parcAtomicUint64_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcAtomicUint64_Equals} function,
 * then calling the `parcAtomicUint64_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint64 instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *
 *     PARCHashCode hashValue = parcAtomicUint64_HashCode(buffer);
 *     parcAtomicUint64_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcAtomicUint64_HashCode(const PARCAtomicUint64 *instance);

/**
 * Determine if an instance of `PARCAtomicUint64` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCAtomicUint64 instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *
 *     if (parcAtomicUint64_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcAtomicUint64_Release(&b);
 * }
 * @endcode
 */
bool parcAtomicUint64_IsValid(const PARCAtomicUint64 *instance);

/**
 * Release a previously acquired reference to the given `PARCAtomicUint64` instance,
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
 *     PARCAtomicUint64 *a = parcAtomicUint64_Create();
 *
 *     parcAtomicUint64_Release(&a);
 * }
 * @endcode
 */
void parcAtomicUint64_Release(PARCAtomicUint64 **instancePtr);

/**
 * Get the current value of the given `PARCAtomicUint64` instance.
 *
 * @param [in] instance A pointer to a valid `PARCAtomicUint64` instance.
 *
 * @return the current value of the given `PARCAtomicUint64` instance.
 *
 * Example:
 * @code
 * {
 *     PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);
 *
 *     uint64_t value = parcAtomicUint64_GetValue(instance);
 *
 *     parcAtomicUint64_Release(&instance);
 * }
 * @endcode
 */
uint64_t parcAtomicUint64_GetValue(const PARCAtomicUint64 *instance);
#endif
