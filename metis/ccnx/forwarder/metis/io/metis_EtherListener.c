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
 * Implements the listener over Ethernet.
 *
 * Right now only supports non-VLAN frames Ethernet (not 802.3/802.2 LLC) frames.
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <net/ethernet.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/messenger/metis_Messenger.h>

#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>

#include <ccnx/forwarder/metis/io/metis_GenericEther.h>
#include <ccnx/forwarder/metis/io/metis_EtherConnection.h>

#ifndef ntohll
#define ntohll(x) (((uint64_t) (ntohl((uint32_t) (x & 0x00000000FFFFFFFF))) << 32) | ntohl(((uint32_t) ((((uint64_t) x) >> 32)))))
#define htonll(x) ntohll(x)
#endif

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>

typedef struct metis_ether_stats {
    uint64_t framesIn;
    uint64_t framesError;
    uint64_t framesReceived;
    uint64_t framesReassembled;
    uint64_t framesNotForUs;
} _MetisEtherStats;

typedef struct metis_ether_listener {
    MetisForwarder *metis;
    MetisLogger *logger;

    MetisGenericEther *genericEther;

    unsigned id;
    CPIAddress *localAddress;

    uint16_t ethertype;

    int ether_fd;
    PARCEvent *ether_event;

    // what size do the read buffers need to be?
    size_t ether_buffer_length;

    // buffer to read next packet in to
    PARCEventBuffer *nextReadBuffer;

    // We store MAC addresses in uint64 and mask them down to 6 bytes.
    // this means all our address comparisons are simple "==" operations

    uint64_t *destinationAddressList;
    size_t destinationAddressSize;

    uint64_t *sourceAddressList;
    size_t sourceAddressSize;

    _MetisEtherStats stats;
} _MetisEtherListener;

// network byte order mask to go from 8-bytes to 6-bytes
#define MAC_MASK          (htonll(0xFFFFFFFFFFFF0000ULL))

static void              _metisEtherListener_OpsDestroy(MetisListenerOps **listenerOpsPtr);
static unsigned          _metisEtherListener_OpsGetInterfaceIndex(const MetisListenerOps *ops);
static const CPIAddress *_metisEtherListener_OpsGetListenAddress(const MetisListenerOps *ops);
static MetisEncapType    _metisEtherListener_OpsGetEncapType(const MetisListenerOps *ops);
static int               _metisEtherListener_OpsGetSocket(const MetisListenerOps *ops);

static MetisListenerOps _etherTemplate = {
    .context           = NULL,
    .destroy           = &_metisEtherListener_OpsDestroy,
    .getInterfaceIndex = &_metisEtherListener_OpsGetInterfaceIndex,
    .getListenAddress  = &_metisEtherListener_OpsGetListenAddress,
    .getEncapType      = &_metisEtherListener_OpsGetEncapType,
    .getSocket         = &_metisEtherListener_OpsGetSocket
};

// Called by Libevent
static void _metisEtherListener_ReadCallback(int fd, PARCEventType what, void *user_data);

static void
_logStats(_MetisEtherListener *listener, PARCLogLevel level)
{
    if (metisLogger_IsLoggable(listener->logger, MetisLoggerFacility_IO, level)) {
        metisLogger_Log(listener->logger, MetisLoggerFacility_IO, level, __func__,
                        "EtherListener %p frames in %" PRIu64 ", errors %" PRIu64 " ok %" PRIu64 " reassemble %" PRIu64 " reject %" PRIu64,
                        (void *) listener,
                        listener->stats.framesIn,
                        listener->stats.framesError,
                        listener->stats.framesReceived,
                        listener->stats.framesReassembled,
                        listener->stats.framesNotForUs);
    }
}


// =============================================
// Public API

