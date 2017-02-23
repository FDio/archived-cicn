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
 * @file parc_PriorityQueue.h
 * @ingroup datastructures
 * @brief A priority queue (heap), where the top item is the minimum by the sort function.
 *
 * The user provides a sort function and the top item will be the minimum
 * as per the < relation.
 *
 */

#ifndef libparc_parc_PriorityQueue_h
#define libparc_parc_PriorityQueue_h

#include <stdlib.h>
#include <stdbool.h>

struct parc_priority_queue;
typedef struct parc_priority_queue PARCPriorityQueue;

typedef int (PARCPriorityQueueCompareTo)(const void *a, const void *b);
typedef void (PARCPriorityQueueDestroyer)(void **elementPtr);

/**
 * Calls {@link parcMemory_Deallocate} to free the element
 *
 * A simple destroyer that only uses {@link parcMemory_Deallocate}.
 *
 * @param [in,out] elementPtr Double pointer to data item, will be NULL'd
 *
 * Example:
 * @code
 *
 * PARCPriorityQueue *q = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, parcPriorityQueue_ParcFreeDestroyer);
 * uint64_t *objectid = parcMemory_Allocate(sizeof(uint64_t));
 * objectid = 100;
 *
 * // this will use parcPriorityQueue_Uint64CompareTo sort order
 * parcPriorityQueue_Add(q, objectid);
 *
 * // this will use the ParcFreeDestroyer
 * parcPriorityQueue_Destroy(&q);
 * @endcode
 */
void parcPriorityQueue_ParcFreeDestroyer(void **elementPtr);

/**
 * Treats the parameters as `uint64_t` pointers and compares them via natural sort order.
 *
 * Treats the parameters as `uint64_t` pointers and compares them via natural sort order.
 * Obeys standared CompareTo semantics.
 *
 * @param [in] a uint64_t pointer
 * @param [in] b uint64_t pointer
 *
 * @return -1 if a < b
 * @return  0 if a == b
 * @return +1 if a > b
 *
 * Example:
 * @code
 *
 * PARCPriorityQueue *q = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, parcPriorityQueue_ParcFreeDestroyer);
 * uint64_t *objectid = parcMemory_Allocate(sizeof(uint64_t));
 * objectid = 100;
 *
 * // this will use parcPriorityQueue_Uint64CompareTo sort order
 * parcPriorityQueue_Add(q, objectid);
 *
 * // this will use the ParcFreeDestroyer
 * parcPriorityQueue_Destroy(&q);
 * @endcode
 */
int parcPriorityQueue_Uint64CompareTo(const void *a, const void *b);


/**
 * Creates a priority queue with a given sort function.
 *
 * The sort function defines the ordering of the Priorty Queue.  The minimum element
 * will always be the head of the queue.
 *
 * The destroyer is called on data elements from {@link parcPriorityQueue_Clear()} and
 * {@link parcPriorityQueue_Destroy()}.  You may use {@linkparcPriorityQueue_ParcFreeDestroyer()} for
 * elements that can be freed by only calling {@link parcMemory_Deallocate}.
 *
 * @param [in] compare Defines the sort order of the priority queue
 * @param [in] destroyer Called for Clear and Destroy operations, may be NULL.
 *
 * @return non-null A pointer to a `PARCPriorityQueue`
 *
 * Example:
 * @code
 * PARCPriorityQueue *q = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, parcPriorityQueue_ParcFreeDestroyer);
 * uint64_t *objectid = parcMemory_Allocate(sizeof(uint64_t));
 * objectid = 100;
 *
 * // this will use parcPriorityQueue_Uint64CompareTo sort order
 * parcPriorityQueue_Add(q, objectid);
 *
 * // this will use the ParcFreeDestroyer
 * parcPriorityQueue_Destroy(&q);
 * @endcode
 */
PARCPriorityQueue *parcPriorityQueue_Create(PARCPriorityQueueCompareTo *compare, PARCPriorityQueueDestroyer *destroyer);


/**
 * Destroy the queue and free remaining elements.
 *
 * Destroys the queue.  If the destroyer was set in Create, then it will be called
 * on all the remaining elements.
 *
 * @param [in,out] queuePtr Double pointer to allocated queue.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcPriorityQueue_Destroy(PARCPriorityQueue **queuePtr);

/**
 * Add an element to the priority queue, returning true if changed
 *
 * A "duplicate" is a data item that compares as equal to another item.  The priority
 * queue supports duplicates.  It is not stable in regard to the ordering of duplicates.
 * Because it supports duplicates, Add will always return true.
 *
 * The priority queue is unbounded.
 *
 * @param [in,out] queue The queue to modify
 * @param [in] data The data to add to the queue, which must be comparable and not NULL
 *
 * @return true The data structure was modified by adding the new value
 * @return false The data structure was not modified
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcPriorityQueue_Add(PARCPriorityQueue *queue, void *data);

/**
 * Removes all elements, calling the data structure's destroyer on each
 *
 * Remvoes all elements.  If the data structure's destroyer is non-NULL, it will be called
 * on each element.
 *
 * @param [in,out] queue The queue to modify
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcPriorityQueue_Clear(PARCPriorityQueue *queue);

/**
 * Returns the head element, but does not remove it.
 *
 * Returns the head element.  The data structure is not modified.  If the
 * priority queue is empty, will return NULL.
 *
 * @param [in] queue The `PARCPriorityQueue` to query.
 *
 * @return non-null The head element
 * @return null The queue is empty
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcPriorityQueue_Peek(PARCPriorityQueue *queue);

/**
 * Removes the head element from the queue and returns it.
 *
 * Removes the head element from the queue and returns it.  If the queue is empty,
 * it returns NULL.
 *
 * @param [in,out] queue The queue to query and modify.
 *
 * @return non-null The head element
 * @return null The queue is empty
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcPriorityQueue_Poll(PARCPriorityQueue *queue);

/**
 * Returns the number of elements in the queue.
 *
 * @param [in] queue The `PARCPriorityQueue` to query.
 *
 * @return number The number of elements in the queue.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcPriorityQueue_Size(const PARCPriorityQueue *queue);
#endif // libparc_parc_PriorityQueue_h
