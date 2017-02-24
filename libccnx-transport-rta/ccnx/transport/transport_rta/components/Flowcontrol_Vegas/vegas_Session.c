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
 * See additional comments in component_Vegas.c
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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/algol/parc_EventTimer.h>

#include <ccnx/common/ccnx_NameSegmentNumber.h>
#include <ccnx/common/ccnx_WireFormatMessage.h>

#include <ccnx/transport/common/transport_Message.h>
#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_Services.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>
#include <ccnx/transport/transport_rta/components/component_Flowcontrol.h>
#include "vegas_private.h"

#include <ccnx/transport/test_tools/traffic_tools.h>

#include <ccnx/common/internal/ccnx_InterestDefault.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>


#define USE_MIN_BASE_RTT 0


// initial congestion window of 2 interests
#define FC_INIT_CWND          2

// maximum cwnd (at 8KB/object, makes this 128 MB)
#define FC_MAX_CWND         16384

#define FC_MAX_SSTHRESH     FC_MAX_CWND

// initial RTT in msec (100 msec)
#define FC_INIT_RTT_MSEC    100

// initial RTO in msec
#define FC_INIT_RTO_MSEC    1000

#define FC_MSS 8704
#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

// ===========================================================
struct vegas_connection_state;

struct fc_window_entry {
    bool valid;
    ticks t;
    ticks t_first_request;
    segnum_t segnum;

    // set to true on the first interest request for
    // the segment, false on subsequent requests
    // Needed for Karn's algorithm on RTT sampling for RTO
    bool first_request;

    // Content Object read
    TransportMessage       *transport_msg;
};

struct vegas_session {
    RtaConnection     *parent_connection;
    RtaFramework      *parent_framework;
    VegasConnectionState *parent_fc;

    // next sampling time
    ticks next_rtt_sample;

    // minimum observed RTT
    int64_t base_RTT;               // absolute minimum observed
    int64_t min_RTT;                // minimum RTT in current sample
    int cnt_RTT;                    // number of RTTs seen in current sample
    int64_t sum_RTT;                // sum of RTTs
    int slow_start_threshold;

    // the currently observed RTT
    ticks current_rtt;

    // we do one detailed sample per RTT
    bool sample_in_progress;
    ticks sample_start;
    uint64_t sample_segnum;
    uint64_t sample_bytes_recevied;

    // Only adjust the cwnd every 2 RTTs.  This
    // indicates if we should adjust the RTT at the
    // end of this sampling period
    int do_fc_this_rtt;

    // circular buffer for segments
    // tail - head (mod FC_MAX_CWND) is how may outstanding interests
    // are in-flight.  If the cwnd has been reduced, it could be larger
    // than current_cwnd.
    uint64_t starting_segnum;       // segnum of the head
    int window_head;                // window index to read from
    int window_tail;                // window index to insert at

    uint32_t current_cwnd;
    ticks last_cwnd_adjust;

    uint64_t final_segnum;          // if we know the final block ID

    struct fc_window_entry window[FC_MAX_CWND];

    PARCEventTimer *tick_event;

    // we will generate Interests with the same version as was received to start the session.
    // Will also use the same lifetime settings as the original Interest.

    CCNxInterestInterface *interestInterface;
    uint32_t lifetime;
    PARCBuffer *keyIdRestriction;
    CCNxName *basename;
    uint64_t name_hash;

    uint64_t cnt_old_segments;
    uint64_t cnt_fast_reexpress;

    // These are for RTO calculation
    ticks SRTT;
    ticks RTTVAR;
    ticks RTO;
    ticks next_rto;        // when the next timer expires

    PARCLogLevel logLevel;
};

// Control parameters, measured in segments (tcp) or objects (ccn)
static int alpha = 2;
static int beta = 32;
static int _gamma = 1;

// ===========================================================



static void vegasSession_ExpressInterests(VegasSession *session);
static int  vegasSession_ExpressInterestForEntry(VegasSession *session, struct fc_window_entry *entry);

static void vegasSession_FastReexpress(VegasSession *session, struct fc_window_entry *ack_entry);
static void vegasSession_ForwardObjectsInOrder(VegasSession *session);

static int  vegasSession_GetSegnumFromObject(CCNxTlvDictionary *contentObjectDictionary, uint64_t *segnum);
static struct fc_window_entry *
vegasSession_GetWindowEntry(VegasSession *session, TransportMessage *tm, uint64_t segnum);

static void vegasSession_ReleaseWindowEntry(struct fc_window_entry *entry);
static void vegasSession_RunAlgorithmOnReceive(VegasSession *session, struct fc_window_entry *entry);

static void vegasSession_SetTimer(VegasSession *session, ticks tick_delay);
static void vegasSession_SlowReexpress(VegasSession *session);

// =======================================================================