static void
_metisEtherListener_FillInEthernetAddresses(_MetisEtherListener *etherListener)
{
    // this may be null
    PARCBuffer *myAddress = metisGenericEther_GetMacAddress(etherListener->genericEther);
    uint64_t macAsUint64 = 0;

    if (myAddress) {
        while (parcBuffer_Remaining(myAddress) > 0) {
            uint8_t b = parcBuffer_GetUint8(myAddress);
            macAsUint64 <<= 8;
            macAsUint64 |= b;
        }
        // the mac address is only 6 bytes, so shift two more
        macAsUint64 <<= 16;
        parcBuffer_Rewind(myAddress);

        // loopback interface as a 0-length link address
        if (parcBuffer_Remaining(myAddress) > 0) {
            etherListener->localAddress = cpiAddress_CreateFromLink(parcBuffer_Overlay(myAddress, 0), parcBuffer_Remaining(myAddress));
        }
    }

    etherListener->destinationAddressList = parcMemory_AllocateAndClear(sizeof(uint64_t) * 3);
    etherListener->destinationAddressSize = 3;
    etherListener->destinationAddressList[0] = htonll(macAsUint64);         // our address
    etherListener->destinationAddressList[1] = htonll(0x01005E0017AA0000);  // CCN address
    etherListener->destinationAddressList[2] = htonll(0xFFFFFFFFFFFF0000);  // broadcast

    etherListener->sourceAddressList = parcMemory_AllocateAndClear(sizeof(uint64_t));
    etherListener->sourceAddressSize = 1;
    etherListener->sourceAddressList[0] = htonll(macAsUint64);         // our address
}

static void
_metisEtherListener_ReleaseEthernetAddresses(_MetisEtherListener *etherListener)
{
    parcMemory_Deallocate((void **) &etherListener->destinationAddressList);
    parcMemory_Deallocate((void **) &etherListener->sourceAddressList);

    if (etherListener->localAddress) {
        cpiAddress_Destroy(&etherListener->localAddress);
    }
}

MetisListenerOps *
metisEtherListener_Create(MetisForwarder *metis, const char *deviceName, uint16_t ethertype)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    assertNotNull(deviceName, "Parameter deviceName must be non-null");

    _MetisEtherListener *etherListener = parcMemory_AllocateAndClear(sizeof(_MetisEtherListener));
    etherListener->ethertype = ethertype;
    etherListener->genericEther = metisGenericEther_Create(metis, deviceName, etherListener->ethertype);

    MetisListenerOps *ops = NULL;

    if (etherListener->genericEther != NULL) {
        etherListener->metis = metis;
        etherListener->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));
        etherListener->nextReadBuffer = parcEventBuffer_Create();
        etherListener->id = metisForwarder_GetNextConnectionId(metis);

        int etherSocket = metisGenericEther_GetDescriptor(etherListener->genericEther);
        bool persistent = true;

        // now wrap it in an event callback
        etherListener->ether_event = metisDispatcher_CreateNetworkEvent(
            metisForwarder_GetDispatcher(etherListener->metis),
            persistent,
            _metisEtherListener_ReadCallback,
            etherListener,
            etherSocket);

        assertNotNull(etherListener->ether_event, "got null event from metisDispatcher_CreateNetworkEvent: %s", strerror(errno));

        // Setup the destination and source ethernet addresses we want to use
        _metisEtherListener_FillInEthernetAddresses(etherListener);

        // Finished all initialization, so start the network event.
        metisDispatcher_StartNetworkEvent(metisForwarder_GetDispatcher(etherListener->metis), etherListener->ether_event);

        // Construct an instance of the MetisListenerOps particular to this context.
        ops = parcMemory_Allocate(sizeof(MetisListenerOps));
        assertNotNull(ops, "Got null from parc_memory_new");

        memcpy(ops, &_etherTemplate, sizeof(MetisListenerOps));
        ops->context = etherListener;

        if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
            char *str = cpiAddress_ToString(etherListener->localAddress);
            metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                            "Create Ethernet Listener id %d on %s addr %s ethertype 0x%04x ether socket %d",
                            etherListener->id,
                            deviceName,
                            str,
                            ethertype,
                            etherSocket);
            parcMemory_Deallocate((void **) &str);
        }
    } else {
        // failed to setup an Ethernet device
        parcMemory_Deallocate((void **) &etherListener);
    }

    return ops;
}

MetisGenericEther *
metisEtherListener_GetGenericEtherFromListener(MetisListenerOps *listenerOps)
{
    assertNotNull(listenerOps, "Parameter listenerOps must be non-null");
    assertTrue(listenerOps->getEncapType(listenerOps) == METIS_ENCAP_ETHER, "Can only call on a METIS_ENCAP_ETHER listener");

    _MetisEtherListener *etherListener = (_MetisEtherListener *) listenerOps->context;
    return etherListener->genericEther;
}

