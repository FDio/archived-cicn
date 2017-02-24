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
 * @file parc_LinkedList.h
 * @ingroup datastructures
 * @brief PARC Double-ended Queue (Deque)
 *
 */
#ifndef libparc_parc_LinkedList_h
#define libparc_parc_LinkedList_h
#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_List.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Iterator.h>

struct parc_linkedlist;
/**
 * A simple linked list.
 *
 * @see {@link parcLinkedList_Create}
 */
typedef struct parc_linkedlist PARCLinkedList;

/**
 * Create a `PARCLinkedList` instance with the default element equality and copy functions.
 *
 * The queue is created with no elements.
 *
 * The default element equals function is used by the `{@link parcLinkedList_Equals} function and
 * simply compares the values using the `==` operator.
 *
 * @return non-NULL A pointer to a `PARCLinkedList` instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
PARCLinkedList *parcLinkedList_Create(void);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcLinkedList_OptionalAssertValid(_instance_)
#else
#  define parcLinkedList_OptionalAssertValid(_instance_) parcLinkedList_AssertValid(_instance_)
#endif
void parcLinkedList_AssertValid(const PARCLinkedList *list);

/**
 * Create a new instance of PARCIterator that iterates through the given PARCLinkedList.
 * The returned value must be released via {@link parcIterator_Release}.
 *
 * @param [in] list A pointer to a valid `PARCLinkedList`.
 *
 * @see parcIterator_Release
 *
 * Example:
 * @code
 * {
 *    PARCIterator *iterator = parcLinkedList_CreateIterator(list);
 *
 *    while (parcIterator_HasNext(iterator)) {
 *        PARCObject *object = parcIterator_Next(iterator);
 *    }
 *
 *    parcIterator_Release(&iterator);
 * }
 * @endcode
 */
PARCIterator *parcLinkedList_CreateIterator(PARCLinkedList *list);

/**
 * Acquire a new reference to an instance of `PARCLinkedList`.
 *
 * The reference count to the instance is incremented.
 *
 * @param [in] list The instance of `PARCLinkedList` to which to refer.
 *
 * @return The same value as the input parameter @p deque
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCLinkedList *parcLinkedList_Acquire(const PARCLinkedList *list);

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
 * @param [in,out] listPtr A pointer to a pointer to the instance of `PARCLinkedList` to release.
 *
 *
 * Example:
 * @code
 * {
 *     PARCLinkedList *buffer = parcLinkedList_Create(10);
 *
 *     parcLinkedList_Release(&buffer);
 * }
 * @endcode
 */
void parcLinkedList_Release(PARCLinkedList **listPtr);

/**
 * Copy a a `PARCLinkedList` to another.
 *
 * @param [in] list A pointer to an instance of `PARCLinkedList`
 *
 * @return A pointer to a copy of the original instance of `PARCLinkedList`
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCLinkedList *parcLinkedList_Copy(const PARCLinkedList *list);

/**
 * Determine if an instance of `PARCLinkedList` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] list A pointer to a `PARCLinkedList` instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCLinkedList *instance = parcLinkedList_Create();
 *
 *     if (parcLinkedList_IsValid(instance)) {
 *         printf("Instance is valid.\n");
 *     }
 * }
 * @endcode
 */
bool parcLinkedList_IsValid(const PARCLinkedList *list);

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
 * @param [in] instance A pointer to the `PARCLinkedList` instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCLinkedList *buffer = parcLinkedList_Allocate(10);
 *     PARCHashCode hash = parcLinkedList_HashCode(buffer);
 *     parcLinkedList_Release(&buffer);
 * }
 * @endcode
 */
PARCHashCode parcLinkedList_HashCode(const PARCLinkedList *instance);