static struct fc_window_entry *
vegasSession_GetWindowEntry(VegasSession *session, TransportMessage *tm, uint64_t segnum)
{
    int offset;
    struct fc_window_entry *entry;

    offset = ((segnum - session->starting_segnum) + session->window_head) % FC_MAX_CWND;
    entry = &session->window[offset];

    assertTrue(entry->valid, "Requesting window entry for invalid entry %p", (void *) entry);
    assertTrue(segnum == entry->segnum, "Expected seqnum not equal to window entry, expected %" PRIu64 ", got %" PRIu64, segnum, entry->segnum);

    if (entry->transport_msg != NULL) {
        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info)) {
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info, __func__,
                          "session %p duplicate segment %" PRIu64 "", (void *) session, entry->segnum);
        }

        transportMessage_Destroy(&entry->transport_msg);
    }

    // store the content object
    entry->transport_msg = tm;

    return entry;
}

static int
vegasSession_GetSegnumFromObject(CCNxTlvDictionary *contentObjectDictionary, uint64_t *segnum)
{
    CCNxName *name = ccnxContentObject_GetName(contentObjectDictionary);
    assertNotNull(name, "Content Object has null name")
    {
        ccnxTlvDictionary_Display(contentObjectDictionary, 0);
    }

    bool success = trafficTools_GetObjectSegmentFromName(name, segnum);

    if (success) {
        return 0;
    }
    return -1;
}

static void
vegasSession_ReduceCongestionWindow(VegasSession *session)
{
    if (session->current_cwnd <= session->slow_start_threshold) {
        // 3/4 it
        session->current_cwnd = session->current_cwnd / 2 + session->current_cwnd / 4;
    } else {
        // in linear mode
        session->current_cwnd--;
    }

    if (session->current_cwnd < 2) {
        session->current_cwnd = 2;
    }

    session->last_cwnd_adjust = rtaFramework_GetTicks(session->parent_framework);
}

static void
vegasSession_RunAlgorithmOnReceive(VegasSession *session, struct fc_window_entry *entry)
{
    ticks now;
    int64_t fc_rtt;

    now = rtaFramework_GetTicks(session->parent_framework);

    // perform statistics updates.

    // If the codec did not include the raw message, we cannot increment the bytes counter
    PARCBuffer *wireFormat = ccnxWireFormatMessage_GetWireFormatBuffer(transportMessage_GetDictionary(entry->transport_msg));

    if (wireFormat) {
        session->sample_bytes_recevied += parcBuffer_Remaining(wireFormat);
    }


    /* add +1 so never have 0 RTT */
    fc_rtt = ((int64_t) now - (int64_t) entry->t_first_request) + 1;
    if (fc_rtt <= 0) {
        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Error)) {
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Error, __func__,
                          "session %p sock %3d : recv segment %" PRIu64 " with negative RTT, t = %" PRIu64 "",
                          (void *) session,
                          rtaConnection_GetConnectionId(session->parent_connection),
                          entry->segnum,
                          entry->t);
        }

        return;
    }

    /* record the absolute minimum RTT ever seen */
    if (fc_rtt < session->base_RTT) {
        session->base_RTT = fc_rtt;
    }

    /* find the minimum RTT for the sample period */
    session->min_RTT = min(session->min_RTT, fc_rtt);
    session->cnt_RTT++;
    session->sum_RTT += fc_rtt;

    // calculate RTO as per RFC6298
    if (entry->first_request) {
        if (session->SRTT == 0) {
            // this is the first one, so do 2.2
            session->SRTT = fc_rtt;
            session->RTTVAR = fc_rtt >> 1;
            session->RTO = session->SRTT +
                           max(rtaFramework_UsecToTicks(1000000), 4 * session->RTTVAR);
        } else {
            // RTTVAR <- (1 - beta) * RTTVAR + beta * |SRTT - R'|
            // using beta = 1/4, so we want 3/4 * RTTVAR
            int64_t abs = ((int64_t) session->SRTT - (int64_t) fc_rtt);

            if (abs < 0) {
                abs = -1 * abs;
            }

            session->RTTVAR = ((session->RTTVAR >> 1) + (session->RTTVAR >> 2)) + (abs >> 2);

            // SRTT <- (1 - alpha) * SRTT + alpha * R'
            // using alpha = 1/8 and (1-alpha) = 1/2 + 1/4 + 1/8 = 7/8
            session->SRTT = (session->SRTT >> 1) + (session->SRTT >> 2) + (session->SRTT >> 3) + (abs >> 3);

            session->RTO = session->SRTT +
                           max(rtaFramework_UsecToTicks(1000000), 4 * session->RTTVAR);
        }
    }

    // we received a packet :)  yay.
    // we get to extend the RTO expiry
    session->next_rto = now + session->RTO;
}

/*
 * called inside workq_mutex lock.
 * After we deliver each segment, we increment session->starting_segnum.  After we deliver the
 * the terminal segement of a stream, session->starting_segnum will be 1 past the final block id.
 */
