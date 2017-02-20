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
 * cicn.c - skeleton vpp-api-test plug-in
 */

#include <vat/vat.h>
#include <vlibapi/api.h>
#include <vlibmemory/api.h>
#include <vlibsocket/api.h>
#include <vppinfra/error.h>
#include <vnet/ip/udp.h>
#include <cicn/cicn_api.h>

uword unformat_sw_if_index (unformat_input_t * input, va_list * args);

/* Declare message IDs */
#include <cicn/cicn_msg_enum.h>

/* declare message handlers for each api */

#define vl_endianfun		/* define message structures */
#include <cicn/cicn_all_api_h.h>
#undef vl_endianfun

/* instantiate all the print functions we know about */
#define vl_print(handle, ...)
#define vl_printfun
#include <cicn/cicn_all_api_h.h>
#undef vl_printfun

/* Get the API version number. */
#define vl_api_version(n,v) static u32 api_version=(v);
#include <cicn/cicn_all_api_h.h>
#undef vl_api_version

/* copied from vpe.api.h */
#define VL_API_PACKED(x) x __attribute__ ((packed))

/* copied from vpe.api.h */
typedef VL_API_PACKED (struct _vl_api_control_ping
		       {
		       u16 _vl_msg_id; u32 client_index;
		       u32 context;
		       }) vl_api_control_ping_t;

typedef struct
{
  /* API message ID base */
  u16 msg_id_base;
  vat_main_t *vat_main;
} cicn_test_main_t;

cicn_test_main_t cicn_test_main;

#define foreach_standard_reply_retval_handler            \
_(cicn_api_node_params_set_reply)                        \
_(cicn_api_fib_entry_nh_add_reply)                       \
_(cicn_api_fib_entry_nh_delete_reply)                    \
_(cicn_api_face_delete_reply)                            \
_(cicn_api_face_events_subscribe_reply)

