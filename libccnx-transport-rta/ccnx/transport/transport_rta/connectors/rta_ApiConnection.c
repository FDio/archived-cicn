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
 * Implements the API connector.  The API connector is a event based component to manage the socket
 * to the API.
 *
 * The API Connector's job is to manage the socket to the API between the RTA Framework and the
 * API.  It does this by using an event directly to manage that socket.  It uses the same
 * event scheduler base as the RTA framework, so its all part of the same event dispatcher.
 *
 * The RTA Transport now only speaks CCNxTlvDictionary messages.  If we receive old timey Interest,
 * ContentObject, etc., we translate them to the Dictionary format. The TransportMessage and CCNxMessage
 * will both go away.
 *
 */

#include <config.h>
#include <stdio.h>
#include <fcntl.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <errno.h>

#include <parc/algol/parc_EventBuffer.h>

#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

#include <ccnx/transport/transport_rta/connectors/rta_ApiConnection.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_Services.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>
#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_ControlFacade.h>

#include <ccnx/transport/transport_rta/config/config_Codec_Tlv.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>


#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

#define PAIR_TRANSPORT 0
#define PAIR_OTHER     1

// we are only putting an 8-byte pointer on the queue, so
// this should be 50 messages
#define MAX_API_QUEUE_BYTES     400


unsigned api_upcall_writes = 0;
unsigned api_downcall_reads = 0;
extern unsigned rta_transport_reads;

// per connection state
struct rta_api_connection {
    // A reference to our connection
    RtaConnection *connection;

    // event queue for socketpair to API
    PARCEventQueue *bev_api;

    // these are assingned to us by the Transport
    int api_fd;
    int transport_fd;
};

// ==========================================================================================
// STATIC PROTOTYPES and their headerdoc

