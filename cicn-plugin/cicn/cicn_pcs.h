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
 * cicn_pcs.h: Opportunistic timeout code for the PIT/CS used in the cicn forwarder.
 */

#ifndef _CICN_PCS_H_
#define _CICN_PCS_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include "cicn_hashtb.h"

/* The PIT and CS are stored as a union */
#define CICN_PIT_NULL_TYPE 0
#define CICN_PIT_TYPE      1
#define CICN_CS_TYPE       2

/* Max number of incoming (interest) faces supported, for now. Note that changing
 * this may change alignment within the PIT struct, so be careful.
 */
typedef struct cicn_pcs_shared_s
{

  /* Installation/creation time (vpp float units, for now) */
  f64 create_time;

  /* Expiration time (vpp float units, for now) */
  f64 expire_time;

  /* Shared 'type' octet */
  uint8_t entry_type;

  /* Shared 'flags' octet */
  uint8_t entry_flags;

  /* Shared size 8 + 8 + 2 = 18B */

} cicn_pcs_shared_t;

/*
 * PIT entry, unioned with a CS entry below
 */
typedef struct cicn_pit_entry_s
{

  /* Shared size 8 + 8 + 2 = 18B */

  /* Egress face and array of ingress faces */
  /* 18B + 2B*8 = 34B */
  uint16_t pe_txface;
  uint16_t pe_rxfaces[CICN_PARAM_PIT_ENTRY_PHOPS_MAX];

  /* Bitmap of FIB faces tried (TODO -- needed in first go?) */
  /* 34 + 2B = 36B */
  uint16_t pe_tx_face_map;

  /* FIB entry id (TODO -- why - related to 'faces tried'?) */
  /* 36 + 4B = 40B */
  uint32_t pe_fib_idx;

  /* Packet buffer idx, if held */
  /* 40 + 4B = 44B */
  uint32_t pe_pkt_buf;

} cicn_pit_entry_t;

/*
 * CS entry, unioned with a PIT entry below
 */
typedef struct cicn_cs_entry_s
{

  /* Shared size 8 + 8 + 2 = 18B */

  /* Ingress face */
  /* 2B = 20B */
  uint16_t cs_rxface;

  /* Packet buffer, if held */
  /* 4B = 24B */
  uint32_t cs_pkt_buf;

  /* Linkage for LRU, in the form of hashtable node indexes */
  /* 8B = 32B */
  uint32_t cs_lru_prev;
  uint32_t cs_lru_next;

} cicn_cs_entry_t;

/*
 * Combined PIT/CS entry data structure, embedded in a hashtable entry
 * after the common hashtable preamble struct. This MUST fit in the available
 * (fixed) space in a hashtable node.
 */
typedef struct cicn_pcs_entry_s
{

  cicn_pcs_shared_t shared;

  union
  {
    cicn_pit_entry_t pit;
    cicn_cs_entry_t cs;
  } u;
} cicn_pcs_entry_t;

/*
 * Overall PIT/CS table, based on the common hashtable
 */
typedef struct cicn_pit_cs_s
{

  cicn_hashtb_t *pcs_table;

  /* Counters for PIT/CS entries */
  uint32_t pcs_pit_count;
  uint32_t pcs_cs_count;

  /* TODO -- CS LRU, represented as ... */
  uint32_t pcs_lru_max;
  uint32_t pcs_lru_count;

  /* Indexes to hashtable nodes forming CS LRU */
  uint32_t pcs_lru_head;
  uint32_t pcs_lru_tail;

} cicn_pit_cs_t;

/* Accessor for pit/cs data inside hash table node */
static inline cicn_pcs_entry_t *
cicn_pit_get_data (cicn_hash_node_t * node)
{
  return (cicn_pcs_entry_t *) (cicn_hashtb_node_data (node));
}

/* Init pit/cs data block (usually inside hash table node) */
static inline void
cicn_pit_init_data (cicn_pcs_entry_t * p)
{
  memset (p, 0, sizeof (cicn_pcs_entry_t));
}

/* Wrapper for init/alloc of a new pit/cs */
static inline int
cicn_pit_create (cicn_pit_cs_t * p, uint32_t num_elems)
{
  int ret =
    cicn_hashtb_alloc (&p->pcs_table, num_elems, sizeof (cicn_pcs_entry_t));
  p->pcs_table->ht_flags |= CICN_HASHTB_FLAG_KEY_FMT_NAME;

  p->pcs_pit_count = p->pcs_cs_count = 0;

  p->pcs_lru_max = CICN_PARAM_CS_LRU_DEFAULT;
  p->pcs_lru_count = 0;
  p->pcs_lru_head = p->pcs_lru_tail = 0;

  return (ret);
}

static inline f64
cicn_pcs_get_exp_time (f64 cur_time_sec, uint64_t lifetime_msec)
{
  return (cur_time_sec + ((f64) lifetime_msec) / SEC_MS);
}

/*
 * Configure CS LRU limit. Zero is accepted, means 'no limit', probably not
 * a good choice.
 */
