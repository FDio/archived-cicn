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
 * The metis connector does the following per connection:
 * - Opens a TCP socket to Metis
 * - Creates an "event" for the socket, does not use the buffer to avoid doing extra copy.
 * - On read events, uses direct socket operations to read in data
 *
 * - DOES NOT HANDLE FRAMING ERRORS.  If somehow metis and the connector get
 *   out of whack (technical term), there is no recovery.
 *
 * - The connection to metis is started in the Opener, but may not complete by the time
 *   the user sends data down in the Downcall_Read.  We should not process the Downcall_Read
 *   until we get the Upcall_Event of connected.  When we finally get the connected event,
 *   we should make the Downcall_Read pending again (or just call it) to flush the pending
 *   user data out to metis.
 *
 * - Because of how we get scheduled, there might be a large batch of messages waiting at the
 *   forwarder.  We don't want to put a giant blob up the stack.  So, we keep a deque of TransportMessage
 *   and only feed a few at a time up.
 *
 * - Accepts both a PARCBuffer or a CCNxCodecNetworkBufferIoVec as the wire format in the DOWN direction.
 * - The UP direction is always a PARCBuffer right now
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Deque.h>
#include <parc/algol/parc_EventBuffer.h>
#include <parc/algol/parc_EventTimer.h>
#include <parc/algol/parc_Network.h>

#include <ccnx/transport/common/transport_Message.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_Services.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>

#include "connector_Forwarder.h"

#include <ccnx/transport/transport_rta/config/config_Forwarder_Metis.h>

#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_ControlFacade.h>

#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

#include <ccnx/common/codec/ccnxCodec_TlvPacket.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>

#include <ccnx/common/ccnx_WireFormatMessage.h>

#define MINIMUM_READ_LENGTH 8

// The message type for a Metis control packet
#define METIS_CONTROL_TYPE 0xA4

// at most 10MB, this is used as the output buffer down to metis
#define METIS_OUTPUT_QUEUE_BYTES (10 * 1024 * 1024)

// How big should we try to make the output socket size?
#define METIS_SEND_SOCKET_BUFFER 65536

// Maximum input backlog in messages, not bytes
#define METIS_INPUT_QUEUE_MESSAGES 100

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

static int  connector_Fwd_Metis_Init(RtaProtocolStack *stack);
static int  connector_Fwd_Metis_Opener(RtaConnection *conn);

static void _eventCallback(int fd, PARCEventType what, void *connectionVoid);
static void connector_Fwd_Metis_Dequeue(int fd, PARCEventType which_event, void *metisStateVoid);

static void connector_Fwd_Metis_Downcall_Read(PARCEventQueue *, PARCEventType, void *conn);
static int  connector_Fwd_Metis_Closer(RtaConnection *conn);
static int  connector_Fwd_Metis_Release(RtaProtocolStack *stack);
static void connector_Fwd_Metis_StateChange(RtaConnection *conn);

RtaComponentOperations fwd_metis_ops = {
    .init          = connector_Fwd_Metis_Init,
    .open          = connector_Fwd_Metis_Opener,
    .upcallRead    = NULL,
    .upcallEvent   = NULL,
    .downcallRead  = connector_Fwd_Metis_Downcall_Read,
    .downcallEvent = NULL,
    .close         = connector_Fwd_Metis_Closer,
    .release       = connector_Fwd_Metis_Release,
    .stateChange   = connector_Fwd_Metis_StateChange
};

typedef enum {
    PacketType_Interest,
    PacketType_ContentObject,
    PacketType_Control,
    PacketType_InterestReturn,
    PacketType_Unknown
} _PacketType;

typedef struct metis_connector_stats {
    unsigned countUpcallReads;
    unsigned countUpcallWriteDataOk;
    unsigned countUpcallWriteDataError;
    unsigned countUpcallWriteDataBlocked;
    unsigned countUpcallWriteDataQueueFull;

    unsigned countUpcallWriteControlOk;
    unsigned countUpcallWriteControlError;

    unsigned countDowncallReads;
    unsigned countDowncallWrites;
    unsigned countDowncallControl;
} _MetisConnectorStats;

/**
 * This structure holds the read-ahead data for the next message being read based
 * on its fixed header
 */
typedef struct next_message_header {
    // this is how we frame received messages on a stream connection.  We
    // wait until we read a complete fixed header, then we can set the length
    // of that message and keep waiting until we receive at least that many bytes.
    size_t length;

    // at the time when we parse out the message length from the fixed header,
    // we also parse out the TLV message type from the fixed header
    _PacketType packetType;
    uint8_t version;

    // we will read bytes into this structure
    union _hdr {
        CCNxCodecSchemaV1FixedHeader v1;
        uint8_t buffer[MINIMUM_READ_LENGTH];
    } fixedHeader;

    uint8_t *readLocation;
    size_t remainingReadLength;

    // The whole message
    PARCBuffer *packet;
} NextMessage;

typedef struct fwd_metis_state {
    uint16_t port;
    int fd;

    // separate events for read and write on fd so we can individually enable them
    PARCEvent *readEvent;
    PARCEvent *writeEvent;

    bool isConnected;

    // This is our read-ahead of the next message fixed header
    NextMessage nextMessage;

    // the transportMessageQueueEvent is used to dequeue from the queue.
    // we make sure its scheduled so long as there's messages in the queue, even if there's
    // nothing else being read
    PARCDeque *transportMessageQueue;
    PARCEventTimer *transportMessageQueueEvent;

    // This buffer is the queue of stuff we need to send to the network
    PARCEventBuffer *metisOutputQueue;

    _MetisConnectorStats stats;
} FwdMetisState;

/**
 * @typedef PacketData
 * @brief Used to pass a record between reading a packet and sending it up the stack
 * @discussion Used internally to pass data between functions
 */
typedef struct packet_data {
    FwdMetisState *fwd_state;
    RtaConnection *conn;
    PARCEventQueue  *out;
    RtaComponentStats *stats;
} PacketData;


// for debugging
static unsigned fwd_metis_references_queued = 0;
static unsigned fwd_metis_references_dequeued = 0;
static unsigned fwd_metis_references_notqueued = 0;


typedef enum {
    ReadReturnCode_Finished,    // read all needed bytes
    ReadReturnCode_PartialRead,  // still need some bytes
    ReadReturnCode_Closed,      // the socket is closed
    ReadReturnCode_Error,       // An error on the socket
} ReadReturnCode;

// ================================

static void
_nextMessage_Display(const NextMessage *next, unsigned indent)
{
    printf("NextMessage %p length %zu type %d version %u readLocation %p remaining %zu\n",
           (void *) next, next->length, next->packetType, next->version, (void *) next->readLocation, next->remainingReadLength);

    printf("fixedHeader\n");
    longBowDebug_MemoryDump((const char *) next->fixedHeader.buffer, MINIMUM_READ_LENGTH);

    if (next->packet) {
        parcBuffer_Display(next->packet, 3);
    }
}

