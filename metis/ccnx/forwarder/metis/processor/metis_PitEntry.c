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
#include <ccnx/forwarder/metis/processor/metis_PitEntry.h>
#include <ccnx/forwarder/metis/core/metis_NumberSet.h>

#include <LongBow/runtime.h>

struct metis_pit_entry {
    MetisMessage *message;
    MetisNumberSet *ingressIdSet;
    MetisNumberSet *egressIdSet;

    MetisFibEntry *fibEntry;

    MetisTicks creationTime;
    MetisTicks expiryTime;

    unsigned refcount;
};

MetisPitEntry *
metisPitEntry_Create(MetisMessage *message, MetisTicks expiryTime, MetisTicks creationTime)
{
    MetisPitEntry *pitEntry = parcMemory_AllocateAndClear(sizeof(MetisPitEntry));
    assertNotNull(pitEntry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisPitEntry));
    pitEntry->message = message;
    pitEntry->ingressIdSet = metisNumberSet_Create();
    pitEntry->egressIdSet = metisNumberSet_Create();
    pitEntry->refcount = 1;

    // add the message to the reverse path set
    metisNumberSet_Add(pitEntry->ingressIdSet, metisMessage_GetIngressConnectionId(message));

    // hack in a 4-second timeout
    pitEntry->expiryTime = expiryTime;
    pitEntry->fibEntry = NULL;

    pitEntry->creationTime = creationTime;
    return pitEntry;
}

void
metisPitEntry_Release(MetisPitEntry **pitEntryPtr)
{
    assertNotNull(pitEntryPtr, "Parameter must be non-null double pointer");
    assertNotNull(*pitEntryPtr, "Parameter must dereference to non-null pointer");

    MetisPitEntry *pitEntry = *pitEntryPtr;
    trapIllegalValueIf(pitEntry->refcount == 0, "Illegal state: has refcount of 0");

    pitEntry->refcount--;
    if (pitEntry->refcount == 0) {
        if(pitEntry->fibEntry != NULL){
            metisFibEntry_Release(&pitEntry->fibEntry);
        }
        metisNumberSet_Release(&pitEntry->ingressIdSet);
        metisNumberSet_Release(&pitEntry->egressIdSet);
        metisMessage_Release(&pitEntry->message);
        parcMemory_Deallocate((void **) &pitEntry);
    }
    *pitEntryPtr = NULL;
}

MetisPitEntry *
metisPitEntry_Acquire(MetisPitEntry *original)
{
    assertNotNull(original, "Parameter original must be non-null");
    original->refcount++;
    return original;
}

void
metisPitEntry_AddIngressId(MetisPitEntry *pitEntry, unsigned ingressId)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    metisNumberSet_Add(pitEntry->ingressIdSet, ingressId);
}

void
metisPitEntry_AddEgressId(MetisPitEntry *pitEntry, unsigned egressId)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    metisNumberSet_Add(pitEntry->egressIdSet, egressId);
}

void
metisPitEntry_AddFibEntry(MetisPitEntry *pitEntry, MetisFibEntry *fibEntry)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    assertNotNull(fibEntry, "Parameter fibEntry must be non-null");
    //the fibEntry should be always the same for all the interests in the same pitEntry 
    if(pitEntry->fibEntry == NULL){
        metisFibEntry_Acquire(fibEntry);
        pitEntry->fibEntry = fibEntry;
    }
}

MetisFibEntry *
metisPitEntry_GetFibEntry(MetisPitEntry *pitEntry)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    return pitEntry->fibEntry;
}

MetisTicks
metisPitEntry_GetExpiryTime(const MetisPitEntry *pitEntry)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    return pitEntry->expiryTime;
}

MetisTicks
metisPitEntry_GetCreationTime(const MetisPitEntry *pitEntry)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    return pitEntry->creationTime;
}

void
metisPitEntry_SetExpiryTime(MetisPitEntry *pitEntry, MetisTicks expiryTime)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    pitEntry->expiryTime = expiryTime;
}


const MetisNumberSet *
metisPitEntry_GetIngressSet(const MetisPitEntry *pitEntry)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    return pitEntry->ingressIdSet;
}

const MetisNumberSet *
metisPitEntry_GetEgressSet(const MetisPitEntry *pitEntry)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    return pitEntry->egressIdSet;
}

MetisMessage *
metisPitEntry_GetMessage(const MetisPitEntry *pitEntry)
{
    assertNotNull(pitEntry, "Parameter pitEntry must be non-null");
    return metisMessage_Acquire(pitEntry->message);
}
