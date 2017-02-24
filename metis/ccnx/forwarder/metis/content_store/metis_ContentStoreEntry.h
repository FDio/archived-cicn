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


#ifndef Metis_metis_ContentStoreEntry_h
#define Metis_metis_ContentStoreEntry_h

#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/content_store/metis_LruList.h>

struct metis_contentstore_entry;
typedef struct metis_contentstore_entry MetisContentStoreEntry;

/**
 * The max time allowed for an ExpiryTime. Will never be exceeded.
 */
extern const uint64_t metisContentStoreEntry_MaxExpiryTime;

/**
 * The max time allowed for an RecommendedCacheTime. Will never be exceeded.
 */
extern const uint64_t metisContentStoreEntry_MaxRecommendedCacheTime;


/**
 * Creates a new `MetisContentStoreEntry` instance, acquiring a reference to the supplied `MetisMessage`.
 *
 * @param message the message to store
 * @param lruList the LRU list that this entry will be stored in.
 * @return A newly created `MetisContentStoreEntry` instance that must eventually be released by calling
 *         {@link metisContentStoreEntry_Release}.
 *
 * @see metisContentStoreEntry_Release
 */
MetisContentStoreEntry *metisContentStoreEntry_Create(MetisMessage *objectMessage, MetisLruList *lruList);

/**
 * Returns a reference counted copy of the supplied `MetisContentStoreEntry`.
 *
 * @param original the MetisContentStoreEntry to return a reference to.
 * @return Reference counted copy, must call <code>metisContentStoreEntry_Destroy()</code> on it.
 */
MetisContentStoreEntry *metisContentStoreEntry_Acquire(const MetisContentStoreEntry *original);

/**
 * Releases one reference count and destroys object when reaches zero
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in,out] entryPtr A pointer to an allocated MetisContentStoreEntry
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisContentStoreEntry_Release(MetisContentStoreEntry **entryPtr);

/**
 * Returns a pointer to the contained {@link MetisMessage}.
 * The caller must called {@link metisMessage_Acquire()} if they want to keep a reference to the returned
 * message.
 *
 * @param storeEntry the MetisContentStoreEntry from which to retrieve the `MetisMessage` pointer.
 * @return the address of the `MetisMessage` contained in the storeEntry.
 * @see metisMessage_Acquire
 */
MetisMessage *metisContentStoreEntry_GetMessage(const MetisContentStoreEntry *storeEntry);

/**
 * Return true if the message stored in this `MetisContentStoreEntry` has an ExpiryTime.
 *
 * @param storeEntry the MetisContentStoreEntry containing the message.
 * @return true if the referenced message has an ExpiryTime. False, otherwise.
 */
bool metisContentStoreEntry_HasExpiryTimeTicks(const MetisContentStoreEntry *storeEntry);

/**
 * Return the ExpiryTime stored in this `MetisContentStoreEntry`.
 *
 * @param storeEntry the MetisContentStoreEntry from which to retrieve the `MetisMessage` pointer.
 * @return the address of the `MetisMessage` contained in the storeEntry.
 */
uint64_t metisContentStoreEntry_GetExpiryTimeTicks(const MetisContentStoreEntry *storeEntry);

/**
 * Return true if the message stored in this `MetisContentStoreEntry` has a RecommendedCacheTime.
 *
 * @param storeEntry the MetisContentStoreEntry containing the message.
 * @return true if the referenced message has a RecommendedCacheTime. False, otherwise.
 */
bool metisContentStoreEntry_HasRecommendedCacheTimeTicks(const MetisContentStoreEntry *storeEntry);

/**
 * Return the RecommendedCacheTime stored in this `MetisContentStoreEntry`.
 *
 * @param storeEntry the MetisContentStoreEntry from which to retrieve the `MetisMessage` pointer.
 * @return the address of the `MetisMessage` contained in the storeEntry.
 */
uint64_t metisContentStoreEntry_GetRecommendedCacheTimeTicks(const MetisContentStoreEntry *storeEntry);