/**
 * PARCEvent calls this when the API queue falls below the watermark
 *
 * We watermark the write queue at MAX_API_QUEUE_BYTES bytes.  When a write takes
 * the queue backlog below that amount, PARCEvent calls this.
 *
 * @param [in] connVoid Void pointer to the RtaConnection
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void rtaApiConnection_WriteCallback(PARCEventQueue *queue, PARCEventType type, void *conn);

/**
 * PARCEvent calls this when there's a message from the API
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void rtaApiConnection_Downcall_Read(PARCEventQueue *bev, PARCEventType type, void *conn);

/**
 * PARCEvent calls this when there's a non-read/write event on the API's socket
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void rtaApiConnection_Downcall_Event(PARCEventQueue *, PARCEventQueueEventType events, void *conn);


/**
 * Drains the input queue and output queue of a connection to the API
 *
 * The input queue and output queue contain pointers to CCNxMessages.  On close,
 * we need to drain these queues and release all the messages.
 *
 * The API Connector is responsible for only draining its input queue.  The output
 * queue up to the API is drained by the RTA Framework.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void rtaApiConnection_DrainApiConnection(RtaApiConnection *apiConnection);

/**
 * Writes a message to the API
 *
 * Takes ownership of the message which is passed up to the API
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void rtaApiConnection_WriteMessageToApi(RtaApiConnection *apiConnection, CCNxMetaMessage *msg);

// ==========================================================================================
// Public API

static void
rtaApiConnection_SetupSocket(RtaApiConnection *apiConnection, RtaConnection *connection)
{
    RtaProtocolStack *stack = rtaConnection_GetStack(connection);
    PARCEventScheduler *base = rtaFramework_GetEventScheduler(rtaProtocolStack_GetFramework(stack));
    int error;

    // Set non-blocking flag
    int flags = fcntl(apiConnection->transport_fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(apiConnection->transport_fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set socket non-blocking(%d) %s\n", errno, strerror(errno));

    apiConnection->bev_api = parcEventQueue_Create(base, apiConnection->transport_fd, 0);
    assertNotNull(apiConnection->bev_api, "Got null result from parcEventQueue_Create");

    // Set buffer size
    int sendbuff = 1000 * 8;

    error = setsockopt(rtaConnection_GetTransportFd(connection), SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff));
    assertTrue(error == 0, "Got error setting SO_SNDBUF: %s", strerror(errno));

    parcEventQueue_SetWatermark(apiConnection->bev_api, PARCEventType_Write, MAX_API_QUEUE_BYTES, 0);
    parcEventQueue_SetCallbacks(apiConnection->bev_api,
                                rtaApiConnection_Downcall_Read,
                                rtaApiConnection_WriteCallback,
                                rtaApiConnection_Downcall_Event,
                                (void *) connection);

    parcEventQueue_Enable(apiConnection->bev_api, PARCEventType_Read | PARCEventType_Write);
}

RtaApiConnection *
rtaApiConnection_Create(RtaConnection *connection)
{
    RtaApiConnection *apiConnection = parcMemory_AllocateAndClear(sizeof(RtaApiConnection));
    assertNotNull(apiConnection, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(RtaApiConnection));

    apiConnection->connection = rtaConnection_Copy(connection);
    apiConnection->api_fd = rtaConnection_GetApiFd(connection);
    apiConnection->transport_fd = rtaConnection_GetTransportFd(connection);
    rtaApiConnection_SetupSocket(apiConnection, connection);

    return apiConnection;
}

void
rtaApiConnection_Destroy(RtaApiConnection **apiConnectionPtr)
{
    assertNotNull(apiConnectionPtr, "Parameter apiConnecitonPtr must be non-null");
    assertNotNull(*apiConnectionPtr, "Parameter apiConnecitonPtr must dereference to non-null");
    RtaApiConnection *apiConnection = *apiConnectionPtr;


    // Send all the outbound messages up to the API.  This at least gets them out
    // of our output queue on to the API's socket.
    parcEventQueue_Finished(apiConnection->bev_api, PARCEventType_Write);
    rtaApiConnection_DrainApiConnection(apiConnection);

    parcEventQueue_Destroy(&(apiConnection->bev_api));

    rtaConnection_Destroy(&apiConnection->connection);

    parcMemory_Deallocate((void **) &apiConnection);

    *apiConnectionPtr = NULL;
}

static void
rtaApiConnection_SendToApiAsDictionary(RtaApiConnection *apiConnection, TransportMessage *tm)
{
    CCNxMetaMessage *msg = ccnxMetaMessage_Acquire(transportMessage_GetDictionary(tm));
    rtaApiConnection_WriteMessageToApi(apiConnection, msg);
}

static CCNxName *
rtaApiConnection_GetNameFromTransportMessage(TransportMessage *tm)
{
    CCNxName *name = NULL;
    CCNxTlvDictionary *dictionary = transportMessage_GetDictionary(tm);
    switch (ccnxTlvDictionary_GetSchemaVersion(dictionary)) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            name = ccnxTlvDictionary_GetName(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
            break;

        default:
            break;
    }
    return name;
}

/**
 * Writes the CCNxMessage inside the transport message up to the API.
 * Its possible that if there's no space in the socket the write will block
 * and return an error.
 *
 * @return true if written to API, false if not (most likely would block)
 */
bool
rtaApiConnection_SendToApi(RtaApiConnection *apiConnection, TransportMessage *tm, RtaComponentStats *stats)
{
    assertNotNull(apiConnection, "Parameter apiConnection must be non-null");

    if (DEBUG_OUTPUT) {
        CCNxName *name = rtaApiConnection_GetNameFromTransportMessage(tm);
        char *nameString = NULL;
        if (name) {
            nameString = ccnxName_ToString(name);
        }

        struct timeval delay = transportMessage_GetDelay(tm);
        printf("%9" PRIu64 " %s putting transport msg %p to   user fd %d delay %.6f name %s\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(apiConnection->connection))),
               __func__,
               (void *) tm,
               apiConnection->api_fd,
               delay.tv_sec + delay.tv_usec * 1E-6,
               nameString);

        if (nameString) {
            parcMemory_Deallocate((void **) &nameString);
        }
    }

    rtaApiConnection_SendToApiAsDictionary(apiConnection, tm);

    rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s conn %p fd_out %d state %p upcalls %u reads %u\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(apiConnection->connection))),
               __func__,
               (void *)  apiConnection->connection,
               apiConnection->transport_fd,
               (void *) apiConnection,
               api_upcall_writes,
               rta_transport_reads);
    }

    return true;
}

