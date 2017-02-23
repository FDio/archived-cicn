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
 * The pending interest table.
 *
 * Interest aggregation strategy:
 * - The first Interest for a name is forwarded
 * - A second Interest for a name from a different reverse path may be aggregated
 * - A second Interest for a name from an existing Interest is forwarded
 * - The Interest Lifetime is like a subscription time.  A reverse path entry is removed once the lifetime
 *   is exceeded.
 * - Whan an Interest arrives or is aggregated, the Lifetime for that reverse hop is extended.  As a simplification,
 *   we only keep a single lifetime not per reverse hop.
 *
 */

#include <config.h>
#include <stdio.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <ccnx/forwarder/metis/processor/metis_PIT.h>
#include <ccnx/forwarder/metis/processor/metis_MatchingRulesTable.h>

#include <ccnx/forwarder/metis/core/metis_Ticks.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_HashCodeTable.h>
#include <parc/algol/parc_Hash.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

#include <LongBow/runtime.h>

struct metis_standard_pit;
typedef struct metis_standard_pit MetisStandardPIT;

struct metis_standard_pit {
    MetisForwarder *metis;
    MetisLogger *logger;

    MetisMatchingRulesTable *table;

    // counters to track how many of each type of Interest we get
    unsigned insertCounterByName;
    unsigned insertCounterByKeyId;
    unsigned insertCounterByObjectHash;
};

static void _metisPIT_StoreInTable(MetisStandardPIT *pit, MetisMessage *interestMessage);

static void
_metisPIT_PitEntryDestroyer(void **dataPtr)
{
    metisPitEntry_Release((MetisPitEntry **) dataPtr);
}

static bool
_metisPIT_IngressSetContains(MetisPitEntry *pitEntry, unsigned connectionId)
{
    const MetisNumberSet *set = metisPitEntry_GetIngressSet(pitEntry);
    bool numberInSet = metisNumberSet_Contains(set, connectionId);
    return numberInSet;
}

static MetisTicks
_metisPIT_CalculateLifetime(MetisStandardPIT *pit, MetisMessage *interestMessage)
{
    uint64_t interestLifetimeTicks = 0;

    if (metisMessage_HasInterestLifetime(interestMessage)) {
        interestLifetimeTicks = metisMessage_GetInterestLifetimeTicks(interestMessage);
    } else {
        interestLifetimeTicks = metisForwarder_NanosToTicks(4000000000ULL);
    }

    MetisTicks expiryTime = metisForwarder_GetTicks(pit->metis) + interestLifetimeTicks;
    return expiryTime;
}

static void
_metisPIT_StoreInTable(MetisStandardPIT *pit, MetisMessage *interestMessage)
{
    MetisMessage *key = metisMessage_Acquire(interestMessage);

    MetisTicks expiryTime = _metisPIT_CalculateLifetime(pit, interestMessage);

    MetisPitEntry *pitEntry = metisPitEntry_Create(key, expiryTime, metisForwarder_GetTicks(pit->metis));
    // this is done in metisPitEntry_Create
    //    metisPitEntry_AddIngressId(pitEntry, metisMessage_GetIngressConnectionId(interestMessage));

    metisMatchingRulesTable_AddToBestTable(pit->table, key, pitEntry);

    if (metisLogger_IsLoggable(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "Message %p added to PIT (expiry %" PRIu64 ") ingress %u",
                        (void *) interestMessage,
                        metisPitEntry_GetExpiryTime(pitEntry),
                        metisMessage_GetIngressConnectionId(interestMessage));
    }
}

static void
_metisPIT_ExtendLifetime(MetisStandardPIT *pit, MetisPitEntry *pitEntry, MetisMessage *interestMessage)
{
    MetisTicks expiryTime = _metisPIT_CalculateLifetime(pit, interestMessage);
    metisPitEntry_SetExpiryTime(pitEntry, expiryTime);
}

// this appears to only be used in some unit tests
__attribute__((unused))
static void
_metisPIT_AddEgressConnectionId(MetisPIT *generic, const MetisMessage *interestMessage, unsigned connectionId)
{
    assertNotNull(generic, "Parameter pit must be non-null");
    assertNotNull(interestMessage, "Parameter interestMessage must be non-null");

    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisPitEntry *entry = metisMatchingRulesTable_Get(pit->table, interestMessage);
    if (entry) {
        metisPitEntry_AddEgressId(entry, connectionId);
    }
}


// ======================================================================
// Interface API

static void
_metisStandardPIT_Destroy(MetisPIT **pitPtr)
{
    assertNotNull(pitPtr, "Parameter must be non-null double pointer");
    assertNotNull(*pitPtr, "Parameter must dereference to non-null pointer");

    MetisStandardPIT *pit = metisPIT_Closure(*pitPtr);

    if (metisLogger_IsLoggable(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "PIT %p destroyed",
                        (void *) pit);
    }

    metisMatchingRulesTable_Destroy(&pit->table);
    metisLogger_Release(&pit->logger);
    parcMemory_Deallocate(pitPtr);
}