static int
connector_Fwd_Metis_Init(RtaProtocolStack *stack)
{
    struct sigaction ignore_action;
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    ignore_action.sa_flags = 0;
    sigaction(SIGPIPE, &ignore_action, NULL);

    return 0;
}


/**
 * Setup the NextMessage structure to begin reading a fixed header
 *
 * All fields are zeroed and the readLocation is set to the first byte of the fixedHeader.
 * The remainingReadLength is set to the size of the fixedHeader.
 *
 * @param [in] next An allocated NextMessage to initialize
 *
 * Example:
 * @code
 * {
 *     NextMessage nextMessage;
 *     _initializeNextMessage(&nextMessage);
 * }
 * @endcode
 */
static void
_initializeNextMessage(NextMessage *next)
{
    memset(next, 0, sizeof(NextMessage));
    next->version = 0xFF;
    next->packetType = PacketType_Unknown;
    next->readLocation = next->fixedHeader.buffer;
    next->remainingReadLength = MINIMUM_READ_LENGTH;
}

static FwdMetisState *
connector_Fwd_Metis_CreateConnectionState(PARCEventScheduler *scheduler)
{
    FwdMetisState *fwd_state = parcMemory_Allocate(sizeof(FwdMetisState));
    assertNotNull(fwd_state, "parcMemory_Allocate(%zu) returned NULL", sizeof(FwdMetisState));

    memset(fwd_state, 0, sizeof(FwdMetisState));
    _initializeNextMessage(&fwd_state->nextMessage);

    fwd_state->fd = 0;
    fwd_state->readEvent = NULL;
    fwd_state->writeEvent = NULL;
    fwd_state->transportMessageQueue = parcDeque_Create();
    fwd_state->transportMessageQueueEvent = parcEventTimer_Create(scheduler, 0, connector_Fwd_Metis_Dequeue, fwd_state);
    fwd_state->isConnected = false;
    fwd_state->metisOutputQueue = parcEventBuffer_Create();

    return fwd_state;
}

static bool
_openSocket(FwdMetisState *fwd_state, uint16_t port)
{
    fwd_state->port = port;
    fwd_state->fd = socket(PF_INET, SOCK_STREAM, 0);

    if (fwd_state->fd < 0) {
        if (DEBUG_OUTPUT) {
            printf("%9c %s failed to open PF_INET SOCK_STREAM socket: (%d) %s\n",
                   ' ', __func__, errno, strerror(errno));
        }
        return false;
    }

    if (DEBUG_OUTPUT) {
        printf("%9c %s create socket %d port %u\n",
               ' ', __func__, fwd_state->fd, fwd_state->port);
    }

    return true;
}

/**
 * @function connector_Fwd_Metis_SetupSocket
 * @abstract Creates the socket and sets the port, but does not call connect
 * @discussion
 *   Creates and sets up the socket descriptor.  makes it non-blocking.
 *   Sets the port in FwdMetisState.
 *
 * This is a full PF_INET socket, not forced to PF_LOCAL.
 *
 * The sendbuffer size is set to METIS_OUTPUT_QUEUE_BYTES
 *
 * precondition: called _openSocket
 *
 * @param <#param1#>
 * @return <#return#>
 */