#define _(n)                                            \
    static void vl_api_##n##_t_handler                  \
    (vl_api_##n##_t * mp)                               \
    {                                                   \
        vat_main_t * vam = cicn_test_main.vat_main;     \
        i32 retval = ntohl(mp->retval);                 \
        if (vam->async_mode) {                          \
            vam->async_errors += (retval < 0);          \
        } else {                                        \
            vam->retval = retval;                       \
            vam->result_ready = 1;                      \
        }                                               \
    }
foreach_standard_reply_retval_handler;
#undef _

/*
 * Table of message reply handlers, must include boilerplate handlers
 * we just generated
 */
#define foreach_vpe_api_reply_msg                                   \
_(CICN_API_NODE_PARAMS_SET_REPLY, cicn_api_node_params_set_reply)   \
_(CICN_API_NODE_PARAMS_GET_REPLY, cicn_api_node_params_get_reply)   \
_(CICN_API_NODE_STATS_GET_REPLY, cicn_api_node_stats_get_reply)     \
_(CICN_API_FACE_ADD_REPLY, cicn_api_face_add_reply)                 \
_(CICN_API_FACE_DELETE_REPLY, cicn_api_face_delete_reply)           \
_(CICN_API_FACE_PARAMS_GET_REPLY, cicn_api_face_params_get_reply)   \
_(CICN_API_FIB_ENTRY_NH_ADD_REPLY, cicn_api_fib_entry_nh_add_reply) \
_(CICN_API_FIB_ENTRY_NH_DELETE_REPLY, cicn_api_fib_entry_nh_delete_reply) \
_(CICN_API_FACE_PROPS_GET_REPLY, cicn_api_face_props_get_reply)     \
_(CICN_API_FACE_STATS_GET_REPLY, cicn_api_face_stats_get_reply)     \
_(CICN_API_FIB_ENTRY_PROPS_GET_REPLY, cicn_api_fib_entry_props_get_reply) \
_(CICN_API_FIB_DETAILS, cicn_api_fib_details)                       \
_(CICN_API_TEST_RUN_GET_REPLY, cicn_api_test_run_get_reply)         \
_(CICN_API_FACE_EVENTS_SUBSCRIBE_REPLY, cicn_api_face_events_subscribe_reply) \
_(CICN_API_FACE_EVENT, cicn_api_face_event)

/* M: construct, but don't yet send a message */

#define M(T,t)                                                  \
do {                                                            \
    vam->result_ready = 0;                                      \
    mp = vl_msg_api_alloc(sizeof(*mp));                         \
    memset (mp, 0, sizeof (*mp));                               \
    mp->_vl_msg_id = ntohs (VL_API_##T + sm->msg_id_base);      \
    mp->client_index = vam->my_client_index;                    \
} while(0);

#define M2(T,t,n)                                               \
do {                                                            \
    vam->result_ready = 0;                                      \
    mp = vl_msg_api_alloc(sizeof(*mp)+(n));                     \
    memset (mp, 0, sizeof (*mp));                               \
    mp->_vl_msg_id = ntohs (VL_API_##T + sm->msg_id_base);      \
    mp->client_index = vam->my_client_index;                    \
} while(0);

/* S: send a message */
#define S (vl_msg_api_send_shmem (vam->vl_input_queue, (u8 *)&mp))

/* W: wait for results, with timeout */
#define W                                       \
do {                                            \
    timeout = vat_time_now (vam) + 1.0;         \
                                                \
    while (vat_time_now (vam) < timeout) {      \
        if (vam->result_ready == 1) {           \
            return (vam->retval);               \
        }                                       \
    }                                           \
    return -99;                                 \
} while(0);

static int
api_cicn_api_node_params_set (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *input = vam->input;
  f64 timeout;
  int enable_disable = 1;
  int pit_size = -1, fib_size = -1, cs_size = -1;
  f64 pit_dflt_lifetime_sec = -1.0f;
  f64 pit_min_lifetime_sec = -1.0f, pit_max_lifetime_sec = -1.0f;

  vl_api_cicn_api_node_params_set_t *mp;

  /* Parse args required to build the message */
  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "disable"))
	{
	  enable_disable = 0;
	}
      else if (unformat (input, "PIT size %d", &pit_size))
	{
	  ;
	}
      else if (unformat (input, "FIB size %d", &fib_size))
	{
	  ;
	}
      else if (unformat (input, "CS size %d", &cs_size))
	{
	  ;
	}
      else if (unformat (input, "PIT dfltlife %f", &pit_dflt_lifetime_sec))
	{
	  ;
	}
      else if (unformat (input, "PIT minlife %f", &pit_min_lifetime_sec))
	{
	  ;
	}
      else if (unformat (input, "PIT maxlife %f", &pit_max_lifetime_sec))
	{
	  ;
	}
      else
	{
	  break;
	}
    }

  /* Construct the API message */
  M (CICN_API_NODE_PARAMS_SET, cicn_api_node_params_set);
  mp->enable_disable = enable_disable;
  mp->pit_max_size = clib_host_to_net_i32 (pit_size);
  mp->fib_max_size = clib_host_to_net_i32 (fib_size);
  mp->cs_max_size = clib_host_to_net_i32 (cs_size);
  //TODO: implement clib_host_to_net_f64 in VPP ?
  mp->pit_dflt_lifetime_sec = pit_dflt_lifetime_sec;
  mp->pit_min_lifetime_sec = pit_min_lifetime_sec;
  mp->pit_max_lifetime_sec = pit_max_lifetime_sec;

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static int
api_cicn_api_node_params_get (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  f64 timeout;
  vl_api_cicn_api_node_params_get_t *mp;

  // Construct the API message
  M (CICN_API_NODE_PARAMS_GET, cicn_api_node_params_get);

  S;
  W;
}

static void
  vl_api_cicn_api_node_params_get_reply_t_handler
  (vl_api_cicn_api_node_params_get_reply_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = ntohl (mp->retval);

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      return;
    }

  vam->retval = retval;
  vam->result_ready = 1;

  if (vam->retval < 0)
    {
      // vpp_api_test infra will also print out string form of error
      fformat (vam->ofp, "   (API call error: %d)\n", vam->retval);
      return;
    }

  fformat (vam->ofp,
	   "Enabled %d\n"
	   "  Features: multithreading:%d, cs:%d, dpdk-cloning:%d, "
	   "vlib-cloning:%d\n",
	   "  Workers %d, FIB size %d PIT size %d\n"
	   "  PIT lifetime dflt %.3f, min %.3f, max %.3f\n"
	   "  CS size %d\n",
	   mp->is_enabled,
	   mp->feature_multithread,
	   mp->feature_cs,
	   mp->feature_dpdk_rtembuf_cloning,
	   mp->feature_vpp_vlib_cloning,
	   clib_net_to_host_u32 (mp->worker_count),
	   clib_net_to_host_u32 (mp->fib_max_size),
	   clib_net_to_host_u32 (mp->pit_max_size),
	   //TODO: implement clib_net_to_host_f64 in VPP ?
	   mp->pit_dflt_lifetime_sec,
	   mp->pit_min_lifetime_sec,
	   mp->pit_max_lifetime_sec, clib_net_to_host_u32 (mp->cs_max_size));
}