void
rtaApiConnection_BlockDown(RtaApiConnection *apiConnection)
{
    assertNotNull(apiConnection, "Parameter apiConnection must be non-null");
    PARCEventType enabled_events = parcEventQueue_GetEnabled(apiConnection->bev_api);

    // we only disable it and log it if it was active
    if (enabled_events & PARCEventType_Read) {
        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s connection %u blocked down, disable PARCEventType_Read\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(apiConnection->connection))),
                   __func__,
                   rtaConnection_GetConnectionId(apiConnection->connection));
        }

        parcEventQueue_Disable(apiConnection->bev_api, PARCEventType_Read);
    }
}

void
rtaApiConnection_UnblockDown(RtaApiConnection *apiConnection)
{
    assertNotNull(apiConnection, "Parameter apiConnection must be non-null");
    PARCEventType enabled_events = parcEventQueue_GetEnabled(apiConnection->bev_api);

    if (!(enabled_events & PARCEventType_Read)) {
        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s connection %u unblocked down, enable PARCEventType_Read\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(apiConnection->connection))),
                   __func__,
                   rtaConnection_GetConnectionId(apiConnection->connection));
        }
        parcEventQueue_Enable(apiConnection->bev_api, PARCEventType_Read);
    }
}

// ==========================================================================================
// Internal implementation

static void
rtaApiConnection_WriteMessageToApi(RtaApiConnection *apiConnection, CCNxMetaMessage *msg)
{
    assertNotNull(msg, "Parameter msg must be non-null");

    int error = parcEventQueue_Write(apiConnection->bev_api, &msg, sizeof(&msg));
    assertTrue(error == 0,
               "write to transport_fd %d write error: (%d) %s",
               apiConnection->transport_fd, errno, strerror(errno));

    // debugging tracking
    api_upcall_writes++;
}

static void
_rtaAPIConnection_ProcessCPIRequest(RtaConnection *conn, PARCJSON *json)
{
    // Is it a request type we know about?

    switch (cpi_getCPIOperation2(json)) {
        case CPI_PAUSE: {
            RtaConnectionStateType oldstate = rtaConnection_GetState(conn);
            if (oldstate == CONN_OPEN) {
                rtaConnection_SetState(conn, CONN_PAUSED);
            }
            break;
        }

        default:
            // do nothing, don't know about this message type
            break;
    }
}

static void
connector_Api_ProcessCpiMessage(RtaConnection *conn, CCNxTlvDictionary *controlDictionary)
{
    if (ccnxControlFacade_IsCPI(controlDictionary)) {
        PARCJSON *json = ccnxControlFacade_GetJson(controlDictionary);
        switch (controlPlaneInterface_GetCPIMessageType(json)) {
            case CPI_REQUEST: {
                _rtaAPIConnection_ProcessCPIRequest(conn, json);
                break;
            }

            case CPI_RESPONSE:
                break;

            case CPI_ACK:
                break;

            default:
                assertTrue(0, "Got unknown CPI message type: %d", controlPlaneInterface_GetCPIMessageType(json));
        }
    }
}

static void
rtaApiConnection_ProcessControlFromApi(RtaApiConnection *apiConnection, RtaProtocolStack *stack, CCNxTlvDictionary *controlDictionary)
{
    if (ccnxControlFacade_IsCPI(controlDictionary)) {
        connector_Api_ProcessCpiMessage(apiConnection->connection, controlDictionary);
    }
}

