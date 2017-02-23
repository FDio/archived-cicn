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
 * This module implements the _Create(), _Start(), and _Destroy() methods.
 * It also has various utilities for timers and events.
 *
 * The command channel is processed in rta_Framework_Commands.c.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

#include <config.h>
#include <stdio.h>
#include <unistd.h>

#include <errno.h>

#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <parc/algol/parc_EventSignal.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>
#include <ccnx/transport/transport_rta/core/rta_ConnectionTable.h>
#include <ccnx/transport/common/transport_Message.h>
#include <ccnx/transport/common/transport_private.h>

#include <ccnx/transport/transport_rta/connectors/connector_Api.h>
#include <ccnx/transport/transport_rta/connectors/connector_Forwarder.h>
#include <ccnx/transport/transport_rta/components/component_Codec.h>
#include <ccnx/transport/transport_rta/components/component_Flowcontrol.h>

#include <ccnx/transport/transport_rta/commands/rta_CommandTransmitStatistics.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

#include "rta_Framework_Commands.h"

// ===================================================

// event callbacks
static void _signal_cb(int signalNumber, PARCEventType event, void *arg);
static void _tick_cb(int, PARCEventType, void *);
static void transmitStatisticsCallback(int fd, PARCEventType what, void *user_data);


// ===========================================
// Public API (create, start, destroy)
// stop are done via the command channel
// start cannot be done via the command channel, as its not running until after start.

void
rta_Framework_LockStatus(RtaFramework *framework)
{
    int res = pthread_mutex_lock(&framework->status_mutex);
    assertTrue(res == 0, "error from pthread_mutex_lock: %d", res);
}