static void
vegasSession_ForwardObjectsInOrder(VegasSession *session)
{
    while (session->window_head != session->window_tail) {
        struct fc_window_entry *entry = &session->window[ session->window_head ];

        // sanity checks
        assertTrue(entry->valid, "Window entry %p for window_head index %u", (void *) entry, session->window_head);
        assertTrue(entry->segnum == session->starting_segnum,
                   "Expected seqnum not equal to window entry, expected %" PRIu64 ", got %" PRIu64,
                   session->starting_segnum,
                   entry->segnum);

        if (entry->transport_msg != NULL) {
            PARCEventQueue *out = rtaComponent_GetOutputQueue(session->parent_connection, FC_VEGAS, RTA_UP);
            RtaComponentStats *stats = rtaConnection_GetStats(session->parent_connection, FC_VEGAS);

            if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
                rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                              "session %p fd %d forward segment %" PRIu64 " up stack",
                              (void *) session,
                              rtaConnection_GetConnectionId(session->parent_connection),
                              entry->segnum);
            }

            if (rtaComponent_PutMessage(out, entry->transport_msg)) {
                // if we successfully put the message up the stack, null
                // the entry so the transport message will not be destroyed
                // when this window entry is released.
                entry->transport_msg = NULL;
                rtaComponentStats_Increment(stats, STATS_UPCALL_OUT);
            }

            vegasSession_ReleaseWindowEntry(entry);
            session->starting_segnum++;
            session->window_head = (session->window_head + 1) % FC_MAX_CWND;
        } else {
            if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
                rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                              "session %p fd %d no message segment %" PRIu64 ", no more in order messages",
                              rtaConnection_GetConnectionId(session->parent_connection),
                              entry->segnum);
            }

            return;
        }
    }
}

static int
fc_ssthresh(VegasSession *session)
{
    return min(session->slow_start_threshold, session->current_cwnd - 1);
}

/**
 * Slow-start increase, double the cwnd
 */
static void
fc_slow_start(VegasSession *session)
{
    session->last_cwnd_adjust = rtaFramework_GetTicks(session->parent_framework);
    session->current_cwnd = session->current_cwnd << 1;
}

static
int
fc_in_cwnd_reduction(VegasSession *session)
{
    return 0;
}

/*
 * Similar to the tcp_current_ssthresh.  If cwnd > ssthresh, then
 * increase ssthres to 1/2 to cwnd, except if we're in a cwnd reduction
 * period.
 */
static inline uint32_t
fc_current_ssthresh(VegasSession *session)
{
    if (fc_in_cwnd_reduction(session)) {
        return session->slow_start_threshold;
    } else {
        return max(session->slow_start_threshold,
                   ((session->current_cwnd >> 1) +
                    (session->current_cwnd >> 2)));
    }
}

static void
vegasSession_CongestionAvoidanceDebug(VegasSession *session, ticks now)
{
    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
        ticks diff = 0;

        if (session->min_RTT != INT_MAX) {
            diff = session->current_cwnd * (session->min_RTT - session->base_RTT) / session->base_RTT;
        }

        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                      "session %p do_cong %d currentRTT %5" PRIu64 " cntRTT %3d minRTT %5" PRId64 " baseRTT %5" PRId64 " cwnd %3d next %8" PRIu64 " SRTT %" PRIu64 " RTO %" PRIu64 " oldsegs %" PRIu64 " fast %" PRIu64 " diff %" PRIu64 " allocs %u",
                      (void *) session,
                      session->do_fc_this_rtt,
                      session->current_rtt,
                      session->cnt_RTT,
                      session->min_RTT == INT_MAX ? 0 : session->min_RTT,
                      session->base_RTT == INT_MAX ? 0 : session->base_RTT,
                      session->current_cwnd,
                      session->next_rtt_sample,
                      session->SRTT,
                      session->RTO,
                      session->cnt_old_segments,
                      session->cnt_fast_reexpress,
                      diff,
                      parcMemory_Outstanding());
    }
}

static void
vegasSession_LossBasedAvoidance(VegasSession *session)
{
    session->current_rtt = session->current_rtt * 2;
    if (session->current_rtt > 4000) {
        session->current_rtt = 4000;
    }
}

/**
 * This is the Vegas algorithm
 */
static void
vegasSession_TimeBasedAvoidance(VegasSession *session)
{
    ticks rtt, diff;
    uint64_t target_cwnd;

    rtt = session->min_RTT;

    /*
     * calculate the target cwnd in segments
     */
    target_cwnd = session->current_cwnd * session->base_RTT / rtt;

    diff = session->current_cwnd * (rtt - session->base_RTT) / session->base_RTT;

    if ((diff > _gamma && session->current_cwnd <= session->slow_start_threshold)) {
        /* If we're in slow start and going too fast, slow down */
        session->current_cwnd = min(session->current_cwnd, (uint32_t) target_cwnd + 1);
        session->slow_start_threshold = fc_ssthresh(session);
        session->last_cwnd_adjust = rtaFramework_GetTicks(session->parent_framework);
    } else if (session->current_cwnd <= session->slow_start_threshold) {
        /* Slow start */
        fc_slow_start(session);
    } else {
        /* Congestion avoidance. */

        //				if (diff > beta || session->cnt_old_segments ) {
        if (diff > beta) {
            /* The old window was too fast, so
             * we slow down.
             */

            session->current_cwnd--;
            session->slow_start_threshold = fc_ssthresh(session);
            session->last_cwnd_adjust = rtaFramework_GetTicks(session->parent_framework);
        } else if (diff < alpha) {
            /* room to grow */
            session->current_cwnd++;
            session->last_cwnd_adjust = rtaFramework_GetTicks(session->parent_framework);
        } else {
            /* middle ground, no changes necessary */
        }
    }

    if (session->current_cwnd < 2) {
        session->current_cwnd = 2;
    } else if (session->current_cwnd > FC_MAX_CWND) {
        session->current_cwnd = FC_MAX_CWND;
    }

    session->slow_start_threshold = fc_current_ssthresh(session);
}

