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
 * @file parc_HashMap.h
 * @ingroup datastructures
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_HashMap
#define PARCLibrary_parc_HashMap
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_Iterator.h>

struct PARCHashMap;
typedef struct PARCHashMap PARCHashMap;

/**
 * Increase the number of references to a `PARCHashMap` instance.
 *
 * Note that new `PARCHashMap` is not created,
 * only that the given `PARCHashMap` reference count is incremented.
 * Discard the reference by invoking `parcHashMap_Release`.
 *
 * @param [in] instance A pointer to a valid PARCHashMap instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     PARCHashMap *b = parcHashMap_Acquire();
 *
 *     parcHashMap_Release(&a);
 *     parcHashMap_Release(&b);
 * }
 * @endcode
 */
PARCHashMap *parcHashMap_Acquire(const PARCHashMap *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcHashMap_OptionalAssertValid(_instance_)
#else
#  define parcHashMap_OptionalAssertValid(_instance_) parcHashMap_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCHashMap` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCHashMap instance.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     parcHashMap_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcHashMap_Release(&b);
 * }
 * @endcode
 */
void parcHashMap_AssertValid(const PARCHashMap *instance);

/**
 * Constructs an empty `PARCHashMap` with a default minimum number of 'buckets'.
 *
 * The capacity will expand and contract as needed to keep load factor table
 * below the max load factor of 0.75 and above the minimum load factor or 0.25.
 * The default minimum number of buckets is 42.
 *
 * @return non-NULL A pointer to a valid PARCHashMap instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     parcHashMap_Release(&a);
 * }
 * @endcode
 */
PARCHashMap *parcHashMap_Create(void);

/**
 * Constructs an empty `PARCHashMap` with the specified minimum number of 'buckets'.
 *
 * The capacity will expand and contract as needed to keep load factor table
 * below the max load factor of 0.75 and above the minimum load factor or 0.25.
 *
 * @param [in] capacity The minimum number of buckets.  Must be greater than 0.
 *
 * @return non-NULL A pointer to a valid PARCHashMap instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_CreateCapacity(43);
 *
 *     parcHashMap_Release(&a);
 * }
 * @endcode
 */
PARCHashMap *parcHashMap_CreateCapacity(unsigned int capacity);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCHashMap instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCHashMap` instance.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     PARCHashMap *copy = parcHashMap_Copy(&b);
 *
 *     parcHashMap_Release(&b);
 *     parcHashMap_Release(&copy);
 * }
 * @endcode
 */
PARCHashMap *parcHashMap_Copy(const PARCHashMap *original);

/**
 * Print a human readable representation of the given `PARCHashMap`.
 *
 * @param [in] instance A pointer to a valid PARCHashMap instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     parcHashMap_Display(a, 0);
 *
 *     parcHashMap_Release(&a);
 * }
 * @endcode
 */
void parcHashMap_Display(const PARCHashMap *instance, int indentation);

/**
 * Determine if two `PARCHashMap` instances are equal.
 *
 * The following equivalence relations on non-null `PARCHashMap` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcHashMap_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcHashMap_Equals(x, y)` must return true if and only if
 *        `parcHashMap_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcHashMap_Equals(x, y)` returns true and
 *        `parcHashMap_Equals(y, z)` returns true,
 *        then `parcHashMap_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcHashMap_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcHashMap_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCHashMap instance.
 * @param [in] y A pointer to a valid PARCHashMap instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *     PARCHashMap *b = parcHashMap_Create();
 *
 *     if (parcHashMap_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcHashMap_Release(&a);
 *     parcHashMap_Release(&b);
 * }
 * @endcode
 * @see parcHashMap_HashCode
 */
bool parcHashMap_Equals(const PARCHashMap *x, const PARCHashMap *y);

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
 * If two instances are equal according to the {@link parcHashMap_Equals} method,
 * then calling the {@link parcHashMap_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcHashMap_Equals} function,
 * then calling the `parcHashMap_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCHashMap instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     uint32_t hashValue = parcHashMap_HashCode(buffer);
 *     parcHashMap_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcHashMap_HashCode(const PARCHashMap *instance);

