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
 * Implements a listener that works with stream connections over a named pipe.
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>

#include <ccnx/forwarder/metis/core/metis_ConnectionTable.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/io/metis_Listener.h>
#include <ccnx/forwarder/metis/io/metis_LocalListener.h>
#include <ccnx/forwarder/metis/io/metis_StreamConnection.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>

struct metis_local_listener {
    MetisForwarder *metis;
    MetisLogger *logger;
    PARCEventSocket *listener;
    CPIAddress *localAddress;
    unsigned id;
};

static void               _metisLocalListener_OpsDestroy(MetisListenerOps **listenerOpsPtr);
static unsigned           _metisLocalListener_OpsGetInterfaceIndex(const MetisListenerOps *ops);
static const CPIAddress  *_metisLocalListener_OpsGetListenAddress(const MetisListenerOps *ops);
static MetisEncapType     _metisLocalListener_OpsGetEncapType(const MetisListenerOps *ops);

static MetisListenerOps localTemplate = {
    .context           = NULL,
    .destroy           = &_metisLocalListener_OpsDestroy,
    .getInterfaceIndex = &_metisLocalListener_OpsGetInterfaceIndex,
    .getListenAddress  = &_metisLocalListener_OpsGetListenAddress,
    .getEncapType      = &_metisLocalListener_OpsGetEncapType,
};

// STREAM daemon listener callback
static void metisListenerLocal_Listen(int, struct sockaddr *, int socklen, void *localVoid);

MetisListenerOps *
metisLocalListener_Create(MetisForwarder *metis, const char *path)
{
    MetisLocalListener *local = parcMemory_AllocateAndClear(sizeof(MetisLocalListener));
    assertNotNull(local, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisLocalListener));
    local->metis = metis;
    local->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));

    struct sockaddr_un addr_unix;
    memset(&addr_unix, 0, sizeof(addr_unix));

    addr_unix.sun_family = PF_UNIX;
    strcpy(addr_unix.sun_path, path);

    unlink(path);

    local->listener = metisDispatcher_CreateListener(metisForwarder_GetDispatcher(metis), metisListenerLocal_Listen,
                                                     (void *) local, -1, (struct sockaddr*) &addr_unix, sizeof(addr_unix));

    assertNotNull(local->listener, "Got null listener from metisDispatcher_CreateListener: (%d) %s", errno, strerror(errno));

    struct sockaddr_un addr_un;
    memset(&addr_un, 0, sizeof(addr_un));
    addr_un.sun_family = AF_UNIX;
    strcpy(addr_un.sun_path, path);

    local->localAddress = cpiAddress_CreateFromUnix(&addr_un);
    local->id = metisForwarder_GetNextConnectionId(metis);

    MetisListenerOps *ops = parcMemory_AllocateAndClear(sizeof(MetisListenerOps));
    assertNotNull(ops, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisListenerOps));
    memcpy(ops, &localTemplate, sizeof(MetisListenerOps));
    ops->context = local;

    return ops;
}

void
metisLocalListener_Destroy(MetisLocalListener **listenerPtr)
{
    assertNotNull(listenerPtr, "Parameter must be non-null double pointer");
    assertNotNull(*listenerPtr, "Parameter must dereference to non-null pointer");

    MetisLocalListener *local = *listenerPtr;

    metisLogger_Release(&local->logger);

    cpiAddress_Destroy(&local->localAddress);
    metisDispatcher_DestroyListener(metisForwarder_GetDispatcher(local->metis), &local->listener);

    parcMemory_Deallocate((void **) &local);
    *listenerPtr = NULL;
}

// ==================================================

/**
 * @function metisListenerLocal_Listen
 * @abstract Called when a client connects to the server socket
 * @discussion
 *   Accepts a client connection.  Creates a new Stream connection and adds it
 *   to the connection table.
 *
 * @param fd the remote client socket (it will be AF_UNIX type)
 * @param sa the remote client address
 * @param socklen the bytes of sa
 * @param localVoid a void point to the MetisLocalListener that owns the server socket
 */
static void
metisListenerLocal_Listen(int fd,
                          struct sockaddr *sa, int socklen, void *localVoid)
{
    MetisLocalListener *local = (MetisLocalListener *) localVoid;
    assertTrue(sa->sa_family == AF_UNIX, "Got wrong address family, expected %d got %d", AF_UNIX, sa->sa_family);

    CPIAddress *remote = cpiAddress_CreateFromUnix((struct sockaddr_un *) sa);
    MetisAddressPair *pair = metisAddressPair_Create(local->localAddress, remote);

    MetisIoOperations *ops = metisStreamConnection_AcceptConnection(local->metis, fd, pair, true);
    MetisConnection *conn = metisConnection_Create(ops);

    metisConnectionTable_Add(metisForwarder_GetConnectionTable(local->metis), conn);

    if (metisLogger_IsLoggable(local->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        char *str = metisAddressPair_ToString(pair);
        metisLogger_Log(local->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "Listener %p started on address pair %s", (void *) local, str);
        free(str);
    }

    cpiAddress_Destroy(&remote);
}

static void
_metisLocalListener_OpsDestroy(MetisListenerOps **listenerOpsPtr)
{
    MetisListenerOps *ops = *listenerOpsPtr;
    MetisLocalListener *local = (MetisLocalListener *) ops->context;

    if (metisLogger_IsLoggable(local->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        metisLogger_Log(local->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "Listener %p destroyed", (void *) local);
    }

    metisLocalListener_Destroy(&local);
    parcMemory_Deallocate((void **) &ops);
    *listenerOpsPtr = NULL;
}

static unsigned
_metisLocalListener_OpsGetInterfaceIndex(const MetisListenerOps *ops)
{
    MetisLocalListener *local = (MetisLocalListener *) ops->context;
    return local->id;
}

static const CPIAddress *
_metisLocalListener_OpsGetListenAddress(const MetisListenerOps *ops)
{
    MetisLocalListener *local = (MetisLocalListener *) ops->context;
    return local->localAddress;
}

static MetisEncapType
_metisLocalListener_OpsGetEncapType(const MetisListenerOps *ops)
{
    return METIS_ENCAP_LOCAL;
}