/**
 * Returns true if the given `PARCLinkedList` contains the specified `PARCObject`.
 *
 * The semantics are such that the function returns `true` if and only if the `PARCLinkedList`
 * list contains at least one `PARCObject` _o_ such that PARCObject_Equals(_o_, object) is true.
 *
 * @param [in] list A pointer to a valid `PARCLinkedList` instance.
 * @param [in] object A pointer to a valid `PARCObject` instance.
 *
 * @return true The given `PARCLinkedList` contains the specified `PARCObject`.
 * @return false The given `PARCLinkedList` does not contain the specified `PARCObject`.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcLinkedList_Contains(const PARCLinkedList *list, const PARCObject *object);

/**
 * Append an element to the tail end of the specified `PARCLinkedList`
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList` to which the element will be appended
 * @param [in] element A pointer to the element to be appended to the instance of `PARCLinkedList`
 *
 * @return  non NULL A pointer to the specific instance of `PARCLinkedList`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCLinkedList *parcLinkedList_Append(PARCLinkedList *list, const PARCObject *element);

/**
 * Append each element from the PARCLinkedList @p other to @p list.
 *
 * @param [in] list A pointer to a valid PARCLinkedList instance that will be receive each element from @p other.
 * @param [in] other A pointer to a valid PARCLinkedList instance containing the elements to append to @p list.
 *
 * @return The value of @p list.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCLinkedList *parcLinkedList_AppendAll(PARCLinkedList *list, const PARCLinkedList *other);

/**
 * Prepend an element to the head end of the specified `PARCLinkedList`
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList` to which the element will be prepended
 * @param [in] element A pointer to the element to be appended to the instance of `PARCLinkedList`
 *
 * @return  non NULL A pointer to the specific instance of `PARCLinkedList`
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCLinkedList *parcLinkedList_Prepend(PARCLinkedList *list, const PARCObject *element);

/**
 * Return the first element of the specified `PARCLinkedList` and remove it from the list.
 * The element's reference count is not modified,
 * the caller must release the returned element when it is finished with it.
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList` from which the first element will be returned and removed
 *
 * @return  non NULL A pointer to the element removed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
PARCObject *parcLinkedList_RemoveFirst(PARCLinkedList *list);

/**
 * Remove the last element in the queue and return it.
 * The element's reference count is not modified,
 * the caller must release the returned element when it is finished with it.
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList` from which the last element will be removed and returned.
 *
 * @return non-NULL A pointer to the element removed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
PARCObject *parcLinkedList_RemoveLast(PARCLinkedList *list);

/**
 * Remove the first occurrence of the given element from the specified 'PARCLinkedList'.
 * The element's reference count is decremented.
 *
 * @param [in] element the element to remove
 * @return true if the element was found in the list and successfully removed
 *
 * Example:
 * @code
 * {
 *     PARCLinkedList *list = parcLinkedList_Create();
 *
 *     PARCBuffer *buffer = parcBuffer_WrapCString("1");
 *     parcLinkedList_Append(list, buffer);
 *     parcBuffer_Release(&buffer);
 *     // ...
 *     PARCBuffer *actual = parcLinkedList_Remove(list, buffer);
 *     parcBuffer_Release(&actual);
 *
 *     // ...
 *
 *     parcLinkedList_Release(&list);
 * }
 * @endcode
 */
bool parcLinkedList_Remove(PARCLinkedList *list, const PARCObject *element);

/**
 * Removes the element at the specified position in this list.
 *
 * Shifts all subsequent elements to the left (subtracts one from their indices).
 * Return the element that was removed from the list without modifying the reference count.
 * The caller must eventually release the returned value.
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList` from which the element will be removed.
 * @param [in] index The index (origin 0) of the element to remove.
 *
 * @return The element that was removed from the list.
 *
 * Example:
 * @code
 * {
 *     PARCLinkedList *list = parcLinkedList_Create();
 *     // add elements to the list.
 *
 *     parcLinkedList_RemoveAtIndex(list, 2); // remove the 3rd element in the list.
 *
 *     // ...
 *
 *     parcLinkedList_Release(&list);
 * }
 * @endcode
 */
PARCObject *parcLinkedList_RemoveAtIndex(PARCLinkedList *list, size_t index);

