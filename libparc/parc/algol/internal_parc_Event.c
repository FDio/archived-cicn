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
#include <parc/algol/parc_Memory.h>
#include "internal_parc_Event.h"

/**
 * Current implementation based on top of libevent2
 */
#include <event2/event.h>
#include <event2/bufferevent.h>

void *
internal_parc_alloc(size_t size)
{
    void *pointer;

    pointer = parcMemory_AllocateAndClear(size);
    return pointer;
}

void *
internal_parc_realloc(void *pointer, size_t newSize)
{
    void *newPointer;
    newPointer = parcMemory_Reallocate(pointer, newSize);
    return newPointer;
}

// We interpose on the standard "free" interface to protect against freed memory accesses.
void
internal_parc_free(void *ptr)
{
    parcMemory_Deallocate((void **) &ptr);
}

static int _libeventInitialized = 0;

void
internal_parc_initializeLibevent(void)
{
    if (_libeventInitialized) {
        return;
    }
    _libeventInitialized = 1;

    // 0x AA BB CC XX
    // AA = major
    // BB = minor
    // CC = patchlevel
    //
    uint32_t version = event_get_version_number();
    trapIllegalValueIf(version < 0x02001000UL,
                       "Libevent version must be at least 2.0.16, got %s",
                       event_get_version());

    // Make sure libevent uses our memory allocator.
    // Libevent allocates an internal object the first time a base is allocated
    // that it never releases.  In order to ensure our outstanding memory counters
    // start at zero we trigger this allocation before interposing our memory allocator.
    //
    // Create a scheduler event base, an event, then free both of them.
    //
    struct event_base *evbase = event_base_new();
    assertNotNull(evbase, "Libevent event_base_new returned NULL");
    struct event *event = event_new(evbase, -1, 0, NULL, NULL);
    assertNotNull(event, "Libevent event_new returned NULL");
    event_del(event);
    event_base_free(evbase);
    event_free(event);

    event_set_mem_functions(internal_parc_alloc,
                            internal_parc_realloc,
                            internal_parc_free);
}

PARCEventPriority
internal_libevent_priority_to_PARCEventPriority(short evpriority)
{
    PARCEventPriority priority = 0;
    switch (evpriority) {
        case 0: priority = PARCEventPriority_Maximum;
            break;
        case 1: priority = PARCEventPriority_Normal;
            break;
        case 2: priority = PARCEventPriority_Minimum;
            break;
        default:
            assertTrue(0, "Unknown Libevent priority 0x%x\n", evpriority);
            break;
    }
    return priority;
}

short
internal_PARCEventPriority_to_libevent_priority(PARCEventPriority priority)
{
    short evpriority = 0;
    switch (priority) {
        case PARCEventPriority_Maximum: evpriority = 0;
            break;
        case PARCEventPriority_Normal: evpriority = 1;
            break;
        case PARCEventPriority_Minimum: evpriority = 2;
            break;
        default:
            assertTrue(0, "Unknown PARCEventPriority 0x%x\n", evpriority);
            break;
    }
    return evpriority;
}

PARCEventSchedulerDispatchType
internal_eventloop_options_to_PARCEventSchedulerDispatchType(short evoptions)
{
    PARCEventSchedulerDispatchType options = 0;
    switch (evoptions) {
        case 0: options = PARCEventSchedulerDispatchType_Blocking;
            break;
        case EVLOOP_ONCE: options = PARCEventSchedulerDispatchType_LoopOnce;
            break;
        case EVLOOP_NONBLOCK: options = PARCEventSchedulerDispatchType_NonBlocking;
            break;
        default:
            assertTrue(0, "Unknown Libevent dispatcher flag 0x%x\n", evoptions);
            break;
    }
    return options;
}

short
internal_PARCEventSchedulerDispatchType_to_eventloop_options(PARCEventSchedulerDispatchType options)
{
    short evoptions = 0;
    switch (options) {
        case PARCEventSchedulerDispatchType_Blocking: evoptions = 0;
            break;
        case PARCEventSchedulerDispatchType_LoopOnce: evoptions = EVLOOP_ONCE;
            break;
        case PARCEventSchedulerDispatchType_NonBlocking: evoptions = EVLOOP_NONBLOCK;
            break;
        default:
            assertTrue(0, "Unknown PARCEventSchedulerDispatchType option 0x%x\n", evoptions);
            break;
    }
    return evoptions;
}

PARCEventQueueOption
internal_bufferevent_options_to_PARCEventQueueOption(short evflags)
{
    PARCEventQueueOption flags = 0;
    if (evflags & BEV_OPT_CLOSE_ON_FREE) {
        flags |= PARCEventQueueOption_CloseOnFree;
        evflags &= ~BEV_OPT_CLOSE_ON_FREE;
    }
    if (evflags & BEV_OPT_DEFER_CALLBACKS) {
        flags |= PARCEventQueueOption_DeferCallbacks;
        evflags &= ~BEV_OPT_DEFER_CALLBACKS;
    }
    assertTrue(evflags == 0, "Unknown Libevent option flag 0x%x\n", evflags);
    return flags;
}

