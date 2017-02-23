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

#include <config.h>
#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <ccnx/forwarder/metis/processor/metis_ContentStoreEntry.h>

#include <LongBow/runtime.h>

struct metis_content_store_entry {
    MetisMessage *message;
    MetisLruListEntry *lruEntry;
    unsigned refcount;
};

MetisContentStoreEntry *
metisContentStoreEntry_Create(MetisMessage *objectMessage, MetisLruList *lruList)
{
    assertNotNull(objectMessage, "Parameter objectMessage must be non-null");

    MetisContentStoreEntry *entry = parcMemory_AllocateAndClear(sizeof(MetisContentStoreEntry));
    assertNotNull(entry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisContentStoreEntry));
    entry->message = metisMessage_Acquire(objectMessage);
    entry->refcount = 1;
    entry->lruEntry = metisLruList_NewHeadEntry(lruList, entry);

    return entry;
}

MetisContentStoreEntry *
metisContentStoreEntry_Acquire(MetisContentStoreEntry *original)
{
    assertNotNull(original, "Parameter must be non-null");
    original->refcount++;
    return original;
}

void
metisContentStoreEntry_Release(MetisContentStoreEntry **entryPtr)
{
    assertNotNull(entryPtr, "Parameter must be non-null double pointer");
    assertNotNull(*entryPtr, "Parameter must dereference to non-null pointer");

    MetisContentStoreEntry *entry = *entryPtr;
    assertTrue(entry->refcount > 0, "Illegal state: has refcount of 0");

    entry->refcount--;
    if (entry->refcount == 0) {
        metisLruList_EntryDestroy(&entry->lruEntry);
        metisMessage_Release(&entry->message);
        parcMemory_Deallocate((void **) &entry);
    }
    *entryPtr = NULL;
}

/**
 * @function metisContentStoreEntry_GetMessage
 * @abstract Returns a reference counted copy of the message.
 * @discussion
 *   Caller must use <code>metisMessage_Release()</code> on the returned message
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisMessage *
metisContentStoreEntry_GetMessage(MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    return metisMessage_Acquire(storeEntry->message);
}

void
metisContentStoreEntry_MoveToHead(MetisContentStoreEntry *storeEntry)
{
    assertNotNull(storeEntry, "Parameter must be non-null");
    metisLruList_EntryMoveToHead(storeEntry->lruEntry);
}
