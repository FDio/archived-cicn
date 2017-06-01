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
 * Management plane:
 * - Handlers for CICN binary API operations (vpp_api_test)
 * - Declarations of and handlers for DBG-cli commands
 * - Internal management operation handlers called by both of the
 *   above, to minimize copied code.
 */
#include <inttypes.h>

#include <vlib/vlib.h>
#include <vppinfra/error.h>
#include <vnet/ip/format.h>
#include <vlibapi/api.h>
#include <vlibmemory/api.h>
#include <vlibsocket/api.h>

#include <vnet/udp/udp.h>	// port registration

#include <cicn/cicn.h>

/* define message IDs */
#include <cicn/cicn_msg_enum.h>

/* define generated endian-swappers */
#define vl_endianfun
#include <cicn/cicn_all_api_h.h>
#undef vl_endianfun

/* instantiate all the print functions we know about */
#define vl_print(handle, ...) vlib_cli_output (handle, __VA_ARGS__)
#define vl_printfun
#include <cicn/cicn_all_api_h.h>
#undef vl_printfun

/* Get the API version number */
#define vl_api_version(n,v) static u32 api_version=(v);
#include <cicn/cicn_all_api_h.h>
#undef vl_api_version

/*
 * Handy macros to initialize/send a message reply.
 * Assumes that certain variables, showed in comments, are available
 * as local variables in using routing.
 */

// allocate rmp buffer, verify queue is valie
#define REPLY_SETUP(t, rmp, q, mp)				\
do {                                                            \
    q = vl_api_client_index_to_input_queue(mp->client_index);   \
    if (!q)                                                     \
        return;                                                 \
                                                                \
    rmp = vl_msg_api_alloc(sizeof(*rmp));                       \
    rmp->_vl_msg_id = ntohs(sm->msg_id_base + (t));             \
    rmp->context = mp->context;                                 \
} while(0);

// set return value and send response
#define REPLY_FINISH(rmp, q, rv)                                \
do {                                                            \
    rmp->retval = ntohl(rv);                                    \
    vl_msg_api_send_shmem (q, (u8 *)&rmp);                      \
} while(0);

// combined single vector to allocate rmp buffer and send rv response
// can only be used for API calls (e.g. "set" calls) that only return rv
#define REPLY_MACRO(t/*, rmp, mp, rv*/)				\
do {                                                            \
    unix_shared_memory_queue_t * q;                             \
    REPLY_SETUP(t, rmp, q, mp);					\
    REPLY_FINISH(rmp, q, rv);                                   \
} while(0);


/*
 * Convert a unix return code to a vnet_api return code.
 * Currently stubby: should have more cases.
 */
static inline vnet_api_error_t
cicn_api_rv_from_unix_rc (int ux_rc)
{
  vnet_api_error_t vae;

  switch (ux_rc)
    {
    case AOK:
      vae = CICN_VNET_API_ERROR_NONE;
      break;
    default:
      vae = VNET_API_ERROR_SYSCALL_ERROR_9;	// should not happen, add cases
      break;
    }
  return (vae);
}

/*
 * Convert a unix return code to a vnet_api return code.
 * Currently stubby: should use cl_error unix_rc if available
 */
static inline vnet_api_error_t
cicn_api_rv_from_clib_error (clib_error_t * cl_err)
{
  vnet_api_error_t vae;

  if (cl_err == NULL)
    {
      vae = CICN_VNET_API_ERROR_NONE;
    }
  else
    {
      vae = VNET_API_ERROR_SYSCALL_ERROR_9;	// should not happen, add cases
    }
  return (vae);
}


/*
 * Hide the details of cli output from the cicn-aware modules
 */
int
cicn_cli_output (const char *fmt, ...)
{
  cicn_main_t *sm = &cicn_main;
  va_list args;
  char buf[200];

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  /* Belt and suspenders */
  buf[sizeof (buf) - 1] = 0;

  vlib_cli_output (sm->vlib_main, buf);

  return (0);
}

/* API message handler */
static void
vl_api_cicn_api_node_params_set_t_handler (vl_api_cicn_api_node_params_set_t *
					   mp)
{
  vl_api_cicn_api_node_params_set_reply_t *rmp;
  vnet_api_error_t rv;

  cicn_main_t *sm = &cicn_main;
  int ux_rc;

  //TODO: implement clib_net_to_host_f64 in VPP ?
  int fib_max_size = clib_net_to_host_i32 (mp->fib_max_size);
  int pit_max_size = clib_net_to_host_i32 (mp->pit_max_size);
  f64 pit_dflt_lifetime_sec = mp->pit_dflt_lifetime_sec;
  f64 pit_min_lifetime_sec = mp->pit_min_lifetime_sec;
  f64 pit_max_lifetime_sec = mp->pit_max_lifetime_sec;
  int cs_max_size = clib_net_to_host_i32 (mp->cs_max_size);

  ux_rc = cicn_infra_plugin_enable_disable ((int) (mp->enable_disable),
					    fib_max_size, pit_max_size,
					    pit_dflt_lifetime_sec,
					    pit_min_lifetime_sec,
					    pit_max_lifetime_sec,
					    cs_max_size);

  rv = cicn_api_rv_from_unix_rc (ux_rc);
  REPLY_MACRO (VL_API_CICN_API_NODE_PARAMS_SET_REPLY /*, rmp, mp, rv */ );
}

/* API message handler */
static void
vl_api_cicn_api_node_params_get_t_handler (vl_api_cicn_api_node_params_get_t *
					   mp)
{
  vl_api_cicn_api_node_params_get_reply_t *rmp;
  vnet_api_error_t rv;

  cicn_main_t *sm = &cicn_main;
  int ux_rc = AOK;

  unix_shared_memory_queue_t *q =
    vl_api_client_index_to_input_queue (mp->client_index);
  if (!q)
    {
      return;
    }

  rmp = vl_msg_api_alloc (sizeof (*rmp));
  rmp->_vl_msg_id =
    ntohs (sm->msg_id_base + VL_API_CICN_API_NODE_PARAMS_GET_REPLY);
  rmp->context = mp->context;
  rmp->is_enabled = sm->is_enabled;

  rmp->feature_multithread = CICN_FEATURE_MULTITHREAD;
  rmp->feature_cs = CICN_FEATURE_CS;
  rmp->feature_clone_replication = CICN_INFRA_CLONE_REPLICATION;

  rmp->worker_count = clib_host_to_net_u32 (sm->worker_count);

  rmp->fib_max_size = clib_host_to_net_u32 (sm->fib.fib_capacity);

  rmp->pit_max_size =
    clib_host_to_net_u32 (cicn_infra_shard_pit_size * sm->shard_count);
  //TODO: add clib_host_to_net_f64 to VPP ?
  rmp->pit_dflt_lifetime_sec = ((f64) sm->pit_lifetime_dflt_ms) / SEC_MS;
  rmp->pit_min_lifetime_sec = ((f64) sm->pit_lifetime_min_ms) / SEC_MS;
  rmp->pit_max_lifetime_sec = ((f64) sm->pit_lifetime_max_ms) / SEC_MS;
  rmp->cs_max_size =
    clib_host_to_net_u32 (cicn_infra_shard_cs_size * sm->shard_count);

  rv = cicn_api_rv_from_unix_rc (ux_rc);
  rmp->retval = clib_host_to_net_i32 (rv);
  vl_msg_api_send_shmem (q, (u8 *) & rmp);
}

static vl_api_cicn_api_node_params_set_t node_ctl_params = {
  .fib_max_size = -1,
  .pit_max_size = -1,
  .pit_dflt_lifetime_sec = -1.0f,
  .pit_min_lifetime_sec = -1.0f,
  .pit_max_lifetime_sec = -1.0f,
  .cs_max_size = -1,
};

/*
 * cli handler for 'control start'
 */
static clib_error_t *
cicn_cli_node_ctl_start_set_command_fn (vlib_main_t * vm,
					unformat_input_t * main_input,
					vlib_cli_command_t * cmd)
{
  int ux_rc;

  /* Catch unexpected extra arguments on this line.
   * Get a line of input but only in the unexpected case that line
   * line not already consumed by matching VLIB_CLI_COMMAND.path)
   * [i.e. on "cicn control start\n", don't consume the following line (cmd)
   *  while catching unexpected extra arguments on "cicn control start XXX"]
   */
  if (main_input->index > 0 &&
      main_input->buffer[main_input->index - 1] != '\n')
    {
      unformat_input_t _line_input, *line_input = &_line_input;
      if (!unformat_user (main_input, unformat_line_input, line_input))
	{
	  return (0);
	}

      while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
	{
	  return clib_error_return (0, "Unknown argument '%U'",
				    format_unformat_error, line_input);
	}
    }

  ux_rc = cicn_infra_plugin_enable_disable (1 /*enable */ ,
					    node_ctl_params.fib_max_size,
					    node_ctl_params.pit_max_size,
					    node_ctl_params.pit_dflt_lifetime_sec,
					    node_ctl_params.pit_min_lifetime_sec,
					    node_ctl_params.pit_max_lifetime_sec,
					    node_ctl_params.cs_max_size);

  switch (ux_rc)
    {
    case AOK:
      break;
    default:
      return clib_error_return (0, "cmd returned %d", ux_rc);
    }

  return (0);
}

