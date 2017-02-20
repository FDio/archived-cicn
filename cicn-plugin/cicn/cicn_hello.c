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
 * cicn_hello.c - ICN hello protocol operation
 */

#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>

#include <vlibapi/api.h>
#include <vlibmemory/api.h>
#include <vlibsocket/api.h>

#include <vnet/ip/udp.h>

#include <cicn/cicn.h>
#include <cicn/cicn_hello_inlines.h>

static vlib_node_registration_t icn_hello_process_node;

/* Stats string values */
static char *icnhelloprocess_error_strings[] = {
#define _(sym,string) string,
  foreach_icnhelloprocess_error
#undef _
};

/*
 * When face is created/hello enabled, fill in adjacency information
 */
clib_error_t *
cicn_hello_adj_update (i32 faceid, int enable)
{
  clib_error_t *rv = 0;

  int ret;
  cicn_main_t *sm = &cicn_main;
  cicn_hello_name_t *hello_name = &sm->hello_name;
  cicn_face_db_entry_t *face;
  cicn_hello_adj_t *hello_adj;
  cicn_hello_fcd_t *fcd;
  struct sockaddr_in *addr;

  ret = cicn_face_entry_find_by_id (faceid, &face);
  if (ret != AOK)
    {
      rv = clib_error_return (0, "face id %d not found", faceid);
      goto done;
    }
  if (face->app_face)
    {
      rv =
	clib_error_return (0,
			   "face id %d is app face, hello protocol disallowed",
			   faceid);
      goto done;
    }

  /* Set the cicn_hello_adj struct values */

  hello_adj = &sm->cicn_hello_adjs[faceid];

  if (enable)
    {
      if (hello_adj->active)
	{
	  rv =
	    clib_error_return (0, "face id %d hello protocol already enabled",
			       faceid);
	  goto done;
	}

      hello_adj->ha_swif = face->swif;

      clib_memcpy (face->fe_ha_name_cmn, &hello_name->hn_wf[0],
		   CICN_HELLO_NAME_CMN_FLEN);

      fcd = &face->fe_ha_fcd_loc;
      addr = &face->src_addr;
      memset (&fcd->fcd_v[0], 0, sizeof (fcd->fcd_v));
      clib_memcpy (&fcd->fcd_v[0], &addr->sin_addr.s_addr, sizeof (u32));
      clib_memcpy (&fcd->fcd_v[sizeof (u32)], &addr->sin_port, sizeof (u16));
      fcd->fcd_v_len = CICN_HELLO_NAME_FACEID_V_LEN;

      // for now, assume nbr's faceid vs. receiving in iMsg notification
      fcd = &face->fe_ha_fcd_nbr;
      addr = &face->dest_addr;
      memset (&fcd->fcd_v[0], 0, sizeof (fcd->fcd_v));
      clib_memcpy (&fcd->fcd_v[0], &addr->sin_addr.s_addr, sizeof (u32));
      clib_memcpy (&fcd->fcd_v[sizeof (u32)], &addr->sin_port, sizeof (u16));
      fcd->fcd_v_len = CICN_HELLO_NAME_FACEID_V_LEN;

      hello_adj->active = 1;

      /* Increment the number of active adjacencies */
      sm->n_active_hello_adjs++;
    }
  else
    {
      if (!hello_adj->active)
	{
	  rv =
	    clib_error_return (0,
			       "face id %d hello protocol already disabled",
			       faceid);
	  goto done;
	}

      hello_adj->active = 0;
      hello_adj->ha_swif = 0;

      fcd = &face->fe_ha_fcd_loc;
      memset (fcd, 0, sizeof (*fcd));

      fcd = &face->fe_ha_fcd_nbr;
      memset (fcd, 0, sizeof (*fcd));

      /* Decrement the number of active adjacencies */
      sm->n_active_hello_adjs--;
    }

  cicn_face_flags_update (face, enable, CICN_FACE_FLAG_HELLO_DOWN);

done:
  return (rv);
}

/*
 * Every hello period, create a hello packet for a peer, to be sent out,
 * using buffer for buf_idx
 */