static bool
_setupSocket(FwdMetisState *fwd_state)
{
    trapUnexpectedStateIf(fwd_state->fd < 1, "Invalid socket %d", fwd_state->fd);

    // Set non-blocking flag
    int flags = fcntl(fwd_state->fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int res = fcntl(fwd_state->fd, F_SETFL, flags | O_NONBLOCK);

    if (res < 0) {
        if (DEBUG_OUTPUT) {
            printf("%9c %s failed to make socket non-blocking: (%d) %s\n",
                   ' ', __func__, errno, strerror(errno));
        }

        close(fwd_state->fd);
        return false;
    }

    const int sendBufferSize = METIS_SEND_SOCKET_BUFFER;
    res = setsockopt(fwd_state->fd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(int));
    if (res < 0) {
        if (DEBUG_OUTPUT) {
            printf("%9c %s failed to set SO_SNDBUF to %d: (%d) %s\n",
                   ' ', __func__, sendBufferSize, errno, strerror(errno));
        }
        // This is a non-fatal error
    }

#if defined(SO_NOSIGPIPE)
    // turn off SIGPIPE, return EPIPE
    const int on = 1;
    res = setsockopt(fwd_state->fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
    if (res < 0) {
        if (DEBUG_OUTPUT) {
            printf("%9c %s failed to set SO_NOSIGPIPE to %d: (%d) %s\n",
                   ' ', __func__, sendBufferSize, errno, strerror(errno));
        }
        // this is not a fatal error, so keep going
    }
#endif

    return true;
}

/**
 * @function connector_Fwd_Metis_SetupConnectionBuffer
 * @abstract Creates the connection buffer and adds it to libevent
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
static bool
_setupSocketEvents(FwdMetisState *fwd_state, RtaConnection *conn)
{
    RtaProtocolStack *stack = rtaConnection_GetStack(conn);
    PARCEventScheduler *scheduler = rtaFramework_GetEventScheduler(rtaProtocolStack_GetFramework(stack));

    // the connect() call will be asynchrnous because the socket is non-blocking, so we
    // need ET_WRITE to trigger a callback when the socket becomes writable (i.e. connected).
    // If there's an error on connect it will be an ET_READ | ET_WRITE event with an error on the socket.
    fwd_state->readEvent = parcEvent_Create(scheduler, fwd_state->fd, PARCEventType_Read | PARCEventType_Persist | PARCEventType_EdgeTriggered, _eventCallback, conn);
    assertNotNull(fwd_state->readEvent, "Got a null readEvent for socket %d", fwd_state->fd);

    fwd_state->writeEvent = parcEvent_Create(scheduler, fwd_state->fd, PARCEventType_Write | PARCEventType_Persist | PARCEventType_EdgeTriggered, _eventCallback, conn);
    assertNotNull(fwd_state->writeEvent, "Got a null readEvent for socket %d", fwd_state->fd);

    // Start the write event.  It will be signaled on a connect error or when we are connected.
    // The read event is not enabled until after connect.

    int failure = parcEvent_Start(fwd_state->writeEvent);
    assertFalse(failure < 0, "Error starting writeEvent event %p: (%d) %s", (void *) fwd_state->writeEvent, errno, strerror(errno));

    return true;
}

/**
 * The connection to the forwarder succeeded, step the state machine
 *
 * Change the state of the connection to connected and notify the user that it's ready.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_connectionSucceeded(FwdMetisState *fwd_state, RtaConnection *conn)
{
    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s Connection %p connected fd %d\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) conn, fwd_state->fd);
    }

    fwd_state->isConnected = true;

    // enable read events
    parcEvent_Start(fwd_state->readEvent);

    rtaConnection_SendStatus(conn, FWD_METIS, RTA_UP, notifyStatusCode_CONNECTION_OPEN, NULL, NULL);
}

static void
_readInEnvironmentConnectionSpecification(struct sockaddr_in *addr_in)
{
    char *forwarderIpEnv = getenv(FORWARDER_CONNECTION_ENV);
    if (forwarderIpEnv == NULL) {
        return;
    }

    char forwarderIpAddress[NI_MAXHOST] = { 0 };
    in_port_t forwarderIpPort = 0;

    // Currently, we only support tcp control connections to the forwarder
    sscanf(forwarderIpEnv, "tcp://%[^:]:%hu", forwarderIpAddress, &forwarderIpPort);

    // If provided, use the specified address in a canonical form
    if (forwarderIpAddress[0] != '\0') {
        // Normalize the provided hostname
        struct sockaddr_in *addr = (struct sockaddr_in *) parcNetwork_SockAddress(forwarderIpAddress, forwarderIpPort);
        char *ipAddress = inet_ntoa(addr->sin_addr);
        parcMemory_Deallocate(&addr);
        if (ipAddress) {
            addr_in->sin_addr.s_addr = inet_addr(ipAddress);
        } else {
            addr_in->sin_addr.s_addr = inet_addr(forwarderIpAddress);
        }
    }

    // If provided, use the specified port
    if (forwarderIpPort != 0) {
        addr_in->sin_port = htons(forwarderIpPort);
    }
}

/**
 * @function connector_Fwd_Metis_BeginConnect
 * @abstract Begins the non-blocking connect() call to 127.0.0.1 on the port in FwdMetisState
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
static bool
connector_Fwd_Metis_BeginConnect(FwdMetisState *fwd_state, RtaConnection *conn)
{
    bool success = false;

    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_port = htons(fwd_state->port);
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Override defaults if specified
    _readInEnvironmentConnectionSpecification(&addr_in);

    if (DEBUG_OUTPUT) {
        char inetAddress[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr_in.sin_addr), inetAddress, INET_ADDRSTRLEN);
        printf("%9" PRIu64 " %s beginning connect socket %d to port %d on %s\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               fwd_state->fd,
               fwd_state->port,
               inetAddress);
    }

    // This will deliver a PARCEventType_Write event on connect success
    int res = connect(fwd_state->fd, (struct sockaddr*) &addr_in, (socklen_t) sizeof(addr_in));

    if (res == 0) {
        // connect succeded immediately
        _connectionSucceeded(fwd_state, conn);
        success = true;
    } else if (errno == EINPROGRESS) {
        // connection is deferred
        success = true;
    } else {
        // a hard error
        printf("Error connecting: (%d) %s\n", errno, strerror(errno));
    }

    return success;
}

/**
 * We maintain an input queue going up the stack and only dequeue a small number of packets
 * with each call from the dispatch loop.  THis is to avoid bursting a bunch of packets up the stack.
 */
static void
connector_Fwd_Metis_Dequeue(int fd, PARCEventType which_event, void *metisStateVoid)
{
    FwdMetisState *fwd_state = (FwdMetisState *) metisStateVoid;

    // random small number.  What is right value for this?
    unsigned max_loops = 6;

    if (DEBUG_OUTPUT) {
        printf("%9d %s deque size %zu\n",
               0,
               __func__,
               parcDeque_Size(fwd_state->transportMessageQueue));
    }

    while (max_loops > 0 && !parcDeque_IsEmpty(fwd_state->transportMessageQueue)) {
        max_loops--;
        TransportMessage *tm = parcDeque_RemoveFirst(fwd_state->transportMessageQueue);

        RtaConnection *conn = rtaConnection_GetFromTransport(tm);
        RtaProtocolStack *stack = rtaConnection_GetStack(conn);
        PARCEventQueue  *out = rtaProtocolStack_GetPutQueue(stack, FWD_METIS, RTA_UP);
        RtaComponentStats *stats = rtaConnection_GetStats(conn, FWD_METIS);

        if (rtaComponent_PutMessage(out, tm)) {
            rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
        }
    }

    // If there are still messages in there, re-schedule
    if (!parcDeque_IsEmpty(fwd_state->transportMessageQueue)) {
        if (DEBUG_OUTPUT) {
            printf("%9d %s rescheduling output queue timer %p\n",
                   0,
                   __func__,
                   (void *) fwd_state->transportMessageQueueEvent);
        }

        struct timeval immediateTimeout = { 0, 0 };
        parcEventTimer_Start(fwd_state->transportMessageQueueEvent, &immediateTimeout);
    }
}

/**
 * Create a TCP socket
 * Set it non-blocking
 * Wrap it in a buffer event
 * Set Read and Event callbacks
 *
 * Return 0 success, -1 failure
 */
static int
connector_Fwd_Metis_Opener(RtaConnection *conn)
{
    bool success = false;

    uint16_t port = metisForwarder_GetPortFromConfig(rtaConnection_GetParameters(conn));

    PARCEventScheduler *scheduler = rtaFramework_GetEventScheduler(rtaConnection_GetFramework(conn));
    FwdMetisState *fwd_state = connector_Fwd_Metis_CreateConnectionState(scheduler);

    if (_openSocket(fwd_state, port)) {
        if (_setupSocket(fwd_state)) {
            if (_setupSocketEvents(fwd_state, conn)) {
                if (connector_Fwd_Metis_BeginConnect(fwd_state, conn)) {
                    // stash it away in the per-connection cubby hole
                    rtaConnection_SetPrivateData(conn, FWD_METIS, fwd_state);
                    success = true;
                }
            }
        }
    }

    if (!success) {
        if (fwd_state->fd) {
            close(fwd_state->fd);
        }
        if (fwd_state->readEvent) {
            parcEvent_Destroy(&(fwd_state->readEvent));
        }
        if (fwd_state->writeEvent) {
            parcEvent_Destroy(&(fwd_state->writeEvent));
        }
        parcMemory_Deallocate((void **) &fwd_state);
        return -1;
    }

    // Socket will be ready for use once we get PARCEventQueue_Connected
    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s open conn %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) conn);
    }

    return 0;
}

/**
 * We received a Metis control packet.  Translate it to a control packet and send it up the stack.
 */
