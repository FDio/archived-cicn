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
 * cicn_pcs.c: Opportunistic timeout code for the PIT/CS used in the cicn forwarder.
 */

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include <vlib/vlib.h>
#include <vppinfra/pool.h>

#include <cicn/cicn.h>
#include <cicn/cicn_hashtb.h>
#include <cicn/cicn_pcs.h>

/*
 * Calling worker thread context, passed in and bundled up
 * to be passed to the bucket scanning code to enable updating
 * data-stuctures in the event of deletions.
 */
typedef struct cicn_pcs_worker_ctx_s
{				/* worker thread context */
  vlib_main_t *vm;		/* vpp */
  cicn_pit_cs_t *pitcs;		/* PIT/CS table */
  uint64_t h;			/* hash */
  u32 *pec;			/* PIT Expired Count */
  u32 *cec;			/* CS Expired Count */

  cicn_hashtb_h ht;		/* derived from .pitcs */
} cicn_pcs_worker_ctx_t;

/*
 * Overflow bucket context: as a bucket is scanned, maintain
 * the location and count of occupied and empty (free) entries
 * to enable bucket compaction.
 */
typedef struct cicn_hashtb_bucket_ctx_s
{				/* bucket context */
  cicn_hash_bucket_t *bucket;
  int occupied[CICN_HASHTB_BUCKET_ENTRIES];	/* occupied */
  int noccupied;		/* occupied */
  int empty[CICN_HASHTB_BUCKET_ENTRIES];	/* free */
  int nempty;			/* free */
} cicn_hashtb_bucket_ctx_t;

/*
 * Free an overflow bucket from a hashtable (derived
 * from the static function in cicn_hashtb.c).
 */
static void
cicn_free_overflow_bucket (cicn_hashtb_h ht, cicn_hash_bucket_t * bucket)
{
  ASSERT (ht->ht_overflow_buckets_used > 0);

  pool_put (ht->ht_overflow_buckets, bucket);
  ht->ht_overflow_buckets_used--;
}

/*
 * Scan a single bucket (8 entries) for timed-out entries.
 *
 * Recursive function, for scanning chain of buckets.
 * - Bucket chains should be short, so recursion should not be deep.
 *   (If bucket chains are long, either hash table is dimensioned too small or
 *    hash function is not distributing names effectively.)
 * - Find and clear out timed out entries on the way down the recursion.
 * - Compact entries and free unused overflow buckets (if possible) on the
 *   way back up the recursion.
 *
 *
 * Recursion in detail:
 * - pre-recursion processing
 *   - scan of the supplied bucket and cleanup of expired entries
 * - recursion processing:
 *   - if a bucket follows supplied bucket, recurse
 * - post-recursion processing:
 *   - if supplied bucket is head of chain (pbctx == 0), done
 *   - if supplied bucket is non-head element of chain, try to compact
 *     entries into supplied parent of supplied bucket and free supplied
 *     bucket if it ends up empty.
 *     - buckets are freed from the tail backwards
 *     - recursive call can have caused supplied bucket to pick up new
 *       entries from its child, so need to rescan supplied bucket after
 *       the recursive call.
 *
 * Arguments are:
 * wctx:	worker context for updating datastructures
 *		at the vpp node level;
 * pbctx:	bucket context of the calling (parent) instance
 *		of cicn_pcs_timeout_opportunity();
 * bucket:	the bucket to scan.
 */
