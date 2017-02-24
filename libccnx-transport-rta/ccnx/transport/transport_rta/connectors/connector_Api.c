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
 * Interface between the event dispatcher and component callbacks to
 * the RtaApiConnection.  The API connector, per se, is implemented in rta_ApiConnection.  This
 * module is the scaffolding to work within the RTA component framework.
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <errno.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/transport/transport_rta/connectors/rta_ApiConnection.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_Services.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>
#include <ccnx/transport/transport_rta/connectors/connector_Api.h>

#include <ccnx/api/control/controlPlaneInterface.h>

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

static int  connector_Api_Init(RtaProtocolStack *stack);
static int  connector_Api_Opener(RtaConnection *conn);
static void connector_Api_Upcall_Read(PARCEventQueue *, PARCEventType, void *conn);
static int  connector_Api_Closer(RtaConnection *conn);
static int  connector_Api_Release(RtaProtocolStack *stack);
static void connector_Api_StateChange(RtaConnection *conn);

RtaComponentOperations api_ops =
{
    .init          = connector_Api_Init,
    .open          = connector_Api_Opener,
    .upcallRead    = connector_Api_Upcall_Read,
    .upcallEvent   = NULL,
    .downcallRead  = NULL,
    .downcallEvent = NULL,
    .close         = connector_Api_Closer,
    .release       = connector_Api_Release,
    .stateChange   = connector_Api_StateChange
};

// ========================

static int
connector_Api_Init(RtaProtocolStack *stack)
{
    // nothing to do here
    if (DEBUG_OUTPUT) {
        printf("%s init stack %p\n",
               __func__,
               (void *) stack);
    }
    return 0;
}

/*
 * Api_Open will put the RtaConnection as the callback parameter in the UpcallRead,
 * because its a per-connection descriptor.
 *
 * Returns 0 on success, -1 on error
 */
static int
connector_Api_Opener(RtaConnection *connection)
{
    RtaComponentStats *stats;
    RtaApiConnection *apiConnection = rtaApiConnection_Create(connection);

    rtaConnection_SetPrivateData(connection, API_CONNECTOR, apiConnection);

    stats = rtaConnection_GetStats(connection, API_CONNECTOR);
    assertNotNull(stats, "%s returned null stats\n", __func__);
    rtaComponentStats_Increment(stats, STATS_OPENS);

    rtaConnection_SetState(connection, CONN_OPEN);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s opened transport_fd %d\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(connection))),
               __func__,
               rtaConnection_GetTransportFd(connection));

        printf("%9" PRIu64 " %s open conn %p state %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(connection))),
               __func__,
               (void *) connection,
               (void *) apiConnection);
    }

    return 0;
}

/*
 * Read a message from below in stack
 * Write a message up to the API
 */
static void
connector_Api_Upcall_Read(PARCEventQueue *eventBuffer, PARCEventType type, void *protocolStackVoid)
{
    TransportMessage *tm;

    assertNotNull(protocolStackVoid, "%s called with null ProtocolStack\n", __func__);

    while ((tm = rtaComponent_GetMessage(eventBuffer)) != NULL) {
        RtaConnection *conn = rtaConnection_GetFromTransport(tm);
        assertNotNull(conn, "got null connection from transport message\n");

        RtaComponentStats *stats = rtaConnection_GetStats(conn, API_CONNECTOR);
        assertNotNull(stats, "returned null stats\n");

        rtaComponentStats_Increment(stats, STATS_UPCALL_IN);

        RtaApiConnection *apiConnection = rtaConnection_GetPrivateData(conn, API_CONNECTOR);
        assertNotNull(apiConnection, "got null apiConnection\n");

        // If we are blocked, only pass control messages
        if (!rtaConnection_BlockedUp(conn) || transportMessage_IsControl(tm)) {
            if (!rtaApiConnection_SendToApi(apiConnection, tm, stats)) {
                // memory is freed at bottom of function
            }
        } else {
            // closed connection, just destroy the message
            if (DEBUG_OUTPUT) {
                printf("%9" PRIu64 " %s conn %p destroying transport message %p due to closed connection\n",
                       rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                       __func__,
                       (void *) conn,
                       (void *) tm);
            }
        }

        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s conn %p total upcall reads in %" PRIu64 " out %" PRIu64 "\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                   __func__,
                   (void *) conn,
                   rtaComponentStats_Get(stats, STATS_UPCALL_IN),
                   rtaComponentStats_Get(stats, STATS_UPCALL_OUT));
        }

        // This is the end of life for the transport message.  If the inner TlvDictionary
        // was put in a CCNxMessage and sent up the stack, then we made another reference to it
        // so this destroy will not destroy that part.
        transportMessage_Destroy(&tm);
    }
}

/*
 * The higher layer should no longer be writing to this
 * socketpair, so we can drain it then close it.
 */
static int
connector_Api_Closer(RtaConnection *conn)
{
    RtaComponentStats *stats;
    RtaApiConnection *apiConnection = rtaConnection_GetPrivateData(conn, API_CONNECTOR);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s starting close conn %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) conn);
    }

    stats = rtaConnection_GetStats(conn, API_CONNECTOR);
    assertNotNull(stats, "%s returned null stats\n", __func__);
    rtaComponentStats_Increment(stats, STATS_CLOSES);

    // This will prevent any new data going in to queues for the connection
    // Existing messages will be destroyed
    rtaConnection_SetState(conn, CONN_CLOSED);

    rtaApiConnection_Destroy(&apiConnection);
    rtaConnection_SetPrivateData(conn, API_CONNECTOR, NULL);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s close conn %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) conn);
    }

    return 0;
}

static int
connector_Api_Release(RtaProtocolStack *stack)
{
    // nothing to do here, there's no ProtocolStack state
    if (DEBUG_OUTPUT) {
        printf("%s release stack %p\n",
               __func__,
               (void *) stack);
    }

    return 0;
}

/**
 * Respond to events for the connection
 *
 * Typcially, the forwarder connector will block and unblock the DOWN direction.  We need
 * to stop putting new data in the down directon if its blocked.
 *
 * The API connector (us) is generally the thing blocking the UP direction, so we don't need
 * to respond to those (our own) events.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *      ComponentOperations api_ops = {
 *          // [other settings]
 *          .stateChange  = connector_Api_StateChange
 *      };
 * }
 * @endcode
 */
static void
connector_Api_StateChange(RtaConnection *conn)
{
    RtaApiConnection *apiConnection = rtaConnection_GetPrivateData(conn, API_CONNECTOR);

    // we do not test the rtaConnection_BlockedUp() because we are the one setting those

    // If we are blocked in the DOWN direction, disable events on the read queue
    if (rtaConnection_BlockedDown(conn)) {
        rtaApiConnection_BlockDown(apiConnection);
    } else {
        rtaApiConnection_UnblockDown(apiConnection);
    }
}