/**
 * A signum function comparing two `MetisContentStoreEntry` instances, using their
 * RecommendedCacheTime and, if necessary, the addresses of the referenced MetisMessage. In other words, if
 * two ContentStoreEntries have the same RecommendedCacheTime, the comparison will then be made on the
 * memory addresses of the MetisMessages referenced by the MetisContentStoreEntrys. So, the only way two
 * MetisContentStoreEntrys will compare equally (0) is if they both have the same RecommendedCacheTime and reference
 * the same MetisMessage.
 *
 * Used to determine the ordering relationship of two `MetisContentStoreEntry` instances.
 * This is used by the {@link MetisTimeOrderedList} to keep a list of MetisContentStoreEntrys, sorted by
 * RecommendedCacheTime.
 *
 * @param [in] storeEntry1 A pointer to a `MetisContentStoreEntry` instance.
 * @param [in] storeEntry2 A pointer to a `MetisContentStoreEntry` instance to be compared to `storeEntry1`.
 *
 * @return 0 if `storeEntry1` and `storeEntry2` are equivalent
 * @return < 0 if `storeEntry1` < `storeEntry2`
 * @return > 0 if `storeEntry1` > `storeEntry2`
 *
 * Example:
 * @code
 * {
 *     MetisContentStoreEntry *entry1 = metisContentStoreEntry_Create(...);
 *     MetisContentStoreEntry *entry2 = metisContentStoreEntry_Create(...);
 *
 *     int val = metisContentStoreEntry_CompareRecommendedCacheTime(entry1, entry2);
 *     if (val < 0) {
 *         // entry1 has a lower RecommendedCacheTime, or the same RecommendedCacheTime as entry2 and a different message.
 *     } else if (val > 0) {
 *         // entry2 has a lower RecommendedCacheTime, or the same RecommendedCacheTime as entry1 and a different message.
 *     } else {
 *         // entry1 and entry2 have the same RecommendedCacheTime AND the same message.
 *     }
 *
 *     metisContentStoreEntry_Release(&entry1);
 *     metisContentStoreEntry_Release(&entry2);
 *
 * }
 * @endcode
 * @see `metisContentStoreEntry_CompareExpiryTime`
 */
int metisContentStoreEntry_CompareRecommendedCacheTime(const MetisContentStoreEntry *storeEntry1, const MetisContentStoreEntry *storeEntry2);

/**
 * A signum function comparing two `MetisContentStoreEntry` instances, using their
 * ExpiryTime and, if necessary, the addresses of the referenced MetisMessage. In other words, if
 * two ContentStoreEntries have the same ExpiryTime, the comparison will then be made on the
 * memory addresses of the MetisMessages referenced by the MetisContentStoreEntrys. So, the only way two
 * MetisContentStoreEntrys will compare equally (0) is if they both have the same ExpiryTime and reference
 * the same MetisMessage.
 *
 * Used to determine the ordering relationship of two `MetisContentStoreEntry` instances.
 * This is used by the {@link MetisTimeOrderedList} to keep a list of MetisContentStoreEntrys, sorted by
 * ExpiryTime.
 *
 * @param [in] storeEntry1 A pointer to a `MetisContentStoreEntry` instance.
 * @param [in] storeEntry2 A pointer to a `MetisContentStoreEntry` instance to be compared to `storeEntry1`.
 *
 * @return 0 if `storeEntry1` and `storeEntry2` are equivalent
 * @return < 0 if `storeEntry1` < `storeEntry2`
 * @return > 0 if `storeEntry1` > `storeEntry2`
 *
 * Example:
 * @code
 * {
 *     MetisContentStoreEntry *entry1 = metisContentStoreEntry_Create(...);
 *     MetisContentStoreEntry *entry2 = metisContentStoreEntry_Create(...);
 *
 *     int val = metisContentStoreEntry_CompareExpiryTime(entry1, entry2);
 *     if (val < 0) {
 *         // entry1 has a lower ExpiryTime, or the same ExpiryTime as entry2 and a different message.
 *     } else if (val > 0) {
 *         // entry2 has a lower ExpiryTime, or the same ExpiryTime as entry1 and a different message.
 *     } else {
 *         // entry1 and entry2 have the same ExpiryTime AND the same message.
 *     }
 *
 *     metisContentStoreEntry_Release(&entry1);
 *     metisContentStoreEntry_Release(&entry2);
 *
 * }
 * @endcode
 * @see `metisContentStoreEntry_CompareRecommendedCacheTime`
 */
int metisContentStoreEntry_CompareExpiryTime(const MetisContentStoreEntry *storeEntry1, const MetisContentStoreEntry *storeEntry2);

/**
 * Move this entry to the head of the LRU list
 *
 * Moves the entry to the head of the LRU list it was created with
 *
 * @param [in] storeEntry An allocated MetisContenstoreEntry
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisContentStoreEntry_MoveToHead(MetisContentStoreEntry *storeEntry);
#endif // Metis_metis_ContentStoreEntry_h