static void
vegasSession_CongestionAvoidance(VegasSession *session)
{
    ticks now = rtaFramework_GetTicks(session->parent_framework);

    vegasSession_CongestionAvoidanceDebug(session, now);

    if (session->do_fc_this_rtt) {
        if (session->cnt_RTT <= 2) {
            vegasSession_LossBasedAvoidance(session);
        } else {
            vegasSession_TimeBasedAvoidance(session);
        }

        session->do_fc_this_rtt = 0;
    } else {
        session->do_fc_this_rtt = 1;
    }

    // Now finish up the statistics and setup for next RTT interval

    session->next_rtt_sample = now + session->current_rtt;

    // low-pass filter the base_RTT from the min_RTT
    // base_RTT = 15/16 base_RTT + 1/16 min_RTT = (240 * base_RTT + 16 * min_RTT ) / 256

    if (!USE_MIN_BASE_RTT && (session->cnt_RTT > 0)) {
        session->base_RTT = (240 * session->base_RTT + 16 * session->min_RTT) >> 8;
        if (session->base_RTT == 0) {
            session->base_RTT = 1;
        }
    }

    // Smooth the RTT for (3 * current + 1 * minimum) / 4

    if (session->cnt_RTT > 0) {
        session->current_rtt = (12 * session->current_rtt + 4 * session->min_RTT) >> 4;
    }

    session->current_rtt = max(session->current_rtt, FC_INIT_RTT_MSEC);

    // reset stats
    session->sample_bytes_recevied = 0;
    session->min_RTT = INT_MAX;
    session->cnt_RTT = 0;
    session->cnt_old_segments = 0;
    session->cnt_fast_reexpress = 0;
    session->sum_RTT = 0;

    vegasSession_CongestionAvoidanceDebug(session, now);
}

/**
 * Slow (course grain) retransmission due to RTO expiry.
 * Re-express the first segment of the window.
 */
static
void
vegasSession_SlowReexpress(VegasSession *session)
{
    struct fc_window_entry *entry = &session->window[ session->window_head ];

    assertTrue(entry->valid, "entry %p segnum %" PRIu64 " invalid state, in window but not valid",
               (void *) entry, entry->segnum);

    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info)) {
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info, __func__,
                      "Session %p conn %p RTO re-expression for segnum %" PRIu64 "",
                      (void *) session, (void *) session->parent_connection, entry->segnum);
    }

    entry->first_request = false;
    vegasSession_ExpressInterestForEntry(session, entry);
}

/**
 * Do fast retransmissions based on SRTT smoothed estimate.
 * ack_entry is the entry for a content object we just received.  Look earlier segments
 * and if they were asked for more than SRTT ago, ask again.
 */
static void
vegasSession_FastReexpress(VegasSession *session, struct fc_window_entry *ack_entry)
{
    ticks now = rtaFramework_GetTicks(session->parent_framework);
    int64_t delta;
    uint64_t segnum;
    uint64_t top_segnum;

    // This method is called after forward_in_order, so it's possible that
    // ack_entry is no longer valid, meaning we've moved the window past it.
    // In that case, we're done.
    if (ack_entry->valid == false) {
        return;
    }

    // we don't retransmit beyond the current cwnd.  ack_entry might be outside
    // the cwnd.

    top_segnum = min(ack_entry->segnum, session->starting_segnum + session->current_cwnd);

    for (segnum = session->starting_segnum; segnum < top_segnum; segnum++) {
        int index = (session->window_head + (segnum - session->starting_segnum)) % FC_MAX_CWND;
        delta = (int64_t) now - ((int64_t) session->window[index].t + (int64_t) session->SRTT);

        // allow up to -1 slack, because the RunAlgorithm adds +1 to fc_rtt.
        if (delta >= -1) {
            // we have past the SRTT timeout

            // if we last re-transmitted him since the last cwnd adjustment, adjust again
            if ((int64_t) session->window[index].t - (int64_t) session->last_cwnd_adjust >= 0) {
                vegasSession_ReduceCongestionWindow(session);
            }

            if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info)) {
                rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info, __func__,
                              "session %p conn %p RTO re-expression for segnum %" PRIu64 "",
                              (void *) session, (void *) session->parent_connection, session->window[index].segnum);
            }

            session->window[index].first_request = false;
            session->cnt_fast_reexpress++;
            vegasSession_ExpressInterestForEntry(session, &session->window[index]);
        }
    }
}