/*
 * cli handler for 'control stop'
 */
static clib_error_t *
cicn_cli_node_ctl_stop_set_command_fn (vlib_main_t * vm,
				       unformat_input_t * main_input,
				       vlib_cli_command_t * cmd)
{
  int ux_rc;

  /* Catch unexpected extra arguments on this line.
   * See comment on cicn_cli_node_ctrl_start_set_command_fn
   */
  if (main_input->index > 0 &&
      main_input->buffer[main_input->index - 1] != '\n')
    {
      unformat_input_t _line_input, *line_input = &_line_input;
      if (!unformat_user (main_input, unformat_line_input, line_input))
	{
	  return (0);
	}

      while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
	{
	  return clib_error_return (0, "Unknown argument '%U'",
				    format_unformat_error, line_input);
	}
    }

  ux_rc = cicn_infra_plugin_enable_disable (0 /*!enable */ ,
					    node_ctl_params.fib_max_size,
					    node_ctl_params.pit_max_size,
					    node_ctl_params.pit_dflt_lifetime_sec,
					    node_ctl_params.pit_min_lifetime_sec,
					    node_ctl_params.pit_max_lifetime_sec,
					    node_ctl_params.cs_max_size);

  switch (ux_rc)
    {
    case AOK:
      break;
    default:
      return clib_error_return (0, "cmd returned %d", ux_rc);
    }

  return (0);
}

#define DFLTD_RANGE_OK(val, min, max)		\
({						\
    __typeof__ (val) _val = (val); 		\
    __typeof__ (min) _min = (min); 		\
    __typeof__ (max) _max = (max); 		\
    (_val == -1) ||				\
    (_val >= _min && _val <= _max);		\
})

/*
 * cli handler for 'control param'
 */
static clib_error_t *
cicn_cli_node_ctl_param_set_command_fn (vlib_main_t * vm,
					unformat_input_t * main_input,
					vlib_cli_command_t * cmd)
{
  int rv = 0;

  int table_size;
  f64 lifetime;

  if (cicn_main.is_enabled)
    {
      return (clib_error_return
	      (0, "params cannot be altered once cicn started"));
    }

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "fib"))
	{
	  if (unformat (line_input, "size %d", &table_size))
	    {
	      if (!DFLTD_RANGE_OK (table_size, CICN_PARAM_FIB_ENTRIES_MIN,
				   CICN_PARAM_FIB_ENTRIES_MAX))
		{
		  rv = VNET_API_ERROR_INVALID_VALUE;
		  break;
		}
	      node_ctl_params.fib_max_size = table_size;
	    }
	  else
	    {
	      rv = VNET_API_ERROR_UNIMPLEMENTED;
	      break;
	    }
	}
      else if (unformat (line_input, "pit"))
	{
	  if (unformat (line_input, "size %d", &table_size))
	    {
	      if (!DFLTD_RANGE_OK (table_size, CICN_PARAM_PIT_ENTRIES_MIN,
				   CICN_PARAM_PIT_ENTRIES_MAX))
		{
		  rv = VNET_API_ERROR_INVALID_VALUE;
		  break;
		}
	      node_ctl_params.pit_max_size = table_size;
	    }
	  else if (unformat (line_input, "dfltlife %f", &lifetime))
	    {
	      if (!DFLTD_RANGE_OK
		  (lifetime, CICN_PARAM_PIT_LIFETIME_BOUND_MIN_SEC,
		   CICN_PARAM_PIT_LIFETIME_BOUND_MAX_SEC))
		{
		  rv = VNET_API_ERROR_INVALID_VALUE;
		  break;
		}
	      node_ctl_params.pit_dflt_lifetime_sec = lifetime;
	    }
	  else if (unformat (line_input, "minlife %f", &lifetime))
	    {
	      if (!DFLTD_RANGE_OK
		  (lifetime, CICN_PARAM_PIT_LIFETIME_BOUND_MIN_SEC,
		   CICN_PARAM_PIT_LIFETIME_BOUND_MAX_SEC))
		{
		  rv = VNET_API_ERROR_INVALID_VALUE;
		  break;
		}
	      node_ctl_params.pit_min_lifetime_sec = lifetime;
	    }
	  else if (unformat (line_input, "maxlife %f", &lifetime))
	    {
	      if (!DFLTD_RANGE_OK
		  (lifetime, CICN_PARAM_PIT_LIFETIME_BOUND_MIN_SEC,
		   CICN_PARAM_PIT_LIFETIME_BOUND_MAX_SEC))
		{
		  rv = VNET_API_ERROR_INVALID_VALUE;
		  break;
		}
	      node_ctl_params.pit_max_lifetime_sec = lifetime;
	    }
	  else
	    {
	      rv = VNET_API_ERROR_UNIMPLEMENTED;
	      break;
	    }
	}
      else if (unformat (line_input, "cs"))
	{
	  if (unformat (line_input, "size %d", &table_size))
	    {
	      if (!DFLTD_RANGE_OK (table_size, CICN_PARAM_CS_ENTRIES_MIN,
				   CICN_PARAM_CS_ENTRIES_MAX))
		{
		  rv = VNET_API_ERROR_INVALID_VALUE;
		  break;
		}
	      node_ctl_params.cs_max_size = table_size;
	    }
	  else
	    {
	      rv = VNET_API_ERROR_UNIMPLEMENTED;
	      break;
	    }
	}
      else
	{
	  rv = VNET_API_ERROR_UNIMPLEMENTED;
	  break;
	}
    }

  switch (rv)
    {
    case 0:
      break;
    case VNET_API_ERROR_UNIMPLEMENTED:
      return clib_error_return (0, "Unknown argument '%U'",
				format_unformat_error, line_input);
    default:
      return clib_error_return (0, "cmd returned %d", rv);
    }

  return (0);
}

/*
 * cli handler for 'enable'
 */
static clib_error_t *
cicn_cli_node_enable_disable_set_command_fn (vlib_main_t * vm,
					     unformat_input_t * main_input,
					     vlib_cli_command_t * cmd)
{
  int enable_disable = 1;
  int ux_rc;

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "disable"))
	{
	  enable_disable = 0;
	}
      else
	{
	  return clib_error_return (0, "Unknown argument '%U'",
				    format_unformat_error, line_input);
	}
    }

  ux_rc = cicn_infra_plugin_enable_disable (enable_disable,
					    node_ctl_params.fib_max_size,
					    node_ctl_params.pit_max_size,
					    node_ctl_params.pit_dflt_lifetime_sec,
					    node_ctl_params.pit_min_lifetime_sec,
					    node_ctl_params.pit_max_lifetime_sec,
					    node_ctl_params.cs_max_size);

  switch (ux_rc)
    {
    case AOK:
      break;

    default:
      return clib_error_return (0, "cicn enable_disable returned %d", ux_rc);
    }
  return 0;
}

/*
 * cli handler for 'cfg name': router's own ICN name
 */
static clib_error_t *
cicn_cli_node_name_set_command_fn (vlib_main_t * vm,
				   unformat_input_t * main_input,
				   vlib_cli_command_t * cmd)
{
  cicn_infra_fwdr_name_t *gfname = &cicn_infra_fwdr_name;
  int delete = 0;
  uint8_t buf[200];
  int len, ret;
  const char *fwdr_name = NULL;
  uint8_t *ptr;

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "delete"))
	{
	  delete = 1;
	}
      else if (unformat (line_input, "%s", &fwdr_name))
	{
	  ;
	}
      else
	{
	  return clib_error_return (0, "Unknown argument '%U'",
				    format_unformat_error, line_input);
	}
    }

  /* Verify that the given name is not empty */
  if (fwdr_name == NULL)
    {
      return clib_error_return (0, "Please specify an non-empty name...");
    }

  /* Handle delete case */
  if (delete)
    {
      if (gfname->fn_reply_payload_flen == 0)
	{
	  return clib_error_return (0,
				    "Forwarder does not have a name yet...");
	}
      else if (strcmp (gfname->fn_str, fwdr_name) == 0)
	{
	  cicn_sstrncpy (gfname->fn_str, "no-name", sizeof (gfname->fn_str));
	  gfname->fn_reply_payload_flen = 0;
	  vlib_cli_output (vm, "name:%s: deleted successfully", fwdr_name);
	}
      else
	{
	  return clib_error_return (0, "Name for deletion not found...");
	}
    }
  else
    {
      /* TODO: Potentially do more validation for the parsed name */
      if (strlen (fwdr_name) > sizeof (buf))
	{
	  return clib_error_return (0, "The given name is too long...");
	}
      /* Convert prefix to wire-format */
      cicn_rd_t cicn_rd;
      len =
	cicn_parse_name_comps_from_str (buf, sizeof (buf), fwdr_name,
					&cicn_rd);
      if (len < 0)
	{
	  return clib_error_return (0,
				    "Could not parse name comps from the name: %s...",
				    cicn_rd_str (&cicn_rd));
	}
      /* Hash the prefix */
      ret = cicn_hashtb_hash_prefixes (buf, len, 0 /*full_name */ ,
				       &gfname->fn_hashinf, 0 /*limit */ );
      if (ret != AOK)
	{
	  return clib_error_return (0, "Could not hash the given name...");
	}
      gfname->fn_match_pfx_hash =
	gfname->fn_hashinf.pfx_hashes[gfname->fn_hashinf.pfx_count - 1];
      cicn_sstrncpy (gfname->fn_str, fwdr_name, sizeof (gfname->fn_str));

      gfname->fn_reply_payload_flen = CICN_TLV_HDR_LEN + len;
      /* Check for overflow */
      if (gfname->fn_reply_payload_flen > CICN_FWDR_NAME_BUFSIZE)
	{
	  vlib_cli_output (vm, "traceroute payload TLV: overflow");
	}

      /* Create the traceroute payload (name TLV) */
      memset (gfname->fn_reply_payload, 0, sizeof (gfname->fn_reply_payload));
      ptr = gfname->fn_reply_payload;
      C_PUTINT16 (&ptr[0], CICN_TLV_PAYLOAD);
      C_PUTINT16 (&ptr[CICN_TLV_TYPE_LEN], len);

      memcpy (&ptr[CICN_TLV_HDR_LEN], buf, len);

      vlib_cli_output (vm, "name %s: added successfully", gfname->fn_str);
    }
  return (0);
}

