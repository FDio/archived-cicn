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
#include <stdlib.h>
#include <unistd.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <ccnx/transport/common/transport_Message.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_Commands.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>

#include <ccnx/api/notify/notify_Status.h>
#include <ccnx/api/control/cpi_ControlFacade.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef DEBUG_OUTPUT
#undef DEBUG_OUTPUT
#endif

#define DEBUG_OUTPUT 0

// SPEW will dump stack traces on reference count events
#define SPEW 0

struct rta_connection {
    RtaProtocolStack        *stack;
    RtaFramework            *framework;

    // unique id for this connection
    unsigned connid;

    // opaque component-specific data and their closers
    void                   *component_data[LAST_COMPONENT];
    RtaComponentStats         *component_stats[LAST_COMPONENT];

    RtaConnectionStateType connState;

    unsigned messages_in_queue;
    unsigned refcount;

    PARCJSON                *params;

    // api_fd is used in status messages up to the user
    // transport_fd is used by the API connector to talk w/ API.
    int api_fd;
    int transport_fd;

    // is the connection blocked in the given direction?
    bool blocked_down;
    bool blocked_up;
};

RtaComponentStats *
rtaConnection_GetStats(RtaConnection *conn, RtaComponents component)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->component_stats[component];
}

RtaConnection *
rtaConnection_Create(RtaProtocolStack *stack, const RtaCommandOpenConnection *cmdOpen)
{
    int i;
    RtaConnection *conn = parcMemory_AllocateAndClear(sizeof(RtaConnection));
    assertNotNull(conn, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(RtaConnection));

    conn->stack = stack;
    conn->framework = rtaProtocolStack_GetFramework(stack);
    conn->connid = rtaProtocolStack_GetNextConnectionId(stack);
    conn->connState = CONN_OPEN;
    conn->api_fd = rtaCommandOpenConnection_GetApiNotifierFd(cmdOpen);
    conn->transport_fd = rtaCommandOpenConnection_GetTransportNotifierFd(cmdOpen);

    conn->params = parcJSON_Copy(rtaCommandOpenConnection_GetConfig(cmdOpen));
    conn->refcount = 1;

    conn->blocked_down = false;
    conn->blocked_up = false;

    for (i = 0; i < LAST_COMPONENT; i++) {
        conn->component_stats[i] = rtaComponentStats_Create(stack, i);
    }

    if (DEBUG_OUTPUT) {
        fprintf(stderr, "%9" PRIu64 " %s connection %p refcount %d\n",
                rtaFramework_GetTicks(conn->framework), __func__, (void *) conn, conn->refcount);
        if (SPEW) {
            longBowRuntime_StackTrace(STDERR_FILENO);
        }

        char *p = parcJSON_ToString(conn->params);
        printf("Connection configuration: %s\n", p);
        parcMemory_Deallocate((void **) &p);
    }

    return conn;
}

RtaConnection *
rtaConnection_Copy(RtaConnection *original)
{
    assertNotNull(original, "Called with null parameter");
    original->refcount++;
    if (DEBUG_OUTPUT) {
        fprintf(stderr, "%9" PRIu64 " %s connection %p refcount %d\n",
                rtaFramework_GetTicks(original->framework), __func__, (void *) original, original->refcount);
        if (SPEW) {
            longBowRuntime_StackTrace(STDERR_FILENO);
        }
    }

    return original;
}

void
rtaConnection_FreeFunc(void **voidPtr)
{
    rtaConnection_Destroy((RtaConnection **) voidPtr);
}

void
rtaConnection_Destroy(RtaConnection **connPtr)
{
    int i;
    RtaConnection *conn;
    assertNotNull(connPtr, "called with null connection pointer\n");
    conn = *connPtr;
    assertNotNull(conn, "called with null connection\n");
    assertTrue(conn->refcount > 0, "Called with 0 refcount, invalid state");

    conn->refcount--;
    if (conn->refcount > 0) {
        if (DEBUG_OUTPUT) {
            fprintf(stderr, "%9" PRIu64 "  %s connection %p skipped, refcount %u\n",
                    rtaFramework_GetTicks(conn->framework), __func__, (void *) conn, conn->refcount);
            if (SPEW) {
                longBowRuntime_StackTrace(STDERR_FILENO);
            }
        }
        return;
    }

    assertTrue(conn->messages_in_queue == 0, "called when messages are still queued\n");

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s connection %p\n", rtaFramework_GetTicks(conn->framework), __func__, (void *) conn);
        if (SPEW) {
            longBowRuntime_StackTrace(STDERR_FILENO);
        }
    }

    // Ok, at this point there's nothing left in queue, so we can
    // get rid of the container now

    for (i = 0; i < LAST_COMPONENT; i++) {
        rtaComponentStats_Destroy(&conn->component_stats[i]);
    }

    rtaFramework_RemoveConnection(conn->framework, conn);
    parcJSON_Release(&conn->params);
    parcMemory_Deallocate((void **) &conn);
    *connPtr = NULL;
}