/**
 * Generates an Interest message for the window entry.
 *
 * No side effects, apart from putting on Interest on the down queue.
 * If the down direction is blocked, this function will not put an interest in the down queue.  It will
 * look like a lost interest to the flow controller, which should cause the flow controller to slow down.
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
static int
vegasSession_ExpressInterestForEntry(VegasSession *session, struct fc_window_entry *entry)
{
    if (!rtaConnection_BlockedDown(session->parent_connection)) {
        ticks now = rtaFramework_GetTicks(session->parent_framework);
        PARCEventQueue    *q_out;
        TransportMessage  *tm_out;
        CCNxName          *chunk_name;

        entry->t = now;

        chunk_name = ccnxName_Copy(session->basename);

        CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, entry->segnum);
        ccnxName_Append(chunk_name, segment);
        ccnxNameSegment_Release(&segment);

        assertNotNull(session->interestInterface, "Got a NULL interestInterface. Should not happen.");

        CCNxTlvDictionary *interestDictionary =
            session->interestInterface->create(chunk_name,
                                               session->lifetime,
                                               NULL,         // ppkid
                                               NULL,         // content object hash
                                               CCNxInterestDefault_HopLimit);

        if (session->keyIdRestriction != NULL) {
            session->interestInterface->setKeyIdRestriction(interestDictionary, session->keyIdRestriction);
        }

        tm_out = transportMessage_CreateFromDictionary(interestDictionary);
        transportMessage_SetInfo(tm_out, rtaConnection_Copy(session->parent_connection), rtaConnection_FreeFunc);

        q_out = rtaComponent_GetOutputQueue(session->parent_connection, FC_VEGAS, RTA_DOWN);

        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
            char *string = ccnxName_ToString(chunk_name);
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                          "session %p entry %p segname %p segnum %" PRIu64 " %s sent",
                          (void *) session,
                          (void *) entry,
                          (void *) chunk_name,
                          entry->segnum,
                          string);
            parcMemory_Deallocate((void **) &string);
        }

        ccnxTlvDictionary_Release(&interestDictionary);
        ccnxName_Release(&chunk_name);

        if (rtaComponent_PutMessage(q_out, tm_out)) {
            rtaComponentStats_Increment(rtaConnection_GetStats(session->parent_connection, FC_VEGAS),
                                        STATS_DOWNCALL_OUT);
        }
    } else {
        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info)) {
            CCNxName *segment_name = ccnxName_Copy(session->basename);
            ccnxName_Append(segment_name, ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, entry->segnum));
            char *string = ccnxName_ToString(segment_name);
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info, __func__,
                          "session %p entry %p segname %p segnum %" PRIu64 " %s SUPPRESSED BLOCKED DOWN QUEUE",
                          (void *) session,
                          (void *) entry,
                          (void *) segment_name,
                          entry->segnum,
                          string);
            parcMemory_Deallocate((void **) &string);
            ccnxName_Release(&segment_name);
        }
    }

    return 0;
}

/*
 * Express interests out to the max allowed by the cwnd.  This function will operate
 * even if the down queue is blocked.  Those interests will be treated as lost, which will cause
 * the flow controller to slow down.
 */
static void
vegasSession_ExpressInterests(VegasSession *session)
{
    ticks now = rtaFramework_GetTicks(session->parent_framework);

    // how many interests are currently outstanding?
    int wsize = session->window_tail - session->window_head;
    if (wsize < 0) {
        wsize += FC_MAX_CWND;
    }

    // if we know the FBID, don't ask for anything beyond that
    while (wsize < session->current_cwnd && (wsize + session->starting_segnum <= session->final_segnum)) {
        // expreess them
        struct fc_window_entry *entry = &session->window[session->window_tail];

        assertFalse(entry->valid,
                    "Window entry %d marked as valid, but its outside the cwind!",
                    session->window_tail);

        session->window_tail = (session->window_tail + 1) % FC_MAX_CWND;

        memset(entry, 0, sizeof(struct fc_window_entry));

        entry->valid = true;
        entry->segnum = session->starting_segnum + wsize;
        entry->first_request = true;
        entry->t_first_request = now;

        if (session->sample_in_progress == 0) {
            // make this interest the sample for the RTT
            session->sample_in_progress = true;
            session->sample_segnum = entry->segnum;
            session->sample_start = now;
            session->sample_bytes_recevied = 0;
        }

        vegasSession_ExpressInterestForEntry(session, entry);

        wsize++;
    }
}

/*
 * This is dispatched from the event loop, so its a loosely accurate time
 */
static void
vegasSession_TimerCallback(int fd, PARCEventType what, void *user_data)
{
    VegasSession *session = (VegasSession *) user_data;
    int64_t delta;
    ticks now;

    assertTrue(what & PARCEventType_Timeout, "%s got unknown signal %d", __func__, what);

    now = rtaFramework_GetTicks(session->parent_framework);
    delta = ((int64_t) now - (int64_t) session->next_rtt_sample);

    if (delta >= 0) {
        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                          "Session %p processing timer, delta %" PRId64,
                          (void *) session, delta);
        }

        // This entry is ready for processing
        vegasSession_CongestionAvoidance(session);

        // set the next timer
        vegasSession_SetTimer(session, session->current_rtt);
    } else {
        vegasSession_SetTimer(session, -1 * delta);
    }

    // check for retransmission
    delta = ((int64_t) now - (int64_t) session->next_rto);
    if (delta >= 0) {
        // Do this once per RTO
        vegasSession_SlowReexpress(session);

        // we're now in a doubling regeme.  Reset the
        // moving average and double the RTO.
        session->SRTT = 0;
        session->RTTVAR = 0;
        session->RTO = session->RTO * 2;
        session->next_rto = now + session->RTO;
    }
}

