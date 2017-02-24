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
 * @file rta_Component.h
 * @brief <#Brief Description#>
 *
 * A Component is a functional block within a protocol stack.  It exists
 * between the API Connector (at the top) and the Forwarder Connector
 * (at the bottom).  All components have a similar interface.  The only
 * slight variation is that components betwen the Forwarder Connector
 * and the Codec deal in "wire" message formats, while components above
 * the connector deal with "parsed" (CCNxMessage) formats.
 *
 * To write a component, follow these procedures:
 * 1) add your component's name to components.h enum.  This is the
 *    symbolic name you will use for it in the code.  We'll call
 *    it PROTO_WIZ.
 * 2) Copy a skeleton, such as component_Verifier.h for your header.
 *    Let's call it component_Wizard.h.  Inside the header, you'll
 *    define the "operations" structure that's exported to the system.
 * @code{.c}
 **#ifndef Libccnx_component_wizard_h
 **#define Libccnx_component_wizard_h
 *
 * // Function structs for component variations
 * extern ComponentOperations proto_wizard_ops;
 *
 **#endif
 * @endcode
 *
 * 3) Copy a skeleton, like component_Verifier_Null.c, for your
 *    implementation.  Let's call it component_Wizard.c.  Inside
 *    you must:
 *    a) instantiate proto_wizard_ops:
 * @code{.c}
 * static int  component_Wizard_Init(ProtocolStack *stack);
 * static int  component_Wizard_Opener(RtaConnection *conn);
 * static void component_Wizard_Upcall_Read(PARCEventQueue *, void *conn);
 * static void component_Wizard_Downcall_Read(PARCEventQueue *, void *conn);
 * static int  component_Wizard_Closer(RtaConnection *conn);
 * static int  component_Wizard_Release(ProtocolStack *stack);
 *
 * ComponentOperations verify_null_ops = {
 *   component_Wizard_Init,
 *   component_Wizard_Opener,
 *   component_Wizard_Upcall_Read,
 *   NULL,
 *   component_Wizard_Downcall_Read,
 *   NULL,
 *   component_Wizard_Closer,
 *   component_Wizard_Release
 * };
 * @endcode
 *
 *       These define the interface your component exposes to the stack
 *       Init:  called once on stack creation
 *       Open:  called once per connection Open
 *       UpcallRead: Called when the "upward" buffer has something to read
 *       DowncallRead: Called when the "downward" buffer has something to read
 *       Closer: called once per connection Close
 *       Release: called on protocol stack destruction.
 *
 *       Optionally, you may include UpcallEvents and DowncallEvents, but
 *       in general those are not useful.
 *
 *       Any of the function pointers in the "ops" may be NULL.
 *
 *    b) Implement your Init.  If you need to create a stack-wide data structure
 *       to track state, you would do something like this, which allocates
 *       memory and sticks it away in component-specific storage in the stack.
 *       Notice that protocolStack_SetPrivateData takes our protocol's name
 *       PROTO_WIZ as a parameter.
 *
 * @code{.c}
 * static int
 * component_Wizard_Init(ProtocolStack *stack)
 * {
 *  struct mydata *data = mydata_Create();
 *  protocolStack_SetPrivateData(stack, PROTO_WIZ, data);
 *  return 0;
 * }
 * @endcode
 *
 *    c) Implement your Opener.  You will very likely want to keep per-connection
 *       state.  This follows a similar method to the Init, but in a connection.
 *       We squirl away the connection-specific data similarly to the stack-wide
 *       data.  In addition, it's good practice to fetch your component's Stats
 *       for the connection and increment the OPENS counter for a successful open.
 *
 * @code{.c}
 * static int
 * component_Wizard_Opener(RtaConnection *connection)
 * {
 *   ComponentStats *stats;
 *   struct myState *mystate;
 *
 *   parcMemory_AlocateAndClear(&mystate, sizeof(void *), sizeof(struct api_conn_state));
 *   rtaConnection_SetPrivateData(connection, PROTO_WIZ, mystate);
 *
 *   stats = rtaConnection_GetStats(connection, PROTO_WIZ);
 *   stats_Increment(stats, STATS_OPENS);
 *   return 0;
 * }
 * @endcode
 *
 *    d) Implement your Close and Release.  These perform the inverse
 *       of the Open and Init.  They should fetch your private data, if
 *       any, and free it:
 * @code{.c}
 * static int
 * component_Wizard_Closer(RtaConnection *conn)
 * {
 *   ComponentStats *stats   = rtaConnection_GetStats(conn, PROTO_WIZ);
 *   struct myState *mystate = rtaConnection_GetPrivateData(conn, PROTO_WIZ);
 *
 *   stats_Increment(stats, STATS_CLOSES);
 *   myState_Destroy(&mystate);
 *   return 0;
 * }
 *
 * static int
 * component_Wizard_Release(ProtocolStack *stack)
 * {
 *   ComponentStats *stats   = protocoLStack_GetStats(stack, PROTO_WIZ);
 *   struct myData *mydata = protocolStack_GetPrivateData(stack, PROTO_WIZ);
 *
 *   stats_Increment(stats, STATS_CLOSES);
 *   myData_Destroy(&mydata);
 *   return 0;
 * }
 * @endcode
 *
 *    d) Implement your Read handlers.  They are similar for the upcall
 *       and downcall handlers.  The main issue to be aware of is that
 *       you must *drain* the queue on each call.  The callback is edge
 *       triggered.
 *
 *       Below we show an example of the Upcall read callback, which means
 *       there is data from below travelling up the stack. Therefore, we
 *       retrieve the RTA_UP output queue to pass messages up the stack.
 *       The while() loop is what drains the queue.
 *
 *       Note also that "ptr" is a pointer to the ProtocolStack that owns
 *       connecition (what your Init was called with).  The Connection information
 *       rides inside the transport message, and is retrieved with a call
 *       to transportMessage_GetInfo().
 *
 * @code{.c}
 * static void
 * component_Wizard_Upcall_Read(PARCEventQueue *in, PARCEvent_EventType event, void *ptr)
 * {
 *   ProtocolStack *stack = (ProtocolStack *) ptr;
 *   PARCEventQueue *out = protocoStack_GetPutQueue(stack, PROTO_WIZ, RTA_UP);
 *   TransportMessage *tm;
 *
 *   while( (tm = rtaComponent_GetMessage(in)) != NULL )
 *   {
 *       RtaConnection  *conn  = transportMessage_GetInfo(tm);
 *       ComponentStats *stats = rtaConnection_GetStats(conn, PROTO_WIZ);
 *       CCNxMessage *msg      = TransportMessage_GetCcnxMessage(tm);
 *
 *       stats_Increment(stats, STATS_UPCALL_IN);
 *
 *       // do something with the CCNxMessage
 *
 *       if( rtaComponent_PutMessage(out, tm) )
 *           stats_Increment(stats, STATS_UPCALL_OUT);
 *   }
 * }
 * @endcode
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
/**
 */

