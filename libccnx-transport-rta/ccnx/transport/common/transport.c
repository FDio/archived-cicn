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
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include "transport.h"
#include "transport_private.h"
#include "rta_Transport.h"

struct transport_context {
    TransportTypes transport_type;
    struct transport_operations ops;
    void *transport_data;
    unsigned references;
};

// the one global transport, for now
static TransportContext *the_context = NULL;

TransportContext *
Transport_Create(TransportTypes type)
{
    if (the_context == NULL) {
        switch (type) {
            case TRANSPORT_RTA:
                the_context = parcMemory_Allocate(sizeof(TransportContext));
                assertNotNull(the_context, "TransportContext could not be allocated, parcMemory_Allocate(%zu) returned NULL", sizeof(TransportContext));

                the_context->references = 0;
                the_context->ops = rta_ops;
                the_context->transport_data = the_context->ops.Create();
                the_context->transport_type = type;
                break;

            default:
                fprintf(stderr, "%s unknown transport type %d\n", __func__, type);
                abort();
                break;
        }
    }

    if (the_context->transport_type == type) {
        the_context->references++;
        return the_context;
    }

    fprintf(stderr, "%s transport type %d not of request type %d\n",
            __func__, the_context->transport_type, type);
    abort();
}

int
Transport_Open(CCNxTransportConfig *transportConfig)
{
    assertNotNull(the_context, "The TransportContext is NULL.");
    assertNotNull(transportConfig, "The parameter transportConfig must be a non-null CCNxTransportConfig pointer");
    return the_context->ops.Open(the_context->transport_data, transportConfig);
}

int
Transport_Send(int desc, CCNxMetaMessage *msg_in)
{
    assertNotNull(the_context, "the_context is null");
    return the_context->ops.Send(the_context->transport_data, desc, msg_in, CCNxStackTimeout_Never);
}

TransportIOStatus
Transport_Recv(int desc, CCNxMetaMessage **msg_out)
{
    return the_context->ops.Recv(the_context->transport_data, desc, msg_out, CCNxStackTimeout_Never);
}

int
Transport_Close(int desc)
{
    return the_context->ops.Close(the_context->transport_data, desc);
}

int
Transport_PassCommand(void *stackCommand)
{
    return the_context->ops.PassCommand(the_context->transport_data, stackCommand);
}

void
Transport_Destroy(TransportContext **ctxPtr)
{
    assertNotNull(ctxPtr, "Transport_Destroy called with null context");
    assertNotNull(*ctxPtr, "Transport_Destroy callled with reference to null");

    TransportContext *ctx = *ctxPtr;

    assertTrue(the_context == ctx, "Passed ctx is not the same");
    assertTrue(ctx->references > 0, "Invalid reference count");

    ctx->references--;
    if (ctx->references == 0) {
        ctx->ops.Destroy(&ctx->transport_data);
        memset(ctx, 0, sizeof(TransportContext));
        parcMemory_Deallocate((void **) &ctx);
        ctx = NULL;
        the_context = NULL;
    }
}
