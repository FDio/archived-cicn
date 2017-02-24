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
// Source code layout:
// - component_Vegas.c: the component wrapper and session multiplexing
// - vegas_Session.c:   code for a specific basename session
// - vegas_Segment.c:   code for specific segment operations

/**
 * Component behavior
 * ===================
 * This component provides flow-controlled in-order delivery of segmented
 * content objects using a sequential segment number in the last component
 * of the object name.
 *
 * The state machine described here is within a single RtaConnection.  Separate
 * connections are independent.
 *
 * Down Stack Behavior
 * ------------------------
 * When an interest comes down the stack, it will initiate a flow-controlled
 * session.  If the last component of the interest name is a segment number,
 * that is the starting segment number.  Otherwise, we assume the interest
 * name is the base name for a segmented object, including the version number.
 *
 * Other types of messages coming down the stack (e.g. control or content objects)
 * are passed down the stack unaltered.
 *
 * If an interest comes down that represents a subset of an existing flow (i.e.
 * it has a segment number beyond the current starting segment of the flow contol
 * window), the window is advanced to that segment number and any un-delivered
 * content objects are dropped.
 *
 * If an interest comes down that represents a superset of an existing flow
 * (i.e. it has a starting segment number less than the current window), the
 * current flow control sessions is re-wound to the lower sequence number
 * and continues from there.
 *
 * Up Stack Behavior
 * ------------------------
 * Non-content objects (e.g. control and interests) are passed up the stack unmodified.
 *
 * A content object that matches a flow control session is managed by the session.
 * They are only passed up the stack in-order, and will be dropped if they are outside
 * the window.
 *
 * A content object that does not match a flow control session is dropped.  That's because
 * the only interests we send down the stack are our own for flow controlled sessions, so
 * no content object should go up the stack unless its part of a flow controlled session.
 *
 * Control Messages
 * ------------------------
 * The API may cancel flow control sessions in several ways:
 *
 * 1) Close the Connection.   This will cancel all in progress sessions and drop
 *  any un-delivered objects.
 *
 * 2) Send a Control message down the stack with the base name to cancel.  The
 *  name is considered the base name of the flow and does not depend on the
 *  starting segment number.
 *
 *  { "CPI_CANCEL_FLOW" : { "FLOW_NAME" : <base name w/o segment number> } }
 *
 * Implementation Notes
 * =========================
 * For each RtaConnection, there's a {@code struct fc_connection_state}.  This
 * contains a list of in-progress sessions indexed by the hash of the base name
 * (name up to but not including final segment).  Right now, it's a linked list
 * but should be implemented as a hash table.
 *
 * Each session is represented by a {@code struct fc_session}.
 *
 * Each entry in the flow control window is a {@code fc_window_entry}.
 *
 * session->window_head and session->window_tail define the limits of the
 * congestion window.  Everything in the interval [head, tail) is expressed
 * as an interest.  The size of that interval may be larger than the
 * congestion window cwnd if we're decreaed the window.  We never decrease
 * tail, only the cwnd.
 *
 *
 * Flow Control Algorithm
 * =========================
 * Based on TCP Vegas.  Please read the Vegas paper.  We use similar
 * variable names to the paper.  Code looks quite a bit like the linux
 * tcp_vegas.c too.
 *
 * Here's the differences.  In CCN, an Interest is like an ACK token, it
 * gives the network permission to send.  The node issuing Interests needs
 * to pace them to not exceed the network capacity.  This is done by
 * observing the delay of Content Objects.  If the delay grows too quickly,
 * then we back off linearly.  If the delay is not much above what we expected
 * based on the minimum observed delay, we increase linearly.
 *
 * During slow start, the interest window (still called "cwnd") doubles
 * every other RTT until we exceed the slow_start_threshold or the delay
 * increases too much.
 *
 * The RTT is calculated every RTT based on the observed minimum RTT during
 * the previous period.
 *
 * We use RFC6298 Retransmission Timeout (RTO) calculation methods per
 * flow control session (object basename).
 *
 * Just to be clear, there are two timers working.  The RTO timer is for
 * retransmitting interests if the flow as stalled out.  The Vegas RTT
 * calculation is for congestion window calculations.
 *
 * We we receive an out-of-order content object, we'll check the earlier
 * segments to see if they have passed the Vegas RTT.  If so, we'll
 * re-express the interests.
 *
 * Each time we re-express an Interest, we might decrese the congestion
 * window.  If the last time the interest was sent was more recent than
 * the last time we decreased the congestion window, we'll decrease the
 * congestion window.  If the last expression of the interest was before
 * the most recent window decrease, the window is left alone.  This means
 * we'll only decreae the window once per re-expression.
 */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <limits.h>