static int
api_cicn_api_face_add (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *input = vam->input;
  f64 timeout;
  ip4_address_t local_addr4, remote_addr4;
  int local_port = 0, remote_port = 0;
  vl_api_cicn_api_face_add_t *mp;

  local_addr4.as_u32 = 0;
  remote_addr4.as_u32 = 0;

  /* Parse args required to build the message */
  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "local %U:%d",
		    unformat_ip4_address, &local_addr4, &local_port))
	{
	  ;
	}
      else if (unformat (input, "remote %U:%d",
			 unformat_ip4_address, &remote_addr4, &remote_port))
	{
	  ;
	}
      else
	{
	  break;
	}
    }

  /* Check for presence of both addresses */
  if ((local_addr4.as_u32 == 0) || (remote_addr4.as_u32 == 0))
    {
      clib_warning ("Please specify both local and remote addresses...");
      return (1);
    }

  /* Check for presence of both addresses */
  if ((local_port == 0) || (remote_port == 0))
    {
      clib_warning ("Please specify both local and remote ports...");
      return (1);
    }

  /* Construct the API message */
  M (CICN_API_FACE_ADD, cicn_api_face_add);
  mp->local_addr = clib_host_to_net_u32 (local_addr4.as_u32);
  mp->local_port = clib_host_to_net_u16 ((u16) local_port);
  mp->remote_addr = clib_host_to_net_u32 (remote_addr4.as_u32);
  mp->remote_port = clib_host_to_net_u16 ((u16) remote_port);

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static void
vl_api_cicn_api_face_add_reply_t_handler (vl_api_cicn_api_face_add_reply_t *
					  mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = ntohl (mp->retval);

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      return;
    }

  vam->retval = retval;
  vam->result_ready = 1;

  if (vam->retval < 0)
    {
      // vpp_api_test infra will also print out string form of error
      fformat (vam->ofp, "   (API call error: %d)\n", vam->retval);
      return;
    }

  fformat (vam->ofp, "New Face ID: %d\n", ntohl (mp->faceid));
}

static int
api_cicn_api_face_delete (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *input = vam->input;
  f64 timeout;
  int faceid = 0;
  vl_api_cicn_api_face_delete_t *mp;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "face %d", &faceid))
	{
	  ;
	}
      else
	{
	  break;
	}
    }

  // Check for presence of face ID
  if (faceid == 0)
    {
      clib_warning ("Please specify face ID");
      return 1;
    }

  // Construct the API message
  M (CICN_API_FACE_DELETE, cicn_api_face_delete);
  mp->faceid = clib_host_to_net_i32 (faceid);

  // send it...
  S;

  // Wait for a reply...
  W;
}

static int
api_cicn_api_face_params_get (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *input = vam->input;
  f64 timeout;
  int faceid = 0;
  vl_api_cicn_api_face_params_get_t *mp;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "face %d", &faceid))
	{
	  ;
	}
      else
	{
	  break;
	}
    }

  // Check for presence of face ID
  if (faceid == 0)
    {
      clib_warning ("Please specify face ID");
      return 1;
    }

  // Construct the API message
  M (CICN_API_FACE_PARAMS_GET, cicn_api_face_params_get);
  mp->faceid = clib_host_to_net_i32 (faceid);

  // send it...
  S;

  // Wait for a reply...
  W;
}