static void
receiveControlMessage(PacketData *data)
{
    CCNxTlvDictionary *packetDictionary =
        ccnxWireFormatMessage_FromControlPacketType(data->fwd_state->nextMessage.version, data->fwd_state->nextMessage.packet);

    bool success = ccnxCodecTlvPacket_BufferDecode(data->fwd_state->nextMessage.packet, packetDictionary);

    if (success) {
        TransportMessage *tm = transportMessage_CreateFromDictionary(packetDictionary);
        transportMessage_SetInfo(tm, rtaConnection_Copy(data->conn), rtaConnection_FreeFunc);

        // send it up the stack
        if (rtaComponent_PutMessage(data->out, tm)) {
            rtaComponentStats_Increment(data->stats, STATS_UPCALL_OUT);
            data->fwd_state->stats.countUpcallWriteControlOk++;
        } else {
            data->fwd_state->stats.countUpcallWriteControlError++;
        }
    } else {
        assertTrue(success, "Error decoding a Metis control packet\n")
        {
            parcBuffer_Display(data->fwd_state->nextMessage.packet, 3);
        }
    }

    // we are now done with our references
    ccnxTlvDictionary_Release(&packetDictionary);
}


static void
_queueNonControl(PacketData *data)
{
    CCNxTlvDictionary *packetDictionary = ccnxWireFormatMessage_Create(data->fwd_state->nextMessage.packet);

    assertNotNull(packetDictionary, "Got a null packet decode")
    {
        parcBuffer_Display(data->fwd_state->nextMessage.packet, 3);
    }

    TransportMessage *tm = transportMessage_CreateFromDictionary(packetDictionary);

    // add the connection info to the transport message before sending up stack
    transportMessage_SetInfo(tm, rtaConnection_Copy(data->conn), rtaConnection_FreeFunc);

    parcDeque_Append(data->fwd_state->transportMessageQueue, tm);

    // start if went from emtpy to 1
    if (parcDeque_Size(data->fwd_state->transportMessageQueue) == 1) {
        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s connection %u schedule dequeue event %p\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(data->conn))),
                   __func__,
                   rtaConnection_GetConnectionId(data->conn),
                   (void *) data->fwd_state->transportMessageQueueEvent);
        }

        struct timeval immediateTimeout = { 0, 0 };
        parcEventTimer_Start(data->fwd_state->transportMessageQueueEvent, &immediateTimeout);
    }

    // we are now done with our references
    ccnxTlvDictionary_Release(&packetDictionary);
}

/**
 * Receive a non-control packet
 *
 * Non-control messages may be dropped due to lack of input buffer space.
 * If the connection has state Block Up or the up queue's length is
 * too many messages deep, the non-control message will be dropped.
 *
 * precondition: the caller knows the message is not a control message
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_receiveNonControl(PacketData *data)
{
    if (rtaConnection_BlockedUp(data->conn)) {
        data->fwd_state->stats.countUpcallWriteDataBlocked++;
        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s connection %u blocked up, drop wireFormat %p\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(data->conn))),
                   __func__,
                   rtaConnection_GetConnectionId(data->conn),
                   (void *) data->fwd_state->nextMessage.packet);
        }
    } else {
        if (parcDeque_Size(data->fwd_state->transportMessageQueue) < METIS_INPUT_QUEUE_MESSAGES) {
            _queueNonControl(data);
            data->fwd_state->stats.countUpcallWriteDataOk++;
        } else {
            data->fwd_state->stats.countUpcallWriteDataQueueFull++;
            if (DEBUG_OUTPUT) {
                printf("%9" PRIu64 " %s connection %u input buffer full, drop wireFormat %p\n",
                       rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(data->conn))),
                       __func__,
                       rtaConnection_GetConnectionId(data->conn),
                       (void *) data->fwd_state->nextMessage.packet);
            }
        }
    }
}

/**
 * We received an entire packet, send it up the stack in a Transport message.
 *
 * If its a control message, we make it a CCNxControlMessage here for symmetry with us
 * encoding the control messages at this level
 */
static void
connector_Fwd_Metis_SendUpStack(PacketData *data)
{
    // Always send control messages up the stack
    if (data->fwd_state->nextMessage.packetType == PacketType_Control) {
        receiveControlMessage(data);
    } else {
        _receiveNonControl(data);
    }
}

