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
 * @file parc_List.h
 * @ingroup datastructures
 * @brief PARC (Generic) List
 *
 * An ordered collection (also known as a sequence).
 * The user of this interface has precise control over where in the list each element is inserted.
 * The user can access elements by their integer index (position in the list), and search for elements in the list.
 * Unlike sets, lists typically allow duplicate elements.
 * More formally, lists typically allow pairs of elements e1 and e2 such that e1.equals(e2), and they typically allow
 * multiple null elements if they allow null elements at all.
 * It is not inconceivable that someone might wish to implement a list that prohibits duplicates,
 * by throwing runtime exceptions when the user attempts to insert them, but we expect this usage to be rare.
 *
 */
#ifndef libparc_parc_List_h
#define libparc_parc_List_h

#include <stdbool.h>

struct parc_list;
/**
 * @typedef PARCList
 * @brief  An ordered collection (also known as a sequence).
 */
typedef struct parc_list PARCList;

#include <parc/algol/parc_HashCode.h>

#include <parc/algol/parc_Collection.h>
#include <parc/algol/parc_Object.h>

/**
 * @typedef PARCListInterface
 * @brief The interface of a `PARCList` including functions such as copy, destroy, add, etc.
 */
typedef struct parc_list_interface {
    /**
     * Copy an instance of `PARCList`
     *
     * @param [in] original An instance of `PARCList` to copy
     *
     * @return  A pointer to the new list.
     */
    void *(*Copy)(const PARCList * original);

    /**
     * Destroy the List
     *
     * @param [in,out] instancePtr
     * @return a pointer to the destroyed List.
     */
    void (*Destroy)(void **instancePtr);

    /**
     * Tests if this list is empty.
     *
     * @param [in] instance
     * @return true if the list is empty
     */
    bool (*IsEmpty)(const void *instance);

    /**
     * Appends the specified element to the end of this list (optional operation).
     *
     * @param [in,out] The instance of `PARCList` to append the element to
     * @param [in] element The pointer to the element to be added to the `PARCList`
     * @return true if the element was added successfully.
     */
    bool (*Add)(void *instance, PARCObject *element);

    /**
     * Inserts the specified element at the specified position in this list (optional operation).
     *
     * @param [in,out] instance The instance of `PARCList` to modify
     * @param [in] index The index in `PARCList` at which to insert the @p element
     * @param [in] element The element to insert in `PARCList` at @p index.
     */
    void (*AddAtIndex)(void *instance, int index, PARCObject *element);

    /**
     * Append elements of @p collection to @p instance
     *
     *   Appends all of the elements in the specified collection to the end of this list,
     *    in the order that they are returned by the specified collection's iterator (optional operation).
     *
     * @param [in,out] instance The `PARCList` to be modified
     * @param [in] collection The collection to be added
     * @return true if add is successful.
     */
    bool (*AddCollection)(void *instance, PARCCollection *collection);

    /**
     * Inserts all of the elements in the specified collection into this list at the specified position (optional operation)
     *
     * @param [in,out] instance The `PARCList` to be modified
     * @param [in] index The position at which to insert the @p collection
     * @param [in] collection The collection to be added
     * @return true if add is successful.
     * @endcode
     */
    bool (*AddCollectionAtIndex)(void *instance, int index, PARCCollection *collection);

    /**
     * Removes all of the elements from this list (optional operation).
     *
     * @param [in,out] instance The instance of `PARCList` to empty.
     */
    void (*Clear)(void *instance);

    /**
     * Returns true if this list contains the specified element.
     *
     * @param [in] instance The instance of `PARCList` to inspect
     * @param [in] element The element to search for in @p instance
     * @return true if the @p element is found in @p instance.
     */
    bool (*Contains)(const void *instance, const PARCObject *element);

    /**
     * Returns true if this list contains all of the elements of the specified collection.
     *
     * @param [in] instance The instance of `PARCList` to inspect
     * @param [in] collection The instance of {@link PARCCollection} whose elements are sought in @p instance
     * @return true if all of the elements in @p collection is found in @p instance
     */
    bool (*ContainsCollection)(const void *instance, const PARCCollection *collection);

    /**
     * Compares the specified object with this list for equality.
     *
     * @param [in] xInstance The first `PARCList` instance to compare
     * @param [in] yInstance The second `PARCList` instance to compare
     * @return true if the two instances are equal
     */
    bool (*Equals)(const void *xInstance, const void *yInstance);

    /**
     * Returns the element at the specified position in this list.
     *
     * @param [in] instance A pointer to the instance of `PARCList`
     * @param index The index of the element to be returned
     * @return A pointer to the element at @p index
     */
    PARCObject *(*GetAtIndex)(const void *instance, size_t index);

    /**
     * Returns the hash code value for this list.
     *
     * @param [in] instance A pointer to the instance of `PARCList`
     * @return int The hash code value
     */
    PARCHashCode (*HashCode)(const void *instance);

    /**
     * Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not contain the element.
     *
     * @param [in] instance A pointer to the instance of `PARCList`
     * @param [in] element A pointer to the element to locate in @p instance
     * @return size_t the index of the first located @p element or -1 if not found
     */
    size_t (*IndexOf)(const void *instance, const PARCObject *element);

    /**
     * Returns the index of the last occurrence of the specified element in this list, or -1 if this list does not contain the element.
     *
     * @param [in] instance A pointer to the instance of `PARCList`
     * @param [in] element A pointer to the element to locate in @p instance
     * @return size_t the index of the last located @p element or -1 if not found
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    size_t (*LastIndexOf)(void *instance, const PARCObject *element);

    /**
     * Removes the element at the specified position in this list (optional operation).
     *
     * @param [in,out] instance A pointer to the instance of `PARCList` to modify
     * @param [in] index The index of the element to remove
     * @return A pointer to the removed element
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    PARCObject *(*RemoveAtIndex)(PARCList * list, size_t index);

    /**
     * Removes the first occurrence of the specified element from this list, if it is present (optional operation).
     *
     * @param [in,out] instance A pointer to the instance of `PARCList` to modify
     * @param element The element to find and remove
     * @return true if element found and removed
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    bool (*Remove)(void *instance, const PARCObject *element);

    /**
     * Removes from this list all of its elements that are contained in the specified collection (optional operation).
     *
     * @param [in,out] instance A pointer to the instance of `PARCList` to modify
     * @param collection The instance of {@link PARCCollection} whose elements should be found in the @p instance and removed.
     * @return true if the elements are found and removed
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    bool (*RemoveCollection)(void *instance, const PARCCollection *collection);

    /**
     * Retains only the elements in this list that are contained in the specified collection (optional operation).
     *
     * @param [in,out] instance A pointer to the instance of `PARCList` to modify
     * @param collection The instance of {@link PARCCollection} whose elements should be retained in
     * the @p instance while all other elements are removed.
     * @return true if the operation is successful
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    bool (*RetainCollection)(void *instance, const PARCCollection *collection);

    /**
     * Replaces the element at the specified position in this list with the specified element (optional operation).
     *
     * @param [in,out] instance A pointer to the instance of `PARCList` to modify
     * @param index The position in @p instance to replace with @p element
     * @param element The element to put into @p instance at @p index, replacing the current value.
     * @return
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    void *(*SetAtIndex)(void *instance, size_t index, PARCObject * element);

    /**
     * Returns the number of elements in this list.
     *
     * @param [in] instance A pointer to the instance of `PARCList` to inspect
     * @return size_t Number of elements in the list
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    size_t (*Size)(const void *instance);

    /**
     * Returns a view of the portion of this list between the specified fromIndex, inclusive, and toIndex, exclusive.
     *
     * @param [in] instance A pointer to the instance of `PARCList` to inspect
     * @param [in] fromIndex The starting index into the list
     * @param [in] toIndex The end index into the list
     * @return A pointer to the sub list
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    PARCList *(*SubList)(const void *instance, size_t fromIndex, size_t toIndex);

    /**
     * Returns an array containing all of the elements in this list in proper sequence (from first to last element).
     *
     * @param [in] instance A pointer to the instance of `PARCList` to inspect
     * @return A pointer to a pointer to the array containing the elements of the list in proper sequence.
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    void** (*ToArray)(const void *instance);
} PARCListInterface;

/**
 * Increase the number of references to a `PARCList`.
 *
 * Note that new `PARCList` is not created,
 * only that the given `PARCList` reference count is incremented.
 * Discard the reference by invoking `parcList_Release`.
 *
 * @param list A pointer to the original `PARCList`.
 * @return The value of the input parameter @p list.
 *
 * Example:
 * @code
 * {
 *     PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);
 *
 *     PARCList *list2 = parcList_Acquire(list);
 *
 *     parcList_Release(&list);
 *     parcList_Release(&list2);
 * }
 * @endcode
 *
 * @see parcList_Release
 */
PARCList *parcList_Acquire(const PARCList *list);

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
 * @param [in,out] listPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);
 *
 *     parcList_Release(&list);
 * }
 * @endcode
 */