#include <sys/queue.h>
#include <stdbool.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_EventQueue.h>

#include <ccnx/transport/common/transport_Message.h>
#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>
#include <ccnx/transport/transport_rta/components/component_Flowcontrol.h>
#include <ccnx/transport/test_tools/traffic_tools.h>

#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_ControlFacade.h>

#include "vegas_private.h"

#include <parc/logging/parc_LogLevel.h>

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

// ===========================================================

typedef struct fc_session_holder {
    uint64_t basename_hash;
    CCNxName      *basename;
    VegasSession  *session;

    // used by fc_connection_state to hold these
    // Should change to hashtable on the hash
    TAILQ_ENTRY(fc_session_holder) list;
} FcSessionHolder;

/**
 * This is the per-connection state.  It allows us to have multiple
 * flow control session on one connection for different names
 */
struct vegas_connection_state {
    RtaConnection           *parent_connection;
    RtaFramework            *parent_framework;

    TAILQ_HEAD(, fc_session_holder)  sessions_head;
};


// ===========================================================

static int  component_Fc_Vegas_Init(RtaProtocolStack *stack);
static int  component_Fc_Vegas_Opener(RtaConnection *conn);
static void component_Fc_Vegas_Upcall_Read(PARCEventQueue *, PARCEventType event, void *conn);
static void component_Fc_Vegas_Downcall_Read(PARCEventQueue *, PARCEventType event, void *conn);
static int  component_Fc_Vegas_Closer(RtaConnection *conn);
static int  component_Fc_Vegas_Release(RtaProtocolStack *stack);
static void component_Fc_Vegas_StateChange(RtaConnection *conn);

// Function structs for component variations
RtaComponentOperations flow_vegas_ops = {
    .init          = component_Fc_Vegas_Init,
    .open          = component_Fc_Vegas_Opener,
    .upcallRead    = component_Fc_Vegas_Upcall_Read,
    .upcallEvent   = NULL,
    .downcallRead  = component_Fc_Vegas_Downcall_Read,
    .downcallEvent = NULL,
    .close         = component_Fc_Vegas_Closer,
    .release       = component_Fc_Vegas_Release,
    .stateChange   = component_Fc_Vegas_StateChange
};


// ======
// Session related functions
static int vegas_HandleInterest(RtaConnection *conn, TransportMessage *tm);
static FcSessionHolder *vegas_LookupSession(VegasConnectionState *fc, TransportMessage *tm);
static FcSessionHolder *vegas_LookupSessionByName(VegasConnectionState *fc, CCNxName *name);

static FcSessionHolder *vegas_CreateSessionHolder(VegasConnectionState *fc, RtaConnection *conn,
                                                  CCNxName *basename, uint64_t name_hash);

static bool vegas_HandleControl(RtaConnection *conn, CCNxTlvDictionary *controlDictionary, PARCEventQueue *outputQueue);

// ================================================

static int
component_Fc_Vegas_Init(RtaProtocolStack *stack)
{
    // we don't do any stack-wide initialization
    return 0;
}

static int
component_Fc_Vegas_Opener(RtaConnection *conn)
{
    struct vegas_connection_state *fcConnState = parcMemory_AllocateAndClear(sizeof(struct vegas_connection_state));
    assertNotNull(fcConnState, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct vegas_connection_state));

    fcConnState->parent_connection = rtaConnection_Copy(conn);
    fcConnState->parent_framework = rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn));

    TAILQ_INIT(&fcConnState->sessions_head);

    rtaConnection_SetPrivateData(conn, FC_VEGAS, fcConnState);
    rtaComponentStats_Increment(rtaConnection_GetStats(conn, FC_VEGAS), STATS_OPENS);

    return 0;
}

/*
 * Read from below.
 * These should only be content objects associated with our stream.
 *
 * Non-content objects are passed up the stack.
 */