/**
 * Return the SO_ERROR value for the given socket
 *
 * If getsockopt returns an error, the return code could be the error from getsockopt.
 *
 * Typically you will get ECONNREFUSED when you cannot connect and one of the many getsockopt
 * errors if there's a problem with the actual socket.
 *
 * @param [in] fd The socket
 *
 * @return errno An errno value
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static int
_getSocketError(int fd)
{
    int value;
    socklen_t valueLength = sizeof(value);
    int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &value, &valueLength);
    if (res < 0) {
        value = res;
    }
    return value;
}

/**
 * Received an event on a socket we have marked as not yet connected
 *
 * Ether it's ready to go or there's an error.  We will receive a PARCEventType_Read and the socket
 * will have an SO_ERROR of 0 if it's now connected.  If the SO_ERROR is non-zero, there
 * was an error on connect.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_disconnectedEventHandler(FwdMetisState *fwd_state, RtaConnection *conn, PARCEventType what)
{
    if (what & PARCEventType_Read) {
        int socketError = _getSocketError(fwd_state->fd);
        if (socketError == 0) {
            // I don't think these happen, they will be write events
            _connectionSucceeded(fwd_state, conn);
        } else {
            // error on connect
            printf("%9" PRIu64 " %s Connection %p got error on SOCK_STREAM, fd %d: %s\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                   __func__,
                   (void *) conn,
                   fwd_state->fd,
                   strerror(errno));

            // make the event non-pending
            parcEvent_Stop(fwd_state->readEvent);
            parcEvent_Stop(fwd_state->writeEvent);

            rtaConnection_SetBlockedDown(conn);

            // at least tell the API whats going on
            rtaConnection_SendStatus(conn, FWD_METIS, RTA_UP, notifyStatusCode_FORWARDER_NOT_AVAILABLE, NULL, NULL);
        }
    }

    if (what & PARCEventType_Write) {
        int socketError = _getSocketError(fwd_state->fd);
        if (socketError == 0) {
            _connectionSucceeded(fwd_state, conn);
        }
    }
}

static void
_setupNextPacketV1(FwdMetisState *fwd_state)
{
    switch (fwd_state->nextMessage.fixedHeader.v1.packetType) {
        case CCNxCodecSchemaV1Types_PacketType_Interest:
            fwd_state->nextMessage.packetType = PacketType_Interest;
            break;
        case CCNxCodecSchemaV1Types_PacketType_ContentObject:
            fwd_state->nextMessage.packetType = PacketType_ContentObject;
            break;
        case CCNxCodecSchemaV1Types_PacketType_Control:
            fwd_state->nextMessage.packetType = PacketType_Control;
            break;
        case CCNxCodecSchemaV1Types_PacketType_InterestReturn:
            fwd_state->nextMessage.packetType = PacketType_InterestReturn;
            break;
        default:
            fwd_state->nextMessage.packetType = PacketType_Unknown;
            break;
    }

    size_t fixedHeaderLength = sizeof(CCNxCodecSchemaV1FixedHeader);
    fwd_state->nextMessage.length = htons(fwd_state->nextMessage.fixedHeader.v1.packetLength);

    fwd_state->nextMessage.packet = parcBuffer_Allocate(fwd_state->nextMessage.length);
    assertNotNull(fwd_state->nextMessage.packet, "Could not allocate packet of size %zu", fwd_state->nextMessage.length);

    // finally copy in the fixed header as we have already read that in
    parcBuffer_PutArray(fwd_state->nextMessage.packet, fixedHeaderLength, fwd_state->nextMessage.fixedHeader.buffer);
}

/**
 * Called after reading whole FixedHeader, will setup the packet buffer
 *
 * After reading the fixed header, we need to allocate a PARCBuffer for the packet.  Setup that
 * buffer and copy the FixedHeader in to it.  Remaining reads will go in to this buffer.
 *
 * After this function completes, the parsed version, packetType, and length of the nextMessage will
 * be filled in, the packet buffer allocated and the fixedHeader copied to that packet buffer.
 *
 * precondition: forwarder->nextMessage.remainingReadLength == 0 && fwd_state->nextMessage.packet == NULL
 *
 * @param [in] fwd_state An allocated forwarder connection state that has read in the fixed header
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_setupNextPacket(FwdMetisState *fwd_state)
{
    trapUnexpectedStateIf(fwd_state->nextMessage.packet != NULL, "Calling _setupNextPacket but the packet field is not NULL");

    fwd_state->nextMessage.version = fwd_state->nextMessage.fixedHeader.buffer[0];

    switch (fwd_state->nextMessage.version) {
        case 1:
            _setupNextPacketV1(fwd_state);
            break;

        default:
            trapUnexpectedState("Illegal packet version %d", fwd_state->nextMessage.version)
            {
                _nextMessage_Display(&fwd_state->nextMessage, 0);
            }
            break;
    }
}

/**
 * Reads the FixedHeader.  If full read will setup the next packet buffer.
 *
 * Reads up to FixedHeader length bytes.  If read whole header will allocate the next packet
 * buffer to right size and copy the Fixed Header in to the buffer.
 *
 * preconditions:
 * - fwd_state->nextMessage.packet should be NULL
 * - fwd_state->nextMessage.remainingReadLength should be the remaining bytes to read of the Fixed Header
 * - fwd_state->nextMessage.readLocation should point to the location in the FixedHeader to start reading
 *
 * postconditions:
 * - fwd_state->nextMessage.remainingReadLength will be decremented by the amount read
 * - If remainingReadLength is decremented to 0, will allocate fwd_state->nextMessage.packet and copy in the FixedHeader
 * - The fields in fwd_state->nextMessage (length, packetType, version) will be set based on the fixed header
 *
 * @param [in] fwd_state An allocated forwarder connection state
 *
 * @retval ReadReturnCode_Finished one entire packet is ready in the buffer
 * @retval ReadReturnCode_PartialRead need more bytes
 * @retval ReadRetrunCode_Closed The socket to metis is closed (a special case of Error)
 * @retval ReadReturnCode_Error An error occured on the socket to metis
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static ReadReturnCode
_readPacketHeader(FwdMetisState *fwd_state)
{
    ReadReturnCode returnCode = ReadReturnCode_Error;

    // This could be switched to MSG_PEEK instead of copying later, but I don't think it makes any significant change.
    ssize_t nread = recv(fwd_state->fd, fwd_state->nextMessage.readLocation, fwd_state->nextMessage.remainingReadLength, 0);
    if (nread > 0) {
        // recv will always runturn at most fwd_state->nextMessage.remainingReadLength, so this won't wrap around to negative.
        fwd_state->nextMessage.remainingReadLength -= nread;

        if (fwd_state->nextMessage.remainingReadLength == 0) {
            returnCode = ReadReturnCode_Finished;
            _setupNextPacket(fwd_state);
        } else {
            fwd_state->nextMessage.readLocation += nread;
            returnCode = ReadReturnCode_PartialRead;
        }
    } else if (nread == 0) {
        // the connection is closed
        returnCode = ReadReturnCode_Closed;
    } else {
        switch (errno) {
            case EAGAIN:
                // call would block.  These can happen becasue _readMessage is in a while loop and we detect
                // the end of the loop because we cannot read another fixed header.
                returnCode = ReadReturnCode_PartialRead;
                break;

            default:
                // an error.  I think all errors will be hard errors and we close the connection
                if (DEBUG_OUTPUT) {
                    printf("%9c %s socket %d recv error: (%d) %s\n",
                           ' ', __func__, fwd_state->fd, errno, strerror(errno));
                }
                returnCode = ReadReturnCode_Error;
                break;
        }
    }

    return returnCode;
}


/**
 * We have finished reading the fixed header, reading the message body
 *
 * Will modify the nextMessage.packet buffer.  When the buffer has 0 remaining, the whole packet has been read
 *
 * precondition: _readHeaderFromMetis read the header and allocated the packet buffer
 *
 * @param [in] fwd_state An allocated forwarder connection state
 *
 * @retval ReadReturnCode_Finished one entire packet is ready in the buffer
 * @retval ReadReturnCode_PartialRead need more bytes
 * @retval ReadRetrunCode_Closed The socket to metis is closed (a special case of Error)
 * @retval ReadReturnCode_Error An error occured on the socket to metis
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static ReadReturnCode
_readPacketBody(FwdMetisState *fwd_state)
{
    ReadReturnCode returnCode = ReadReturnCode_Error;

    trapUnexpectedStateIf(fwd_state->nextMessage.packet == NULL, "Trying to read a message with a null packet buffer");

    size_t remaining = parcBuffer_Remaining(fwd_state->nextMessage.packet);

    if (DEBUG_OUTPUT) {
        printf("%9c %s socket %d read up to %zu bytes\n",
               ' ', __func__, fwd_state->fd, remaining);
    }

    void *overlay = parcBuffer_Overlay(fwd_state->nextMessage.packet, 0);
    ssize_t nread = recv(fwd_state->fd, overlay, remaining, 0);

    if (nread > 0) {
        // good read
        parcBuffer_SetPosition(fwd_state->nextMessage.packet, parcBuffer_Position(fwd_state->nextMessage.packet) + nread);

        if (nread == remaining) {
            returnCode = ReadReturnCode_Finished;
        } else {
            returnCode = ReadReturnCode_PartialRead;
        }
    } else if (nread == 0) {
        // connection closed
        returnCode = ReadReturnCode_Closed;
    } else {
        switch (errno) {
            case EAGAIN:
                // call would block.  These can happen becasue _readMessage is in a while loop and we detect
                // the end of the loop because we cannot read the entire message body.
                returnCode = ReadReturnCode_PartialRead;
                break;

            default:
                // an error.  I think all errors will be hard errors and we close the connection
                if (DEBUG_OUTPUT) {
                    printf("%9c %s socket %d recv error: (%d) %s\n",
                           ' ', __func__, fwd_state->fd, errno, strerror(errno));
                }
                returnCode = ReadReturnCode_Error;
        }
    }


    if (DEBUG_OUTPUT) {
        printf("%9c %s socket %u msg_length %zu read_length %zd remaining %zu\n",
               ' ',
               __func__,
               fwd_state->fd,
               fwd_state->nextMessage.length,
               nread,
               parcBuffer_Remaining(fwd_state->nextMessage.packet));
    }

    return returnCode;
}

/**
 * Read packet from metis
 *
 * Reads the fixed heder.  Once fixed header is done, begins reading the packet body.  Keeps
 * all the incremental state to do partial reads.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval ReadReturnCode_Finished one entire packet is ready in the buffer
 * @retval ReadReturnCode_PartialRead need more bytes
 * @retval ReadRetrunCode_Closed The socket to metis is closed (a special case of Error)
 * @retval ReadReturnCode_Error An error occured on the socket to metis
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static ReadReturnCode
_readPacket(FwdMetisState *fwd_state)
{
    ReadReturnCode returnCode = ReadReturnCode_PartialRead;

    // are we still reading the header?
    if (fwd_state->nextMessage.remainingReadLength > 0) {
        returnCode = _readPacketHeader(fwd_state);
    } else {
        returnCode = ReadReturnCode_Finished;
    }

    // After reading the header, it may be possible to read the body too
    if (returnCode == ReadReturnCode_Finished && fwd_state->nextMessage.remainingReadLength == 0) {
        returnCode = _readPacketBody(fwd_state);
    }

    return returnCode;
}

/**
 * Read as many packets as we can from Metis
 *
 * Will read the stream socket from metis until we get a PartialRead return code from
 * either the attempt to read the header or the body.
 *
 * On read error, will send a notification message the connection is closed up to
 * the API and will disable read and write events.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_readFromMetis(FwdMetisState *fwd_state, RtaConnection *conn)
{
    RtaProtocolStack *stack = rtaConnection_GetStack(conn);
    RtaComponentStats *stats = rtaConnection_GetStats(conn, FWD_METIS);

    ReadReturnCode readCode;
    while ((readCode = _readPacket(fwd_state)) == ReadReturnCode_Finished) {
        rtaComponentStats_Increment(stats, STATS_UPCALL_IN);
        fwd_state->stats.countUpcallReads++;

        // setup the buffer for reading
        parcBuffer_Flip(fwd_state->nextMessage.packet);

        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s sending packet buffer %p up stack length %zu\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                   __func__,
                   (void *) fwd_state->nextMessage.packet,
                   parcBuffer_Remaining(fwd_state->nextMessage.packet));
        }

        // this is just to make the signature of connector_Fwd_Metis_SendUpStack tractable, PacketData
        // is not exposed outside this scope.

        PARCEventQueue  *out = rtaProtocolStack_GetPutQueue(stack, FWD_METIS, RTA_UP);
        PacketData data = {
            .fwd_state = fwd_state,
            .conn      = conn,
            .out       = out,
            .stats     = stats,
        };

        connector_Fwd_Metis_SendUpStack(&data);

        // done with the packet buffer.  Release our hold on it.  If it was sent up the stack
        // another reference count was made.
        parcBuffer_Release(&fwd_state->nextMessage.packet);

        // now setup for next packet
        _initializeNextMessage(&fwd_state->nextMessage);
    }

    if (readCode == ReadReturnCode_Closed) {
        fwd_state->isConnected = false;
        parcEvent_Stop(fwd_state->readEvent);
        parcEvent_Stop(fwd_state->writeEvent);
        rtaConnection_SendStatus(conn, FWD_METIS, RTA_UP, notifyStatusCode_CONNECTION_CLOSED, NULL, "Socket operation returned closed by remote");
    } else if (readCode == ReadReturnCode_Error) {
        fwd_state->isConnected = false;
        parcEvent_Stop(fwd_state->readEvent);
        parcEvent_Stop(fwd_state->writeEvent);
        rtaConnection_SendStatus(conn, FWD_METIS, RTA_UP, notifyStatusCode_CONNECTION_CLOSED, NULL, "Socket operation returned error");
    }

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s total upcall reads in %" PRIu64 " out %" PRIu64 "\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               rtaComponentStats_Get(stats, STATS_UPCALL_IN),
               rtaComponentStats_Get(stats, STATS_UPCALL_OUT));
    }
}

/**
 * Append a vector to the buffer
 *
 * @param [in] wireFormat The wire format packet, assumes current position is start of packet
 * @param [in] fwd_output The libevent buffer to add the memory reference to
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_queueIoVecMessageToMetis(CCNxCodecNetworkBufferIoVec *vec, PARCEventBuffer *fwd_output)
{
    fwd_metis_references_queued++;

    int iovcnt = ccnxCodecNetworkBufferIoVec_GetCount(vec);
    const struct iovec *array = ccnxCodecNetworkBufferIoVec_GetArray(vec);

    for (int i = 0; i < iovcnt; i++) {
        if (parcEventBuffer_Append(fwd_output, array[i].iov_base, array[i].iov_len) < 0) {
            trapUnrecoverableState("%s error writing to bev_local", __func__);
        }
    }
}

/**
 * Append to the buffer
 *
 * @param [in] wireFormat The wire format packet, assumes current position is start of packet
 * @param [in] fwd_output The libevent buffer to add the memory reference to
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_queueBufferMessageToMetis(PARCBuffer *wireFormat, PARCEventBuffer *fwd_output)
{
    fwd_metis_references_queued++;

    void *overlay = parcBuffer_Overlay(wireFormat, 0);
    size_t length = parcBuffer_Remaining(wireFormat);

    if (parcEventBuffer_Append(fwd_output, overlay, length) < 0) {
        trapUnrecoverableState("%s error writing to bev_local", __func__);
    }
}

/**
 * Write as much as possible from the output buffer to metis
 *
 * Write as much as we can to metis.  If there is nothing left, deactivate the write event.
 * If there is still bytes left in the output buffer, activate the write event.
 *
 * postconditions:
 * - Write as many bytes as possible from the output buffer to metis
 * - If there are still bytes remaining, enable the write event
 * - If there are no bytes remaining, disable the write event.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_dequeueMessagesToMetis(FwdMetisState *fwdConnState)
{
    // if we try to write a 0 length buffer, write will return -1 like an error
    if (parcEventBuffer_GetLength(fwdConnState->metisOutputQueue) > 0) {
        fwdConnState->stats.countDowncallWrites++;
        int nwritten = parcEventBuffer_WriteToFileDescriptor(fwdConnState->metisOutputQueue, fwdConnState->fd, -1);
        if (nwritten < 0) {
            // an error
            trapNotImplemented("Bugzid: 2194");
        }

        if (DEBUG_OUTPUT) {
            printf("%9c %s wrote %d bytes to socket %d, %zu bytes remaining\n",
                   ' ',
                   __func__,
                   nwritten,
                   fwdConnState->fd,
                   parcEventBuffer_GetLength(fwdConnState->metisOutputQueue));
        }

        // if we could not write the whole buffer, make sure we have a write event pending
        if (parcEventBuffer_GetLength(fwdConnState->metisOutputQueue) > 0) {
            parcEvent_Start(fwdConnState->writeEvent);
            if (DEBUG_OUTPUT) {
                printf("%9c %s enabled write event\n", ' ', __func__);
            }
        } else {
            parcEvent_Stop(fwdConnState->writeEvent);
            if (DEBUG_OUTPUT) {
                printf("%9c %s disabled write event\n", ' ', __func__);
            }
        }
    }
}


/**
 * Called when we get an event on a socket we believe is connected
 *
 * libevent will call this with an PARCEventType_Read on connection close too (the read length will be 0).
 *
 * @param [in] fwd_state An allocated forwarder connection state
 * @param [in] conn The corresponding RTA connection
 * @param [in] what The Libevent set of events
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_connectedEventHandler(FwdMetisState *fwd_state, RtaConnection *conn, short what)
{
    if (what & PARCEventType_Read) {
        _readFromMetis(fwd_state, conn);
    }

    if (what & PARCEventType_Write) {
        _dequeueMessagesToMetis(fwd_state);
    }
}

/**
 * Called for any activity on the socket.  Maybe in either connected or disconnected state.
 */