static int
cicn_pcs_timeout_opportunity (cicn_pcs_worker_ctx_t * wctx,
			      cicn_hashtb_bucket_ctx_t * pbctx,
			      cicn_hash_bucket_t * bucket)
{
  int i, r;
  uint16_t timeout;
  cicn_hashtb_h ht;
  cicn_hash_bucket_t *b;
  cicn_hash_node_t *node;
  cicn_hash_entry_t *entry;
  cicn_pcs_entry_t *pcs;
  cicn_hashtb_bucket_ctx_t bctx;

  /*
   * Initialise the bucket context for this scan;
   * if this bucket has an overflow entry, the context
   * will be passed to it (and seen as pbctx).
   */
  memset (&bctx, 0, sizeof (bctx));
  bctx.bucket = bucket;

  /*
   * Scan the bucket for expired entries and release them,
   * updating bctx with the location and count of occupied
   * and empty entries.
   */
  ht = wctx->ht;
  for (i = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++)
    {
      entry = &bucket->hb_entries[i];
      if (entry->he_node == 0)
	{
	  bctx.empty[bctx.nempty++] = i;
	  continue;
	}
      if (entry->he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	{
	  if (i != CICN_HASHTB_BUCKET_ENTRIES - 1)
	    assert (!(entry->he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW));
	  bctx.occupied[bctx.noccupied++] = i;
	  break;
	}

      if (entry->he_flags & CICN_HASH_ENTRY_FLAG_DELETED)
	{
	  bctx.occupied[bctx.noccupied++] = i;
	  continue;
	}

      if (entry->he_flags & CICN_HASH_ENTRY_FLAG_FAST_TIMEOUT)
	{
	  timeout = cicn_infra_fast_timer;
	}
      else
	{
	  timeout = cicn_infra_slow_timer;
	}
      if (cicn_infra_seq16_gt (entry->he_timeout, timeout))
	{
	  bctx.occupied[bctx.noccupied++] = i;
	  continue;
	}

      /*
       * Entry has timed out, update the relevant statistics
       * at the vpp node level and release the resources; the entry
       * is now counted as empty.
       * Parallel to cicn_pcs_delete(): cannot call cicn_pcs_delete()
       * since that can call cicn_hashtb_delete() and cause supplied
       * bucket to get freed in middle of chain scan.
       */
      node = pool_elt_at_index (ht->ht_nodes, entry->he_node);
      pcs = cicn_pit_get_data (node);
      switch (pcs->shared.entry_type)
	{
	default:
	  /*
	   * Should not happen, not sure how to signal this?
	   * Count the bucket as occupied and continue? or...
	   * assert(entry->he_flags & (CICN_CS_TYPE|CICN_PIT_TYPE));
	   */
	  bctx.occupied[bctx.noccupied++] = i;
	  continue;

	case CICN_PIT_TYPE:
	  wctx->pitcs->pcs_pit_count--;
	  (*wctx->pec)++;
	  break;

	case CICN_CS_TYPE:
	  wctx->pitcs->pcs_cs_count--;
	  /* Clean up CS LRU */
	  cicn_cs_lru_dequeue (wctx->pitcs, node, pcs);
	  if (pcs->u.cs.cs_pkt_buf != 0)
	    {
	      BUFTRC ("  CS-TO", pcs->u.cs.cs_pkt_buf);
	      vlib_buffer_free_one (wctx->vm, pcs->u.cs.cs_pkt_buf);
	      pcs->u.cs.cs_pkt_buf = 0;
	    }
	  (*wctx->cec)++;
	  break;
	}
      cicn_hashtb_init_entry (entry, 0, 0ll);
      cicn_hashtb_free_node (ht, node);

      bctx.empty[bctx.nempty++] = i;
    }

  /* recursion phase: recursively process child of this bucket, if any
   * - entry conveniently points to the supplied bucket's last entry,
   *   which indicates if another bucket is present in the bucket chain
   */
  r = AOK;
  if (entry->he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
    {
      b = pool_elt_at_index (ht->ht_overflow_buckets, entry->he_node);
      r = cicn_pcs_timeout_opportunity (wctx, &bctx, b);
      if (r != AOK)
	{
	  goto done;
	}
    }

  /*
   * post-recursion phase, case 1:
   * - supplied bucket is head bucket, no further compaction required
   */
  if (pbctx == 0)
    {
      r = AOK;
      goto done;
    }

  /*
   * post-recursion phase, case 2:
   * - supplied bucket is non-head (aka overflow) bucket, try to compact
   *   supplied bucket's entries into supplied parent of supplied bucket
   *   - pbctx is parent, try to compact entries into it from this bucket
   *     if room exists
   *   - pbctx is guaranteed still valid since not considered for
   *     freeing until this routine returns.
   *   - because child of this this bucket may have compacted its entries
   *     into supplied bucket, rescan supplied bucket to reinitialise
   *     the context.
   * - if supplied bucket ends up empty, free it
   *   - supplied bucket is empty through combination of its entries
   *     timing out, no child entries got compressed into it,
   *     and/or supplied buckets entries get compressed into parent
   */

  /* rescan */
  memset (&bctx, 0, sizeof (bctx));
  bctx.bucket = bucket;
  for (i = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++)
    {
      entry = &bucket->hb_entries[i];
      if (entry->he_node == 0)
	{
	  bctx.empty[bctx.nempty++] = i;
	  continue;
	}
      bctx.occupied[bctx.noccupied++] = i;
    }

  /*
   * Try to move any entries up to the parent bucket.
   * Always set entry at the top of the loop before checking there is
   * room in the parent so it will point to the first valid entry not
   * moved up to the parent if the loop is exited before either all
   * are copied or only an overflow bucket entry is left.
   */
  for (entry = 0, i = 0; i < bctx.noccupied; i++)
    {
      entry = &bucket->hb_entries[bctx.occupied[i]];
      if (pbctx->nempty == 0)
	{
	  break;
	}
      if (entry->he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	{
	  assert (i == bctx.noccupied - 1);
	  break;
	}

      pbctx->bucket->hb_entries[pbctx->empty[--pbctx->nempty]] = *entry;
      cicn_hashtb_init_entry (entry, 0, 0ll);
    }

  /*
   * How many are left in this bucket?
   */
  switch (bctx.noccupied - i)
    {
    default:
      /*
       * Couldn't empty all the entries in this overflow bucket,
       * maybe next time...
       */
      break;

    case 0:
      /*
       * This overflow bucket is empty, clear the parent's overflow entry
       * and release this bucket.
       */
      cicn_hashtb_init_entry (&pbctx->
			      bucket->hb_entries[CICN_HASHTB_BUCKET_ENTRIES -
						 1], 0, 0ll);
      cicn_free_overflow_bucket (ht, bucket);
      break;

    case 1:
      /*
       * If it's an overflow bucket entry, can move it to the parent's
       * overflow bucket entry (which points here) and free this bucket;
       * similarly for a non-overflow bucket entry, unless the hashtable has
       * CICN_HASHTB_FLAG_USE_SEVEN set, in which case there's nothing to be
       * done - already checked the parent has no free space elsewhere.
       */
      if ((entry->he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW) ||
	  !(ht->ht_flags & CICN_HASHTB_FLAG_USE_SEVEN))
	{
	  pbctx->bucket->hb_entries[CICN_HASHTB_BUCKET_ENTRIES - 1] = *entry;
	  cicn_free_overflow_bucket (ht, bucket);
	}
      break;
    }

  r = AOK;

done:
  return (r);
}

/*
 * Opportunistic timeout:
 * given a hash value and some context, scan all the entries in the
 * relevant hashtable bucket (and any overflow buckets it may have)
 * for entries that have timed out and free them;
 * as a side effect, try to compact and free any overflow buckets.
 *
 * Could perhaps be generalised to other functions requiring a scan
 * of a hashtable bucket, or easily adapted to using a timer-wheel if
 * opportunistic scanning was found to be inadeqaute.
 */
int
cicn_pcs_timeout (vlib_main_t * vm,
		  cicn_pit_cs_t * pitcs, uint64_t h, u32 * pec, u32 * cec)
{
  uint32_t bidx;
  cicn_hashtb_h ht;
  cicn_hash_bucket_t *bucket;
  cicn_pcs_worker_ctx_t wctx;

  /*
   * Construct the worker thread context passed to the actual scan
   * routine - it needs to be able to update datastuctures.
   */
  memset (&wctx, 0, sizeof (wctx));
  wctx.vm = vm;
  wctx.pitcs = pitcs;
  ht = pitcs->pcs_table;
  wctx.ht = ht;
  wctx.h = h;
  wctx.pec = pec;
  wctx.cec = cec;

  /*
   * Locate the bucket in the table using some
   * bits of the low half of the hash.
   */
  bidx = (h & (ht->ht_bucket_count - 1));
  bucket = ht->ht_buckets + bidx;

  return (cicn_pcs_timeout_opportunity (&wctx, 0, bucket));
}