/* shared routine betweeen API and CLI, leveraging API message structure */
static int
cicn_mgmt_node_stats_get (vl_api_cicn_api_node_stats_get_reply_t * rmp)
{
  rmp->pkts_processed = 0;
  rmp->pkts_interest_count = 0;
  rmp->pkts_data_count = 0;
  rmp->pkts_nak_count = 0;
  rmp->pkts_from_cache_count = 0;
  rmp->pkts_nacked_interests_count = 0;
  rmp->pkts_nak_hoplimit_count = 0;
  rmp->pkts_nak_no_route_count = 0;
  rmp->pkts_no_pit_count = 0;
  rmp->pit_expired_count = 0;
  rmp->cs_expired_count = 0;
  rmp->cs_lru_count = 0;
  rmp->pkts_drop_no_buf = 0;
  rmp->interests_aggregated = 0;
  rmp->interests_retx = 0;
  rmp->pit_entries_count = 0;
  rmp->cs_entries_count = 0;

  vlib_error_main_t *em;
  vlib_node_t *n;
  foreach_vlib_main ((
		       {
		       em = &this_vlib_main->error_main;
		       n = vlib_get_node (this_vlib_main, icnfwd_node.index);
		       u32 node_cntr_base_idx = n->error_heap_index;
		       rmp->pkts_processed +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_PROCESSED]);
		       rmp->pkts_interest_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_INTERESTS]);
		       rmp->pkts_data_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_DATAS]);
		       rmp->pkts_nak_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_NAKS]);
		       rmp->pkts_from_cache_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_CACHED]);
		       rmp->pkts_nacked_interests_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_NACKED_INTERESTS]);
		       rmp->pkts_nak_hoplimit_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_HOPLIMIT_EXCEEDED]);
		       rmp->pkts_nak_no_route_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_NO_ROUTE]);
		       rmp->pkts_no_pit_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_NO_PIT]);
		       rmp->pit_expired_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_PIT_EXPIRED]);
		       rmp->cs_expired_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_CS_EXPIRED]);
		       rmp->cs_lru_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_CS_LRU]);
		       rmp->pkts_drop_no_buf +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_NO_BUFS]);
		       rmp->interests_aggregated +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_INTEREST_AGG]);
		       rmp->interests_retx +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_INT_RETRANS]);
		       rmp->pit_entries_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_INT_COUNT]);
		       rmp->cs_entries_count +=
		       clib_host_to_net_u64 (em->counters[node_cntr_base_idx +
							  ICNFWD_ERROR_CS_COUNT]);
		       }));
  return (AOK);
}

/* API message handler */
static void
vl_api_cicn_api_node_stats_get_t_handler (vl_api_cicn_api_node_stats_get_t *
					  mp)
{
  vl_api_cicn_api_node_stats_get_reply_t *rmp;
  cicn_main_t *sm = &cicn_main;
  vnet_api_error_t vaec = CICN_VNET_API_ERROR_NONE;

  unix_shared_memory_queue_t *q =
    vl_api_client_index_to_input_queue (mp->client_index);
  if (!q)
    return;

  rmp = vl_msg_api_alloc (sizeof (*rmp));
  rmp->_vl_msg_id =
    ntohs (sm->msg_id_base + VL_API_CICN_API_NODE_STATS_GET_REPLY);
  rmp->context = mp->context;

  int ux_rc = cicn_mgmt_node_stats_get (rmp);
  if (ux_rc != AOK)
    {
      vaec = cicn_api_rv_from_unix_rc (ux_rc);
    }

  rmp->retval = clib_host_to_net_i32 (vaec);

  vl_msg_api_send_shmem (q, (u8 *) & rmp);
}

/*
 * cli handler for 'cfg salt': per-router hash salt/nonce
 */
static clib_error_t *
cicn_cli_salt_set_command_fn (vlib_main_t * vm, unformat_input_t * main_input,
			      vlib_cli_command_t * cmd)
{
  return (clib_error_return (0, "Not yet implemented..."));
}

typedef enum
{
  CICN_MGMT_FACE_OP_NONE = 0,
  CICN_MGMT_FACE_OP_CREATE,
  CICN_MGMT_FACE_OP_DELETE,
  CICN_MGMT_FACE_OP_ADMIN,
  CICN_MGMT_FACE_OP_HELLO,
} cicn_mgmt_face_op_e;

/*
 * Push Face notifications to all subscribers
 */
static void
cicn_api_face_event_send (int faceid, int faceflags)
{
  cicn_main_t *sm = &cicn_main;
  vl_api_cicn_api_face_event_t *event = vl_msg_api_alloc (sizeof (*event));

  int i;
  for (i = 0; i < sm->n_face_event_subscribers; i++)
    {
      unix_shared_memory_queue_t *mq =
	vl_api_client_index_to_input_queue (sm->
					    face_event_subscribers
					    [i].client_index);
      if (!mq)
	continue;

      memset (event, 0, sizeof (*event));
      event->_vl_msg_id =
	ntohs (sm->msg_id_base + VL_API_CICN_API_FACE_EVENT);
      event->context = sm->face_event_subscribers[i].context;
      event->client_index = sm->face_event_subscribers[i].client_index;
      event->faceid = clib_host_to_net_i32 (faceid);
      event->flags = clib_host_to_net_i32 (faceflags);

      vl_msg_api_send_shmem (mq, (u8 *) & event);
    }
}

/*
 * Face add routine common to binary api and cli.
 *
 * Adds UDPv4 face and returns new Face ID if successful, -1 otherwise
 *
 * TODO -- how to un-register? doesn't seem to be an api for that.
 */
static vnet_api_error_t
cicn_mgmt_face_add (ip4_address_t local_addr4, int local_port,
		    ip4_address_t remote_addr4, int remote_port,
		    int app_face, int *faceid)
{
  vnet_api_error_t rv;

  cicn_main_t *sm = &cicn_main;
  vnet_main_t *vnm = vnet_get_main ();
  vnet_interface_main_t *im = &vnm->interface_main;
  vnet_sw_interface_t *swif_list = 0, *si;
  ip4_main_t *im4 = &ip4_main;
  ip_lookup_main_t *lm4 = &im4->lookup_main;
  ip4_address_t *addr4;
  ip_interface_address_t *ia = 0;
  int found_p;

  /* Look for a matching swif for the local address */
  found_p = 0;
  swif_list = vec_new (vnet_sw_interface_t, pool_elts (im->sw_interfaces));
  _vec_len (swif_list) = 0;

  pool_foreach (si, im->sw_interfaces, (
					 {
					 vec_add1 (swif_list, si[0]);
					 }));

  vec_foreach (si, swif_list)
  {
    foreach_ip_interface_address (lm4, ia, si->sw_if_index, 1, (
								 {
								 addr4 =
								 ip_interface_address_get_address
								 (lm4, ia);
								 if
								 (addr4->as_u32
								  ==
								  local_addr4.as_u32)
								 {
								 found_p = 1;
								 break;}
								 }
				  ));

    if (found_p)
      {
	break;
      }
  };

  if (!found_p)
    {
      rv = VNET_API_ERROR_NO_SUCH_ENTRY;
      goto done;
    }

  //  vlib_cli_output(sm->vlib_main, "cicn: face swif %d", si->sw_if_index);

  /* TODO -- Check that the swif is 'up'? */
  //  if ((si->flags & VNET_SW_INTERFACE_FLAG_ADMIN_UP) == 0) {}

  /* Create  a cicn 'face', and capture needed info in the face cache */
  int ux_rc;
  cicn_rd_t cicn_rd;
  *faceid = -1;
  ux_rc = cicn_face_add (local_addr4.as_u32, local_port, remote_addr4.as_u32,
			 remote_port, app_face, si->sw_if_index, faceid,
			 &cicn_rd);
  if (ux_rc != AOK)
    {				// TODO: look at cicn_rc.rd_cicn_rc first
      rv = cicn_api_rv_from_unix_rc (ux_rc);
      goto done;
    }
  cicn_face_db_entry_t *face;
  ux_rc = cicn_face_entry_find_by_id (*faceid, &face);
  if (ux_rc != AOK)
    {
      rv = cicn_api_rv_from_unix_rc (ux_rc);
      goto done;
    }

  /* Update config generation number */
  CICN_INFRA_CFG_GEN_INCR ();

  /* TODO -- output new face id on success? */

  /* On success, start taking packets on the local port. Packets are
   * delivered to our work-distribution nodes, which then pass them to
   * forwarding nodes.
   */

  /* TODO -- only register the port if it's unique? */

  /* If there are worker threads, register our distribution node,
   * which will decide how packets go to forwarding threads.
   */
  if (sm->worker_count > 1)
    {
#if CICN_FEATURE_MULTITHREAD
      udp_register_dst_port (sm->vlib_main, local_port,
			     icndist_node.index, 1);
#else
      ASSERT (sm->worker_count <= 1);
#endif
    }
  else
    {
      /* Register the forwarding node directly otherwise (in
       * single-threaded mode, e.g.)
       */
      udp_register_dst_port (sm->vlib_main, local_port, icnfwd_node.index, 1);
    }

  rv = CICN_VNET_API_ERROR_NONE;

done:

  vec_free (swif_list);

  return (rv);
}