static void
_eventCallback(int fd, PARCEventType what, void *connectionVoid)
{
    RtaConnection *conn = (RtaConnection *) connectionVoid;
    FwdMetisState *fwd_state = (FwdMetisState *) rtaConnection_GetPrivateData(conn, FWD_METIS);;

    if (!fwd_state->isConnected) {
        _disconnectedEventHandler(fwd_state, conn, what);

        // once we connect, we should try a read immediately too
    }

    if (fwd_state->isConnected) {
        _connectedEventHandler(fwd_state, conn, what);
    }
}

/**
 * Updates the connections's Blocked Down state
 *
 * If the bytes in our output buffer are greater than METIS_OUTPUT_QUEUE_BYTES, then
 * we will set the Blocked Down condition on the connection.  This will prevent the
 * API connector from accepting more messages.
 *
 * Messages already in the connection queue will still be processed.
 *
 * @param [in] fwd_output The libevent buffer to check the backlog
 * @param [in] conn The RtaConnection the set or clear the blocked down condition
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_updateBlockedDownState(PARCEventBuffer *fwd_output, RtaConnection *conn)
{
    size_t queue_bytes = parcEventBuffer_GetLength(fwd_output);
    if (queue_bytes > METIS_OUTPUT_QUEUE_BYTES) {
        // block down

        if (!rtaConnection_BlockedDown(conn)) {
            rtaConnection_SetBlockedDown(conn);
        }

        // note that we continue execution and put the packet we have in hand on the queue
        // setting the blocked down state only affects the API connector.  Packets already in the system
        // will keep flowing down to us
    } else {
        // if it is blocked, unblock it
        if (rtaConnection_BlockedDown(conn)) {
            rtaConnection_ClearBlockedDown(conn);
        }
    }
}

static void
connector_Fwd_Metis_Downcall_HandleConnected(FwdMetisState *fwdConnState, TransportMessage *tm, RtaConnection *conn, RtaComponentStats *stats)
{
    _updateBlockedDownState(fwdConnState->metisOutputQueue, conn);

    CCNxTlvDictionary *dictionary = transportMessage_GetDictionary(tm);

    bool queued = false;

    CCNxCodecNetworkBufferIoVec *vec = ccnxWireFormatMessage_GetIoVec(dictionary);
    if (vec != NULL) {
        _queueIoVecMessageToMetis(vec, fwdConnState->metisOutputQueue);
        queued = true;
    } else {
        PARCBuffer *wireFormat = ccnxWireFormatMessage_GetWireFormatBuffer(dictionary);
        if (wireFormat != NULL) {
            _queueBufferMessageToMetis(wireFormat, fwdConnState->metisOutputQueue);
            queued = true;
        }
    }

    if (queued) {
        rtaComponentStats_Increment(stats, STATS_DOWNCALL_OUT);

        if (DEBUG_OUTPUT) {
            struct timeval delay = transportMessage_GetDelay(tm);
            printf("%9" PRIu64 " %s total downcall reads %" PRIu64 " references queued %u dequeued %u not queued %u last delay %.6f\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                   __func__,
                   rtaComponentStats_Get(stats, STATS_DOWNCALL_IN),
                   fwd_metis_references_queued,
                   fwd_metis_references_dequeued,
                   fwd_metis_references_notqueued,
                   delay.tv_sec + delay.tv_usec * 1E-6);
        }
    } else {
        fwd_metis_references_notqueued++;
    }

    // The transport message is destroyed in connector_Fwd_Metis_Downcall_Read()
}

static void
_ackRequest(RtaConnection *conn, PARCJSON *request)
{
    PARCJSON *response = cpiAcks_CreateAck(request);
    CCNxTlvDictionary *ackDict = ccnxControlFacade_CreateCPI(response);

    TransportMessage *tm_ack = transportMessage_CreateFromDictionary(ackDict);
    ccnxTlvDictionary_Release(&ackDict);
    parcJSON_Release(&response);

    transportMessage_SetInfo(tm_ack, rtaConnection_Copy(conn), rtaConnection_FreeFunc);

    RtaProtocolStack *stack = rtaConnection_GetStack(conn);
    PARCEventQueue  *out = rtaProtocolStack_GetPutQueue(stack, FWD_METIS, RTA_UP);
    if (rtaComponent_PutMessage(out, tm_ack)) {
        RtaComponentStats *stats = rtaConnection_GetStats(conn, FWD_METIS);
        rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
    }
}

static bool
_handleDownControl(FwdMetisState *fwdConnState, RtaConnection *conn, TransportMessage *tm)
{
    bool consumedMessage = false;

    CCNxTlvDictionary *dict = transportMessage_GetDictionary(tm);
    if (ccnxTlvDictionary_IsControl(dict)) {
        if (ccnxControlFacade_IsCPI(dict)) {
            PARCJSON *json = ccnxControlFacade_GetJson(dict);
            if (controlPlaneInterface_GetCPIMessageType(json) == CPI_REQUEST) {
                if (cpi_getCPIOperation2(json) == CPI_PAUSE) {
                    if (DEBUG_OUTPUT) {
                        printf("%9" PRIu64 " %s conn %p recieved PAUSE\n",
                               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                               __func__,
                               (void *) conn);
                    }
                    _ackRequest(conn, json);
                    consumedMessage = true;
                }

                if (cpi_getCPIOperation2(json) == CPI_FLUSH) {
                    if (DEBUG_OUTPUT) {
                        printf("%9" PRIu64 " %s conn %p recieved FLUSH\n",
                               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                               __func__,
                               (void *) conn);
                    }
                    _ackRequest(conn, json);
                    consumedMessage = true;
                }
            }
        }
    }

    if (consumedMessage) {
        fwdConnState->stats.countDowncallControl++;
    }

    return consumedMessage;
}

/**
 * send raw packet from codec to forwarder.  We are passed the ProtocolStack on the ptr.
 */