/**
 * Determine if an instance of `PARCHashMap` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCHashMap instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     if (parcHashMap_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcHashMap_Release(&a);
 * }
 * @endcode
 *
 */
bool parcHashMap_IsValid(const PARCHashMap *instance);

/**
 * Release a previously acquired reference to the given `PARCHashMap` instance,
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
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     parcHashMap_Release(&a);
 * }
 * @endcode
 */
void parcHashMap_Release(PARCHashMap **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCHashMap instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     PARCJSON *json = parcHashMap_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcHashMap_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcHashMap_ToJSON(const PARCHashMap *instance);


PARCBufferComposer *parcHashMap_BuildString(const PARCHashMap *hashMap, PARCBufferComposer *composer);

/**
 * Produce a null-terminated string representation of the specified `PARCHashMap`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCHashMap instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCHashMap *a = parcHashMap_Create();
 *
 *     char *string = parcHashMap_ToString(a);
 *
 *     parcHashMap_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcHashMap_Display
 */
char *parcHashMap_ToString(const PARCHashMap *instance);

/**
 * Wakes up a single thread that is waiting on this object (see `parcLinkedList_Wait)`.
 * If any threads are waiting on this object, one of them is chosen to be awakened.
 * The choice is arbitrary and occurs at the discretion of the underlying implementation.
 *
 * The awakened thread will not be able to proceed until the current thread relinquishes the lock on this object.
 * The awakened thread will compete in the usual manner with any other threads that might be actively
 * competing to synchronize on this object;
 * for example, the awakened thread enjoys no reliable privilege or disadvantage in being the next thread to lock this object.
 *
 * @param [in] object A pointer to a valid PARCLinkedList instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcLinkedList_Notify(object);
 * }
 * @endcode
 */
parcObject_ImplementNotify(parcHashMap, PARCHashMap);

/**
 * Causes the calling thread to wait until either another thread invokes the parcHashMap_Notify() function on the same object.
 *  *
 * @param [in] object A pointer to a valid `PARCHashMap` instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcHashMap_Wait(object);
 * }
 * @endcode
 */
parcObject_ImplementWait(parcHashMap, PARCHashMap);

/**
 * Obtain the lock on the given `PARCHashMap` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] object A pointer to a valid `PARCHashMap` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, or the `PARCHashMap` is invalid.
 *
 * Example:
 * @code
 * {
 *     if (parcHashMap_Lock(object)) {
 *
 *     }
 * }
 * @endcode
 */
parcObject_ImplementLock(parcHashMap, PARCHashMap);

/**
 * Try to obtain the advisory lock on the given PARCHashMap instance.
 *
 * Once the lock is obtained, the caller must release the lock as soon as possible.
 *
 * @param [in] object A pointer to a valid PARCHashMap instance.
 *
 * @return true The PARCHashMap is locked.
 * @return false The PARCHashMap is unlocked.
 *
 * Example:
 * @code
 * {
 *     parcHashMap_TryLock(object);
 * }
 * @endcode
 */
parcObject_ImplementTryLock(parcHashMap, PARCHashMap);

/**
 * Try to unlock the advisory lock on the given `PARCHashMap` instance.
 *
 * @param [in] object A pointer to a valid `PARCHashMap` instance.
 *
 * @return true The `PARCHashMap` was locked and now is unlocked.
 * @return false The `PARCHashMap` was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     parcHashMap_Unlock(object);
 * }
 * @endcode
 */
parcObject_ImplementUnlock(parcHashMap, PARCHashMap);

/**
 * Determine if the advisory lock on the given `PARCHashMap` instance is locked.
 *
 * @param [in] object A pointer to a valid `PARCHashMap` instance.
 *
 * @return true The `PARCHashMap` is locked.
 * @return false The `PARCHashMap` is unlocked.
 * Example:
 * @code
 * {
 *     if (parcHashMap_IsLocked(object)) {
 *         ...
 *     }
 * }
 * @endcode
 */
parcObject_ImplementIsLocked(parcHashMap, PARCHashMap);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] hashMap A pointer to a valid PARCHashMap instance.
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCHashMap *parcHashMap_Put(PARCHashMap *hashMap, const PARCObject *key, const PARCObject *value);

