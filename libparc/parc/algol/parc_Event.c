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

#include <parc/assert/parc_Assert.h>

#include "internal_parc_Event.h"
#include <parc/algol/parc_EventScheduler.h>
#include <parc/algol/parc_Event.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/logging/parc_Log.h>
#include <parc/logging/parc_LogReporterFile.h>

static int _parc_event_debug_enabled = 0;

#define parcEvent_LogDebug(parcEvent, ...) \
    if (_parc_event_debug_enabled) \
        parcLog_Debug(parcEventScheduler_GetLogger(parcEvent->parcEventScheduler), __VA_ARGS__)

/**
 * Current implementation based on top of libevent2
 */
#include <event2/event.h>

/**
 * @typedef PARCEvent
 * @brief A structure containing private event state data variables
 */
struct PARCEvent {
    /**
     * The event instance.
     */
    struct event *event;

    // Event scheduler we have been queued with
    PARCEventScheduler *parcEventScheduler;

    // Interpose on callback
    PARCEvent_Callback *callback;
    void *callbackUserData;

    void *user_data;
};

static void
_parc_event_callback(evutil_socket_t fd, short flags, void *context)
{
    PARCEvent *parcEvent = (PARCEvent *) context;
    parcEvent_LogDebug(parcEvent, "_parc_event_callback(fd=%x,flags=%x,parcEvent=%p)\n", fd, flags, parcEvent);

    parcEvent->callback((int) fd, internal_libevent_type_to_PARCEventType(flags), parcEvent->callbackUserData);
}

PARCEvent *
parcEvent_Create(PARCEventScheduler *parcEventScheduler, int fd, PARCEventType flags, PARCEvent_Callback *callback, void *callbackArgs)
{
    PARCEvent *parcEvent = parcMemory_Allocate(sizeof(PARCEvent));
    parcAssertNotNull(parcEvent, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCEvent));

    parcEvent->parcEventScheduler = parcEventScheduler;
    parcEvent->callback = callback;
    parcEvent->callbackUserData = callbackArgs;

    parcEvent->event = event_new(parcEventScheduler_GetEvBase(parcEventScheduler), fd,
                                 internal_PARCEventType_to_libevent_type(flags), _parc_event_callback, parcEvent);
    parcAssertNotNull(parcEvent->event, "Could not create a new event!");

    parcEvent_LogDebug(parcEvent,
                       "parcEvent_Create(base=%p,fd=%x,events=%x,cb=%p,args=%p)\n",
                       parcEventScheduler_GetEvBase(parcEventScheduler), fd, flags, callback, parcEvent);

    return parcEvent;
}

int
parcEvent_Start(PARCEvent *parcEvent)
{
    parcEvent_LogDebug(parcEvent, "parcEvent_Start(%p)\n", parcEvent);
    parcAssertNotNull(parcEvent, "parcEvent_Start must be passed a valid event!");

    int result = event_add(parcEvent->event, NULL);
    return result;
}

int
parcEvent_Stop(PARCEvent *parcEvent)
{
    parcEvent_LogDebug(parcEvent, "parcEvent_Stop(%p)\n", parcEvent);
    parcAssertNotNull(parcEvent, "parcEvent_Stop must be passed a valid event!");

    int result = event_del(parcEvent->event);
    return result;
}

int
parcEvent_Poll(PARCEvent *parcEvent, PARCEventType event)
{
    parcEvent_LogDebug(parcEvent, "parcEvent_Stop(%p)\n", parcEvent);
    parcAssertNotNull(parcEvent, "parcEvent_Stop must be passed a valid event!");

    int result = event_pending(parcEvent->event, event, NULL);
    return result;
}

void
parcEvent_Destroy(PARCEvent **parcEvent)
{
    parcEvent_LogDebug((*parcEvent), "parcEvent_Destroy(%p)\n", *parcEvent);
    parcAssertNotNull(*parcEvent, "parcEvent_Destroy must be passed a valid parcEvent!");
    parcAssertNotNull((*parcEvent)->event, "parcEvent_Destroy passed a null event!");

    event_free((*parcEvent)->event);
    parcMemory_Deallocate((void **) parcEvent);
}

int
parcEvent_SetPriority(PARCEvent *parcEvent, PARCEventPriority priority)
{
    parcEvent_LogDebug(parcEvent, "parcEvent_Stop(%p)\n", parcEvent);

    return event_priority_set(parcEvent->event, internal_PARCEventPriority_to_libevent_priority(priority));
}

void
parcEvent_EnableDebug(void)
{
    _parc_event_debug_enabled = 1;
}

void
parcEvent_DisableDebug(void)
{
    _parc_event_debug_enabled = 0;
}