static inline void
cicn_pit_set_lru_max (cicn_pit_cs_t * p, uint32_t limit)
{
  p->pcs_lru_max = limit;
}

/*
 * Accessor for PIT interest counter.
 */
static inline uint32_t
cicn_pit_get_int_count (const cicn_pit_cs_t * pitcs)
{
  return (pitcs->pcs_pit_count);
}

/*
 * Accessor for PIT cs entries counter.
 */
static inline uint32_t
cicn_pit_get_cs_count (const cicn_pit_cs_t * pitcs)
{
  return (pitcs->pcs_cs_count);
}

/*
 * Convert a PIT entry into a CS entry (assumes that the entry is already
 * in the hashtable.)
 * This is primarily here to maintain the internal counters.
 */
static inline int
cicn_pit_to_cs (cicn_pit_cs_t * p, cicn_pcs_entry_t * pcs)
{
  ASSERT (pcs->shared.entry_type = CICN_PIT_TYPE);

  pcs->shared.entry_type = CICN_CS_TYPE;

  p->pcs_pit_count--;
  p->pcs_cs_count++;

  return (AOK);
}

/*
 * Is CS enabled?
 * CICN_FEATURE_CS is tri-valued: value of 2 means do a run-time check
 */
static inline int
cicn_cs_enabled (cicn_pit_cs_t * pit)
{
  switch (CICN_FEATURE_CS)
    {
    case 0:
    default:
      return (0);
    case 1:
      return (1);
    case 2:
      return (pit->pcs_lru_max > 0);
    }
}

/*
 * Insert a new CS element at the head of the CS LRU
 */
static inline void
cicn_cs_lru_insert (cicn_pit_cs_t * p, cicn_hash_node_t * pnode,
		    cicn_pcs_entry_t * pcs)
{
  cicn_hash_node_t *lrunode;
  cicn_pcs_entry_t *lrupcs;
  uint32_t idx;

  idx = cicn_hashtb_node_idx_from_node (p->pcs_table, pnode);

  if (p->pcs_lru_head != 0)
    {
      lrunode = cicn_hashtb_node_from_idx (p->pcs_table, p->pcs_lru_head);
      lrupcs = cicn_pit_get_data (lrunode);

      ASSERT (lrupcs->u.cs.cs_lru_prev == 0);
      lrupcs->u.cs.cs_lru_prev = idx;

      pcs->u.cs.cs_lru_prev = 0;
      pcs->u.cs.cs_lru_next = p->pcs_lru_head;

      p->pcs_lru_head = idx;

    }
  else
    {
      ASSERT (p->pcs_lru_tail == 0);	/* We think the list is empty */

      p->pcs_lru_head = p->pcs_lru_tail = idx;

      pcs->u.cs.cs_lru_next = pcs->u.cs.cs_lru_prev = 0;
    }

  p->pcs_lru_count++;
}

/*
 * Dequeue an LRU element, for example when it has expired.
 */
static inline void
cicn_cs_lru_dequeue (cicn_pit_cs_t * pit, cicn_hash_node_t * pnode,
		     cicn_pcs_entry_t * pcs)
{
  cicn_hash_node_t *lrunode;
  cicn_pcs_entry_t *lrupcs;

  if (pcs->u.cs.cs_lru_prev != 0)
    {
      /* Not already on the head of the LRU */
      lrunode = cicn_hashtb_node_from_idx (pit->pcs_table,
					   pcs->u.cs.cs_lru_prev);
      lrupcs = cicn_pit_get_data (lrunode);

      lrupcs->u.cs.cs_lru_next = pcs->u.cs.cs_lru_next;
    }
  else
    {
      ASSERT (pit->pcs_lru_head ==
	      cicn_hashtb_node_idx_from_node (pit->pcs_table, pnode));
      pit->pcs_lru_head = pcs->u.cs.cs_lru_next;
    }

  if (pcs->u.cs.cs_lru_next != 0)
    {
      /* Not already the end of the LRU */
      lrunode = cicn_hashtb_node_from_idx (pit->pcs_table,
					   pcs->u.cs.cs_lru_next);
      lrupcs = cicn_pit_get_data (lrunode);

      lrupcs->u.cs.cs_lru_prev = pcs->u.cs.cs_lru_prev;
    }
  else
    {
      /* This was the last LRU element */
      ASSERT (pit->pcs_lru_tail ==
	      cicn_hashtb_node_idx_from_node (pit->pcs_table, pnode));
      pit->pcs_lru_tail = pcs->u.cs.cs_lru_prev;
    }

  pit->pcs_lru_count -= 1;
}

/*
 * Move a CS LRU element to the head, probably after it's been used.
 */
static inline void
cicn_cs_lru_update_head (cicn_pit_cs_t * pit, cicn_hash_node_t * pnode,
			 cicn_pcs_entry_t * pcs)
{

  if (pcs->u.cs.cs_lru_prev != 0)
    {
      /* Not already on the head of the LRU, detach it from its current
       * position
       */
      cicn_cs_lru_dequeue (pit, pnode, pcs);

      /* Now detached from the list; attach at head */
      cicn_cs_lru_insert (pit, pnode, pcs);

    }
  else
    {
      ASSERT (pit->pcs_lru_head ==
	      cicn_hashtb_node_idx_from_node (pit->pcs_table, pnode));
    }
}

