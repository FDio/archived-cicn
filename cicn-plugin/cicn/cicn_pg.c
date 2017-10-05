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
 * cicn_pg.c: VPP packet-generator ('pg') graph nodes and assorted utilities.
 */

#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/pg/pg.h>
#include <vnet/ip/ip.h>
#include <vnet/ethernet/ethernet.h>

#include <cicn/cicn.h>

/* Registration struct for a graph node */
vlib_node_registration_t icn_pg_node;

/* Stats, which end up called "error" even though they aren't... */
#define foreach_icnpg_error			\
    _(PROCESSED, "ICN PG packets processed")	\
    _(DROPPED, "ICN PG packets dropped") 	\
    _(INTEREST_MSGS_GENERATED, "ICN PG Interests generated") \
    _(CONTENT_MSGS_RECEIVED, "ICN PG Content msgs received") \
    _(NACKS_RECEIVED, "ICN PG NACKs received")

typedef enum
{
#define _(sym,str) ICNPG_ERROR_##sym,
  foreach_icnpg_error
#undef _
    ICNPG_N_ERROR,
} icnpg_error_t;

static char *icnpg_error_strings[] = {
#define _(sym,string) string,
  foreach_icnpg_error
#undef _
};

/* Next graph nodes, which reference the list in the actual registration
 * block below
 */
typedef enum
{
  ICNPG_NEXT_LOOKUP,
  ICNPG_NEXT_DROP,
  ICNPG_N_NEXT,
} icnpg_next_t;

/* Trace context struct */
typedef struct
{
  u32 next_index;
  u32 sw_if_index;
  u8 pkt_type;
  u16 msg_type;
} icnpg_trace_t;

typedef struct icnpg_main_s
{
  uint64_t namecounter;
} icnpg_main_t;

icnpg_main_t icnpg_main = {
  .namecounter = 0ULL
};

/* packet trace format function */
static u8 *
format_icnpg_trace (u8 * s, va_list * args)
{
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  icnpg_trace_t *t = va_arg (*args, icnpg_trace_t *);

  s = format (s, "ICNPG: pkt: %d, msg %d, sw_if_index %d, next index %d",
	      (int) t->pkt_type, (int) t->msg_type,
	      t->sw_if_index, t->next_index);
  return (s);
}

/*
 * Node function for the icn packet-generator client. The goal here is to
 * manipulate/tweak a stream of packets that have been injected by the
 * vpp packet generator to generate icn request traffic.
 */
