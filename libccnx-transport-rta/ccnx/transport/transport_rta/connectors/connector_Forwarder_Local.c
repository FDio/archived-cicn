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
 * PF_LOCAL forwarder glue, mostly for testing.  This uses a
 * STREAM socket with a user specified coding.  Each message
 * on the stream is of this format:
 *
 * uint32_t   process pid
 * uint32_t   user_socket_fd
 * uint32_t   message bytes that follow
 * uint8_t[]  message encoded with user specified codec
 *
 * The user_socket_fd will be the same number that the API was assigned
 * in transportRta_Socket->api_socket_pair[PAIR_OTHER].
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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <parc/algol/parc_EventBuffer.h>

#include <LongBow/runtime.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/transport/transport_rta/core/rta_Framework_Services.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>
#include <ccnx/transport/transport_rta/connectors/connector_Forwarder.h>

#include <ccnx/transport/transport_rta/config/config_Forwarder_Local.h>
#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_ControlFacade.h>

#include <ccnx/common/ccnx_WireFormatMessage.h>

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

static int  connector_Fwd_Local_Init(RtaProtocolStack *stack);
static int  connector_Fwd_Local_Opener(RtaConnection *conn);
static void connector_Fwd_Local_Upcall_Read(PARCEventQueue *, PARCEventType, void *conn);
static void connector_Fwd_Local_Upcall_Event(PARCEventQueue *, PARCEventQueueEventType, void *stack);
static void connector_Fwd_Local_Downcall_Read(PARCEventQueue *, PARCEventType, void *conn);
static int  connector_Fwd_Local_Closer(RtaConnection *conn);
static int  connector_Fwd_Local_Release(RtaProtocolStack *stack);
static void connector_Fwd_Local_StateChange(RtaConnection *conn);

RtaComponentOperations fwd_local_ops = {
    .init          = connector_Fwd_Local_Init,
    .open          = connector_Fwd_Local_Opener,
    .upcallRead    = connector_Fwd_Local_Upcall_Read,
    .upcallEvent   = connector_Fwd_Local_Upcall_Event,
    .downcallRead  = connector_Fwd_Local_Downcall_Read,
    .downcallEvent = NULL,
    .close         = connector_Fwd_Local_Closer,
    .release       = connector_Fwd_Local_Release,
    .stateChange   = connector_Fwd_Local_StateChange
};

struct fwd_local_state {
    int fd;
    PARCEventQueue *bev_local;
    int connected;
};

typedef struct {
    uint32_t pid;
    uint32_t fd;
    uint32_t length;
    uint32_t pad;       // make it 16 bytes
} __attribute__ ((packed)) localhdr;

// ================================
// NULL

static int
connector_Fwd_Local_Init(RtaProtocolStack *stack)
{
    // no stack-wide initialization
    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s init stack %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(stack)),
               __func__,
               (void *) stack);
    }
    return 0;
}

/*
 * Create a PF_LOCAL socket
 * Set it non-blocking
 * Wrap it in a buffer event
 * Set Read and Event callbacks
 * connect to LOCAL_NAME
 *
 * Return 0 success, -1 failure
 */