static void
  vl_api_cicn_api_face_params_get_reply_t_handler
  (vl_api_cicn_api_face_params_get_reply_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = ntohl (mp->retval);
  u8 *sbuf = 0, *dbuf = 0;

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      return;
    }

  vam->retval = retval;
  vam->result_ready = 1;

  if (vam->retval < 0)
    {
      // vpp_api_test infra will also print out string form of error
      fformat (vam->ofp, "   (API call error: %d)\n", vam->retval);
      return;
    }

  u32 local_addr = clib_net_to_host_u32 (mp->local_addr);
  vec_reset_length (sbuf);
  sbuf = format (sbuf, "%U", format_ip4_address, &local_addr);

  u32 remote_addr = clib_net_to_host_u32 (mp->remote_addr);
  vec_reset_length (dbuf);
  dbuf = format (dbuf, "%U", format_ip4_address, &remote_addr);

  fformat (vam->ofp, "%s:%d <-> %s:%d swif %d flags %d\n",
	   sbuf,
	   clib_net_to_host_u16 (mp->local_port),
	   dbuf,
	   clib_net_to_host_u16 (mp->remote_port),
	   clib_net_to_host_i32 (mp->sw_interface_id),
	   clib_net_to_host_i32 (mp->flags));
}

static int
api_cicn_api_fib_entry_nh_add (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *input = vam->input;
  f64 timeout;
  vl_api_cicn_api_fib_entry_nh_add_t *mp;

  const char *prefix = NULL;
  int faceid = 0;
  int weight = CICN_API_FIB_ENTRY_NHOP_WGHT_UNSET;

  /* TODO -- support next-hop weights */

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "prefix %s", &prefix))
	{
	  ;
	}
      else if (unformat (input, "face %d", &faceid))
	{
	  ;
	}
      else if (unformat (input, "weight %d", &weight))
	{
	  ;
	}
      else
	{
	  break;
	}
    }

  /* Check parse */
  if ((prefix == NULL) || (strlen (prefix) == 0) || (faceid == 0))
    {
      clib_warning ("Please specify prefix and faceid...");
      return 1;
    }

  /* Construct the API message */
  M (CICN_API_FIB_ENTRY_NH_ADD, cicn_api_fib_entry_nh_add);
  memcpy (mp->prefix, prefix, strlen (prefix));
  mp->faceid = clib_host_to_net_i32 (faceid);
  mp->weight = clib_host_to_net_i32 (weight);

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static int
api_cicn_api_fib_entry_nh_delete (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *input = vam->input;
  f64 timeout;
  vl_api_cicn_api_fib_entry_nh_delete_t *mp;

  const char *prefix = NULL;
  int faceid = 0;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "prefix %s", &prefix))
	{
	  ;
	}
      else if (unformat (input, "face %d", &faceid))
	{
	  ;
	}
      else
	{
	  break;
	}
    }

  /* Check parse */
  if ((prefix == NULL) || (strlen (prefix) == 0))
    {
      clib_warning ("Please specify prefix");
      return 1;
    }

  /* Construct the API message */
  M (CICN_API_FIB_ENTRY_NH_DELETE, cicn_api_fib_entry_nh_delete);
  memcpy (mp->prefix, prefix, strlen (prefix));
  mp->faceid = clib_host_to_net_i32 (faceid);

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static int
api_cicn_api_face_props_get (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  f64 timeout;
  vl_api_cicn_api_face_props_get_t *mp;

  /* Construct the API message */
  M (CICN_API_FACE_PROPS_GET, cicn_api_face_props_get);

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static void
  vl_api_cicn_api_face_props_get_reply_t_handler
  (vl_api_cicn_api_face_props_get_reply_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = ntohl (mp->retval);
  u8 *sbuf = 0, *dbuf = 0;

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      return;
    }

  vam->retval = retval;
  vam->result_ready = 1;

  if (vam->retval < 0)
    {
      // vpp_api_test infra will also print out string form of error
      fformat (vam->ofp, "   (API call error: %d)\n", vam->retval);
      return;
    }

  i32 nentries = clib_net_to_host_i32 (mp->nentries);

  cicn_api_face_entry_t *faces = (cicn_api_face_entry_t *) & mp->face[0];
  int i;
  for (i = 0; i < nentries; i++)
    {
      cicn_api_face_entry_t *face = &faces[i];

      u32 local_addr = clib_net_to_host_u32 (face->local_addr);
      vec_reset_length (sbuf);
      sbuf = format (sbuf, "%U", format_ip4_address, &local_addr);

      u32 remote_addr = clib_net_to_host_u32 (face->remote_addr);
      vec_reset_length (dbuf);
      dbuf = format (dbuf, "%U", format_ip4_address, &remote_addr);

      fformat (vam->ofp,
	       "Face %d:  %s:%d <-> %s:%d swif %d flags %d, fib_nhs:%d\n",
	       clib_net_to_host_i32 (face->faceid),
	       sbuf,
	       clib_net_to_host_u16 (face->local_port),
	       dbuf,
	       clib_net_to_host_u16 (face->remote_port),
	       clib_net_to_host_i32 (face->sw_interface_id),
	       clib_net_to_host_i32 (face->flags),
	       clib_net_to_host_u32 (face->fib_nhs));
    }
}

