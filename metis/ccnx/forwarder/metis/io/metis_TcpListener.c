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
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <ccnx/forwarder/metis/io/metis_TcpListener.h>
#include <ccnx/forwarder/metis/io/metis_StreamConnection.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_ConnectionTable.h>
#include <ccnx/forwarder/metis/io/metis_Listener.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Network.h>

typedef struct metis_tcp_listener {
    MetisForwarder *metis;
    MetisLogger *logger;

    PARCEventSocket *listener;

    CPIAddress *localAddress;

    unsigned id;

    // is the localAddress as 127.0.0.0 address?
    bool isLocalAddressLocal;
} _MetisTcpListener;

static void _metisTcpListener_Destroy(_MetisTcpListener **listenerPtr);

static void               _metisTcpListener_OpsDestroy(MetisListenerOps **listenerOpsPtr);
static unsigned           _metisTcpListener_OpsGetInterfaceIndex(const MetisListenerOps *ops);
static const CPIAddress *_metisTcpListener_OpsGetListenAddress(const MetisListenerOps *ops);
static MetisEncapType     _metisTcpListener_OpsGetEncapType(const MetisListenerOps *ops);

static MetisListenerOps _tcpTemplate = {
    .context           = NULL,
    .destroy           = &_metisTcpListener_OpsDestroy,
    .getInterfaceIndex = &_metisTcpListener_OpsGetInterfaceIndex,
    .getListenAddress  = &_metisTcpListener_OpsGetListenAddress,
    .getEncapType      = &_metisTcpListener_OpsGetEncapType,
    .getSocket         = NULL
};

// STREAM daemon listener callback
static void _metisTcpListener_Listen(int, struct sockaddr *, int socklen, void *tcpVoid);


MetisListenerOps *
metisTcpListener_CreateInet6(MetisForwarder *metis, struct sockaddr_in6 sin6)
{
    _MetisTcpListener *tcp = parcMemory_AllocateAndClear(sizeof(_MetisTcpListener));
    assertNotNull(tcp, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisTcpListener));

    tcp->metis = metis;
    tcp->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));

    tcp->listener = metisDispatcher_CreateListener(metisForwarder_GetDispatcher(metis), _metisTcpListener_Listen,
                                                   (void *) tcp, -1, (struct sockaddr*) &sin6, sizeof(sin6));

    if (tcp->listener == NULL) {
        metisLogger_Log(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                        "metisDispatcher_CreateListener failed to create listener (%d) %s",
                        errno, strerror(errno));
        metisLogger_Release(&tcp->logger);
        parcMemory_Deallocate((void **) &tcp);
        return NULL;
    }

    tcp->localAddress = cpiAddress_CreateFromInet6(&sin6);
    tcp->id = metisForwarder_GetNextConnectionId(metis);
    tcp->isLocalAddressLocal = parcNetwork_IsSocketLocal((struct sockaddr *) &sin6);

    MetisListenerOps *ops = parcMemory_AllocateAndClear(sizeof(MetisListenerOps));
    assertNotNull(ops, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisListenerOps));

    memcpy(ops, &_tcpTemplate, sizeof(MetisListenerOps));
    ops->context = tcp;

    if (metisLogger_IsLoggable(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        char *str = cpiAddress_ToString(tcp->localAddress);
        metisLogger_Log(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "TcpListener %p created for address %s (isLocal %d)",
                        (void *) tcp, str, tcp->isLocalAddressLocal);
        parcMemory_Deallocate((void **) &str);
    }

    return ops;
}

