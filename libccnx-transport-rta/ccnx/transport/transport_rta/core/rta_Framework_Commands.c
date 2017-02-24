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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <errno.h>
#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_private.h>

#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/commands/rta_Command.h>

#include <parc/algol/parc_Event.h>

#ifdef DEBUG_OUTPUT
#undef DEBUG_OUTPUT
#endif

#define DEBUG_OUTPUT 0

extern FILE *GlobalStatisticsFile;

static bool _rtaFramework_ExecuteCreateStack(RtaFramework *framework, const RtaCommandCreateProtocolStack *createStack);
static bool _rtaFramework_ExecuteDestroyStack(RtaFramework *framework, const RtaCommandDestroyProtocolStack *destroyStack);
static bool _rtaFramework_ExecuteOpenConnection(RtaFramework *framework, const RtaCommandOpenConnection *openConnection);
static bool _rtaFramework_ExecuteCloseConnection(RtaFramework *framework, const RtaCommandCloseConnection *closeConnection);
static bool _rtaFramework_ExecuteTransmitStatistics(RtaFramework *framework, const RtaCommandTransmitStatistics *transmitStats);
static bool _rtaFramework_ExecuteShutdownFramework(RtaFramework *framework);

static void rtaFramework_DrainApiDescriptor(int fd);

void
rtaFramework_CommandCallback(int fd, PARCEventType what, void *user_framework)
{
    RtaFramework *framework = (RtaFramework *) user_framework;

    // flag the notifier that we are starting a batch of reads
    parcNotifier_PauseEvents(framework->commandNotifier);

    RtaCommand *command = NULL;
    while ((command = rtaCommand_Read(framework->commandRingBuffer)) != NULL) {
        // The shutdown command can broadcast a change of state before the function
        // returns, so we need to free the RtaCommand before executing the shutdown.
        // Therefore, we include the rtaCommand_Destroy() as part of the switch.

        if (rtaCommand_IsOpenConnection(command)) {
            _rtaFramework_ExecuteOpenConnection(framework, rtaCommand_GetOpenConnection(command));
            rtaCommand_Release(&command);
        } else if (rtaCommand_IsCloseConnection(command)) {
            _rtaFramework_ExecuteCloseConnection(framework, rtaCommand_GetCloseConnection(command));
            rtaCommand_Release(&command);
        } else if (rtaCommand_IsCreateProtocolStack(command)) {
            _rtaFramework_ExecuteCreateStack(framework, rtaCommand_GetCreateProtocolStack(command));
            rtaCommand_Release(&command);
        } else if (rtaCommand_IsDestroyProtocolStack(command)) {
            _rtaFramework_ExecuteDestroyStack(framework, rtaCommand_GetDestroyProtocolStack(command));
            rtaCommand_Release(&command);
        } else if (rtaCommand_IsTransmitStatistics(command)) {
            _rtaFramework_ExecuteTransmitStatistics(framework, rtaCommand_GetTransmitStatistics(command));
            rtaCommand_Release(&command);
        } else if (rtaCommand_IsShutdownFramework(command)) {
            // release the command before executing shutdown
            rtaCommand_Release(&command);
            _rtaFramework_ExecuteShutdownFramework(framework);
        } else {
            rtaCommand_Display(command, 3);
            rtaCommand_Release(&command);
            trapUnexpectedState("Got unknown command type");
        }
    }

    // resume notifications
    parcNotifier_StartEvents(framework->commandNotifier);
}

// =========================================
// Internal command processing