static int
api_cicn_api_face_stats_get (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *input = vam->input;
  f64 timeout;
  vl_api_cicn_api_face_stats_get_t *mp;
  int faceid = 0;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "face %d", &faceid))
	{
	  ;
	}
      else
	{
	  break;
	}
    }

  // Check for presence of face ID
  if (faceid == 0)
    {
      clib_warning ("Please specify face ID");
      return 1;
    }

  /* Construct the API message */
  M (CICN_API_FACE_STATS_GET, cicn_api_face_stats_get);
  mp->faceid = clib_host_to_net_i32 (faceid);

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static void
  vl_api_cicn_api_face_stats_get_reply_t_handler
  (vl_api_cicn_api_face_stats_get_reply_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = ntohl (mp->retval);

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      return;
    }

  vam->retval = retval;
  vam->result_ready = 1;

  if (vam->retval < 0)
    {
      // vpp_api_test infra will also print out string form of error
      fformat (vam->ofp, "   (API call error: %d)\n", vam->retval);
      return;
    }

  fformat (vam->ofp,
	   "Face %d "
	   "orig_interests %d orig_datas %d orig_naks %d "
	   "term_interests %d term_datas %d term_naks %d "
	   "in_interests %d in_datas %d in_naks %d "
	   "out_interests %d out_datas %d out_naks %d\n",
	   clib_net_to_host_i32 (mp->faceid),
	   clib_net_to_host_u64 (mp->orig_interests),
	   clib_net_to_host_u64 (mp->orig_datas),
	   clib_net_to_host_u64 (mp->orig_naks),
	   clib_net_to_host_u64 (mp->term_interests),
	   clib_net_to_host_u64 (mp->term_datas),
	   clib_net_to_host_u64 (mp->term_naks),
	   clib_net_to_host_u64 (mp->in_interests),
	   clib_net_to_host_u64 (mp->in_datas),
	   clib_net_to_host_u64 (mp->in_naks),
	   clib_net_to_host_u64 (mp->out_interests),
	   clib_net_to_host_u64 (mp->out_datas),
	   clib_net_to_host_u64 (mp->out_naks));
}

