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
#include <parc/algol/parc_ArrayList.h>

#include <ccnx/forwarder/metis/io/metis_ListenerSet.h>

#include <LongBow/runtime.h>

struct metis_listener_set {
    PARCArrayList *listOfListeners;
};

static void
metisListenerSet_DestroyListenerOps(void **opsPtr)
{
    MetisListenerOps *ops = *((MetisListenerOps **) opsPtr);
    ops->destroy(&ops);
}

MetisListenerSet *
metisListenerSet_Create()
{
    MetisListenerSet *set = parcMemory_AllocateAndClear(sizeof(MetisListenerSet));
    assertNotNull(set, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisListenerSet));
    set->listOfListeners = parcArrayList_Create(metisListenerSet_DestroyListenerOps);

    return set;
}

void
metisListenerSet_Destroy(MetisListenerSet **setPtr)
{
    assertNotNull(setPtr, "Parameter must be non-null double pointer");
    assertNotNull(*setPtr, "Parameter must dereference to non-null pointer");

    MetisListenerSet *set = *setPtr;
    parcArrayList_Destroy(&set->listOfListeners);
    parcMemory_Deallocate((void **) &set);
    *setPtr = NULL;
}

/**
 * @function metisListenerSet_Add
 * @abstract Adds the listener to the set
 * @discussion
 *     Unique set based on pair (MetisEncapType, localAddress)
 *
 * @param <#param1#>
 * @return <#return#>
 */
bool
metisListenerSet_Add(MetisListenerSet *set, MetisListenerOps *ops)
{
    assertNotNull(set, "Parameter set must be non-null");
    assertNotNull(ops, "Parameter ops must be non-null");

    int opsEncap = ops->getEncapType(ops);
    const CPIAddress *opsAddress = ops->getListenAddress(ops);

    // make sure its not in the set
    size_t length = parcArrayList_Size(set->listOfListeners);
    for (size_t i = 0; i < length; i++) {
        MetisListenerOps *entry = parcArrayList_Get(set->listOfListeners, i);

        int entryEncap = entry->getEncapType(entry);
        const CPIAddress *entryAddress = entry->getListenAddress(entry);

        if (opsEncap == entryEncap && cpiAddress_Equals(opsAddress, entryAddress)) {
            // duplicate
            return false;
        }
    }

    parcArrayList_Add(set->listOfListeners, ops);
    return true;
}

size_t
metisListenerSet_Length(const MetisListenerSet *set)
{
    assertNotNull(set, "Parameter set must be non-null");
    return parcArrayList_Size(set->listOfListeners);
}

/**
 * Returns the listener at the given index
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] set An allocated listener set
 * @param [in] index The index position (0 <= index < metisListenerSet_Count)
 *
 * @retval non-null The listener at index
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisListenerOps *
metisListenerSet_Get(const MetisListenerSet *set, size_t index)
{
    assertNotNull(set, "Parameter set must be non-null");
    return parcArrayList_Get(set->listOfListeners, index);
}

MetisListenerOps *
metisListenerSet_Find(const MetisListenerSet *set, MetisEncapType encapType, const CPIAddress *localAddress)
{
    assertNotNull(set, "Parameter set must be non-null");
    assertNotNull(localAddress, "Parameter localAddress must be non-null");

    MetisListenerOps *match = NULL;

    for (size_t i = 0; i < parcArrayList_Size(set->listOfListeners) && !match; i++) {
        MetisListenerOps *ops = parcArrayList_Get(set->listOfListeners, i);
        assertNotNull(ops, "Got null listener ops at index %zu", i);

        if (ops->getEncapType(ops) == encapType) {
            if (cpiAddress_Equals(localAddress, ops->getListenAddress(ops))) {
                match = ops;
            }
        }
    }

    return match;
}
