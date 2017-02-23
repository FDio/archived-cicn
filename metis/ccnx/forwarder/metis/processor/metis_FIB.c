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
 * Right now, the FIB table is sparse.  There can be an entry for /a and for /a/b/c, but
 * not for /a/b.  This means we need to exhastively lookup all the components to make sure
 * there's not a route for it.
 *
 */

#include <config.h>
#include <stdio.h>

#include <ccnx/forwarder/metis/processor/metis_FIB.h>
#include <ccnx/forwarder/metis/processor/metis_FibEntry.h>
#include <ccnx/forwarder/metis/processor/metis_HashTableFunction.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_TreeRedBlack.h>

#include <LongBow/runtime.h>

// =====================================================

/**
 * @function hashTableFunction_FibEntryDestroyer
 * @abstract Used in the hash table to destroy the data pointer when an item's removed
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
static void
_hashTableFunction_FibEntryDestroyer(void **dataPtr)
{
    metisFibEntry_Release((MetisFibEntry **) dataPtr);
}

/**
 * @function hashTableFunction_TlvNameDestroyer
 * @abstract Used in the hash table to destroy the key pointer when an item's removed
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
static void
_hashTableFunction_TlvNameDestroyer(void **dataPtr)
{
    metisTlvName_Release((MetisTlvName **) dataPtr);
}

// =====================================================

struct metis_fib {
    // KEY = tlvName, VALUE = FibEntry
    PARCHashCodeTable *tableByName;

    // KEY = tlvName.  We use a tree for the keys because that
    // has the same average insert and remove time.  The tree
    // is only used by GetEntries, which in turn is used by things
    // that want to enumerate the FIB
    PARCTreeRedBlack *tableOfKeys;

    MetisLogger *logger;

    // If there are no forward paths, we return an emtpy set.  Allocate this
    // once and return a reference to it whenever we need an empty set.
    MetisNumberSet *emptySet;
};

static MetisFibEntry *_metisFIB_CreateFibEntry(MetisFIB *fib, MetisTlvName *tlvName, const char *fwdStrategy);

// =====================================================
// Public API

MetisFIB *
metisFIB_Create(MetisLogger *logger)
{
    unsigned initialSize = 1024;

    MetisFIB *fib = parcMemory_AllocateAndClear(sizeof(MetisFIB));
    assertNotNull(fib, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisFIB));
    fib->emptySet = metisNumberSet_Create();
    fib->logger = metisLogger_Acquire(logger);
    fib->tableByName = parcHashCodeTable_Create_Size(metisHashTableFunction_TlvNameEquals,
                                                     metisHashTableFunction_TlvNameHashCode,
                                                     _hashTableFunction_TlvNameDestroyer,
                                                     _hashTableFunction_FibEntryDestroyer,
                                                     initialSize);

    fib->tableOfKeys =
        parcTreeRedBlack_Create(metisHashTableFunction_TlvNameCompare, NULL, NULL, NULL, NULL, NULL);

    if (metisLogger_IsLoggable(fib->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(fib->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "FIB %p created with initialSize %u",
                        (void *) fib, initialSize);
    }

    return fib;
}

void
metisFIB_Destroy(MetisFIB **fibPtr)
{
    assertNotNull(fibPtr, "Parameter must be non-null double pointer");
    assertNotNull(*fibPtr, "Parameter must dereference to non-null pointer");

    MetisFIB *fib = *fibPtr;

    if (metisLogger_IsLoggable(fib->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(fib->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "FIB %p destroyed",
                        (void *) fib);
    }

    metisNumberSet_Release(&fib->emptySet);
    metisLogger_Release(&fib->logger);
    parcTreeRedBlack_Destroy(&fib->tableOfKeys);
    parcHashCodeTable_Destroy(&fib->tableByName);
    parcMemory_Deallocate((void **) &fib);
    *fibPtr = NULL;
}

MetisFibEntry *
metisFIB_Match(MetisFIB *fib, const MetisMessage *interestMessage)
{
    assertNotNull(fib, "Parameter fib must be non-null");
    assertNotNull(interestMessage, "Parameter interestMessage must be non-null");

    if (metisMessage_HasName(interestMessage)) {
        // this is NOT reference counted, don't destroy it
        MetisTlvName *tlvName = metisMessage_GetName(interestMessage);
        MetisFibEntry *longestMatchingFibEntry = NULL;

        // because the FIB table is sparse, we need to scan all the name segments in order.
        for (size_t i = 0; i < metisTlvName_SegmentCount(tlvName); i++) {
            MetisTlvName *prefixName = metisTlvName_Slice(tlvName, i + 1);
            MetisFibEntry *fibEntry = parcHashCodeTable_Get(fib->tableByName, prefixName);
            if (fibEntry != NULL) {

                // we can accept the FIB entry if it does not contain the ingress connection id or if
                // there is more than one forward path besides the ingress connection id.
                const MetisNumberSet *nexthops = metisFibEntry_GetNexthops(fibEntry);
                bool containsIngressConnectionId = metisNumberSet_Contains(nexthops, metisMessage_GetIngressConnectionId(interestMessage));
                size_t nextHopsCount = metisNumberSet_Length(nexthops);
                // Further control on the nextHopCount, because if the first condition is true (no ingress connection among the next hops), the number of next hops could still be 0.
                if ((!containsIngressConnectionId && nextHopsCount > 0) || nextHopsCount > 1) {
                    longestMatchingFibEntry = fibEntry;
                }
            }
            metisTlvName_Release(&prefixName);
        }
        return longestMatchingFibEntry;
    }
    
    return NULL;
}

bool
metisFIB_AddOrUpdate(MetisFIB *fib, CPIRouteEntry *route, char const * fwdStrategy) 
{
    assertNotNull(fib, "Parameter fib must be non-null");
    assertNotNull(route, "Parameter route must be non-null");

    const CCNxName *ccnxName = cpiRouteEntry_GetPrefix(route);
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);

    MetisFibEntry *fibEntry = parcHashCodeTable_Get(fib->tableByName, tlvName);
    if (fibEntry == NULL) {
        if(fwdStrategy == NULL){
            fwdStrategy = "random"; //default strategy for now
        }
        fibEntry = _metisFIB_CreateFibEntry(fib, tlvName, fwdStrategy);
    }

    metisFibEntry_AddNexthop(fibEntry, route);

    // if anyone saved the name in a table, they copied it.
    metisTlvName_Release(&tlvName);

    return true;
}

bool
metisFIB_Remove(MetisFIB *fib, CPIRouteEntry *route)
{
    assertNotNull(fib, "Parameter fib must be non-null");
    assertNotNull(route, "Parameter route must be non-null");

    bool routeRemoved = false;

    const CCNxName *ccnxName = cpiRouteEntry_GetPrefix(route);
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);

    MetisFibEntry *fibEntry = parcHashCodeTable_Get(fib->tableByName, tlvName);
    if (fibEntry != NULL) {
        metisFibEntry_RemoveNexthopByRoute(fibEntry, route);
        if (metisFibEntry_NexthopCount(fibEntry) == 0) {
            parcTreeRedBlack_Remove(fib->tableOfKeys, tlvName);

            // this will de-allocate the key, so must be done last
            parcHashCodeTable_Del(fib->tableByName, tlvName);

            routeRemoved = true;
        }
    }

    metisTlvName_Release(&tlvName);
    return routeRemoved;
}

size_t
metisFIB_Length(const MetisFIB *fib)
{
    assertNotNull(fib, "Parameter fib must be non-null");
    return parcHashCodeTable_Length(fib->tableByName);
}

MetisFibEntryList *
metisFIB_GetEntries(const MetisFIB *fib)
{
    assertNotNull(fib, "Parameter fib must be non-null");
    MetisFibEntryList *list = metisFibEntryList_Create();

    PARCArrayList *values = parcTreeRedBlack_Values(fib->tableOfKeys);
    for (size_t i = 0; i < parcArrayList_Size(values); i++) {
        MetisFibEntry *original = (MetisFibEntry *) parcArrayList_Get(values, i);
        metisFibEntryList_Append(list, original);
    }
    parcArrayList_Destroy(&values);
    return list;
}

void
metisFIB_RemoveConnectionIdFromRoutes(MetisFIB *fib, unsigned connectionId)
{
    assertNotNull(fib, "Parameter fib must be non-null");

    // Walk the entire tree and remove the connection id from every entry.
    PARCArrayList *values = parcTreeRedBlack_Values(fib->tableOfKeys);
    for (size_t i = 0; i < parcArrayList_Size(values); i++) {
        MetisFibEntry *original = (MetisFibEntry *) parcArrayList_Get(values, i);
        metisFibEntry_RemoveNexthopByConnectionId(original, connectionId);
    }
    parcArrayList_Destroy(&values);
}

// =========================================================================
// Private API

/**
 * @function metisFib_CreateFibEntry
 * @abstract Create the given FIB entry
 * @discussion
 *    PRECONDITION: You know that the FIB entry does not exist already
 *
 * @param <#param1#>
 * @return <#return#>
 */
static MetisFibEntry *
_metisFIB_CreateFibEntry(MetisFIB *fib, MetisTlvName *tlvName, const char *fwdStrategy)
{
    MetisFibEntry *entry = metisFibEntry_Create(tlvName, fwdStrategy);

    // add a reference counted name, as we specified a key destroyer when we
    // created the table.
    MetisTlvName *copy = metisTlvName_Acquire(tlvName);
    parcHashCodeTable_Add(fib->tableByName, copy, entry);

    // this is an index structure.  It does not have its own destroyer functions in
    // the data structure.  The data in this table is the same pointer as in the hash table.
    parcTreeRedBlack_Insert(fib->tableOfKeys, copy, entry);

    return entry;
}
