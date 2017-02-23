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
#include <limits.h>

#include <ccnx/forwarder/metis/io/metis_IoOperations.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>
#include <ccnx/forwarder/metis/io/metis_AddressPair.h>
#include <ccnx/forwarder/metis/core/metis_Wldr.h>
#include <ccnx/forwarder/metis/core/metis_Ticks.h>
#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

struct metis_connection {
    const MetisAddressPair *addressPair;
    MetisIoOperations *ops;

    unsigned refCount;

    bool probing_active;
    unsigned probing_interval;
    unsigned counter;
    MetisTicks last_sent;
    MetisTicks delay;

    MetisWldr *wldr;
};

MetisConnection *
metisConnection_Create(MetisIoOperations *ops)
{
    assertNotNull(ops, "Parameter ops must be non-null");
    MetisConnection *conn = parcMemory_AllocateAndClear(sizeof(MetisConnection));
    assertNotNull(conn, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisConnection));
    conn->addressPair = metisIoOperations_GetAddressPair(ops);
    conn->ops = ops;
    conn->refCount = 1;
    conn->wldr = NULL;
    conn->probing_active = false;
    conn->probing_interval = 0;
    conn->counter = 0;
    conn->last_sent = 0;
    conn->delay = INT_MAX;
    return conn;
}

MetisConnection *
metisConnection_Acquire(MetisConnection *connection)
{
    assertNotNull(connection, "Parameter conn must be non-null");
    connection->refCount++;
    return connection;
}

void
metisConnection_Release(MetisConnection **connectionPtr)
{
    assertNotNull(connectionPtr, "Parameter must be non-null double pointer");
    assertNotNull(*connectionPtr, "Parameter must dereference to non-null pointer");
    MetisConnection *conn = *connectionPtr;

    assertTrue(conn->refCount > 0, "Invalid state, connection reference count should be positive, got 0.");
    conn->refCount--;
    if (conn->refCount == 0) {
        // don't destroy addressPair, its part of ops.
        metisIoOperations_Release(&conn->ops);
        if (conn->wldr != NULL) {
            metisWldr_Destroy(&(conn->wldr));
        }
        parcMemory_Deallocate((void **) &conn);
    }
    *connectionPtr = NULL;
}

bool
metisConnection_Send(const MetisConnection *conn, MetisMessage *message)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    assertNotNull(message, "Parameter message must be non-null");

    if (metisIoOperations_IsUp(conn->ops)) {
        uint8_t connectionId = (uint8_t) metisConnection_GetConnectionId(conn);
        metisMessage_UpdatePathLabel(message, connectionId);
        if (conn->wldr != NULL) {
            metisWldr_SetLabel(conn->wldr, message);
        }
        return metisIoOperations_Send(conn->ops, NULL, message);
    }
    return false;
}


static void
_sendProbe(MetisConnection *conn, unsigned probeType)
{
    assertNotNull(conn, "Parameter conn must be non-null");

    if (probeType == METIS_PACKET_TYPE_PROBE_REQUEST) {
        MetisTicks now = metisIoOperations_SendProbe(conn->ops, probeType);
        if (now != 0) {
            conn->last_sent = now;
        }
    } else {
        metisIoOperations_SendProbe(conn->ops, probeType);
    }
}


void
metisConnection_Probe(MetisConnection *conn)
{
    _sendProbe(conn, METIS_PACKET_TYPE_PROBE_REQUEST);
}

void
metisConnection_HandleProbe(MetisConnection *conn, uint8_t *pkt, MetisTicks actualTime)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    assertNotNull(pkt, "Parameter pkt must be non-null");

    if (pkt[1] == METIS_PACKET_TYPE_PROBE_REQUEST) {
        _sendProbe(conn, METIS_PACKET_TYPE_PROBE_REPLY);
    } else if (pkt[1] == METIS_PACKET_TYPE_PROBE_REPLY) {
        MetisTicks delay = actualTime - conn->last_sent;
        if (delay == 0) {
            delay = 1;
        }
        if (delay < conn->delay) {
            conn->delay = delay;
        }
    } else {
        printf("receivde unkwon probe type\n");
    }
}

uint64_t
metisConnection_GetDelay(MetisConnection *conn)
{
    return (uint64_t) conn->delay;
}


MetisIoOperations *
metisConnection_GetIoOperations(const MetisConnection *conn)
{
    return conn->ops;
}

unsigned
metisConnection_GetConnectionId(const MetisConnection *conn)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    return metisIoOperations_GetConnectionId(conn->ops);
}

const MetisAddressPair *
metisConnection_GetAddressPair(const MetisConnection *conn)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    return metisIoOperations_GetAddressPair(conn->ops);
}

bool
metisConnection_IsUp(const MetisConnection *conn)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    return metisIoOperations_IsUp(conn->ops);
}

bool
metisConnection_IsLocal(const MetisConnection *conn)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    return metisIoOperations_IsLocal(conn->ops);
}

const void *
metisConnection_Class(const MetisConnection *conn)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    return metisIoOperations_Class(conn->ops);
}

bool
metisConnection_ReSend(const MetisConnection *conn, MetisMessage *message)
{
    assertNotNull(conn, "Parameter conn must be non-null");
    assertNotNull(message, "Parameter message must be non-null");

    if (metisConnection_IsUp(conn)) {
        //here the wldr header is alreay set: this message is a retransmission or a notification

        //we don't need to update the path label. In fact the path label was already set in the first
        //transmission of this packet (in metisConnection_Send). Since we are using pointers this
        //message has the same path label. However it could be a good idea to remove the path label
        //so that raaqm will discard this packet for the RTT estimation.

        return metisIoOperations_Send(conn->ops, NULL, message);
    }
    return false;
}

void
metisConnection_EnableWldr(MetisConnection *conn)
{
    if (!metisConnection_IsLocal(conn)) {
        if (conn->wldr == NULL) {
            printf("----------------- enable wldr\n");
            conn->wldr = metisWldr_Init();
        }
    }
}

void
metisConnection_DisableWldr(MetisConnection *conn)
{
    if (!metisConnection_IsLocal(conn)) {
        if (conn->wldr != NULL) {
            printf("----------------- disable wldr\n");
            metisWldr_Destroy(&(conn->wldr));
            conn->wldr = NULL;
        }
    }
}


bool
metisConnection_HasWldr(const MetisConnection *conn)
{
    if (conn->wldr == NULL) {
        return false;
    } else {
        return true;
    }
}

void
metisConnection_DetectLosses(MetisConnection *conn, MetisMessage *message)
{
    metisWldr_DetectLosses(conn->wldr, conn, message);
}

