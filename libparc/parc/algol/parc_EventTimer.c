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
#include <parc/algol/parc_EventTimer.h>

static int _parc_event_timer_debug_enabled = 0;

#define parcEventTimer_LogDebug(parcEventTimer, ...) \
    if (_parc_event_timer_debug_enabled) \
        parcLog_Debug(parcEventScheduler_GetLogger(parcEventTimer->eventScheduler), __VA_ARGS__)

/**
 * Current implementation based on top of libevent2
 */
#include <event2/event.h>
#include <event2/util.h>

struct PARCEventTimer {
    /**
     * The event instance.
     */
    struct event *event;

    // Event scheduler we have been queued with
    PARCEventScheduler *eventScheduler;

    PARCEventTimer_Callback *callback;
    void *callbackUserData;
};

static void
_parc_event_timer_callback(evutil_socket_t fd, short flags, void *context)
{
    PARCEventTimer *parcEventTimer = (PARCEventTimer *) context;
    parcEventTimer_LogDebug(parcEventTimer,
                            "_parc_event_timer_callback(fd=%x,flags=%x,parcEventTimer=%p)\n",
                            fd, flags, parcEventTimer);
    parcEventTimer->callback((int) fd, internal_libevent_type_to_PARCEventType(flags),
                             parcEventTimer->callbackUserData);
}

PARCEventTimer *
parcEventTimer_Create(PARCEventScheduler *eventScheduler, PARCEventType flags, PARCEvent_Callback *callback, void *callbackArgs)
{
    PARCEventTimer *parcEventTimer = parcMemory_Allocate(sizeof(PARCEventTimer));
    parcAssertNotNull(parcEventTimer, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCEventTimer));

    parcEventTimer->eventScheduler = eventScheduler;
    parcEventTimer->callback = callback;
    parcEventTimer->callbackUserData = callbackArgs;

    // NB: the EV_TIMEOUT flag is ignored when constructing an event
    parcEventTimer->event = event_new(parcEventScheduler_GetEvBase(eventScheduler), -1,
                                      internal_PARCEventType_to_libevent_type(flags),
                                      _parc_event_timer_callback, parcEventTimer);
    parcAssertNotNull(parcEventTimer->event, "Could not create a new event!");

    parcEventTimer_LogDebug(parcEventTimer,
                            "parcEventTimer_Create(base=%p,events=%x,cb=%p,args=%p) = %p\n",
                            parcEventScheduler_GetEvBase(eventScheduler), flags,
                            callback, callbackArgs, parcEventTimer);

    return parcEventTimer;
}

int
parcEventTimer_Start(PARCEventTimer *parcEventTimer, struct timeval *timeout)
{
    parcEventTimer_LogDebug(parcEventTimer,
                            "parcEventTimer_Start(event=%p, timeout=%d:%d)\n",
                            parcEventTimer, timeout->tv_sec, timeout->tv_usec);
    parcAssertNotNull(parcEventTimer, "parcEventTimer_Start must be passed a valid event!");

    int result = event_add(parcEventTimer->event, timeout);
    return result;
}

int
parcEventTimer_Stop(PARCEventTimer *parcEventTimer)
{
    parcEventTimer_LogDebug(parcEventTimer, "parcEventTimer_Stop(event=%p)\n", parcEventTimer);
    parcAssertNotNull(parcEventTimer, "parcEventTimer_Stop must be passed a valid event!");

    int result = event_del(parcEventTimer->event);
    return result;
}

void
parcEventTimer_Destroy(PARCEventTimer **parcEventTimer)
{
    parcEventTimer_LogDebug((*parcEventTimer), "parcEventTimer_Destroy(parcEventTimer=%p)\n", *parcEventTimer);
    parcAssertNotNull(*parcEventTimer, "parcEventTimer_Destroy must be passed a valid parcEventTimer!");
    parcAssertNotNull((*parcEventTimer)->event, "parcEventTimer_Destroy passed a null event!");

    event_free((*parcEventTimer)->event);
    parcMemory_Deallocate((void **) parcEventTimer);
}

void
parcEventTimer_EnableDebug(void)
{
    _parc_event_timer_debug_enabled = 1;
}

void
parcEventTimer_DisableDebug(void)
{
    _parc_event_timer_debug_enabled = 0;
}
