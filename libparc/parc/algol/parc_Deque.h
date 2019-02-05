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
 * @file parc_Deque.h
 * @ingroup datastructures
 * @brief PARC Double-ended Queue (Deque)
 *
 *
 */
#ifndef libparc_parc_Deque_h
#define libparc_parc_Deque_h
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_List.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Iterator.h>

struct parc_deque;
/**
 * A double-ended queue.
 *
 * @see {@link parcDeque_Create}
 * @see {@link parcDeque_CreateCustom}
 */
typedef struct parc_deque PARCDeque;

/**
 * Create a `PARCDeque` instance with the default element equality and copy functions.
 *
 * The queue is created with no elements.
 *
 * The default element equals function is used by the `{@link parcDeque_Equals} function and
 * simply compares the values using the `==` operator.
 * Users that need more sophisticated comparisons of the elements need to supply their own
 * function via the {@link parcDeque_CreateCustom} function.
 *
 * @return non-NULL A pointer to a `PARCDeque` instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
PARCDeque *parcDeque_Create(void);

PARCIterator *parcDeque_Iterator(PARCDeque *deque);

/**
 * Create a PARCDeque instance that uses the {@link PARCObjectDescriptor} providing functions for element equality and copy function.
 *
 * The queue is created with no elements.
 *
 * @param [in] interface A pointer to a `PARCObjectDescriptor` instance.
 *
 * @return non-NULL A pointer to a `PARCDeque` instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCDeque *parcDeque_CreateObjectInterface(const PARCObjectDescriptor *interface);

/**
 * Create a `PARCDeque` instance with a custom element equality and copy function.
 *
 * The queue is created with no elements.
 *
 * The supplied element equals function is used by the `parcDeque_Equals`
 * function which must return `true` if the elements are equal, and `false` if unequal.
 *
 * @param [in] elementEquals The function to be used for equals
 * @param [in] elementCopy The function to be used for copy
 *
 * @return non-NULL A pointer to a `PARCDeque` instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see {@link parcDeque_CreateObjectInterface}
 */
PARCDeque *parcDeque_CreateCustom(bool (*elementEquals)(const void *, const void *), void *(*elementCopy)(const void *));

/**
 * Acquire a new reference to an instance of `PARCDeque`.
 *
 * The reference count to the instance is incremented.
 *
 * @param [in] deque The instance of `PARCDeque` to which to refer.
 *
 * @return The same value as the input parameter @p deque
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCDeque *parcDeque_Acquire(const PARCDeque *deque);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's interface will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] dequePtr A pointer to a pointer to the instance of `PARCDeque` to release.
 *
 *
 * Example:
 * @code
 * {
 *     PARCDeque *buffer = parcDeque_Create(10);
 *
 *     parcDeque_Release(&buffer);
 * }
 * @endcode
 */
void parcDeque_Release(PARCDeque **dequePtr);

/**
 * Copy a a `PARCDeque` to another.
 *
 * @param [in] deque A pointer to an instance of `PARCDeque`
 *
 * @return A pointer to a copy of the original instance of `PARCDeque`
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCDeque *parcDeque_Copy(const PARCDeque *deque);

/**
 * Append an element to the tail end of the specified `PARCDeque`
 *
 * @param [in,out] deque A pointer to the instance of `PARCDeque` to which the element will be appended
 * @param [in] element A pointer to the element to be appended to the instance of `PARCDeque`
 *
 * @return  non NULL A pointer to the specific instance of `PARCDeque`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCDeque *parcDeque_Append(PARCDeque *deque, void *element);

/**
 * Prepend an element to the head end of the specified `PARCDeque`
 *
 *
 * @param [in,out] deque A pointer to the instance of `PARCDeque` to which the element will be prepended
 * @param [in] element A pointer to the element to be appended to the instance of `PARCDeque`
 *
 * @return  non NULL A pointer to the specific instance of `PARCDeque`
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCDeque *parcDeque_Prepend(PARCDeque *deque, void *element);

/**
 * Return the first element of the specified `PARCDeque` and remove it from the queue
 *
 * @param [in,out] deque A pointer to the instance of `PARCDeque` from which the first element will be returned and removed
 *
 * @return  non NULL A pointer to the element removed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
void *parcDeque_RemoveFirst(PARCDeque *deque);

/**
 * Return the last element of the specified `PARCDeque` and remove it from the queue
 *
 * @param [in,out] deque A pointer to the instance of `PARCDeque` from which the last element will be returned and removed
 *
 * @return  non NULL A pointer to the element removed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
void *parcDeque_RemoveLast(PARCDeque *deque);

/**
 * Return the first element of the specified `PARCDeque` but do NOT remove it from the queue
 *
 * @param [in] deque A pointer to the instance of `PARCDeque` from which the first element will be returned
 *
 * @return  non NULL A pointer to the first element
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
void *parcDeque_PeekFirst(const PARCDeque *deque);

/**
 * Return the last element of the specified `PARCDeque` but do NOT remove it from the queue
 *
 * @param [in] deque A pointer to the instance of `PARCDeque` from which the last element will be returned
 *
 * @return  non NULL A pointer to the last element
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
void *parcDeque_PeekLast(const PARCDeque *deque);

/**
 * Return the size of the specified queue
 *
 * @param [in] deque A pointer to the instance of `PARCDeque`
 *
 * @return `size_t` The size of the queue
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
size_t parcDeque_Size(const PARCDeque *deque);

/**
 * Return True if the `PARCDeque` is empty or False if not.
 *
 * @param [in] deque A pointer to the instance of `PARCDeque`
 *
 * @return bool True if the `PARCDeque` is empty or False if not.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
bool parcDeque_IsEmpty(const PARCDeque *deque);

/**
 * Get a pointer to the specified element.
 *
 * @param [in] deque A pointer to a `PARCDeque` instance.
 * @param [in] index The index of the element to be retrieved.
 *
 * @throws `parcTrapOutOfBounds`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
void *parcDeque_GetAtIndex(const PARCDeque *deque, size_t index);

/**
 * Determine if two `PARCDeque` instances are equal.
 *
 * This function implements the following equivalence relations on non-null `PARCDeque` instances:
 *
 *   * It is reflexive: for any non-null reference value x, `parcDeque_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcDeque_Equals(x, y)` must return true if and only if
 *        `parcDeque_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcDeque_Equals(x, y)` returns true and
 *        `parcDeque_Equals(y, z)` returns true,
 *        then `parcDeque_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcDeque_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcDeque_Equals(x, NULL)` must return false.
 *
 * Two `PARCDeque` instances with different element equality functions are always unequal.
 *
 * @param [in] x A pointer to a `PARCDeque` instance.
 * @param [in] y A pointer to a `PARCDeque` instance.
 *
 * @return true `PARCDeque` x and y are equal.
 * @return false `PARCDeque` x and y are not equal.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcDeque_Equals(const PARCDeque *x, const PARCDeque *y);

/**
 * Print a human readable representation of the given `PARCDeque`.
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] deque A pointer to the instance to display.
 *
 * Example:
 * @code
 * {
 *     PARCDeque *instance = parcDeque_Create();
 *
 *     parcDeque_Display(instance, 0);
 *
 *     parcDeque_Release(&instance);
 * }
 * @endcode
 *
 */
