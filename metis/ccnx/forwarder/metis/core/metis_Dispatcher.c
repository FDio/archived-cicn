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
 * @header metis_Dispatch.c
 * @abstract Event dispatcher for Metis.  Uses parcEvent
 * @discussion
 *     Wraps the functions we use in parcEvent, along with mets_StreamBuffer and metis_Message.
 *     The dispatcher is the event loop, so it manages things like signals, timers, and network events.
 *
 */

#include <config.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <parc/algol/parc_EventTimer.h>
#include <parc/algol/parc_EventQueue.h>

#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>

#ifndef INPORT_ANY
#define INPORT_ANY 0
#endif

struct metis_dispatcher {
    PARCEventScheduler *Base;
    MetisLogger *logger;
};

// ==========================================
// Public API

PARCEventScheduler *
metisDispatcher_GetEventScheduler(MetisDispatcher *dispatcher)
{
    return dispatcher->Base;
}

MetisDispatcher *
metisDispatcher_Create(MetisLogger *logger)
{
    MetisDispatcher *dispatcher = parcMemory_AllocateAndClear(sizeof(MetisDispatcher));
    assertNotNull(dispatcher, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisDispatcher));

    dispatcher->Base = parcEventScheduler_Create();
    dispatcher->logger = metisLogger_Acquire(logger);

    assertNotNull(dispatcher->Base, "Got NULL from parcEventScheduler_Create()");

    return dispatcher;
}

void
metisDispatcher_Destroy(MetisDispatcher **dispatcherPtr)
{
    assertNotNull(dispatcherPtr, "Parameter must be non-null double pointer");
    assertNotNull(*dispatcherPtr, "Parameter must dereference to non-null pointer");
    MetisDispatcher *dispatcher = *dispatcherPtr;

    metisLogger_Release(&dispatcher->logger);
    parcEventScheduler_Destroy(&(dispatcher->Base));
    parcMemory_Deallocate((void **) &dispatcher);
    *dispatcherPtr = NULL;
}

void
metisDispatcher_Stop(MetisDispatcher *dispatcher)
{
    struct timeval delay = { 0, 1000 };

    parcEventScheduler_Stop(dispatcher->Base, &delay);
}

void
metisDispatcher_Run(MetisDispatcher *dispatcher)
{
    assertNotNull(dispatcher, "Parameter must be non-null");

    parcEventScheduler_Start(dispatcher->Base, 0);
}


void
metisDispatcher_RunDuration(MetisDispatcher *dispatcher, struct timeval *duration)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(duration, "Parameter duration must be non-null");

    parcEventScheduler_Stop(dispatcher->Base, duration);
    parcEventScheduler_Start(dispatcher->Base, 0);
}


void
metisDispatcher_RunCount(MetisDispatcher *dispatcher, unsigned count)
{
    assertNotNull(dispatcher, "Parameter must be non-null");

    for (unsigned i = 0; i < count; i++) {
        parcEventScheduler_Start(dispatcher->Base, PARCEventSchedulerDispatchType_LoopOnce);
    }
}

PARCEventSocket *
metisDispatcher_CreateListener(MetisDispatcher *dispatcher, PARCEventSocket_Callback *callback, void *user_data, int backlog, const struct sockaddr *sa, int socklen)
{
    PARCEventSocket *listener = parcEventSocket_Create(dispatcher->Base, callback, NULL, user_data, sa, socklen);
    if (listener == NULL) {
        perror("Problem creating listener");
    }
    return listener;
}

void
metisDispatcher_DestroyListener(MetisDispatcher *dispatcher, PARCEventSocket **listenerPtr)
{
    assertNotNull(listenerPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listenerPtr, "Parameter must dereference to non-null pointer");
    parcEventSocket_Destroy(listenerPtr);
}

PARCEventQueue *
metisDispatcher_CreateStreamBufferFromSocket(MetisDispatcher *dispatcher, MetisSocketType fd)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    PARCEventQueue *buffer = parcEventQueue_Create(dispatcher->Base, fd, PARCEventQueueOption_CloseOnFree | PARCEventQueueOption_DeferCallbacks);
    assertNotNull(buffer, "Got null from parcEventBufver_Create for socket %d", fd);
    return buffer;
}

PARCEventTimer *
metisDispatcher_CreateTimer(MetisDispatcher *dispatcher, bool isPeriodic, PARCEvent_Callback *callback, void *userData)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(callback, "Parameter callback must be non-null");

    PARCEventType flags = 0;
    if (isPeriodic) {
        flags |= PARCEventType_Persist;
    }
    PARCEventTimer *event = parcEventTimer_Create(dispatcher->Base, flags, callback, userData);
    return event;
}

