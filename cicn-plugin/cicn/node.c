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
 * node.c - icn plug-in nodes for vpp
 */

#include <vlib/vlib.h>
#include <vnet/vnet.h>

#include <cicn/cicn.h>
#include <cicn/cicn_hashtb.h>
#include <cicn/cicn_pcs.h>
#include <cicn/cicn_infra_inlines.h>
#include <cicn/cicn_hello_inlines.h>

int cicn_buftrc = 0;		// make permanent or delete? Set to 1 to enable trace

#define CICN_IP_TTL_DEFAULT  128

/*
 * First forwarder worker node starts here
 */

/* Trace context struct */
typedef struct
{
  u32 next_index;
  u32 sw_if_index;
  u8 pkt_type;
  u16 msg_type;
} icnfwd_trace_t;

/* packet trace format function */
static u8 *
icnfwd_format_trace (u8 * s, va_list * args)
{
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  icnfwd_trace_t *t = va_arg (*args, icnfwd_trace_t *);

  s = format (s, "ICNFWD: pkt: %d, msg %d, sw_if_index %d, next index %d",
	      (int) t->pkt_type, (int) t->msg_type,
	      t->sw_if_index, t->next_index);
  return (s);
}

/*
 * Node context data; we think this is per-thread/instance
 */
typedef struct icnfwd_runtime_s
{
  /* TODO -- use this more when we add shards */
  int id;
  cicn_pit_cs_t pitcs;
} icnfwd_runtime_t;

/* Registration struct for a graph node */
vlib_node_registration_t icnfwd_node;

/* Next graph nodes, which reference the list in the actual registration
 * block below
 */
typedef enum
{
  ICNFWD_NEXT_LOOKUP,
  ICNFWD_NEXT_ERROR_DROP,
  ICNFWD_N_NEXT,
} icnfwd_next_t;

/* Stats string values */
static char *icnfwd_error_strings[] = {
#define _(sym,string) string,
  foreach_icnfwd_error
#undef _
};

/* Prototypes */
static int cicn_trim_cs_lru (vlib_main_t * vm, vlib_node_runtime_t * node,
			     cicn_pit_cs_t * pit);

/*
 *
 */
static void
update_node_counter (vlib_main_t * vm, u32 node_idx, u32 counter_idx, u64 val)
{
  vlib_node_t *node = vlib_get_node (vm, node_idx);
  vlib_error_main_t *em = &(vm->error_main);
  u32 base_idx = node->error_heap_index;

  em->counters[base_idx + counter_idx] = val;
}

/*
 * Prepare a packet buffer for the CS. We'll clone this mbuf and use a
 * newly-allocated mbuf to hold the header/rewrite info needed to send
 * each packet out.
 */
static int
prep_buffer_for_cs (vlib_main_t * vm, vlib_buffer_t * b0)
{
  int ret = EINVAL;

  /* Update the CS mbuf (and vlib buffer) so that it includes only the
   * ICN payload
   */

  /* Advance the vlib buffer to the beginning of the ICN payload */
  vlib_buffer_advance (b0, sizeof (ip4_header_t) + sizeof (udp_header_t));

  ret = AOK;

  return (ret);
}

/*
 * Clone a packet being referenced in a CS entry, using another packet
 * (received interest packet) as a header to hold content response
 * rewrite info and pointer to cloned cs entry buffer.
 */
static int
cicn_clone_cs_buffer (vlib_buffer_t * hdr_b0, const cicn_pcs_entry_t * pcs,
		      vlib_main_t * vm, vlib_buffer_free_list_t * fl,
		      cicn_face_db_entry_t * outface)
{
  int ret = EINVAL;
  vlib_buffer_t *cs_b0;

  BUFTRC ("CS-H-SW", GBI (vm, hdr_b0));
  if (PREDICT_FALSE (pcs->u.cs.cs_pkt_buf == 0))
    {
      goto done;
    }
  BUFTRC ("CS-H-CS", pcs->u.cs.cs_pkt_buf);

  cs_b0 = vlib_get_buffer (vm, pcs->u.cs.cs_pkt_buf);

  /* At this point, the base CS buffer is pointing at the ICN payload
   * part of the packet, and we'll be using the other buffer
   * to hold the egress/tx rewrite info.
   */
  hdr_b0->current_data = 0;
  hdr_b0->current_length = sizeof (ip4_header_t) + sizeof (udp_header_t);

  vlib_buffer_attach_clone (vm, hdr_b0, cs_b0);

  /* Looks like success */
  ret = AOK;

done:

  return (ret);
}

/*
 * ICN forwarder node: handling of Interests and Content Msgs delivered
 * based on udp_register_dst_port().
 * - 1 packet at a time
 * - ipv4 udp only
 */
