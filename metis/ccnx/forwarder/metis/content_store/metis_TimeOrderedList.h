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

#ifndef __Metis__metis_TimeOrderedList__
#define __Metis__metis_TimeOrderedList__

#include <stdio.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/content_store/metis_ContentStoreEntry.h>
#include <parc/algol/parc_TreeRedBlack.h>

struct metis_timeordered_list;
typedef struct metis_timeordered_list MetisTimeOrderedList;

/**
 * A signum function that takes two instances of MetisContentStoreEntrys and
 * returns a value based on their relative values.
 */
typedef PARCTreeRedBlack_KeyCompare MetisTimeOrderList_KeyCompare;

/**
 * Create a new instance of `MetisTimeOrderedList` that will maintain the order of its
 * list items using the supplied `keyCompareFunction`.
 *
 * The newly created `MetisTimeOrderedList` must eventually be released by calling
 * {@link metisTimeOrderedList_Release}.
 *
 * @param keyCompareFunction the signum comparison function to use to sort stored items.
 * @return a new instance of `MetisTimeOrderList`.
 * @return NULL if the new instance couldn't be created.
 *
 * Example:
 * @code
 * {
 *     MetisTimeOrderedList *list =
 *         metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
 *
 *     ...
 *
 *     metisTimeOrderedList_Release(&list);
 * }
 * @endcode
 *
 * @see metisTimeOrderedList_Release
 * @see metisContentStoreEntry_CompareExpiryTime
 * @see metisContentStoreEntry_CompareRecommendedCacheTime
 */
MetisTimeOrderedList *metisTimeOrderedList_Create(MetisTimeOrderList_KeyCompare *keyCompareFunction);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] listP A pointer to a pointer to the instance to release.
 * Example:
 * @code
 * {
 *     MetisTimeOrderedList *list =
 *         metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
 *
 *     ...
 *
 *     metisTimeOrderedList_Release(&list);
 * }
 * @endcode
 */
void metisTimeOrderedList_Release(MetisTimeOrderedList **listP);

/**
 * Add a {@link MetisContentStoreEntry} instance to the specified list. Note that a new refernece to
 * the specified `storeEntry` is not acquired.
 *
 * @param list the list instance into which to add the specified storeEntry.
 * @param storeEntry the storeEntry instance to add.
 *
 * Example:
 * @code
 * {
 *     MetisLogger *logger = metisLogger_Create(...);
 *     MetisTimeOrderedList *list =
 *         metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
 *     MetisLruList *lruList = metisLruList_Create();
 *     MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, 2, logger);
 *
 *     MetisContentStoreEntry *entry = metisContentStoreEntry_Create(message, lruList, 2l, 1l);
 *
 *     metisTimeOrderedList_Add(list, entry);
 *
 *     metisTimeOrderedList_Release(&list);
 *
 *     metisContentStoreEntry_Release(&entry);
 *     metisMessage_Release(&message);
 *     metisLruList_Destroy(&lruList);
 *     metisLogger_Release(&logger);
 * }
 * @endcode
 * @see metisTimeOrderedList_Remove
 */
void metisTimeOrderedList_Add(MetisTimeOrderedList *list, MetisContentStoreEntry *storeEntry);

/**
 * Remove a {@link MetisContentStoreEntry} instance from the specified list.
 *
 * @param list the list instance from which to remove the specified storeEntry.
 * @param storeEntry the storeEntry instance to remove.
 * @return true if the removal was succesful.
 * @return false if the removal was not succesful.
 *
 * Example:
 * @code
 * {
 *     MetisLogger *logger = metisLogger_Create(...);
 *     MetisTimeOrderedList *list =
 *         metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
 *     MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, 2, logger);
 *
 *     MetisContentStoreEntry *entry = metisContentStoreEntry_Create(message, NULL, 2l, 1l);
 *
 *     metisTimeOrderedList_Add(list, entry);
 *
 *        ...
 *
 *     metisTimeOrderedList_Remove(list, entry);
 *     metisTimeOrderedList_Release(&list);
 *
 *     metisContentStoreEntry_Release(&entry);
 *     metisMessage_Release(&message);
 *     metisLogger_Release(&logger);
 * }
 * @endcode
 * @see metisTimeOrderedList_Add
 */
bool metisTimeOrderedList_Remove(MetisTimeOrderedList *list, MetisContentStoreEntry *storeEntry);

/**
 * Return the oldest {@link MetisContentStoreEntry} instance in this list. That is, the one with the smallest
 * time value.
 *
 * @param list the list instance from which to retrieve the oldest storeEntry.
 * @param the oldest `MetisContentStoreEntry` in the list
 * @param NULL if no `MetisContentStoreEntry` was available.
 *
 * Example:
 * @code
 * {
 *     MetisLogger *logger = metisLogger_Create(...);
 *     MetisTimeOrderedList *list =
 *         metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
 *     MetisLruList *lruList = metisLruList_Create();
 *     MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, 2, logger);
 *
 *     MetisContentStoreEntry *entry = metisContentStoreEntry_Create(message, lruList, 2l, 1l);
 *
 *     metisTimeOrderedList_Add(list, entry);
 *
 *     MetisContentStoreEntry *oldest = metisTimeOrderedList_GetOldest(list);
 *
 *     ...
 *
 *     metisTimeOrderedList_Release(&list);
 *
 *     metisContentStoreEntry_Release(&entry);
 *     metisMessage_Release(&message);
 *     metisLruList_Destroy(&lruList);
 *     metisLogger_Release(&logger);
 * }
 * @endcode
 * @see metisTimeOrderedList_Remove
 */
MetisContentStoreEntry *metisTimeOrderedList_GetOldest(MetisTimeOrderedList *list);

/**
 * Return the number of items currently stored in the list.
 *
 * @param list the `MetisTimeOrderedList` instance from which to retrieve the count.
 * @return the number of items in the list.
 *
 * Example:
 * @code
 * {
 *     MetisLogger *logger = metisLogger_Create(...);
 *     MetisTimeOrderedList *list =
 *         metisTimeOrderedList_Create((MetisTimeOrderList_KeyCompare *) metisContentStoreEntry_CompareExpiryTime);
 *     MetisLruList *lruList = metisLruList_Create();
 *     MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) "\x00" "ehlo", 5, 111, 2, logger);
 *
 *     MetisContentStoreEntry *entry = metisContentStoreEntry_Create(message, lruList, 2l, 1l);
 *
 *     metisTimeOrderedList_Add(list, entry);
 *
 *     ...
 *     size_t numItemsInList = metisTimeOrderedList_Length(list);
 *     ...
 *
 *     metisTimeOrderedList_Release(&list);
 *
 *     metisContentStoreEntry_Release(&entry);
 *     metisMessage_Release(&message);
 *     metisLruList_Destroy(&lruList);
 *     metisLogger_Release(&logger);
 * }
 * @endcode
 */
size_t metisTimeOrderedList_Length(MetisTimeOrderedList *list);
#endif /* defined(__Metis__metis_TimeOrderedList__) */