// There's a bit too much going on in this function, need to break it
// apart for testability and style.
static MetisPITVerdict
_metisStandardPIT_ReceiveInterest(MetisPIT *generic, MetisMessage *interestMessage)
{
    assertNotNull(generic, "Parameter pit must be non-null");
    assertNotNull(interestMessage, "Parameter interestMessage must be non-null");

    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisPitEntry *pitEntry = metisMatchingRulesTable_Get(pit->table, interestMessage);

    if (pitEntry) {
        // has it expired?
        MetisTicks now = metisForwarder_GetTicks(pit->metis);
        if (now < metisPitEntry_GetExpiryTime(pitEntry)) {
            _metisPIT_ExtendLifetime(pit, pitEntry, interestMessage);

            // Is the reverse path already in the PIT entry?
            if (_metisPIT_IngressSetContains(pitEntry, metisMessage_GetIngressConnectionId(interestMessage))) {
                // It is already in the PIT entry, so this is a retransmission, so forward it.

                if (metisLogger_IsLoggable(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
                    metisLogger_Log(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                                    "Message %p existing entry (expiry %" PRIu64 ") and reverse path, forwarding",
                                    (void *) interestMessage,
                                    metisPitEntry_GetExpiryTime(pitEntry));
                }

                return MetisPITVerdict_Forward;
            }

            // It is in the PIT but this is the first interest for the reverse path
            metisPitEntry_AddIngressId(pitEntry, metisMessage_GetIngressConnectionId(interestMessage));

            if (metisLogger_IsLoggable(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
                metisLogger_Log(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                                "Message %p existing entry (expiry %" PRIu64 ") and reverse path is new, aggregate",
                                (void *) interestMessage,
                                metisPitEntry_GetExpiryTime(pitEntry));
            }

            return MetisPITVerdict_Aggregate;
        }
        //this is a timeout....
        MetisFibEntry *fibEntry = metisPitEntry_GetFibEntry(pitEntry);
        if (fibEntry != NULL) {
            metisFibEntry_OnTimeout(fibEntry, metisPitEntry_GetEgressSet(pitEntry));
        }

        // it's an old entry, remove it
        metisMatchingRulesTable_RemoveFromBest(pit->table, interestMessage);
    }

    _metisPIT_StoreInTable(pit, interestMessage);

    return MetisPITVerdict_Forward;
}

static MetisNumberSet *
_metisStandardPIT_SatisfyInterest(MetisPIT *generic, const MetisMessage *objectMessage)
{
    assertNotNull(generic, "Parameter pit must be non-null");
    assertNotNull(objectMessage, "Parameter objectMessage must be non-null");

    MetisStandardPIT *pit = metisPIT_Closure(generic);

    // we need to look in all three tables to see if there's anything
    // to satisy in each of them and take the union of the reverse path sets.

    MetisNumberSet *ingressSetUnion = metisNumberSet_Create();

    PARCArrayList *list = metisMatchingRulesTable_GetUnion(pit->table, objectMessage);
    for (size_t i = 0; i < parcArrayList_Size(list); i++) {
        MetisPitEntry *pitEntry = (MetisPitEntry *) parcArrayList_Get(list, i);

        MetisFibEntry *fibEntry = metisPitEntry_GetFibEntry(pitEntry);
        if (fibEntry != NULL) {
            //this is a rough estimation of the residual RTT
            MetisTicks rtt = metisForwarder_GetTicks(pit->metis) - metisPitEntry_GetCreationTime(pitEntry);
            metisFibEntry_ReceiveObjectMessage(fibEntry, metisPitEntry_GetEgressSet(pitEntry), objectMessage, rtt); //need to implement RTT
        }

        // this is a reference counted return
        const MetisNumberSet *ingressSet = metisPitEntry_GetIngressSet(pitEntry);
        metisNumberSet_AddSet(ingressSetUnion, ingressSet);

        // and remove it from the PIT.  Key is a reference counted copy of the pit entry message
        MetisMessage *key = metisPitEntry_GetMessage(pitEntry);
        metisMatchingRulesTable_RemoveFromBest(pit->table, key);
        metisMessage_Release(&key);
    }
    parcArrayList_Destroy(&list);

    return ingressSetUnion;
}

static void
_metisStandardPIT_RemoveInterest(MetisPIT *generic, const MetisMessage *interestMessage)
{
    assertNotNull(generic, "Parameter pit must be non-null");
    assertNotNull(interestMessage, "Parameter interestMessage must be non-null");

    MetisStandardPIT *pit = metisPIT_Closure(generic);

    if (metisLogger_IsLoggable(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "Message %p removed from PIT",
                        (void *) interestMessage);
    }

    metisMatchingRulesTable_RemoveFromBest(pit->table, interestMessage);
}

static MetisPitEntry *
_metisStandardPIT_GetPitEntry(const MetisPIT *generic, const MetisMessage *interestMessage)
{
    assertNotNull(generic, "Parameter pit must be non-null");
    assertNotNull(interestMessage, "Parameter interestMessage must be non-null");

    MetisStandardPIT *pit = metisPIT_Closure(generic);

    MetisPitEntry *entry = metisMatchingRulesTable_Get(pit->table, interestMessage);
    if (entry) {
        return metisPitEntry_Acquire(entry);
    }
    return NULL;
}


// ======================================================================
// Public API

MetisPIT *
metisStandardPIT_Create(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");

    size_t allocation = sizeof(MetisPIT) + sizeof(MetisStandardPIT);

    MetisPIT *generic = parcMemory_AllocateAndClear(allocation);
    assertNotNull(generic, "parcMemory_AllocateAndClear(%zu) returned NULL", allocation);
    generic->closure = (uint8_t *) generic + sizeof(MetisPIT);

    MetisStandardPIT *pit = metisPIT_Closure(generic);
    pit->metis = metis;
    pit->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));
    pit->table = metisMatchingRulesTable_Create(_metisPIT_PitEntryDestroyer);

    if (metisLogger_IsLoggable(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug)) {
        metisLogger_Log(pit->logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug, __func__,
                        "PIT %p created",
                        (void *) pit);
    }

    generic->getPitEntry = _metisStandardPIT_GetPitEntry;
    generic->receiveInterest = _metisStandardPIT_ReceiveInterest;
    generic->release = _metisStandardPIT_Destroy;
    generic->removeInterest = _metisStandardPIT_RemoveInterest;
    generic->satisfyInterest = _metisStandardPIT_SatisfyInterest;

    return generic;
}