/**
 * Return the first element of the specified `PARCLinkedList` but do NOT remove it from the queue
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList` from which the first element will be returned
 *
 * @return  non NULL A pointer to the first element
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
PARCObject *parcLinkedList_GetFirst(const PARCLinkedList *list);

/**
 * Return the last element of the specified `PARCLinkedList` but do NOT remove it from the queue
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList` from which the last element will be returned
 *
 * @return  non NULL A pointer to the last element
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
PARCObject *parcLinkedList_GetLast(const PARCLinkedList *list);

/**
 * Return the size of the specified queue
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList`
 *
 * @return `size_t` The size of the queue
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
size_t parcLinkedList_Size(const PARCLinkedList *list);

/**
 * Return True if the `PARCLinkedList` is empty or False if not.
 *
 * @param [in] list A pointer to the instance of `PARCLinkedList`
 *
 * @return bool True if the `PARCLinkedList` is empty or False if not.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
bool parcLinkedList_IsEmpty(const PARCLinkedList *list);

/**
 * Get a pointer to the specified element.
 *
 * @param [in] list A pointer to a `PARCLinkedList` instance.
 * @param [in] index The index of the element to be retrieved.
 *
 * @throws `trapOutOfBounds`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCObject *parcLinkedList_GetAtIndex(const PARCLinkedList *list, size_t index);

/**
 * Replace the element at the specified position in this list with the given element.
 *
 * @param [in] list A pointer to a `PARCLinkedList` instance.
 * @param [in] index The index of the element to be replaced.
 * @param [in] element A pointer to a valid PARCObject instance that will replace the current element at @p index.
 *
 * @throws `trapOutOfBounds`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCObject *parcLinkedList_SetAtIndex(PARCLinkedList *list, size_t index, PARCObject *element);

/**
 * Determine if two `PARCLinkedList` instances are equal.
 *
 * This function implements the following equivalence relations on non-null `PARCLinkedList` instances:
 *
 *   * It is reflexive: for any non-null reference value x, `parcLinkedList_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcLinkedList_Equals(x, y)` must return true if and only if
 *        `parcLinkedList_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcLinkedList_Equals(x, y)` returns true and
 *        `parcLinkedList_Equals(y, z)` returns true,
 *        then `parcLinkedList_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcLinkedList_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcLinkedList_Equals(x, NULL)` must return false.
 *
 * Two `PARCLinkedList` instances with different element equality functions are always unequal.
 *
 * @param [in] x A pointer to a `PARCLinkedList` instance.
 * @param [in] y A pointer to a `PARCLinkedList` instance.
 *
 * @return true `PARCLinkedList` x and y are equal.
 * @return false `PARCLinkedList` x and y are not equal.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcLinkedList_Equals(const PARCLinkedList *x, const PARCLinkedList *y);

/**
 * Print a human readable representation of the given `PARCLinkedList`.
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] list A pointer to the instance to display.
 *
 * Example:
 * @code
 * {
 *     PARCLinkedList *instance = parcLinkedList_Create();
 *
 *     parcLinkedList_Display(instance, 0);
 *
 *     parcLinkedList_Release(&instance);
 * }
 * @endcode
 *
 */
void parcLinkedList_Display(const PARCLinkedList *list, int indentation);

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
parcObject_ImplementNotify(parcLinkedList, PARCLinkedList);

/**
 * Wakes up all threads that are waiting on the given object's lock.
 *
 * A thread waits on an object by calling one of the wait methods, `parcLinkedList_Wait`, `parcLinkedList_WaitFor`, `parcLinkedList_WaitUntil`.
 * The awakened threads will proceed after the current thread relinquishes the lock on the given object.
 * The awakened threads will compete in the usual manner with any other threads that might be actively competing
 * to synchronize on this object.
 * Awakened threads have no priority between them in being the next thread to lock this object.
 *
 * This method can only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid `PARCLinkedList` instance.
 *
 * Example:
 * @code
 * {
 *     if (parcLinkedList_Lock(object)) {
 *         parcLinkedList_NotifyAll(object);
 *         parcLinkedList_Unlock(object);
 *     }
 * }
 * @endcode
 */
parcObject_ImplementNotifyAll(parcLinkedList, PARCLinkedList);

