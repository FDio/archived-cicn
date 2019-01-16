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

#include <parc/algol/parc_EventScheduler.h>
#include <parc/algol/parc_EventSocket.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/logging/parc_Log.h>
#include <parc/logging/parc_LogReporterFile.h>

static int _parc_event_socket_debug_enabled = 0;

#define parcEventSocket_LogDebug(parcEventSocket, ...) \
    if (_parc_event_socket_debug_enabled) \
        parcLog_Debug(parcEventScheduler_GetLogger(parcEventSocket->eventScheduler), __VA_ARGS__)

/**
 * Current implementation based on top of libevent2
 */

#include <sys/errno.h>
#include <event2/listener.h>

/**
 * @typedef PARCEventSocket
 * @brief A structure containing private event state
 */
struct PARCEventSocket {
    struct evconnlistener *listener;

    // Event scheduler we have been queued with
    PARCEventScheduler *eventScheduler;

    // Interpose on EventSocket callbacks
    PARCEventSocket_Callback *socketCallback;
    void *socketUserData;
    PARCEventSocket_ErrorCallback *socketErrorCallback;
    void *socketErrorUserData;
};

static void
_parc_evconn_callback(struct evconnlistener *listener, evutil_socket_t fd,
                      struct sockaddr *address, int socklen, void *ctx)
{
    PARCEventSocket *parcEventSocket = (PARCEventSocket *) ctx;
    parcEventSocket_LogDebug(parcEventSocket, "_parc_evconn_callback(fd=%d,,parcEventSocket=%p)\n", fd, parcEventSocket);

    parcEventSocket->socketCallback((int) fd, address, socklen, parcEventSocket->socketUserData);
}

static void
_parc_evconn_error_callback(struct evconnlistener *listener, void *ctx)
{
    PARCEventSocket *parcEventSocket = (PARCEventSocket *) ctx;

    int error = EVUTIL_SOCKET_ERROR();
    char *errorString = evutil_socket_error_to_string(error);
    parcEventSocket_LogDebug(parcEventSocket,
                             "_parc_evconn_error_callback(error=%d,errorString=%s,parcEventSocket=%p)\n",
                             error, errorString, parcEventSocket);

    parcEventSocket->socketErrorCallback(parcEventSocket->eventScheduler,
                                         error, errorString, parcEventSocket->socketErrorUserData);
}

PARCEventSocket *
parcEventSocket_Create(PARCEventScheduler *eventScheduler,
                       PARCEventSocket_Callback *callback,
                       PARCEventSocket_ErrorCallback *errorCallback,
                       void *userData, const struct sockaddr *sa, int socklen)
{
    PARCEventSocket *parcEventSocket = parcMemory_AllocateAndClear(sizeof(PARCEventSocket));
    parcAssertNotNull(parcEventSocket, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCEventSocket));

    parcEventSocket->eventScheduler = eventScheduler;
    parcEventSocket->socketCallback = callback;
    parcEventSocket->socketErrorCallback = errorCallback;
    parcEventSocket->socketUserData = userData;
    parcEventSocket->socketErrorUserData = userData;
    parcEventSocket->listener = evconnlistener_new_bind(parcEventScheduler_GetEvBase(eventScheduler),
                                                        _parc_evconn_callback, parcEventSocket,
                                                        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                                        sa, socklen);
    if (parcEventSocket->listener == NULL) {
        parcLog_Error(parcEventScheduler_GetLogger(eventScheduler),
                      "Libevent evconnlistener_new_bind error (%d): %s",
                      errno, strerror(errno));
        parcEventSocket_Destroy(&parcEventSocket);
        return NULL;
    }

    if (errorCallback) {
        evconnlistener_set_error_cb(parcEventSocket->listener, _parc_evconn_error_callback);
    }
    parcEventSocket_LogDebug(parcEventSocket,
                             "parcEventSocket_Create(cb=%p,args=%p) = %p\n",
                             callback, userData, parcEventSocket);
    return parcEventSocket;
}

void
parcEventSocket_Destroy(PARCEventSocket **socketEvent)
{
    parcAssertNotNull(*socketEvent, "parcEventSocket_Destroy must be passed a valid socketEvent!");

    if ((*socketEvent)->listener) {
        evconnlistener_free((*socketEvent)->listener);
    }
    parcEventSocket_LogDebug((*socketEvent), "parcEventSocket_Destroy(%p)\n", *socketEvent);
    parcMemory_Deallocate((void **) socketEvent);
}

void
parcEventSocket_EnableDebug(void)
{
    _parc_event_socket_debug_enabled = 1;
}

void
parcEventSocket_DisableDebug(void)
{
    _parc_event_socket_debug_enabled = 0;
}
