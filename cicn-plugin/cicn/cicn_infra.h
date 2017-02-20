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
/*
 * plugin infrastructure: global data structure, common definitions,
 * statistics, etc
 */
#ifndef _CICN_INFRA_H_
#define _CICN_INFRA_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

typedef int (*test_cicn_api_handler_fn) (test_cicn_api_op_t *
					 test_cicn_api_op);

// cicn plugin global state: see also
// - icnfwd_runtime_s for per-worker state
// - fib and pits
typedef struct cicn_main_s
{
  /* Binary API message ID base */
  u16 msg_id_base;

  /* Have we been enabled */
  u16 is_enabled;

  /* Convenience */
  vlib_main_t *vlib_main;
  vnet_main_t *vnet_main;
  ethernet_main_t *ethernet_main;

  /* Global FIB instance */
  cicn_fib_t fib;

  /* Info about vpp worker threads, used in our packet distribution node */
  u32 shard_count;		// 1 in single-threaded or 1 worker mode: see worker_count
  u32 worker_count;		// 0 in single-threaded case: see shard_count
  u32 worker_first_index;

  /* Node index for forwarder node in dpdk worker handoff context */
  u32 fwd_next_node;

  /* Global PIT lifetime info */
  uint64_t pit_lifetime_dflt_ms;
  uint64_t pit_lifetime_min_ms;
  uint64_t pit_lifetime_max_ms;

  /* Global ICN Hello Protocol Polling Interval */
  f64 cicn_hello_interval;

  /* The name of the ICN Hello Protocol Interests */
  cicn_hello_name_t hello_name;

  /* Is Hello Protocol polling interval set from cli/api? */
  u32 cicn_hello_interval_cfgd;

  /* Next node id for Hello Interests */
  u32 cicn_hello_next_node_id;

  /* Array of ICN Adjacencies indexed by faceid */
  cicn_hello_adj_t cicn_hello_adjs[CICN_PARAM_FACES_MAX];

  /* Array of ICN Hello data by faceid */
  cicn_hello_data cicn_hello_data_array[CICN_PARAM_FACES_MAX];

  /* Number of active adjacencies */
  u32 n_active_hello_adjs;

  uword *cicn_rc_strings;	// to print string forms of return codes

  /* Event subscribers' info */
  i32 n_face_event_subscribers;
    vl_api_cicn_api_face_events_subscribe_t
    face_event_subscribers[CICN_PARAM_API_EVENT_SUBSCRIBERS_MAX];

  /* Have we been enabled for packet-generation? */
  u32 pgen_enabled;

  /* pgen client side */
  /*   Source and destination info */
  u32 pgen_clt_src_addr;
  int pgen_clt_src_port;
  u32 pgen_clt_dest_addr;
  int pgen_clt_dest_port;

  /* pgen server side */
  /*     Have we enabled the packet generator server? */
  u32 pgen_svr_enabled;

  /*     Arbitrary content */
  u32 pgen_svr_buffer_idx;

  test_cicn_api_handler_fn test_cicn_api_handler;
} cicn_main_t;

extern cicn_main_t cicn_main;

static inline char *
cicn_sstrncpy (char *dst, const char *src, size_t n)
{
  char *res = strncpy (dst, src, n);
  dst[n - 1] = '\000';
  return (res);
}

/* Forwarder's name data structure */
typedef struct cicn_infra_fwdr_name_s
{
  uint64_t fn_match_pfx_hash;	// hash of fname's relevant pfx for match
  int fn_reply_payload_flen;	// total bytes in reply payload
#define CICN_FWDR_NAME_BUFSIZE 200
  uint8_t fn_reply_payload[CICN_FWDR_NAME_BUFSIZE];	// wire-fmt reply payload
  cicn_prefix_hashinf_t fn_hashinf;	// hash of fname components
  char fn_str[CICN_FWDR_NAME_BUFSIZE];	// fname ascii version for output
} cicn_infra_fwdr_name_t;

/* Global name of the forwarder */
extern cicn_infra_fwdr_name_t cicn_infra_fwdr_name;

extern int cicn_infra_fwdr_initialized;

/*
 * Generation numbers for coordination between config changes and running
 * worker threads. Certain configuration changes (deletes, especially)
 * cause the master config generation to increment. Each worker checks the
 * master value and updates its own dedicated value as it begins each
 * frame of work. We hope this allows us to safely integrate configuration
 * changes without more costly synchronization.
 */

/* Each value is actually a stand-alone cache line in size, so that
 * the worker threads don't have to be entangled trying to make high-rate
 * updates to shared cache lines.
 */
typedef struct cicn_infra_shard_s
{
  volatile uint64_t cfg_generation;
  uint64_t _extra[7];
  cicn_face_stats_t face_stats[CICN_PARAM_FACES_MAX];
} cicn_infra_shard_t;

/* Global generation value, updated for (some? all?) config changes */
extern cicn_infra_shard_t cicn_infra_gshard;

#define CICN_INFRA_CFG_GEN_INCR() (cicn_infra_gshard.cfg_generation++)

/* Fixed array for worker threads, to be indexed by worker index */
#define CICN_INFRA_WORKERS_MAX  24
extern cicn_infra_shard_t cicn_infra_shards[CICN_INFRA_WORKERS_MAX];

