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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <LongBow/longBow_Event.h>
#include <LongBow/longBow_EventType.h>
#include <LongBow/longBow_Backtrace.h>
#include <LongBow/longBow_Location.h>
#include <LongBow/longBow_Status.h>
#include <LongBow/private/longBow_Memory.h>

/** @cond private */
struct longbow_event {
    const LongBowEventType *type;
    const LongBowLocation *location;
    const char *kind;
    const char *message;
    const LongBowBacktrace *backtrace;
};
/** @endcond */

const char *
longBowEvent_GetName(const LongBowEvent *event)
{
    return longBowEventType_GetName(event->type);
}

LongBowEvent *
longBowEvent_Create(const LongBowEventType *eventType, const LongBowLocation *location, const char *kind, const char *message, const LongBowBacktrace *backtrace)
{
    LongBowEvent *result = longBowMemory_Allocate(sizeof(LongBowEvent));
    if (result != NULL) {
        result->type = eventType;
        result->location = location;
        result->kind = kind;
        result->message = strndup(message, strlen(message));
        result->backtrace = backtrace;
    }

    return result;
}

void
longBowEvent_Destroy(LongBowEvent **assertionPtr)
{
    LongBowEvent *assertion = *assertionPtr;

    if (assertion->location != NULL) {
        longBowLocation_Destroy((LongBowLocation **) &assertion->location);
    }
    if (assertion->message != NULL) {
        free((void *) assertion->message);
    }

    if (assertion->backtrace != NULL) {
        longBowBacktrace_Destroy((LongBowBacktrace **) &assertion->backtrace);
    }
    longBowMemory_Deallocate((void **) assertionPtr);
}

const LongBowLocation *
longBowEvent_GetLocation(const LongBowEvent *event)
{
    return event->location;
}

const LongBowEventType *
longBowEvent_GetEventType(const LongBowEvent *event)
{
    return event->type;
}

const char *
longBowEvent_GetKind(const LongBowEvent *event)
{
    return event->kind;
}

const char *
longBowEvent_GetMessage(const LongBowEvent *event)
{
    return event->message;
}

const LongBowBacktrace *
longBowEvent_GetBacktrace(const LongBowEvent *event)
{
    return event->backtrace;
}

char **
longBowEvent_CreateSymbolicCallstack(const LongBowEvent *event)
{
    char **result = longBowBacktrace_Symbols(event->backtrace);

    return result;
}

size_t
longBowEvent_GetCallStackLength(const LongBowEvent *event)
{
    return longBowBacktrace_GetFrameCount(event->backtrace);
}