static int
connector_Fwd_Local_Opener(RtaConnection *conn)
{
    PARCEventScheduler *base;
    RtaProtocolStack *stack;
    const char *sock_name;

    stack = rtaConnection_GetStack(conn);
    base = rtaFramework_GetEventScheduler(rtaProtocolStack_GetFramework(stack));

    sock_name = localForwarder_GetPath(rtaConnection_GetParameters(conn));
    assertNotNull(sock_name, "connector_Fwd_Local_Opener called without setting LOCAL_NAME");

    if (sock_name == NULL) {
        return -1;
    }

    struct fwd_local_state *fwd_state = parcMemory_Allocate(sizeof(struct fwd_local_state));
    assertNotNull(fwd_state, "parcMemory_Allocate(%zu) returned NULL", sizeof(struct fwd_local_state));

    rtaConnection_SetPrivateData(conn, FWD_LOCAL, fwd_state);

    fwd_state->fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (fwd_state->fd < 0) {
        perror("socket PF_LOCAL");
    }
    assertFalse(fwd_state->fd < 0, "socket PF_LOCAL error");

    struct sockaddr_un addr_unix;
    memset(&addr_unix, 0, sizeof(struct sockaddr_un));
    addr_unix.sun_family = AF_UNIX;

    trapIllegalValueIf(sizeof(addr_unix.sun_path) <= strlen(sock_name), "sock_name too long, maximum length %zu", sizeof(addr_unix.sun_path) - 1);
    strcpy(addr_unix.sun_path, sock_name);

    // Setup the socket as non-blocking then wrap in a parcEventQueue.

    int flags = fcntl(fwd_state->fd, F_GETFL, NULL);
    assertFalse(flags < 0, "fcntl failed to obtain file descriptor flags (%d)\n", errno);

    int failure = fcntl(fwd_state->fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    assertTrue(failure == 0, "could not make socket non-blocking");
    if (failure < 0) {
        rtaConnection_SetPrivateData(conn, FWD_LOCAL, NULL);
        close(fwd_state->fd);
        parcMemory_Deallocate((void **) &fwd_state);
        return -1;
    }

    fwd_state->bev_local = parcEventQueue_Create(base, fwd_state->fd, PARCEventQueueOption_CloseOnFree);

    assertNotNull(fwd_state->bev_local, "Null buffer event for local socket.");

    parcEventQueue_SetCallbacks(fwd_state->bev_local,
                                connector_Fwd_Local_Upcall_Read,
                                NULL,
                                connector_Fwd_Local_Upcall_Event,
                                conn);

    parcEventQueue_Enable(fwd_state->bev_local, PARCEventType_Read);

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;

    trapIllegalValueIf(sizeof(addr_unix.sun_path) <= strlen(sock_name), "sock_name too long, maximum length %zu", sizeof(addr_unix.sun_path) - 1);
    strcpy(addr_unix.sun_path, sock_name);

    // This will deliver a PARCEventQueue_Connected on connect success
    if (parcEventQueue_ConnectSocket(fwd_state->bev_local,
                                     (struct sockaddr*) &addr_unix,
                                     (socklen_t) sizeof(addr_unix)) < 0) {
        perror("connect PF_LOCAL");
        assertTrue(0, "connect PF_LOCAL");
        rtaConnection_SetPrivateData(conn, FWD_LOCAL, NULL);
        close(fwd_state->fd);
        parcMemory_Deallocate((void **) &fwd_state);
        return -1;
    }

    // Socket will be ready for use once we get PARCEventQueueEventType_Connected
    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s open conn %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               (void *) conn);
    }

    return 0;
}

/*
 * Read from bev_local.  We are passed the connection on the ptr.
 */
static void
connector_Fwd_Local_Upcall_Read(PARCEventQueue *bev, PARCEventType type, void *ptr)
{
    RtaConnection *conn = (RtaConnection *) ptr;
    RtaProtocolStack *stack = rtaConnection_GetStack(conn);
    PARCEventBuffer *in = parcEventBuffer_GetQueueBufferInput(bev);
    PARCEventQueue  *out = rtaProtocolStack_GetPutQueue(stack, FWD_LOCAL, RTA_UP);
    RtaComponentStats *stats = rtaConnection_GetStats(conn, FWD_LOCAL);
    TransportMessage *tm;

    unsigned char *mem;
    int res;

    // only move forward if enough bytes available

    while (parcEventBuffer_GetLength(in) >= sizeof(localhdr)) {
        size_t msg_length;

        mem = parcEventBuffer_Pullup(in, sizeof(localhdr));
        if (mem == NULL) {
            // not enough bytes
            parcEventBuffer_Destroy(&in);
            return;
        }

        msg_length = ((localhdr *) mem)->length;
        if (parcEventBuffer_GetLength(in) < msg_length + sizeof(localhdr)) {
            // not enough bytes
            parcEventBuffer_Destroy(&in);
            return;
        }

        PARCBuffer *wireFormat = parcBuffer_Allocate(msg_length);
        assertNotNull(wireFormat, "parcBuffer_Allocate(%zu) returned NULL", msg_length);

        rtaComponentStats_Increment(stats, STATS_UPCALL_IN);

        // we can read a whole message.  Read it directly in to a buffer
        // Skip the FWD_LOCAL header
        res = parcEventBuffer_Read(in, NULL, sizeof(localhdr));
        assertTrue(res == 0, "Got error draining header from buffer");

        uint8_t *overlay = parcBuffer_Overlay(wireFormat, msg_length);
        res = parcEventBuffer_Read(in, overlay, msg_length);
        overlay = NULL;

        assertTrue(res == msg_length,
                   "parcEventBuffer_Read returned wrong size, expected %zu got %d",
                   msg_length, res);

        parcBuffer_Flip(wireFormat);

        if (rtaConnection_GetState(conn) == CONN_OPEN) {
            CCNxWireFormatMessage *wireFormatMessage = ccnxWireFormatMessage_Create(wireFormat);
            CCNxTlvDictionary *dictionary = ccnxWireFormatMessage_GetDictionary(wireFormatMessage);
            if (dictionary != NULL) {
                // wrap it for transport module
                tm = transportMessage_CreateFromDictionary(dictionary);

                // add the connection info to the transport message before sending up stack
                transportMessage_SetInfo(tm, rtaConnection_Copy(conn), rtaConnection_FreeFunc);

                // send it up the stack
                if (rtaComponent_PutMessage(out, tm)) {
                    rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
                }

                // Now release our hold on the wireFormatMessage (aka dictionary)
                ccnxWireFormatMessage_Release(&wireFormatMessage);
            } else {
                printf("Failed to create CCNxTlvDictionary from wireformat\n");
                parcBuffer_Display(wireFormat, 3);
            }
        } else {
            //drop packets
        }

        parcBuffer_Release(&wireFormat);
    }

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s total upcall reads in %" PRIu64 " out %" PRIu64 "\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               rtaComponentStats_Get(stats, STATS_UPCALL_IN),
               rtaComponentStats_Get(stats, STATS_UPCALL_OUT));
    }
    parcEventBuffer_Destroy(&in);
}