static uword
icnfwd_node_fn (vlib_main_t * vm,
		vlib_node_runtime_t * node, vlib_frame_t * frame)
{
  u32 n_left_from, *from, *to_next;
  icnfwd_next_t next_index;
  u32 pkts_processed = 0;
  u32 pkts_interest_count = 0, pkts_data_count = 0, pkts_nak_count = 0;
  u32 pkts_control_request_count = 0, pkts_control_reply_count = 0;
  u32 pkts_from_cache_count = 0;
  u32 pkts_nacked_interests_count = 0;
  u32 pkts_nak_hoplimit_count = 0, pkts_nak_no_route_count = 0;
  u32 pkts_no_pit_count = 0, pit_expired_count = 0, cs_expired_count = 0;
  u32 no_bufs_count = 0, pkts_interest_agg = 0, pkts_int_retrans = 0;
  u32 pit_int_count, pit_cs_count;
  u32 pkts_hello_int_rec = 0, pkts_hello_data_sent = 0;
  u32 pkts_hello_data_rec = 0;
  int i, ret;
  icnfwd_runtime_t *rt;
  cicn_prefix_hashinf_t pfxhash;
  f64 tnow;
  vlib_buffer_free_list_t *fl;
  cicn_main_t *sm = &cicn_main;

  fl = vlib_buffer_get_free_list (vm, VLIB_BUFFER_DEFAULT_FREE_LIST_INDEX);

  rt = vlib_node_get_runtime_data (vm, icnfwd_node.index);

  /* Alloc the pit/cs for each shard when the icn feature
   * is enabled, access by thread in the node context.
   */
  if (rt->pitcs.pcs_table == NULL)
    {
      cicn_pit_create (&rt->pitcs, cicn_infra_shard_pit_size);
      cicn_pit_set_lru_max (&rt->pitcs, cicn_infra_shard_cs_size);
    }

  /* Maybe update our thread's config generation number, if the global
   * number has changed
   */
  if (cicn_infra_gshard.cfg_generation !=
      cicn_infra_shards[vm->cpu_index].cfg_generation)
    {
      cicn_infra_shards[vm->cpu_index].cfg_generation =
	cicn_infra_gshard.cfg_generation;
    }

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;
  next_index = node->cached_next_index;

  /* Capture time in vpp terms */
  tnow = vlib_time_now (vm);

  while (n_left_from > 0)
    {
      u32 n_left_to_next;

      vlib_get_next_frame (vm, node, next_index, to_next, n_left_to_next);

      /* TODO -- just doing 1-at-a-time for now, to simplify things a bit. */

      /* TODO -- more interesting stats and trace */

      while (n_left_from > 0 && n_left_to_next > 0)
	{
	  u32 bi0;
	  vlib_buffer_t *b0;
	  u32 next0 = ICNFWD_NEXT_LOOKUP;
	  u32 sw_if_index0;
	  udp_header_t *udp0;
	  ip4_header_t *ip0;
	  u8 *body0;
	  u32 len0;
	  u8 pkt_type;
	  u16 msg_type, bkt_ent_exp_time, sval;
	  cicn_pkt_hdr_desc_t pkt_hdr_desc0;
	  u8 *nameptr;
	  u32 namelen;
	  u64 hashval;
	  struct sockaddr_in srcaddr, destaddr;
	  cicn_face_db_entry_t *inface, *outface;
	  cicn_face_stats_t *inface_stats, *outface_stats;
	  cicn_hash_node_t *nodep;
	  cicn_pcs_entry_t *pitp;
	  cicn_fib_entry_t *pentry;
	  uint16_t faceid;
	  int clone_count;
	  vlib_buffer_t *hdr_vec[CICN_PARAM_PIT_ENTRY_PHOPS_MAX];
	  vlib_buffer_t *cs_b0;
	  cicn_face_db_entry_t *face_vec[CICN_PARAM_PIT_ENTRY_PHOPS_MAX];
	  u64 seq_num;
	  int trace_p = 0;

	  /* Prefetch for next iteration. */
	  if (n_left_from > 1)
	    {
	      vlib_buffer_t *p2;

	      p2 = vlib_get_buffer (vm, from[1]);

	      vlib_prefetch_buffer_header (p2, LOAD);

	      CLIB_PREFETCH (p2->data, (CLIB_CACHE_LINE_BYTES * 2), STORE);
	    }

	  /* TODO -- we're not dealing with 'chained' buffers yet */

	  /* Dequeue a packet buffer */
	  bi0 = from[0];
	  BUFTRC ("CICN-SW", bi0);
	  from += 1;
	  n_left_from -= 1;

	  b0 = vlib_get_buffer (vm, bi0);

	  if (PREDICT_FALSE ((node->flags & VLIB_NODE_FLAG_TRACE) &&
			     (b0->flags & VLIB_BUFFER_IS_TRACED)))
	    {
	      trace_p = 1;
	    }

	  /*
	   * From the udp code, we think we're handed the payload part
	   * of the packet
	   */
	  ASSERT (b0->current_data >=
		  (sizeof (ip4_header_t) + sizeof (udp_header_t)));

	  /* Capture pointer to the payload */
	  body0 = vlib_buffer_get_current (b0);
	  len0 = b0->current_length;

	  /* Walk 'back' to the ip header */
	  vlib_buffer_advance (b0, -(sizeof (udp_header_t)));
	  udp0 = vlib_buffer_get_current (b0);
	  vlib_buffer_advance (b0, -(sizeof (ip4_header_t)));
	  ip0 = vlib_buffer_get_current (b0);

	  sw_if_index0 = vnet_buffer (b0)->sw_if_index[VLIB_RX];

	  /*
	   * Do a quick, in-place parse/validate pass, locating
	   * a couple of key pieces of info about the packet
	   * TODO -- could we pass this info from the dist node?
	   */
	  ret = cicn_parse_pkt (body0, len0, &pkt_type, &msg_type,
				&nameptr, &namelen, &pkt_hdr_desc0);

	  if (ret != AOK)
	    {
	      /* Just drop on error? */
	      pkt_type = 0;
	      msg_type = 0;
	      next0 = ICNFWD_NEXT_ERROR_DROP;
	      goto trace_single;
	    }

	  /* Use result to determine next steps: forward, reply from CS,
	   * drop, nack
	   */

	  if (pkt_type == CICN_PKT_TYPE_INTEREST)
	    {
	      pkts_interest_count++;
	    }
	  else if (pkt_type == CICN_PKT_TYPE_CONTENT)
	    {
	      pkts_data_count++;
	    }
	  else if (pkt_type == CICN_PKT_TYPE_NAK)
	    {
	      pkts_nak_count++;
	    }
	  else if (pkt_type == CICN_PKT_TYPE_CONTROL_REQUEST)
	    {
	      pkts_control_request_count++;
	    }
	  else if (pkt_type == CICN_PKT_TYPE_CONTROL_REPLY)
	    {
	      pkts_control_reply_count++;
	    }

	  /* Locate ingress face */
	  srcaddr.sin_addr.s_addr = ip0->src_address.as_u32;
	  srcaddr.sin_port = udp0->src_port;

	  destaddr.sin_addr.s_addr = ip0->dst_address.as_u32;
	  destaddr.sin_port = udp0->dst_port;

	  /* Search for a match where the _local_ and _remote_ addresses
	   * correspond to the _dest_ and _src_ addresses from the packet.
	   */
	  ret = cicn_face_entry_find_by_addr (&destaddr /*local */ ,
					      &srcaddr /*remote */ , &inface);

	  /* If no matching face, don't do any more */
	  if (PREDICT_FALSE (ret != AOK || inface == NULL ||
			     (inface->flags & CICN_FACE_FLAGS_DOWN_HARD)))
	    {
	      /* Just drop on error? */
	      next0 = ICNFWD_NEXT_ERROR_DROP;
	      goto trace_single;
	    }

	  cicn_infra_shard_t *wshard = &cicn_infra_shards[vm->cpu_index];
	  inface_stats = &wshard->face_stats[cicn_face_db_index (inface)];

	  /* If content, use PIT info to determine egress face */

	  if (pkt_type == CICN_PKT_TYPE_CONTENT ||
	      pkt_type == CICN_PKT_TYPE_CONTROL_REPLY)
	    {

	      inface_stats->in_datas++;

	      if (PREDICT_FALSE (inface->flags & CICN_FACE_FLAG_HELLO_DOWN))
		{
		  // hello down, only hello messages should be processed
		  goto hello_reply_rcvd_check;
		}

	      /* Compute the full name hash for content lookup */
	      hashval = cicn_hashtb_hash_name (nameptr, namelen);

	      /*
	       * Search PIT/CS by name hash
	       */
	      /*
	       * Opportunistic scan of hash row/bucket for expirations.
	       * Some old code below could be removed with this addition,
	       * it won't be executed anyway.
	       *
	       * The timeout scan and the node lookup could be
	       * easily integrated.
	       */
	      cicn_pcs_timeout (vm, &rt->pitcs, hashval,
				&pit_expired_count, &cs_expired_count);

	      ret =
		cicn_hashtb_lookup_node (rt->pitcs.pcs_table, nameptr,
					 namelen, hashval, &nodep);

	      if (PREDICT_FALSE (ret != AOK))
		{
		  goto hello_reply_rcvd_check;	//no PIT entry: maybe a hello?
		}

	      pitp = cicn_pit_get_data (nodep);

	      if (PREDICT_FALSE (pitp->shared.entry_type != CICN_PIT_TYPE))
		{
		  /* Whatever this is, it's not a PIT */
		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      /* Is the PIT entry expired? */
	      if (PREDICT_FALSE (tnow > pitp->shared.expire_time))
		{
		  /*
		   * Remove existing entry; treat this as if no PIT entry
		   */
		  cicn_pcs_delete (&rt->pitcs, &pitp, &nodep, vm);
		  pit_expired_count++;

		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      /* Content should arrive on face where interest tx happened */
	      if (PREDICT_FALSE (pitp->u.pit.pe_txface != inface->faceid))
		{
		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      /*
	       * Hold the packet buffer in the CS, and
	       * then use it to satisfy the PIT entry. We allocate unique
	       * packet mbufs to hold the rewrite info for each reply we'll
	       * send; the rewrite mbufs all share clones of the reply
	       * payload.
	       */

	      /* Prepare the packet for cs and for cloning. */
	      BUFTRC ("CS--ADD", bi0);
	      ret = prep_buffer_for_cs (vm, b0);
	      if (PREDICT_FALSE (ret != AOK))
		{

		  cicn_pcs_delete (&rt->pitcs, &pitp, &nodep, vm);
		  no_bufs_count++;

		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      /* For each packet we will send, allocate a new packet
	       * buffer to hold the rewrite/header info and a clone of
	       * the ICN payload packet buf. We also capture the tx faceid.
	       */
	      ret = AOK;
	      cs_b0 = b0;
	      for (clone_count = 0, i = 0; i < CICN_PARAM_PIT_ENTRY_PHOPS_MAX;
		   i++)
		{

		  if (pitp->u.pit.pe_rxfaces[i] != 0)
		    {

		      ret =
			cicn_face_entry_find_by_id (pitp->u.pit.pe_rxfaces[i],
						    &outface);
		      if (PREDICT_FALSE
			  ((ret != AOK)
			   || (outface->flags & CICN_FACE_FLAGS_DOWN)))
			{
			  /* Can't use this face, skip the entry */
			  ret = AOK;
			  continue;
			}

		      face_vec[clone_count] = outface;
		      hdr_vec[clone_count] =
			cicn_infra_vlib_buffer_alloc (vm);
		      BUFTRC ("CLN-HDR", GBI (vm, hdr_vec[clone_count]));

		      if (PREDICT_FALSE (hdr_vec[clone_count] == NULL))
			{
			  /* Need to check this index in the arrays in
			   * the error-handling code below.
			   */
			  clone_count++;

			  ret = ENOMEM;
			  break;
			}

		      clone_count++;
		    }
		}

	      /* If error, clean up any buffers we allocated */
	      if (PREDICT_FALSE (ret != AOK))
		{
		  for (i = 0; i < clone_count; i++)
		    {
		      BUFTRC ("ERR-FRE",
			      vlib_get_buffer_index (vm, hdr_vec[i]));
		      if (hdr_vec[i])
			{
			  cicn_infra_vlib_buffer_free (hdr_vec[i], vm);
			}
		    }

		  /* Drop */
		  cicn_pcs_delete (&rt->pitcs, &pitp, &nodep, vm);
		  no_bufs_count++;

		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      /* No valid PIT faces found? Not much we can do.
	       * TODO -- for now, leaving the PIT entry; should we delete it?
	       */
	      if (PREDICT_FALSE (clone_count == 0))
		{
		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      // no cs entry for ctrl responses
	      if (pkt_type == CICN_PKT_TYPE_CONTENT)
		{

		  if (cicn_cs_enabled (&rt->pitcs))
		    {
		      /* At this point we think we're safe to proceed.
		       * Store the CS buf in the PIT/CS hashtable entry
		       */

		      /* Start turning the PIT into a CS. Note that we may be
		       * stepping on the PIT part of the union as we
		       * update the CS part, so don't expect the PIT part
		       * to be valid after this point.
		       */
		      cicn_pit_to_cs (&rt->pitcs, pitp);

		      pitp->u.cs.cs_rxface = inface->faceid;
		      pitp->shared.create_time = tnow;

		      uint64_t dmsg_lifetime;
		      ret =
			cicn_parse_hdr_time_ms (body0, &pkt_hdr_desc0,
						CICN_HDR_TLV_CACHE_TIME,
						&dmsg_lifetime);
		      if (ret != AOK)
			{	// no header timeout, use default
			  dmsg_lifetime = CICN_PARAM_CS_LIFETIME_DFLT;
			}
		      else if (dmsg_lifetime != 0)
			{
			  if (dmsg_lifetime < CICN_PARAM_CS_LIFETIME_MIN)
			    {
			      dmsg_lifetime = CICN_PARAM_CS_LIFETIME_MIN;
			    }
			  else if (dmsg_lifetime > CICN_PARAM_CS_LIFETIME_MAX)
			    {
			      dmsg_lifetime = CICN_PARAM_CS_LIFETIME_MAX;
			    }
			}
		      pitp->shared.expire_time =
			cicn_pcs_get_exp_time (tnow, dmsg_lifetime);

		      /* Update hashtable-level expiration value too */
		      bkt_ent_exp_time =
			cicn_infra_get_slow_exp_time (dmsg_lifetime);

		      cicn_hashtb_entry_set_expiration (rt->pitcs.pcs_table,
							nodep,
							bkt_ent_exp_time, 0);

		      /* Store the original packet buffer in the CS node */
		      pitp->u.cs.cs_pkt_buf = vlib_get_buffer_index (vm, b0);

		      /* Add to CS LRU */
		      cicn_cs_lru_insert (&rt->pitcs, nodep, pitp);
		    }
		  else
		    {
		      cicn_pcs_delete (&rt->pitcs, &pitp, &nodep, vm);
		    }

		  /* Set up to enqueue frames to the transmit next-node */
		  if (next_index != ICNFWD_NEXT_LOOKUP)
		    {
		      vlib_put_next_frame (vm, node, next_index,
					   n_left_to_next);
		      next_index = next0 = ICNFWD_NEXT_LOOKUP;
		      vlib_get_next_frame (vm, node, next_index, to_next,
					   n_left_to_next);

		      /* Ensure that we have space for at least one packet */
		      if (PREDICT_FALSE (n_left_to_next <= 0))
			{
			  vlib_put_next_frame (vm, node, next_index,
					       n_left_to_next);

			  vlib_get_next_frame (vm, node, next_index, to_next,
					       n_left_to_next);
			}
		    }

		  ASSERT (n_left_to_next > 0);

		  /* Connect each header buffer to a clone
		   * of the payload buffer. The last packet will go through
		   * to the normal end of the node loop.
		   */
		  for (i = 0; i < clone_count; i++)
		    {

		      b0 = hdr_vec[i];
		      outface = face_vec[i];

		      if (PREDICT_FALSE (trace_p != 0))
			{
			  b0->flags |= VLIB_BUFFER_IS_TRACED;
			}

		      bi0 = vlib_get_buffer_index (vm, b0);

		      b0->current_data = 0;
		      b0->current_length = (sizeof (ip4_header_t) +
					    sizeof (udp_header_t));
		      vlib_buffer_attach_clone (vm, b0, cs_b0);

		      /* Refresh the ip and udp headers
		       * before the final part of the rewrite
		       */
		      ip0 = vlib_buffer_get_current (b0);
		      udp0 = (udp_header_t *) ((uint8_t *) ip0 +
					       sizeof (ip4_header_t));

		      memset (ip0, 0,
			      sizeof (ip4_header_t) + sizeof (udp_header_t));

		      ip0->ip_version_and_header_length = 0x45;
		      ip0->protocol = IP_PROTOCOL_UDP;

		      sval = vlib_buffer_length_in_chain (vm, b0);
		      ip0->length = clib_host_to_net_u16 (sval);

		      sval -= sizeof (ip4_header_t);
		      udp0->length = clib_host_to_net_u16 (sval);

		      vnet_buffer (b0)->sw_if_index[VLIB_TX] = ~0;

		      if (i == (clone_count - 1))
			{
			  /* Last packet - drop out of the loop, let the
			   * transit path finish with 'b0' now
			   */
			  break;
			}

		      /* Rewrite ip and udp headers */

		      ip0->src_address.as_u32 =
			outface->src_addr.sin_addr.s_addr;
		      ip0->dst_address.as_u32 =
			outface->dest_addr.sin_addr.s_addr;

		      /* TODO -- Refresh IP ttl - is that ok? */
		      ip0->ttl = CICN_IP_TTL_DEFAULT;

		      /* TODO -- Not optimizing the IP checksum currently */
		      ip0->checksum = ip4_header_checksum (ip0);

		      udp0->src_port = outface->src_addr.sin_port;
		      udp0->dst_port = outface->dest_addr.sin_port;

		      /* TODO -- clear UDP checksum; is this ok? */
		      udp0->checksum = 0;

		      pkts_from_cache_count++;

		      /* Update face-level stats */
		      outface_stats =
			&wshard->face_stats[cicn_face_db_index (outface)];
		      outface_stats->out_datas++;

		      /* Enqueue packet to next graph node */
		      to_next[0] = bi0;
		      to_next += 1;
		      n_left_to_next -= 1;

		      BUFTRC ("ICN-TX2", bi0);
		      if (n_left_to_next == 0)
			{
			  vlib_put_next_frame (vm, node, next_index,
					       n_left_to_next);

			  vlib_get_next_frame (vm, node, next_index, to_next,
					       n_left_to_next);
			}
		    }
		}

	      /* We're now processing the last (or only) PIT entry; 'b0',
	       * 'bi0', 'ip0', 'udp0', and 'outface' should be set
	       * properly. We'll just drop through to the normal
	       * 'send one packet code below.
	       */

	      /* Update face-level stats */
	      outface_stats =
		&wshard->face_stats[cicn_face_db_index (outface)];
	      outface_stats->out_datas++;

	      next0 = ICNFWD_NEXT_LOOKUP;

	      goto ready_to_send;

	    hello_reply_rcvd_check:
	      // not a normal content msg, maybe it's hello reply
	      if (cicn_hello_match (inface, pkt_type, nameptr, namelen,
				    &sm->hello_name, &seq_num))
		{
		  /// it's a hello response
		  inface_stats->term_datas++;
		  pkts_hello_data_rec++;
		  /* Copy seq_num to global array of Up/Down data */
		  sm->cicn_hello_data_array[inface->faceid].seq_num = seq_num;
		  sm->cicn_hello_data_array[inface->faceid].faceid =
		    inface->faceid;

		  // Signal an event to the background process
		  vlib_process_signal_event_pointer (vm,
						     vlib_get_node_by_name
						     (vm,
						      (u8 *)
						      "icn-hello-process")->index,
						     CICN_HELLO_EVENT_DATA_RCVD,
						     &sm->cicn_hello_data_array
						     [inface->faceid]);
		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      /* No PIT entry, not a hello, drop */
	      pkts_no_pit_count++;
	      next0 = ICNFWD_NEXT_ERROR_DROP;
	      goto trace_single;

	      /* END: Content/Control Response */

	    }
	  else if (pkt_type == CICN_PKT_TYPE_INTEREST ||
		   pkt_type == CICN_PKT_TYPE_CONTROL_REQUEST)
	    {
	      cicn_packet_hdr_t *pkt_hdr0 = (cicn_packet_hdr_t *) body0;
	      uint8_t *msg_tlv = (uint8_t *) (pkt_hdr0 + 1);

	      inface_stats->in_interests++;

	      if (PREDICT_FALSE (pkt_hdr0->pkt_hop_limit == 0))
		{
		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      pkt_hdr0->pkt_hop_limit--;

	      /* Check whether this is an ICN Hello Interest */
	      if (PREDICT_FALSE
		  (cicn_hello_match
		   (inface, pkt_type, nameptr, namelen, &sm->hello_name,
		    NULL /*seq_num */ )))
		{
		  goto hello_request_forus;
		}

	      if (PREDICT_FALSE (inface->flags & CICN_FACE_FLAG_HELLO_DOWN))
		{
		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      if (PREDICT_FALSE (pkt_hdr0->pkt_hop_limit == 0))
		{
		  /*
		   * If traceroute request, return our information.
		   * Otherwise NAK the interest in all cases.
		   * (draft-irtf-icnrg-ccnxsemantics-03 says
		   *  "If the HopLimit equals 0, ... it MAY be sent to a
		   *   publisher application or serviced from a local
		   *   Content Store.". The current implemention does not.)
		   */
		  if (msg_type == CICN_MSG_TYPE_TRACEROUTE_REQUEST)
		    {
		      goto traceroute_request_forus;
		    }

		  pkt_hdr0->pkt_type = CICN_PKT_TYPE_NAK;
		  pkt_hdr0->pkt_nack_code = CICN_MSG_ERR_HOPLIM;

		  outface = inface;
		  outface_stats = inface_stats;

		  pkts_nacked_interests_count++;
		  pkts_nak_hoplimit_count++;
		  outface_stats->orig_naks++;
		  outface_stats->out_naks++;

		  next0 = ICNFWD_NEXT_LOOKUP;
		  goto ready_to_send;
		}

	      /* Compute the name and prefix hashes */

	      /* TODO -- could we carry hash value in from the dist node? */

	      /* TODO -- use FIB max-comps hint? */

	      /*
	       * Full and LPM prefix hashing for PIT and FIB lookups
	       */
	      ret =
		cicn_hashtb_hash_prefixes (nameptr, namelen,
					   TRUE /*fullname */ , &pfxhash,
					   0 /*!limit */ );
	      if (ret != AOK)
		{
		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      /* If this is a ping request, parse the target name and compare
	       * it to the name of the forwarder
	       */
	      if (pkt_type == CICN_PKT_TYPE_CONTROL_REQUEST &&
		  (msg_type == CICN_MSG_TYPE_ECHO_REQUEST ||
		   msg_type == CICN_MSG_TYPE_TRACEROUTE_REQUEST))
		{

		  /* Check whether the hash of the ping request target
		   * prefix matches the hash of the forwarder's name.
		   * If so, respond
		   * If a name has not been given to the forwarder,
		   * or if the hashes do not match, forward the control
		   * packet as a regular insterest.
		   */

		  /* We received an echo request with an invalid name */
		  if (pfxhash.pfx_count < 3)
		    {
		      next0 = ICNFWD_NEXT_ERROR_DROP;
		      goto trace_single;
		    }

		  if (cicn_infra_fwdr_name.fn_reply_payload_flen != 0 &&
		      cicn_infra_fwdr_name.fn_match_pfx_hash ==
		      pfxhash.pfx_hashes[pfxhash.pfx_count - 3])
		    {
		      if (msg_type == CICN_MSG_TYPE_ECHO_REQUEST)
			{
			  goto echo_request_forus;
			}
		      else
			{
			  goto traceroute_request_forus;
			}
		    }
		}

	      /*
	       * Opportunistic scan of hash row/bucket for expirations.
	       * Some old code below could be removed with this addition,
	       * it won't be executed anyway.
	       *
	       * The timeout scan and the node lookup could be
	       * easily integrated.
	       */
	      cicn_pcs_timeout (vm, &rt->pitcs, pfxhash.pfx_full_hash,
				&pit_expired_count, &cs_expired_count);

	      /*
	       * Search PIT/CS by full-name hash
	       */
	      ret =
		cicn_hashtb_lookup_node (rt->pitcs.pcs_table, nameptr,
					 namelen, pfxhash.pfx_full_hash,
					 &nodep);
	      if (ret != AOK)
		{
		  goto interest_is_new;
		}

	      pitp = cicn_pit_get_data (nodep);

	      if (pitp->shared.entry_type == CICN_CS_TYPE)
		{
		  /* Case: Existing CS entry */

		  /* Check for expired CS entry (if not done during the
		   * scan)
		   */
		  if ((tnow > pitp->shared.expire_time) ||
		      (pitp->u.cs.cs_pkt_buf == 0))
		    {

		      /* Delete and clean up expired CS entry */
		      cicn_pcs_delete (&rt->pitcs, &pitp, &nodep, vm);
		      cs_expired_count++;

		      goto interest_is_new;
		    }

		  /* Update the CS LRU, moving this item to the head */
		  cicn_cs_lru_update_head (&rt->pitcs, nodep, pitp);

		  /*
		   * Clone the CS packet, and prepare the incoming request
		   * packet to hold the rewrite info as a particle.
		   */
		  if (cicn_clone_cs_buffer (b0, pitp, vm, fl,
					    inface /*outface */ ) != AOK)
		    {
		      no_bufs_count++;
		      next0 = ICNFWD_NEXT_ERROR_DROP;
		      goto trace_single;
		    }

		  /* Refresh the ip and udp headers before the final part of
		   * the rewrite down below
		   */
		  ip0 = vlib_buffer_get_current (b0);
		  udp0 = (udp_header_t *) ((uint8_t *) ip0 +
					   sizeof (ip4_header_t));

		  memset (ip0, 0,
			  sizeof (ip4_header_t) + sizeof (udp_header_t));

		  ip0->ip_version_and_header_length = 0x45;
		  ip0->protocol = IP_PROTOCOL_UDP;

		  sval = vlib_buffer_length_in_chain (vm, b0);
		  ip0->length = clib_host_to_net_u16 (sval);

		  sval -= sizeof (ip4_header_t);
		  udp0->length = clib_host_to_net_u16 (sval);

		  pkts_from_cache_count++;

		  /* Reply to sender */
		  outface = inface;
		  inface_stats->out_datas++;

		  next0 = ICNFWD_NEXT_LOOKUP;
		  goto ready_to_send;

		}

	      /*
	       * Case: Existing PIT entry
	       */

	      /* Is the PIT entry expired? */
	      if (tnow > pitp->shared.expire_time)
		{

		  /* Remove existing entry, and treat this as new Interest */
		  cicn_pcs_delete (&rt->pitcs, &pitp, &nodep, vm);
		  pit_expired_count++;
		  goto interest_is_new;
		}

	      /*
	       * PIT aggregation: simple form for now, no change in PIT
	       * expiration.
	       *
	       * TODO -- many open questions: retransmissions,
	       * PIT entry expiration time handling.
	       */
	      for (i = 0; i < CICN_PARAM_PIT_ENTRY_PHOPS_MAX; i++)
		{

		  /* Note that this loop is vulnerable if we remove
		   * rx faces from the middle of the PIT array. We don't
		   * do that right now, so I think this is ok.
		   */
		  if (pitp->u.pit.pe_rxfaces[i] == inface->faceid)
		    {
		      /*
		       * Already in the PIT - a retransmission? We allow
		       * retransmits, by capturing the egress face
		       * and jumping to the 'send interest' code.
		       */
		      ret =
			cicn_face_entry_find_by_id (pitp->u.pit.pe_txface,
						    &outface);
		      if (ret == AOK)
			{
			  pkts_int_retrans++;
			  next0 = ICNFWD_NEXT_LOOKUP;
			  goto ready_to_send;
			}

		      break;

		    }
		  else if (pitp->u.pit.pe_rxfaces[i] == 0)
		    {
		      /* Found an available slot in the PIT */
		      pitp->u.pit.pe_rxfaces[i] = inface->faceid;
		      break;
		    }
		}

	      /* TODO -- stat for 'full PIT'? */

	      /*
	       * At this point, we've dealt with the PIT aggregation,
	       * and we can drop the current packet.
	       */
	      pkts_interest_agg++;
	      next0 = ICNFWD_NEXT_ERROR_DROP;
	      goto trace_single;

	    interest_is_new:

	      /*
	       * Need PIT entry:
	       * - find outface from FIB lookup
	       * - init new PIT entry.
	       */
	      outface = NULL;

	      ret = cicn_fib_lookup (&cicn_main.fib, &pfxhash, &pentry);
	      if (PREDICT_FALSE (ret != AOK))
		{
		  goto interest_noroute_check;
		}

	      /* Look for the right next-hop - for now, use max weight */
	      u8 weight = 0;
	      for (i = 0; i < CICN_PARAM_FIB_ENTRY_NHOPS_MAX; i++)
		{
		  cicn_face_db_entry_t *face;
		  if ((pentry->fe_next_hops[i].nh_faceid == 0))
		    {
		      continue;
		    }
		  if (pentry->fe_next_hops[i].nh_weight <= weight)
		    {
		      continue;
		    }
		  faceid = pentry->fe_next_hops[i].nh_faceid;

		  /* Find tx face by face id */
		  ret = cicn_face_entry_find_by_id (faceid, &face);
		  if (PREDICT_FALSE (ret != AOK))
		    {
		      continue;
		    }
		  if (PREDICT_FALSE ((face->flags & CICN_FACE_FLAGS_DOWN)))
		    {
		      continue;
		    }
		  outface = face;
		  weight = pentry->fe_next_hops[i].nh_weight;
		}

	    interest_noroute_check:
	      if (outface == NULL)
		{
		  pkt_hdr0->pkt_type = CICN_PKT_TYPE_NAK;
		  pkt_hdr0->pkt_nack_code = CICN_MSG_ERR_NOROUTE;

		  outface = inface;
		  outface_stats = inface_stats;

		  pkts_nacked_interests_count++;
		  pkts_nak_no_route_count++;
		  outface_stats->orig_naks++;
		  outface_stats->out_naks++;

		  next0 = ICNFWD_NEXT_LOOKUP;
		  goto ready_to_send;
		}

	      /* Create PIT node and init PIT entry */
	      nodep = cicn_hashtb_alloc_node (rt->pitcs.pcs_table);
	      if (PREDICT_FALSE (nodep == NULL))
		{
		  /* Nothing we can do - no mem */

		  no_bufs_count++;

		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      pitp = cicn_pit_get_data (nodep);

	      cicn_pit_init_data (pitp);

	      pitp->shared.entry_type = CICN_PIT_TYPE;
	      pitp->shared.create_time = tnow;
	      pitp->u.pit.pe_txface = outface->faceid;
	      pitp->u.pit.pe_rxfaces[0] = inface->faceid;

	      /*
	       * Interest lifetime based on optional hdr_tlv, ranges, default
	       * - special case is lifetime of 0
	       *   - this is "forward but do not expect content return" case
	       *   - code sequence here (and above for content)
	       *     always checks if an existing PIT entry has
	       *     expired. If so, it is deleted before continuing
	       *     to process current message. Thus, should be
	       *     benign to enter an interest with a 0 lifetime
	       *     into the PIT: it will always be be found to be
	       *     expired at the earliest opportunity, the only
	       *     cost being the held hash resources.
	       * - corresponding expiry time appears in pit entry and
	       *   (compressed) in bucket entry
	       */
	      uint64_t imsg_lifetime;
	      ret =
		cicn_parse_hdr_time_ms (body0, &pkt_hdr_desc0,
					CICN_HDR_TLV_INT_LIFETIME,
					&imsg_lifetime);
	      if (ret != AOK)
		{		// no header timeout, use default
		  imsg_lifetime = sm->pit_lifetime_dflt_ms;
		}
	      else if (imsg_lifetime != 0)
		{
		  if (imsg_lifetime < sm->pit_lifetime_min_ms)
		    {
		      imsg_lifetime = sm->pit_lifetime_min_ms;
		    }
		  else if (imsg_lifetime > sm->pit_lifetime_max_ms)
		    {
		      imsg_lifetime = sm->pit_lifetime_max_ms;
		    }
		}
	      pitp->shared.expire_time =
		cicn_pcs_get_exp_time (tnow, imsg_lifetime);
	      bkt_ent_exp_time = cicn_infra_get_fast_exp_time (imsg_lifetime);

	      /* TODO -- decide whether to hold/clone interest packet mbuf */

	      /* Set up the hash node and insert it */
	      ret =
		cicn_hashtb_init_node (rt->pitcs.pcs_table, nodep,
				       pfxhash.pfx_full_hash, nameptr,
				       namelen);
	      if (ret == AOK)
		{
		  ret = cicn_pit_insert (&rt->pitcs, pitp, nodep);
		}
	      if (ret != AOK)
		{
		  /* Just dropping on error for now */

		  /* Return hashtable node */
		  cicn_hashtb_free_node (rt->pitcs.pcs_table, nodep);

		  next0 = ICNFWD_NEXT_ERROR_DROP;
		  goto trace_single;
		}

	      // Set the hashtable-level expiration value in bucket entry
	      cicn_hashtb_entry_set_expiration (rt->pitcs.pcs_table, nodep,
						bkt_ent_exp_time,
						CICN_HASH_ENTRY_FLAG_FAST_TIMEOUT);

	      /* Looks like we're ok to forward */
	      outface_stats =
		&wshard->face_stats[cicn_face_db_index (outface)];
	      outface_stats->out_interests++;

	      next0 = ICNFWD_NEXT_LOOKUP;
	      goto ready_to_send;


	      /*
	       * Code routes control requests for us to these labels:
	       * respond to with control reply
	       */

	    hello_request_forus:
	      /* Hello Request: For now, just change the packet and msg type
	       * (do not attach any extra payload) and reflect back
	       */
	      pkt_hdr0->pkt_type = CICN_PKT_TYPE_CONTROL_REPLY;
	      C_PUTINT16 (&msg_tlv[0], CICN_MSG_TYPE_CONTENT);

	      outface = inface;
	      outface_stats = inface_stats;

	      pkts_hello_int_rec++;
	      pkts_hello_data_sent++;
	      inface_stats->term_interests++;
	      outface_stats->orig_datas++;
	      outface_stats->out_datas++;

	      /* Send it out directly without doing anything further */
	      next0 = ICNFWD_NEXT_LOOKUP;
	      goto ready_to_send;

	    echo_request_forus:
	      /* Tweak packet and message types and send back
	       * as a ping request reply
	       */
	      pkt_hdr0->pkt_type = CICN_PKT_TYPE_CONTROL_REPLY;
	      C_PUTINT16 (msg_tlv, CICN_MSG_TYPE_ECHO_REPLY);

	      outface = inface;
	      outface_stats = inface_stats;

	      pkts_control_reply_count++;
	      inface_stats->term_interests++;
	      outface_stats->out_datas++;

	      next0 = ICNFWD_NEXT_LOOKUP;
	      goto ready_to_send;

	    traceroute_request_forus:
	      /* Update msg type and hop limit value */
	      pkt_hdr0->pkt_type = CICN_PKT_TYPE_CONTROL_REPLY;
	      C_PUTINT16 (msg_tlv, CICN_MSG_TYPE_TRACEROUTE_REPLY);
	      pkt_hdr0->pkt_hop_limit = CICN_DEFAULT_HOP_LIMIT;
	      if (cicn_infra_fwdr_name.fn_reply_payload_flen > 0)
		{
		  int payload_size =
		    cicn_infra_fwdr_name.fn_reply_payload_flen;
		  vlib_buffer_add_data (vm, b0->free_list_index,
					bi0,
					cicn_infra_fwdr_name.fn_reply_payload,
					payload_size);

		  uint16_t imsg_size;
		  C_GETINT16 (imsg_size, &msg_tlv[CICN_TLV_TYPE_LEN]);
		  C_PUTINT16 (&msg_tlv[CICN_TLV_TYPE_LEN],
			      imsg_size + payload_size);
		  pkt_hdr0->pkt_len =
		    clib_host_to_net_u16 (clib_net_to_host_u16
					  (pkt_hdr0->pkt_len) + payload_size);
		  udp0->length =
		    clib_host_to_net_u16 (clib_net_to_host_u16 (udp0->length)
					  + payload_size);
		  ip0->length =
		    clib_host_to_net_u16 (clib_net_to_host_u16 (ip0->length) +
					  payload_size);
		}

	      outface = inface;
	      outface_stats = inface_stats;

	      pkts_control_reply_count++;
	      inface_stats->term_interests++;
	      outface_stats->out_datas++;

	      next0 = ICNFWD_NEXT_LOOKUP;
	      goto ready_to_send;

	    }
	  else if (pkt_type == CICN_PKT_TYPE_NAK)
	    {

	      inface_stats->in_naks++;

	    }
	  else
	    {
	      /* Don't expect any other packets: just drop? */

	      next0 = ICNFWD_NEXT_ERROR_DROP;
	      goto trace_single;
	    }

	ready_to_send:

	  /* Use info to prep and enqueue: we expect that
	   * the egress face and the next-node have been set.
	   */

	  /* TODO -- worth optimizing/remembering egress interface? */

	  /* TODO -- assuming ipv4 udp egress for now */

	  vnet_buffer (b0)->sw_if_index[VLIB_TX] = ~0;

	  /* Rewrite ip and udp headers */
	  ip0->src_address.as_u32 = outface->src_addr.sin_addr.s_addr;
	  ip0->dst_address.as_u32 = outface->dest_addr.sin_addr.s_addr;

	  /* TODO -- Refresh IP ttl - is that ok? */
	  ip0->ttl = CICN_IP_TTL_DEFAULT;

	  /* TODO -- Not optimizing the IP checksum currently */
	  ip0->checksum = ip4_header_checksum (ip0);

	  udp0->src_port = outface->src_addr.sin_port;
	  udp0->dst_port = outface->dest_addr.sin_port;

	  /* TODO -- clear UDP checksum; is this ok? */
	  udp0->checksum = 0;

	  /* Next-node should already be ok at this point */

	trace_single:

	  /* Maybe trace */
	  if (PREDICT_FALSE ((node->flags & VLIB_NODE_FLAG_TRACE) &&
			     (b0->flags & VLIB_BUFFER_IS_TRACED)))
	    {

	      icnfwd_trace_t *t = vlib_add_trace (vm, node, b0, sizeof (*t));
	      t->pkt_type = pkt_type;
	      t->msg_type = msg_type;
	      t->sw_if_index = sw_if_index0;
	      t->next_index = next0;
	    }

	  /* Speculatively enqueue packet b0 (index in bi0)
	   * to the current next frame
	   */
	  to_next[0] = bi0;
	  to_next += 1;
	  n_left_to_next -= 1;

	  /* Incr packet counter */
	  pkts_processed += 1;

	  BUFTRC ((next0 == ICNFWD_NEXT_ERROR_DROP) ? "DROPTX1" : "ICN-TX1",
		  bi0);
	  /* Verify speculative enqueue, maybe switch current next frame */
	  vlib_validate_buffer_enqueue_x1 (vm, node, next_index,
					   to_next, n_left_to_next,
					   bi0, next0);
	}

      /*
       * End of 1-at-a-time loop; finish 'next' processing
       */
      vlib_put_next_frame (vm, node, next_index, n_left_to_next);
    }

  /* Check the CS LRU, and trim if necessary. */
  cicn_trim_cs_lru (vm, node, &(rt->pitcs));

  pit_int_count = cicn_pit_get_int_count (&(rt->pitcs));
  pit_cs_count = cicn_pit_get_cs_count (&(rt->pitcs));

  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_PROCESSED, pkts_processed);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_INTERESTS, pkts_interest_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_DATAS, pkts_data_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_NAKS, pkts_nak_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_CACHED, pkts_from_cache_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_NACKED_INTERESTS,
			       pkts_nacked_interests_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_HOPLIMIT_EXCEEDED,
			       pkts_nak_hoplimit_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_NO_ROUTE,
			       pkts_nak_no_route_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_NO_PIT, pkts_no_pit_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_PIT_EXPIRED, pit_expired_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_CS_EXPIRED, cs_expired_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_NO_BUFS, no_bufs_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_INTEREST_AGG, pkts_interest_agg);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_INT_RETRANS, pkts_int_retrans);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_CONTROL_REQUESTS,
			       pkts_control_request_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_CONTROL_REPLIES,
			       pkts_control_reply_count);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_HELLO_INTERESTS_RCVD,
			       pkts_hello_int_rec);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_HELLO_DMSGS_SENT,
			       pkts_hello_data_sent);
  vlib_node_increment_counter (vm, icnfwd_node.index,
			       ICNFWD_ERROR_HELLO_DMSGS_RCVD,
			       pkts_hello_data_rec);


  update_node_counter (vm, icnfwd_node.index,
		       ICNFWD_ERROR_INT_COUNT, pit_int_count);
  update_node_counter (vm, icnfwd_node.index,
		       ICNFWD_ERROR_CS_COUNT, pit_cs_count);
  ASSERT (rt->pitcs.pcs_lru_count == pit_cs_count);

  return (frame->n_vectors);
}

/*
 * Check the CS LRU, trim if necessary
 */
static int
cicn_trim_cs_lru (vlib_main_t * vm, vlib_node_runtime_t * node,
		  cicn_pit_cs_t * pit)
{
#define LRU_TRIM_COUNT 512

  int i, count = 0, bufcount;
  u32 node_list[LRU_TRIM_COUNT], buf_list[LRU_TRIM_COUNT];
  cicn_hash_node_t *np;
  cicn_pcs_entry_t *pcs;

  if (pit->pcs_lru_count > pit->pcs_lru_max)
    {

      /* Collect an armful of entries from the back of the LRU */
      count = cicn_cs_lru_trim (pit, node_list, LRU_TRIM_COUNT);

      bufcount = 0;

      for (i = 0; i < count; i++)
	{
	  /* Retrieve the CS data */
	  np = cicn_hashtb_node_from_idx (pit->pcs_table, node_list[i]);

	  pcs = cicn_pit_get_data (np);

	  /* Extract the packet buffer id and save it */
	  if (pcs->u.cs.cs_pkt_buf != 0)
	    {
	      BUFTRC ("  CS-TRIM", pcs->u.cs.cs_pkt_buf);
	      buf_list[bufcount++] = pcs->u.cs.cs_pkt_buf;
	      pcs->u.cs.cs_pkt_buf = 0;
	    }

	  /* Remove the hash node from the hashtable and free it */
	  cicn_cs_delete_trimmed (pit, &pcs, &np, vm);

	}

      /* Free packet buffers */
      BUFTRC ("CS-TRIM-ALL", bufcount);
      if (bufcount > 0)
	{
	  vlib_buffer_free (vm, buf_list, bufcount);
	}
    }

  return (count);
}

/*
 * Node registration for the forwarder node
 */
VLIB_REGISTER_NODE (icnfwd_node) =
{
  .function = icnfwd_node_fn,
  .name = "icnfwd",
  .vector_size = sizeof (u32),
  .runtime_data_bytes = sizeof (icnfwd_runtime_t),
  .format_trace = icnfwd_format_trace,
  .type = VLIB_NODE_TYPE_INTERNAL,
  .n_errors = ARRAY_LEN (icnfwd_error_strings),
  .error_strings = icnfwd_error_strings,
  .n_next_nodes = ICNFWD_N_NEXT,
  .next_nodes = {
    [ICNFWD_NEXT_LOOKUP] = "ip4-lookup",
    [ICNFWD_NEXT_ERROR_DROP] = "error-drop",
  }
,};

/*
 * TODO -- new worker node ends here
 */

#if CICN_FEATURE_MULTITHREAD
/*
 * Work-distribution node (first pass, anyway). We use the full-name hash
 * to direct packets at forwarding worker threads. We've informed the
 * handoff node running at the edge of each graph instance of the existence
 * of our forwarding node, as part of setup/enable. As a result, the
 * target thread's handoff node will be able to hand our packets
 * directly to our forwarding node.
 */

/*
 * Node context data; we think this is per-thread/instance/graph
 */
typedef struct icndist_runtime_s
{

  /* Vector of queues directed at each forwarding worker thread */
  vlib_frame_queue_elt_t **handoff_q_elt_by_worker;

} icndist_runtime_t;

/* Registration struct for a graph node */
vlib_node_registration_t icndist_node;

/*
 * Next graph nodes, which reference the list in the actual registration
 * block below
 */
typedef enum
{
  ICNDIST_NEXT_FWD,
  ICNDIST_NEXT_ERROR_DROP,
  ICNDIST_N_NEXT,
} icndist_next_t;

/* Stats string values */
static char *icndist_error_strings[] = {
#define _(sym,string) string,
  foreach_icndist_error
#undef _
};

/* Trace context struct */
typedef struct
{
  u32 next_worker;
  u32 sw_if_index;
  u8 pkt_type;
  u16 msg_type;
} icndist_trace_t;

/* Distribution node packet trace format function */
static u8 *
icndist_format_trace (u8 * s, va_list * args)
{
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  icndist_trace_t *t = va_arg (*args, icndist_trace_t *);

  s = format (s, "ICN-DIST: pkt: %d, msg %d, sw_if_index %d, next worker %d",
	      (int) t->pkt_type, (int) t->msg_type,
	      t->sw_if_index, t->next_worker);
  return (s);
}

/*
 * IP-worker allocates a free packet frame to fill in and handed off
 * to ICN-worker.
 */
static inline vlib_frame_queue_elt_t *
get_new_handoff_queue_elt (u32 vlib_worker_index)
{
  vlib_frame_queue_t *fq;
  vlib_frame_queue_elt_t *elt;
  u64 new_tail;

  fq = vlib_frame_queues[vlib_worker_index];
  ASSERT (fq);

  new_tail = __sync_add_and_fetch (&fq->tail, 1);

  /* Wait until a ring slot is available */
  while (new_tail >= fq->head_hint + fq->nelts)
    {
      vlib_worker_thread_barrier_check ();
    }

  elt = fq->elts + (new_tail & (fq->nelts - 1));

  /* Should not happen that available ring slot is marked valid */
  while (elt->valid)
    ;

  elt->msg_type = VLIB_FRAME_QUEUE_ELT_DISPATCH_FRAME;
  elt->last_n_vectors = elt->n_vectors = 0;

  return (elt);
}

/*
 * IP-worker gets frame for ICN-worker, allocating new frame if needed.
 */
static inline vlib_frame_queue_elt_t *
icn_get_handoff_queue_elt (u32 vlib_worker_index,
			   vlib_frame_queue_elt_t ** handoff_queue_elt)
{
  vlib_frame_queue_elt_t *elt;

  if (handoff_queue_elt[vlib_worker_index])
    {
      return (handoff_queue_elt[vlib_worker_index]);
    }

  elt = get_new_handoff_queue_elt (vlib_worker_index);

  handoff_queue_elt[vlib_worker_index] = elt;

  return (elt);
}

/*
 * Enables the frame once the IP-worker is done with it.
 */
static inline void
icn_put_handoff_queue_elt (vlib_frame_queue_elt_t * hf)
{
  CLIB_MEMORY_BARRIER ();
  hf->valid = 1;
}

/*
 * Second-level work-distribution node: IP-worker got packets based on
 * IP 5-tuple hash, redistributes to (final) ICN-worker based on ICN name hash.
 */
static uword
icndist_node_fn (vlib_main_t * vm,
		 vlib_node_runtime_t * node, vlib_frame_t * frame)
{
  u32 n_left_from, *from;
  u32 *to_next;
  icndist_next_t next_index;
  u32 pkts_processed = 0, pkts_interest_count = 0, pkts_data_count = 0;
  u32 pkts_dropped = 0;
  int i, ret;
  icndist_runtime_t *rt;
  u32 current_worker_index = ~0;
  u32 next_worker_index = 0;
  vlib_frame_queue_elt_t *hf = 0;
  u32 n_left_to_next_worker = 0, *to_next_worker = 0;
  cicn_main_t *icnmain = &cicn_main;
  u32 n_left_to_next;
  u32 drop_count = 0, drop_list[VLIB_FRAME_SIZE];

  /* Retrieve the per-thread context struct */
  rt = vlib_node_get_runtime_data (vm, icndist_node.index);

  /*
   * If necessary, do one-time init
   */
  if (rt->handoff_q_elt_by_worker == NULL)
    {

      /* Init/allocate a vector we'll use to store queues directed at
       * each worker thread we're using for forwarding.
       */
      vec_validate (rt->handoff_q_elt_by_worker,
		    icnmain->worker_first_index + icnmain->worker_count - 1);

      /* TODO -- dpdk io does a 'congested_handoff' queue also? Do we have
       * to do that too, or is there other infra that will do something
       * sane if we're overrunning the forwarder threads?
       */
    }

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;

  next_index = node->cached_next_index;
  next_index = ICNDIST_NEXT_FWD;

  vlib_get_next_frame (vm, node, next_index, to_next, n_left_to_next);

  /* TODO -- just doing 1-at-a-time for now, to simplify things a bit. */

  /* TODO -- v6 support too? */
  /* TODO -- more interesting stats and trace */

  /* TODO -- simpler loop since we don't use the vlib api? */
//      while (n_left_from > 0 && n_left_to_next > 0) {

  while (n_left_from > 0)
    {
      u32 bi0;
      vlib_buffer_t *b0;
      u32 sw_if_index0;
      u8 *body0, *ptr0;
      u32 len0;
      u8 pkt_type;
      u16 msg_type;
      cicn_pkt_hdr_desc_t pkt_hdr_desc0;
      u8 *nameptr;
      u32 namelen;
      u64 hashval;

      /* Prefetch for next iteration. */
      if (n_left_from > 1)
	{
	  vlib_buffer_t *p2;

	  p2 = vlib_get_buffer (vm, from[1]);

	  vlib_prefetch_buffer_header (p2, LOAD);

	  CLIB_PREFETCH (p2->data, (2 * CLIB_CACHE_LINE_BYTES), LOAD);
	}

      /* Set up to access the packet */

      /* We don't use the normal 'to next node' variables, because we're
       * mostly moving packets to other threads. We only use the direct
       * path for packets destined for the current thread's forwarder
       * node; even error/drop packets are dealt with all at once, at
       * the end of the loop.
       */
      bi0 = from[0];
      from += 1;
      n_left_from -= 1;

      b0 = vlib_get_buffer (vm, bi0);

      /*
       * From the IPv4  udp code, we think we're handed the payload part
       * of the packet
       */
      ASSERT (b0->current_data >=
	      (sizeof (ip4_header_t) + sizeof (udp_header_t)));

      /* Capture pointer to the payload */
      ptr0 = body0 = vlib_buffer_get_current (b0);
      len0 = b0->current_length;

      sw_if_index0 = vnet_buffer (b0)->sw_if_index[VLIB_RX];

      /* Reset destination worker thread idx */
      next_worker_index = icnmain->worker_first_index;

      /*
       * Do a quick, in-place parse/validate pass, locating
       * a couple of key pieces of info about the packet
       */
      ret = cicn_parse_pkt (ptr0, len0, &pkt_type, &msg_type,
			    &nameptr, &namelen, &pkt_hdr_desc0);

      /* If we can't even get at the name, we just drop */
      if (ret != AOK)
	{
	  /* Just drop on error? */
	  drop_list[drop_count] = bi0;
	  drop_count++;

	  pkts_dropped++;
	  goto trace_single;
	}

      if (pkt_type == CICN_PKT_TYPE_INTEREST)
	{
	  pkts_interest_count++;
	}
      else if (pkt_type == CICN_PKT_TYPE_CONTENT)
	{
	  pkts_data_count++;
	}

      /* Compute the full name hash, for distribution
       * (so only doing the full-name hash here, no LPM prefix hashing).
       * TODO - could we capture the hash and pass it along?
       */
      hashval = cicn_hashtb_hash_name (nameptr, namelen);

      /* Use the hash to identify the correct worker thread */

      if (PREDICT_TRUE (is_pow2 (icnmain->worker_count)))
	{
	  next_worker_index += hashval & (icnmain->worker_count - 1);
	}
      else
	{
	  next_worker_index += hashval % icnmain->worker_count;
	}

      /* Use normal next-node path if we're
       * using the forwarding node on the current thread; that'd
       * save some work.
       */
      if (next_worker_index == vm->cpu_index)
	{
	  if (n_left_to_next == 0)
	    {
	      vlib_put_next_frame (vm, node, next_index, n_left_to_next);

	      vlib_get_next_frame (vm, node, next_index,
				   to_next, n_left_to_next);
	    }

	  ASSERT (n_left_to_next > 0);

	  to_next[0] = bi0;
	  to_next++;
	  n_left_to_next--;

	  /* Skip to end of the loop */
	  goto trace_single;
	}

      /* On the target worker thread, the buffers will arrive at the
       * dpdk handoff node. Convince target's dpdk handoff node to move
       * the buffers to the icn orwarding node.
       */
      vnet_buffer (b0)->handoff.next_index = cicn_main.fwd_next_node;

      /* Locate or allocate a queue for the thread; update current
       * queue if we're changing destination threads.
       */
      if (next_worker_index != current_worker_index)
	{
	  if (hf)
	    {
	      hf->n_vectors = VLIB_FRAME_SIZE - n_left_to_next_worker;
	    }

	  hf = icn_get_handoff_queue_elt (next_worker_index,
					  rt->handoff_q_elt_by_worker);

	  n_left_to_next_worker = VLIB_FRAME_SIZE - hf->n_vectors;
	  to_next_worker = &hf->buffer_index[hf->n_vectors];
	  current_worker_index = next_worker_index;
	}

      /* Enqueue to correct worker thread */
      to_next_worker[0] = bi0;
      to_next_worker++;
      n_left_to_next_worker--;

      /*
       * If we've filled a frame, pass it on
       */
      if (n_left_to_next_worker == 0)
	{
	  hf->n_vectors = VLIB_FRAME_SIZE;
	  icn_put_handoff_queue_elt (hf);
	  current_worker_index = ~0;
	  rt->handoff_q_elt_by_worker[next_worker_index] = 0;
	  hf = 0;
	}

    trace_single:

      /* Maybe trace */
      if (PREDICT_FALSE ((node->flags & VLIB_NODE_FLAG_TRACE)
			 && (b0->flags & VLIB_BUFFER_IS_TRACED)))
	{

	  icndist_trace_t *t = vlib_add_trace (vm, node, b0, sizeof (*t));

	  t->sw_if_index = sw_if_index0;
	  t->pkt_type = pkt_type;
	  t->msg_type = msg_type;
	  t->next_worker = next_worker_index;
	}

      /* Incr packet counter */
      pkts_processed += 1;
    }

  /*
   * End of 1-at-a-time loop through the incoming frame
   */

  /* Finish handing frames to threads, and reset */
  if (hf)
    {
      hf->n_vectors = VLIB_FRAME_SIZE - n_left_to_next_worker;
    }

  /* Ship remaining frames to the worker nodes */
  for (i = 0; i < vec_len (rt->handoff_q_elt_by_worker); i++)
    {
      if (rt->handoff_q_elt_by_worker[i])
	{
	  hf = rt->handoff_q_elt_by_worker[i];
	  /*
	   * It works better to let the handoff node
	   * rate-adapt, always ship the handoff queue element.
	   */
	  if (1 || hf->n_vectors == hf->last_n_vectors)
	    {
	      icn_put_handoff_queue_elt (hf);
	      rt->handoff_q_elt_by_worker[i] = 0;
	    }
	  else
	    {
	      hf->last_n_vectors = hf->n_vectors;
	    }
	}

#if 0				/* TODO -- no congested queues for now? */
      congested_handoff_queue_by_worker_index[i] =
	(vlib_frame_queue_t *) (~0);
#endif /* TODO */
    }

  hf = 0;
  current_worker_index = ~0;

  /* Dispose of any pending 'normal' frame within this thread */
  vlib_put_next_frame (vm, node, next_index, n_left_to_next);

  /* Deal with any error/drop packets */
  if (drop_count > 0)
    {
      vlib_error_drop_buffers (vm, node, drop_list, 1, drop_count,
			       ICNDIST_NEXT_ERROR_DROP /* next index */ ,
			       icndist_node.index, ICNDIST_ERROR_DROPS);
    }

  /* Update counters */
  vlib_node_increment_counter (vm, icndist_node.index,
			       ICNDIST_ERROR_PROCESSED, pkts_processed);
  vlib_node_increment_counter (vm, icndist_node.index,
			       ICNDIST_ERROR_INTERESTS, pkts_interest_count);
  vlib_node_increment_counter (vm, icndist_node.index,
			       ICNDIST_ERROR_DATAS, pkts_data_count);
  vlib_node_increment_counter (vm, icndist_node.index,
			       ICNDIST_ERROR_DROPS, pkts_dropped);

  return (frame->n_vectors);
}

/* End of the packet-distributing node function */

/*
 * Node registration block for the work-distributing node.
 * TODO -- using the same trace func as the icnfwd node for now
 */
VLIB_REGISTER_NODE (icndist_node) =
{
  .function = icndist_node_fn,
  .name = "icndist",
  .vector_size = sizeof (u32),
  .runtime_data_bytes = sizeof (icndist_runtime_t),
  .format_trace = icndist_format_trace,
  .type = VLIB_NODE_TYPE_INTERNAL,
  .n_errors = ARRAY_LEN (icndist_error_strings),
  .error_strings = icndist_error_strings,
  .n_next_nodes = ICNDIST_N_NEXT,
  .next_nodes = {
    [ICNDIST_NEXT_FWD] = "icnfwd",
    [ICNDIST_NEXT_ERROR_DROP] = "error-drop",
  }
,};
#endif // CICN_FEATURE_MULTITHREAD