/* Per shard limits */
uint32_t cicn_infra_shard_pit_size;
uint32_t cicn_infra_shard_cs_size;

/* cicn-owned return code for cases where unix rc are insufficient */
#define foreach_cicn_rc							\
_(OK, 0, "ok")								\
_(FACE_UNKNOWN, 1, "face unknown")					\
_(FIB_PFX_COMP_LIMIT, 2, "fib prefix too man components")		\
_(FIB_PFX_SIZE_LIMIT, 3, "fib prefix too long")				\
_(FIB_NHOP_LIMIT, 4, "fib next hop limit exceeded")

typedef enum
{
#define _(a,b,c) CICN_RC_##a = b,
  foreach_cicn_rc
#undef _
} cicn_rc_e;

struct cicn_rd_s
{
  cicn_rc_e rd_cicn_rc;
  int rd_ux_rc;
};

static inline void
cicn_rd_set (cicn_rd_t * cicn_rd, cicn_rc_e cicn_rc, int ux_rc)
{
  cicn_rd->rd_cicn_rc = cicn_rc;
  cicn_rd->rd_ux_rc = ux_rc;
}

/*
 * Printable error representation
 */
const char *cicn_rc_str (cicn_rc_e crc);

const char *cicn_rd_str (cicn_rd_t * cicn_rd);

/*
 * wrapped timer sequence package (increment, comparison)
 */

/*
 * wrappable counter math (assumed uint16_t): return sum of addends
 */
static inline uint16_t
cicn_infra_seq16_sum (uint16_t addend1, uint16_t addend2)
{
  return (addend1 + addend2);
}

/*
 * for comparing wrapping numbers, return lt,eq,gt 0 for a lt,eq,gt b
 */
static inline int
cicn_infra_seq16_cmp (uint16_t a, uint16_t b)
{
  return ((int16_t) (a - b));
}

/*
 * below are wrappers for lt, le, gt, ge seq16 comparators
 */
static inline int
cicn_infra_seq16_lt (uint16_t a, uint16_t b)
{
  return (cicn_infra_seq16_cmp (a, b) < 0);
}

static inline int
cicn_infra_seq16_le (uint16_t a, uint16_t b)
{
  return (cicn_infra_seq16_cmp (a, b) <= 0);
}

static inline int
cicn_infra_seq16_gt (uint16_t a, uint16_t b)
{
  return (cicn_infra_seq16_cmp (a, b) > 0);
}

static inline int
cicn_infra_seq16_ge (uint16_t a, uint16_t b)
{
  return (cicn_infra_seq16_cmp (a, b) >= 0);
}

/* Definitions and Forward refs for the time counters we're trying out.
 * Counters are maintained by the background process.
 */
#define SEC_MS 1000
#define CICN_INFRA_FAST_TIMER_SECS  1
#define CICN_INFRA_FAST_TIMER_MSECS (CICN_INFRA_FAST_TIMER_SECS * SEC_MS)
#define CICN_INFRA_SLOW_TIMER_SECS  60
#define CICN_INFRA_SLOW_TIMER_MSECS (CICN_INFRA_SLOW_TIMER_SECS * SEC_MS)

extern uint16_t cicn_infra_fast_timer;	/* Counts at 1 second intervals */
extern uint16_t cicn_infra_slow_timer;	/* Counts at 1 minute intervals */

/*
 * Utilities to convert lifetime into expiry time based on
 * compressed clock, suitable for the opportunistic hashtable
 * entry timeout processing.
 */

// convert time in msec to time in clicks
static inline uint16_t
cicn_infra_ms2clicks (uint64_t time_ms, uint64_t ms_per_click)
{
  f64 time_clicks =
    ((f64) (time_ms + ms_per_click - 1)) / ((f64) ms_per_click);
  return ((uint16_t) time_clicks);
}

static inline uint16_t
cicn_infra_get_fast_exp_time (uint64_t lifetime_ms)
{
  uint16_t lifetime_clicks =
    cicn_infra_ms2clicks (lifetime_ms, CICN_INFRA_FAST_TIMER_MSECS);
  return (cicn_infra_seq16_sum (cicn_infra_fast_timer, lifetime_clicks));
}

static inline uint16_t
cicn_infra_get_slow_exp_time (uint64_t lifetime_ms)
{
  uint16_t lifetime_clicks =
    cicn_infra_ms2clicks (lifetime_ms, CICN_INFRA_SLOW_TIMER_MSECS);
  return (cicn_infra_seq16_sum (cicn_infra_slow_timer, lifetime_clicks));
}

int
cicn_infra_plugin_enable_disable (int enable_disable,
				  int fib_max_size,
				  int pit_max_size,
				  f64 pit_dflt_lifetime_sec_req,
				  f64 pit_min_lifetime_sec_req,
				  f64 pit_max_lifetime_sec_req,
				  int cs_max_size);

/* First versions of the ICN nodes: the forwarder node, the work-distributor
 * node and the packet-generator client and server nodes
 */
extern vlib_node_registration_t icnfwd_node;
extern vlib_node_registration_t icndist_node;
extern vlib_node_registration_t icn_pg_node;
extern vlib_node_registration_t icn_pg_server_node;

#endif // _CICN_INFRA_H_