/*
 * Event on connection to forwarder.
 * Passed the RtaConnection in the pointer
 */
static void
connector_Fwd_Local_Upcall_Event(PARCEventQueue *queue, PARCEventQueueEventType events, void *ptr)
{
    RtaConnection *conn = (RtaConnection *) ptr;

    struct fwd_local_state *fwd_state = rtaConnection_GetPrivateData(conn, FWD_LOCAL);

    if (events & PARCEventQueueEventType_Connected) {
        if (DEBUG_OUTPUT) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            printf("%6lu.%06ld %s (pid %d) connected socket %d\n",
                   tv.tv_sec, (long) tv.tv_usec,
                   __func__,
                   getpid(),
                   rtaConnection_GetTransportFd(conn));
        }

        fwd_state->connected = 1;
        rtaConnection_SendStatus(conn, FWD_LOCAL, RTA_UP, notifyStatusCode_CONNECTION_OPEN, NULL, NULL);
    } else if (events & PARCEventQueueEventType_Error) {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        longBowRuntime_StackTrace(1);

        if (events & PARCEventQueueEventType_Reading) {
            printf("%6lu.%06ld %s (pid %d) Got read error on PF_LOCAL, transport socket %d: (%d) %s\n",
                   tv.tv_sec, (long) tv.tv_usec,
                   __func__,
                   getpid(),
                   rtaConnection_GetTransportFd(conn),
                   errno,
                   strerror(errno));
        } else if (events & PARCEventQueueEventType_Writing) {
            printf("%6lu.%06ld %s (pid %d) Got write error on PF_LOCAL, transport socket %d: (%d) %s\n",
                   tv.tv_sec, (long) tv.tv_usec,
                   __func__,
                   getpid(),
                   rtaConnection_GetTransportFd(conn),
                   errno,
                   strerror(errno));
        } else {
            printf("%6lu.%06ld %s (pid %d) Got error on PF_LOCAL, transport socket %d: (%d) %s\n",
                   tv.tv_sec, (long) tv.tv_usec,
                   __func__,
                   getpid(),
                   rtaConnection_GetTransportFd(conn),
                   errno,
                   strerror(errno));
        }

        /* An error occured while connecting. */
        rtaConnection_SendStatus(conn, FWD_LOCAL, RTA_UP, notifyStatusCode_FORWARDER_NOT_AVAILABLE, NULL, NULL);
    }
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
    PARCEventQueue  *out = rtaProtocolStack_GetPutQueue(stack, FWD_LOCAL, RTA_UP);
    if (rtaComponent_PutMessage(out, tm_ack)) {
        RtaComponentStats *stats = rtaConnection_GetStats(conn, FWD_LOCAL);
        rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
    }
}

static void
connector_Fwd_Local_ProcessControl(RtaConnection *conn, TransportMessage *tm)
{
    CCNxTlvDictionary *controlDictionary = transportMessage_GetDictionary(tm);

    if (ccnxControlFacade_IsCPI(controlDictionary)) {
        PARCJSON *json = ccnxControlFacade_GetJson(controlDictionary);
        if (controlPlaneInterface_GetCPIMessageType(json) == CPI_REQUEST) {
            if (cpi_getCPIOperation2(json) == CPI_PAUSE) {
                if (DEBUG_OUTPUT) {
                    printf("%9" PRIu64 " %s conn %p recieved PAUSE\n",
                           rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                           __func__,
                           (void *) conn);
                }
                _ackRequest(conn, json);
            } else if (cpi_getCPIOperation2(json) == CPI_FLUSH) {
                if (DEBUG_OUTPUT) {
                    printf("%9" PRIu64 " %s conn %p recieved FLUSH\n",
                           rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                           __func__,
                           (void *) conn);
                }
                _ackRequest(conn, json);
            } else {
                // some other message.  We just ACK everything in the local connector.
                _ackRequest(conn, json);
            }
        }
    }
}

