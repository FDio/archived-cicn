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
 * Part of cicn plugin's  dpdk/rte shim layer for using dpdk/rte mechanisms
 * directly while hiding that fact from the bulk of the cicn plugin coce.
 * - cicn plugin should not be looking at dpdk headers and should not need
 *    to. As of v17.01, howeverhowever, buffer cloning to support 0-copy on
 *    - content message replication
 *    - content message transmission based on CS hits
 *      is only available with dpdk, hence those mechanisms are used
 *      by cicn plugin.)
 * - when vlib_buffer cloning support is provided, this shim layer
 *   can be deprecated/deleted, and cicn plugin will be simpler and will
 *   be able to run with a vpp that does not include dpdk.
 * This file contains the code to use dpdk "struct rte_mbuf *" buffer
 * headers for 0-copy cloning of content messages that are in CS, while
 * hiding these references from the cicn plugin main code.
 */
#ifndef _CICN_RTE_MBUF_INLINES_H_
#define _CICN_RTE_MBUF_INLINES_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <cicn/cicn_rte_mbuf.h>
#include <vlib/vlib.h>

/*
 * Wrapper for buffer allocation that returns pointer rather than index
 */
static inline vlib_buffer_t *
cicn_infra_vlib_buffer_alloc (vlib_main_t * vm, vlib_buffer_free_list_t * fl,
			      unsigned socket_id,
			      cicn_face_db_entry_t * outface)
{
  vlib_buffer_t *b0;
  u32 bi0;
  if (vlib_buffer_alloc (vm, &bi0, 1) != 1)
    {
      b0 = 0;
      goto done;
    }
  b0 = vlib_get_buffer (vm, bi0);

done:
  return (b0);
}

/*
 * Wrapper for buffer free that uses pointer rather than index
 */
static inline void
cicn_infra_vlib_buffer_free (vlib_buffer_t * b0, vlib_main_t * vm,
			     cicn_face_db_entry_t * outface)
{
  u32 bi0 = vlib_get_buffer_index (vm, b0);
  vlib_buffer_free_one (vm, bi0);
}

#if CICN_FEATURE_VPP_VLIB_CLONING	// to cut over, need API from vpp gerrit 4872
/*
 * Long-term, vlib_buffer_clone() API will be supported and
 * the cicn_rte_mbuf*.h files and all references to rte_mbuf can be removed from
 * cicn plugin, which will then perform better and be linkable with vpp-lite.
 *
 * For a brief interim, can leave this file but
 * with #define CICN_FEATURE_VPP_VLIB_CLONING 1
 * Some code below (e.g. cicn_infra_vlib_buffer_clone_attach_finalize()
 * contents) must be moved to node.c.
 *
 * See comments on alternate definition under !CICN_FEATURE_VPP_VLIB_CLONING
 */

/*
 * not used if rte not used.
 */
static inline unsigned
cicn_infra_rte_socket_id (void)
{
  return (0);
}

static inline void
cicn_infra_vlib_buffer_cs_prep_finalize (vlib_main_t * vm,
					 vlib_buffer_t * cs_b0)
{
  // No action
}

static inline vlib_buffer_t *
cicn_infra_vlib_buffer_clone (vlib_buffer_t * src_b0, vlib_main_t * vm,
			      vlib_buffer_free_list_t * fl,
			      unsigned socket_id,
			      cicn_face_db_entry_t * outface)
{
  return (vlib_buffer_clone (src_b0));
}

/*
 * Force dpdk drivers to rewalk chain that has been changed
 */
static inline void
cicn_infra_vlib_buffer_clone_attach_finalize (vlib_buffer_t * hdr_b0,
					      vlib_buffer_t * clone_b0)
{
  // no action
}
#else // !CICN_FEATURE_VPP_VLIB_CLONING

/*
 * Replacement for rte_mempool_get_bulk():
 * - rte_mempool_get_bulk() does not coexist with vlib_buffer_free(): vpp
 *   runs out of buffers (even when only 1 buffer is being allocated per call).
 * - this replacement instead calls vlib_buffer_alloc(), which does coexist
 *   with vlib_buffer_free().
 */