/*
 * Remove a batch of nodes from the CS LRU, copying their node indexes into
 * the caller's array. We expect this is done when the LRU size exceeds
 * the CS's limit.
 */
static inline int
cicn_cs_lru_trim (cicn_pit_cs_t * pit, uint32_t * node_list, int sz)
{
  cicn_hash_node_t *lrunode;
  cicn_pcs_entry_t *lrupcs;
  uint32_t idx;
  int i;

  idx = pit->pcs_lru_tail;

  for (i = 0; i < sz; i++)
    {

      if (idx == 0)
	{
	  break;
	}

      lrunode = cicn_hashtb_node_from_idx (pit->pcs_table, idx);
      lrupcs = cicn_pit_get_data (lrunode);

      node_list[i] = idx;

      idx = lrupcs->u.cs.cs_lru_prev;
    }

  pit->pcs_lru_count -= i;

  pit->pcs_lru_tail = idx;
  if (idx != 0)
    {
      lrunode = cicn_hashtb_node_from_idx (pit->pcs_table, idx);
      lrupcs = cicn_pit_get_data (lrunode);

      lrupcs->u.cs.cs_lru_next = 0;
    }
  else
    {
      /* If the tail is empty, the whole lru is empty */
      pit->pcs_lru_head = 0;
    }

  return (i);
}

/*
 * Insert PIT/CS entry into the hashtable
 * The main purpose of this wrapper is helping maintain the per-PIT stats.
 */
static inline int
cicn_pit_insert (cicn_pit_cs_t * pitcs, cicn_pcs_entry_t * entry,
		 cicn_hash_node_t * node)
{
  int ret;

  ASSERT (entry == cicn_hashtb_node_data (node));

  ret = cicn_hashtb_insert (pitcs->pcs_table, node);
  if (ret == AOK)
    {
      if (entry->shared.entry_type == CICN_PIT_TYPE)
	{
	  pitcs->pcs_pit_count++;
	}
      else
	{
	  pitcs->pcs_cs_count++;
	}
    }

  return (ret);
}

/*
 * Delete a PIT/CS entry from the hashtable, freeing the hash node struct.
 * The caller's pointers are zeroed!
 * If cs_trim is true, entry has already been removed from lru list
 * The main purpose of this wrapper is helping maintain the per-PIT stats.
 */
static inline int
cicn_pcs_delete_internal (cicn_pit_cs_t * pitcs,
			  cicn_pcs_entry_t ** pcs_entryp,
			  cicn_hash_node_t ** nodep, vlib_main_t * vm,
			  int cs_trim)
{
  int ret;
  cicn_pcs_entry_t *pcs = *pcs_entryp;

  ASSERT (pcs == cicn_hashtb_node_data (*nodep));

  if (pcs->shared.entry_type == CICN_PIT_TYPE)
    {
      pitcs->pcs_pit_count--;
    }
  else
    {
      pitcs->pcs_cs_count--;
      // Clean up LRU queue unless entry already removed by bulk CS LRU trim
      if (!cs_trim)
	{
	  cicn_cs_lru_dequeue (pitcs, *nodep, pcs);
	}
      /* Free any associated packet buffer */
      if (pcs->u.cs.cs_pkt_buf != 0)
	{
	  BUFTRC ("PCS-DEL", pcs->u.cs.cs_pkt_buf);
	  vlib_buffer_free_one (vm, pcs->u.cs.cs_pkt_buf);
	  pcs->u.cs.cs_pkt_buf = 0;
	}
    }

  ret = cicn_hashtb_delete (pitcs->pcs_table, nodep);
  *pcs_entryp = NULL;

  return (ret);
}

/*
 * Delete entry normally
 */
static inline int
cicn_pcs_delete (cicn_pit_cs_t * pitcs, cicn_pcs_entry_t ** pcs_entryp,
		 cicn_hash_node_t ** nodep, vlib_main_t * vm)
{
  return (cicn_pcs_delete_internal
	  (pitcs, pcs_entryp, nodep, vm, 0 /*!cs_trim */ ));

}

/*
 * Delete entry which has already been bulk-removed from lru list
 */
static inline int
cicn_cs_delete_trimmed (cicn_pit_cs_t * pitcs, cicn_pcs_entry_t ** pcs_entryp,
			cicn_hash_node_t ** nodep, vlib_main_t * vm)
{
  return (cicn_pcs_delete_internal
	  (pitcs, pcs_entryp, nodep, vm, 1 /*cs_trim */ ));

}

/*
 * Other than the hash value, the arguments are all context needed
 * to update the worker context.
 */
int
cicn_pcs_timeout (vlib_main_t * vm,
		  cicn_pit_cs_t * pitcs, uint64_t h, u32 * pec, u32 * cec);


#endif /* _CICN_PCS_H_ */