static void
rtaApiConnection_Downcall_ProcessDictionary(RtaApiConnection *apiConnection, RtaProtocolStack *stack,
                                            PARCEventQueue *queue_out, RtaComponentStats *stats, CCNxTlvDictionary *messageDictionary)
{
    // Look at the control message before checking for the connection closed
    if (ccnxTlvDictionary_IsControl(messageDictionary)) {
        rtaApiConnection_ProcessControlFromApi(apiConnection, stack, messageDictionary);
    }

    // In paused or closed state, we only pass control messages
    if ((rtaConnection_GetState(apiConnection->connection) == CONN_OPEN) || (ccnxTlvDictionary_IsControl(messageDictionary))) {
        TransportMessage *tm = transportMessage_CreateFromDictionary(messageDictionary);

        // Set the auxiliary information to the message's connection
        transportMessage_SetInfo(tm, rtaConnection_Copy(apiConnection->connection), rtaConnection_FreeFunc);

        if (DEBUG_OUTPUT) {
            CCNxName *name = NULL;
            if (ccnxTlvDictionary_IsInterest(messageDictionary)) {
                name = ccnxInterest_GetName(messageDictionary);
            } else if (ccnxTlvDictionary_IsContentObject(messageDictionary)) {
                name = ccnxContentObject_GetName(messageDictionary);
            }

            char *noname = "NONAME";
            char *nameString = noname;
            if (name) {
                nameString = ccnxName_ToString(name);
            }

            printf("%9" PRIu64 " %s putting transport msg %p from user fd %d: %s\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(apiConnection->connection))),
                   __func__,
                   (void *) tm, apiConnection->api_fd,
                   nameString);

            if (nameString != noname) {
                parcMemory_Deallocate((void **) &nameString);
            }

            //ccnxTlvDictionary_Display(0, messageDictionary);
        }

        // send down the stack.  If it fails, it destroys the message.
        if (rtaComponent_PutMessage(queue_out, tm)) {
            rtaComponentStats_Increment(stats, STATS_DOWNCALL_OUT);
        }
    }
}

static void
rtaApiConnection_Downcall_ProcessMessage(RtaApiConnection *apiConnection, RtaProtocolStack *stack, PARCEventBuffer *eb_in,
                                         PARCEventQueue *queue_out, RtaComponentStats *stats)
{
    api_downcall_reads++;
    CCNxMetaMessage *msg;

    int bytesRemoved = parcEventBuffer_Read(eb_in, &msg, sizeof(CCNxMetaMessage *));
    assertTrue(bytesRemoved == sizeof(CCNxMetaMessage *),
               "Error, did not remove an entire pointer, expected %zu got %d",
               sizeof(CCNxMetaMessage *),
               bytesRemoved);

    rtaComponentStats_Increment(stats, STATS_DOWNCALL_IN);

    // This will save its own reference to the messageDictionary
    rtaApiConnection_Downcall_ProcessDictionary(apiConnection, stack, queue_out, stats, msg);

    // At this point, the CCNxMetaMessage passed in by the application thread has been
    // acquired rtaApiConnection_Downcall_ProcessDictionary(), so we can Release the reference we
    // acquired in rtaTransport_Send().
    ccnxMetaMessage_Release(&msg);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s conn %p total downcall reads in %" PRIu64 " out %" PRIu64 "\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(apiConnection->connection))),
               __func__,
               (void *) apiConnection->connection,
               rtaComponentStats_Get(stats, STATS_DOWNCALL_IN),
               rtaComponentStats_Get(stats, STATS_DOWNCALL_OUT));
    }
}


/*
 * Called by PARCEvent when there's a message to read from the API
 * Read a message from the API.
 * rtaConnectionVoid is the RtaConnection associated with the api descriptor
 */