static void
component_Fc_Vegas_Upcall_Read(PARCEventQueue *in, PARCEventType event, void *stack_ptr)
{
    TransportMessage *tm;

    while ((tm = rtaComponent_GetMessage(in)) != NULL) {
        struct timeval delay = transportMessage_GetDelay(tm);

        RtaConnection *conn = rtaConnection_GetFromTransport(tm);
        RtaComponentStats *stats = rtaConnection_GetStats(conn, FC_VEGAS);

        rtaComponentStats_Increment(stats, STATS_UPCALL_IN);

        if (transportMessage_IsControl(tm)) {
            PARCEventQueue *out = rtaComponent_GetOutputQueue(conn, FC_VEGAS, RTA_UP);

            if (rtaComponent_PutMessage(out, tm)) {
                rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
            } else {
                //TODO
            }
        } else if (transportMessage_IsContentObject(tm)) {
            // this takes ownership of the transport message
            VegasConnectionState *fc = rtaConnection_GetPrivateData(conn, FC_VEGAS);
            FcSessionHolder *holder = vegas_LookupSession(fc, tm);

            // it's quite possible that we get content objects for sessions that
            // no longer exist.  They are dropped.
            if (holder != NULL) {
                vegasSession_ReceiveContentObject(holder->session, tm);
            } else {
                transportMessage_Destroy(&tm);
            }
        } else {
            PARCEventQueue *out = rtaComponent_GetOutputQueue(conn, FC_VEGAS, RTA_UP);
            if (rtaComponent_PutMessage(out, tm)) {
                rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
            } else {
                //TODO
            }
        }

        if (DEBUG_OUTPUT) {
            printf("%s total upcall reads in %" PRIu64 " out %" PRIu64 " last delay %.6f\n",
                   __func__,
                   rtaComponentStats_Get(stats, STATS_UPCALL_IN),
                   rtaComponentStats_Get(stats, STATS_UPCALL_OUT),
                   delay.tv_sec + delay.tv_usec * 1E-6);
        }
    }
}

static void
component_Fc_Vegas_Downcall_Read(PARCEventQueue *in, PARCEventType event, void *conn)
{
    RtaProtocolStack *stack = (RtaProtocolStack *) conn;
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(stack, FC_VEGAS, RTA_DOWN);
    TransportMessage *tm;

//    printf("%s reading from queue %p\n", __func__, in);

    while ((tm = rtaComponent_GetMessage(in)) != NULL) {
        RtaConnection  *conn = rtaConnection_GetFromTransport(tm);
        RtaComponentStats *stats = rtaConnection_GetStats(conn, FC_VEGAS);
        rtaComponentStats_Increment(stats, STATS_DOWNCALL_IN);

        if (transportMessage_IsControl(tm)) {
            CCNxTlvDictionary *controlDictionary = transportMessage_GetDictionary(tm);
            if (ccnxControlFacade_IsCPI(controlDictionary) && vegas_HandleControl(conn, controlDictionary, in)) {
                transportMessage_Destroy(&tm);
            } else {
                // we did not consume the message, so forward it down
                if (rtaComponent_PutMessage(out, tm)) {
                    rtaComponentStats_Increment(stats, STATS_DOWNCALL_OUT);
                }
            }
        } else if (transportMessage_IsInterest(tm)) {
            vegas_HandleInterest(conn, tm);

            // The flow controller consumes Interests going down the stack and will
            // start issuing its own interests instead.
            transportMessage_Destroy(&tm);
        } else {
            if (rtaComponent_PutMessage(out, tm)) {
                rtaComponentStats_Increment(stats, STATS_DOWNCALL_OUT);
            }
        }

        if (DEBUG_OUTPUT) {
            struct timeval delay = tm ? transportMessage_GetDelay(tm) : (struct timeval) { 0, 0 };
            printf("%s total downcall reads in %" PRIu64 " out %" PRIu64 " last delay %.6f\n",
                   __func__,
                   rtaComponentStats_Get(stats, STATS_DOWNCALL_IN),
                   rtaComponentStats_Get(stats, STATS_DOWNCALL_OUT),
                   delay.tv_sec + delay.tv_usec * 1E-6);
        }
    }
}

static int
component_Fc_Vegas_Closer(RtaConnection *conn)
{
    VegasConnectionState *fcConnState;

    assertNotNull(conn, "Got null connection\n");
    if (conn == NULL) {
        return -1;
    }

    fcConnState = rtaConnection_GetPrivateData(conn, FC_VEGAS);

    assertNotNull(fcConnState, "could not retrieve private data for FC_VEGAS on connid %u\n",
                  rtaConnection_GetConnectionId(conn));
    if (fcConnState == NULL) {
        return -1;
    }

    rtaConnection_Destroy(&fcConnState->parent_connection);

    rtaComponentStats_Increment(rtaConnection_GetStats(conn, FC_VEGAS), STATS_CLOSES);

    // close down all the sessions
    while (!TAILQ_EMPTY(&fcConnState->sessions_head)) {
        FcSessionHolder *holder = TAILQ_FIRST(&fcConnState->sessions_head);

        vegasSession_Destroy(&holder->session);

        TAILQ_REMOVE(&fcConnState->sessions_head, holder, list);
        parcMemory_Deallocate((void **) &holder);
    }

    parcMemory_Deallocate((void **) &fcConnState);

    return 0;
}