static uword
icnpg_client_node_fn (vlib_main_t * vm, vlib_node_runtime_t * node,
		      vlib_frame_t * frame)
{
  u32 n_left_from, *from, *to_next;
  icnpg_next_t next_index;
  u32 pkts_processed = 0, pkts_dropped = 0;
  u32 interest_msgs_generated = 0, content_msgs_received = 0;
  u32 nacks_received = 0;
  u32 bi0, bi1;
  vlib_buffer_t *b0, *b1;
  u8 pkt_type0 = 0, pkt_type1 = 0;
  u16 msg_type0 = 0, msg_type1 = 0;
  cicn_pkt_hdr_desc_t pkt_hdr_desc0, pkt_hdr_desc1;
  u8 *body0, *body1;
  u32 len0, len1;
  udp_header_t *udp0, *udp1;
  ip4_header_t *ip0, *ip1;
  uint8_t *name0, *name1;
  uint32_t namelen0, namelen1;
  cicn_main_t *sm = &cicn_main;
  icnpg_main_t *ipgm = &icnpg_main;

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;
  next_index = node->cached_next_index;

  while (n_left_from > 0)
    {
      u32 n_left_to_next;

      vlib_get_next_frame (vm, node, next_index, to_next, n_left_to_next);

      while (n_left_from >= 4 && n_left_to_next >= 2)
	{
	  u32 next0 = ICNPG_NEXT_DROP;
	  u32 next1 = ICNPG_NEXT_DROP;
	  u32 sw_if_index0, sw_if_index1;

	  /* Prefetch next iteration. */
	  {
	    vlib_buffer_t *p2, *p3;

	    p2 = vlib_get_buffer (vm, from[2]);
	    p3 = vlib_get_buffer (vm, from[3]);

	    vlib_prefetch_buffer_header (p2, LOAD);
	    vlib_prefetch_buffer_header (p3, LOAD);

	    CLIB_PREFETCH (p2->data, (2 * CLIB_CACHE_LINE_BYTES), STORE);
	    CLIB_PREFETCH (p3->data, (2 * CLIB_CACHE_LINE_BYTES), STORE);
	  }

	  /* speculatively enqueue b0 and b1 to the current next frame */
	  to_next[0] = bi0 = from[0];
	  to_next[1] = bi1 = from[1];
	  from += 2;
	  to_next += 2;
	  n_left_from -= 2;
	  n_left_to_next -= 2;

	  b0 = vlib_get_buffer (vm, bi0);
	  b1 = vlib_get_buffer (vm, bi1);

	  sw_if_index0 = vnet_buffer (b0)->sw_if_index[VLIB_RX];
	  sw_if_index1 = vnet_buffer (b1)->sw_if_index[VLIB_RX];

	  /* We think that the udp code has handed us the payloads,
	   * so we need to walk back to the IP headers
	   */
	  ASSERT (b0->current_data >=
		  (sizeof (ip4_header_t) + sizeof (udp_header_t)));

	  ASSERT (b1->current_data >=
		  (sizeof (ip4_header_t) + sizeof (udp_header_t)));

	  body0 = vlib_buffer_get_current (b0);
	  len0 = b0->current_length;

	  body1 = vlib_buffer_get_current (b1);
	  len1 = b1->current_length;

	  vlib_buffer_advance (b0, -(sizeof (udp_header_t)));
	  udp0 = vlib_buffer_get_current (b0);
	  vlib_buffer_advance (b0, -(sizeof (ip4_header_t)));
	  ip0 = vlib_buffer_get_current (b0);

	  vlib_buffer_advance (b1, -(sizeof (udp_header_t)));
	  udp1 = vlib_buffer_get_current (b1);
	  vlib_buffer_advance (b1, -(sizeof (ip4_header_t)));
	  ip1 = vlib_buffer_get_current (b1);

	  /* Check icn packets, locate names */
	  if (cicn_parse_pkt (body0, len0, &pkt_type0, &msg_type0,
			      &name0, &namelen0, &pkt_hdr_desc0) == AOK)
	    {

	      if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_INTEREST) &&
				(msg_type0 == CICN_MSG_TYPE_INTEREST)))
		{
		  /* Increment the appropriate message counter */
		  interest_msgs_generated++;

		  /* Stuff the counter value into the last name-comp */
		  C_PUTINT64 ((name0 + namelen0 - 8), ipgm->namecounter);
		  ipgm->namecounter += 1ULL;

		  /* Rewrite and send */

		  /* Rewrite and send */
		  ip0->src_address.as_u32 = sm->pgen_clt_src_addr;
		  ip0->dst_address.as_u32 = sm->pgen_clt_dest_addr;

		  ip0->checksum = ip4_header_checksum (ip0);

		  udp0->src_port = sm->pgen_clt_src_port;
		  udp0->dst_port = sm->pgen_clt_dest_port;
		  udp0->checksum = 0;

		  next0 = ICNPG_NEXT_LOOKUP;
		}
	      else if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_CONTENT) &&
				     (msg_type0 == CICN_MSG_TYPE_CONTENT)))
		{
		  /* If we receive a content message, increment a counter */
		  content_msgs_received++;
		}
	      else if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_NAK)))
		{
		  /* If we receive a NACK, just increment a counter */
		  nacks_received++;
		}
	    }

	  if (cicn_parse_pkt (body1, len1, &pkt_type1, &msg_type1,
			      &name1, &namelen1, &pkt_hdr_desc1) == AOK)
	    {
	      if (PREDICT_TRUE ((pkt_type1 == CICN_PKT_TYPE_INTEREST) &&
				(msg_type1 == CICN_MSG_TYPE_INTEREST)))
		{
		  /* Increment the appropriate message counter */
		  interest_msgs_generated++;

		  /* Stuff the counter value into the last name-comp */
		  C_PUTINT64 ((name1 + namelen1 - 8), ipgm->namecounter);
		  ipgm->namecounter += 1ULL;

		  /* Rewrite and send */

		  /* Rewrite and send */
		  ip1->src_address.as_u32 = sm->pgen_clt_src_addr;
		  ip1->dst_address.as_u32 = sm->pgen_clt_dest_addr;

		  ip1->checksum = ip4_header_checksum (ip0);

		  udp1->src_port = sm->pgen_clt_src_port;
		  udp1->dst_port = sm->pgen_clt_dest_port;
		  udp1->checksum = 0;

		  next1 = ICNPG_NEXT_LOOKUP;
		}
	      else if (PREDICT_TRUE ((pkt_type1 == CICN_PKT_TYPE_CONTENT) &&
				     (msg_type1 == CICN_MSG_TYPE_CONTENT)))
		{
		  /* If we receive a content message, increment a counter */
		  content_msgs_received++;
		}
	      else if (PREDICT_TRUE ((pkt_type1 == CICN_PKT_TYPE_NAK)))
		{
		  /* If we receive a NACK, just increment a counter */
		  nacks_received++;
		}
	    }

	  /* Send pkt to next node */
	  vnet_buffer (b0)->sw_if_index[VLIB_TX] = ~0;
	  vnet_buffer (b1)->sw_if_index[VLIB_TX] = ~0;

	  pkts_processed += 2;

	  if (PREDICT_FALSE ((node->flags & VLIB_NODE_FLAG_TRACE)))
	    {
	      if (b0->flags & VLIB_BUFFER_IS_TRACED)
		{
		  icnpg_trace_t *t =
		    vlib_add_trace (vm, node, b0, sizeof (*t));
		  t->pkt_type = pkt_type0;
		  t->msg_type = msg_type0;
		  t->sw_if_index = sw_if_index0;
		  t->next_index = next0;
		}
	      if (b1->flags & VLIB_BUFFER_IS_TRACED)
		{
		  icnpg_trace_t *t =
		    vlib_add_trace (vm, node, b1, sizeof (*t));
		  t->pkt_type = pkt_type1;
		  t->msg_type = msg_type1;
		  t->sw_if_index = sw_if_index1;
		  t->next_index = next1;
		}
	    }

	  if (next0 == ICNPG_NEXT_DROP)
	    {
	      pkts_dropped++;
	    }
	  if (next1 == ICNPG_NEXT_DROP)
	    {
	      pkts_dropped++;
	    }

	  /* verify speculative enqueues, maybe switch current next frame */
	  vlib_validate_buffer_enqueue_x2 (vm, node, next_index,
					   to_next, n_left_to_next,
					   bi0, bi1, next0, next1);
	}

      while (n_left_from > 0 && n_left_to_next > 0)
	{
	  u32 next0 = ICNPG_NEXT_DROP;
	  u32 sw_if_index0;

	  /* speculatively enqueue b0 to the current next frame */
	  bi0 = from[0];
	  to_next[0] = bi0;
	  from += 1;
	  to_next += 1;
	  n_left_from -= 1;
	  n_left_to_next -= 1;

	  b0 = vlib_get_buffer (vm, bi0);

	  sw_if_index0 = vnet_buffer (b0)->sw_if_index[VLIB_RX];

	  /* We think that the udp code has handed us the payloads,
	   * so we need to walk back to the IP headers
	   */
	  ASSERT (b0->current_data >=
		  (sizeof (ip4_header_t) + sizeof (udp_header_t)));

	  body0 = vlib_buffer_get_current (b0);
	  len0 = b0->current_length;

	  vlib_buffer_advance (b0, -(sizeof (udp_header_t)));
	  udp0 = vlib_buffer_get_current (b0);
	  vlib_buffer_advance (b0, -(sizeof (ip4_header_t)));
	  ip0 = vlib_buffer_get_current (b0);

	  /* Check icn packets, locate names */
	  if (cicn_parse_pkt (body0, len0, &pkt_type0, &msg_type0,
			      &name0, &namelen0, &pkt_hdr_desc0) == AOK)
	    {

	      if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_INTEREST) &&
				(msg_type0 == CICN_MSG_TYPE_INTEREST)))
		{
		  /* Increment the appropriate message counter */
		  interest_msgs_generated++;

		  /* Stuff the counter value into the last name-comp */
		  C_PUTINT64 ((name0 + namelen0 - 8), ipgm->namecounter);
		  ipgm->namecounter += 1ULL;

		  /* Rewrite and send */
		  ip0->src_address.as_u32 = sm->pgen_clt_src_addr;
		  ip0->dst_address.as_u32 = sm->pgen_clt_dest_addr;

		  ip0->checksum = ip4_header_checksum (ip0);

		  udp0->src_port = sm->pgen_clt_src_port;
		  udp0->dst_port = sm->pgen_clt_dest_port;
		  udp0->checksum = 0;

		  next0 = ICNPG_NEXT_LOOKUP;
		}
	      else if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_CONTENT) &&
				     (msg_type0 == CICN_MSG_TYPE_CONTENT)))
		{
		  /* If we receive a content message, increment a counter */
		  content_msgs_received++;
		}
	      else if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_NAK)))
		{
		  /* If we receive a NACK, just increment a counter */
		  nacks_received++;
		}
	    }

	  /* Send pkt to ip lookup */
	  vnet_buffer (b0)->sw_if_index[VLIB_TX] = ~0;

	  if (PREDICT_FALSE ((node->flags & VLIB_NODE_FLAG_TRACE)
			     && (b0->flags & VLIB_BUFFER_IS_TRACED)))
	    {
	      icnpg_trace_t *t = vlib_add_trace (vm, node, b0, sizeof (*t));
	      t->pkt_type = pkt_type0;
	      t->msg_type = msg_type0;
	      t->sw_if_index = sw_if_index0;
	      t->next_index = next0;
	    }

	  pkts_processed += 1;

	  if (next0 == ICNPG_NEXT_DROP)
	    {
	      pkts_dropped++;
	    }

	  /* verify speculative enqueue, maybe switch current next frame */
	  vlib_validate_buffer_enqueue_x1 (vm, node, next_index,
					   to_next, n_left_to_next,
					   bi0, next0);
	}

      vlib_put_next_frame (vm, node, next_index, n_left_to_next);
    }

  vlib_node_increment_counter (vm, icn_pg_node.index,
			       ICNPG_ERROR_PROCESSED, pkts_processed);
  vlib_node_increment_counter (vm, icn_pg_node.index,
			       ICNPG_ERROR_DROPPED, pkts_dropped);
  vlib_node_increment_counter (vm, icn_pg_node.index,
			       ICNPG_ERROR_INTEREST_MSGS_GENERATED,
			       interest_msgs_generated);
  vlib_node_increment_counter (vm, icn_pg_node.index,
			       ICNPG_ERROR_CONTENT_MSGS_RECEIVED,
			       content_msgs_received);
  vlib_node_increment_counter (vm, icn_pg_node.index,
			       ICNPG_ERROR_NACKS_RECEIVED, nacks_received);

  return (frame->n_vectors);
}


