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
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>

#include <ccnx/transport/common/transport.h>
#include <ccnx/transport/common/transport_private.h>
#include <ccnx/transport/common/transport_Message.h>

#define DEBUG_OUTPUT 0

struct transport_message {
    CCNxTlvDictionary *dictionary;
    TransportMessage_Free *freefunc;
    void *info;

    struct timeval creationTime;
};

static size_t _transport_messages_created = 0;
static size_t _transport_messages_destroyed = 0;

static void
_transportMessage_GetTimeOfDay(struct timeval *outputTime)
{
#ifdef DEBUG
    // if in debug mode, time messages
    gettimeofday(outputTime, NULL);
#else
    *outputTime = (struct timeval) { 0, 0 };
#endif
}

TransportMessage *
transportMessage_CreateFromDictionary(CCNxTlvDictionary *dictionary)
{
    assertNotNull(dictionary, "Cannot create TransportMessage from NULL Dictionary");
    if (dictionary == NULL) {
        return NULL;
    }

    TransportMessage *tm = parcMemory_AllocateAndClear(sizeof(TransportMessage));

    if (tm != NULL) {
        tm->dictionary = ccnxTlvDictionary_Acquire(dictionary);

        _transportMessage_GetTimeOfDay(&tm->creationTime);

        _transport_messages_created++;

        if (DEBUG_OUTPUT) {
            printf("%-35s allocs %zu destroys %zu pointer %p dict   %p\n",
                   __func__,
                   _transport_messages_created,
                   _transport_messages_destroyed,
                   (void *) tm,
                   (void *) dictionary);
        }
    }

    return tm;
}

CCNxTlvDictionary *
transportMessage_GetDictionary(TransportMessage *tm)
{
    assertNotNull(tm, "TransportMessage_GetWireMessage called on NULL transport message");
    return tm->dictionary;
}

bool
transportMessage_isValid(const TransportMessage *message)
{
    bool result = false;
    if (message != NULL) {
        result = true;
    }

    return result;
}

void
transportMessage_AssertValid(const TransportMessage *message)
{
    assertTrue(transportMessage_isValid(message), "TransportMessage @ %p is invalid.", (void *) message);
}

/*
 * Frees the TransportMessage wrapper, but user is responsible
 * for destroying the inner pieces.
 */
static void
_transportMessage_Destroy(TransportMessage **msgPtr)
{
    assertNotNull(msgPtr, "TransportMessage_Destroy called on NULL transport message pointer");
    if (msgPtr != NULL) {
        TransportMessage *msg = *msgPtr;
        transportMessage_OptionalAssertValid(msg);

        _transport_messages_destroyed++;

        if (DEBUG_OUTPUT) {
            printf("%-35s allocs %zu destroys %zu pointer %p\n",
                   __func__,
                   _transport_messages_created,
                   _transport_messages_destroyed,
                   (void *) msg);
        }

        if (msg->freefunc != NULL) {
            msg->freefunc(&msg->info);
        }

        parcMemory_Deallocate((void **) &msg);
        *msgPtr = NULL;
    }
}

void
transportMessage_Destroy(TransportMessage **tmPtr)
{
    TransportMessage *tm = *tmPtr;
    assertNotNull(tmPtr, "called with NULL transport message double pointer");

    assertNotNull(tm, "called with NULL transport message dereference");

    if (tm->dictionary != NULL) {
        ccnxTlvDictionary_Release(&tm->dictionary);
        tm->dictionary = NULL;
    }

    _transportMessage_Destroy(tmPtr);
}

/*
 * Add some stack payload to a transport message.  Will not be freed.
 */
void
transportMessage_SetInfo(TransportMessage *tm, void *info, TransportMessage_Free *freefunc)
{
    assertNotNull(tm, "%s called with NULL transport message", __func__);
    tm->info = info;
    tm->freefunc = freefunc;
}

void *
transportMessage_GetInfo(const TransportMessage *tm)
{
    assertNotNull(tm, "%s called with NULL transport message", __func__);
    return tm->info;
}


struct timeval
transportMessage_GetDelay(const TransportMessage *tm)
{
    struct timeval now;
    _transportMessage_GetTimeOfDay(&now);
    timersub(&now, &tm->creationTime, &now);
    return now;
}

bool
transportMessage_IsControl(const TransportMessage *tm)
{
    return ccnxTlvDictionary_IsControl(tm->dictionary);
}

bool
transportMessage_IsInterest(const TransportMessage *tm)
{
    return ccnxTlvDictionary_IsInterest(tm->dictionary);
}

bool
transportMessage_IsContentObject(const TransportMessage *tm)
{
    return ccnxTlvDictionary_IsContentObject(tm->dictionary);
}