static int
component_Fc_Vegas_Release(RtaProtocolStack *stack)
{
    // no stack-wide memory
    return 0;
}

static void
component_Fc_Vegas_StateChange(RtaConnection *conn)
{
    assertNotNull(conn, "Got null connection\n");

    VegasConnectionState *fcConnState = rtaConnection_GetPrivateData(conn, FC_VEGAS);
    assertNotNull(fcConnState, "could not retrieve private data for FC_VEGAS on connid %u\n",
                  rtaConnection_GetConnectionId(conn));

    // should replace this with a hash table
    FcSessionHolder *holder;
    TAILQ_FOREACH(holder, &fcConnState->sessions_head, list)
    {
        if (vegasSession_GetConnectionId(holder->session) == rtaConnection_GetConnectionId(conn)) {
            vegasSession_StateChanged(holder->session);
        }
    }
}

// =======================================================================

/**
 * If the last component is a segment number, it is ignored
 */
static FcSessionHolder *
vegas_LookupSessionByName(VegasConnectionState *fc, CCNxName *name)
{
    uint64_t hash;
    FcSessionHolder *holder;
    int trim_segnum = 0;

    assertNotNull(name, "Name is null\n");
    if (name == NULL) {
        return NULL;
    }

    size_t segmentCount = ccnxName_GetSegmentCount(name);
    assertTrue(segmentCount > 1,
               "expected name with at least 2 components, but only got %zu, name = '%s'\n",
               segmentCount,
               ccnxName_ToString(name));


    if (segmentCount > 0) {
        CCNxNameSegment *segment = ccnxName_GetSegment(name, segmentCount - 1);
        if (ccnxNameSegment_GetType(segment) == CCNxNameLabelType_CHUNK) {
            trim_segnum = 1;
        }
    }

    hash = ccnxName_LeftMostHashCode(name, segmentCount - trim_segnum);

    if (DEBUG_OUTPUT) {
        printf("%s name %p hash %16" PRIX64 "\n", __func__, (void *) name, hash);
        ccnxName_Display(name, 0);
    }

    // should replace this with a hash table
    TAILQ_FOREACH(holder, &fc->sessions_head, list)
    {
        if (holder->basename_hash == hash) {
            return holder;
        }
    }

    return NULL;
}

/*
 * Precondition: only called for Content Objects.
 * If the last component is a segment number, it is ignored
 *
 * Match the name of the content object to an active flow control session,
 * or return NULL if not found.
 */
static FcSessionHolder *
vegas_LookupSession(VegasConnectionState *fc, TransportMessage *tm)
{
    assertTrue(transportMessage_IsContentObject(tm),
               "Transport message is not a ContentObject\n");

    CCNxTlvDictionary *contentObjectDictionary = transportMessage_GetDictionary(tm);
    CCNxName *name = ccnxContentObject_GetName(contentObjectDictionary);

    return vegas_LookupSessionByName(fc, name);
}

// =============================================

/*
 * Precondition: it's an interest
 */
static int
vegas_HandleInterest(RtaConnection *conn, TransportMessage *tm)
{
    assertTrue(transportMessage_IsInterest(tm), "Transport message is not an interest");

    VegasConnectionState *fc = rtaConnection_GetPrivateData(conn, FC_VEGAS);
    CCNxTlvDictionary *interestDictionary = transportMessage_GetDictionary(tm);

    // we do not modify or destroy this name
    CCNxName *original_name = ccnxInterest_GetName(interestDictionary);

    CCNxName *basename = ccnxName_Copy(original_name);

    if (DEBUG_OUTPUT) {
        printf("%s orig name %p basename %p\n", __func__, (void *) original_name, (void *) basename);
    }

    // can we decode the last component as a segment number?
    uint64_t segnum = 0;
    bool segnum_found = trafficTools_GetObjectSegmentFromName(basename, &segnum);
    if (segnum_found) {
        // it is a segment number
        ccnxName_Trim(basename, 1);
    }

    FcSessionHolder *holder = vegas_LookupSessionByName(fc, basename);

    if (holder == NULL) {
        // create a new session
        // This takes ownership of the basename
        uint64_t name_hash = ccnxName_HashCode(basename);
        holder = vegas_CreateSessionHolder(fc, conn, basename, name_hash);

        CCNxInterestInterface *interestImpl = ccnxInterestInterface_GetInterface(interestDictionary);

        uint32_t lifetime = ccnxInterest_GetLifetime(interestDictionary);

        PARCBuffer *keyIdRestriction = ccnxInterest_GetKeyIdRestriction(interestDictionary); // might be NULL

        holder->session = vegasSession_Create(fc, conn, basename, segnum,
                                              interestImpl, lifetime, keyIdRestriction);

        vegasSession_Start(holder->session);

        rtaConnection_SendStatus(conn,
                                 FC_VEGAS,
                                 RTA_UP,
                                 notifyStatusCode_FLOW_CONTROL_STARTED,
                                 original_name,
                                 NULL);
    } else {
        assertTrue(segnum_found, "Duplicate interest w/o segnum for existing session");

        if (segnum_found) {
            vegasSession_Seek(holder->session, segnum);
        }

        ccnxName_Release(&basename);
    }

    return 0;
}