/* *INDENT-OFF* */
VLIB_REGISTER_NODE (icn_pg_node) =
{
  .function = icnpg_client_node_fn,
  .name = "icnpg",
  .vector_size = sizeof (u32),
  .format_trace = format_icnpg_trace,
  .type = VLIB_NODE_TYPE_INTERNAL,
  .n_errors = ARRAY_LEN (icnpg_error_strings),
  .error_strings = icnpg_error_strings,
  .n_next_nodes = ICNPG_N_NEXT,
  /* edit / add dispositions here */
  .next_nodes = {
    [ICNPG_NEXT_LOOKUP] = "ip4-lookup",
    [ICNPG_NEXT_DROP] = "ip4-drop",
  }
,};
/* *INDENT-ON* */

/*
 * End of packet-generator client node
 */

/*
 * Beginning of packet-generation server node
 */

/* Registration struct for a graph node */
vlib_node_registration_t icn_pg_server_node;

/* Stats, which end up called "error" even though they aren't... */
#define foreach_icnpg_server_error		\
_(PROCESSED, "ICN PG Server packets processed")	\
_(DROPPED, "ICN PG Server packets dropped")

typedef enum
{
#define _(sym,str) ICNPG_SERVER_ERROR_##sym,
  foreach_icnpg_server_error
#undef _
    ICNPG_SERVER_N_ERROR,
} icnpg_server_error_t;