/**
 * Create a protocol holder and insert it in the framework's
 * protocols_head list.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static FrameworkProtocolHolder *
rtaFramework_CreateProtocolHolder(RtaFramework *framework, PARCJSON *params, uint64_t kv_hash, int stack_id)
{
    // request for a new protocol stack, create it
    FrameworkProtocolHolder *holder = parcMemory_AllocateAndClear(sizeof(FrameworkProtocolHolder));
    assertNotNull(holder, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(FrameworkProtocolHolder));

    TAILQ_INSERT_TAIL(&framework->protocols_head, holder, list);

    holder->kv_hash = kv_hash;
    holder->stack_id = stack_id;

    if (DEBUG_OUTPUT) {
        printf("%s created protocol holder %p hash %" PRIu64 "\n",
               __func__,
               (void *) holder,
               kv_hash);
    }

    return holder;
}

/**
 * Lookup the existing protocol holder for stackid.
 * Returns NULL if not found.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static FrameworkProtocolHolder *
rtaFramework_GetProtocolStackByStackId(RtaFramework *framework, int stack_id)
{
    FrameworkProtocolHolder *holder;
    TAILQ_FOREACH(holder, &framework->protocols_head, list)
    {
        if (holder->stack_id == stack_id) {
            return holder;
        }
    }
    return NULL;
}

static bool
_rtaFramework_ExecuteCreateStack(RtaFramework *framework, const RtaCommandCreateProtocolStack *createStack)
{
    // if we're in INIT mode, we need to bump
    // wait for notificaiton from event thread
    if (framework->status == FRAMEWORK_INIT) {
        rta_Framework_LockStatus(framework);
        if (framework->status == FRAMEWORK_INIT) {
            framework->status = FRAMEWORK_SETUP;
        }
        rta_Framework_BroadcastStatus(framework);
        rta_Framework_UnlockStatus(framework);
    }

    FrameworkProtocolHolder *holder =
        rtaFramework_GetProtocolStackByStackId(framework, rtaCommandCreateProtocolStack_GetStackId(createStack));
    assertNull(holder, "Found a holder with stack_id %d, but we're asked to create it!",
               rtaCommandCreateProtocolStack_GetStackId(createStack));

    uint64_t kv_hash = ccnxStackConfig_HashCode(rtaCommandCreateProtocolStack_GetStackConfig(createStack));

    // this creates it and inserts in framework->protocols_head
    holder = rtaFramework_CreateProtocolHolder(framework, NULL, kv_hash, rtaCommandCreateProtocolStack_GetStackId(createStack));

    holder->stack =
        rtaProtocolStack_Create(framework, rtaCommandCreateProtocolStack_GetConfig(createStack), rtaCommandCreateProtocolStack_GetStackId(createStack));
    rtaProtocolStack_Configure(holder->stack);

    if (DEBUG_OUTPUT) {
        printf("%s created protocol %p kv_hash %016" PRIX64 " stack_id %d\n",
               __func__, (void *) holder->stack, kv_hash, rtaCommandCreateProtocolStack_GetStackId(createStack));
    }
    return 0;
}

static bool
_rtaFramework_ExecuteOpenConnection(RtaFramework *framework, const RtaCommandOpenConnection *openConnection)
{
    int res;
    FrameworkProtocolHolder *holder;
    RtaConnection *rtaConnection;

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s framework %p\n",
               rtaFramework_GetTicks(framework), __func__, (void *) framework);
    }

    holder = rtaFramework_GetProtocolStackByStackId(framework, rtaCommandOpenConnection_GetStackId(openConnection));
    assertNotNull(holder, "Could not find stack_id %d", rtaCommandOpenConnection_GetStackId(openConnection));

    rtaConnection = rtaConnectionTable_GetByApiFd(framework->connectionTable, rtaCommandOpenConnection_GetApiNotifierFd(openConnection));
    assertNull(rtaConnection, "Found api_fd %d, but it should not exist!", rtaCommandOpenConnection_GetApiNotifierFd(openConnection));

    rtaConnection = rtaConnection_Create(holder->stack, openConnection);
    res = rtaConnectionTable_AddConnection(framework->connectionTable, rtaConnection);
    assertTrue(res == 0, "Got error from rtaConnectionTable_AddConnection: %d", res);

    res = rtaProtocolStack_Open(holder->stack, rtaConnection);
    assertTrue(res == 0, "Got error from rtaProtocolStack_Open: %d", res);

    rtaConnection_SetState(rtaConnection, CONN_OPEN);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s created connection %p stack_id %d api_fd %d transport_fd %d\n",
               rtaFramework_GetTicks(framework),
               __func__,
               (void *) rtaConnection,
               rtaCommandOpenConnection_GetStackId(openConnection),
               rtaCommandOpenConnection_GetApiNotifierFd(openConnection),
               rtaCommandOpenConnection_GetTransportNotifierFd(openConnection));
    }

    return true;
}


/**
 * Mark a connection as closed.  If there are no pending
 * packets in queues, destroy it too.
 * It's non-static because we call from rta_Framework.c
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaFramework_CloseConnection(RtaFramework *framework, RtaConnection *connection)
{
    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s connection %p api_fd %d\n",
               rtaFramework_GetTicks(framework),
               __func__, (void *) connection, rtaConnection_GetApiFd(connection));
    }

    assertFalse(rtaConnection_GetState(connection) == CONN_CLOSED,
                "connection api_fd %d is already closed", rtaConnection_GetApiFd(connection));

    rtaConnection_SetState(connection, CONN_CLOSED);
    rtaProtocolStack_Close(rtaConnection_GetStack(connection), connection);

    rtaFramework_DrainApiDescriptor(rtaConnection_GetApiFd(connection));


    // Remove it from the connection table, which will free our reference to it.

    rtaConnectionTable_Remove(framework->connectionTable, connection);

    // Done.  The rtaConnection will be removed when the last queued
    // messages for it are gone.  We keep the connection holder, so if we do
    // a Destroy we'll know about it.  RtaConnection will call
    // rtaFramework_RemoveConnection(...) when RtaConnection_Destroy() refcount
    // is zero and it's going to fully remove the connection.

    return 0;
}


static bool
_rtaFramework_ExecuteCloseConnection(RtaFramework *framework, const RtaCommandCloseConnection *closeConnection)
{
    RtaConnection *connection = rtaConnectionTable_GetByApiFd(framework->connectionTable, rtaCommandCloseConnection_GetApiNotifierFd(closeConnection));
    assertNotNull(connection, "Could not find api_fd %d", rtaCommandCloseConnection_GetApiNotifierFd(closeConnection));

    return (rtaFramework_CloseConnection(framework, connection) == 0);
}

/**
 * When the transport is closing the descriptor
 * to the API, it should call this to drain any pending but unretrieved
 * messages out of the API's side of the socket
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
rtaFramework_DrainApiDescriptor(int fd)
{
    unsigned count = 0;

    if (DEBUG_OUTPUT) {
        printf("%s fd %d\n", __func__, fd);
    }

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    // Now drain the user side of stuff they have not read
    CCNxMetaMessage *msg;
    while (read(fd, &msg, sizeof(CCNxMetaMessage *)) == sizeof(CCNxMetaMessage *)) {
        count++;
        ccnxMetaMessage_Release(&msg);
    }

    if (DEBUG_OUTPUT) {
        printf("%s destroyed %u messages\n", __func__, count);
    }
}

/**
 * This is a deferred callback from the RtaConnection when its last TransportMessage
 * has been purged from the queues.
 *
 * Don't call anything inside here that ends up back in the RtaConnection.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
rtaFramework_RemoveConnection(RtaFramework *framework, RtaConnection *rtaConnection)
{
    rtaFramework_DrainApiDescriptor(rtaConnection_GetApiFd(rtaConnection));

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s connection %p closing api_fd %d\n",
               rtaFramework_GetTicks(framework),
               __func__, (void *) rtaConnection, rtaConnection_GetApiFd(rtaConnection));
    }

    close(rtaConnection_GetApiFd(rtaConnection));
    close(rtaConnection_GetTransportFd(rtaConnection));
}

void
rtaFramework_DestroyProtocolHolder(RtaFramework *framework, FrameworkProtocolHolder *holder)
{
    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s proto_holder %p\n",
               rtaFramework_GetTicks(framework),
               __func__, (void *) holder);
    }

    // remove any and all connections associated with this protocol stack.
    // If the connections still have packets floating around in queues, the connection
    // will stay around until they all get flushed then will destroy on
    // the last packet destruction
    rtaConnectionTable_RemoveByStack(framework->connectionTable, holder->stack_id);

    rtaProtocolStack_Destroy(&holder->stack);

    TAILQ_REMOVE(&framework->protocols_head, holder, list);

    parcMemory_Deallocate((void **) &holder);
}


static bool
_rtaFramework_ExecuteDestroyStack(RtaFramework *framework, const RtaCommandDestroyProtocolStack *destroyStack)
{
    FrameworkProtocolHolder *holder = rtaFramework_GetProtocolStackByStackId(framework, rtaCommandDestroyProtocolStack_GetStackId(destroyStack));
    assertNotNull(holder, "Could not find stack_id %d", rtaCommandDestroyProtocolStack_GetStackId(destroyStack));

    rtaConnectionTable_RemoveByStack(framework->connectionTable, rtaCommandDestroyProtocolStack_GetStackId(destroyStack));

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s proto_holder %p\n",
               rtaFramework_GetTicks(framework),
               __func__, (void *) holder);
    }

    rtaFramework_DestroyProtocolHolder(framework, holder);

    return true;
}

/**
 * This will update the shared framework->status, so needs a lock around
 * the work it does.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_rtaFramework_ExecuteShutdownFramework(RtaFramework *framework)
{
    FrameworkProtocolHolder *holder;

    // %%% LOCK
    rta_Framework_LockStatus(framework);
    if (framework->status != FRAMEWORK_RUNNING) {
        RtaFrameworkStatus status = framework->status;
        rta_Framework_UnlockStatus(framework);
        // %%% UNLOCK
        assertTrue(0, "Invalid state, expected FRAMEWORK_RUNNING or later, got %d", status);
        return -1;
    }

    holder = TAILQ_FIRST(&framework->protocols_head);
    while (holder != NULL) {
        FrameworkProtocolHolder *temp = TAILQ_NEXT(holder, list);
        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s stack_id %d\n",
                   framework->clock_ticks, __func__, holder->stack_id);
        }

        rtaFramework_DestroyProtocolHolder(framework, holder);
        holder = temp;
    }

    parcEventScheduler_Stop(framework->base, &(struct timeval) { .tv_sec = 0, .tv_usec = 1000 });
    framework->status = FRAMEWORK_STOPPING;
    rta_Framework_BroadcastStatus(framework);
    rta_Framework_UnlockStatus(framework);
    // %%% UNLOCK

    return 0;
}

// Goes into rta_Framework_Commands.c
static bool
_rtaFramework_ExecuteTransmitStatistics(RtaFramework *framework, const RtaCommandTransmitStatistics *transmitStats)
{
    if (GlobalStatisticsFile != NULL) {
        fclose(GlobalStatisticsFile);
    }

    GlobalStatisticsFile = fopen(rtaCommandTransmitStatistics_GetFilename(transmitStats), "a");
    assertNotNull(GlobalStatisticsFile, "Failed to open %s", rtaCommandTransmitStatistics_GetFilename(transmitStats));

    if (GlobalStatisticsFile != NULL) {
        struct timeval period = rtaCommandTransmitStatistics_GetPeriod(transmitStats);
        parcEventTimer_Start(framework->transmit_statistics_event, &period);
    } else {
        fprintf(stderr, "Will not report statistics: Failed to open %s for output.", rtaCommandTransmitStatistics_GetFilename(transmitStats));
    }

    return 0;
}