MetisListenerOps *
metisTcpListener_CreateInet(MetisForwarder *metis, struct sockaddr_in sin)
{
    _MetisTcpListener *tcp = parcMemory_AllocateAndClear(sizeof(_MetisTcpListener));
    assertNotNull(tcp, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisTcpListener));

    tcp->metis = metis;
    tcp->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));
    tcp->listener = metisDispatcher_CreateListener(metisForwarder_GetDispatcher(metis), _metisTcpListener_Listen,
                                                   (void *) tcp, -1, (struct sockaddr*) &sin, sizeof(sin));

    if (tcp->listener == NULL) {
        metisLogger_Log(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                        "metisDispatcher_CreateListener failed to create listener (%d) %s",
                        errno, strerror(errno));

        metisLogger_Release(&tcp->logger);
        parcMemory_Deallocate((void **) &tcp);
        return NULL;
    }

    tcp->localAddress = cpiAddress_CreateFromInet(&sin);
    tcp->id = metisForwarder_GetNextConnectionId(metis);
    tcp->isLocalAddressLocal = parcNetwork_IsSocketLocal((struct sockaddr *) &sin);

    MetisListenerOps *ops = parcMemory_AllocateAndClear(sizeof(MetisListenerOps));
    assertNotNull(ops, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisListenerOps));

    memcpy(ops, &_tcpTemplate, sizeof(MetisListenerOps));
    ops->context = tcp;

    if (metisLogger_IsLoggable(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        char *str = cpiAddress_ToString(tcp->localAddress);
        metisLogger_Log(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "TcpListener %p created for address %s (isLocal %d)",
                        (void *) tcp, str, tcp->isLocalAddressLocal);
        parcMemory_Deallocate((void **) &str);
    }

    return ops;
}

static void
_metisTcpListener_Destroy(_MetisTcpListener **listenerPtr)
{
    assertNotNull(listenerPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listenerPtr, "Parameter must derefernce to non-null pointer");
    _MetisTcpListener *tcp = *listenerPtr;

    if (metisLogger_IsLoggable(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        char *str = cpiAddress_ToString(tcp->localAddress);
        metisLogger_Log(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "TcpListener %p destroyed", (void *) tcp);
        parcMemory_Deallocate((void **) &str);
    }

    metisLogger_Release(&tcp->logger);
    metisDispatcher_DestroyListener(metisForwarder_GetDispatcher(tcp->metis), &tcp->listener);
    cpiAddress_Destroy(&tcp->localAddress);
    parcMemory_Deallocate((void **) &tcp);
    *listenerPtr = NULL;
}

// ==================================================

static void
_metisTcpListener_Listen(int fd, struct sockaddr *sa, int socklen, void *tcpVoid)
{
    _MetisTcpListener *tcp = (_MetisTcpListener *) tcpVoid;

    CPIAddress *remote;

    switch (sa->sa_family) {
        case AF_INET:
            remote = cpiAddress_CreateFromInet((struct sockaddr_in *) sa);
            break;

        case AF_INET6:
            remote = cpiAddress_CreateFromInet6((struct sockaddr_in6 *) sa);
            break;

        default:
            trapIllegalValue(sa, "Expected INET or INET6, got %d", sa->sa_family);
            abort();
    }

    MetisAddressPair *pair = metisAddressPair_Create(tcp->localAddress, remote);

    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(tcp->metis, fd, pair, tcp->isLocalAddressLocal);
    MetisConnection *conn = metisConnection_Create(ops);

    metisConnectionTable_Add(metisForwarder_GetConnectionTable(tcp->metis), conn);

    if (metisLogger_IsLoggable(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        metisLogger_Log(tcp->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "TcpListener %p listen started", (void *) tcp);
    }

    cpiAddress_Destroy(&remote);
}

static void
_metisTcpListener_OpsDestroy(MetisListenerOps **listenerOpsPtr)
{
    MetisListenerOps *ops = *listenerOpsPtr;
    _MetisTcpListener *tcp = (_MetisTcpListener *) ops->context;
    _metisTcpListener_Destroy(&tcp);
    parcMemory_Deallocate((void **) &ops);
    *listenerOpsPtr = NULL;
}

static unsigned
_metisTcpListener_OpsGetInterfaceIndex(const MetisListenerOps *ops)
{
    _MetisTcpListener *tcp = (_MetisTcpListener *) ops->context;
    return tcp->id;
}

static const CPIAddress *
_metisTcpListener_OpsGetListenAddress(const MetisListenerOps *ops)
{
    _MetisTcpListener *tcp = (_MetisTcpListener *) ops->context;
    return tcp->localAddress;
}

static MetisEncapType
_metisTcpListener_OpsGetEncapType(const MetisListenerOps *ops)
{
    return METIS_ENCAP_TCP;
}