/**
 * precondition: the entry is valid
 */
static void
vegasSession_ReleaseWindowEntry(struct fc_window_entry *entry)
{
    assertTrue(entry->valid, "Called on invalid window entry");
    if (!entry->valid) {
        return;
    }

    if (entry->transport_msg != NULL) {
        transportMessage_Destroy(&entry->transport_msg);
    }
    entry->valid = false;
}

static void
vegasSession_SetTimer(VegasSession *session, ticks tick_delay)
{
    struct timeval timeout;
    uint64_t usec = rtaFramework_TicksToUsec(tick_delay);
    const unsigned usec_per_sec = 1000000;

    timeout.tv_sec = usec / usec_per_sec;
    timeout.tv_usec = (int) (usec - timeout.tv_sec * usec_per_sec);

    // this replaces any prior events
    parcEventTimer_Start(session->tick_event, &timeout);

    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                      "session %p tick_delay %" PRIu64 " timeout %.6f",
                      (void *) session,
                      tick_delay,
                      timeout.tv_sec + 1E-6 * timeout.tv_usec);
    }
}

// =============================================
// Private API

/**
 * Unsets the final segment number indicating we do not know the value
 *
 * Sets the final segment number to the maximum possible value, which effectively
 * lets us run off to infinity.
 *
 * @param [in] session An allocated vegas session
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
_vegasSession_UnsetFinalSegnum(VegasSession *session)
{
    session->final_segnum = ULLONG_MAX;
}

VegasSession *
vegasSession_Create(VegasConnectionState *fc, RtaConnection *conn, CCNxName *basename, segnum_t begin,
                    CCNxInterestInterface *interestInterface, uint32_t lifetime, PARCBuffer *keyIdRestriction)
{
    assertNotNull(conn, "Called with null connection");
    assertNotNull(basename,
                  "conn %p connid %u called with null basename",
                  (void *) conn,
                  rtaConnection_GetConnectionId(conn));

    if (conn == NULL || basename == NULL) {
        return NULL;
    }

    VegasSession *session = parcMemory_AllocateAndClear(sizeof(VegasSession));
    assertNotNull(session, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(VegasSession));
    session->parent_connection = conn;
    session->parent_framework = rtaProtocolStack_GetFramework(rtaConnection_GetStack(conn));
    session->interestInterface = interestInterface;
    session->lifetime = lifetime;
    session->basename = basename;
    if (keyIdRestriction != NULL) {
        session->keyIdRestriction = parcBuffer_Acquire(keyIdRestriction);
    }
    session->parent_fc = fc;

    session->tick_event = parcEventTimer_Create(rtaFramework_GetEventScheduler(session->parent_framework), 0, vegasSession_TimerCallback, (void *) session);

    session->starting_segnum = 0;
    session->current_cwnd = FC_INIT_CWND;
    session->min_RTT = INT_MAX;
    session->base_RTT = INT_MAX;
    session->do_fc_this_rtt = 0;
    session->current_rtt = rtaFramework_UsecToTicks(FC_INIT_RTT_MSEC * 1000);
    session->slow_start_threshold = FC_MAX_SSTHRESH;

    session->SRTT = 0;
    session->RTTVAR = 0;
    session->RTO = rtaFramework_UsecToTicks(FC_INIT_RTO_MSEC * 1000);
    session->next_rto = ULLONG_MAX;
    session->cnt_old_segments = 0;
    session->cnt_fast_reexpress = 0;

    _vegasSession_UnsetFinalSegnum(session);

    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Notice)) {
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Notice, __func__,
                      "session %p initialized connid %u ",
                      (void *) session,
                      rtaConnection_GetConnectionId(conn));
    }
    return session;
}

static void
vegasSession_Close(VegasSession *session)
{
    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Notice)) {
        char *p = ccnxName_ToString(session->basename);
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Notice, __func__,
                      "session %p close starting segnum %" PRIu64 " final chunk ID %" PRIu64 " for name %s",
                      (void *) session, session->starting_segnum, session->final_segnum, p);
        parcMemory_Deallocate((void **) &p);
    }

    ccnxName_Release(&session->basename);

    while (session->window_head != session->window_tail) {
        struct fc_window_entry *entry = &session->window[ session->window_head ];

        // sanity checks
        assertTrue(entry->valid, "connid %u session %p entry %d in window but not valid",
                   rtaConnection_GetConnectionId(session->parent_connection),
                   (void *) session,
                   session->window_head);

        if (entry->valid) {
            if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
                char *p = ccnxName_ToString(session->basename);
                rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                              "session %p releasing window entry %d", (void *) session, session->window_head);
                parcMemory_Deallocate((void **) &p);
            }

            vegasSession_ReleaseWindowEntry(entry);
        }

        session->window_head = (session->window_head + 1) % FC_MAX_CWND;
    }
}

void
vegasSession_Destroy(VegasSession **sessionPtr)
{
    VegasSession *session;

    assertNotNull(sessionPtr, "Called with null double pointer");
    session = *sessionPtr;

    if (session->keyIdRestriction != NULL) {
        parcBuffer_Release(&session->keyIdRestriction);
    }

    vegasSession_Close(session);

    parcEventTimer_Destroy(&(session->tick_event));
    parcMemory_Deallocate((void **) &session);
    sessionPtr = NULL;
}

int
vegasSession_Start(VegasSession *session)
{
    ticks now = rtaFramework_GetTicks(session->parent_framework);

    // express the initial interests
    vegasSession_ExpressInterests(session);

    session->next_rtt_sample = now - 1;
    session->next_rto = now + session->RTO;

    // put it on the work queue for procesing

    vegasSession_SetTimer(session, session->current_rtt);

    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info)) {
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info, __func__,
                      "Session %p start", (void *) session);
    }

    return 0;
}

int
vegasSession_Pause(VegasSession *session)
{
    trapNotImplemented("vegasSession_Pause");
}

int
vegasSession_Resume(VegasSession *session)
{
    trapNotImplemented("vegasSession_Resume");
}

int
vegasSession_Seek(VegasSession *session, segnum_t absolutePosition)
{
    trapNotImplemented("vegasSession_See)");
}

/**
 * Retrieves the final block ID from the content object
 *
 * Retreives the final block ID from the object, if it exists, and returns it in
 * an output parameter.  Returns true if found and returned, false otherwise.
 *
 * @param [in] obj The Content Object to get the FBID form
 * @param [out] output Pointer to the seqnum ouptut
 *
 * @return true If the content object contained a FBID and the output set
 * @return false If there is no FBID in the content object
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
vegasSession_GetFinalBlockIdFromContentObject(CCNxTlvDictionary *obj, uint64_t *output)
{
    bool result = false;
    if (ccnxContentObject_HasFinalChunkNumber(obj)) {
        *output = ccnxContentObject_GetFinalChunkNumber(obj);
        result = true;
    }
    return result;
}

/**
 * Sets the final block id in the session based on the signed info
 *
 * If the final block id exists in the signed info, set the session's FBID.
 *
 * Rules on FinalChunkNumber:
 *
 * 1) The “final chunk” of a stream is identified by a content object having a FinalChunkNumber
 *    set in its metadata that equals the chunk number in its name.
 *
 * 2) An application may set the FinalChunkNumber early to let a receiver know when the end is coming.  These early advisories are not binding.
 *
 * 3) If the application has ever set the FinalChunkNumber it may not decrease it.  If the actual end happens before a previous advisory,
 *    the application must publish no-payload content objects such that Rule #1 is satisfied
 *
 *
 * @param [in,out] session The Vegas session
 * @param [in] obj The signed content object to get the FBID from
 * @param [in] nameChunkNumber is the chunk number in the name
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
vegasSession_SetFinalBlockId(VegasSession *session, CCNxTlvDictionary *contentObjectDictionary, uint64_t nameChunkNumber)
{
    // Get the FinalChunkNumber out of the metadata and update our notion of it
    uint64_t finalChunkNumber;
    if (vegasSession_GetFinalBlockIdFromContentObject(contentObjectDictionary, &finalChunkNumber)) {
        session->final_segnum = finalChunkNumber;

        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info)) {
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info, __func__,
                          "Session %p finalChunkNumber %" PRIu64, (void *) session, session->final_segnum);
        }
    } else {
        // There is no final chunk number in the metadata.  If the nameChunkNumber == session->final_seqnum, then
        // our idea of the final_seqnum is wrong and we should unset it as the producer did not actually close
        // the stream when they said they would

        if (session->final_segnum == nameChunkNumber) {
            if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Warning)) {
                rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Warning, __func__,
                              "Session %p finalChunkNumber %" PRIu64 " not set in final chunk, resetting",
                              (void *) session, session->final_segnum);
            }

            _vegasSession_UnsetFinalSegnum(session);
        }
    }
}

/**
 * We received a duplicate segment from before the start of the current congestion window
 *
 *
 * If we receive a segment from before the start of the current congestion window, then it
 * must be a duplicate (we don't have skip forward implemented).  Reduce the congestion window size.
 * We only reduce the window once per RTT interval no matter how many early duplicates we get.
 *
 * @param [in,out] session The Vegas session to reduce the window of.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
vegasSession_ReceivedBeforeWindowStart(VegasSession *session, uint64_t segnum)
{
    // once per cwnd, reduce the window on out-of-order
    if (session->cnt_old_segments == 0) {
        vegasSession_ReduceCongestionWindow(session);
    }

    session->cnt_old_segments++;

    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                      "Session %p connid %3u : recv old segment %" PRIu64 ", starting is %" PRIu64 ", cnt %" PRIu64 "",
                      (void *) session,
                      rtaConnection_GetConnectionId(session->parent_connection),
                      segnum,
                      session->starting_segnum,
                      session->cnt_old_segments);
    }
}

static void
vegasSession_SendMoreInterests(VegasSession *session, struct fc_window_entry *entry)
{
    // This will check if there's any earlier segments whose
    // RTT has expired and will re-ask for them.  This is the
    // out-of-order fast retransmit.
    vegasSession_FastReexpress(session, entry);

    // have we finished?
    if (session->starting_segnum < session->final_segnum) {
        // express more interests if we have the window for it
        vegasSession_ExpressInterests(session);
    } else
    if (session->starting_segnum > session->final_segnum) {
        // if starting_segment > final_segnum it means that we have delivered the last
        // segment up the stack.

        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info)) {
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Info, __func__,
                          "Session %p connid %u starting_segnum %" PRIu64 ", final_segnum %" PRIu64 ", FINAL SEGMENT DELIVERED, CLOSING",
                          (void *) session,
                          rtaConnection_GetConnectionId(session->parent_connection),
                          session->starting_segnum,
                          session->final_segnum);
        }

        parcEventTimer_Stop(session->tick_event);
        vegas_EndSession(session->parent_fc, session);
    }
    // else session->starting_segnum == session->final_segnum, we're not done yet.
}

static CCNxName *
vegasSession_GetNameFromTransportMessage(TransportMessage *tm)
{
    CCNxName *name = NULL;
    CCNxTlvDictionary *dictionary = transportMessage_GetDictionary(tm);
    switch (ccnxTlvDictionary_GetSchemaVersion(dictionary)) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            name = ccnxTlvDictionary_GetName(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
            break;

        default:
            break;
    }
    return name;
}



int
vegasSession_ReceiveContentObject(VegasSession *session, TransportMessage *tm)
{
    assertTrue(transportMessage_IsContentObject(tm),
               "Transport message is not a content object");

    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
        CCNxName *name = vegasSession_GetNameFromTransportMessage(tm);
        char *nameString = NULL;
        if (name) {
            nameString = ccnxName_ToString(name);
        }
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                      "Session %p connid %3u receive tm %p: %s",
                      (void *) session,
                      rtaConnection_GetConnectionId(session->parent_connection),
                      (void *) tm,
                      nameString);
        if (nameString) {
            parcMemory_Deallocate((void **) &nameString);
        }
    }

    CCNxTlvDictionary *contentObjectDictionary = transportMessage_GetDictionary(tm);

    // get segment number
    uint64_t segnum;
    int res = vegasSession_GetSegnumFromObject(contentObjectDictionary, &segnum);
    if (res != 0) {
        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Warning)) {
            CCNxName *name = vegasSession_GetNameFromTransportMessage(tm);
            char *nameString = NULL;
            if (name) {
                nameString = ccnxName_ToString(name);
            }
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Warning, __func__,
                          "Session %p connid %3u receive tm %p has no segment number: %s",
                          (void *) session,
                          rtaConnection_GetConnectionId(session->parent_connection),
                          (void *) tm,
                          nameString);
            if (nameString) {
                parcMemory_Deallocate((void **) &nameString);
            }
        }

        // couldn't figure it out
        transportMessage_Destroy(&tm);
        return -1;
    }

    // drop out of order
    if (segnum < session->starting_segnum) {
        vegasSession_ReceivedBeforeWindowStart(session, segnum);

        if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
            rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                          "Session %p connid %3u : tm %p received segnum %" PRIu64 " before current head %" PRIu64 "",
                          (void *) session,
                          __func__,
                          rtaConnection_GetConnectionId(session->parent_connection),
                          (void *) tm,
                          segnum,
                          session->starting_segnum);
        }

        transportMessage_Destroy(&tm);
        return -1;
    }

    // Update our idea of the final chunk number.  This must be done
    // before running the algorithm because session->final_segnum is used
    // to decide if we're done.
    vegasSession_SetFinalBlockId(session, contentObjectDictionary, segnum);


    // now run the algorithm on the received object

    struct fc_window_entry *entry = vegasSession_GetWindowEntry(session, tm, segnum);

    if (rtaLogger_IsLoggable(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug)) {
        CCNxName *name = vegasSession_GetNameFromTransportMessage(tm);
        char *nameString = NULL;
        if (name) {
            nameString = ccnxName_ToString(name);
        }
        rtaLogger_Log(rtaFramework_GetLogger(session->parent_framework), RtaLoggerFacility_Flowcontrol, PARCLogLevel_Debug, __func__,
                      "Session %p connid %3u receive tm %p segment %" PRIu64 " receive: %s",
                      (void *) session,
                      rtaConnection_GetConnectionId(session->parent_connection),
                      (void *) tm,
                      segnum,
                      nameString);
        if (nameString) {
            parcMemory_Deallocate((void **) &nameString);
        }
    }

    vegasSession_RunAlgorithmOnReceive(session, entry);

    // forward in-order objects to the user fc
    if (!rtaConnection_BlockedUp(session->parent_connection)) {
        vegasSession_ForwardObjectsInOrder(session);
    }

    vegasSession_SendMoreInterests(session, entry);

    return 0;
}

unsigned
vegasSession_GetConnectionId(VegasSession *session)
{
    assertNotNull(session, "Parameter session must be non-null");
    return rtaConnection_GetConnectionId(session->parent_connection);
}

void
vegasSession_StateChanged(VegasSession *session)
{
    if (rtaConnection_BlockedUp(session->parent_connection)) {
        // if we're blocked in the up direction, don't do anything.  We make this
        // check every time we're about ti send stuff up the stack in vegasSession_ReceiveContentObject().
    } else {
        // unblocked, forward packets
        vegasSession_ForwardObjectsInOrder(session);
    }

    if (rtaConnection_BlockedDown(session->parent_connection)) {
        // stop generating interests
    } else {
        // restart interests
    }
}