static void
_metisEtherListener_Destroy(_MetisEtherListener **listenerPtr)
{
    assertNotNull(listenerPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listenerPtr, "Parameter must derefernce to non-null pointer");

    _MetisEtherListener *etherListener = *listenerPtr;
    parcEventBuffer_Destroy(&(etherListener->nextReadBuffer));
    metisDispatcher_DestroyNetworkEvent(metisForwarder_GetDispatcher(etherListener->metis), &etherListener->ether_event);

    metisLogger_Release(&etherListener->logger);

    metisGenericEther_Release(&etherListener->genericEther);
    _metisEtherListener_ReleaseEthernetAddresses(etherListener);
    parcMemory_Deallocate((void **) &etherListener);
    *listenerPtr = NULL;
}

static void
_metisEtherListener_OpsDestroy(MetisListenerOps **listenerOpsPtr)
{
    MetisListenerOps *ops = *listenerOpsPtr;
    _MetisEtherListener *etherListener = (_MetisEtherListener *) ops->context;
    _metisEtherListener_Destroy(&etherListener);
    parcMemory_Deallocate((void **) &ops);
    *listenerOpsPtr = NULL;
}

static unsigned
_metisEtherListener_OpsGetInterfaceIndex(const MetisListenerOps *ops)
{
    _MetisEtherListener *etherListener = (_MetisEtherListener *) ops->context;
    return etherListener->id;
}

static const CPIAddress *
_metisEtherListener_OpsGetListenAddress(const MetisListenerOps *ops)
{
    _MetisEtherListener *etherListener = (_MetisEtherListener *) ops->context;
    return etherListener->localAddress;
}

static MetisEncapType
_metisEtherListener_OpsGetEncapType(const MetisListenerOps *ops)
{
    return METIS_ENCAP_ETHER;
}

static int
_metisEtherListener_OpsGetSocket(const MetisListenerOps *ops)
{
    _MetisEtherListener *etherListener = (_MetisEtherListener *) ops->context;
    return etherListener->ether_fd;
}


// =============================================
// Internal functions

/**
 * Construct an address pair to match the remote
 *
 * The pair will always be (ourMacAddress, header->sourceAddress), even if the
 * packet was received via a group or broadcast dmac.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static MetisAddressPair *
_metisEtherListener_ConstructAddressPair(_MetisEtherListener *etherListener, PARCEventBuffer *buffer)
{
    struct ether_header *header = (struct ether_header *) parcEventBuffer_Pullup(buffer, ETHER_HDR_LEN);

    CPIAddress *remoteAddress = cpiAddress_CreateFromLink(header->ether_shost, ETHER_ADDR_LEN);

    MetisAddressPair *pair = metisAddressPair_Create(etherListener->localAddress, remoteAddress);
    cpiAddress_Destroy(&remoteAddress);

    return pair;
}

/**
 * Lookup a connection in the connection table based on an address pair
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] etherListener An allocated MetisEtherListener
 * @param [in] pair The Address pair to lookup
 *
 * @return null Not found
 * @return non-null The connection
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static const MetisConnection *
_metisEtherListener_LookupConnectionId(_MetisEtherListener *etherListener, MetisAddressPair *pair)
{
    MetisConnectionTable *connTable = metisForwarder_GetConnectionTable(etherListener->metis);

    const MetisConnection *conn = metisConnectionTable_FindByAddressPair(connTable, pair);
    return conn;
}

/**
 * @function _metisEtherListener_CreateNewConnection
 * @abstract Creates a new Metis connection for the peer
 * @discussion
 *   PRECONDITION: you know there's not an existing connection with the address pair
 *
 *   Creates a new connection and adds it to the connection table.
 *
 * @param <#param1#>
 * @return The connection id for the new connection
 */
static const MetisConnection *
_metisEtherListener_CreateNewConnection(_MetisEtherListener *etherListener, MetisAddressPair *pair)
{
    // metisEtherConnection_Create takes ownership of the pair
    MetisIoOperations *ops = metisEtherConnection_Create(etherListener->metis, etherListener->genericEther, pair);
    MetisConnection *conn = metisConnection_Create(ops);

    metisConnectionTable_Add(metisForwarder_GetConnectionTable(etherListener->metis), conn);

    return conn;
}