#ifndef Libccnx_rta_component_h
#define Libccnx_rta_component_h

#include "components.h"
#include "rta_ComponentQueue.h"
#include "rta_ComponentStats.h"

/**
 * Init:         one time initialization on first instantiation (0 success, -1 failure)
 * Open:         Per connection open, returns valid descriptor or -1 on failure
 * upcallRead:   Callback when one or more messages are available
 * downcallRead: Callback when one or more messages are available.
 * xEvent:       Called for events on the queue
 * Close:        Per connection close
 * Release:      One time release of state when whole stack taken down
 * stateChagne:  Called when there is a state change related to the connection
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef struct {
    int (*init)(RtaProtocolStack *stack);
    int (*open)(RtaConnection *conn);
    void (*upcallRead)(PARCEventQueue *queue, PARCEventType events, void *stack);
    void (*upcallEvent)(PARCEventQueue *queue, PARCEventQueueEventType events, void *stack);
    void (*downcallRead)(PARCEventQueue *queue, PARCEventType events, void *stack);
    void (*downcallEvent)(PARCEventQueue *queue, PARCEventQueueEventType events, void *stack);
    int (*close)(RtaConnection *conn);
    int (*release)(RtaProtocolStack *stack);
    void (*stateChange)(RtaConnection *conn);
} RtaComponentOperations;

extern PARCEventQueue *rtaComponent_GetOutputQueue(RtaConnection *conn,
                                                   RtaComponents component,
                                                   RtaDirection direction);

/**
 * Send a message between components.  The API connector and Forwarder connector
 * must set the connection information in the transport message with
 * rtaConnection_SetInTransport().
 *
 * returns 1 on success, 0 on failure
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
extern int rtaComponent_PutMessage(PARCEventQueue *queue, TransportMessage *tm);

/**
 * Fetch a message from the queue.  Will return NULL if no message
 * is available.
 *
 * As a side effect, it will drain message on a closed connection.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
extern TransportMessage *rtaComponent_GetMessage(PARCEventQueue *queue);
#endif
