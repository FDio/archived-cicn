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
 * cicn.c - skeleton vpp engine plug-in
 */

#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>

#include <cicn/cicn.h>
#include <cicn/cicn_api_handler.h>

static vlib_node_registration_t icn_process_node;

cicn_main_t cicn_main;
/* Module vars */
int cicn_infra_fwdr_initialized = 0;

cicn_face_db_t cicn_face_db;

/* Global forwarder name info */
cicn_infra_fwdr_name_t cicn_infra_fwdr_name;

/* Global generation value, updated for (some? all?) config changes */
cicn_infra_shard_t cicn_infra_gshard;

/* Fixed array for worker threads, to be indexed by worker index */
cicn_infra_shard_t cicn_infra_shards[CICN_INFRA_WORKERS_MAX];

/* Global time counters we're trying out for opportunistic hashtable
 * expiration.
 */
uint16_t cicn_infra_fast_timer;	/* Counts at 1 second intervals */
uint16_t cicn_infra_slow_timer;	/* Counts at 1 minute intervals */

/*
 * Initialize support for cicn_rc_e codes
 * - build hash table mapping codes to printable strings
 */
static void
cicn_rc_strings_init (void)
{
  cicn_main.cicn_rc_strings = hash_create (0, sizeof (uword));

#define _(n,v,s) hash_set (cicn_main.cicn_rc_strings, v, s);
  foreach_cicn_rc;
#undef _
}

/*
 * modify/return supplied vector with printable representation of crc,
 * which is string name if available, otherwise numeric value.
 */
const u8 *
cicn_rc_c_string (u8 * s, cicn_rc_e crc)
{
  uword *p;

  p = hash_get (cicn_main.cicn_rc_strings, crc);

  if (p)
    {
      s = format (s, "%s", p[0]);
    }
  else
    {
      s = format (s, "%d", crc);
    }
  return (s);
}

/*
 * Return printable representation of crc.
 */
const char *
cicn_rc_str (cicn_rc_e crc)
{
  char *crc_str;

  uword *p;

  p = hash_get (cicn_main.cicn_rc_strings, crc);

  if (p)
    {
      crc_str = (char *) p[0];
    }
  else
    {
      crc_str = "unknown";
    }
  return (crc_str);
}

/*
 * Return printable representation of cicn_rd.
 * - if cicn_rc is set to an error, use that code for string
 * - otherwise use ux_rc
 */
const char *
cicn_rd_str (cicn_rd_t * cicn_rd)
{
  const char *str;
  if (cicn_rd->rd_cicn_rc != CICN_RC_OK)
    {
      str = cicn_rc_str (cicn_rd->rd_cicn_rc);
    }
  else
    {
      str = strerror (cicn_rd->rd_ux_rc);
    }
  return (str);
}

/*
 * Init CICN forwarder with configurable FIB, PIT, CS sizes
 */
static int
cicn_infra_fwdr_init (uint32_t fib_size, uint32_t shard_pit_size,
		      uint32_t shard_cs_size)
{
  int ret = 0;

  if (cicn_infra_fwdr_initialized)
    {
      cicn_cli_output ("cicn: already enabled");
      goto done;
    }

  cicn_rc_strings_init ();

  /* Initialize the forwarder's name structure */
  cicn_sstrncpy (cicn_infra_fwdr_name.fn_str, "no-name",
		 sizeof (cicn_infra_fwdr_name.fn_str));
  cicn_infra_fwdr_name.fn_reply_payload_flen = 0;

  /* Init per worker limits */
  cicn_infra_shard_pit_size = shard_pit_size;
  cicn_infra_shard_cs_size = shard_cs_size;

  /* Init face cache */
  cicn_face_db.entry_count = 0;

  /* Init event subscribers' info */
  cicn_main.n_face_event_subscribers = 0;

  /* Init the config generation number values */
  cicn_infra_gshard.cfg_generation = 1LL;
  memset (cicn_infra_shards, 0, sizeof (cicn_infra_shards));

  /* Init the global time-compression counters */
  cicn_infra_fast_timer = 1;
  cicn_infra_slow_timer = 1;

  /* Init global FIB */
  ret = cicn_fib_create (&(cicn_main.fib), fib_size);

done:

  cicn_cli_output ("cicn: fwdr initialize => %d", ret);

  if ((ret == AOK) && !cicn_infra_fwdr_initialized)
    {
      cicn_infra_fwdr_initialized = 1;
    }

  return (ret);
}

/*
 * Action function shared between message handler and debug CLI
 * NOTICE: we're only 'enabling' now
 */