static void
rtaApiConnection_Downcall_Read(PARCEventQueue *bev, PARCEventType type, void *rtaConnectionVoid)
{
    RtaConnection *conn = (RtaConnection *) rtaConnectionVoid;

    assertNotNull(rtaConnectionVoid, "Parameter must be a non-null void *");

    RtaProtocolStack *stack = rtaConnection_GetStack(conn);
    assertNotNull(stack, "rtaConnection_GetStack returned null");

    RtaComponentStats *stats = rtaConnection_GetStats(conn, API_CONNECTOR);
    assertNotNull(stats, "rtaConnection_GetStats returned null");

    RtaApiConnection *apiConnection = rtaConnection_GetPrivateData(conn, API_CONNECTOR);
    assertNotNull(apiConnection, "rtaConnection_GetPrivateData got null");

    PARCEventBuffer *eb_in = parcEventBuffer_GetQueueBufferInput(bev);

    PARCEventQueue *queue_out = rtaComponent_GetOutputQueue(conn, API_CONNECTOR, RTA_DOWN);
    assertNotNull(queue_out, "component_GetOutputQueue returned null");

    while (parcEventBuffer_GetLength(eb_in) >= sizeof(TransportMessage *)) {
        rtaApiConnection_Downcall_ProcessMessage(apiConnection, stack, eb_in, queue_out, stats);
    }
    parcEventBuffer_Destroy(&eb_in);
}

/*
 * This is used on the connection to the API out of the transport box
 */
static void
rtaApiConnection_Downcall_Event(PARCEventQueue *bev, PARCEventQueueEventType events, void *ptr)
{
}

/**
 * Drains all the CCNxMessages off an event buffer and destroys them
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
drainBuffer(PARCEventBuffer *buffer, RtaConnection *conn)
{
    size_t length;

    while ((length = parcEventBuffer_GetLength(buffer)) > 0) {
        CCNxMetaMessage *msg;
        ssize_t len;

        len = parcEventBuffer_Read(buffer, &msg, sizeof(CCNxMetaMessage *));
        assertTrue(len == sizeof(CCNxMetaMessage *),
                   "Removed incorrect length, expected %zu got %zd: (%d) %s",
                   sizeof(CCNxMetaMessage *),
                   len,
                   errno,
                   strerror(errno));

        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s conn %p drained message %p\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                   __func__,
                   (void *) conn,
                   (void *) msg);
        }
        ccnxMetaMessage_Release(&msg);
    }
}

/**
 * Called on Destroy to clear our input buffer.  This does not
 * drain the output (to API) buffer, that is done by the RTA Framework
 */
static void
rtaApiConnection_DrainApiConnection(RtaApiConnection *apiConnection)
{
    // drain and free the transport_fd
    parcEventQueue_Disable(apiConnection->bev_api, PARCEventType_Read);

    PARCEventBuffer *in = parcEventBuffer_GetQueueBufferInput(apiConnection->bev_api);
    drainBuffer(in, apiConnection->connection);
    parcEventBuffer_Destroy(&in);

    // There may be some messages in the output buffer that
    // have not actually been written to the kernel socket.
    // Drain those too, as the API will never see them

    if (DEBUG_OUTPUT) {
        PARCEventBuffer *out = parcEventBuffer_GetQueueBufferOutput(apiConnection->bev_api);
        printf("%9" PRIu64 " %s conn %p output buffer has %zu bytes\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(apiConnection->connection))),
               __func__,
               (void *) apiConnection->connection,
               parcEventBuffer_GetLength(out));
        parcEventBuffer_Destroy(&out);
    }
}

/**
 * Called by PARCEvent when we cross below the write watermark
 */
static void
rtaApiConnection_WriteCallback(PARCEventQueue *queue, PARCEventType type, void *connVoid)
{
    // we dropped below the write watermark, unblock the connection in the UP direction
    RtaConnection *conn = (RtaConnection *) connVoid;
    if (rtaConnection_BlockedUp(conn)) {
        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s connection %u output fell below watermark, unblocking UP\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                   __func__,
                   rtaConnection_GetConnectionId(conn));
        }

        rtaConnection_ClearBlockedUp(conn);
    }
}
