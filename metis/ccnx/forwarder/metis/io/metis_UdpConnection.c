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
 * Embodies the reader/writer for a UDP connection
 *
 * NB The Send() function may overflow the output buffer
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <ccnx/forwarder/metis/io/metis_UdpConnection.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <parc/algol/parc_Hash.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>

typedef struct metis_udp_state {
    MetisForwarder *metis;
    MetisLogger *logger;

    // the udp listener socket we receive packets on
    int udpListenerSocket;

    MetisAddressPair *addressPair;

    // We need to access this all the time, so grab it out
    // of the addressPair;
    struct sockaddr *peerAddress;
    socklen_t peerAddressLength;

    bool isLocal;
    bool isUp;
    unsigned id;

    unsigned delay;
} _MetisUdpState;

// Prototypes
static bool                    _send(MetisIoOperations *ops, const CPIAddress *nexthop, MetisMessage *message);
static const CPIAddress       *_getRemoteAddress(const MetisIoOperations *ops);
static const MetisAddressPair *_getAddressPair(const MetisIoOperations *ops);
static unsigned                _getConnectionId(const MetisIoOperations *ops);
static bool                    _isUp(const MetisIoOperations *ops);
static bool                    _isLocal(const MetisIoOperations *ops);
static void                    _destroy(MetisIoOperations **opsPtr);
static CPIConnectionType       _getConnectionType(const MetisIoOperations *ops);
static MetisTicks              _sendProbe(MetisIoOperations *ops, unsigned probeType);
/*
 * This assigns a unique pointer to the void * which we use
 * as a GUID for this class.
 */
static const void *_metisIoOperationsGuid = __FILE__;

/*
 * Return our GUID
 */
static const void *
_metisStreamConnection_Class(const MetisIoOperations *ops)
{
    return _metisIoOperationsGuid;
}

static MetisIoOperations _template = {
    .closure           = NULL,
    .send              = &_send,
    .getRemoteAddress  = &_getRemoteAddress,
    .getAddressPair    = &_getAddressPair,
    .getConnectionId   = &_getConnectionId,
    .isUp              = &_isUp,
    .isLocal           = &_isLocal,
    .destroy           = &_destroy,
    .class             = &_metisStreamConnection_Class,
    .getConnectionType = &_getConnectionType,
    .sendProbe         = &_sendProbe
};

// =================================================================

static void _setConnectionState(_MetisUdpState *Udp, bool isUp);
static bool _saveSockaddr(_MetisUdpState *udpConnState, const MetisAddressPair *pair);

MetisIoOperations *
metisUdpConnection_Create(MetisForwarder *metis, int fd, const MetisAddressPair *pair, bool isLocal)
{
    MetisIoOperations *io_ops = NULL;

    _MetisUdpState *udpConnState = parcMemory_AllocateAndClear(sizeof(_MetisUdpState));
    assertNotNull(udpConnState, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisUdpState));

    udpConnState->metis = metis;
    udpConnState->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));

    bool saved = _saveSockaddr(udpConnState, pair);
    if (saved) {
        udpConnState->udpListenerSocket = fd;
        udpConnState->id = metisForwarder_GetNextConnectionId(metis);
        udpConnState->addressPair = metisAddressPair_Acquire(pair);
        udpConnState->isLocal = isLocal;

        // allocate a connection
        io_ops = parcMemory_AllocateAndClear(sizeof(MetisIoOperations));
        assertNotNull(io_ops, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisIoOperations));
        memcpy(io_ops, &_template, sizeof(MetisIoOperations));
        io_ops->closure = udpConnState;

        _setConnectionState(udpConnState, true);

        if (metisLogger_IsLoggable(udpConnState->logger, MetisLoggerFacility_IO, PARCLogLevel_Info)) {
            char *str = metisAddressPair_ToString(udpConnState->addressPair);
            metisLogger_Log(udpConnState->logger, MetisLoggerFacility_IO, PARCLogLevel_Info, __func__,
                            "UdpConnection %p created for address %s (isLocal %d)",
                            (void *) udpConnState, str, udpConnState->isLocal);
            free(str);
        }

        metisMessenger_Send(metisForwarder_GetMessenger(metis), metisMissive_Create(MetisMissiveType_ConnectionCreate, udpConnState->id));
        metisMessenger_Send(metisForwarder_GetMessenger(metis), metisMissive_Create(MetisMissiveType_ConnectionUp, udpConnState->id));
    } else {
        // _saveSockaddr will already log an error, no need for extra log message here
        metisLogger_Release(&udpConnState->logger);
        parcMemory_Deallocate((void **) &udpConnState);
    }

    return io_ops;
}

// =================================================================
// I/O Operations implementation