static void
connector_Fwd_Metis_Downcall_Read(PARCEventQueue *in, PARCEventType event, void *ptr)
{
    TransportMessage *tm;

    while ((tm = rtaComponent_GetMessage(in)) != NULL) {
        RtaConnection *conn = rtaConnection_GetFromTransport(tm);
        FwdMetisState *fwdConnState = rtaConnection_GetPrivateData(conn, FWD_METIS);
        RtaComponentStats *stats = rtaConnection_GetStats(conn, FWD_METIS);
        rtaComponentStats_Increment(stats, STATS_DOWNCALL_IN);
        fwdConnState->stats.countDowncallReads++;

        bool consumedControl = _handleDownControl(fwdConnState, conn, tm);
        if (!consumedControl) {
            // we did not consume the message as a control packet for the metis connector

            if (fwdConnState->isConnected) {
                // If the socket is connected, this will "do the right thing" and consume the transport message.
                connector_Fwd_Metis_Downcall_HandleConnected(fwdConnState, tm, conn, stats);
            } else {
                // Oops, got a packet before we're connected.
                printf("\nConnection %p transport message %p on fd %d that's not open\n", (void *) conn, (void *) tm, fwdConnState->fd);
            }

            // now attempt to write to the network
            _dequeueMessagesToMetis(fwdConnState);

            if (DEBUG_OUTPUT) {
                printf("%9" PRIu64 " %s total downcall reads in %" PRIu64 " out %" PRIu64 "\n",
                       rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                       __func__,
                       rtaComponentStats_Get(stats, STATS_DOWNCALL_IN),
                       rtaComponentStats_Get(stats, STATS_DOWNCALL_OUT));
            }
        }

        transportMessage_Destroy(&tm);
    }
}