int
cicn_infra_plugin_enable_disable (int enable_disable,
				  int fib_size_req,
				  int pit_size_req,
				  f64 pit_dflt_lifetime_sec_req,
				  f64 pit_min_lifetime_sec_req,
				  f64 pit_max_lifetime_sec_req,
				  int cs_size_req)
{
  int ret = 0;

  cicn_main_t *sm = &cicn_main;
  vlib_thread_main_t *tm = vlib_get_thread_main ();
  vlib_thread_registration_t *tr;
  uword *p;
  uint32_t fib_size, pit_size, cs_size;

  /* Notice if we're already enabled... */
  if (sm->is_enabled)
    {
      vlib_cli_output (sm->vlib_main, "cicn: already enabled");
      ret = 0;
      goto done;
    }

  /* Figure out how many workers will be running */
  p = hash_get_mem (tm->thread_registrations_by_name, "workers");
  tr = (vlib_thread_registration_t *) p[0];
  if (tr)
    {
      sm->worker_count = tr->count;
      sm->worker_first_index = tr->first_index;
      vlib_cli_output (sm->vlib_main,
		       "cicn: worker count %u, first idx %u",
		       sm->worker_count, sm->worker_first_index);
    }
  else
    {
      sm->worker_count = 0;
      sm->worker_first_index = 0;

      vlib_cli_output (sm->vlib_main, "cicn: no worker threads");
    }
  sm->shard_count = (sm->worker_count == 0) ? 1 : sm->worker_count;

  /* Set up params and call fwdr_init set up FIB, PIT/CS, forwarder nodes */

  /* Check the range and assign some globals */
  if (pit_min_lifetime_sec_req < 0)
    {
      sm->pit_lifetime_min_ms = CICN_PARAM_PIT_LIFETIME_DFLT_MIN_MS;
    }
  else
    {
      if (pit_min_lifetime_sec_req < CICN_PARAM_PIT_LIFETIME_BOUND_MIN_SEC ||
	  pit_min_lifetime_sec_req > CICN_PARAM_PIT_LIFETIME_BOUND_MAX_SEC)
	{
	  ret = EINVAL;
	  goto done;
	}
      sm->pit_lifetime_min_ms = pit_min_lifetime_sec_req * SEC_MS;
    }

  if (pit_max_lifetime_sec_req < 0)
    {
      sm->pit_lifetime_max_ms = CICN_PARAM_PIT_LIFETIME_DFLT_MAX_MS;
    }
  else
    {
      if (pit_max_lifetime_sec_req < CICN_PARAM_PIT_LIFETIME_BOUND_MIN_SEC ||
	  pit_max_lifetime_sec_req > CICN_PARAM_PIT_LIFETIME_BOUND_MAX_SEC)
	{
	  ret = EINVAL;
	  goto done;
	}
      sm->pit_lifetime_max_ms = pit_max_lifetime_sec_req * SEC_MS;
    }
  if (sm->pit_lifetime_min_ms > sm->pit_lifetime_max_ms)
    {
      ret = EINVAL;
      goto done;
    }

  if (pit_dflt_lifetime_sec_req < 0)
    {
      sm->pit_lifetime_dflt_ms = CICN_PARAM_PIT_LIFETIME_DFLT_DFLT_MS;
    }
  else
    {
      sm->pit_lifetime_dflt_ms = pit_dflt_lifetime_sec_req * SEC_MS;
    }
  if (sm->pit_lifetime_dflt_ms < sm->pit_lifetime_min_ms ||
      sm->pit_lifetime_dflt_ms > sm->pit_lifetime_max_ms)
    {
      goto done;
    }

  if (fib_size_req < 0)
    {
      fib_size = CICN_PARAM_FIB_ENTRIES_DFLT;
    }
  else
    {
      if (fib_size_req < CICN_PARAM_FIB_ENTRIES_MIN ||
	  fib_size_req > CICN_PARAM_FIB_ENTRIES_MAX)
	{
	  ret = EINVAL;
	  goto done;
	}
      fib_size = (uint32_t) fib_size_req;
    }

  if (pit_size_req < 0)
    {
      pit_size = CICN_PARAM_PIT_ENTRIES_DFLT;
    }
  else
    {
      if (pit_size_req < CICN_PARAM_PIT_ENTRIES_MIN ||
	  pit_size_req > CICN_PARAM_PIT_ENTRIES_MAX)
	{
	  ret = EINVAL;
	  goto done;
	}
      pit_size = (uint32_t) pit_size_req;
    }

  if (cs_size_req < 0)
    {
      cs_size = CICN_PARAM_CS_ENTRIES_DFLT;
    }
  else
    {
      if (cs_size_req > CICN_PARAM_CS_ENTRIES_MAX)
	{
	  ret = EINVAL;
	  goto done;
	}
      cs_size = (uint32_t) cs_size_req;
    }

  pit_size = pit_size / sm->shard_count;
  cs_size = cs_size / sm->shard_count;

  ret = cicn_infra_fwdr_init (fib_size, pit_size, cs_size);
  if (ret != 0)
    {
      vlib_cli_output (sm->vlib_main,
		       "cicn: enable_disable failed => %d", ret);
      goto done;
    }

#if CICN_FEATURE_MULTITHREAD
  /* If we're not running main-thread only, set up a relationship between
   * the dpdk worker handoff node and our forwarding node.
   */
  if (sm->worker_count > 1)
    {
      /* Engage with the worker thread handoff node so that we can dispatch
       * through it from our dist node directly to our forwarder node
       */
      sm->fwd_next_node = vlib_node_add_next (sm->vlib_main,
					      handoff_dispatch_node.index,
					      icnfwd_node.index);
      vlib_cli_output (sm->vlib_main,
		       "cicn: handoff node %u, fwd node next idx %u",
		       handoff_dispatch_node.index, sm->fwd_next_node);
    }
#endif //CICN_FEATURE_MULTITHREAD

  ret = cicn_hello_plugin_activation_init (sm->vlib_main);	//start adj protocol
  if (ret != AOK)
    {
      goto done;
    }

  sm->is_enabled = 1;

done:

  return (ret);
}

