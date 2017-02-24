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
#include <ccnx/forwarder/metis/processor/metis_LruList.h>

struct metis_content_store_entry;
typedef struct metis_content_store_entry MetisContentStoreEntry;

/**
 * @function metisContentStoreEntry_Create
 * @abstract Creates a content store entry, saving a reference to the message
 * @discussion
 *   When <code>metisContentStoreEntry_Destroy()</code> is called, will release its reference
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisContentStoreEntry *metisContentStoreEntry_Create(MetisMessage *objectMessage, MetisLruList *lruList);

/**
 * @function metisContentStoreEntry_Copy
 * @abstract Returns a reference counted copy
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return Reference counted copy, must call <code>metisContentStoreEntry_Destroy()</code> on it.
 */
MetisContentStoreEntry *metisContentStoreEntry_Acquire(MetisContentStoreEntry *original);

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
 * @function metisContentStoreEntry_GetMessage
 * @abstract Returns a reference counted copy of the message.
 * @discussion
 *   Caller must use <code>metisMessage_Release()</code> on the returned message
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisMessage *metisContentStoreEntry_GetMessage(MetisContentStoreEntry *storeEntry);

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