/*
 * Face add routine common to binary api and cli.
 *
 * Removes specified face
 */
static clib_error_t *
cicn_mgmt_face_remove (int faceid)
{
  return clib_error_return (0, "face deletion not implemented");
}

/* API message handler */
static void
vl_api_cicn_api_face_add_t_handler (vl_api_cicn_api_face_add_t * mp)
{
  vl_api_cicn_api_face_add_reply_t *rmp;
  vnet_api_error_t rv;
  unix_shared_memory_queue_t *q;

  cicn_main_t *sm = &cicn_main;
  int faceid = -1;

  ip4_address_t local_addr =
    (ip4_address_t) (clib_net_to_host_u32 (mp->local_addr));

  uint16_t local_port = clib_net_to_host_u16 (mp->local_port);

  ip4_address_t remote_addr =
    (ip4_address_t) (clib_net_to_host_u32 (mp->remote_addr));

  uint16_t remote_port = clib_net_to_host_u16 (mp->remote_port);

  REPLY_SETUP (VL_API_CICN_API_FACE_ADD_REPLY, rmp, q, mp);

  rv =
    cicn_mgmt_face_add (local_addr, (int) local_port, remote_addr,
			(int) remote_port, 0 /*is_app */ , &faceid);

  if (rv >= 0)
    {
      rmp->faceid = clib_host_to_net_i32 (faceid);
    }

  REPLY_FINISH (rmp, q, rv);

  if (rv >= 0)
    {
      // send event: for api, defer until after api response
      cicn_api_face_event_send (faceid, CICN_FACE_FLAGS_DEFAULT);
    }
}

/* API message handler */
static void
vl_api_cicn_api_face_delete_t_handler (vl_api_cicn_api_face_delete_t * mp)
{
  vl_api_cicn_api_face_delete_reply_t *rmp;
  vnet_api_error_t rv;

  cicn_main_t *sm = &cicn_main;
  clib_error_t *cl_err = 0;

  int faceid = clib_net_to_host_i32 (mp->faceid);
  cl_err = cicn_mgmt_face_remove (faceid);

  rv = cicn_api_rv_from_clib_error (cl_err);
  REPLY_MACRO (VL_API_CICN_API_FACE_DELETE_REPLY /*, rmp, mp, rv */ );

  // TODO: check error value or rv value
  cicn_api_face_event_send (mp->faceid, CICN_FACE_FLAG_DELETED);
}

/* API message handler */
static void
vl_api_cicn_api_face_params_get_t_handler (vl_api_cicn_api_face_params_get_t *
					   mp)
{
  vl_api_cicn_api_face_params_get_reply_t *rmp;
  vnet_api_error_t rv;
  unix_shared_memory_queue_t *q;

  cicn_main_t *sm = &cicn_main;

  int faceid = clib_net_to_host_i32 (mp->faceid);

  REPLY_SETUP (VL_API_CICN_API_FACE_PARAMS_GET_REPLY, rmp, q, mp);

  rv = cicn_face_api_entry_params_serialize (faceid, rmp);

  REPLY_FINISH (rmp, q, rv);
}

/* API message handler */
static void
vl_api_cicn_api_face_props_get_t_handler (vl_api_cicn_api_face_props_get_t *
					  mp)
{
  vl_api_cicn_api_face_props_get_reply_t *rmp;
  vnet_api_error_t rv = 0;
  unix_shared_memory_queue_t *q;

  cicn_main_t *sm = &cicn_main;

  REPLY_SETUP (VL_API_CICN_API_FACE_PROPS_GET_REPLY, rmp, q, mp);

  rv = cicn_face_api_entry_props_serialize (rmp);

  REPLY_FINISH (rmp, q, rv);
}

/* API message handler */
static void
vl_api_cicn_api_face_stats_get_t_handler (vl_api_cicn_api_face_stats_get_t *
					  mp)
{
  vl_api_cicn_api_face_stats_get_reply_t *rmp;
  vnet_api_error_t rv;
  unix_shared_memory_queue_t *q;

  cicn_main_t *sm = &cicn_main;

  int faceid = clib_net_to_host_i32 (mp->faceid);

  REPLY_SETUP (VL_API_CICN_API_FACE_STATS_GET_REPLY, rmp, q, mp);

  rv = cicn_face_api_entry_stats_serialize (faceid, rmp);

  REPLY_FINISH (rmp, q, rv);
}

/* API message handler */
static void
  vl_api_cicn_api_face_events_subscribe_t_handler
  (vl_api_cicn_api_face_events_subscribe_t * mp)
{
  cicn_main_t *sm = &cicn_main;
  vl_api_cicn_api_face_events_subscribe_reply_t *rmp;

  int rv = VNET_API_ERROR_INVALID_ARGUMENT;

  u16 enable = clib_net_to_host_u16 (mp->enable_disable);

  if (enable == 1)
    {
      // if the maximum number of event subscribers is not exceeded yet
      if (sm->n_face_event_subscribers <
	  CICN_PARAM_API_EVENT_SUBSCRIBERS_MAX - 1)
	{
	  // save the info about the event subscriber
	  memcpy (&(sm->face_event_subscribers[sm->n_face_event_subscribers]),
		  mp, sizeof (*mp));

	  sm->n_face_event_subscribers++;

	  rv = CICN_VNET_API_ERROR_NONE;
	}
    }
  else if (enable == 0)
    {
      rv = VNET_API_ERROR_UNSPECIFIED;

      // find the event subscriber with matching client_index
      int i;
      for (i = 0; i < sm->n_face_event_subscribers; i++)
	{
	  if (mp->client_index == sm->face_event_subscribers[i].client_index)
	    {
	      // shift left the remaining items
	      int j;
	      for (j = i; j < sm->n_face_event_subscribers; j++)
		{
		  memcpy (&(sm->face_event_subscribers[j]),
			  &(sm->face_event_subscribers[j + 1]), sizeof (*mp));
		  rv = CICN_VNET_API_ERROR_NONE;
		}
	      sm->n_face_event_subscribers--;
	      break;
	    }
	}
    }

  REPLY_MACRO (VL_API_CICN_API_FACE_EVENTS_SUBSCRIBE_REPLY	/*, rmp, mp, rv */
    );
}

static clib_error_t *
cicn_mgmt_face_add_cli (ip4_address_t local_addr4, int local_port,
			ip4_address_t remote_addr4, int remote_port,
			int app_face, int *faceid)
{
  int rv;

  rv =
    cicn_mgmt_face_add (local_addr4, local_port, remote_addr4, remote_port,
			app_face, faceid);

  switch (rv)
    {
    case 0:
      break;
    case VNET_API_ERROR_NO_SUCH_ENTRY:
      return (clib_error_return (0, "No matching interface"));
      break;

    case VNET_API_ERROR_INVALID_SW_IF_INDEX:
      return
	clib_error_return (0,
			   "Invalid interface, only works on physical ports");
      break;

    case VNET_API_ERROR_UNIMPLEMENTED:
      return clib_error_return (0,
				"Device driver doesn't support redirection");
      break;

    default:
      return clib_error_return (0, "cicn_cfg_face returned %d", rv);
    }

  // send event in different places for cli, api: see api case
  cicn_api_face_event_send (*faceid, CICN_FACE_FLAGS_DEFAULT);
  return 0;
}

/*
 * cli handler for 'cfg face local <addr:port> remote <addr:port>'
 */