/*
 * The entry-point for the ICN background process, which does...
 * background things, like garbage collection, for us.
 */
static uword
icn_process_fn (vlib_main_t * vm, vlib_node_runtime_t * rt, vlib_frame_t * f)
{
#define CICN_PROCESS_WAIT_TIME  1.0	/* 1 second */

  f64 timeout = CICN_PROCESS_WAIT_TIME;
  f64 tnow, tnext = 0.0;
  uword event_type;
  uword *event_data = 0;
  int timer_counter = 0;

  while (1)
    {
      vlib_process_wait_for_event_or_clock (vm, timeout);

      event_type = vlib_process_get_events (vm, &event_data);

      tnow = vlib_time_now (cicn_main.vlib_main);
      if (tnext == 0.0)
	{
	  tnext = tnow + CICN_INFRA_FAST_TIMER_SECS;
	}

      /* Update the timeout compression counters we're trying for
       * opportunistic timeouts in the hashtables.
       */
      if (tnow >= tnext)
	{
	  cicn_infra_fast_timer =
	    cicn_infra_seq16_sum (cicn_infra_fast_timer, 1);

	  if ((++timer_counter % CICN_INFRA_SLOW_TIMER_SECS) == 0)
	    {
	      cicn_infra_slow_timer =
		cicn_infra_seq16_sum (cicn_infra_slow_timer, 1);
	      timer_counter = 0;
	    }

	  tnext = tnow + CICN_INFRA_FAST_TIMER_SECS;
	}

      switch (event_type)
	{
	case ~0:
	default:
	  /* Reset timeout */
	  timeout = CICN_PROCESS_WAIT_TIME;
	  break;

	}			/* End switch() */

      vec_reset_length (event_data);
    }

  /* NOTREACHED */
  return 0;
}

VLIB_REGISTER_NODE (icn_process_node, static) =
{
  .function = icn_process_fn,
  .type = VLIB_NODE_TYPE_PROCESS,
  .name = "icn-process",
  .process_log2_n_stack_bytes = 16
};

/*
 * Init entry-point for the icn plugin
 */
static clib_error_t *
cicn_init (vlib_main_t * vm)
{
  clib_error_t *error = 0;

  cicn_main_t *sm = &cicn_main;

  sm->vlib_main = vm;
  sm->vnet_main = vnet_get_main ();

  /* Init other elements in the 'main' struct */
  sm->is_enabled = 0;
  sm->fwd_next_node = ~0;

  sm->pgen_enabled = 0;
  sm->pgen_clt_src_addr = sm->pgen_clt_dest_addr = 0;
  sm->pgen_clt_src_port = sm->pgen_clt_dest_port = 0;

  sm->pgen_svr_enabled = 0;

  error = cicn_api_plugin_hookup (vm);

  return error;
}

VLIB_INIT_FUNCTION (cicn_init);

/* *INDENT-OFF* */
VLIB_PLUGIN_REGISTER () = {
};
/* *INDENT-ON* */