/**
 * Destroy the FwdMetisState object.
 *
 * Destroys any packets waiting in queue, frees the libevent structures used by the connection to Metis.
 * Frees the FwdMetisState object and will NULL *fwdStatePtr.
 *
 * @param [in,out] fwdStatePtr Double pointer to the allocated state.  Will be NULL'd on output.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_fwdMetisState_Release(FwdMetisState **fwdStatePtr)
{
    FwdMetisState *fwd_state = *fwdStatePtr;

    while (!parcDeque_IsEmpty(fwd_state->transportMessageQueue)) {
        TransportMessage *tm = parcDeque_RemoveFirst(fwd_state->transportMessageQueue);
        transportMessage_Destroy(&tm);
    }

    parcDeque_Release(&fwd_state->transportMessageQueue);

    if (fwd_state->readEvent) {
        parcEvent_Destroy(&(fwd_state->readEvent));
    }

    if (fwd_state->writeEvent) {
        parcEvent_Destroy(&(fwd_state->writeEvent));
    }

    parcEventTimer_Destroy(&(fwd_state->transportMessageQueueEvent));

    if (fwd_state->metisOutputQueue) {
        parcEventBuffer_Destroy(&(fwd_state->metisOutputQueue));
    }

    if (fwd_state->nextMessage.packet) {
        parcBuffer_Release(&fwd_state->nextMessage.packet);
    }

    close(fwd_state->fd);

    parcMemory_Deallocate((void **) &fwd_state);
    *fwdStatePtr = NULL;
}

static int
connector_Fwd_Metis_Closer(RtaConnection *conn)
{
    FwdMetisState *fwd_state = rtaConnection_GetPrivateData(conn, FWD_METIS);
    rtaConnection_SetPrivateData(conn, FWD_METIS, NULL);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s called on fwd_state %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))), __func__, (void *) fwd_state);
    }

    RtaComponentStats *stats = rtaConnection_GetStats(conn, FWD_METIS);
    rtaComponentStats_Increment(stats, STATS_CLOSES);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s closed fwd_state %p deque length %zu\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) fwd_state,
               parcDeque_Size(fwd_state->transportMessageQueue));

        printf("%9" PRIu64 " %s closed fwd_state %p stats: up { reads %u wok %u werr %u wblk %u wfull %u wctrlok %u wctrlerr %u }\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) fwd_state,
               fwd_state->stats.countUpcallReads, fwd_state->stats.countUpcallWriteDataOk, fwd_state->stats.countUpcallWriteDataError,
               fwd_state->stats.countUpcallWriteDataBlocked, fwd_state->stats.countUpcallWriteDataQueueFull,
               fwd_state->stats.countUpcallWriteControlOk, fwd_state->stats.countUpcallWriteControlError);

        printf("%9" PRIu64 " %s closed fwd_state %p stats: dn { reads %u wok %u wctrlok %u }\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) fwd_state,
               fwd_state->stats.countDowncallReads, fwd_state->stats.countDowncallWrites, fwd_state->stats.countDowncallControl);
    }

    _fwdMetisState_Release(&fwd_state);

    return 0;
}

static int
connector_Fwd_Metis_Release(RtaProtocolStack *stack)
{
    return 0;
}

/**
 * Enable to disable the read event based on the Blocked Up state
 *
 * If we receive a Blocked Up state change and the read event is pending, make it
 * not pending.  If we receive a not blocked up state change and the read event is not
 * pending, make it pending.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
connector_Fwd_Metis_StateChange(RtaConnection *conn)
{
    struct fwd_metis_state *fwd_state = rtaConnection_GetPrivateData(conn, FWD_METIS);

    int isReadPending = parcEvent_Poll(fwd_state->readEvent, PARCEventType_Read);


    // If we are blocked in the UP direction, disable events on the read queue
    if (rtaConnection_BlockedUp(conn)) {
        // we only disable it and log it if it was active
        if (isReadPending) {
            if (DEBUG_OUTPUT) {
                printf("%9" PRIu64 " %s connection %u blocked up, disable PARCEventType_Read\n",
                       rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                       __func__,
                       rtaConnection_GetConnectionId(conn));
            }

            parcEvent_Stop(fwd_state->readEvent);
        }
    } else {
        if ((!isReadPending) && fwd_state->isConnected) {
            if (DEBUG_OUTPUT) {
                printf("%9" PRIu64 " %s connection %u unblocked up, enable PARCEventType_Read\n",
                       rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                       __func__,
                       rtaConnection_GetConnectionId(conn));
            }
            parcEvent_Start(fwd_state->readEvent);
        }
    }

    // We do not need to do anything with DOWN direction, becasue we're the component sending
    // those block down messages.
}