void parcList_Release(PARCList **listPtr);

/**
 * Create an independent copy the given `PARCList`
 *
 * A new list is created as a complete copy of the original.
 *
 * @param [in] list A valid pointer to a `PARCList` instance (cannot be NULL)
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCList` instance.
 *
 * @throws parcTrapIllegalValue if @p instance is NULL.
 *
 * Example:
 * @code
 * {
 *     PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);
 *
 *     PARCList *copy = parcList_Copy(list);
 *
 *     parcList_Release(&copy);
 *     parcList_Release(&list);
 * }
 * @endcode
 *
 */
PARCList *parcList_Copy(const PARCList *list);

/**
 * Tests if this list is empty.
 *
 *   Return true if the list is empty, false otherwise
 *
 * @param list A pointer to the instance of `PARCList` to test
 * @return True if the list is empty, else False
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_IsEmpty(const PARCList *list);

/**
 * Appends the specified element to the end of this list (optional operation).
 *
 * @param [in,out] list A pointer to the instance of `PARCList` to modify
 * @param [in] element The element to add to the end of the `PARCList`
 * @return True if the add is successful
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_Add(PARCList *list, void *element);

/**
 * Add all of the pointers in the given array of pointers to the `PARCList`.
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified.
 * @param [in] argc The number of values in @p argv.
 * @param [in] argv An array void * values.
 *
 * @return True if the add is successful
 *
 * Example:
 * @code
 * {
 *   PARCList *list = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
 *
 *   int elements[] = { 1, 2, 3 };
 *
 *   parcList_AddAll(array, 3, elements);
 *   size_t actual = parcList_Length(array);
 *
 *   parcAssertTrue(3 == actual, "Expected=%d, actual=%d", 3, actual);
 *
 *   parcListRelease(&array);
 * }
 * @endcode
 */