static clib_error_t *
cicn_cli_face_set_command_fn (vlib_main_t * vm, unformat_input_t * main_input,
			      vlib_cli_command_t * cmd)
{
  clib_error_t *cl_err = 0;
  cicn_main_t *sm = &cicn_main;
  cicn_face_db_entry_t *face_entry = NULL;
  ip4_address_t local_addr4, remote_addr4;
  int local_port = 0, remote_port = 0;
  int faceid = -1;
  cicn_mgmt_face_op_e face_op = CICN_MGMT_FACE_OP_NONE;
  const char *cfg_admin_str = NULL;
  int cfg_admin_up = 0;
  const char *cfg_hello_str = NULL;
  int cfg_hello_enable = 0;
  int app_face = 0;
  int ret;

  local_addr4.as_u32 = 0;
  remote_addr4.as_u32 = 0;

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "id %d", &faceid))
	{
	  if (unformat (line_input, "delete"))
	    {
	      /* TODO -- handle delete case... */
	      face_op = CICN_MGMT_FACE_OP_DELETE;
	    }
	  else if (unformat (line_input, "admin %s", &cfg_admin_str))
	    {
	      face_op = CICN_MGMT_FACE_OP_ADMIN;
	      if (strcmp (cfg_admin_str, "up") == 0)
		{
		  cfg_admin_up = 1;
		}
	      else if (strcmp (cfg_admin_str, "down") == 0)
		{
		  cfg_admin_up = 0;
		}
	      else
		{
		  return (clib_error_return
			  (0, "Unknown face state %s", cfg_admin_str));
		}
	    }
	  else if (unformat (line_input, "hello %s", &cfg_hello_str))
	    {
	      face_op = CICN_MGMT_FACE_OP_HELLO;
	      if (strcmp (cfg_hello_str, "enable") == 0)
		{
		  cfg_hello_enable = 1;
		}
	      else if (strcmp (cfg_hello_str, "disable") == 0)
		{
		  cfg_hello_enable = 0;
		}
	      else
		{
		  return (clib_error_return
			  (0, "Unknown hello option (%s)", cfg_hello_str));
		}
	    }
	  else
	    {
	      return clib_error_return (0, "Please specify face operation");
	    }
	}
      else if (unformat (line_input, "add"))
	{
	  face_op = CICN_MGMT_FACE_OP_CREATE;
	  if (unformat (line_input, "local %U:%d",
			unformat_ip4_address, &local_addr4, &local_port))
	    {
	      if (unformat (line_input, "remote %U:%d",
			    unformat_ip4_address, &remote_addr4,
			    &remote_port))
		{
		  if (unformat (line_input, "app_face"))
		    {
		      app_face = 1;
		    }
		}
	    }
	}
      else
	{
	  return clib_error_return (0, "Unknown input '%U'",
				    format_unformat_error, line_input);
	  break;
	}
    }

  if (faceid != -1)
    {
      ret = cicn_face_entry_find_by_id (faceid, &face_entry);
      if (ret != AOK)
	{
	  return clib_error_return (0, "faceid %d not valid", faceid);
	}
    }

  switch (face_op)
    {
    case CICN_MGMT_FACE_OP_CREATE:
      /* Check for presence of local address/port */
      if ((local_addr4.as_u32 == 0) || (local_port == 0))
	{
	  return clib_error_return (0, "local address/port not specified");
	}

      /* Check for presence of remote address/port */
      if ((remote_addr4.as_u32 == 0) || (remote_port == 0))
	{
	  return clib_error_return (0, "remote address/port not specified");
	}
      cl_err =
	cicn_mgmt_face_add_cli (local_addr4, local_port, remote_addr4,
				remote_port, app_face, &faceid);
      if (cl_err == 0)
	{
	  vlib_cli_output (sm->vlib_main, "Face ID: %d", faceid);
	}
      else
	{
	  vlib_cli_output (sm->vlib_main, "Face add failed");
	}
      break;
    case CICN_MGMT_FACE_OP_DELETE:
      cl_err = cicn_mgmt_face_remove (faceid);
      break;
    case CICN_MGMT_FACE_OP_ADMIN:
      cicn_face_flags_update (face_entry, !cfg_admin_up,
			      CICN_FACE_FLAG_ADMIN_DOWN);
      break;
    case CICN_MGMT_FACE_OP_HELLO:
      cl_err = cicn_hello_adj_update (faceid, cfg_hello_enable);
      break;
    default:
      return clib_error_return (0, "Operation (%d) not implemented", face_op);
      break;
    }
  return cl_err;
}


/* API message handler */
static void
vl_api_cicn_api_fib_entry_nh_add_t_handler (vl_api_cicn_api_fib_entry_nh_add_t
					    * mp)
{
  vl_api_cicn_api_fib_entry_nh_add_reply_t *rmp;
  vnet_api_error_t rv = CICN_VNET_API_ERROR_NONE;

  cicn_main_t *sm = &cicn_main;

  const char *prefix = (const char *) (&mp->prefix);
  int faceid = clib_net_to_host_i32 (mp->faceid);
  int weight = clib_net_to_host_i32 (mp->weight);

  if ((prefix == NULL) || (strlen (prefix) <= 0)
      || (strlen (prefix) > CICN_PARAM_FIB_ENTRY_PFX_WF_BYTES_MAX) ||
      (faceid <= 0))
    {
      rv = VNET_API_ERROR_INVALID_ARGUMENT;
    }

  if (weight == CICN_API_FIB_ENTRY_NHOP_WGHT_UNSET)
    {
      weight = CICN_PARAM_FIB_ENTRY_NHOP_WGHT_DFLT;
    }
  if ((weight < 0) || (weight > CICN_PARAM_FIB_ENTRY_NHOP_WGHT_MAX))
    {
      rv = VNET_API_ERROR_INVALID_ARGUMENT;
    }

  if (rv == CICN_VNET_API_ERROR_NONE)
    {
      int ux_rc;
      cicn_rd_t cicn_rd;
      ux_rc = cicn_fib_entry_nh_update (prefix, faceid, weight, 1 /*add */ ,
					&cicn_rd);
      if (ux_rc == AOK)
	{
	  CICN_INFRA_CFG_GEN_INCR ();
	}
      switch (cicn_rd.rd_cicn_rc)
	{
	case CICN_RC_OK:
	  rv = cicn_api_rv_from_unix_rc (ux_rc);
	  break;
	case CICN_RC_FIB_PFX_COMP_LIMIT:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_1;
	  break;
	case CICN_RC_FIB_PFX_SIZE_LIMIT:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_2;
	  break;
	case CICN_RC_FIB_NHOP_LIMIT:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_3;
	  break;
	case CICN_RC_FACE_UNKNOWN:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_4;
	  break;
	default:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_10;	// should not happen
	  break;
	}
    }

  REPLY_MACRO (VL_API_CICN_API_FIB_ENTRY_NH_ADD_REPLY /*, rmp, mp, rv */ );
}

/* API message handler */
static void
  vl_api_cicn_api_fib_entry_nh_delete_t_handler
  (vl_api_cicn_api_fib_entry_nh_delete_t * mp)
{
  vl_api_cicn_api_fib_entry_nh_delete_reply_t *rmp;
  vnet_api_error_t rv = CICN_VNET_API_ERROR_NONE;

  cicn_main_t *sm = &cicn_main;

  const char *prefix = (const char *) (&mp->prefix);
  int faceid = clib_net_to_host_i32 (mp->faceid);

  if ((prefix == NULL) || (strlen (prefix) <= 0) ||
      (strlen (prefix) > CICN_PARAM_FIB_ENTRY_PFX_WF_BYTES_MAX))
    {
      rv = VNET_API_ERROR_INVALID_ARGUMENT;
    }

  if (rv == CICN_VNET_API_ERROR_NONE)
    {
      int ux_rc;
      cicn_rd_t cicn_rd;
      ux_rc =
	cicn_fib_entry_nh_update (prefix, faceid, 0 /*dummy */ , 0 /*!add */ ,
				  &cicn_rd);
      if (rv == 0)
	{
	  CICN_INFRA_CFG_GEN_INCR ();
	}
      switch (cicn_rd.rd_cicn_rc)
	{
	case CICN_RC_OK:
	  rv = cicn_api_rv_from_unix_rc (ux_rc);
	  break;
	case CICN_RC_FIB_PFX_COMP_LIMIT:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_1;
	  break;
	case CICN_RC_FIB_PFX_SIZE_LIMIT:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_2;
	  break;
	case CICN_RC_FIB_NHOP_LIMIT:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_3;
	  break;
	case CICN_RC_FACE_UNKNOWN:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_4;
	  break;
	default:
	  rv = VNET_API_ERROR_SYSCALL_ERROR_10;	// should not happen
	  break;
	}
    }

  REPLY_MACRO (VL_API_CICN_API_FIB_ENTRY_NH_DELETE_REPLY /*, rmp, mp, rv */ );
}

/* API message handler */
static void
  vl_api_cicn_api_fib_entry_props_get_t_handler
  (vl_api_cicn_api_fib_entry_props_get_t * mp)
{
  vl_api_cicn_api_fib_entry_props_get_reply_t *rmp;
  vnet_api_error_t rv;
  unix_shared_memory_queue_t *q;

  cicn_main_t *sm = &cicn_main;

  REPLY_SETUP (VL_API_CICN_API_FIB_ENTRY_PROPS_GET_REPLY, rmp, q, mp);

  rv =
    cicn_fib_api_entry_props_serialize (rmp,
					clib_net_to_host_i32 (mp->pagenum));

  REPLY_FINISH (rmp, q, rv);
}