static inline int
cicn_infra_pvt_rte_mempool_get_bulk (vlib_main_t * vm,
				     struct rte_mempool *rmp,
				     void **rte_mbufs, u32 new_bufs)
{
  u32 bi_bufs[5];

  int i;
  ASSERT (new_bufs <= ARRAY_LEN (bi_bufs));

  if (vlib_buffer_alloc (vm, bi_bufs, new_bufs) != new_bufs)
    {
      return -ENOENT;
    }
  for (i = 0; i < new_bufs; i++)
    {
      vlib_buffer_t *b0 = vlib_get_buffer (vm, bi_bufs[i]);
      rte_mbufs[i] = rte_mbuf_from_vlib_buffer (b0);
    }
  return (0);
}

// #include <vnet/dpdk_replication.h> // copied/modified below

/*
 * Modified copy of .../vpp/vnet/vnet/dpdk_replication.h:
 * - maintain foreign indentation for easier comparison
 * - call cicn_infra_pvt_rte_mempool_get_bulk() in place of calling
 *   rte_mempool_get_bulk(), avoiding the issue described at
 *   cicn_infra_pvt_rte_mempool_get_bulk(), above.
 */
static inline vlib_buffer_t *
cicn_infra_pvt_vlib_dpdk_copy_buffer (vlib_main_t * vm, vlib_buffer_t * b)
{
  u32 new_buffers_needed = 1;
  unsigned socket_id = rte_socket_id ();
  struct rte_mempool *rmp = vm->buffer_main->pktmbuf_pools[socket_id];
  struct rte_mbuf *rte_mbufs[5];
  vlib_buffer_free_list_t *fl;
  vlib_buffer_t *rv;
  u8 *copy_src, *copy_dst;
  vlib_buffer_t *src_buf, *dst_buf;

  fl = vlib_buffer_get_free_list (vm, VLIB_BUFFER_DEFAULT_FREE_LIST_INDEX);

  if (PREDICT_FALSE (b->flags & VLIB_BUFFER_NEXT_PRESENT))
    {
      vlib_buffer_t *tmp = b;
      int i;

      while (tmp->flags & VLIB_BUFFER_NEXT_PRESENT)
	{
	  new_buffers_needed++;
	  tmp = vlib_get_buffer (vm, tmp->next_buffer);
	}

      /* Should never happen... */
      if (PREDICT_FALSE (new_buffers_needed > ARRAY_LEN (rte_mbufs)))
	{
	  clib_warning ("need %d buffers", new_buffers_needed);
	  return 0;
	}

#if 0				// bug workaround: vlib_buffer_free() of these does not work right
      if (rte_mempool_get_bulk (rmp, (void **) rte_mbufs,
				new_buffers_needed) < 0)
	return 0;
#else
      if (cicn_infra_pvt_rte_mempool_get_bulk (vm, rmp, (void **) rte_mbufs,
					       new_buffers_needed) < 0)
	return 0;
#endif

      src_buf = b;
      rv = dst_buf = vlib_buffer_from_rte_mbuf (rte_mbufs[0]);
      vlib_buffer_init_for_free_list (dst_buf, fl);
      copy_src = b->data + src_buf->current_data;
      copy_dst = dst_buf->data + src_buf->current_data;

      for (i = 0; i < new_buffers_needed; i++)
	{
	  clib_memcpy (copy_src, copy_dst, src_buf->current_length);
	  dst_buf->current_data = src_buf->current_data;
	  dst_buf->current_length = src_buf->current_length;
	  dst_buf->flags = src_buf->flags;

	  if (i == 0)
	    {
	      dst_buf->total_length_not_including_first_buffer =
		src_buf->total_length_not_including_first_buffer;
	      vnet_buffer (dst_buf)->sw_if_index[VLIB_RX] =
		vnet_buffer (src_buf)->sw_if_index[VLIB_RX];
	      vnet_buffer (dst_buf)->sw_if_index[VLIB_TX] =
		vnet_buffer (src_buf)->sw_if_index[VLIB_TX];
	      vnet_buffer (dst_buf)->l2 = vnet_buffer (b)->l2;
	    }

	  if (i < new_buffers_needed - 1)
	    {
	      src_buf = vlib_get_buffer (vm, src_buf->next_buffer);
	      dst_buf = vlib_buffer_from_rte_mbuf (rte_mbufs[i + 1]);
	      vlib_buffer_init_for_free_list (dst_buf, fl);
	      copy_src = src_buf->data;
	      copy_dst = dst_buf->data;
	    }
	}
      return rv;
    }

#if 0				// bug workaround: vlib_buffer_free() of these does not work right
  if (rte_mempool_get_bulk (rmp, (void **) rte_mbufs, 1) < 0)
    return 0;
#else
  if (cicn_infra_pvt_rte_mempool_get_bulk (vm, rmp, (void **) rte_mbufs, 1) <
      0)
    return 0;
#endif

  rv = vlib_buffer_from_rte_mbuf (rte_mbufs[0]);
  vlib_buffer_init_for_free_list (rv, fl);

  clib_memcpy (rv->data + b->current_data, b->data + b->current_data,
	       b->current_length);
  rv->current_data = b->current_data;
  rv->current_length = b->current_length;
  vnet_buffer (rv)->sw_if_index[VLIB_RX] =
    vnet_buffer (b)->sw_if_index[VLIB_RX];
  vnet_buffer (rv)->sw_if_index[VLIB_TX] =
    vnet_buffer (b)->sw_if_index[VLIB_TX];
  vnet_buffer (rv)->l2 = vnet_buffer (b)->l2;

  return (rv);
}