void
metisDispatcher_StartTimer(MetisDispatcher *dispatcher, PARCEventTimer *timerEvent, struct timeval *delay)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(timerEvent, "Parameter timerEvent must be non-null");
    int failure = parcEventTimer_Start(timerEvent, delay);
    assertFalse(failure < 0, "Error starting timer event %p: (%d) %s", (void *) timerEvent, errno, strerror(errno));
}

void
metisDispatcher_StopTimer(MetisDispatcher *dispatcher, PARCEventTimer *event)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(event, "Parameter event must be non-null");

    int failure = parcEventTimer_Stop(event);
    assertFalse(failure < 0, "Error stopping signal event %p: (%d) %s", (void *) event, errno, strerror(errno));
}

void
metisDispatcher_DestroyTimerEvent(MetisDispatcher *dispatcher, PARCEventTimer **eventPtr)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(eventPtr, "Parameter eventPtr must be non-null double pointer");
    assertNotNull(*eventPtr, "Paramter eventPtr must dereference to non-null pointer");

    parcEventTimer_Destroy(eventPtr);
    eventPtr = NULL;
}

PARCEvent *
metisDispatcher_CreateNetworkEvent(MetisDispatcher *dispatcher, bool isPersistent, PARCEvent_Callback *callback, void *userData, int fd)
{
    short flags = PARCEventType_Timeout | PARCEventType_Read;
    if (isPersistent) {
        flags |= PARCEventType_Persist;
    }

    PARCEvent *event = parcEvent_Create(dispatcher->Base, fd, flags, callback, userData);
    assertNotNull(event, "Got null from parcEvent_Create for socket %d", fd);
    return event;
}

void
metisDispatcher_DestroyNetworkEvent(MetisDispatcher *dispatcher, PARCEvent **eventPtr)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(eventPtr, "Parameter eventPtr must be non-null double pointer");
    assertNotNull(*eventPtr, "Paramter eventPtr must dereference to non-null pointer");

    parcEvent_Destroy(eventPtr);
    eventPtr = NULL;
}

void
metisDispatcher_StartNetworkEvent(MetisDispatcher *dispatcher, PARCEvent *event)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(event, "Parameter event must be non-null");

    int failure = parcEvent_Start(event);
    assertFalse(failure < 0, "Error starting signal event %p: (%d) %s", (void *) event, errno, strerror(errno));
}

void
metisDispatcher_StopNetworkEvent(MetisDispatcher *dispatcher, PARCEvent *event)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(event, "Parameter event must be non-null");

    int failure = parcEvent_Stop(event);
    assertFalse(failure < 0, "Error stopping signal event %p: (%d) %s", (void *) event, errno, strerror(errno));
}

PARCEventSignal *
metisDispatcher_CreateSignalEvent(MetisDispatcher *dispatcher, PARCEventSignal_Callback *callback, void *userData, int signal)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(callback, "Parameter callback must be non-null");

    PARCEventSignal *event = parcEventSignal_Create(dispatcher->Base, signal, PARCEventType_Signal | PARCEventType_Persist, callback, userData);
    assertNotNull(event, "Got null event when creating signal catcher for signal %d", signal);

    return event;
}

void
metisDispatcher_DestroySignalEvent(MetisDispatcher *dispatcher, PARCEventSignal **eventPtr)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(eventPtr, "Parameter eventPtr must be non-null double pointer");
    assertNotNull(*eventPtr, "Paramter eventPtr must dereference to non-null pointer");

    parcEventSignal_Destroy(eventPtr);
    eventPtr = NULL;
}

void
metisDispatcher_StartSignalEvent(MetisDispatcher *dispatcher, PARCEventSignal *event)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(event, "Parameter event must be non-null");

    int failure = parcEventSignal_Start(event);
    assertFalse(failure < 0, "Error starting signal event %p: (%d) %s", (void *) event, errno, strerror(errno));
}

void
metisDispatcher_StopSignalEvent(MetisDispatcher *dispatcher, PARCEventSignal *event)
{
    assertNotNull(dispatcher, "Parameter dispatcher must be non-null");
    assertNotNull(event, "Parameter event must be non-null");

    int failure = parcEventSignal_Stop(event);
    assertFalse(failure < 0, "Error stopping signal event %p: (%d) %s", (void *) event, errno, strerror(errno));
}

/**
 * Bind to a local address/port then connect to peer.
 */
static bool
metisDispatcher_StreamBufferBindAndConnect(MetisDispatcher *dispatcher, PARCEventQueue *buffer,
                                           struct sockaddr *localSock, socklen_t localSockLength,
                                           struct sockaddr *remoteSock, socklen_t remoteSockLength)
{
    // we need to bind, then connect.  Special operation, so we make our
    // own fd then pass it off to the buffer event