/*
 * cli handler for 'cfg fib'
 */
static clib_error_t *
cicn_cli_fib_set_command_fn (vlib_main_t * vm, unformat_input_t * main_input,
			     vlib_cli_command_t * cmd)
{
  clib_error_t *cl_err = 0;

  int ux_rc;
  cicn_rd_t cicn_rd;

  int addpfx = -1;
  const char *prefix = NULL;
  int faceid = 0;
  int weight = CICN_PARAM_FIB_ENTRY_NHOP_WGHT_DFLT;

  /* TODO -- support next-hop weights */

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (addpfx == -1 && unformat (line_input, "add"))
	{
	  addpfx = 1;
	}
      else if (addpfx == -1 && unformat (line_input, "delete"))
	{
	  addpfx = 0;
	}
      else if (addpfx != -1 && unformat (line_input, "prefix %s", &prefix))
	{
	  ;
	}
      else if (addpfx != -1 && unformat (line_input, "face %d", &faceid))
	{
	  ;
	}
      else if (addpfx == 1 && unformat (line_input, "weight %d", &weight))
	{
	  ;
	}
      else
	{
	  return clib_error_return (0, "Unknown input '%U'",
				    format_unformat_error, line_input);
	}
    }

  /* Check parse */
  if ((prefix == NULL) || (addpfx > 0 && faceid == 0))
    {
      return clib_error_return (0, "Please specify prefix and faceid...");
    }

  if (addpfx &&
      ((weight < 0) || (weight > CICN_PARAM_FIB_ENTRY_NHOP_WGHT_MAX)))
    {
      return clib_error_return (0,
				"Next-hop weight must be between 0 and %d",
				(int) CICN_PARAM_FIB_ENTRY_NHOP_WGHT_MAX);
    }

  ux_rc = cicn_fib_entry_nh_update (prefix, faceid, weight, addpfx, &cicn_rd);
  if (ux_rc == AOK)
    {
      CICN_INFRA_CFG_GEN_INCR ();
    }
  else
    {
      const char *subcode_str = cicn_rd_str (&cicn_rd);
      cl_err =
	clib_error_return (0, "Unable to modify fib: %s (%d)", subcode_str,
			   ux_rc);
    }

  return (cl_err);
}

/*
 * cli handler for 'cicn hello'
 */
static clib_error_t *
cicn_cli_hello_protocol_set_command_fn (vlib_main_t * vm,
					unformat_input_t * main_input,
					vlib_cli_command_t * cmd)
{
  clib_error_t *cl_err = 0;

  cicn_main_t *sm = &cicn_main;
  int interval = -1;

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "interval %d", &interval))
	{
	  ;
	}
      else
	{
	  return (clib_error_return
		  (0, "Unknown input '%U'", format_unformat_error,
		   line_input));
	  break;
	}
    }

  /* Check that hello protocol interval > 0 */
  if (interval > 0)
    {
      sm->cicn_hello_interval = interval / 1000.0;
      sm->cicn_hello_interval_cfgd = 1;
      vlib_cli_output (vm, "Hello protocol interval was set successfully",
		       sm->cicn_hello_interval);
    }
  else
    {
      cl_err =
	clib_error_return (0,
			   "cicn: the hello protocol time interval must be positive");
    }

  return (cl_err);
}

/*
 * cli handler for 'cicn show'
 */
static clib_error_t *
cicn_cli_show_command_fn (vlib_main_t * vm, unformat_input_t * main_input,
			  vlib_cli_command_t * cmd)
{
  int i, face_p = 0, fib_p = 0, all_p, detail_p = 0, internal_p = 0;

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  /* TODO -- support specific args */
  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "face all"))
	{
	  face_p = 1;
	}
      else if (unformat (line_input, "fib all"))
	{
	  fib_p = 1;
	}
      else if (unformat (line_input, "detail"))
	{
	  detail_p = 1;
	}
      else if (unformat (line_input, "internal"))
	{
	  /* We consider 'internal' a superset, so include 'detail' too */
	  internal_p = 1;
	  detail_p = 1;
	}
      else
	{
	  return (clib_error_return
		  (0, "Unknown input '%U'", format_unformat_error,
		   line_input));
	  break;
	}
    }

  /* If nothing specified, show everything */
  if ((face_p == 0) && (fib_p == 0))
    {
      all_p = 1;
    }

  if (!cicn_main.is_enabled)
    {
      if (node_ctl_params.fib_max_size == -1 &&
	  node_ctl_params.pit_max_size == -1 &&
	  node_ctl_params.pit_dflt_lifetime_sec == -1 &&
	  node_ctl_params.pit_min_lifetime_sec == -1 &&
	  node_ctl_params.pit_max_lifetime_sec == -1 &&
	  node_ctl_params.cs_max_size == -1)
	{
	  cicn_cli_output ("cicn: not enabled");
	  goto done;
	}
      vlib_cli_output (vm, "Forwarder %s: %sabled\nPreconfiguration:\n",
		       cicn_infra_fwdr_name.fn_str,
		       cicn_main.is_enabled ? "en" : "dis");

      if (node_ctl_params.fib_max_size != -1)
	{
	  vlib_cli_output (vm, "  FIB:: max entries:%d\n,",
			   node_ctl_params.fib_max_size);
	}
      if (node_ctl_params.pit_max_size != -1)
	{
	  vlib_cli_output (vm, "  PIT:: max entries:%d\n",
			   node_ctl_params.pit_max_size);
	}
      if (node_ctl_params.pit_dflt_lifetime_sec != -1)
	{
	  vlib_cli_output (vm, "  PIT:: dflt lifetime: %05.3f seconds\n",
			   node_ctl_params.pit_dflt_lifetime_sec);
	}
      if (node_ctl_params.pit_min_lifetime_sec != -1)
	{
	  vlib_cli_output (vm, "  PIT:: min lifetime: %05.3f seconds\n",
			   node_ctl_params.pit_min_lifetime_sec);
	}
      if (node_ctl_params.pit_max_lifetime_sec != -1)
	{
	  vlib_cli_output (vm, "  PIT:: max lifetime: %05.3f seconds\n",
			   node_ctl_params.pit_max_lifetime_sec);
	}
      if (node_ctl_params.cs_max_size != -1)
	{
	  vlib_cli_output (vm, "  CS:: max entries:%d\n",
			   node_ctl_params.cs_max_size);
	}

      goto done;
    }

  /* Globals */
  vlib_cli_output (vm,
		   "Forwarder %s: %sabled\n"
		   "  FIB:: max entries:%d\n"
		   "  PIT:: max entries:%d,"
		   " lifetime default: %05.3f sec (min:%05.3f, max:%05.3f)\n"
		   "  CS::  max entries:%d\n",
		   cicn_infra_fwdr_name.fn_str,
		   cicn_main.is_enabled ? "en" : "dis",
		   cicn_main.fib.fib_capacity,
		   cicn_infra_shard_pit_size * cicn_main.shard_count,
		   ((f64) cicn_main.pit_lifetime_dflt_ms) / SEC_MS,
		   ((f64) cicn_main.pit_lifetime_min_ms) / SEC_MS,
		   ((f64) cicn_main.pit_lifetime_max_ms) / SEC_MS,
		   cicn_infra_shard_cs_size * cicn_main.shard_count);

  vl_api_cicn_api_node_stats_get_reply_t rm = { 0, }
  , *rmp = &rm;
  if (cicn_mgmt_node_stats_get (&rm) == AOK)
    {
      vlib_cli_output (vm,	//compare vl_api_cicn_api_node_stats_get_reply_t_handler block
		       "  PIT entries (now): %d\n"
		       "  CS entries (now): %d\n"
		       "  Forwarding statistics:\n"
		       "    pkts_processed: %d\n"
		       "    pkts_interest_count: %d\n"
		       "    pkts_data_count: %d\n"
		       "    pkts_nak_count: %d\n"
		       "    pkts_from_cache_count: %d\n"
		       "    pkts_nacked_interests_count: %d\n"
		       "    pkts_nak_hoplimit_count: %d\n"
		       "    pkts_nak_no_route_count: %d\n"
		       "    pkts_no_pit_count: %d\n"
		       "    pit_expired_count: %d\n"
		       "    cs_expired_count: %d\n"
		       "    cs_lru_count: %d\n"
		       "    pkts_drop_no_buf: %d\n"
		       "    interests_aggregated: %d\n"
		       "    interests_retransmitted: %d\n",
		       clib_net_to_host_u64 (rmp->pit_entries_count),
		       clib_net_to_host_u64 (rmp->cs_entries_count),
		       clib_net_to_host_u64 (rmp->pkts_processed),
		       clib_net_to_host_u64 (rmp->pkts_interest_count),
		       clib_net_to_host_u64 (rmp->pkts_data_count),
		       clib_net_to_host_u64 (rmp->pkts_nak_count),
		       clib_net_to_host_u64 (rmp->pkts_from_cache_count),
		       clib_net_to_host_u64
		       (rmp->pkts_nacked_interests_count),
		       clib_net_to_host_u64 (rmp->pkts_nak_hoplimit_count),
		       clib_net_to_host_u64 (rmp->pkts_nak_no_route_count),
		       clib_net_to_host_u64 (rmp->pkts_no_pit_count),
		       clib_net_to_host_u64 (rmp->pit_expired_count),
		       clib_net_to_host_u64 (rmp->cs_expired_count),
		       clib_net_to_host_u64 (rmp->cs_lru_count),
		       clib_net_to_host_u64 (rmp->pkts_drop_no_buf),
		       clib_net_to_host_u64 (rmp->interests_aggregated),
		       clib_net_to_host_u64 (rmp->interests_retx));
    }

  if (internal_p)
    {
      vlib_cli_output (vm, "cicn: config gen %" PRIu64 "",
		       cicn_infra_gshard.cfg_generation);

      for (i = 0; i <= cicn_main.worker_count; i++)
	{
	  vlib_cli_output (vm, "cicn: worker [%d] gen %" PRIu64 "",
			   i, cicn_infra_shards[i].cfg_generation);
	}
    }

  /* TODO -- Just show all faces */
  if (face_p || all_p)
    {
      cicn_face_show (-1, detail_p, internal_p);
    }

  /* TODO -- just show fib */
  if (fib_p || all_p)
    {
      cicn_fib_show (NULL, detail_p, internal_p);
    }