static void
_destroy(MetisIoOperations **opsPtr)
{
    assertNotNull(opsPtr, "Parameter opsPtr must be non-null double pointer");
    assertNotNull(*opsPtr, "Parameter opsPtr must dereference to non-null pointer");

    MetisIoOperations *ops = *opsPtr;
    assertNotNull(metisIoOperations_GetClosure(ops), "ops->context must not be null");

    _MetisUdpState *udpConnState = (_MetisUdpState *) metisIoOperations_GetClosure(ops);
    metisAddressPair_Release(&udpConnState->addressPair);
    parcMemory_Deallocate((void **) &(udpConnState->peerAddress));

    metisMessenger_Send(metisForwarder_GetMessenger(udpConnState->metis), metisMissive_Create(MetisMissiveType_ConnectionDestroyed, udpConnState->id));

    if (metisLogger_IsLoggable(udpConnState->logger, MetisLoggerFacility_IO, PARCLogLevel_Info)) {
        metisLogger_Log(udpConnState->logger, MetisLoggerFacility_IO, PARCLogLevel_Info, __func__,
                        "UdpConnection %p destroyed",
                        (void *) udpConnState);
    }

    // do not close udp->udpListenerSocket, the listener will close
    // that when its done

    metisLogger_Release(&udpConnState->logger);
    parcMemory_Deallocate((void **) &udpConnState);
    parcMemory_Deallocate((void **) &ops);

    *opsPtr = NULL;
}

static bool
_isUp(const MetisIoOperations *ops)
{
    assertNotNull(ops, "Parameter must be non-null");
    const _MetisUdpState *udpConnState = (const _MetisUdpState *) metisIoOperations_GetClosure(ops);
    return udpConnState->isUp;
}

static bool
_isLocal(const MetisIoOperations *ops)
{
    assertNotNull(ops, "Parameter must be non-null");
    const _MetisUdpState *udpConnState = (const _MetisUdpState *) metisIoOperations_GetClosure(ops);
    return udpConnState->isLocal;
}

static const CPIAddress *
_getRemoteAddress(const MetisIoOperations *ops)
{
    assertNotNull(ops, "Parameter must be non-null");
    const _MetisUdpState *udpConnState = (const _MetisUdpState *) metisIoOperations_GetClosure(ops);
    return metisAddressPair_GetRemote(udpConnState->addressPair);
}

static const MetisAddressPair *
_getAddressPair(const MetisIoOperations *ops)
{
    assertNotNull(ops, "Parameter must be non-null");
    const _MetisUdpState *udpConnState = (const _MetisUdpState *) metisIoOperations_GetClosure(ops);
    return udpConnState->addressPair;
}

static unsigned
_getConnectionId(const MetisIoOperations *ops)
{
    assertNotNull(ops, "Parameter must be non-null");
    const _MetisUdpState *udpConnState = (const _MetisUdpState *) metisIoOperations_GetClosure(ops);
    return udpConnState->id;
}

/**
 * @function metisUdpConnection_Send
 * @abstract Non-destructive send of the message.
 * @discussion
 *   sends a message to the peer.
 *
 * @param dummy is ignored.  A udp connection has only one peer.
 * @return <#return#>
 */
static bool
_send(MetisIoOperations *ops, const CPIAddress *dummy, MetisMessage *message)
{
    assertNotNull(ops, "Parameter ops must be non-null");
    assertNotNull(message, "Parameter message must be non-null");
    _MetisUdpState *udpConnState = (_MetisUdpState *) metisIoOperations_GetClosure(ops);

    PARCEventBuffer *writeBuffer = parcEventBuffer_Create();
    metisMessage_Append(writeBuffer, message);

    const uint8_t *buffer = parcEventBuffer_Pullup(writeBuffer, -1);
    size_t bufferLength = parcEventBuffer_GetLength(writeBuffer);

    ssize_t writeLength = sendto(udpConnState->udpListenerSocket, buffer, bufferLength, 0, udpConnState->peerAddress, udpConnState->peerAddressLength);

    if (writeLength < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            parcEventBuffer_Destroy(&writeBuffer);
            return false;
        } else  {
            //this print is for debugging
            printf("Incorrect write length %zd, expected %zd: (%d) %s\n", writeLength, bufferLength, errno, strerror(errno));
            parcEventBuffer_Destroy(&writeBuffer);
            return false;
        }
    }

    parcEventBuffer_Destroy(&writeBuffer);
    return true;
}

static CPIConnectionType
_getConnectionType(const MetisIoOperations *ops)
{
    return cpiConnection_UDP;
}