/*
 * Could call rte_socket_id() wherever needed, not sure how expensive it is.
 * For now, export and cache.
 */
static inline unsigned
cicn_infra_rte_socket_id (void)
{
  return (rte_socket_id ());
}

/*
 * For cs_pref, update rte_mbuf fields to correspond to vlib_buffer fields.
 * (Probably could be skipped for non-dpdk drivers that must use copying.)
 */
static inline void
cicn_infra_vlib_buffer_cs_prep_finalize (vlib_main_t * vm,
					 vlib_buffer_t * cs_b0)
{
  /* Adjust the dpdk buffer header, so we can use this copy for
   * future cache hits.
   * - if dpdk buffer header invalid (e.g. content msg arrived on veth intfc,
   *   initialize it.
   * - effectively, advanceg the mbuf past the incoming IP and UDP headers,
   *   so that the buffer points  to the start of the ICN payload that is
   *   to be replicated.
   */
  struct rte_mbuf *cs_mb0;
  i16 delta;

  cs_mb0 = rte_mbuf_from_vlib_buffer (cs_b0);
  if ((cs_b0->flags & VNET_BUFFER_RTE_MBUF_VALID) == 0)
    {
      rte_pktmbuf_reset (cs_mb0);
    }

  delta = vlib_buffer_length_in_chain (vm, cs_b0) - (i16) (cs_mb0->pkt_len);

  cs_mb0->data_len += delta;
  cs_mb0->pkt_len += delta;
  cs_mb0->data_off = (RTE_PKTMBUF_HEADROOM + cs_b0->current_data);
}

/*
 * Wrapper for buffer "cloning" that uses
 * - rte_mbuf buffer cloning for dpdk drivers that support cloning
 * - vlib buffer copying for non-dpdk drivers that must use copying.
 *
 * CICN multicast support from vpp is currently problematic.
 * Three mechanisms on offer, CICN currently uses [1] for physical
 * output faces and [3] for virtual output faces:
 * 1. rte_mbuf's rte_pktmbuf_clone()
 *    - advantages
 *      - PIT deaggregation (multicast) case
 *        - high-performance creation of clone chains (relying on
 *          reference-counting mechanism)
 *        - avoids copying
 *        - allows parallel transmission
 *      - CS hit case
 *        - allows modular handling of sending content and deleting CS entries
 *          (relying on reference counting mechanism)
 *    - disadvantages
 *      - requires allocating indirect buffers, which has a cost even
 *        without copying (but Content messages are generally large)
 *      - rte_pktmbufs are a DPDK mechanism
 *        - not supported by non-DPDK (i.e. virtual) drivers
 *        - not supported by vpp-lite, which is used for unit test
 * 2. recycling-based replication (recirculation)
 *    - advantages
 *      - avoids copying
 *      - currently approved by vpp team
 *    - disadvantages
 *      - increased latency since need to transmit copies serially since
 *        only one buffer
 *      - mechanism not quite yet fully supported: notification that
 *        transmission <n> has occurred and recycle for transmission <n+1>
 *        may start does not occur on transmission completion, but on next
 *        transmission on that interface
 * 3. cicn_infra_pvt_vlib_dpdk_copy_buffer (was vlib_dpdk_clone_buffer())
 *    - advantages
 *      - works in both cases, for all drivers
 *    - disadvantages
 *      - slow, due to copying
 */
