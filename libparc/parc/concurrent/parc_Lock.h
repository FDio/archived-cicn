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
 * @file parc_Lock.h
 * @ingroup threading
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_Lock
#define PARCLibrary_parc_Lock
#include <stdbool.h>

#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_JSON.h>

struct PARCLock;
typedef struct PARCLock PARCLock;

/**
 * Increase the number of references to a `PARCLock` instance.
 *
 * Note that new `PARCLock` is not created,
 * only that the given `PARCLock` reference count is incremented.
 * Discard the reference by invoking `parcLock_Release`.
 *
 * @param [in] instance A pointer to a valid PARCLock instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCLock *a = parcLock_Create();
 *
 *     PARCLock *b = parcLock_Acquire();
 *
 *     parcLock_Release(&a);
 *     parcLock_Release(&b);
 * }
 * @endcode
 */
PARCLock *parcLock_Acquire(const PARCLock *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcLock_OptionalAssertValid(_instance_)
#else
#  define parcLock_OptionalAssertValid(_instance_) parcLock_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCLock` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCLock instance.
 *
 * Example:
 * @code
 * {
 *     PARCLock *a = parcLock_Create();
 *
 *     parcLock_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcLock_Release(&b);
 * }
 * @endcode
 */
void parcLock_AssertValid(const PARCLock *instance);

/**
 * Create an instance of PARCLock
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCLock instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCLock *a = parcLock_Create();
 *
 *     parcLock_Release(&a);
 * }
 * @endcode
 */
PARCLock *parcLock_Create(void);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCLock instance.
 * @param [in] other A pointer to a valid PARCLock instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCLock *a = parcLock_Create();
 *     PARCLock *b = parcLock_Create();
 *
 *     if (parcLock_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcLock_Release(&a);
 *     parcLock_Release(&b);
 * }
 * @endcode
 *
 * @see parcLock_Equals
 */
int parcLock_Compare(const PARCLock *instance, const PARCLock *other);

/**
 * Print a human readable representation of the given `PARCLock`.
 *
 * @param [in] instance A pointer to a valid PARCLock instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCLock *a = parcLock_Create();
 *
 *     parcLock_Display(a, 0);
 *
 *     parcLock_Release(&a);
 * }
 * @endcode
 */
void parcLock_Display(const PARCLock *instance, int indentation);

/**
 * Determine if an instance of `PARCLock` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCLock instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCLock *a = parcLock_Create();
 *
 *     if (parcLock_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcLock_Release(&a);
 * }
 * @endcode
 *
 */
bool parcLock_IsValid(const PARCLock *instance);

/**
 * Release a previously acquired reference to the given `PARCLock` instance,
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
 *     PARCLock *a = parcLock_Create();
 *
 *     parcLock_Release(&a);
 * }
 * @endcode
 */
void parcLock_Release(PARCLock **instancePtr);

/**
 * Append a representation of the specified `PARCLock` instance to the given `PARCBufferComposer`.
 *
 * @param [in] name A pointer to a `PARCLock` instance whose representation should be appended to the @p composer.
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
 *     parcLock_BuildString(instance, result);
 *
 *     char *string = parcBufferComposer_ToString(result);
 *     printf("Hello: %s\n", string);
 *     parcMemory_Deallocate(string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcLock_BuildString(const PARCLock *name, PARCBufferComposer *composer);

/**
 * Produce a null-terminated string representation of the specified `PARCLock`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCLock instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCLock *a = parcLock_Create();
 *
 *     char *string = parcLock_ToString(a);
 *
 *     parcLock_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcLock_Display
 */
char *parcLock_ToString(const PARCLock *instance);

/**
 * Obtain the lock on the given `PARCLock` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] lock A pointer to a valid `PARCLock` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, or the `PARCLock` is invalid.
 *
 * Example:
 * @code
 * {
 *     if (parcLock_Lock(lock)) {
 *
 *     }
 * }
 * @endcode
 */
bool parcLock_Lock(PARCLock *lock);

/**
 * Try to obtain the advisory lock on the given `PARCObject` instance.
 *
 * Once the lock is obtained, the caller must release the lock via `parcObject_Unlock`.
 *
 * @param [in] lock A pointer to a valid `PARCObject` instance.
 *
 * @return true The `PARCObject` is locked.
 * @return false The `PARCObject` is unlocked.
 *
 * Example:
 * @code
 * {
 *     while (parcLock_TryLock(object))
 *         ;
 * }
 * @endcode
 */
bool parcLock_TryLock(PARCLock *lock);

/**
 * Try to unlock the advisory lock on the given PARCObject instance.
 *
 * @param [in] lock A pointer to a valid PARCObject instance.
 *
 * @return true The PARCObject was locked and now is unlocked.
 * @return false The PARCObject was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     parcLock_Unlock(object);
 * }
 * @endcode
 */
bool parcLock_Unlock(PARCLock *lock);

/**
 * Determine if the advisory lock on the given PARCObject instance is locked.
 *
 * @param [in] lock A pointer to a valid PARCObject instance.
 *
 * @return true The PARCObject is locked.
 * @return false The PARCObject is unlocked.
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcLock_IsLocked(const PARCLock *lock);

/**
 * Causes the calling thread to wait until either another thread invokes the parcObject_Notify() function on the same object.
 *
 * The calling thread must own this object's lock.
 * The calling thread will release ownership of this lock and wait until another thread invokes `parcObject_Notify`
 * on the same object. The original calling thread then re-obtains ownership of the lock and resumes execution.
 *
 * This function must only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] lock A pointer to a valid PARCObject instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcObject_Wait(object);
 * }
 * @endcode
 */
void parcLock_Wait(PARCLock *lock);

/**
 * Wakes up a single thread that is waiting on this object (see `parcObject_Wait)`.
 * If any threads are waiting on this object, one of them is chosen to be awakened.
 * The choice is arbitrary and occurs at the discretion of the underlying implementation.
 *
 * The awakened thread will not be able to proceed until the current thread relinquishes the lock on this object.
 * The awakened thread will compete in the usual manner with any other threads that might be actively
 * competing to synchronize on this object;
 * for example, the awakened thread enjoys no reliable privilege or disadvantage in being the next thread to lock this object.
 *
 * @param [in] lock A pointer to a valid `PARCObject` instance.
 *
 * Example:
 * @code
 * {
 *
 *     parcObject_Notify(object);
 * }
 * @endcode
 */
void parcLock_Notify(PARCLock *lock);
#endif