static MetisTicks
_sendProbe(MetisIoOperations *ops, unsigned probeType)
{
    assertNotNull(ops, "Parameter ops must be non-null");
    _MetisUdpState *udpConnState = (_MetisUdpState *) metisIoOperations_GetClosure(ops);


    uint8_t *pkt;
    size_t pkt_size = 8;
    pkt = (uint8_t *) malloc(sizeof(uint8_t) * pkt_size);
    for (unsigned i = 0; i < pkt_size; i++) {
        pkt[i] = 0;
    }
    pkt[0] = 1;         //tlv type
    pkt[1] = probeType; //packet type
    pkt[6] = 8;         //header len (16bit, network order)

    ssize_t writeLen = sendto(udpConnState->udpListenerSocket, pkt, pkt_size, 0, udpConnState->peerAddress, udpConnState->peerAddressLength);

    if (writeLen < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            free(pkt);
            return 0;
        } else  {
            //this print is for debugging
            printf("Incorrect write length %zd, expected %zd: (%d) %s\n", writeLen, pkt_size, errno, strerror(errno));
            free(pkt);
            return 0;
        }
    }

    free(pkt);
    return metisForwarder_GetTicks(udpConnState->metis);
}

/*static MetisTicks
 * _handleProbe(MetisIoOperations *ops, MetisTicks last_sent, uint8_t *pkt)
 * {
 *  assertNotNull(ops, "Parameter ops must be non-null");
 *  assertNotNull(pkt, "Parameter pkt must be non-null");
 *
 *  _MetisUdpState *udpConnState = (_MetisUdpState *) metisIoOperations_GetClosure(ops);
 *
 *  MetisTicks delay = 0;
 *
 *  if(pkt[1] == METIS_PACKET_TYPE_PROBE_REQUEST){
 *      _sendProbeType(ops, METIS_PACKET_TYPE_PROBE_REPLY);
 *  } else if (pkt[1] == METIS_PACKET_TYPE_PROBE_REPLY) {
 *      MetisTicks now = metisForwarder_GetTicks(udpConnState->metis);
 *      delay = now - last_sent;
 *  } else {
 *      printf("receivde unkwon probe type\n");
 *  }
 *
 *  return delay;
 * }*/

// =================================================================
// Internal API

static bool
_saveSockaddr(_MetisUdpState *udpConnState, const MetisAddressPair *pair)
{
    bool success = false;
    const CPIAddress *remoteAddress = metisAddressPair_GetRemote(pair);

    switch (cpiAddress_GetType(remoteAddress)) {
        case cpiAddressType_INET: {
            size_t bytes = sizeof(struct sockaddr_in);
            udpConnState->peerAddress = parcMemory_Allocate(bytes);
            assertNotNull(udpConnState->peerAddress, "parcMemory_Allocate(%zu) returned NULL", bytes);

            cpiAddress_GetInet(remoteAddress, (struct sockaddr_in *) udpConnState->peerAddress);
            udpConnState->peerAddressLength = (socklen_t) bytes;

            success = true;
            break;
        }

        case cpiAddressType_INET6: {
            size_t bytes = sizeof(struct sockaddr_in6);
            udpConnState->peerAddress = parcMemory_Allocate(bytes);
            assertNotNull(udpConnState->peerAddress, "parcMemory_Allocate(%zu) returned NULL", bytes);

            cpiAddress_GetInet6(remoteAddress, (struct sockaddr_in6 *) udpConnState->peerAddress);
            udpConnState->peerAddressLength = (socklen_t) bytes;

            success = true;
            break;
        }

        default:
            if (metisLogger_IsLoggable(udpConnState->logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
                char *str = cpiAddress_ToString(remoteAddress);
                metisLogger_Log(udpConnState->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                                "Remote address is not INET or INET6: %s", str);
                parcMemory_Deallocate((void **) &str);
            }
            break;
    }
    return success;
}

static void
_setConnectionState(_MetisUdpState *udpConnState, bool isUp)
{
    assertNotNull(udpConnState, "Parameter Udp must be non-null");

    MetisMessenger *messenger = metisForwarder_GetMessenger(udpConnState->metis);

    bool oldStateIsUp = udpConnState->isUp;
    udpConnState->isUp = isUp;

    if (oldStateIsUp && !isUp) {
        // bring connection DOWN
        MetisMissive *missive = metisMissive_Create(MetisMissiveType_ConnectionDown, udpConnState->id);
        metisMessenger_Send(messenger, missive);
        return;
    }


    if (!oldStateIsUp && isUp) {
        // bring connection UP
        MetisMissive *missive = metisMissive_Create(MetisMissiveType_ConnectionUp, udpConnState->id);
        metisMessenger_Send(messenger, missive);
        return;
    }
}
