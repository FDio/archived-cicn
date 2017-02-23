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
 *
 * Implements the RtaCommandTransmitStatistics object which signals to RTA Framework to open a new connection
 * with the given configuration.
 */

#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>
#include <sys/param.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <ccnx/transport/transport_rta/commands/rta_CommandTransmitStatistics.h>

struct rta_command_transmitstatistics {
    struct timeval period;
    char *filename;
};

// ======= Private API

static void
_rtaCommandTransmitStatistics_Destroy(RtaCommandTransmitStatistics **transmitStatsPtr)
{
    RtaCommandTransmitStatistics *transmitStats = *transmitStatsPtr;
    parcMemory_Deallocate((void **) &(transmitStats->filename));
}

parcObject_ExtendPARCObject(RtaCommandTransmitStatistics, _rtaCommandTransmitStatistics_Destroy,
                            NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(rtaCommandTransmitStatistics, RtaCommandTransmitStatistics);

parcObject_ImplementRelease(rtaCommandTransmitStatistics, RtaCommandTransmitStatistics);

// ======= Public API

RtaCommandTransmitStatistics *
rtaCommandTransmitStatistics_Create(struct timeval period, const char *filename)
{
    assertNotNull(filename, "Filename must be non-null");

    RtaCommandTransmitStatistics *transmitStats = parcObject_CreateInstance(RtaCommandTransmitStatistics);
    memcpy(&transmitStats->period, &period, sizeof(struct timeval));
    transmitStats->filename = parcMemory_StringDuplicate(filename, PATH_MAX);

    return transmitStats;
}

struct timeval
rtaCommandTransmitStatistics_GetPeriod(const RtaCommandTransmitStatistics *transmitStats)
{
    assertNotNull(transmitStats, "Parameter transmitStats must be non-null");
    return transmitStats->period;
}

const char *
rtaCommandTransmitStatistics_GetFilename(const RtaCommandTransmitStatistics *transmitStats)
{
    assertNotNull(transmitStats, "Parameter transmitStats must be non-null");
    return transmitStats->filename;
}