static int
api_cicn_api_node_stats_get (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  f64 timeout;
  vl_api_cicn_api_node_stats_get_t *mp;

  /* Construct the API message */
  M (CICN_API_NODE_STATS_GET, cicn_api_node_stats_get);

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static void
  vl_api_cicn_api_node_stats_get_reply_t_handler
  (vl_api_cicn_api_node_stats_get_reply_t * rmp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = ntohl (rmp->retval);

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      return;
    }

  vam->retval = retval;
  vam->result_ready = 1;

  if (vam->retval < 0)
    {
      // vpp_api_test infra will also print out string form of error
      fformat (vam->ofp, "   (API call error: %d)\n", vam->retval);
      return;
    }
  else
    {
      fformat (vam->ofp,	// compare cicn_cli_show_command_fn block: should match
	       "  PIT entries (now): %d\n"
	       "  CS entries (now): %d\n"
	       "  Forwarding statistics:"
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
	       clib_net_to_host_u64 (rmp->pkts_nacked_interests_count),
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
}

static int
api_cicn_api_fib_entry_props_get (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  f64 timeout;
  vl_api_cicn_api_fib_entry_props_get_t *mp;

  /* Construct the API message */
  M (CICN_API_FIB_ENTRY_PROPS_GET, cicn_api_fib_entry_props_get);
  mp->pagenum = 0;

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static void
  vl_api_cicn_api_fib_entry_props_get_reply_t_handler
  (vl_api_cicn_api_fib_entry_props_get_reply_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = ntohl (mp->retval);

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      return;
    }

  vam->retval = retval;
  vam->result_ready = 1;

  if (vam->retval < 0)
    {
      // vpp_api_test infra will also print out string form of error
      fformat (vam->ofp, "   (API call error: %d)\n", vam->retval);
      return;
    }

  i32 nentries = clib_net_to_host_i32 (mp->nentries);

  fformat (vam->ofp, "Entries %d\n", nentries);

  cicn_api_fib_entry_t *entries = (cicn_api_fib_entry_t *) & mp->fibentry[0];

  int i;
  for (i = 0; i < nentries; i++)
    {
      cicn_api_fib_entry_t *entry = &entries[i];

      fformat (vam->ofp, "%s:", entry->prefix);

      int j;
      for (j = 0; j < clib_net_to_host_i32 (entry->nfaces); j++)
	{
	  fformat (vam->ofp, " (face: %d, wght %d)",
		   clib_net_to_host_i32 (entry->faceid[j]),
		   clib_net_to_host_i32 (entry->faceweight[j]));
	}

      fformat (vam->ofp, "\n");
    }
}

static void
vl_api_cicn_api_fib_details_t_handler (vl_api_cicn_api_fib_details_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;

  fformat (vam->ofp, "%s:", mp->prefix);

  int j;
  for (j = 0; j < clib_net_to_host_i32 (mp->nfaces); j++)
    {
      fformat (vam->ofp, " (face: %d, wght %d)",
	       clib_net_to_host_i32 (mp->faceid[j]),
	       clib_net_to_host_i32 (mp->faceweight[j]));
    }
  fformat (vam->ofp, "\n");
}

static int
api_cicn_api_test_run_get (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  f64 timeout;
  vl_api_cicn_api_test_run_get_t *mp;

  /* Construct the API message */
  M (CICN_API_TEST_RUN_GET, cicn_api_test_run_get);

  /* send it... */
  S;

  /* Wait for a reply... */
  W;
}

static void
  vl_api_cicn_api_test_run_get_reply_t_handler
  (vl_api_cicn_api_test_run_get_reply_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;
  i32 retval = clib_net_to_host_i32 (mp->retval);

  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
      goto done;
    }

  vam->retval = retval;
  vam->result_ready = 1;
  if (vam->retval < 0)
    {
      goto done;
    }

  i32 nentries = clib_net_to_host_i32 (mp->nentries);
  cicn_api_test_suite_results_t *suites =
    (cicn_api_test_suite_results_t *) & mp->suites[0];

  int i;
  for (i = 0; i < nentries; i++)
    {
      cicn_api_test_suite_results_t *suite = &suites[i];
      int ntests = clib_net_to_host_i32 (suite->ntests);
      int nsuccesses = clib_net_to_host_i32 (suite->nsuccesses);
      int nfailures = clib_net_to_host_i32 (suite->nfailures);
      int nskipped = clib_net_to_host_i32 (suite->nskipped);
      int j, cnt;

      fformat (vam->ofp,
	       "Suite %s:  %d tests: %d successes, %d failures, %d skipped\n",
	       suite->suitename, ntests, nsuccesses, nfailures, nskipped);

      if (nfailures != 0)
	{
	  fformat (vam->ofp, "  Failed Test(s):");
	  for (j = 0, cnt = 0; j < 8 * sizeof (suite->failures_mask); j++)
	    {
	      if ((suite->failures_mask[j / 8] & (1 << (j % 8))) == 0)
		{
		  continue;
		}
	      cnt++;
	      fformat (vam->ofp, " %d%s", j + 1,
		       (cnt < nfailures) ? ", " : " ");
	    }
	  fformat (vam->ofp, "\n");
	}
      if (nskipped != 0)
	{
	  fformat (vam->ofp, "  Skipped Test(s):");
	  for (j = 0, cnt = 0; j < 8 * sizeof (suite->skips_mask); j++)
	    {
	      if ((suite->skips_mask[j / 8] & (1 << (j % 8))) == 0)
		{
		  continue;
		}
	      cnt++;
	      fformat (vam->ofp, " %d%s", j + 1,
		       (cnt < nskipped) ? ", " : " ");
	    }
	  fformat (vam->ofp, "\n");
	}
    }

done:;				// ";" meets requirement for statement after label
}

static int
api_cicn_api_face_events_subscribe (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  unformat_input_t *i = vam->input;
  vl_api_cicn_api_face_events_subscribe_t *mp;
  f64 timeout;
  int enable = -1;

  while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (i, "enable"))
	enable = 1;
      else if (unformat (i, "disable"))
	enable = 0;
      else
	break;
    }

  if (enable == -1)
    {
      errmsg ("missing enable|disable\n");
      return -99;
    }

  M (CICN_API_FACE_EVENTS_SUBSCRIBE, cicn_api_face_events_subscribe);
  mp->enable_disable = clib_host_to_net_u16 (enable);
  mp->context = clib_host_to_net_u32 (10101 /*random number */ );

  S;
  W;
}