static inline vlib_buffer_t *
cicn_infra_vlib_buffer_clone (vlib_buffer_t * src_b0, vlib_main_t * vm,
			      vlib_buffer_free_list_t * fl,
			      unsigned socket_id,
			      cicn_face_db_entry_t * outface)
{
  vlib_buffer_t *dst_b0;

  if (outface->swif_cloning_supported)
    {
      vlib_buffer_main_t *bm = vm->buffer_main;
      struct rte_mbuf *src_mb0 = rte_mbuf_from_vlib_buffer (src_b0);
      struct rte_mbuf *dst_mb0;
      dst_mb0 = rte_pktmbuf_clone (src_mb0, bm->pktmbuf_pools[socket_id]);
      if (dst_mb0 == 0)
	{
	  dst_b0 = 0;
	  goto done;
	}

      // rte_mbuf_clone uses rte_mbuf (dpdk) buffer header:
      // copy relevant value to vlib_buffer_t header
      dst_b0 = vlib_buffer_from_rte_mbuf (dst_mb0);
      vlib_buffer_init_for_free_list (dst_b0, fl);
      ASSERT (dst_b0->current_data == 0);
      dst_b0->current_data = src_b0->current_data;
      dst_b0->current_length = dst_mb0->data_len;
    }
  else
    {
      dst_b0 = cicn_infra_pvt_vlib_dpdk_copy_buffer (vm, src_b0);
      if (dst_b0 == 0)
	{
	  goto done;
	}
    }

  //TODO: af_packet device.c chain walker ignores VLIB_BUFFER_NEXT_PRESENT
  //      clear next_buffer to maintain buffer sanity
  ASSERT ((dst_b0->flags & VLIB_BUFFER_NEXT_PRESENT) == 0);
  if (!(dst_b0->flags & VLIB_BUFFER_NEXT_PRESENT))
    {
      dst_b0->next_buffer = 0;
    }
  ASSERT ((dst_b0->flags & VNET_BUFFER_RTE_MBUF_VALID) == 0);

done:
  return (dst_b0);
}

/*
 * For clone attach, vlib_buffer chain is being changed, invalidating
 * rte_mbuf chain (if present). Update the rte_mbuf chain information to
 * be valid.
 */
static inline void
cicn_infra_vlib_buffer_clone_attach_finalize (vlib_buffer_t * hdr_b0,
					      vlib_buffer_t * clone_b0,
					      cicn_face_db_entry_t * outface)
{
  struct rte_mbuf *hdr_mb0;
  struct rte_mbuf *clone_mb0;
  int hdr_rte_mbuf_valid;

  hdr_mb0 = rte_mbuf_from_vlib_buffer (hdr_b0);
  clone_mb0 = rte_mbuf_from_vlib_buffer (clone_b0);

  hdr_rte_mbuf_valid = ((hdr_b0->flags & VNET_BUFFER_RTE_MBUF_VALID) != 0);
  ASSERT ((clone_b0->flags & VNET_BUFFER_RTE_MBUF_VALID) == 0);

  /* Update main rte_mbuf fields, even for non-dkdk output interfaces */
  if (!hdr_rte_mbuf_valid)
    {
      rte_pktmbuf_reset (hdr_mb0);
    }
  hdr_mb0->data_len = hdr_b0->current_length;
  hdr_mb0->pkt_len = hdr_b0->current_length +
    hdr_b0->total_length_not_including_first_buffer;
  hdr_mb0->next = clone_mb0;
  hdr_mb0->nb_segs = clone_mb0->nb_segs + 1;

  if (!outface->swif_is_dpdk_driver)
    {
      goto done;
    }

  hdr_b0->flags |= VNET_BUFFER_RTE_MBUF_VALID;
  clone_b0->flags |= VNET_BUFFER_RTE_MBUF_VALID;

  /* copy metadata from source packet (see sr_replicate.c) */
  hdr_mb0->port = clone_mb0->port;
  hdr_mb0->vlan_tci = clone_mb0->vlan_tci;
  hdr_mb0->vlan_tci_outer = clone_mb0->vlan_tci_outer;
  hdr_mb0->tx_offload = clone_mb0->tx_offload;
  hdr_mb0->hash = clone_mb0->hash;

  hdr_mb0->ol_flags = clone_mb0->ol_flags & ~(IND_ATTACHED_MBUF);

  __rte_mbuf_sanity_check (hdr_mb0, 1);

done:;
}
#endif // !CICN_FEATURE_VPP_VLIB_CLONING

#endif // CICN_RTE_MBUF_INLINES_H_
