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
 * @file parc_SortedList.h
 * @ingroup datastructures
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_SortedList
#define PARCLibrary_parc_SortedList
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_Iterator.h>

struct PARCSortedList;
typedef struct PARCSortedList PARCSortedList;

typedef int (*PARCSortedListEntryCompareFunction)(const PARCObject *objA, const PARCObject *objB);

/**
 * Increase the number of references to a `PARCSortedList` instance.
 *
 * Note that new `PARCSortedList` is not created,
 * only that the given `PARCSortedList` reference count is incremented.
 * Discard the reference by invoking `parcSortedList_Release`.
 *
 * @param [in] instance A pointer to a valid PARCSortedList instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     PARCSortedList *b = parcSortedList_Acquire();
 *
 *     parcSortedList_Release(&a);
 *     parcSortedList_Release(&b);
 * }
 * @endcode
 */
PARCSortedList *parcSortedList_Acquire(const PARCSortedList *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcSortedList_OptionalAssertValid(_instance_)
#else
#  define parcSortedList_OptionalAssertValid(_instance_) parcSortedList_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCSortedList` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCSortedList instance.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     parcSortedList_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcSortedList_Release(&b);
 * }
 * @endcode
 */
void parcSortedList_AssertValid(const PARCSortedList *instance);

/**
 * Create an instance of PARCSortedList
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCSortedList instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     parcSortedList_Release(&a);
 * }
 * @endcode
 */
PARCSortedList *parcSortedList_Create(void);

/**
 * Create an instance of PARCSortedList and provide a comparison function.
 *
 * @param [in] compare A pointer to a function that implements the Compare contract.
 *
 * @return non-NULL A pointer to a valid PARCSortedList instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     parcSortedList_Release(&a);
 * }
 * @endcode
 */
PARCSortedList *parcSortedList_CreateCompare(PARCSortedListEntryCompareFunction compare);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCSortedList instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCSortedList` instance.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     PARCSortedList *copy = parcSortedList_Copy(&b);
 *
 *     parcSortedList_Release(&b);
 *     parcSortedList_Release(&copy);
 * }
 * @endcode
 */
PARCSortedList *parcSortedList_Copy(const PARCSortedList *original);

/**
 * Print a human readable representation of the given `PARCSortedList`.
 *
 * @param [in] instance A pointer to a valid PARCSortedList instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     parcSortedList_Display(a, 0);
 *
 *     parcSortedList_Release(&a);
 * }
 * @endcode
 */
void parcSortedList_Display(const PARCSortedList *instance, int indentation);

/**
 * Determine if two `PARCSortedList` instances are equal.
 *
 * The following equivalence relations on non-null `PARCSortedList` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcSortedList_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcSortedList_Equals(x, y)` must return true if and only if
 *        `parcSortedList_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcSortedList_Equals(x, y)` returns true and
 *        `parcSortedList_Equals(y, z)` returns true,
 *        then `parcSortedList_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcSortedList_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcSortedList_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCSortedList instance.
 * @param [in] y A pointer to a valid PARCSortedList instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *     PARCSortedList *b = parcSortedList_Create();
 *
 *     if (parcSortedList_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcSortedList_Release(&a);
 *     parcSortedList_Release(&b);
 * }
 * @endcode
 * @see parcSortedList_HashCode
 */
bool parcSortedList_Equals(const PARCSortedList *x, const PARCSortedList *y);

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
 * If two instances are equal according to the {@link parcSortedList_Equals} method,
 * then calling the {@link parcSortedList_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcSortedList_Equals} function,
 * then calling the `parcSortedList_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCSortedList instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     PARCHashCode hashValue = parcSortedList_HashCode(buffer);
 *     parcSortedList_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcSortedList_HashCode(const PARCSortedList *instance);

/**
 * Determine if an instance of `PARCSortedList` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCSortedList instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     if (parcSortedList_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcSortedList_Release(&a);
 * }
 * @endcode
 *
 */