bool parcList_AddAll(PARCList *list, size_t argc, void **argv);

/**
 * Inserts the specified element at the specified position in this list (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified
 * @param [in] index The specified position in the list
 * @param [in] element The element to be added to the specified position.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcList_AddAtIndex(PARCList *list, int index, void *element);

/**
 *   Appends all of the elements in the specified collection to the end of this list,
 *    in the order that they are returned by the specified collection's iterator (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified
 * @param [in] collection A pointer to an istance of {@link PARCCollection} to be added to the list
 * @return True if add is successful
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_AddCollection(PARCList *list, PARCCollection *collection);

/**
 * Inserts all of the elements in the specified collection into this list at the specified position (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified
 * @param [in] index The position at which to insert the collection
 * @param [in] collection A pointer to an istance of {@link PARCCollection} to be inserted into the list
 * @return True if insertion is successful
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_AddCollectionAtIndex(PARCList *list, int index, PARCCollection *collection);

/**
 * Removes all of the elements from this list (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be cleared
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcList_Clear(PARCList *list);

/**
 * Returns true if this list contains the specified element.
 *
 * @param [in] list A pointer to the `PARCList` instance to be checked
 * @param [in] element The element to be added to the specified position.
 * @return True if the element is contained in the list
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_Contains(const PARCList *list, void *element);

/**
 * Returns true if this list contains all of the elements of the specified collection.
 *
 * @param [in] list A pointer to the `PARCList` instance to be checked
 * @param [in] collection A pointer to the instance of {@link PARCCollection} to be checked.
 * @return True if all of the elements in the collection are found in the list.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_ContainsCollection(PARCList *list, PARCCollection *collection);


/**
 * Determine if two `PARCList` instances are equal.
 *
 * Two `PARCList` instances are equal if, and only if,
 * the size of the lists are equal and each element in the list is equal and
 * in the same order.
 *
 * The following equivalence relations on non-null `PARCList` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `PARCList_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `parcList_Equals(x, y)` must return true if and only if
 *        `parcList_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcList_Equals(x, y)` returns true and
 *        `parcList_Equals(y, z)` returns true,
 *        then  `parcList_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `parcList_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `parcList_Equals(x, NULL)` must
 *      return false.
 *
 * @param [in] x A pointer to a `PARCList` instance.
 * @param [in] y A pointer to a `PARCList` instance.
 * @return true if the two `PARCList` instances are equal.
 *
 * Example:
 * @code
 * {
 *    PARCList *a = parcList_Create();
 *    PARCList *b = parcList_Create();
 *
 *    if (parcList_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool parcList_Equals(const PARCList *x, const PARCList *y);

/**
 * Returns the element at the specified position in this list.
 * If the index is out of bounds, it will trap with an out-of-bounds.
 *
 * @param [in] list A pointer to the `PARCList` instance to be checked
 * @param [in] index The index of the element to be returned
 * @return A pointer to the element.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcList_GetAtIndex(const PARCList *list, size_t index);

/**
 * Returns the hash code value for this list.
 *
 * @param [in] list A pointer to the `PARCList` instance to be hashed
 * @return The hash code value
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int parcList_HashCode(const PARCList *list);

/**
 * Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not contain the element.
 *
 * @param [in] list A pointer to the `PARCList` instance to be hashed.
 * @param [in] element A pointer to an element to check for list inclusion.
 * @return The index of the first occurance of @p element, or -1 if it is not present
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t parcList_IndexOf(const PARCList *list, PARCObject *element);

/**
 * Returns the index of the last occurrence of the specified element in this list, or -1 if this list does not contain the element.
 *
 * @param [in] list A pointer to the `PARCList` instance to be hashed.
 * @param [in] element A pointer to an element to check for list inclusion.
 * @return The index of the last occurance of @p element, or -1 if it is not present
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t parcList_LastIndexOf(const PARCList *list, PARCObject *element);

/**
 * Removes the element at the specified position in this list (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified.
 * @param [in] index  The index of the element to be removed
 * @return  non NULL A pointer to the element removed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcList_RemoveAtIndex(PARCList *list, size_t index);

/**
 * Removes the first occurrence of the specified element from this list, if it is present (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified.
 * @param [in] element A pointer to the element to be removed from the `PARCList`
 * @return true The element was found and removed, false if it was not found.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_Remove(PARCList *list, PARCObject *element);

/**
 * Removes from this list all of its elements that are contained in the specified collection (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified.
 * @param [in] collection A pointer to the instance of {@link PARCCollection} to be removed from the `PARCList`
 * @return true The collection was found and removed, false if it was not found.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_RemoveCollection(PARCList *list, PARCCollection *collection);

/**
 * Retains only the elements in this list that are contained in the specified collection (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified.
 * @param [in] collection A pointer to the instance of {@link PARCCollection} to be found and retained in the `PARCList`
 * @return true if the function was successful
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcList_RetainCollection(PARCList *list, PARCCollection *collection);

/**
 * Replaces the element at the specified position in this list with the specified element (optional operation).
 *
 * @param [in,out] list A pointer to the `PARCList` instance to be modified.
 * @param [in] index The position at which the element should be replaced with @p element
 * @param [in] element A pointer to the element to be inserted at the specified position
 * @return A pointer to the element previously at that position.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCObject *parcList_SetAtIndex(PARCList *list, size_t index, PARCObject *element);

/**
 * Returns the number of elements in this list.
 *
 * @param [in] list A pointer to the `PARCList` instance to be measured.
 * @return The size of the @p list
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcList_Size(const PARCList *list);

/**
 * Returns a view of the portion of this list between the specified fromIndex, inclusive, and toIndex, exclusive.
 *
 * @param [in] list A pointer to the `PARCList` instance to be measured.
 * @param [in] fromIndex The position to start the view (inclusive)
 * @param [in] toIndex The position to end the view (exclusive)
 * @return a pointer to an instance of `PARCList` containing the subList requested
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCList *parcList_SubList(PARCList *list, size_t fromIndex, size_t toIndex);

/**
 * Returns an array containing all of the elements in this list in proper sequence (from first to last element).
 *
 * @param [in] list A pointer to the `PARCList` instance to be sorted.
 * @return A pointer to a pointer to an Array containing the sorted elements
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void**parcList_ToArray(PARCList *list);

/**
 * Create an instance of `PARCList` that uses the @p interface to provide functions and the @p instance to provide
 * initial elements of the list.
 * @param [in] instance  An initial set of elements for the new instance of `PARCList`
 * @param [in] interface A pointer to an instance of {@link PARCListInterface} containing a set of list functions
 * @return A pointer to a new instance of `PARCList`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCList *parcList(void *instance, PARCListInterface *interface);

/**
 * Create an instance of `PARCList` that uses the @p interface to provide functions and the @p instance to provide
 * initial elements of the list.
 * @param [in] instance  An initial set of elements for the new instance of `PARCList`
 * @param [in] interface A pointer to an instance of {@link PARCListInterface} containing a set of list functions
 * @return A pointer to a new instance of `PARCList`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCList *parcList_Create(void *instance, PARCListInterface *interface);

#endif // libparc_parc_List_h