    int fd = socket(localSock->sa_family, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    if (flags < 0) {
        perror("F_GETFL");
        close(fd);
        return -1;
    }
    int failure = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (failure) {
        perror("F_SETFL");
        close(fd);
        return -1;
    }

    failure = bind(fd, localSock, localSockLength);
    if (failure) {
        perror("bind");
        close(fd);
        return false;
    }

    parcEventQueue_SetFileDescriptor(buffer, fd);

    failure = parcEventQueue_ConnectSocket(buffer, remoteSock, remoteSockLength);
    if (failure && (errno != EINPROGRESS)) {
        perror("connect");
        close(fd);
        return false;
    }
    return true;
}

/**
 * Connect to an INET peer
 * @return NULL on error, otherwise a streambuffer
 */
static PARCEventQueue *
metisDispatcher_StreamBufferConnect_INET(MetisDispatcher *dispatcher, const CPIAddress *localAddress, const CPIAddress *remoteAddress)
{
    struct sockaddr_in localSock, remoteSock;
    cpiAddress_GetInet(localAddress, &localSock);
    cpiAddress_GetInet(remoteAddress, &remoteSock);

    PARCEventQueue *buffer = parcEventQueue_Create(dispatcher->Base, -1, PARCEventQueueOption_CloseOnFree);
    assertNotNull(buffer, "got null buffer from parcEventQueue_Create()");

    bool success = metisDispatcher_StreamBufferBindAndConnect(dispatcher, buffer,
                                                              (struct sockaddr *) &localSock, sizeof(localSock),
                                                              (struct sockaddr *) &remoteSock, sizeof(remoteSock));
    if (!success) {
        parcEventQueue_Destroy(&buffer);
        buffer = NULL;
    }

    return buffer;
}

/**
 * Connect to an INET peer
 * @return NULL on error, otherwise a streambuffer
 */
static PARCEventQueue *
//static MetisStreamBuffer *
metisDispatcher_StreamBufferConnect_INET6(MetisDispatcher *dispatcher, const CPIAddress *localAddress, const CPIAddress *remoteAddress)
{
    struct sockaddr_in6 localSock, remoteSock;
    cpiAddress_GetInet6(localAddress, &localSock);
    cpiAddress_GetInet6(remoteAddress, &remoteSock);

    PARCEventQueue *buffer = parcEventQueue_Create(dispatcher->Base, -1, PARCEventQueueOption_CloseOnFree);
    assertNotNull(buffer, "got null buffer from parcEventQueue_Create()");

    bool success = metisDispatcher_StreamBufferBindAndConnect(dispatcher, buffer,
                                                              (struct sockaddr *) &localSock, sizeof(localSock),
                                                              (struct sockaddr *) &remoteSock, sizeof(remoteSock));
    if (!success) {
        parcEventQueue_Destroy(&buffer);
        buffer = NULL;
    }

    return buffer;
}

PARCEventQueue *
metisDispatcher_StreamBufferConnect(MetisDispatcher *dispatcher, const MetisAddressPair *pair)
{
    const CPIAddress *localAddress = metisAddressPair_GetLocal(pair);
    const CPIAddress *remoteAddress = metisAddressPair_GetRemote(pair);


    // they must be of the same address family
    if (cpiAddress_GetType(localAddress) != cpiAddress_GetType(remoteAddress)) {
        char message[2048];
        char *localAddressString = cpiAddress_ToString(localAddress);
        char *remoteAddressString = cpiAddress_ToString(remoteAddress);
        snprintf(message,
                 2048,
                 "Remote address not same type as local address, expected %d got %d\nlocal %s remote %s",
                 cpiAddress_GetType(localAddress),
                 cpiAddress_GetType(remoteAddress),
                 localAddressString,
                 remoteAddressString);

        parcMemory_Deallocate((void **) &localAddressString);
        parcMemory_Deallocate((void **) &remoteAddressString);

        assertTrue(cpiAddress_GetType(localAddress) == cpiAddress_GetType(remoteAddress), "%s", message);
    }

    switch (cpiAddress_GetType(localAddress)) {
        case cpiAddressType_INET:
            return metisDispatcher_StreamBufferConnect_INET(dispatcher, localAddress, remoteAddress);
            break;
        case cpiAddressType_INET6:
            return metisDispatcher_StreamBufferConnect_INET6(dispatcher, localAddress, remoteAddress);
            break;
        default:
            trapIllegalValue(pair, "local address unsupported CPI address type: %d", cpiAddress_GetType(localAddress));
    }
}