done:
  if (all_p && internal_p)
    {
      vlib_cli_output (vm,
		       "Plugin features: multithreading:%d, cs:%d, "
		       "clone_replication:%d\n",
		       CICN_FEATURE_MULTITHREAD,
		       CICN_FEATURE_CS,
		       CICN_INFRA_CLONE_REPLICATION);
    }
  return (0);
}

/*
 * cli handler for 'pgen'
 */
static clib_error_t *
cicn_cli_pgen_client_set_command_fn (vlib_main_t * vm,
				     unformat_input_t * main_input,
				     vlib_cli_command_t * cmd)
{
  cicn_main_t *sm = &cicn_main;
  ip4_address_t src_addr, dest_addr;
  int local_port = 0, src_port = 0, dest_port = 0;
  int rv = VNET_API_ERROR_UNIMPLEMENTED;

  if (sm->is_enabled)
    {
      /* That's no good - you only get one or the other */
      return (clib_error_return (0, "Already enabled for forwarding"));
    }

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "port %d", &local_port))
	{
	  ;
	}
      else if (unformat (line_input, "dest %U:%d",
			 unformat_ip4_address, &dest_addr, &dest_port))
	{
	  ;
	}
      else if (unformat (line_input, "src %U:%d",
			 unformat_ip4_address, &src_addr, &src_port))
	{
	  ;
	}
      else
	{
	  return (clib_error_return
		  (0, "Unknown input '%U'", format_unformat_error,
		   line_input));
	  break;
	}
    }

  /* Attach our packet-gen node for ip4 udp local traffic */
  if ((local_port == 0) || (dest_port == 0) || (src_port == 0))
    {
      return clib_error_return (0,
				"Error: must supply local port and rewrite address info");
    }

  udp_register_dst_port (sm->vlib_main, local_port, icn_pg_node.index, 1);

  sm->pgen_clt_src_addr = src_addr.as_u32;
  sm->pgen_clt_src_port = htons ((uint16_t) src_port);
  sm->pgen_clt_dest_addr = dest_addr.as_u32;
  sm->pgen_clt_dest_port = htons ((uint16_t) dest_port);

  sm->pgen_enabled = 1;
  rv = 0;

  switch (rv)
    {
    case 0:
      break;

    case VNET_API_ERROR_UNIMPLEMENTED:
      return clib_error_return (0, "Unimplemented, NYI");
      break;

    default:
      return clib_error_return (0, "cicn enable_disable returned %d", rv);
    }

  return 0;
}

/*
 * cli handler for 'pgen'
 */
static clib_error_t *
cicn_cli_pgen_server_set_command_fn (vlib_main_t * vm,
				     unformat_input_t * main_input,
				     vlib_cli_command_t * cmd)
{
  clib_error_t *cl_err;
  int rv = CICN_VNET_API_ERROR_NONE;

  cicn_main_t *sm = &cicn_main;
  int local_port = 0;
  int payload_size = 0;

  if (sm->is_enabled)
    {
      /* That's no good - you only get one or the other */
      return (clib_error_return (0, "Already enabled for forwarding"));
    }

  /* Get a line of input. */
  unformat_input_t _line_input, *line_input = &_line_input;
  if (!unformat_user (main_input, unformat_line_input, line_input))
    {
      return (0);
    }

  /* Parse the arguments */
  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "port %d", &local_port))
	{
	  ;
	}
      else if (unformat (line_input, "size %d", &payload_size))
	{
	  if (payload_size > 1200)
	    {
	      return (clib_error_return (0,
					 "Payload size must be <= 1200 bytes..."));
	    }
	}
      else
	{
	  return (clib_error_return
		  (0, "Unknown input '%U'", format_unformat_error,
		   line_input));
	  break;
	}
    }

  /* Attach our packet-gen node for ip4 udp local traffic */
  if (local_port == 0 || payload_size == 0)
    {
      return clib_error_return (0,
				"Error: must supply local port and payload size");
    }

  /* Allocate the buffer with the actual content payload TLV */
  vlib_buffer_alloc (vm, &sm->pgen_svr_buffer_idx, 1);
  vlib_buffer_t *rb = NULL;
  rb = vlib_get_buffer (vm, sm->pgen_svr_buffer_idx);

  /* Initialize the buffer data with zeros */
  memset (rb->data, 0, payload_size);
  C_PUTINT16 (rb->data, CICN_TLV_PAYLOAD);
  C_PUTINT16 (rb->data + 2, payload_size - 4);
  rb->current_length = payload_size;

  /* Register the UDP port of the server */
  udp_register_dst_port (sm->vlib_main, local_port,
			 icn_pg_server_node.index, 1);

  sm->pgen_svr_enabled = 1;

  switch (rv)
    {
    case 0:
      cl_err = 0;
      break;

    case VNET_API_ERROR_UNIMPLEMENTED:
      cl_err = clib_error_return (0, "Unimplemented, NYI");
      break;

    default:
      cl_err = clib_error_return (0, "cicn pgen server returned %d", rv);
    }

  return cl_err;
}

/* API message handler */
static void
vl_api_cicn_api_test_run_get_t_handler (vl_api_cicn_api_test_run_get_t * mp)
{
  vl_api_cicn_api_test_run_get_reply_t *rmp;
  cicn_main_t *sm = &cicn_main;
  vnet_api_error_t vaec = CICN_VNET_API_ERROR_NONE;

  unix_shared_memory_queue_t *q =
    vl_api_client_index_to_input_queue (mp->client_index);
  if (!q)
    {
      return;
    }

  rmp = vl_msg_api_alloc (sizeof (*rmp));
  rmp->_vl_msg_id =
    ntohs (sm->msg_id_base + VL_API_CICN_API_TEST_RUN_GET_REPLY);
  rmp->context = mp->context;
  if (sm->test_cicn_api_handler == NULL)
    {
      vaec = VNET_API_ERROR_UNIMPLEMENTED;
    }
  else
    {
      test_cicn_api_op_t test_cicn_api_op = {.reply = rmp };
      vaec = (sm->test_cicn_api_handler) (&test_cicn_api_op);
    }

  rmp->retval = clib_host_to_net_i32 (vaec);

  vl_msg_api_send_shmem (q, (u8 *) & rmp);
}

static void
cicn_cli_test_results_output (vl_api_cicn_api_test_run_get_reply_t * rmp)
{
  u8 *strbuf = NULL;
  i32 nentries = clib_net_to_host_i32 (rmp->nentries);
  cicn_api_test_suite_results_t *suites =
    (cicn_api_test_suite_results_t *) & rmp->suites[0];

  int i;
  for (i = 0; i < nentries; i++)
    {
      cicn_api_test_suite_results_t *suite = &suites[i];
      int ntests = clib_net_to_host_i32 (suite->ntests);
      int nsuccesses = clib_net_to_host_i32 (suite->nsuccesses);
      int nfailures = clib_net_to_host_i32 (suite->nfailures);
      int nskipped = clib_net_to_host_i32 (suite->nskipped);
      int j, cnt;

      strbuf = format (strbuf,
		       "Suite %s:  %d tests: %d successes, %d failures, %d skipped\n",
		       suite->suitename, ntests, nsuccesses, nfailures,
		       nskipped);

      if (nfailures != 0)
	{
	  strbuf = format (strbuf, "  Failed Test(s):");
	  for (j = 0, cnt = 0; j < 8 * sizeof (suite->failures_mask); j++)
	    {
	      if ((suite->failures_mask[j / 8] & (1 << (j % 8))) == 0)
		{
		  continue;
		}
	      cnt++;
	      strbuf =
		format (strbuf, " %d%s", j + 1,
			(cnt < nfailures) ? ", " : " ");
	    }
	  strbuf = format (strbuf, "\n");
	}
      if (nskipped != 0)
	{
	  strbuf = format (strbuf, "  Skipped Test(s):");
	  for (j = 0, cnt = 0; j < 8 * sizeof (suite->skips_mask); j++)
	    {
	      if ((suite->skips_mask[j / 8] & (1 << (j % 8))) == 0)
		{
		  continue;
		}
	      cnt++;
	      strbuf =
		format (strbuf, " %d%s", j + 1,
			(cnt < nskipped) ? ", " : " ");
	    }
	  strbuf = format (strbuf, "\n");
	}
    }

  vec_terminate_c_string (strbuf);
  vlib_cli_output (cicn_main.vlib_main, "%s", strbuf);
  if (strbuf)
    {
      vec_free (strbuf);
    }
}

