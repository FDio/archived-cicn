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

#include <parc/algol/parc_EventBuffer.h>

#include <ccnx/transport/common/transport_Message.h>
#include <ccnx/transport/transport_rta/rta_Transport.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>


#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

PARCEventQueue *
rtaComponent_GetOutputQueue(RtaConnection *conn,
                            RtaComponents component,
                            RtaDirection direction)
{
    RtaProtocolStack *stack;

    assertNotNull(conn, "called with null connection\n");

    stack = rtaConnection_GetStack(conn);
    assertNotNull(stack, "resolved null stack\n");

    return rtaProtocolStack_GetPutQueue(stack, component, direction);
}

int
rtaComponent_PutMessage(PARCEventQueue *queue, TransportMessage *tm)
{
    RtaConnection *conn = rtaConnection_GetFromTransport(tm);
    assertNotNull(conn, "Got null connection from transport message\n");

    if (rtaConnection_GetState(conn) != CONN_CLOSED) {
        PARCEventBuffer *out = parcEventBuffer_GetQueueBufferOutput(queue);
        int res;

        rtaConnection_IncrementMessagesInQueue(conn);

        if (DEBUG_OUTPUT) {
            printf("%s  queue %-12s tm %p\n",
                   __func__,
                   rtaProtocolStack_GetQueueName(rtaConnection_GetStack(conn), queue),
                   (void *) tm);
        }

        res = parcEventBuffer_Append(out, (void *) &tm, sizeof(&tm));
        assertTrue(res == 0, "%s parcEventBuffer_Append returned error\n", __func__);
        parcEventBuffer_Destroy(&out);
        return 1;
    } else {
        // drop
        transportMessage_Destroy(&tm);

        return 0;
    }
}

TransportMessage *
rtaComponent_GetMessage(PARCEventQueue *queue)
{
    PARCEventBuffer *in = parcEventBuffer_GetQueueBufferInput(queue);

    while (parcEventBuffer_GetLength(in) >= sizeof(TransportMessage *)) {
        ssize_t len;
        TransportMessage *tm;
        RtaConnection *conn;

        len = parcEventBuffer_Read(in, (void *) &tm, sizeof(&tm));

        assertTrue(len == sizeof(TransportMessage *),
                   "parcEventBuffer_Read returned error");

        // Is the transport message for an open connection?
        conn = rtaConnection_GetFromTransport(tm);
        assertNotNull(conn, "%s GetInfo returnd null connection\n", __func__);

        if (DEBUG_OUTPUT) {
            printf("%s queue %-12s tm %p\n",
                   __func__,
                   rtaProtocolStack_GetQueueName(rtaConnection_GetStack(conn), queue),
                   (void *) tm);
        }

        (void) rtaConnection_DecrementMessagesInQueue(conn);

        if (rtaConnection_GetState(conn) != CONN_CLOSED) {
            parcEventBuffer_Destroy(&in);
            return tm;
        }

        // it's a closed connection

        if (DEBUG_OUTPUT) {
            printf("%s clearing connection %p reference in transport\n",
                   __func__, (void *) conn);
        }
        //drop
        transportMessage_Destroy(&tm);
    }

    parcEventBuffer_Destroy(&in);
    return NULL;
}