bool parcSortedList_IsValid(const PARCSortedList *instance);

/**
 * Release a previously acquired reference to the given `PARCSortedList` instance,
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
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     parcSortedList_Release(&a);
 * }
 * @endcode
 */
void parcSortedList_Release(PARCSortedList **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCSortedList instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     PARCJSON *json = parcSortedList_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcSortedList_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcSortedList_ToJSON(const PARCSortedList *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCSortedList`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCSortedList instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCSortedList *a = parcSortedList_Create();
 *
 *     char *string = parcSortedList_ToString(a);
 *
 *     parcSortedList_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcSortedList_Display
 */
char *parcSortedList_ToString(const PARCSortedList *instance);

/**
 * Wakes up a single thread that is waiting on this object (see `parcSortedList_Wait)`.
 * If any threads are waiting on this object, one of them is chosen to be awakened.
 * The choice is arbitrary and occurs at the discretion of the underlying implementation.
 *
 * The awakened thread will not be able to proceed until the current thread relinquishes the lock on this object.
 * The awakened thread will compete in the usual manner with any other threads that might be actively
 * competing to synchronize on this object;
 * for example, the awakened thread enjoys no reliable privilege or disadvantage in being the next thread to lock this object.
 *
 * @param [in] object A pointer to a valid PARCSortedList instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcSortedList_Notify(object);
 * }
 * @endcode
 */
parcObject_ImplementNotify(parcSortedList, PARCSortedList);

/**
 * Wakes up all threads that are waiting on the given object's lock.
 *
 * A thread waits on an object by calling one of the wait methods, `parcSortedList_Wait`, `parcSortedList_WaitFor`, `parcSortedList_WaitUntil`.
 * The awakened threads will proceed after the current thread relinquishes the lock on the given object.
 * The awakened threads will compete in the usual manner with any other threads that might be actively competing
 * to synchronize on this object.
 * Awakened threads have no priority between them in being the next thread to lock this object.
 *
 * This method can only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid `PARCSortedList` instance.
 *
 * Example:
 * @code
 * {
 *     if (parcSortedList_Lock(object)) {
 *         parcSortedList_NotifyAll(object);
 *         parcSortedList_Unlock(object);
 *     }
 * }
 * @endcode
 */
parcObject_ImplementNotifyAll(parcSortedList, PARCSortedList);

/**
 * Causes the calling thread to wait until either another thread invokes the parcSortedList_Notify() function on the same object.
 *  *
 * @param [in] object A pointer to a valid `PARCSortedList` instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcSortedList_Wait(object);
 * }
 * @endcode
 */
parcObject_ImplementWait(parcSortedList, PARCSortedList);

parcObject_ImplementWaitFor(parcSortedList, PARCSortedList);

/**
 * Causes the calling thread to wait until either another thread invokes the `parcSortedList_Notify()`
 * function on the same object or the system time equals or exceeds the specified time.
 *
 * The calling thread must own the object's lock.
 * The calling thread will release ownership of this lock and wait until another thread invokes
 * `parcSortedList_Notify` or the computer's system time equals or exceeds that specified by @p time.
 * on the same object.
 * The original calling thread then re-obtains ownership of the lock and resumes execution.
 *
 * This function must only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid PARCSortedList instance.
 *
 * @returns false if the alloted time was exceeded.
 * @returns true if another thread invoked the `parcSortedList_Notify()` or `parcSortedList_NotifyAll()` function
 *
 * Example:
 * @code
 * {
 *     struct timeval tv;
 *     gettimeofday(&tv, NULL);
 *
 *     struct timespec absoluteTime;
 *     absoluteTime.tv_sec = tv.tv_sec + 0;
 *     absoluteTime.tv_nsec = 0;
 *
 *     parcSortedList_WaitUntil(object, &absoluteTime);
 * }
 * @endcode
 */
parcObject_ImplementWaitUntil(parcSortedList, PARCSortedList);

/**
 * Obtain the lock on the given `PARCSortedList` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] object A pointer to a valid `PARCSortedList` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, or the `PARCSortedList` is invalid.
 *
 * Example:
 * @code
 * {
 *     if (parcSortedList_Lock(object)) {
 *
 *     }
 * }
 * @endcode
 */
parcObject_ImplementLock(parcSortedList, PARCSortedList);

/**
 * Try to obtain the advisory lock on the given PARCSortedList instance.
 *
 * Once the lock is obtained, the caller must release the lock as soon as possible.
 *
 * @param [in] object A pointer to a valid PARCSortedList instance.
 *
 * @return true The PARCSortedList is locked.
 * @return false The PARCSortedList is unlocked.
 *
 * Example:
 * @code
 * {
 *     parcSortedList_TryLock(object);
 * }
 * @endcode
 */
parcObject_ImplementTryLock(parcSortedList, PARCSortedList);

/**
 * Try to unlock the advisory lock on the given `PARCSortedList` instance.
 *
 * @param [in] object A pointer to a valid `PARCSortedList` instance.
 *
 * @return true The `PARCSortedList` was locked and now is unlocked.
 * @return false The `PARCSortedList` was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     parcSortedList_Unlock(object);
 * }
 * @endcode
 */
parcObject_ImplementUnlock(parcSortedList, PARCSortedList);

/**
 * Determine if the advisory lock on the given `PARCSortedList` instance is locked.
 *
 * @param [in] object A pointer to a valid `PARCSortedList` instance.
 *
 * @return true The `PARCSortedList` is locked.
 * @return false The `PARCSortedList` is unlocked.
 * Example:
 * @code
 * {
 *     if (parcSortedList_IsLocked(object)) {
 *         ...
 *     }
 * }
 * @endcode
 */
parcObject_ImplementIsLocked(parcSortedList, PARCSortedList);

PARCIterator *parcSortedList_CreateIterator(PARCSortedList *instance);

void parcSortedList_Add(PARCSortedList *instance, PARCObject *element);

size_t parcSortedList_Size(const PARCSortedList *list);

PARCObject *parcSortedList_GetAtIndex(const PARCSortedList *list, const size_t index);

/**
 * Return the first element of the specified list.
 * The element's reference count is not modified,
 * the caller must acquire its own reference to the element if it is needed beyond lifetime of the given list.
 *
 * @param [in] list A pointer to the instance of `PARCSortedList` from which the first element will be returned.
 *
 * @return  non NULL A pointer to the element
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCObject *parcSortedList_GetFirst(const PARCSortedList *list);

/**
 * Return the last element of the specified list.
 * The element's reference count is not modified,
 * the caller must acquire its own reference to the element if it is needed beyond lifetime of the given list.
 *
 * @param [in] list A pointer to the instance of `PARCSortedList` from which the last element will be returned.
 *
 * @return  non NULL A pointer to the element
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCObject *parcSortedList_GetLast(const PARCSortedList *list);


bool parcSortedList_Remove(PARCSortedList *list, const PARCObject *object);
/**
 * Return the first element of the specified list and remove it from the list.
 * The element's reference count is not modified,
 * the caller must release the returned element when it is finished with it.
 *
 * @param [in] list A pointer to the instance of `PARCSortedList` from which the first element will be returned and removed
 *
 * @return  non NULL A pointer to the element removed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCObject *parcSortedList_RemoveFirst(PARCSortedList *list);

/**
 * Return the last element of the specified list and remove it from the list.
 * The element's reference count is not modified,
 * the caller must release the returned element when it is finished with it.
 *
 * @param [in] list A pointer to the instance of `PARCSortedList` from which the last element will be removed and returned.
 *
 * @return non-NULL A pointer to the element removed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCObject *parcSortedList_RemoveLast(PARCSortedList *list);

#endif