void parcDeque_Display(const PARCDeque *deque, const int indentation);

/**
 * Wakes up a single thread that is waiting on this object (see `parcDeque_Wait)`.
 * If any threads are waiting on this object, one of them is chosen to be awakened.
 * The choice is arbitrary and occurs at the discretion of the underlying implementation.
 *
 * The awakened thread will not be able to proceed until the current thread relinquishes the lock on this object.
 * The awakened thread will compete in the usual manner with any other threads that might be actively
 * competing to synchronize on this object;
 * for example, the awakened thread enjoys no reliable privilege or disadvantage in being the next thread to lock this object.
 *
 * @param [in] object A pointer to a valid PARCDeque instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcDeque_Notify(object);
 * }
 * @endcode
 */
parcObject_ImplementNotify(parcDeque, PARCDeque);

/**
 * Causes the calling thread to wait until either another thread invokes the parcDeque_Notify() function on the same object.
 *  *
 * @param [in] object A pointer to a valid `PARCDeque` instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcDeque_Wait(object);
 * }
 * @endcode
 */
parcObject_ImplementWait(parcDeque, PARCDeque);

/**
 * Obtain the lock on the given `PARCDeque` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] object A pointer to a valid `PARCDeque` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, or the `PARCDeque` is invalid.
 *
 * Example:
 * @code
 * {
 *     if (parcDeque_Lock(object)) {
 *
 *     }
 * }
 * @endcode
 */
parcObject_ImplementLock(parcDeque, PARCDeque);

/**
 * Try to obtain the advisory lock on the given PARCDeque instance.
 *
 * Once the lock is obtained, the caller must release the lock as soon as possible.
 *
 * @param [in] object A pointer to a valid PARCDeque instance.
 *
 * @return true The PARCDeque is locked.
 * @return false The PARCDeque is unlocked.
 *
 * Example:
 * @code
 * {
 *     parcDeque_TryLock(object);
 * }
 * @endcode
 */
parcObject_ImplementTryLock(parcDeque, PARCDeque);

/**
 * Try to unlock the advisory lock on the given `PARCDeque` instance.
 *
 * @param [in] object A pointer to a valid `PARCDeque` instance.
 *
 * @return true The `PARCDeque` was locked and now is unlocked.
 * @return false The `PARCDeque` was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     parcDeque_Unlock(object);
 * }
 * @endcode
 */
parcObject_ImplementUnlock(parcDeque, PARCDeque);

/**
 * Determine if the advisory lock on the given `PARCDeque` instance is locked.
 *
 * @param [in] object A pointer to a valid `PARCDeque` instance.
 *
 * @return true The `PARCDeque` is locked.
 * @return false The `PARCDeque` is unlocked.
 * Example:
 * @code
 * {
 *     if (parcDeque_IsLocked(object)) {
 *         ...
 *     }
 * }
 * @endcode
 */
parcObject_ImplementIsLocked(parcDeque, PARCDeque);

#endif // libparc_parc_Deque_h