/**
 * Returns the value to which the specified key is mapped,
 * or null if this map contains no mapping for the key.
 * If this map contains a mapping from a key _k_ to a value _v_ such that `(key==null ? k==null : key.equals(k))`,
 * then this method returns _v_; otherwise it returns null. (There can be at most one such mapping.)
 *
 * A return value of `NULL` does not necessarily indicate that the map contains no mapping for the key.
 * It is possible that the map explicitly maps the key to `NULL`.
 * Use the `parcHashMap_ContainsKey` function to distinguish these cases.
 *
 * @param [in] hashMap A pointer to a valid PARCHashMap instance.
 *
 * @return The value to which the specified key is mapped, or `NULL` if this map contains no mapping for the key
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const PARCObject *parcHashMap_Get(const PARCHashMap *hashMap, const PARCObject *key);

/**
 * Removes the mapping for the specified key from this `PARCHashMap`, if present.
 *
 * @param [in] hashMap A pointer to a valid PARCHashMap instance.
 *
 * @return true The key existed and was removed.
 * @return true The key did not exist.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcHashMap_Remove(PARCHashMap *hashMap, const PARCObject *key);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] hashMap A pointer to a valid PARCHashMap instance.
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
size_t parcHashMap_Size(const PARCHashMap *hashMap);


/**
 * Computes the standard deviation of the PARCHashMap's bucket sizes from a value of 1.0
 * (as opposed to the mean) and weighs the result by in inverse of the current load
 * factor. The deviation from 1.0 is used because the hash-map's max load factor is < 1.0
 * and thus the ideal average chain-length is 1.0.
 *
 * A result of 0.0 equates to an ideal distribution, a result of ~1.0 should represent
 * a fairly normal or random distribution, and a result > 1.5 or so implies some amount
 * of undesirable clumping may be happening.
 *
 * @param [in] hashMap A pointer to a valid PARCHashMap instance.
 *
 * @return The clustering number
 */
double parcHashMap_GetClusteringNumber(const PARCHashMap *hashMap);


/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] hashMap A pointer to a valid PARCHashMap instance.
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcHashMap_Contains(PARCHashMap *hashMap, const PARCObject *key);

/**
 * Create a new instance of PARCIterator that iterates through the values of the specified `PARCHashMap`.
 * The returned value must be released via {@link parcIterator_Release}.
 *
 * @param [in] hashMap A pointer to a valid `PARCHashMap`.
 *
 * @see parcIterator_Release
 * Example:
 * @code
 * {
 *    PARCIterator *iterator = parcHashMap_CreateValueIterator(list);
 *
 *    while (parcIterator_HasNext(iterator)) {
 *        PARCObject *object = parcIterator_Next(iterator);
 *    }
 *
 *    parcIterator_Release(&iterator);
 * }
 * @endcode
 */
PARCIterator *parcHashMap_CreateValueIterator(PARCHashMap *hashMap);

/**
 * Create a new instance of PARCIterator that iterates through the keys of the specified `PARCHashMap`.
 * The returned value must be released via {@link parcIterator_Release}.
 *
 * @param [in] hashMap A pointer to a valid `PARCHashMap`.
 *
 * @see parcIterator_Release
 * Example:
 * @code
 * {
 *    PARCIterator *iterator = parcHashMap_CreateKeyIterator(list);
 *
 *    while (parcIterator_HasNext(iterator)) {
 *        PARCObject *object = parcIterator_Next(iterator);
 *    }
 *
 *    parcIterator_Release(&iterator);
 * }
 * @endcode
 */
PARCIterator *parcHashMap_CreateKeyIterator(PARCHashMap *hashMap);
#endif