static void
connector_Fwd_Local_WriteIovec(struct fwd_local_state *fwdConnState, RtaConnection *conn, CCNxCodecNetworkBufferIoVec *vec, RtaComponentStats *stats)
{
    localhdr lh;

    memset(&lh, 0, sizeof(localhdr));
    lh.pid = getpid();
    lh.fd = rtaConnection_GetTransportFd(conn);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s total downcall reads %" PRIu64 "\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
               __func__,
               rtaComponentStats_Get(stats, STATS_DOWNCALL_IN));
    }

    int iovcnt = ccnxCodecNetworkBufferIoVec_GetCount(vec);
    const struct iovec *array = ccnxCodecNetworkBufferIoVec_GetArray(vec);

    lh.length = 0;
    for (int i = 0; i < iovcnt; i++) {
        lh.length += array[i].iov_len;
    }

    if (parcEventQueue_Write(fwdConnState->bev_local, &lh, sizeof(lh)) < 0) {
        trapUnrecoverableState("%s error writing to bev_local", __func__);
    }

    for (int i = 0; i < iovcnt; i++) {
        if (parcEventQueue_Write(fwdConnState->bev_local, array[i].iov_base, array[i].iov_len) < 0) {
            trapUnrecoverableState("%s error writing iovec to bev_local", __func__);
        }
    }
}

/* send raw packet from codec to forwarder */
static void
connector_Fwd_Local_Downcall_Read(PARCEventQueue *in, PARCEventType event, void *ptr)
{
    TransportMessage *tm;

    while ((tm = rtaComponent_GetMessage(in)) != NULL) {
        RtaConnection *conn;
        struct fwd_local_state *fwdConnState;
        RtaComponentStats *stats;

        CCNxTlvDictionary *messageDictionary = transportMessage_GetDictionary(tm);

        conn = rtaConnection_GetFromTransport(tm);
        fwdConnState = rtaConnection_GetPrivateData(conn, FWD_LOCAL);
        stats = rtaConnection_GetStats(conn, FWD_LOCAL);
        rtaComponentStats_Increment(stats, STATS_DOWNCALL_IN);

        // ignore configuration messages for the send
        if (ccnxTlvDictionary_IsControl(messageDictionary)) {
            connector_Fwd_Local_ProcessControl(conn, tm);
        } else {
            CCNxCodecNetworkBufferIoVec *vec = ccnxWireFormatMessage_GetIoVec(messageDictionary);
            assertNotNull(vec, "%s got null wire format\n", __func__);

            connector_Fwd_Local_WriteIovec(fwdConnState, conn, vec, stats);

            rtaComponentStats_Increment(stats, STATS_DOWNCALL_OUT);
        }

        // we can release everything here. connector_Fwd_Local_WriteIovec made its own references
        // to the wire format if it needed them.
        transportMessage_Destroy(&tm);

        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s total downcall reads in %" PRIu64 " out %" PRIu64 "\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))),
                   __func__,
                   rtaComponentStats_Get(stats, STATS_DOWNCALL_IN),
                   rtaComponentStats_Get(stats, STATS_DOWNCALL_OUT));
        }
    }
}

static int
connector_Fwd_Local_Closer(RtaConnection *conn)
{
    struct fwd_local_state *fwd_state = rtaConnection_GetPrivateData(conn, FWD_LOCAL);
    RtaComponentStats *stats;

    assertNotNull(fwd_state, "invalid state");
    assertNotNull(fwd_state->bev_local, "invalid PARCEventQueue pointer");

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s called on fwd_state %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))), __func__, (void *) fwd_state);
    }

    stats = rtaConnection_GetStats(conn, FWD_LOCAL);

    // this will close too
    parcEventQueue_Destroy(&(fwd_state->bev_local));
    memset(fwd_state, 0, sizeof(struct fwd_local_state));
    parcMemory_Deallocate((void **) &fwd_state);

    rtaConnection_SetPrivateData(conn, FWD_LOCAL, NULL);
    rtaComponentStats_Increment(stats, STATS_CLOSES);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s closed fwd_state %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn))), __func__, (void *) fwd_state);
    }

    return 0;
}

static int
connector_Fwd_Local_Release(RtaProtocolStack *stack)
{
    // no stack-wide initialization
    if (DEBUG_OUTPUT) {
        printf("%s release stack %p\n",
               __func__,
               (void *) stack);
    }

    return 0;
}

static void
connector_Fwd_Local_StateChange(RtaConnection *conn)
{
    //not implemented
}