static void
cicn_hello_packet_build (u32 bi0, cicn_hello_adj_t * hello_adj,
			 cicn_face_db_entry_t * face)
{
  cicn_main_t *sm = &cicn_main;
  vlib_main_t *vm = sm->vlib_main;

  vlib_buffer_t *b0 = vlib_get_buffer (vm, bi0);
  vnet_buffer (b0)->sw_if_index[VLIB_RX] = hello_adj->ha_swif;	//TODO: correct?
  vnet_buffer (b0)->sw_if_index[VLIB_TX] = ~0;

  /* Increment the last sent seq num (i.e. first sent is 1, not 0) */
  hello_adj->last_sent_seq_num++;

  u32 icn_name_len = CICN_TLV_HDR_LEN + sm->hello_name.hn_wf_v_len;

  u32 icn_len = sizeof (cicn_packet_hdr_t) + CICN_TLV_HDR_LEN + icn_name_len;

  /* Zero all the way through the icn packet header, not ICN message */
  u8 *ptr0 = vlib_buffer_get_current (b0);
  memset (ptr0, 0, sizeof (ip4_header_t) + sizeof (udp_header_t) +
	  sizeof (cicn_packet_hdr_t));

  /* Build IP header in place */
  ip4_header_t *ip0 = (ip4_header_t *) ptr0;
  b0->current_length = sizeof (ip4_header_t);
  ASSERT ((((uintptr_t) ip0) & 0x3) == 0);	// assert alignment for assigns below

  ip0->ip_version_and_header_length = 0x45;
  ip0->ttl = 128;
  ip0->protocol = IP_PROTOCOL_UDP;
  ip0->src_address.as_u32 = face->src_addr.sin_addr.s_addr;
  ip0->dst_address.as_u32 = face->dest_addr.sin_addr.s_addr;
  ip0->length =
    clib_host_to_net_u16 (sizeof (ip4_header_t) + sizeof (udp_header_t) +
			  icn_len);
  ip0->checksum = ip4_header_checksum (ip0);

  /* Build UDP header in place */
  udp_header_t *udp0 = (udp_header_t *) (ip0 + 1);
  b0->current_length += sizeof (udp_header_t);

  udp0->src_port = face->src_addr.sin_port;
  udp0->dst_port = face->dest_addr.sin_port;
  udp0->checksum = 0x0000;
  udp0->length = clib_host_to_net_u16 (sizeof (udp_header_t) + icn_len);

  /* Build ICN header */
  cicn_packet_hdr_t *h = (cicn_packet_hdr_t *) (udp0 + 1);
  b0->current_length += icn_len;

  h->pkt_ver = CICN_PROTO_VERSION_CURRENT;
  h->pkt_type = CICN_PKT_TYPE_CONTROL_REQUEST;
  h->pkt_hop_limit = CICN_DEFAULT_HOP_LIMIT;
  h->pkt_flags = 0;
  h->pkt_hdr_len = sizeof (cicn_packet_hdr_t);
  C_PUTINT16 (&h->pkt_len, icn_len);

  /* The message type and length (currently just the name tlv) */
  uint8_t *msg_tlv_p = (uint8_t *) (h + 1);
  C_PUTINT16 (&msg_tlv_p[0], CICN_MSG_TYPE_INTEREST);
  C_PUTINT16 (&msg_tlv_p[CICN_TLV_TYPE_LEN], CICN_HELLO_NAME_TOT_FLEN);

  /* Copy name tlv, updating adjacency and seq_number components */
  uint8_t *name_tlv_p = &msg_tlv_p[CICN_TLV_HDR_LEN];
  u8 *fid_tlv_p = &name_tlv_p[CICN_HELLO_NAME_CMN_FLEN];
  u8 *seq_tlv_p = &fid_tlv_p[CICN_HELLO_NAME_FACEID_FLEN];

  clib_memcpy (name_tlv_p, face->fe_ha_name_cmn, CICN_HELLO_NAME_CMN_FLEN);

  cicn_parse_tlv_build (fid_tlv_p, CICN_NAME_COMP,
			CICN_HELLO_NAME_FACEID_V_LEN,
			&face->fe_ha_fcd_loc.fcd_v[0]);

  cicn_parse_tlv_hdr_build (seq_tlv_p, CICN_NAME_COMP,
			    CICN_HELLO_NAME_SEQ_V_LEN);
  C_PUTINT64 (&seq_tlv_p[CICN_TLV_HDR_LEN], hello_adj->last_sent_seq_num);
}