static void
vl_api_cicn_api_face_event_t_handler (vl_api_cicn_api_face_event_t * mp)
{
  vat_main_t *vam = cicn_test_main.vat_main;

  fformat (vam->ofp, "Event Face %d Flags %d\n",
	   clib_net_to_host_i32 (mp->faceid),
	   clib_net_to_host_i32 (mp->flags));
}

/*
 * List of messages that the api test plugin sends,
 * and that the data plane plugin processes
 */
#define foreach_vpe_api_msg \
_(cicn_api_node_params_set, "FIB size <sz> PIT size <sz> CS size <sz>"   \
"PIT minlimit <f> PIT maxlimit <f> [disable] ")                         \
_(cicn_api_node_params_get, "")                                         \
_(cicn_api_node_stats_get, "")                                          \
_(cicn_api_face_add, "local <IPv4-addr:port> remote <IPv4-addr:port>")  \
_(cicn_api_face_delete, "face <faceID>")                                \
_(cicn_api_face_stats_get, "face <faceID>")                             \
_(cicn_api_face_params_get, "face <faceID>")                            \
_(cicn_api_face_props_get, "")                                          \
_(cicn_api_fib_entry_nh_add, "prefix </prefix> face <faceID> weight <weight>")\
_(cicn_api_fib_entry_nh_delete, "prefix </prefix>")                     \
_(cicn_api_fib_entry_props_get, "")                                     \
_(cicn_api_face_events_subscribe, "enable|disable")                     \
_(cicn_api_test_run_get, "testsuite <ID>")

void
vat_api_hookup (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  /* Hook up handlers for replies from the data plane plug-in */
#define _(N,n)                                                  \
    vl_msg_api_set_handlers((VL_API_##N + sm->msg_id_base),     \
                           #n,                                  \
                           vl_api_##n##_t_handler,              \
                           vl_noop_handler,                     \
                           vl_api_##n##_t_endian,               \
                           vl_api_##n##_t_print,                \
                           sizeof(vl_api_##n##_t), 1);
  foreach_vpe_api_reply_msg;
#undef _

  /* API messages we can send */
#define _(n,h) hash_set_mem (vam->function_by_name, #n, api_##n);
  foreach_vpe_api_msg;
#undef _

  /* Help strings */
#define _(n,h) hash_set_mem (vam->help_by_name, #n, h);
  foreach_vpe_api_msg;
#undef _
}

clib_error_t *
vat_plugin_register (vat_main_t * vam)
{
  cicn_test_main_t *sm = &cicn_test_main;
  u8 *name;

  sm->vat_main = vam;

  /* Ask the vpp engine for the first assigned message-id */
  name = format (0, "cicn_%08x%c", api_version, 0);
  sm->msg_id_base = vl_client_get_first_plugin_msg_id ((char *) name);

  if (sm->msg_id_base != (u16) ~ 0)
    vat_api_hookup (vam);

  vec_free (name);

  return 0;
}
