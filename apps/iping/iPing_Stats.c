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

#include <stdio.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/transport/common/transport_MetaMessage.h>

#include <parc/algol/parc_HashMap.h>
#include <parc/algol/parc_DisplayIndented.h>

#include "iPing_Stats.h"

typedef struct ping_stats_entry {
  uint64_t sendTimeInUs;
  uint64_t rtt;
  size_t size;
  CCNxName *nameSent;
} CCNxPingStatsEntry;

struct ping_stats {
  uint64_t totalRtt;
  size_t totalReceived;
  size_t totalSent;
  size_t totalLost;
  PARCHashMap *pings;
};

static bool _ccnxPingStatsEntry_Destructor(CCNxPingStatsEntry **statsPtr) {
  CCNxPingStatsEntry *entry = *statsPtr;
  ccnxName_Release(&entry->nameSent);

  return true;
}

static bool _ccnxPingStats_Destructor(CCNxPingStats **statsPtr) {
  CCNxPingStats *stats = *statsPtr;
  parcHashMap_Release(&stats->pings);
  return true;
}

parcObject_Override(CCNxPingStatsEntry,
                    PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxPingStatsEntry_Destructor);

parcObject_ImplementAcquire(ccnxPingStatsEntry, CCNxPingStatsEntry);
parcObject_ImplementRelease(ccnxPingStatsEntry, CCNxPingStatsEntry);

CCNxPingStatsEntry *ccnxPingStatsEntry_Create() {
  return parcObject_CreateInstance(CCNxPingStatsEntry);
}

parcObject_Override(CCNxPingStats, PARCObject, .destructor = (PARCObjectDestructor *) _ccnxPingStats_Destructor);

parcObject_ImplementAcquire(ccnxPingStats, CCNxPingStats);
parcObject_ImplementRelease(ccnxPingStats, CCNxPingStats);

CCNxPingStats *ccnxPingStats_Create(void) {
  CCNxPingStats *stats = parcObject_CreateInstance(CCNxPingStats);

  stats->pings = parcHashMap_Create();
  stats->totalSent = 0;
  stats->totalReceived = 0;
  stats->totalRtt = 0;

  return stats;
}

void ccnxPingStats_RecordRequest(CCNxPingStats *stats, CCNxName *name, uint64_t currentTime) {
  CCNxPingStatsEntry *entry = ccnxPingStatsEntry_Create();

  entry->nameSent = ccnxName_Acquire(name);
  entry->sendTimeInUs = currentTime;

  stats->totalSent++;

  char *nameString = ccnxName_ToString(name);
  printf("Insert entry %s\n", nameString);
  parcMemory_Deallocate(&nameString);
  parcHashMap_Put(stats->pings, name, entry);
}

uint64_t ccnxPingStats_RecordResponse(CCNxPingStats *stats,
                                      CCNxName *nameResponse,
                                      uint64_t currentTime,
                                      CCNxMetaMessage *message,
                                      bool *existing) {
  CCNxPingStatsEntry *entry = (CCNxPingStatsEntry *) parcHashMap_Get(stats->pings, nameResponse);
  if (entry != NULL) {
    *existing = true;
    stats->totalReceived++;
    uint64_t rtt = currentTime - entry->sendTimeInUs;
    stats->totalRtt += rtt;
    ccnxPingStatsEntry_Release(&entry);
    parcHashMap_Remove(stats->pings, nameResponse);
    return rtt;
  } else {
    *existing = false;
    char *nameString = ccnxName_ToString(nameResponse);
    printf("No entry for name %s\n", nameString);
    parcMemory_Deallocate(&nameString);
  }
  return 0;
}

size_t ccnxPingStats_RecordLost(CCNxPingStats *stats, CCNxName *nameResponse) {
  //size_t pingsReceived = stats->totalReceived + 1;
  CCNxPingStatsEntry *entry = (CCNxPingStatsEntry *) parcHashMap_Get(stats->pings, nameResponse);
  if (entry != NULL) {
    stats->totalLost++;
    ccnxPingStatsEntry_Release(&entry);
    parcHashMap_Remove(stats->pings, nameResponse);
  }
  return 0;
}

bool ccnxPingStats_Display(CCNxPingStats *stats) {
  if (stats->totalReceived > 0) {
    parcDisplayIndented_PrintLine(0,
                                  "Sent = %zu : Received = %zu : AvgDelay %llu us",
                                  stats->totalSent,
                                  stats->totalReceived,
                                  stats->totalRtt / stats->totalReceived);
    return true;
  }
  return false;
}