/*
 * At period expiry, walk through all adjacencies, building and sending
 * hello packets. Return number of hello packets sent.
 */
u32
cicn_hello_periodic (vlib_main_t * vm, vlib_node_runtime_t * node)
{
  cicn_main_t *sm = &cicn_main;
  vlib_frame_t *f;
  u32 *to_next;
  u32 bi0;
  u32 active_adjs_found = 0;
  int j = 0;
  u64 seq_num_gap;
  cicn_face_db_entry_t *face_entry;

  /* If no active adjacencies, don't walk array */
  if (sm->n_active_hello_adjs == 0)
    {
      return (0);
    }

  /* Get a frame */
  f = vlib_get_frame_to_node (vm, sm->cicn_hello_next_node_id);
  ASSERT (f->n_vectors == 0);
  to_next = vlib_frame_vector_args (f);

  for (j = 0; j < CICN_PARAM_FACES_MAX; j++)
    {
      /* If we have found all the adjs, break */
      if (active_adjs_found >= sm->n_active_hello_adjs)
	{
	  break;
	}

      /* If this adj is not active, continue */
      if (!sm->cicn_hello_adjs[j].active)
	{
	  continue;
	}
      if (cicn_face_entry_find_by_id (j, &face_entry) != AOK)
	{
	  continue;
	}

      active_adjs_found++;

      /* Find the gap between the last sent and last acked seq num */
      seq_num_gap = sm->cicn_hello_adjs[j].last_sent_seq_num -
	sm->cicn_hello_adjs[j].last_received_seq_num;
      /* If we go above the threshold, mark the interface as down */
      if (seq_num_gap >= CICN_PARAM_HELLO_MISSES_DOWN_DFLT)
	{
	  face_entry->flags |= CICN_FACE_FLAG_HELLO_DOWN;
	}
      vlib_buffer_alloc (vm, &bi0, 1);

      /* Create the icn hello packet in bi0 */
      cicn_hello_packet_build (bi0, &sm->cicn_hello_adjs[j], face_entry);

      cicn_infra_shard_t *wshard = &cicn_infra_shards[vm->cpu_index];
      cicn_face_stats_t *outface_stats =
	&wshard->face_stats[cicn_face_db_index (face_entry)];
      outface_stats->orig_interests++;
      outface_stats->out_interests++;

      /* Move the buffers to the frame */
      to_next[0] = bi0;
      to_next++;
      f->n_vectors++;
    }

  /* Dispatch the frame to the node */
  vlib_put_frame_to_node (vm, sm->cicn_hello_next_node_id, f);
  return (active_adjs_found);
}

/*
 * At cicn enable time, initialize hello's periodic state
 * - sm->cicn_hello_next_node_id
 * - sm->hello_name (string, wire-format, and initial (2-component) hash
 */
int
cicn_hello_plugin_activation_init (vlib_main_t * vm)
{
  cicn_main_t *sm = &cicn_main;

  /* Up/Down next node id */
  vlib_node_t *next_node = vlib_get_node_by_name (vm, (u8 *) "ip4-lookup");
  sm->cicn_hello_next_node_id = next_node->index;

  /* Set the values of the ICN hello name struct */
  cicn_hello_name_t *hello_name = &sm->hello_name;
  cicn_sstrncpy (hello_name->hn_str, CICN_HELLO_NAME_TEMPLATE,
		 sizeof (hello_name->hn_str));

  cicn_rd_t cicn_rd;
  C_PUTINT16 (&hello_name->hn_wf[0], CICN_TLV_NAME);
  hello_name->hn_wf_v_len =
    cicn_parse_name_comps_from_str (&hello_name->hn_wf[CICN_TLV_HDR_LEN],
				    sizeof (hello_name->hn_wf) -
				    CICN_TLV_HDR_LEN, hello_name->hn_str,
				    &cicn_rd);
  if (hello_name->hn_wf_v_len != CICN_HELLO_NAME_TOT_FLEN - CICN_TLV_HDR_LEN)
    {
      vlib_cli_output (sm->vlib_main,
		       "Error parsing hello name template: %s (%d)",
		       cicn_rd_str (&cicn_rd), hello_name->hn_wf_v_len);
      return EINVAL;
    }
  C_PUTINT16 (&hello_name->hn_wf[CICN_TLV_TYPE_LEN], hello_name->hn_wf_v_len);

  return (AOK);
}