static clib_error_t *
cicn_cli_test_fn (vlib_main_t * vm, unformat_input_t * main_input,
		  vlib_cli_command_t * cmd)
{
  clib_error_t *cl_err;
  int rv;

  cicn_main_t *sm = &cicn_main;

  if (sm->test_cicn_api_handler == NULL)
    {
      rv = VNET_API_ERROR_UNIMPLEMENTED;
    }
  else
    {
      // leverage API message for CLI
      vl_api_cicn_api_test_run_get_reply_t rmp = { 0, };
      test_cicn_api_op_t test_cicn_api_op = {.reply = &rmp };
      rv = (sm->test_cicn_api_handler) (&test_cicn_api_op);
      cicn_cli_test_results_output (test_cicn_api_op.reply);
    }

  switch (rv)
    {
    case 0:
      cl_err = 0;
      break;

    case VNET_API_ERROR_UNIMPLEMENTED:
      cl_err =
	clib_error_return (0, "Unimplemented, test modules not linked");
      break;

    default:
      cl_err = clib_error_return (0, "cicn pgen server returned %d", rv);
    }

  return cl_err;
}



/* List of message types that this plugin understands */

#define foreach_cicn_plugin_api_msg                       \
_(CICN_API_NODE_PARAMS_SET, cicn_api_node_params_set)     \
_(CICN_API_NODE_PARAMS_GET, cicn_api_node_params_get)     \
_(CICN_API_NODE_STATS_GET, cicn_api_node_stats_get)       \
_(CICN_API_FACE_ADD, cicn_api_face_add)                   \
_(CICN_API_FACE_DELETE, cicn_api_face_delete)             \
_(CICN_API_FACE_PARAMS_GET, cicn_api_face_params_get)     \
_(CICN_API_FACE_PROPS_GET, cicn_api_face_props_get)       \
_(CICN_API_FACE_STATS_GET, cicn_api_face_stats_get)       \
_(CICN_API_FACE_EVENTS_SUBSCRIBE, cicn_api_face_events_subscribe) \
_(CICN_API_FIB_ENTRY_NH_ADD, cicn_api_fib_entry_nh_add)   \
_(CICN_API_FIB_ENTRY_NH_DELETE, cicn_api_fib_entry_nh_delete) \
_(CICN_API_FIB_ENTRY_PROPS_GET, cicn_api_fib_entry_props_get) \
_(CICN_API_TEST_RUN_GET, cicn_api_test_run_get)

/* Set up the API message handling tables */
clib_error_t *
cicn_api_plugin_hookup (vlib_main_t * vm)
{
  cicn_main_t *sm = &cicn_main;

  /* Get a correctly-sized block of API message decode slots */
  u8 *name = format (0, "cicn_%08x%c", api_version, 0);
  sm->msg_id_base = vl_msg_api_get_msg_ids ((char *) name,
					    VL_MSG_FIRST_AVAILABLE);
  vec_free (name);

#define _(N,n)                                                  \
    vl_msg_api_set_handlers(sm->msg_id_base + VL_API_##N,       \
			    #n,					\
			    vl_api_##n##_t_handler,		\
			    vl_noop_handler,			\
			    vl_api_##n##_t_endian,		\
			    vl_api_##n##_t_print,		\
			    sizeof(vl_api_##n##_t), 1);
  foreach_cicn_plugin_api_msg;
#undef _

  int smart_fib_update = 0;
#if CICN_FEATURE_MULTITHREAD	// smart fib update believed working, not tested
  smart_fib_update = 1;
#endif // CICN_FEATURE_MULTITHREAD
  /*
   * Thread-safe API messages
   * i.e. disable thread synchronization
   */
  api_main_t *am = &api_main;
  if (smart_fib_update)
    {
      am->is_mp_safe[sm->msg_id_base + VL_API_CICN_API_FIB_ENTRY_NH_ADD] = 1;
      am->is_mp_safe[sm->msg_id_base + VL_API_CICN_API_FIB_ENTRY_NH_DELETE] =
	1;
    }

  return 0;
}


/* cli declaration for 'control' (root path of multiple commands, for help) */
VLIB_CLI_COMMAND (cicn_cli_node_ctl_command, static) =
{
.path = "cicn control",.short_help = "cicn control"};

/* cli declaration for 'control start' */
VLIB_CLI_COMMAND (cicn_cli_node_ctl_start_set_command, static) =
{
.path = "cicn control start",.short_help = "cicn control start",.function =
    cicn_cli_node_ctl_start_set_command_fn,};

/* cli declaration for 'control stop' */
VLIB_CLI_COMMAND (cicn_cli_node_ctl_stop_set_command, static) =
{
.path = "cicn control stop",.short_help = "cicn control stop",.function =
    cicn_cli_node_ctl_stop_set_command_fn,};

/* cli declaration for 'control param' */
VLIB_CLI_COMMAND (cicn_cli_node_ctl_param_set_command, static) =
{
.path = "cicn control param",.short_help =
    "cicn control param { pit { size <entries> | { dfltlife | minlife | maxlife } <seconds> } | fib size <entries> | cs size <entries> }\n",.function
    = cicn_cli_node_ctl_param_set_command_fn,};

/* cli declaration for 'enable-disable'*/
VLIB_CLI_COMMAND (cicn_cli_node_enable_disable_set_command, static) =
{
.path = "cicn enable-disable",.short_help =
    "cicn enable-disable [disable]",.function =
    cicn_cli_node_enable_disable_set_command_fn,};

/* cli declaration for 'cfg' */
VLIB_CLI_COMMAND (cicn_cli_set_command, static) =
{
.path = "cicn cfg",.short_help = "cicn cfg",};

/* cli declaration for 'cfg name' */
VLIB_CLI_COMMAND (cicn_cli_node_name_set_command, static) =
{
.path = "cicn cfg name",.short_help =
    "cicn cfg name <name> [delete]",.function =
    cicn_cli_node_name_set_command_fn,.long_help =
    "Add (or remove) an administrative name for this router\n" "\n"
    "Multiple names are allowed. (NYI...)\n",};

/* cli declaration for 'cfg salt' */
VLIB_CLI_COMMAND (cicn_cli_salt_set_command, static) =
{
.path = "cicn cfg salt",.short_help = "cicn cfg salt <number>",.function =
    cicn_cli_salt_set_command_fn,};

/* cli declaration for 'cfg face' */
VLIB_CLI_COMMAND (cicn_cli_face_set_command, static) =
{
.path = "cicn cfg face",.short_help =
    "cicn cfg face { add local <addr:port> remote <addr:port> | "
    "id <id> { delete | admin { down | up } | hello { enable | disable } }",.function
    = cicn_cli_face_set_command_fn,};

/* cli declaration for 'cfg fib' */
VLIB_CLI_COMMAND (cicn_cli_fib_set_command, static) =
{
.path = "cicn cfg fib",.short_help =
    "cicn cfg fib {add | delete } prefix <prefix> face <faceid> "
    "[weight <weight>]",.function = cicn_cli_fib_set_command_fn,};

/* cli declaration for 'cfg hello-protocol' */
VLIB_CLI_COMMAND (cicn_cli_hello_protocol_set_command, static) =
{
.path = "cicn cfg hello-protocol",.short_help =
    "cicn cfg hello-protocol interval <num_of_mseconds>",.function =
    cicn_cli_hello_protocol_set_command_fn,};

/* cli declaration for 'show' */
VLIB_CLI_COMMAND (cicn_cli_show_command, static) =
{
.path = "cicn show",.short_help =
    "cicn show [face ['all' | faceid]] "
    "[fib ['all' | prefix]] "
    "[detail] [internal]",.function = cicn_cli_show_command_fn,};

/* cli declaration for 'cicn pgen client' */
VLIB_CLI_COMMAND (cicn_cli_pgen_client_set_command, static) =
{
.path = "cicn pgen client",.short_help =
    "cicn pgen client port <port> src <addr:port> dest <addr:port>",.long_help
    = "Run icn in packet-gen client mode\n",.function =
    cicn_cli_pgen_client_set_command_fn,};

/* cli declaration for 'cicn pgen server' */
VLIB_CLI_COMMAND (cicn_cli_pgen_server_set_command, static) =
{
.path = "cicn pgen server",.short_help =
    "cicn pgen server port <port> size <content_payload_size>",.long_help =
    "Run icn in packet-gen server mode\n",.function =
    cicn_cli_pgen_server_set_command_fn,};

/* cli declaration for 'test' */
VLIB_CLI_COMMAND (cicn_cli_test_command, static) =
{
.path = "cicn test",.short_help = "cicn test",.function = cicn_cli_test_fn,};
