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
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>
#include <unistd.h>

#include "internal_parc_Event.h"
#include <parc/algol/parc_EventScheduler.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/logging/parc_Log.h>
#include <parc/logging/parc_LogReporterFile.h>

/**
 * Current implementation based on top of libevent2
 */
#include <event2/event.h>

static int _parc_event_scheduler_debug_enabled = 0;

#define parcEventScheduler_LogDebug(parcEventScheduler, ...) \
    if (_parc_event_scheduler_debug_enabled) \
        parcLog_Debug(parcEventScheduler->log, __VA_ARGS__)

struct PARCEventScheduler {
    /**
     * Base of the libevent manager.
     */
    struct event_base *evbase;
    PARCLog *log;
};

static PARCLog *
_parc_logger_create(void)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(output);
    parcOutputStream_Release(&output);

    PARCLog *log = parcLog_Create("localhost", "test_parc_Log", NULL, reporter);
    parcLogReporter_Release(&reporter);

    parcLog_SetLevel(log, PARCLogLevel_All);
    return log;
}

void *
parcEventScheduler_GetEvBase(PARCEventScheduler *parcEventScheduler)
{
    return (void *) parcEventScheduler->evbase;
}

PARCEventScheduler *
parcEventScheduler_Create(void)
{
    internal_parc_initializeLibevent();

    PARCEventScheduler *parcEventScheduler = parcMemory_Allocate(sizeof(PARCEventScheduler));
    assertNotNull(parcEventScheduler, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCEventScheduler));

    // Initialize libevent base pointer.
    parcEventScheduler->evbase = event_base_new();

    assertNotNull(parcEventScheduler->evbase, "Could not obtain an event base!");
    int result = event_base_priority_init(parcEventScheduler->evbase, PARCEventPriority_NumberOfPriorities);
    assertTrue(result == 0, "Could not set scheduler priorities (%d)", result);

    parcEventScheduler->log = _parc_logger_create();
    assertNotNull(parcEventScheduler->log, "Could not create parc logger");

    parcEventScheduler_LogDebug(parcEventScheduler, "parcEventScheduler_Create() = %p\n", parcEventScheduler);

    return parcEventScheduler;
}

int
parcEventScheduler_Start(PARCEventScheduler *parcEventScheduler, PARCEventSchedulerDispatchType type)
{
    parcEventScheduler_LogDebug(parcEventScheduler, "parcEventScheduler_Start(%p, %d)\n", parcEventScheduler, type);
    assertNotNull(parcEventScheduler, "parcEventScheduler_Start must be passed a valid base parcEventScheduler!");
    int result = event_base_loop(parcEventScheduler->evbase,
                                 internal_PARCEventSchedulerDispatchType_to_eventloop_options(type));
    return result;
}

int
parcEventScheduler_DispatchBlocking(PARCEventScheduler *parcEventScheduler)
{
    return parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
}

int
parcEventScheduler_DispatchNonBlocking(PARCEventScheduler *parcEventScheduler)
{
    return parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_NonBlocking);
}

int
parcEventScheduler_Stop(PARCEventScheduler *parcEventScheduler, struct timeval *delay)
{
    parcEventScheduler_LogDebug(parcEventScheduler, "parcEventScheduler_Stop(%p, %p)\n", parcEventScheduler, delay);
    assertNotNull(parcEventScheduler, "parcEventScheduler_Stop must be passed a valid base parcEventScheduler!");
    int result = event_base_loopexit(parcEventScheduler->evbase, delay);
    return result;
}

int
parcEventScheduler_Abort(PARCEventScheduler *parcEventScheduler)
{
    parcEventScheduler_LogDebug(parcEventScheduler, "parcEventScheduler_Abort(%p)\n", parcEventScheduler);
    assertNotNull(parcEventScheduler, "parcEventScheduler_Abort must be passed a valid base parcEventScheduler!");
    int result = event_base_loopbreak(parcEventScheduler->evbase);
    return result;
}

static int _event_enable_debug_mode_called = 0;

void
parcEventScheduler_EnableDebug(void)
{
    _parc_event_scheduler_debug_enabled = 1;
    if (_event_enable_debug_mode_called == 0) {
        event_enable_debug_mode();
        _event_enable_debug_mode_called = 1;
    }
}

void
parcEventScheduler_DisableDebug(void)
{
    _parc_event_scheduler_debug_enabled = 0;
}

void
parcEventScheduler_Destroy(PARCEventScheduler **parcEventScheduler)
{
    parcEventScheduler_LogDebug((*parcEventScheduler), "parcEventScheduler_Destroy(%p)\n", *parcEventScheduler);

    assertNotNull(*parcEventScheduler, "parcEventScheduler_Destroy must be passed a valid base parcEventScheduler!");
    assertNotNull((*parcEventScheduler)->evbase, "parcEventScheduler_Destroy passed a NULL event base member!");

    event_base_free((*parcEventScheduler)->evbase);
    parcLog_Release(&((*parcEventScheduler)->log));
    parcMemory_Deallocate((void **) parcEventScheduler);
}

PARCLog *
parcEventScheduler_GetLogger(PARCEventScheduler *parcEventScheduler)
{
    return parcEventScheduler->log;
}