static FcSessionHolder *
vegas_CreateSessionHolder(VegasConnectionState *fc, RtaConnection *conn, CCNxName *basename, uint64_t name_hash)
{
    FcSessionHolder *holder = parcMemory_AllocateAndClear(sizeof(FcSessionHolder));
    assertNotNull(holder, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(FcSessionHolder));
    holder->basename_hash = name_hash;
    holder->basename = basename;
    holder->session = NULL;

    TAILQ_INSERT_TAIL(&fc->sessions_head, holder, list);

    if (DEBUG_OUTPUT) {
        printf("%s created holder %p hash %016" PRIX64 "\n", __func__, (void *) holder, holder->basename_hash);
    }
    return holder;
}

/**
 * This is called by a session when it is done
 */
void
vegas_EndSession(VegasConnectionState *fc, VegasSession *session)
{
    FcSessionHolder *holder;

    // should replace this with a hash table
    TAILQ_FOREACH(holder, &fc->sessions_head, list)
    {
        if (holder->session == session) {
            TAILQ_REMOVE(&fc->sessions_head, holder, list);
            break;
        }
    }

    assertNotNull(holder, "invalid state, got null holder");

    rtaConnection_SendStatus(fc->parent_connection,
                             FC_VEGAS,
                             RTA_UP,
                             notifyStatusCode_FLOW_CONTROL_FINISHED,
                             holder->basename,
                             NULL);

    vegasSession_Destroy(&holder->session);
    parcMemory_Deallocate((void **) &holder);
}

static void
vegas_SendControlPlaneResponse(RtaConnection *conn, CCNxTlvDictionary *controlDictionary, PARCEventQueue *outputQueue)
{
    TransportMessage *tm = transportMessage_CreateFromDictionary(controlDictionary);

    transportMessage_SetInfo(tm, rtaConnection_Copy(conn), rtaConnection_FreeFunc);

    if (rtaComponent_PutMessage(outputQueue, tm)) {
        RtaComponentStats *stats = rtaConnection_GetStats(conn, FC_VEGAS);
        rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
    }
}

/**
 * @function vegas_HandleControl
 * @abstract Process CPI reqeusts
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return true if we consumed the message, false if it should go down the stack
 */
static bool
vegas_HandleControl(RtaConnection *conn, CCNxTlvDictionary *controlDictionary, PARCEventQueue *outputQueue)
{
    bool success = false;

    if (ccnxControlFacade_IsCPI(controlDictionary)) {
        PARCJSON *json = ccnxControlFacade_GetJson(controlDictionary);
        if (cpi_getCPIOperation2(json) == CPI_CANCEL_FLOW) {
            VegasConnectionState *fc = rtaConnection_GetPrivateData(conn, FC_VEGAS);
            CCNxName *name = cpiCancelFlow_GetFlowName(json);

            PARCJSON *reply = NULL;
            FcSessionHolder *holder = vegas_LookupSessionByName(fc, name);
            if (holder != NULL) {
                if (DEBUG_OUTPUT) {
                    char *string = ccnxName_ToString(name);
                    printf("%s Cancelling flow %s\n", __func__, string);
                    parcMemory_Deallocate((void **) &string);
                }

                TAILQ_REMOVE(&fc->sessions_head, holder, list);
                vegasSession_Destroy(&holder->session);
                parcMemory_Deallocate((void **) &holder);

                reply = cpiAcks_CreateAck(json);
            } else {
                if (DEBUG_OUTPUT) {
                    char *string = ccnxName_ToString(name);
                    printf("%s got request to cancel unknown flow %s\n", __func__, string);
                    parcMemory_Deallocate((void **) &string);
                }

                reply = cpiAcks_CreateNack(json);
            }
            CCNxTlvDictionary *response = ccnxControlFacade_CreateCPI(reply);
            vegas_SendControlPlaneResponse(conn, response, outputQueue);
            ccnxTlvDictionary_Release(&response);

            parcJSON_Release(&reply);
            ccnxName_Release(&name);

            // we consume it
            success = true;
        }
    }
    return success;
}