void
rta_Framework_UnlockStatus(RtaFramework *framework)
{
    int res = pthread_mutex_unlock(&framework->status_mutex);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

void
rta_Framework_WaitStatus(RtaFramework *framework)
{
    int res = pthread_cond_wait(&framework->status_cv, &framework->status_mutex);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

void
rta_Framework_BroadcastStatus(RtaFramework *framework)
{
    int res = pthread_cond_broadcast(&framework->status_cv);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}


/**
 * This is called whenever the connection table wants to free a connection.
 * It should call the protocol stack's closers on the connection, then
 * destroy the connection.  It is called either (a) inside the worker thread,
 * or (b) after the worker thread has stopped, so no locking needed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
rtaFramework_ConnectionTableFreeFunc(RtaConnection **connectionPtr)
{
    RtaConnection *connection;
    assertNotNull(connectionPtr, "Called with null double pointer");
    connection = *connectionPtr;
    assertNotNull(connection, "Parameter must not dereference to null");

    if (rtaConnection_GetState(connection) != CONN_CLOSED) {
        rtaFramework_CloseConnection(rtaConnection_GetFramework(connection), connection);
    }

    rtaConnection_Destroy(connectionPtr);
}

static void
_signal_cb(int signalNumber, PARCEventType event, void *arg)
{
}

static void
rtaFramework_InitializeEventScheduler(RtaFramework *framework)
{
    framework->base = parcEventScheduler_Create();
    assertNotNull(framework->base, "Could not initialize event scheduler!");

    framework->signal_pipe = parcEventSignal_Create(framework->base, SIGPIPE, PARCEventType_Signal | PARCEventType_Persist, _signal_cb, framework);
    parcEventSignal_Start(framework->signal_pipe);

    if (gettimeofday(&framework->starttime, NULL) != 0) {
        perror("Error getting time of day");
        trapUnexpectedState("Could not read gettimeofday");
    }
}

static void
rtaFramework_SetupMillisecondTimer(RtaFramework *framework)
{
    struct timeval wtnow_timeout;

    // setup a milli-second timer
    wtnow_timeout.tv_sec = 0;
    wtnow_timeout.tv_usec = 1000000 / WTHZ;

    framework->tick_event = parcEventTimer_Create(
        framework->base,
        PARCEventType_Persist,
        _tick_cb,
        (void *) framework);

    parcEventTimer_Start(framework->tick_event, &wtnow_timeout);
}

static void
rtaFramework_CreateCommandChannel(RtaFramework *framework)
{
    int fd = parcNotifier_Socket(framework->commandNotifier);

    // setup a PARCEventQueue for command_fd

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    assertFalse(flags == -1, "fcntl failed to obtain file descriptor flags (%d)", errno);
    int res = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assertTrue(res == 0, "rtaFramework_Create failed to set socket non-blocking: %s", strerror(errno));

    framework->commandEvent = parcEvent_Create(framework->base, fd, PARCEventType_Read | PARCEventType_Persist, rtaFramework_CommandCallback, (void *) framework);

    // The command port is the highest priority
    parcEvent_SetPriority(framework->commandEvent, PARCEventPriority_Maximum);

    parcEvent_Start(framework->commandEvent);

    // the notifier socket is now ready to fire
}

/*
 * Until things get plumbed from above via control messages, we will use
 * environment variables in the form "RtaFacility_facility=level" with a special facility "RtaFacility_All".
 * The "All" is processed first, then more specific facilities, so one could set all to a default
 * level then set specific ones to over-ride.
 *
 * Default log level is Error
 *
 * Strings:
 *    RtaFacility_Framework
 *    RtaFacility_Api
 *    RtaFacility_Flowcontrol
 *    RtaFacility_Codec
 *    RtaFacility_Forwarder
 */
static void
_setLogLevels(RtaFramework *framework)
{
    for (int i = 0; i < RtaLoggerFacility_END; i++) {
        rtaLogger_SetLogLevel(framework->logger, i, PARCLogLevel_Error);
    }

    char *levelString = getenv("RtaFacility_All");
    if (levelString) {
        PARCLogLevel level = parcLogLevel_FromString(levelString);
        if (level != PARCLogLevel_All) {
            for (int i = 0; i < RtaLoggerFacility_END; i++) {
                rtaLogger_SetLogLevel(framework->logger, i, level);
            }
        }
    }

    // no do specific facilities
    char buffer[1024];
    for (int i = 0; i < RtaLoggerFacility_END; i++) {
        snprintf(buffer, 1024, "RtaFacility_%s", rtaLogger_FacilityString(i));
        levelString = getenv(buffer);
        if (levelString) {
            PARCLogLevel level = parcLogLevel_FromString(levelString);
            if (level != PARCLogLevel_All) {
                rtaLogger_SetLogLevel(framework->logger, i, level);
            }
        }
    }
}

/**
 * Create a framework. This is a thread-safe function.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaFramework *
rtaFramework_Create(PARCRingBuffer1x1 *commandRingBuffer, PARCNotifier *commandNotifier)
{
    RtaFramework *framework = parcMemory_AllocateAndClear(sizeof(RtaFramework));
    assertNotNull(framework, "RtaFramework parcMemory_AllocateAndClear returned null");

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    framework->logger = rtaLogger_Create(reporter, parcClock_Monotonic());
    parcLogReporter_Release(&reporter);

    _setLogLevels(framework);

    // setup the event scheduler

    // mutes, condition variable, and protected state for starting
    // and stopping the event thread from an outside thread.
    pthread_mutex_init(&framework->status_mutex, NULL);
    pthread_cond_init(&framework->status_cv, NULL);
    framework->status = FRAMEWORK_INIT;

    framework->commandRingBuffer = parcRingBuffer1x1_Acquire(commandRingBuffer);
    framework->commandNotifier = parcNotifier_Acquire(commandNotifier);

    framework->connid_next = 1;
    TAILQ_INIT(&framework->protocols_head);

    //TODO: make 16384 configurable.
    framework->connectionTable = rtaConnectionTable_Create(16384, rtaFramework_ConnectionTableFreeFunc);
    assertNotNull(framework->connectionTable, "Could not allocate conneciton table");

    rtaFramework_InitializeEventScheduler(framework);

    rtaFramework_SetupMillisecondTimer(framework);

    framework->transmit_statistics_event = parcEventTimer_Create(framework->base,
                                                                 PARCEventType_Persist,
                                                                 transmitStatisticsCallback,
                                                                 (void *) framework);


    rtaFramework_CreateCommandChannel(framework);

    if (rtaLogger_IsLoggable(framework->logger, RtaLoggerFacility_Framework, PARCLogLevel_Info)) {
        rtaLogger_Log(framework->logger, RtaLoggerFacility_Framework, PARCLogLevel_Info, __func__,
                      "framework %p created", (void *) framework);
    }

    return framework;
}

static void
rtaFramework_DestroyEventScheduler(RtaFramework *framework)
{
    parcEventTimer_Destroy(&(framework->tick_event));
    parcEventTimer_Destroy(&(framework->transmit_statistics_event));

    if (framework->signal_int != NULL) {
        parcEventSignal_Destroy(&(framework->signal_int));
    }
    if (framework->signal_usr1 != NULL) {
        parcEventSignal_Destroy(&(framework->signal_usr1));
    }

    parcEvent_Destroy(&(framework->commandEvent));
    parcNotifier_Release(&framework->commandNotifier);
    parcRingBuffer1x1_Release(&framework->commandRingBuffer);

    parcEventSignal_Destroy(&(framework->signal_pipe));
    parcEventScheduler_Destroy(&(framework->base));
}

void
rtaFramework_Destroy(RtaFramework **frameworkPtr)
{
    RtaFramework *framework;

    assertNotNull(frameworkPtr, "Parameter must be non-null RtaFramework double pointer");
    framework = *frameworkPtr;
    assertNotNull(framework, "Parameter must dereference to non-Null RtaFramework pointer");

    rtaLogger_Log(framework->logger, RtaLoggerFacility_Framework, PARCLogLevel_Info, __func__,
                  "framework %p destroy", (void *) framework);

    // status can be STOPPED or INIT.  It's ok to destroy one that's never been started.

    // %%%% LOCK
    rta_Framework_LockStatus(framework);
    assertTrue(framework->status == FRAMEWORK_SHUTDOWN ||
               framework->status == FRAMEWORK_INIT ||
               framework->status == FRAMEWORK_TEARDOWN,
               "Framework invalid state, got %d",
               framework->status);
    rta_Framework_UnlockStatus(framework);
    // %%%% UNLOCK

    rtaConnectionTable_Destroy(&framework->connectionTable);

    rtaFramework_DestroyEventScheduler(framework);

    rtaLogger_Release(&framework->logger);

    parcMemory_Deallocate((void **) &framework);

    *frameworkPtr = NULL;
}

RtaLogger *
rtaFramework_GetLogger(RtaFramework *framework)
{
    return framework->logger;
}

/**
 * May block briefly, returns the current status of the framework.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaFrameworkStatus
rtaFramework_GetStatus(RtaFramework *framework)
{
    RtaFrameworkStatus status;
    // %%%% LOCK
    rta_Framework_LockStatus(framework);
    status = framework->status;
    rta_Framework_UnlockStatus(framework);
    // %%%% UNLOCK
    return status;
}

/**
 * Blocks until the framework status equals or exeeds the desired status
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaFrameworkStatus
rtaFramework_WaitForStatus(RtaFramework *framework,
                           RtaFrameworkStatus status)
{
    // %%%% LOCK
    rta_Framework_LockStatus(framework);
    while (framework->status < status) {
        rta_Framework_WaitStatus(framework);
    }
    rta_Framework_UnlockStatus(framework);
    // %%%% UNLOCK

    return status;
}

// =================================================================

// Transport Operations

PARCEventScheduler *
rtaFramework_GetEventScheduler(RtaFramework *framework)
{
    assertNotNull(framework, "Parameter must be non-NULL RtaFramework");
    return framework->base;
}

unsigned
rtaFramework_GetNextConnectionId(RtaFramework *framework)
{
    assertNotNull(framework, "Parameter must be non-NULL RtaFramework");

    return framework->connid_next++;
}

// ============================
// Internal functions

/*
 * This is dispatched from the event loop, so its a loosely accurate time
 */
static void
_tick_cb(int fd, PARCEventType what, void *user_data)
{
    RtaFramework *framework = (RtaFramework *) user_data;
    assertTrue(what & PARCEventType_Timeout, "%s got unknown signal %d", __func__, what);
    framework->clock_ticks++;

    if (framework->killme) {
        int res;

        if (rtaLogger_IsLoggable(framework->logger, RtaLoggerFacility_Framework, PARCLogLevel_Debug)) {
            rtaLogger_Log(framework->logger, RtaLoggerFacility_Framework, PARCLogLevel_Debug, __func__,
                          "framework %p exiting base loop", (void *) framework);
        }

        res = parcEventScheduler_Abort(framework->base);
        assertTrue(res == 0, "error on parcEventScheduler_Abort: %d", res);
    }
}

FILE *GlobalStatisticsFile = NULL;

static void
transmitStatisticsCallback(int fd, PARCEventType what, void *user_data)
{
    RtaFramework *framework = (RtaFramework *) user_data;
    assertTrue(what & PARCEventType_Timeout, "unknown signal %d", what);

    FrameworkProtocolHolder *holder;
    TAILQ_FOREACH(holder, &framework->protocols_head, list)
    {
        RtaProtocolStack *stack = holder->stack;
        PARCArrayList *list = rtaProtocolStack_GetStatistics(stack, GlobalStatisticsFile);
        parcArrayList_Destroy(&list);
    }
}