/*
 * The entry-point for the ICN adjacency process, which periodically
 * sends adjacency packets.
 */
static uword
icn_hello_process_fn (vlib_main_t * vm,
		      vlib_node_runtime_t * rt, vlib_frame_t * f)
{
  cicn_main_t *sm = &cicn_main;
  f64 up_down_time_remaining;
  uword event_type;
  cicn_hello_data *d;
  uword *event_data = 0;
  int i = 0;
  cicn_face_db_entry_t *face_entry;

  up_down_time_remaining = sm->cicn_hello_interval;

  /* Loop forever */
  while (1)
    {
      up_down_time_remaining = vlib_process_wait_for_event_or_clock (vm,
								     up_down_time_remaining);
      /* Get the events (if any) */
      event_type = vlib_process_get_events (vm, &event_data);
      if (!sm->is_enabled)
	{
	  ASSERT (vec_len (event_data) == 0);
	  up_down_time_remaining = sm->cicn_hello_interval;
	  continue;
	}

      switch (event_type)
	{
	case ~0:
	  break;
	case CICN_HELLO_EVENT_DATA_RCVD:
	  for (i = 0; i < vec_len (event_data); i++)
	    {
	      /* We got a hello Data packet */
	      d = (cicn_hello_data *) event_data[i];
	      if (sm->cicn_hello_adjs[d->faceid].last_received_seq_num <
		  d->seq_num)
		{
		  sm->cicn_hello_adjs[d->faceid].last_received_seq_num =
		    d->seq_num;
		  /* Find the face and, if down, bring it up */
		  if (cicn_face_entry_find_by_id (d->faceid, &face_entry) !=
		      AOK)
		    {
		      continue;
		    }
		  if (face_entry->flags & CICN_FACE_FLAG_HELLO_DOWN)
		    {
		      cicn_face_flags_update (face_entry, 0 /*!set */ ,
					      CICN_FACE_FLAG_HELLO_DOWN);
		    }
		}
	    }
	default:
	  ;
	}

      vec_reset_length (event_data);

      /* peer timeout scan, send up-down Interest */
      if (vlib_process_suspend_time_is_zero (up_down_time_remaining))
	{
	  u32 adjs_sent = cicn_hello_periodic (vm, rt);
	  vlib_node_increment_counter (vm, icn_hello_process_node.index,
				       ICNHELLOPROCESS_ERROR_HELLO_INTERESTS_SENT,
				       adjs_sent);

	  up_down_time_remaining = sm->cicn_hello_interval;
	}
    }

  /* NOTREACHED */
  return 0;
}

clib_error_t *
cicn_hello_boot_init (vlib_main_t * vm)
{
  cicn_main_t *sm = &cicn_main;

  sm->n_active_hello_adjs = 0;
  sm->cicn_hello_interval_cfgd = 0;
  sm->cicn_hello_interval = CICN_PARAM_HELLO_POLL_INTERVAL_DFLT;

  return (0);
}


VLIB_REGISTER_NODE (icn_hello_process_node, static) =
{
.function = icn_hello_process_fn,.type = VLIB_NODE_TYPE_PROCESS,.name =
    "icn-hello-process",.process_log2_n_stack_bytes = 16,.n_errors =
    ARRAY_LEN (icnhelloprocess_error_strings),.error_strings =
    icnhelloprocess_error_strings,};

VLIB_INIT_FUNCTION (cicn_hello_boot_init);
