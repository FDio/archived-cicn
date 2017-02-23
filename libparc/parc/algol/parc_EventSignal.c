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

#include "internal_parc_Event.h"
#include <parc/algol/parc_EventScheduler.h>
#include <parc/algol/parc_EventSignal.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/logging/parc_Log.h>
#include <parc/logging/parc_LogReporterFile.h>

static int _parc_event_signal_debug_enabled = 0;

#define parcEventSignal_LogDebug(parcEventSignal, ...) \
    if (_parc_event_signal_debug_enabled) \
        parcLog_Debug(parcEventScheduler_GetLogger(parcEventSignal->eventScheduler), __VA_ARGS__)

/**
 * Current implementation based on top of libevent2
 */
#include <event2/event.h>
#include <event2/util.h>

struct PARCEventSignal {
    /**
     * The event instance.
     */
    struct event *event;

    // Event scheduler we have been queued with
    PARCEventScheduler *eventScheduler;

    PARCEventSignal_Callback *callback;
    void *callbackUserData;
};

static void
_parc_event_signal_callback(evutil_socket_t fd, short flags, void *context)
{
    PARCEventSignal *parcEventSignal = (PARCEventSignal *) context;
    parcEventSignal_LogDebug(parcEventSignal,
                             "_parc_event_signal_callback(fd=%x,flags=%x,parcEventSignal=%p)\n",
                             fd, flags, parcEventSignal);
    parcEventSignal->callback((int) fd, internal_libevent_type_to_PARCEventType(flags),
                              parcEventSignal->callbackUserData);
}

PARCEventSignal *
parcEventSignal_Create(PARCEventScheduler *eventScheduler, int signal, PARCEventType flags, PARCEvent_Callback *callback, void *callbackArgs)
{
    PARCEventSignal *parcEventSignal = parcMemory_Allocate(sizeof(PARCEventSignal));
    assertNotNull(parcEventSignal, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCEventSignal));

    parcEventSignal->eventScheduler = eventScheduler;
    parcEventSignal->callback = callback;
    parcEventSignal->callbackUserData = callbackArgs;

    parcEventSignal->event = event_new(parcEventScheduler_GetEvBase(eventScheduler), signal,
                                       internal_PARCEventType_to_libevent_type(flags),
                                       _parc_event_signal_callback, parcEventSignal);
    assertNotNull(parcEventSignal->event, "Could not create a new event!");

    parcEventSignal_LogDebug(parcEventSignal,
                             "parcEventSignal_Create(base=%p,signal=%x,flags=%x,cb=%p,args=%p) = %p\n",
                             parcEventScheduler_GetEvBase(eventScheduler), signal, flags,
                             callback, callbackArgs, parcEventSignal);
    return parcEventSignal;
}

int
parcEventSignal_Start(PARCEventSignal *parcEventSignal)
{
    parcEventSignal_LogDebug(parcEventSignal, "parcEventSignal_Start(event=%p)\n", parcEventSignal);
    assertNotNull(parcEventSignal, "parcEventStart_Signal must be passed a valid event!");

    int result = event_add(parcEventSignal->event, NULL);
    return result;
}

int
parcEventSignal_Stop(PARCEventSignal *parcEventSignal)
{
    parcEventSignal_LogDebug(parcEventSignal, "parcEventSignal_Stop(event=%p)\n", parcEventSignal);
    assertNotNull(parcEventSignal, "parcEvent_Stop must be passed a valid event!");

    int result = event_del(parcEventSignal->event);
    return result;
}

void
parcEventSignal_Destroy(PARCEventSignal **parcEventSignal)
{
    parcEventSignal_LogDebug((*parcEventSignal), "parcEventSignal_Destroy(event=%p)\n", parcEventSignal);
    assertNotNull(*parcEventSignal, "parcEvent_Destroy must be passed a valid parcEventSignal!");
    assertNotNull((*parcEventSignal)->event, "parcEvent_Destroy passed a null event!");
    event_free((*parcEventSignal)->event);
    parcMemory_Deallocate((void **) parcEventSignal);
}

void
parcEventSignal_EnableDebug(void)
{
    _parc_event_signal_debug_enabled = 1;
}

void
parcEventSignal_DisableDebug(void)
{
    _parc_event_signal_debug_enabled = 0;
}