/**
 * Causes the calling thread to wait until either another thread invokes the parcLinkedList_Notify() function on the same object.
 *
 * @param [in] object A pointer to a valid `PARCLinkedList` instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcLinkedList_Wait(object);
 * }
 * @endcode
 */
parcObject_ImplementWait(parcLinkedList, PARCLinkedList);

parcObject_ImplementWaitFor(parcLinkedList, PARCLinkedList);

parcObject_ImplementWaitUntil(parcLinkedList, PARCLinkedList);

/**
 * Obtain the lock on the given `PARCLinkedList` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] object A pointer to a valid `PARCLinkedList` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, or the `PARCLinkedList` is invalid.
 *
 * Example:
 * @code
 * {
 *     if (parcLinkedList_Lock(object)) {
 *
 *     }
 * }
 * @endcode
 */
parcObject_ImplementLock(parcLinkedList, PARCLinkedList);

/**
 * Try to obtain the advisory lock on the given PARCLinkedList instance.
 *
 * Once the lock is obtained, the caller must release the lock as soon as possible.
 *
 * @param [in] object A pointer to a valid PARCLinkedList instance.
 *
 * @return true The PARCLinkedList is locked.
 * @return false The PARCLinkedList is unlocked.
 *
 * Example:
 * @code
 * {
 *     parcLinkedList_TryLock(object);
 * }
 * @endcode
 */
parcObject_ImplementTryLock(parcLinkedList, PARCLinkedList);

/**
 * Try to unlock the advisory lock on the given `PARCLinkedList` instance.
 *
 * @param [in] object A pointer to a valid `PARCLinkedList` instance.
 *
 * @return true The `PARCLinkedList` was locked and now is unlocked.
 * @return false The `PARCLinkedList` was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     parcLinkedList_Unlock(object);
 * }
 * @endcode
 */
parcObject_ImplementUnlock(parcLinkedList, PARCLinkedList);

/**
 * Determine if the advisory lock on the given `PARCLinkedList` instance is locked.
 *
 * @param [in] object A pointer to a valid `PARCLinkedList` instance.
 *
 * @return true The `PARCLinkedList` is locked.
 * @return false The `PARCLinkedList` is unlocked.
 * Example:
 * @code
 * {
 *     if (parcLinkedList_IsLocked(object)) {
 *         ...
 *     }
 * }
 * @endcode
 */
parcObject_ImplementIsLocked(parcLinkedList, PARCLinkedList);

/**
 * Determine if two `PARCLinkedList` instances are equivalent sets.
 *
 * The lists are examined without regard to order.
 * If both lists, x and y are of equal length, and all of the elements in list `x` are present in list `y`, this function returns true.
 *
 * @param [in] x A pointer to a valid `PARCLinkedList` instance.
 * @param [in] y A pointer to a valid `PARCLinkedList` instance.
 * @return true The instances are equivalent.
 * @return false The instances are equivalent.
 *
 */
bool parcLinkedList_SetEquals(const PARCLinkedList *x, const PARCLinkedList *y);

/**
 * Insert the given element into the list such that it is the index'th element in the list.
 */
PARCLinkedList *parcLinkedList_InsertAtIndex(PARCLinkedList *list, size_t index, const PARCObject *element);

/**
 * Apply a function to every element in the given PARCLinkedList.
 *
 * The function is applied in order, any return value is ignored.
 *
 * @param [in] list A pointer to a valid PARCLinkedList instance.
 * @param [in] function A pointer to a function that will be called with each element of the list.
 * @param [in] parameter A pointer to arbitrary data that will supplied as an additional parameter to @p function
 *
 */
#define parcLinkedList_Apply(_list_, _function_, _parameter_) \
    parcLinkedList_ApplyImpl(_list_, (void (*))_function_, (const void *) _parameter_)
void parcLinkedList_ApplyImpl(PARCLinkedList *list, void (*function)(PARCObject *, const void *), const void *parameter);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
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
PARCList *parcLinkedList_AsPARCList(PARCLinkedList *list);
#endif // libparc_parc_Deque_h