/**
 * Read an ethernet frame and return its buffer.
 *
 * Will use ether->nextReadBuffer.  If we read a frame, will allocate a new nextReadBuffer.
 *
 * @param [in] fd The ethernet frame socket
 *
 * @return NULL could not read a frame
 * @return non-null Ethernet frame in the buffer
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static PARCEventBuffer *
_metisEtherListener_ReadEtherFrame(_MetisEtherListener *etherListener)
{
    PARCEventBuffer *readBuffer = NULL;

    bool success = metisGenericEther_ReadNextFrame(etherListener->genericEther, etherListener->nextReadBuffer);

    if (success) {
        readBuffer = etherListener->nextReadBuffer;
        etherListener->nextReadBuffer = parcEventBuffer_Create();

        if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
            metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                            "read %zu bytes",
                            parcEventBuffer_GetLength(readBuffer));
        }
    }

    return readBuffer;
}

/**
 * Compares source MAC address to our address
 *
 * buffer points to the start of the Ethernet frame.  Checks if the source address
 * is our address.
 *
 * The check is done against the array of addresses in ether->sourceAddressList
 *
 * @param [in] header The Ethernet header
 *
 * @return true  The Ethernet source address is our MAC address
 * @return false It is not our MAC address
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_metisEtherListener_IsOurSourceAddress(_MetisEtherListener *ether, struct ether_header *header)
{
    // this copies the source address, then masks our copy
    uint64_t u64_shost;
    memcpy(&u64_shost, header->ether_shost, ETHER_ADDR_LEN);
    u64_shost &= MAC_MASK;

    for (int i = 0; i < ether->sourceAddressSize; i++) {
        if (u64_shost == ether->sourceAddressList[i]) {
            return true;
        }
    }
    return false;
}

/**
 * Compares destination MAC address to our receive addresses
 *
 * The check is done against the array of addresses in ether->destinationAddressList
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
static bool
_metisEtherListener_IsOurDestinationAddress(_MetisEtherListener *ether, struct ether_header *header)
{
    // this copies the source address, then masks our copy
    uint64_t u64_dhost;
    memcpy(&u64_dhost, header->ether_dhost, ETHER_ADDR_LEN);
    u64_dhost &= MAC_MASK;

    for (int i = 0; i < ether->destinationAddressSize; i++) {
        if (u64_dhost == ether->destinationAddressList[i]) {
            return true;
        }
    }
    return false;
}

/**
 * <#One Line Description#>
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
static bool
_metisEtherListener_IsOurProtocol(_MetisEtherListener *ether, struct ether_header *header)
{
    // TODO: check the ethertype
    return true;
}

typedef enum {
    ParseResult_Accept,
    ParseResult_Reject,
    ParseResult_Error
} _ParseResult;

/**
 * Processes an ethernet frame to make sure its for us
 *
 * Ensures that the frame is for us, and not from our source address.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return ParseResult_Accept We should recieve and process the frame
 * @return ParseResult_Reject Do not receive the frame
 * @return ParseResult_Error There was an error looking at the Ethernet header
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static _ParseResult
_metisEtherListener_ParseEtherFrame(_MetisEtherListener *etherListener, PARCEventBuffer *buffer)
{
    _ParseResult result = ParseResult_Error;

    struct ether_header *header = (struct ether_header *) parcEventBuffer_Pullup(buffer, ETHER_HDR_LEN);

    if (header) {
        result = ParseResult_Reject;
        if (_metisEtherListener_IsOurProtocol(etherListener, header)) {
            if (_metisEtherListener_IsOurDestinationAddress(etherListener, header)) {
                if (!_metisEtherListener_IsOurSourceAddress(etherListener, header)) {
                    // ok, it is the right protocol, a good destination address
                    // and not our source address.  We should ccept this.

                    result = ParseResult_Accept;
                }
            }
        }
    }

    return result;
}

static const MetisConnection *
_metisEtherListener_LookupOrCreateConnection(_MetisEtherListener *etherListener, PARCEventBuffer *buffer)
{
    MetisAddressPair *pair = _metisEtherListener_ConstructAddressPair(etherListener, buffer);

    const MetisConnection *conn = _metisEtherListener_LookupConnectionId(etherListener, pair);

    if (!conn) {
        conn = _metisEtherListener_CreateNewConnection(etherListener, pair);

        if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
            char *str = metisAddressPair_ToString(pair);
            metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                            "Create connid %u address pair %s", metisConnection_GetConnectionId(conn), str);
            free(str);
        }
    }
    metisAddressPair_Release(&pair);

    return conn;
}

/**
 * Accept a fragment, put it in rassembler, and pass reassembled frames up stack.
 *
 * precondition: message is not null
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
_acceptFragment(_MetisEtherListener *etherListener, const MetisConnection *conn, MetisMessage *message)
{
    MetisHopByHopFragmenter *fragmenter = metisEtherConnection_GetFragmenter(conn);
    assertNotNull(fragmenter, "Could not get fragmenter from the underlying connection");

    bool receiveQueueNotEmpty = metisHopByHopFragmenter_Receive(fragmenter, message);
    if (receiveQueueNotEmpty) {
        MetisMessage *assembled = NULL;
        while ((assembled = metisHopByHopFragmenter_PopReceiveQueue(fragmenter)) != NULL) {
            etherListener->stats.framesReassembled++;
            metisForwarder_Receive(etherListener->metis, assembled);
        }
    }
}

static void
_acceptFrame(_MetisEtherListener *etherListener, PARCEventBuffer *buffer, int fd)
{
    const MetisConnection *conn = _metisEtherListener_LookupOrCreateConnection(etherListener, buffer);

    // remove the ethernet header
    parcEventBuffer_Read(buffer, NULL, ETHER_HDR_LEN);

    // takes ownership of buffer (will destroy it if there's an error)
    MetisMessage *message = metisMessage_CreateFromBuffer(metisConnection_GetConnectionId(conn), metisForwarder_GetTicks(etherListener->metis), buffer, etherListener->logger);

    size_t readLength = parcEventBuffer_GetLength(buffer);
    if (message) {
        etherListener->stats.framesReceived++;

        if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
            metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                            "read %zu bytes from fd %d connid %d",
                            readLength,
                            fd,
                            metisConnection_GetConnectionId(conn));
            _logStats(etherListener, PARCLogLevel_Debug);
        }

        _acceptFragment(etherListener, conn, message);
        metisMessage_Release(&message);

    } else {
        etherListener->stats.framesError++;

        if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning)) {
            metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                            "read %zu bytes from fd %d connid %d: Error parsing skeleton",
                            readLength,
                            fd,
                            metisConnection_GetConnectionId(conn));
            _logStats(etherListener, PARCLogLevel_Warning);
        }
    }
}

static void
_rejectFrame(_MetisEtherListener *etherListener, PARCEventBuffer *buffer, int fd)
{
    etherListener->stats.framesNotForUs++;

    if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning)) {
        metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                        "read %zu bytes from fd %d: reject frame",
                        parcEventBuffer_GetLength(buffer),
                        fd);
        _logStats(etherListener, PARCLogLevel_Warning);
    }
}

static void
_errorFrame(_MetisEtherListener *etherListener, PARCEventBuffer *buffer, int fd)
{
    etherListener->stats.framesError++;

    if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning)) {
        metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                        "read %zu bytes from fd %d: error parsing Ethernet header",
                        parcEventBuffer_GetLength(buffer),
                        fd);
        _logStats(etherListener, PARCLogLevel_Warning);
    }
}

static void
_metisEtherListener_ReadCallback(int fd, PARCEventType what, void *user_data)
{
    // ether is datagram based, we don't have a connection
    _MetisEtherListener *etherListener = (_MetisEtherListener *) user_data;

    if (metisLogger_IsLoggable(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        metisLogger_Log(etherListener->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "socket %d what %s%s%s%s data %p",
                        fd,
                        (what & PARCEventType_Timeout) ? " timeout" : "",
                        (what & PARCEventType_Read)    ? " read" : "",
                        (what & PARCEventType_Write)   ? " write" : "",
                        (what & PARCEventType_Signal)  ? " signal" : "",
                        user_data);
    }

    if (what & PARCEventType_Read) {
        while (true) {
            PARCEventBuffer *buffer = _metisEtherListener_ReadEtherFrame(etherListener);

            if (buffer == NULL) {
                break;
            }

            etherListener->stats.framesIn++;

            _ParseResult result = _metisEtherListener_ParseEtherFrame(etherListener, buffer);

            switch (result) {
                case ParseResult_Accept:
                    _acceptFrame(etherListener, buffer, fd);
                    break;

                case ParseResult_Reject:
                    _rejectFrame(etherListener, buffer, fd);
                    parcEventBuffer_Destroy(&buffer);
                    break;

                case ParseResult_Error:
                    _errorFrame(etherListener, buffer, fd);
                    parcEventBuffer_Destroy(&buffer);
                    break;

                default:
                    trapUnexpectedState("Do not understand parse result %d", result);
            }
        }
    }
}