short
internal_PARCEventQueueOption_to_bufferevent_options(PARCEventQueueOption flags)
{
    short evflags = 0;
    if (flags & PARCEventQueueOption_CloseOnFree) {
        evflags |= BEV_OPT_CLOSE_ON_FREE;
        flags &= ~PARCEventQueueOption_CloseOnFree;
    }
    if (flags & PARCEventQueueOption_DeferCallbacks) {
        evflags |= BEV_OPT_DEFER_CALLBACKS;
        flags &= ~PARCEventQueueOption_DeferCallbacks;
    }
    assertTrue(flags == 0, "Unknown PARCEventQueueOption flag 0x%x\n", flags);
    return evflags;
}

PARCEventQueueEventType
internal_bufferevent_type_to_PARCEventQueueEventType(short evtypes)
{
    PARCEventQueueEventType types = 0;
    if (evtypes & BEV_EVENT_READING) {
        types |= PARCEventQueueEventType_Reading;
        evtypes &= ~BEV_EVENT_READING;
    }
    if (evtypes & BEV_EVENT_WRITING) {
        types |= PARCEventQueueEventType_Writing;
        evtypes &= ~BEV_EVENT_WRITING;
    }
    if (evtypes & BEV_EVENT_EOF) {
        types |= PARCEventQueueEventType_EOF;
        evtypes &= ~BEV_EVENT_EOF;
    }
    if (evtypes & BEV_EVENT_ERROR) {
        types |= PARCEventQueueEventType_Error;
        evtypes &= ~BEV_EVENT_ERROR;
    }
    if (evtypes & BEV_EVENT_TIMEOUT) {
        types |= PARCEventQueueEventType_Timeout;
        evtypes &= ~BEV_EVENT_TIMEOUT;
    }
    if (evtypes & BEV_EVENT_CONNECTED) {
        types |= PARCEventQueueEventType_Connected;
        evtypes &= ~BEV_EVENT_CONNECTED;
    }
    assertTrue(evtypes == 0, "Unknown Libevent type flag 0x%x\n", evtypes);
    return types;
}

short
internal_PARCEventQueueEventType_to_bufferevent_type(PARCEventQueueEventType types)
{
    short evtypes = 0;
    if (types & PARCEventQueueEventType_Reading) {
        evtypes |= BEV_EVENT_READING;
        types &= ~PARCEventQueueEventType_Reading;
    }
    if (types & PARCEventQueueEventType_Writing) {
        evtypes |= BEV_EVENT_WRITING;
        types &= ~PARCEventQueueEventType_Writing;
    }
    if (types & PARCEventQueueEventType_EOF) {
        evtypes |= BEV_EVENT_EOF;
        types &= ~PARCEventQueueEventType_EOF;
    }
    if (types & PARCEventQueueEventType_Error) {
        evtypes |= BEV_EVENT_ERROR;
        types &= ~PARCEventQueueEventType_Error;
    }
    if (types & PARCEventQueueEventType_Timeout) {
        evtypes |= BEV_EVENT_TIMEOUT;
        types &= ~PARCEventQueueEventType_Timeout;
    }
    if (types & PARCEventQueueEventType_Connected) {
        evtypes |= BEV_EVENT_CONNECTED;
        types &= ~PARCEventQueueEventType_Connected;
    }
    assertTrue(types == 0, "Unknown PARCEventQueueEventType 0x%x\n", types);
    return evtypes;
}

PARCEventType
internal_libevent_type_to_PARCEventType(short evtypes)
{
    PARCEventType types = 0;
    if (evtypes & EV_TIMEOUT) {
        types |= PARCEventType_Timeout;
        evtypes &= ~EV_TIMEOUT;
    }
    if (evtypes & EV_READ) {
        types |= PARCEventType_Read;
        evtypes &= ~EV_READ;
    }
    if (evtypes & EV_WRITE) {
        types |= PARCEventType_Write;
        evtypes &= ~EV_WRITE;
    }
    if (evtypes & EV_SIGNAL) {
        types |= PARCEventType_Signal;
        evtypes &= ~EV_SIGNAL;
    }
    if (evtypes & EV_PERSIST) {
        types |= PARCEventType_Persist;
        evtypes &= ~EV_PERSIST;
    }
    if (evtypes & EV_ET) {
        types |= PARCEventType_EdgeTriggered;
        evtypes &= ~EV_ET;
    }
    assertTrue(evtypes == 0, "Unknown Libevent event type 0x%x\n", evtypes);
    return types;
}

short
internal_PARCEventType_to_libevent_type(PARCEventType types)
{
    short evtypes = 0;
    if (types & PARCEventType_Timeout) {
        evtypes |= EV_TIMEOUT;
        types &= ~PARCEventType_Timeout;
    }
    if (types & PARCEventType_Read) {
        evtypes |= EV_READ;
        types &= ~PARCEventType_Read;
    }
    if (types & PARCEventType_Write) {
        evtypes |= EV_WRITE;
        types &= ~PARCEventType_Write;
    }
    if (types & PARCEventType_Signal) {
        evtypes |= EV_SIGNAL;
        types &= ~PARCEventType_Signal;
    }
    if (types & PARCEventType_Persist) {
        evtypes |= EV_PERSIST;
        types &= ~PARCEventType_Persist;
    }
    if (types & PARCEventType_EdgeTriggered) {
        evtypes |= EV_ET;
        types &= ~PARCEventType_EdgeTriggered;
    }
    assertTrue(types == 0, "Unknown Libevent event type 0x%x\n", types);
    return evtypes;
}