static char *icnpg_server_error_strings[] = {
#define _(sym,string) string,
  foreach_icnpg_server_error
#undef _
};

/* Next graph nodes, which reference the list in the actual registration
 * block below
 */
typedef enum
{
  ICNPG_SERVER_NEXT_LOOKUP,
  ICNPG_SERVER_NEXT_DROP,
  ICNPG_SERVER_N_NEXT,
} icnpg_server_next_t;

/* Trace context struct */
typedef struct
{
  u32 next_index;
  u32 sw_if_index;
  u8 pkt_type;
  u16 msg_type;
} icnpg_server_trace_t;

/* packet trace format function */
static u8 *
format_icnpg_server_trace (u8 * s, va_list * args)
{
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  icnpg_server_trace_t *t = va_arg (*args, icnpg_server_trace_t *);

  s =
    format (s, "ICNPG SERVER: pkt: %d, msg %d, sw_if_index %d, next index %d",
	    (int) t->pkt_type, (int) t->msg_type, t->sw_if_index,
	    t->next_index);
  return (s);
}

/*
 * Node function for the icn packet-generator server.
 */
static uword
icnpg_node_server_fn (vlib_main_t * vm,
		      vlib_node_runtime_t * node, vlib_frame_t * frame)
{
  u32 n_left_from, *from, *to_next;
  icnpg_server_next_t next_index;
  u32 pkts_processed = 0, pkts_dropped = 0;
  u32 bi0, bi1;
  vlib_buffer_t *b0, *b1;
  u8 pkt_type0 = 0, pkt_type1 = 0;
  cicn_pkt_hdr_desc_t pkt_hdr_desc0, pkt_hdr_desc1;
  u16 msg_type0 = 0, msg_type1 = 0;
  u8 *body0, *body1;
  u32 len0, len1;
  udp_header_t *udp0, *udp1;
  ip4_header_t *ip0, *ip1;
  uint8_t *name0, *name1;
  uint32_t namelen0, namelen1;
  cicn_main_t *sm = &cicn_main;

  from = vlib_frame_vector_args (frame);

  n_left_from = frame->n_vectors;
  next_index = node->cached_next_index;

  while (n_left_from > 0)
    {
      u32 n_left_to_next;

      vlib_get_next_frame (vm, node, next_index, to_next, n_left_to_next);


      while (n_left_from >= 4 && n_left_to_next >= 2)
	{
	  u32 next0 = ICNPG_SERVER_NEXT_DROP;
	  u32 next1 = ICNPG_SERVER_NEXT_DROP;
	  u32 sw_if_index0, sw_if_index1;

	  /* Prefetch next iteration. */
	  {
	    vlib_buffer_t *p2, *p3;

	    p2 = vlib_get_buffer (vm, from[2]);
	    p3 = vlib_get_buffer (vm, from[3]);

	    vlib_prefetch_buffer_header (p2, LOAD);
	    vlib_prefetch_buffer_header (p3, LOAD);

	    CLIB_PREFETCH (p2->data, (2 * CLIB_CACHE_LINE_BYTES), STORE);
	    CLIB_PREFETCH (p3->data, (2 * CLIB_CACHE_LINE_BYTES), STORE);
	  }

	  /* speculatively enqueue b0 and b1 to the current next frame */
	  to_next[0] = bi0 = from[0];
	  to_next[1] = bi1 = from[1];
	  from += 2;
	  to_next += 2;
	  n_left_from -= 2;
	  n_left_to_next -= 2;

	  b0 = vlib_get_buffer (vm, bi0);
	  b1 = vlib_get_buffer (vm, bi1);

	  sw_if_index0 = vnet_buffer (b0)->sw_if_index[VLIB_RX];
	  sw_if_index1 = vnet_buffer (b1)->sw_if_index[VLIB_RX];

	  /* We think that the udp code has handed us the payloads,
	   * so we need to walk back to the IP headers
	   */
	  ASSERT (b0->current_data >=
		  (sizeof (ip4_header_t) + sizeof (udp_header_t)));

	  ASSERT (b1->current_data >=
		  (sizeof (ip4_header_t) + sizeof (udp_header_t)));

	  body0 = vlib_buffer_get_current (b0);
	  len0 = b0->current_length;

	  body1 = vlib_buffer_get_current (b1);
	  len1 = b1->current_length;

	  /* Check icn packets, locate names */
	  if (cicn_parse_pkt (body0, len0, &pkt_type0, &msg_type0,
			      &name0, &namelen0, &pkt_hdr_desc0) == AOK)
	    {

	      if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_INTEREST) &&
				(msg_type0 == CICN_MSG_TYPE_INTEREST)))
		{

		  /* Change message and packet from Interest to Content */
		  *(body0 + 1) = CICN_PKT_TYPE_CONTENT;
		  C_PUTINT16 (body0 + 14, CICN_MSG_TYPE_CONTENT);

		  vlib_buffer_t *rb = NULL;
		  rb = vlib_get_buffer (vm, sm->pgen_svr_buffer_idx);

		  /* Get the packet length */
		  uint16_t pkt_len;
		  C_GETINT16 (pkt_len, body0 + 2);

		  /* Figure out how many bytes we can add to the content
		   *
		   * Rule of thumb: We want the size of the IP packet
		   * to be <= 1400 bytes
		   */
		  u16 bytes_to_copy = rb->current_length;
		  if (bytes_to_copy + pkt_len +
		      sizeof (udp_header_t) + sizeof (ip4_header_t) > 1400)
		    {
		      bytes_to_copy = 1400 - pkt_len - sizeof (ip4_header_t) -
			sizeof (ip4_header_t);
		    }

		  /* Add content to the data packet */
		  u32 index = vlib_buffer_add_data (sm->vlib_main,
						    b0->free_list_index, bi0,
						    rb->data,
						    bytes_to_copy);

		  b0 = vlib_get_buffer (vm, index);
		  body0 = vlib_buffer_get_current (b0);

		  // Update interest lifetime to cache time
		  C_PUTINT16 (body0 + 8, CICN_HDR_TLV_CACHE_TIME);

		  // Update the length of the message
		  uint16_t msg_len;
		  C_GETINT16 (msg_len, body0 + 16);
		  C_PUTINT16 (body0 + 16, msg_len + bytes_to_copy);

		  // Update the length of the packet
		  C_PUTINT16 (body0 + 2, pkt_len + bytes_to_copy);

		  vlib_buffer_advance (b0, -(sizeof (udp_header_t)));
		  udp0 = vlib_buffer_get_current (b0);
		  vlib_buffer_advance (b0, -(sizeof (ip4_header_t)));
		  ip0 = vlib_buffer_get_current (b0);

		  /* Rewrite and send */
		  u32 src_addr = ip0->src_address.as_u32;
		  ip0->src_address.as_u32 = ip0->dst_address.as_u32;
		  ip0->dst_address.as_u32 = src_addr;

		  udp0->length =
		    clib_host_to_net_u16 (vlib_buffer_length_in_chain
					  (sm->vlib_main,
					   b0) - sizeof (ip4_header_t));

		  ip0->length =
		    clib_host_to_net_u16 (vlib_buffer_length_in_chain
					  (sm->vlib_main, b0));

		  ip0->checksum = ip4_header_checksum (ip0);

		  u16 src_port = udp0->src_port;
		  udp0->src_port = udp0->dst_port;
		  udp0->dst_port = src_port;
		  udp0->checksum = 0;

		  next0 = ICNPG_SERVER_NEXT_LOOKUP;
		}
	    }

	  if (cicn_parse_pkt (body1, len1, &pkt_type1, &msg_type1,
			      &name1, &namelen1, &pkt_hdr_desc1) == AOK)
	    {
	      if (PREDICT_TRUE ((pkt_type1 == CICN_PKT_TYPE_INTEREST) &&
				(msg_type1 == CICN_MSG_TYPE_INTEREST)))
		{

		  /* Change message and packet types from Interest to Content */
		  *(body1 + 1) = CICN_PKT_TYPE_CONTENT;
		  C_PUTINT16 (body1 + 14, CICN_MSG_TYPE_CONTENT);

		  vlib_buffer_t *rb = NULL;
		  rb = vlib_get_buffer (vm, sm->pgen_svr_buffer_idx);

		  /* Get the packet length */
		  uint16_t pkt_len;
		  C_GETINT16 (pkt_len, body1 + 2);

		  /* Figure out how many bytes we can add to the content
		   *
		   * Rule of thumb: We want the size of the IP packet
		   * to be <= 1400 bytes
		   */
		  u16 bytes_to_copy = rb->current_length;
		  if (bytes_to_copy + pkt_len +
		      sizeof (udp_header_t) + sizeof (ip4_header_t) > 1400)
		    {
		      bytes_to_copy = 1400 - pkt_len - sizeof (ip4_header_t) -
			sizeof (ip4_header_t);
		    }

		  /* Add content to the data packet */
		  u32 index = vlib_buffer_add_data (sm->vlib_main,
						    b1->free_list_index, bi1,
						    rb->data,
						    bytes_to_copy);

		  b1 = vlib_get_buffer (vm, index);
		  body1 = vlib_buffer_get_current (b1);

		  // Update interest lifetime to cache time
		  C_PUTINT16 (body1 + 8, CICN_HDR_TLV_CACHE_TIME);

		  // Update the length of the message
		  uint16_t msg_len;
		  C_GETINT16 (msg_len, body1 + 16);
		  C_PUTINT16 (body1 + 16, msg_len + bytes_to_copy);

		  // Update the length of the packet
		  C_PUTINT16 (body1 + 2, pkt_len + bytes_to_copy);

		  vlib_buffer_advance (b1, -(sizeof (udp_header_t)));
		  udp1 = vlib_buffer_get_current (b1);
		  vlib_buffer_advance (b1, -(sizeof (ip4_header_t)));
		  ip1 = vlib_buffer_get_current (b1);

		  /* Rewrite and send */
		  u32 src_addr = ip1->src_address.as_u32;
		  ip1->src_address.as_u32 = ip1->dst_address.as_u32;
		  ip1->dst_address.as_u32 = src_addr;

		  udp1->length =
		    clib_host_to_net_u16 (vlib_buffer_length_in_chain
					  (sm->vlib_main,
					   b1) - sizeof (ip4_header_t));

		  ip1->length =
		    clib_host_to_net_u16 (vlib_buffer_length_in_chain
					  (sm->vlib_main, b1));

		  ip1->checksum = ip4_header_checksum (ip1);

		  u16 src_port = udp1->src_port;
		  udp1->src_port = udp1->dst_port;
		  udp1->dst_port = src_port;
		  udp1->checksum = 0;

		  next1 = ICNPG_SERVER_NEXT_LOOKUP;
		}
	    }

	  /* Send pkt to next node */
	  vnet_buffer (b0)->sw_if_index[VLIB_TX] = ~0;
	  vnet_buffer (b1)->sw_if_index[VLIB_TX] = ~0;

	  pkts_processed += 2;

	  if (PREDICT_FALSE ((node->flags & VLIB_NODE_FLAG_TRACE)))
	    {
	      if (b0->flags & VLIB_BUFFER_IS_TRACED)
		{
		  icnpg_server_trace_t *t =
		    vlib_add_trace (vm, node, b0, sizeof (*t));
		  t->pkt_type = pkt_type0;
		  t->msg_type = msg_type0;
		  t->sw_if_index = sw_if_index0;
		  t->next_index = next0;
		}
	      if (b1->flags & VLIB_BUFFER_IS_TRACED)
		{
		  icnpg_server_trace_t *t =
		    vlib_add_trace (vm, node, b1, sizeof (*t));
		  t->pkt_type = pkt_type1;
		  t->msg_type = msg_type1;
		  t->sw_if_index = sw_if_index1;
		  t->next_index = next1;
		}
	    }

	  if (next0 == ICNPG_SERVER_NEXT_DROP)
	    {
	      pkts_dropped++;
	    }
	  if (next1 == ICNPG_SERVER_NEXT_DROP)
	    {
	      pkts_dropped++;
	    }

	  /* verify speculative enqueues, maybe switch current next frame */
	  vlib_validate_buffer_enqueue_x2 (vm, node, next_index,
					   to_next, n_left_to_next,
					   bi0, bi1, next0, next1);
	}

      while (n_left_from > 0 && n_left_to_next > 0)
	{
	  u32 next0 = ICNPG_SERVER_NEXT_DROP;
	  u32 sw_if_index0;

	  /* speculatively enqueue b0 to the current next frame */
	  bi0 = from[0];
	  to_next[0] = bi0;
	  from += 1;
	  to_next += 1;
	  n_left_from -= 1;
	  n_left_to_next -= 1;

	  b0 = vlib_get_buffer (vm, bi0);

	  sw_if_index0 = vnet_buffer (b0)->sw_if_index[VLIB_RX];

	  /* We think that the udp code has handed us the payloads,
	   * so we need to walk back to the IP headers
	   */
	  ASSERT (b0->current_data >=
		  (sizeof (ip4_header_t) + sizeof (udp_header_t)));

	  body0 = vlib_buffer_get_current (b0);
	  len0 = b0->current_length;

	  /* Check icn packets, locate names */
	  if (cicn_parse_pkt (body0, len0, &pkt_type0, &msg_type0,
			      &name0, &namelen0, &pkt_hdr_desc0) == AOK)
	    {

	      if (PREDICT_TRUE ((pkt_type0 == CICN_PKT_TYPE_INTEREST) &&
				(msg_type0 == CICN_MSG_TYPE_INTEREST)))
		{

		  /* Change message and packet types from Interest to Content */
		  *(body0 + 1) = CICN_PKT_TYPE_CONTENT;
		  C_PUTINT16 (body0 + 8, CICN_MSG_TYPE_CONTENT);

		  vlib_buffer_t *rb = NULL;
		  rb = vlib_get_buffer (vm, sm->pgen_svr_buffer_idx);

		  /* Get the packet length */
		  uint16_t pkt_len;
		  C_GETINT16 (pkt_len, body0 + 2);

		  /* Figure out how many bytes we can add to the content
		   *
		   * Rule of thumb: We want the size of the IP packet
		   * to be <= 1400 bytes
		   */
		  u16 bytes_to_copy = rb->current_length;
		  if (bytes_to_copy + pkt_len +
		      sizeof (udp_header_t) + sizeof (ip4_header_t) > 1400)
		    {
		      bytes_to_copy = 1400 - pkt_len - sizeof (ip4_header_t) -
			sizeof (ip4_header_t);
		    }

		  /* Add content to the data packet */
		  u32 index = vlib_buffer_add_data (sm->vlib_main,
						    b0->free_list_index, bi0,
						    rb->data,
						    bytes_to_copy);

		  b0 = vlib_get_buffer (vm, index);
		  body0 = vlib_buffer_get_current (b0);

		  // Update the length of the message
		  uint16_t msg_len;
		  C_GETINT16 (msg_len, body0 + 10);
		  C_PUTINT16 (body0 + 10, msg_len + bytes_to_copy);

		  // Update the length of the packet
		  C_PUTINT16 (body0 + 2, pkt_len + bytes_to_copy);

		  vlib_buffer_advance (b0, -(sizeof (udp_header_t)));
		  udp0 = vlib_buffer_get_current (b0);
		  vlib_buffer_advance (b0, -(sizeof (ip4_header_t)));
		  ip0 = vlib_buffer_get_current (b0);

		  /* Rewrite and send */
		  u32 src_addr = ip0->src_address.as_u32;
		  ip0->src_address.as_u32 = ip0->dst_address.as_u32;
		  ip0->dst_address.as_u32 = src_addr;
		  udp0->length =
		    clib_host_to_net_u16 (vlib_buffer_length_in_chain
					  (sm->vlib_main,
					   b0) - sizeof (ip4_header_t));

		  ip0->length =
		    clib_host_to_net_u16 (vlib_buffer_length_in_chain
					  (sm->vlib_main, b0));

		  ip0->checksum = ip4_header_checksum (ip0);

		  u16 src_port = udp0->src_port;
		  udp0->src_port = udp0->dst_port;
		  udp0->dst_port = src_port;
		  udp0->checksum = 0;

		  next0 = ICNPG_SERVER_NEXT_LOOKUP;
		}
	    }

	  /* Send pkt to ip lookup */
	  vnet_buffer (b0)->sw_if_index[VLIB_TX] = ~0;

	  if (PREDICT_FALSE ((node->flags & VLIB_NODE_FLAG_TRACE)
			     && (b0->flags & VLIB_BUFFER_IS_TRACED)))
	    {
	      icnpg_server_trace_t *t =
		vlib_add_trace (vm, node, b0, sizeof (*t));
	      t->pkt_type = pkt_type0;
	      t->msg_type = msg_type0;
	      t->sw_if_index = sw_if_index0;
	      t->next_index = next0;
	    }

	  pkts_processed += 1;

	  if (next0 == ICNPG_SERVER_NEXT_DROP)
	    {
	      pkts_dropped++;
	    }

	  /* verify speculative enqueue, maybe switch current next frame */
	  vlib_validate_buffer_enqueue_x1 (vm, node, next_index,
					   to_next, n_left_to_next,
					   bi0, next0);
	}

      vlib_put_next_frame (vm, node, next_index, n_left_to_next);
    }

  vlib_node_increment_counter (vm, icn_pg_server_node.index,
			       ICNPG_SERVER_ERROR_PROCESSED, pkts_processed);
  vlib_node_increment_counter (vm, icn_pg_server_node.index,
			       ICNPG_SERVER_ERROR_DROPPED, pkts_dropped);

  return (frame->n_vectors);
}

/* *INDENT-OFF* */
VLIB_REGISTER_NODE (icn_pg_server_node) =
{
  .function = icnpg_node_server_fn,
  .name = "icnpg-server",
  .vector_size = sizeof (u32),
  .format_trace = format_icnpg_server_trace,
  .type =VLIB_NODE_TYPE_INTERNAL,
  .n_errors = ARRAY_LEN (icnpg_server_error_strings),
  .error_strings = icnpg_server_error_strings,
  .n_next_nodes = ICNPG_SERVER_N_NEXT,
  /* edit / add dispositions here */
  .next_nodes = {
    [ICNPG_SERVER_NEXT_LOOKUP] = "ip4-lookup",
    [ICNPG_SERVER_NEXT_DROP] = "ip4-drop",
  }
,};

/* *INDENT-ON* */

/*
 * End of packet-generator server node
 */


/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