RtaProtocolStack *
rtaConnection_GetStack(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->stack;
}

/*
 * Used to store per-connection state from Open.
 * Should be freed in Close, but you don't need to set it NULL.
 */
void
rtaConnection_SetPrivateData(RtaConnection *conn,
                             RtaComponents component,
                             void *private)
{
    assertNotNull(conn, "called with null connection\n");
    conn->component_data[component] = private;
}

/*
 * Used to store per-connection state from Open
 */
void *
rtaConnection_GetPrivateData(RtaConnection *conn,
                             RtaComponents component)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->component_data[component];
}

RtaConnectionStateType
rtaConnection_GetState(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->connState;
}

void
rtaConnection_SetState(RtaConnection *conn, RtaConnectionStateType connState)
{
    assertNotNull(conn, "called with null connection\n");
    conn->connState = connState;
    rtaProtocolStack_ConnectionStateChange(conn->stack, conn);
}

/*
 * returns number in queue, including this one
 */
unsigned
rtaConnection_IncrementMessagesInQueue(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    assertTrue(conn->connState != CONN_CLOSED, "%s called when connection closed\n", __func__);
    conn->messages_in_queue++;
    return conn->messages_in_queue;
}

unsigned
rtaConnection_DecrementMessagesInQueue(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    assertTrue(conn->messages_in_queue > 0, "Trying to decrement a queue with 0 messages already");

    conn->messages_in_queue--;
    return conn->messages_in_queue;
}

int
rtaConnection_GetApiFd(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->api_fd;
}


int
rtaConnection_GetTransportFd(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->transport_fd;
}

int
rtaConnection_GetStackId(RtaConnection *conn)
{
    return rtaProtocolStack_GetStackId(conn->stack);
}

unsigned
rtaConnection_MessagesInQueue(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->messages_in_queue;
}

unsigned
rtaConnection_GetConnectionId(const RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection\n");
    return conn->connid;
}

void
rtaConnection_SendNotifyStatus(RtaConnection *conn, RtaComponents component, RtaDirection direction, const NotifyStatus *status)
{
    PARCJSON *json = notifyStatus_ToJSON(status);

    CCNxTlvDictionary *notification = ccnxControlFacade_CreateNotification(json);
    parcJSON_Release(&json);

    TransportMessage *tm = transportMessage_CreateFromDictionary(notification);
    ccnxTlvDictionary_Release(&notification);

    PARCEventQueue *out = rtaComponent_GetOutputQueue(conn, component, direction);

    transportMessage_SetInfo(tm, rtaConnection_Copy(conn), rtaConnection_FreeFunc);
    rtaComponent_PutMessage(out, tm);
}

void
rtaConnection_SendStatus(RtaConnection *conn,
                         RtaComponents component,
                         RtaDirection direction,
                         NotifyStatusCode code,
                         CCNxName *optionalName,
                         const char *optionalMessage)
{
    NotifyStatus *status = notifyStatus_Create(conn->api_fd, code, optionalName, optionalMessage);
    rtaConnection_SendNotifyStatus(conn, component, direction, status);
    notifyStatus_Release(&status);
}

RtaConnection *
rtaConnection_GetFromTransport(TransportMessage *tm)
{
    return (RtaConnection *) transportMessage_GetInfo(tm);
}

RtaFramework *
rtaConnection_GetFramework(const RtaConnection *connection)
{
    assertNotNull(connection, "called with null connection");
    return connection->framework;
}

PARCJSON *
rtaConnection_GetParameters(RtaConnection *conn)
{
    assertNotNull(conn, "called with null connection");
    return conn->params;
}

bool
rtaConnection_BlockedDown(const RtaConnection *connection)
{
    assertNotNull(connection, "Parameter connection must be non-null");
    return (connection->connState != CONN_OPEN) || connection->blocked_down;
}

bool
rtaConnection_BlockedUp(const RtaConnection *connection)
{
    assertNotNull(connection, "Parameter connection must be non-null");
    return (connection->connState != CONN_OPEN) || connection->blocked_up;
}

void
rtaConnection_SetBlockedDown(RtaConnection *connection)
{
    assertNotNull(connection, "Parameter connection must be non-null");
    connection->blocked_down = true;
    rtaProtocolStack_ConnectionStateChange(connection->stack, connection);
}

void
rtaConnection_ClearBlockedDown(RtaConnection *connection)
{
    assertNotNull(connection, "Parameter connection must be non-null");
    connection->blocked_down = false;
    rtaProtocolStack_ConnectionStateChange(connection->stack, connection);
}

void
rtaConnection_SetBlockedUp(RtaConnection *connection)
{
    assertNotNull(connection, "Parameter connection must be non-null");
    connection->blocked_up = true;
    rtaProtocolStack_ConnectionStateChange(connection->stack, connection);
}

void
rtaConnection_ClearBlockedUp(RtaConnection *connection)
{
    assertNotNull(connection, "Parameter connection must be non-null");
    connection->blocked_up = false;
    rtaProtocolStack_ConnectionStateChange(connection->stack, connection);
}
