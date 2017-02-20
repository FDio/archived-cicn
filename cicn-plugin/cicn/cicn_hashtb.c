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
 * cicn_hashtb.c: Fast-path, vpp-aware hashtable, the base for the PIT/CS and FIB
 * used in the cicn forwarder.
 *
 *  - As is the case in other areas, we can't share headers between the vpp code
 *    and the cicn/cndn code: there are conflicting definitions.
 */

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include <vlib/vlib.h>
#include <vppinfra/pool.h>

#include "cicn_hashtb.h"
#include "cicn_siphash.h"	/* Inline implementation of siphash-2-4 */
#include "cicn_parser.h"

/* return dvd/dvr, rounded up (intended for integer values) */
#define	CEIL(dvd, dvr)				\
({ 						\
    __typeof__ (dvd) _dvd = (dvd); 		\
    __typeof__ (dvr) _dvr = (dvr); 		\
    (_dvd + _dvr - 1)/_dvr;			\
})

#ifndef ALIGN8
#define ALIGN8(p) (((p) + 0x7) & ~(0x7))
#endif

#ifndef ALIGNPTR8
#define ALIGNPTR8(p) ((void *)(((uint8_t * )(p) + 0x7) & ~(0x7)))
#endif

#ifndef ALIGN64
#define ALIGN64(p) (((p) + 0x3f) & ~(0x3f))
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Default hash seed for now; TODO: needs to be random/per-box eventually */
unsigned char cicn_default_sip_seed[16] = {
  0x12, 0x34, 0x56, 0x78, 0x98, 0x76, 0x54, 0x32,
  0x12, 0x34, 0x56, 0x78, 0x98, 0x76, 0x54, 0x32,
};

/* Offset to aligned start of additional data (PIT/CS, FIB)
 * embedded in each node.
 */
uint32_t ht_node_data_offset_aligned;

/* Some support for posix vs vpp mem management */
#define MEM_ALLOC(x) clib_mem_alloc_aligned((x), 8)
#define MEM_FREE(p) clib_mem_free((p))

/*
 * Internal utilities
 */

/* Allocate an overflow bucket */
static cicn_hash_bucket_t *
alloc_overflow_bucket (cicn_hashtb_h h)
{
  cicn_hash_bucket_t *newbkt = NULL;

  if (h->ht_overflow_buckets_used < h->ht_overflow_bucket_count)
    {
      pool_get_aligned (h->ht_overflow_buckets, newbkt, 8);

      if (newbkt)
	{
	  h->ht_overflow_buckets_used++;
	}
    }

  return (newbkt);
}

/* Free an overflow bucket; clear caller's pointer */
static void
free_overflow_bucket (cicn_hashtb_h h, cicn_hash_bucket_t ** pb)
{
  cicn_hash_bucket_t *bkt = *pb;

  ASSERT (h->ht_overflow_buckets_used > 0);

  pool_put (h->ht_overflow_buckets, bkt);
  h->ht_overflow_buckets_used--;
  *pb = NULL;
}

/* Allocate an overflow key buffer */
static cicn_hash_key_t *
alloc_key_buf (cicn_hashtb_h h)
{
  cicn_hash_key_t *hk = NULL;

  if (h->ht_keys_used < h->ht_key_count)
    {
      pool_get_aligned (h->ht_extra_keys, hk, 8);
      if (hk)
	{
	  h->ht_keys_used++;
	}
    }

  return (hk);
}

/* for iterating over key chunks, get next chunk given current chunk */
static inline cicn_hash_key_t *
next_key_buf (cicn_hashtb_h h, const cicn_hash_key_t * hk_cur)
{
  cicn_hash_key_t *hk_next;

  if (hk_cur == NULL)
    {
      return (NULL);
    }
  hk_next = (hk_cur->kl.idx_next == CICN_HASH_INVALID_IDX) ? NULL :
    pool_elt_at_index (h->ht_extra_keys, hk_cur->kl.idx_next);

  return (hk_next);
}

/* Free an overflow key buffer; clear caller's pointer. */
static void
free_key_buf (cicn_hashtb_h h, cicn_hash_key_t ** pkey)
{
  cicn_hash_key_t *k = *pkey;

  ASSERT (h->ht_keys_used > 0);

  pool_put (h->ht_extra_keys, k);
  h->ht_keys_used--;

  *pkey = NULL;
}

/*
 * Init, allocate a new hashtable
 */
int
cicn_hashtb_alloc (cicn_hashtb_h * ph, uint32_t max_elems,
		   size_t app_data_size)
{
  int ret = EINVAL;
  cicn_hashtb_h h = NULL;
  uint32_t count;
  size_t sz;
  cicn_hash_node_t *nodep;
  cicn_hash_bucket_t *bucket;
  cicn_hash_key_t *hkey;

  if (ph == NULL)
    {
      goto done;
    }

  if (max_elems < CICN_HASHTB_MIN_ENTRIES ||
      max_elems > CICN_HASHTB_MAX_ENTRIES)
    {
      goto done;
    }

  /* Allocate and init main hashtable struct */
  h = MEM_ALLOC (sizeof (cicn_hashtb_t));
  if (h == NULL)
    {
      ret = ENOMEM;
      goto done;
    }

  memset (h, 0, sizeof (cicn_hashtb_t));

  /* Compute main table bucket (row) count and size, and allocate */
  count = ALIGN8 (CEIL (max_elems, CICN_HASHTB_FILL_FACTOR));

  h->ht_bucket_count = count;

  /* We _really_ expect to have buckets aligned on cache lines ... */
  sz = sizeof (cicn_hash_bucket_t);
  assert (sz == ALIGN64 (sz));

  h->ht_buckets = MEM_ALLOC (count * sz);
  if (h->ht_buckets == NULL)
    {
      ret = ENOMEM;
      goto done;
    }

  memset (h->ht_buckets, 0, count * sz);

  /* First time through, compute offset to aligned extra data start in
   * each node struct
   * it's crucial that both the node struct (that the base hashtable uses)
   * and the extra data area (that's also probably a struct) are aligned.
   */
  if (ht_node_data_offset_aligned == 0)
    {
      count = STRUCT_OFFSET_OF (cicn_hash_node_t, hn_data);
      ht_node_data_offset_aligned = ALIGN8 (count);
    }

  // check app struct fits into space provided (CICN_HASH_NODE_APP_DATA_SIZE)
  uint32_t ht_node_data_size;
  ht_node_data_size = sizeof (cicn_hash_node_t) - ht_node_data_offset_aligned;
  if (app_data_size > ht_node_data_size)
    {
      clib_error
	("cicn hashtable: fatal error: requested app data size(%u) > hashtb node's configured bytes available(%u)",
	 app_data_size, ht_node_data_size);
    }

  /*
   * Compute entry node count and size, allocate
   * Allocate/'Hide' the zero-th node so can use zero as an 'empty' value
   */
  pool_alloc_aligned (h->ht_nodes, max_elems, 8);
  if (h->ht_nodes == NULL)
    {
      ret = ENOMEM;
      goto done;
    }

  pool_get_aligned (h->ht_nodes, nodep, 8);	// alloc node 0
  nodep = nodep;		/* Silence 'not used' warning */

  h->ht_node_count = max_elems;
  h->ht_nodes_used = 1;

  /*
   * Compute overflow bucket count and size, allocate
   */
  count = ALIGN8 (CEIL (max_elems, CICN_HASHTB_OVERFLOW_FRACTION));

  pool_alloc_aligned (h->ht_overflow_buckets, count, 8);
  if (h->ht_overflow_buckets == NULL)
    {
      ret = ENOMEM;
      goto done;
    }

  /* 'Hide' the zero-th node so we can use zero as an 'empty' value */
  pool_get_aligned (h->ht_overflow_buckets, bucket, 8);
  bucket = bucket;		/* Silence 'not used' warning */

  h->ht_overflow_bucket_count = count;
  h->ht_overflow_buckets_used = 1;

  /*
   * Compute overflow key buffer count and size, allocate
   */
  count = ALIGN8 (CEIL (max_elems, CICN_HASHTB_KEY_RATIO));

  pool_alloc_aligned (h->ht_extra_keys, count, 8);
  if (h->ht_extra_keys == NULL)
    {
      ret = ENOMEM;
      goto done;
    }

  /* 'Hide' the zero-th node so we can use zero as an 'empty' value */
  pool_get_aligned (h->ht_extra_keys, hkey, 8);
  hkey = hkey;			/* Silence 'not used' warning */

  h->ht_key_count = count;
  h->ht_keys_used = 1;

  /* Success */
  ret = AOK;

done:

  if (h)
    {
      if ((ret == AOK) && ph)
	{
	  *ph = h;
	}
      else
	{
	  cicn_hashtb_free (&h);
	}
    }

  return (ret);
}

/*
 * Free, de-allocate a hashtable
 */
int
cicn_hashtb_free (cicn_hashtb_h * ph)
{
  int ret = 0;

  if (ph)
    {
      if ((*ph)->ht_extra_keys)
	{
	  pool_free ((*ph)->ht_extra_keys);
	  (*ph)->ht_extra_keys = 0;
	}
      if ((*ph)->ht_nodes)
	{
	  pool_free ((*ph)->ht_nodes);
	  (*ph)->ht_nodes = 0;
	}
      if ((*ph)->ht_overflow_buckets)
	{
	  pool_free ((*ph)->ht_overflow_buckets);
	  (*ph)->ht_overflow_buckets = 0;
	}
      if ((*ph)->ht_buckets)
	{
	  MEM_FREE ((*ph)->ht_buckets);
	  (*ph)->ht_buckets = 0;
	}
      MEM_FREE (*ph);

      *ph = NULL;
    }

  return (ret);
}

/*
 * Hash a bytestring, using siphash-2-4.
 */
uint64_t
cicn_hashtb_hash_bytestring (const uint8_t * in, uint32_t inlen)
{
  return (cicn_siphash (in, inlen, cicn_default_sip_seed));
}

/*
 * Hash a name, using siphash-2-4. TODO -- want a table handle here?
 */
uint64_t
cicn_hashtb_hash_name (const uint8_t * key, uint32_t keylen)
{
  if (key == NULL || keylen < CICN_TLV_HDR_LEN)
    {
      return (-1LL);
    }
  return (cicn_siphash (&key[CICN_TLV_HDR_LEN], keylen - CICN_TLV_HDR_LEN,
			cicn_default_sip_seed));
}

/*
 * Hash a name, returning hash values of prefixes (for LPM, e.g.) in
 *   addition to (or instead of) hash of full name
 *   - Hash of prefixes (by necessity) and of full name (for consistency)
 *     skips the name header tlv and starts at the first name component tlv.
 *   - version using incremental hashing, i.e. a single pass over string
 *     reusing the results for hashing each prefix in calculating the
 *     hash of the following prefix (rather than re-hashing from the
 *     beginning of the bytestring for each successive prefix as in the
 *     nonincr version).
 * Args:
 * - is_full_name:
 *   - if true
 *     - 'name' points to the beginning of the entire name TLV;
 *     - calculate hash of entire name, as well as prefixes
 *   - if false
 *      - name to the first name-comp sub-tlv
 *      - not required to compute the full-name hash, though currently
 *        this version does compute full-name hash.
 *        TODO: is avoiding that full hash a worthwhile savings?
 * - limit: if 'limit' > 0, limit prefixes to less than array size (8)
 */
static inline int
cicn_hashtb_hash_prefixes_incr (const uint8_t * name, uint16_t namelen,
				int is_full_name, cicn_prefix_hashinf_t * pfx,
				int limit)
{
  int ret = AOK;

  cicn_siphash_hi_t hi_state;
  uint64_t cur_hash = 0;

  int comp_offset;		// offset (from name_val) of comp

  /* Must point to something, and it must be at least as long
   * as an empty name or name-comp
   */
  if ((name == NULL) || (namelen < CICN_TLV_HDR_LEN) || (pfx == NULL))
    {
      ret = EINVAL;
      goto done;
    }

  /* Establish sane limit on number of comps */
  if (limit == 0 || limit > CICN_HASHTB_MAX_NAME_COMPS)
    {
      limit = CICN_HASHTB_MAX_NAME_COMPS;
    }
  pfx->pfx_overflow = 0;

  // Capture tlv pointer and len in the context struct
  // If leading name tlv (packet vs. fib prefix), skip it
  if (is_full_name)
    {
      pfx->pfx_ptr = name + CICN_TLV_HDR_LEN;
      pfx->pfx_len = namelen - CICN_TLV_HDR_LEN;
    }
  else
    {
      /* Capture tlv pointer and len in the context struct */
      pfx->pfx_ptr = name;
      pfx->pfx_len = namelen;
    }

  cicn_siphash_hi_initialize (&hi_state, cicn_default_sip_seed);

  int comp_flen;		// len of comp incl. hdr
  int pfx_idx;			// index into returned prefix arrays
  for (comp_offset = 0, pfx_idx = 0;
       comp_offset < pfx->pfx_len; comp_offset += comp_flen, pfx_idx++)
    {

      const unsigned char *comp;	// pointer to component record (hdr)
      int comp_type;		// type of component record (from component (sub)-TLV)
      uint16_t comp_vlen;	// len of comp excl. hdr (component name/value len)
      int pfx_len;		// len of pfx (equal to offset of following comp)

      comp = &pfx->pfx_ptr[comp_offset];
      C_GETINT16 (comp_type, &comp[0]);
      C_GETINT16 (comp_vlen, &comp[CICN_TLV_TYPE_LEN]);
      comp_flen = CICN_TLV_HDR_LEN + comp_vlen;

      pfx_len = comp_offset + comp_flen;
      if (pfx_len > pfx->pfx_len)
	{
	  ret = EINVAL;
	  goto done;
	}

      if (comp_type == CICN_NAME_COMP_CHUNK)
	{
	  /* assume FIB entries dont include chunk#: terminate partial hashes
	   * and proceed to full name hash
	   *
	   * for now, only chunk# (above) ends partial hashing, i.e. do not
	   * rule out creating and matching on FIB entries that include
	   * non-NameComponent components (that precede chunk#).
	   */
	  comp_flen = -1;	/* end indicator */
	  pfx_len = pfx->pfx_len;
	}
      else if (pfx_idx >= limit)
	{			/* Are we out of partial hash slots? */
	  /*
	   * - no room in arrays to save remaining hashes or, in
	   *   fib lookup case, no reason to calculate remaining
	   *   partial hashes
	   * - do one last hash covering full string, taking advantage
	   *   of v_whole rather than starting from scratch, to save in
	   *   name_hash as always (even though will not saved in array
	   *   in overflow case).
	   * - re-check for overflow below, i.e. after efficient
	   *   whole-string hash calculation, and break out of loop there
	   */
	  pfx->pfx_overflow = 1;
	  comp_flen = -1;	/* end indicator */
	  pfx_len = pfx->pfx_len;
	}

      cur_hash =
	cicn_siphash_hi_calculate (&hi_state, pfx->pfx_ptr, pfx_len,
				   comp_offset);

      if (comp_flen < 0)
	{
	  /*
	   * - No benefit to calculating more partial hashes.
	   *   - In overflow case, no room to store results.
	   *   - In chunk component case, partial hash not useful
	   * - Due to similar check above, full string's hash has been
	   *   calculated, break out of loop with cur_hash and
	   *   pfx_idx having right values for return arguments.
	   * Return appropriate rc:
	   * - if actually out of room, return overflow (callers may not
	   *   need to know this)
	   * - otherwise, max requested depth (less than array size)
	   *   reached, but that's okay (not currently used).
	   */
	  if (pfx_idx >= ARRAY_LEN (pfx->pfx_hashes))
	    {
	      ret = ENOSPC;
	    }
	  break;
	}

      pfx->pfx_lens[pfx_idx] = pfx_len;
      pfx->pfx_hashes[pfx_idx] = cur_hash;
    }				/* for */

  // pfx_idx is now count (idx+1) (normal loop exit happens after
  // for-loop increment and loop breaks happen before next array entry
  // is filled in
  pfx->pfx_count = pfx_idx;

  if (pfx_idx == 0)
    {				// case of empty name still has hash
      cur_hash =
	cicn_siphash_hi_calculate (&hi_state, pfx->pfx_ptr, pfx->pfx_len,
				   comp_offset);
    }

  pfx->pfx_full_hash = cur_hash;

done:
  return (ret);
}

int
cicn_hashtb_hash_prefixes (const uint8_t * name, uint16_t namelen,
			   int is_full_name, cicn_prefix_hashinf_t * pfx,
			   int limit)
{
  int ret;

  ret =
    cicn_hashtb_hash_prefixes_incr (name, namelen, is_full_name, pfx, limit);

  return (ret);
}

/*
 * Basic api to lookup a specific hash+key tuple. This does the entire
 * lookup operation, retrieving node structs and comparing keys,
 * so it's not optimized for prefetching or high performance.
 *
 * Returns zero and mails back a node on success, errno otherwise.
 */
int
cicn_hashtb_lookup_node (cicn_hashtb_h h, const uint8_t * key,
			 uint32_t keylen, uint64_t hashval,
			 cicn_hash_node_t ** nodep)
{
  return (cicn_hashtb_lookup_node_ex
	  (h, key, keylen, hashval, FALSE /*deleted nodes */ , nodep));
}

/*
 * Extended api to lookup a specific hash+key tuple. The implementation
 * allows the caller to locate nodes that are marked for deletion, which
 * is part of some hashtable applications, such as the FIB.
 *
 * This does the entire lookup operation, retrieving node structs
 * and comparing keys, so it's not optimized for prefetching or high performance.
 *
 * Returns zero and mails back a node on success, errno otherwise.
 */
int
cicn_hashtb_lookup_node_ex (cicn_hashtb_h h, const uint8_t * key,
			    uint32_t keylen, uint64_t hashval,
			    int include_deleted_p, cicn_hash_node_t ** nodep)
{
  int i, ret = EINVAL;
  int found_p = FALSE;
  uint32_t bidx;
  cicn_hash_bucket_t *bucket;
  cicn_hash_node_t *node;

  /* Check args */
  if ((h == NULL) || (key == NULL) || (keylen == 0))
    {
      goto done;
    }

  /* Use some bits of the low half of the hash
   * to locate a row/bucket in the table
   */
  bidx = (hashval & (h->ht_bucket_count - 1));

  bucket = h->ht_buckets + bidx;

  /* Check the entries in the bucket for matching hash value */

loop_buckets:

  for (i = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++)
    {

      /* If an entry is marked for deletion, ignore it unless the caller
       * explicitly wants these nodes.
       */
      if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_DELETED)
	{
	  if (!include_deleted_p)
	    {
	      continue;
	    }
	}

      /* Be prepared to continue to an overflow bucket if necessary.
       * We only expect the last entry in a bucket to refer to an overflow
       * bucket...
       */
      if (i == (CICN_HASHTB_BUCKET_ENTRIES - 1) &&
	  bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	{
	  bucket = pool_elt_at_index (h->ht_overflow_buckets,
				      bucket->hb_entries[i].he_node);
	  goto loop_buckets;
	}

      if (bucket->hb_entries[i].he_msb64 == hashval)
	{
	  /* Found a candidate - must retrieve the actual node and
	   * check the key.
	   */
	  node = pool_elt_at_index (h->ht_nodes,
				    bucket->hb_entries[i].he_node);

	  /* Check the key itself; we've already checked the hash value */
	  ASSERT (node->hn_hash == hashval);

	  if (key && (keylen > 0))
	    {
	      if (keylen != node->hn_keysize)
		{
		  continue;
		}

	      if (keylen <= CICN_HASH_KEY_BYTES)
		{
		  if (memcmp (key, node->hn_key.ks.key, keylen) != 0)
		    {
		      continue;
		    }
		}
	      else
		{
		  int key_bseen;	// bytes processed/compared equal
		  int kc_bytes;
		  const cicn_hash_key_t *kc;	// key chunks
		  for (kc = &node->hn_key, key_bseen = 0; kc != NULL;
		       kc = next_key_buf (h, kc), key_bseen += kc_bytes)
		    {
		      int key_bleft = keylen - key_bseen;
		      kc_bytes = (key_bleft <= CICN_HASH_KEY_LIST_BYTES) ?
			key_bleft : CICN_HASH_KEY_LIST_BYTES;
		      if (memcmp (&key[key_bseen], kc->kl.key, kc_bytes))
			{
			  break;	// key chunk didn't match
			}
		    }
		  if (kc != NULL)
		    {		// found a mismatch before end of key
		      continue;
		    }
		}
	    }

	  /* Found a match */
	  if (nodep)
	    {
	      *nodep = node;
	    }

	  found_p = TRUE;
	  break;
	}
    }

  if (found_p)
    {
      ret = AOK;
    }
  else
    {
      ret = ENOENT;
    }

done:

  return (ret);
}

/* Compute hash node index from node pointer */
#define NODE_IDX_FROM_NODE(p, h) \
    ((p) - ((h)->ht_nodes))

/*
 * Utility to init a new entry in a hashtable bucket/row. We use this
 * to add new a node+hash, and to clear out an entry during removal.
 */
void
cicn_hashtb_init_entry (cicn_hash_entry_t * entry, uint32_t nodeidx,
			uint64_t hashval)
{
  entry->he_msb64 = hashval;
  entry->he_node = nodeidx;

  /* Clear out some other fields in the entry */
  entry->he_flags = 0;
  entry->he_timeout = 0;
}

/*
 * Insert a node into the hashtable. We expect the caller has a) computed
 * the hash value to use, b) initialized the node with the hash and key info,
 * and c) filled in its app-specific data portion of the node.
 */
int
cicn_hashtb_insert (cicn_hashtb_h h, cicn_hash_node_t * node)
{
  int i, ret = EINVAL;
  uint32_t bidx;
  cicn_hash_bucket_t *bucket, *newbkt;
  int use_seven;

  if (h == NULL)
    {
      goto done;
    }

  /* Use some bits of the low half of the hash
   * to locate a row/bucket in the table
   */
  bidx = (node->hn_hash & (h->ht_bucket_count - 1));

  bucket = h->ht_buckets + bidx;

  use_seven = (h->ht_flags & CICN_HASHTB_FLAG_USE_SEVEN);

  /* Locate a free entry slot in the bucket */

loop_buckets:

  for (i = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++)
    {

      /*
       * If an entry is marked for deletion, ignore it
       */
      if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_DELETED)
	{
	  continue;
	}

      if ((bucket->hb_entries[i].he_msb64 == 0LL) &&
	  (bucket->hb_entries[i].he_node == 0))
	{
	  /* Found a candidate -- fill it in */

	  /* Special case if the application asked not to use the last
	   * entry in each bucket.
	   */
	  if ((i != (CICN_HASHTB_BUCKET_ENTRIES - 1)) || !use_seven)
	    {

	      cicn_hashtb_init_entry (&(bucket->hb_entries[i]),
				      NODE_IDX_FROM_NODE (node, h),
				      node->hn_hash);

	      ret = AOK;
	      break;
	    }
	}

      /* Be prepared to continue to an overflow bucket if necessary,
       * or to add a new overflow bucket.
       * We only expect the last entry in a bucket to refer to an overflow
       * bucket...
       */
      if (i == (CICN_HASHTB_BUCKET_ENTRIES - 1))
	{
	  if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	    {

	      /* Existing overflow bucket - re-start the search loop */
	      bucket = pool_elt_at_index (h->ht_overflow_buckets,
					  bucket->hb_entries[i].he_node);
	      goto loop_buckets;

	    }
	  else
	    {
	      /* Overflow - reached the end of a bucket without finding a
	       * free entry slot. Need to allocate an overflow bucket, and
	       * connect it to this bucket.
	       */
	      newbkt = alloc_overflow_bucket (h);
	      if (newbkt == NULL)
		{
		  ret = ENOMEM;
		  goto done;
		}

	      /* We're touching some more bytes than we absolutely have to
	       * here, but ... that seems ok.
	       */
	      memset (newbkt, 0, sizeof (cicn_hash_bucket_t));

	      if (!use_seven)
		{
		  /* Copy existing entry into new bucket - we really expect
		   * these to be properly aligned so they can be treated as
		   * ints. TODO -- do in 8-byte assignments?
		   */
		  memcpy (&(newbkt->hb_entries[0]),
			  &(bucket->hb_entries[i]),
			  sizeof (cicn_hash_entry_t));
		}

	      /* Connect original bucket to the index of the
	       * new overflow bucket
	       */
	      bucket->hb_entries[i].he_flags |= CICN_HASH_ENTRY_FLAG_OVERFLOW;
	      bucket->hb_entries[i].he_node =
		(newbkt - h->ht_overflow_buckets);

	      /* Add new entry to new overflow bucket */
	      bucket = newbkt;

	      /* Use entry [1] in the new bucket _if_ we just copied into
	       * entry [zero] above.
	       */
	      if (!use_seven)
		{

		  cicn_hashtb_init_entry (&(bucket->hb_entries[1]),
					  NODE_IDX_FROM_NODE (node, h),
					  node->hn_hash);
		}
	      else
		{

		  cicn_hashtb_init_entry (&(bucket->hb_entries[0]),
					  NODE_IDX_FROM_NODE (node, h),
					  node->hn_hash);
		}

	      /* And we're done with the overflow bucket */
	      ret = AOK;
	      break;
	    }
	}
    }

done:

  return (ret);
}

/*
 * Delete a node from a hashtable using the node itself, and delete/free
 * the node. Caller's pointer is cleared on success.
 */
int
cicn_hashtb_delete (cicn_hashtb_h h, cicn_hash_node_t ** pnode)
{
  int ret = EINVAL;

  if ((h == NULL) || (pnode == NULL) || (*pnode == NULL))
    {
      goto done;
    }

  ret = cicn_hashtb_remove_node (h, *pnode);
  if (ret == AOK)
    {
      cicn_hashtb_free_node (h, *pnode);
      *pnode = NULL;
    }

done:
  return (ret);
}

/*
 * Delete an entry from a hashtable using the node itself
 */
int
cicn_hashtb_remove_node (cicn_hashtb_h h, cicn_hash_node_t * node)
{
  int i, count, ret = EINVAL;
  uint32_t bidx, overflow_p, nodeidx;
  cicn_hash_bucket_t *bucket, *parent;

  if ((h == NULL) || (node == NULL))
    {
      goto done;
    }

  /* Use some bits of the low half of the hash
   * to locate a row/bucket in the table
   */
  bidx = (node->hn_hash & (h->ht_bucket_count - 1));

  nodeidx = NODE_IDX_FROM_NODE (node, h);

  bucket = h->ht_buckets + bidx;

  overflow_p = FALSE;

loop_buckets:

  for (i = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++)
    {
      /* Note that we do consider entries that are marked for
       * delete here, unlike some other operations.
       */

      /* Be prepared to continue to an overflow bucket if necessary.
       * We only expect the last entry in a bucket to refer to an overflow
       * bucket...
       */
      if (i == (CICN_HASHTB_BUCKET_ENTRIES - 1) &&
	  bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	{

	  bucket = pool_elt_at_index (h->ht_overflow_buckets,
				      bucket->hb_entries[i].he_node);

	  overflow_p = TRUE;

	  goto loop_buckets;
	}

      /* Look for an entry carrying this node index */
      if (nodeidx == bucket->hb_entries[i].he_node)
	{
	  ret = AOK;
	  break;
	}
    }

  /* If we didn't find the matching entry, we're done */
  if (ret != AOK)
    {
      ret = ENOENT;
      goto done;
    }

  /* Clear out the entry. */
  cicn_hashtb_init_entry (&(bucket->hb_entries[i]), 0, 0LL);

  if (!overflow_p)
    {
      /* And we're done, in the easy case where we didn't change an
       * overflow bucket
       */
      goto done;
    }

  /* The special case: if this is the last remaining entry in an
   * overflow bucket, liberate the bucket. That in turn has a special
   * case if this bucket is in the middle of a chain of overflow buckets.
   *
   * Note that we're not trying aggressively (yet) to condense buckets
   * at every possible opportunity.
   */

  /* Reset this flag; we'll set it again if this bucket links to another */
  overflow_p = FALSE;

  for (i = 0, count = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++)
    {
      if (bucket->hb_entries[i].he_node != 0)
	{
	  count++;
	}

      if (i == (CICN_HASHTB_BUCKET_ENTRIES - 1) &&
	  (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW))
	{
	  count--;		/* Doesn't count as a 'real' entry */
	  overflow_p = TRUE;
	}
    }

  if (count > 0)
    {
      /* Still a (real) entry in the row */
      goto done;
    }

  /* Need to locate the predecessor of 'bucket': start at
   * the beginning of the chain of buckets and move forward
   */
  bidx = (node->hn_hash & (h->ht_bucket_count - 1));

  for (parent = h->ht_buckets + bidx; parent != NULL;)
    {

      if ((parent->hb_entries[(CICN_HASHTB_BUCKET_ENTRIES - 1)].he_flags &
	   CICN_HASH_ENTRY_FLAG_OVERFLOW) == 0)
	{
	  parent = NULL;
	  break;
	}

      bidx = parent->hb_entries[(CICN_HASHTB_BUCKET_ENTRIES - 1)].he_node;

      if (pool_elt_at_index (h->ht_overflow_buckets, bidx) == bucket)
	{
	  /* Found the predecessor of 'bucket'. If 'bucket' has a successor,
	   * connect 'parent' to it, and take 'bucket out of the middle.
	   */
	  if (overflow_p)
	    {
	      parent->hb_entries[(CICN_HASHTB_BUCKET_ENTRIES - 1)].he_node =
		bucket->hb_entries[(CICN_HASHTB_BUCKET_ENTRIES - 1)].he_node;
	    }
	  else
	    {
	      /* Just clear the predecessor entry pointing at 'bucket' */
	      cicn_hashtb_init_entry (&parent->hb_entries
				      [(CICN_HASHTB_BUCKET_ENTRIES - 1)], 0,
				      0LL);
	    }

	  break;
	}

      /* After the first iteration, 'parent' will be an overflow bucket too */
      parent = pool_elt_at_index (h->ht_overflow_buckets, bidx);
    }

  /* We really expect to have found the predecessor */
  ASSERT (parent != NULL);

  /* And now, finally, we can put 'bucket' back on the free list */
  free_overflow_bucket (h, &bucket);

done:

  return (ret);
}

/*
 * Prepare a hashtable node, supplying the key, and computed hash info.
 */
int
cicn_hashtb_init_node (cicn_hashtb_h h, cicn_hash_node_t * node,
		       uint64_t hashval, const uint8_t * key, uint32_t keylen)
{
  int ret = EINVAL;
  uint32_t keyidx;
  cicn_hash_key_t *hk, *prevhk;

  assert (h != NULL);
  assert (node != NULL);

  if ((h == NULL) || (node == NULL))
    {
      goto done;
    }

  /* Init the node struct */
  node->hn_hash = hashval;
  node->hn_flags = CICN_HASH_NODE_FLAGS_DEFAULT;
  node->hn_keysize = 0;

  if (key && (keylen > 0))
    {
      if (keylen > CICN_PARAM_HASHTB_KEY_BYTES_MAX)
	{
	  /* Whoops - key is too darn big */
	  ret = EINVAL;
	  goto done;
	}

      node->hn_keysize = keylen;

      if (keylen <= CICN_HASH_KEY_BYTES)
	{
	  /* Use the node's embedded key buffer */
	  memcpy (node->hn_key.ks.key, key, keylen);
	}
      else
	{
	  /*
	   * Key is too large for the embedded buffer alone;
	   * must use a chain of key buffers; we capture the chain
	   * by index.
	   */
	  prevhk = NULL;
	  hk = &(node->hn_key);

	  do
	    {

	      /* Put new key buf index into previous buf */
	      if (prevhk)
		{
		  /* Compute index of new key buf */
		  keyidx = hk - h->ht_extra_keys;
		  prevhk->kl.idx_next = keyidx;
		}

	      /* Copy as much key material as we can */
	      if (keylen > CICN_HASH_KEY_LIST_BYTES)
		{
		  memcpy (hk->kl.key, key, CICN_HASH_KEY_LIST_BYTES);
		  key += CICN_HASH_KEY_LIST_BYTES;
		  keylen -= CICN_HASH_KEY_LIST_BYTES;
		}
	      else
		{
		  /* Last piece of the key */
		  memcpy (hk->kl.key, key, keylen);
		  keylen = 0;

		  /* Terminate the chain of key buffers */
		  hk->kl.idx_next = CICN_HASH_INVALID_IDX;
		  break;
		}

	      prevhk = hk;

	      hk = alloc_key_buf (h);

	    }
	  while (hk);

	  if (keylen > 0)
	    {
	      /* Whoops - failed to get enough key buffers */
	      ret = ENOMEM;
	      goto done;
	    }
	}
    }

  ret = AOK;

done:

  return (ret);
}

/*
 * Release a hashtable node back to the free list when an entry is cleared
 */
void
cicn_hashtb_free_node (cicn_hashtb_h h, cicn_hash_node_t * node)
{
  uint32_t klen, keyidx;
  cicn_hash_key_t *keyp;

  ASSERT (h->ht_nodes_used > 0);

  /* If there is a chain of key structs, need to free them also */
  if (node->hn_keysize > CICN_HASH_KEY_BYTES)
    {

      /* Remaining key bytes (for consistency check) */
      klen = node->hn_keysize - CICN_HASH_KEY_LIST_BYTES;

      /* Walk along the chain of key buffers until we reach the end */
      for (keyidx = node->hn_key.kl.idx_next; keyidx != CICN_HASH_INVALID_IDX;
	   /* can't step in iterator: keyp already freed */ )
	{

	  keyp = pool_elt_at_index (h->ht_extra_keys, keyidx);
	  if (!keyp)
	    {			// should not happen given valid keyidx
	      break;
	    }
	  keyidx = keyp->kl.idx_next;

	  /* Return the key buf to the free pool */
	  free_key_buf (h, &keyp);

	  /* Consistency checks on klen */
	  if (klen > CICN_HASH_KEY_LIST_BYTES)
	    {
	      klen -= CICN_HASH_KEY_LIST_BYTES;
	      ASSERT (keyidx != CICN_HASH_INVALID_IDX);
	    }
	  else
	    {
	      klen = 0;
	      ASSERT (keyidx == CICN_HASH_INVALID_IDX);
	    }
	}
    }

  /* Return 'node' to the free list */
  pool_put (h->ht_nodes, node);
  h->ht_nodes_used--;

}

/*
 * Walk a hashtable, iterating through the nodes, keeping context in 'ctx'.
 */
int
cicn_hashtb_next_node (cicn_hashtb_h h, cicn_hash_node_t ** pnode,
		       uint64_t * ctx)
{
  int i, j, ret = EINVAL;
  uint32_t bidx, entry;
  cicn_hash_bucket_t *bucket;

  if ((h == NULL) || (pnode == NULL) || (ctx == NULL))
    {
      goto done;
    }

  /* Special-case for new iteration */
  if (*ctx == CICN_HASH_WALK_CTX_INITIAL)
    {
      bidx = 0;
      bucket = &h->ht_buckets[0];
      entry = 0;
      j = 0;
      i = 0;
      goto search_table;
    }

  /* Convert context to bucket and entry indices */
  bidx = *ctx & 0xffffffffLL;
  entry = *ctx >> 32;

  if (bidx >= h->ht_bucket_count)
    {
      ret = ENOENT;
      goto done;
    }

  bucket = h->ht_buckets + bidx;

  /* Init total index into entries (includes fixed bucket and overflow) */
  j = 0;


skip_processed_bucket_chunks:
  /* Figure out where to resume the search for the next entry in
   * the table, by trying to find the last entry returned, from the cookie.
   * Loop walks one (regular or overflow) bucket chunk, label is used for
   * walking chain of chunks.
   * Note that if there was a deletion or an addition that created an
   * overflow, iterator can skip entries or return duplicate entries,
   * for entries that are present from before the walk starts until
   * after it ends.
   * TODO: Add mechanism to break at bucket row boundaries, to avoid
   * skip/duplication of entries not changed during walk.
   */

  for (i = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++, j++)
    {
      if (j > entry)
	{
	  /*
	   * Start search for next here, use existing 'bucket' and 'i'
	   */
	  break;
	}

      /*
       * If an entry is marked for deletion, ignore it
       */
      if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_DELETED)
	{
	  continue;
	}

      /* Be prepared to continue to an overflow bucket if necessary.
       * (We only expect the last entry in a bucket to refer to an overflow
       * bucket...)
       */
      if (i == (CICN_HASHTB_BUCKET_ENTRIES - 1))
	{
	  if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	    {
	      bucket = pool_elt_at_index (h->ht_overflow_buckets,
					  bucket->hb_entries[i].he_node);

	      /* Increment overall entry counter 'j' */
	      j++;

	      goto skip_processed_bucket_chunks;
	    }

	  /* end of row (end of fixed bucket plus any overflows) */
	  i = 0;
	  j = 0;

	  bidx++;

	  /* Special case - we're at the end */
	  if (bidx >= h->ht_bucket_count)
	    {
	      ret = ENOENT;
	      goto done;
	    }
	  bucket = h->ht_buckets + bidx;
	  break;
	}
    }

search_table:

  /* Now we're searching through the table for the next entry that's set */

  for (; i < CICN_HASHTB_BUCKET_ENTRIES; i++, j++)
    {
      /*
       * If an entry is marked for deletion, ignore it
       */
      if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_DELETED)
	{
	  continue;
	}

      /* Is this entry set? */
      if (bucket->hb_entries[i].he_node != 0)
	{

	  /* Retrieve the node struct */
	  *pnode = pool_elt_at_index (h->ht_nodes,
				      bucket->hb_entries[i].he_node);

	  /* Set 'entry' as we exit, so we can update the cookie */
	  entry = j;
	  ret = AOK;
	  break;
	}

      /* Be prepared to continue to an overflow bucket if necessary.
       * (We only expect the last entry in a bucket to refer to an overflow
       * bucket...)
       */
      if (i == (CICN_HASHTB_BUCKET_ENTRIES - 1))
	{
	  if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	    {
	      bucket = pool_elt_at_index (h->ht_overflow_buckets,
					  bucket->hb_entries[i].he_node);
	      /* Reset per-bucket index 'i', here (not done in iterator) */
	      i = 0;
	      /* Increment overall entry counter 'j' */
	      j++;

	      goto search_table;
	    }
	  else
	    {
	      /* Move to next bucket, resetting per-bucket and overall
	       * entry indexes
	       */
	      i = 0;
	      j = 0;

	      bidx++;

	      /* Special case - we're at the end */
	      if (bidx >= h->ht_bucket_count)
		{
		  ret = ENOENT;
		  goto done;
		}

	      bucket = h->ht_buckets + bidx;
	      goto search_table;
	    }
	}
    }

done:

  if (ret == AOK)
    {
      /* Update context */
      *ctx = bidx;
      *ctx |= ((uint64_t) entry << 32);
    }

  return (ret);
}

/*
 * Update the per-entry compression expiration value for a hashtable node.
 */
int
cicn_hashtb_entry_set_expiration (cicn_hashtb_h h,
				  cicn_hash_node_t * node,
				  uint16_t entry_timeout, uint8_t entry_flags)
{
  int i, ret = EINVAL;
  uint32_t bidx, nodeidx;
  cicn_hash_bucket_t *bucket;

  if ((h == NULL) || (node == NULL))
    {
      goto done;
    }

  /* Use some bits of the low half of the hash
   * to locate a row/bucket in the table
   */
  bidx = (node->hn_hash & (h->ht_bucket_count - 1));

  nodeidx = NODE_IDX_FROM_NODE (node, h);

  bucket = h->ht_buckets + bidx;

loop_buckets:

  for (i = 0; i < CICN_HASHTB_BUCKET_ENTRIES; i++)
    {
      /*
       * If an entry is marked for deletion, ignore it
       */
      if (bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_DELETED)
	{
	  continue;
	}

      /* Be prepared to continue to an overflow bucket if necessary.
       * We only expect the last entry in a bucket to refer to an overflow
       * bucket...
       */
      if (i == (CICN_HASHTB_BUCKET_ENTRIES - 1) &&
	  bucket->hb_entries[i].he_flags & CICN_HASH_ENTRY_FLAG_OVERFLOW)
	{

	  bucket = pool_elt_at_index (h->ht_overflow_buckets,
				      bucket->hb_entries[i].he_node);

	  goto loop_buckets;
	}

      /* Look for an entry carrying this node index */
      if (nodeidx == bucket->hb_entries[i].he_node)
	{
	  ret = AOK;
	  break;
	}
    }

  /* If we didn't find the matching entry, we're done */
  if (ret != AOK)
    {
      ret = ENOENT;
      goto done;
    }

  /* Update the entry. */
  bucket->hb_entries[i].he_timeout = entry_timeout;
  if (entry_flags & CICN_HASH_ENTRY_FLAG_FAST_TIMEOUT)
    {
      bucket->hb_entries[i].he_flags |= CICN_HASH_ENTRY_FLAG_FAST_TIMEOUT;
    }
  else
    {
      bucket->hb_entries[i].he_flags &= ~CICN_HASH_ENTRY_FLAG_FAST_TIMEOUT;
    }

done:
  return (ret);
}

int
cicn_hashtb_key_to_buf (u8 ** vec_res, cicn_hashtb_h h,
			const cicn_hash_node_t * node)
{
  int ret = AOK;
  u8 *vec = *vec_res;

  if (node->hn_keysize <= CICN_HASH_KEY_BYTES)
    {
      vec_add (vec, node->hn_key.ks.key, node->hn_keysize);
      goto success;
    }

  const cicn_hash_key_t *kc;
  for (kc = &node->hn_key; kc != NULL; kc = next_key_buf (h, kc))
    {

      int key_bleft = node->hn_keysize - vec_len (vec);
      int kc_bytes = (key_bleft <= CICN_HASH_KEY_LIST_BYTES) ?
	key_bleft : CICN_HASH_KEY_LIST_BYTES;
      vec_add (vec, kc->kl.key, kc_bytes);
    }

success:

  *vec_res = vec;
  return (ret);
}

int
cicn_hashtb_key_to_str (cicn_hashtb_h h, const cicn_hash_node_t * node,
			char *buf, int bufsize, int must_fit)
{
  int ret = EINVAL;

  u8 *kvec = NULL;
  int bstr_len;

  ret = cicn_hashtb_key_to_buf (&kvec, h, node);

  if (h->ht_flags & CICN_HASHTB_FLAG_KEY_FMT_PFX)
    {
      ret =
	cicn_parse_prefix_to_str (buf, bufsize, kvec, vec_len (kvec),
				  &bstr_len);
    }
  else if (h->ht_flags & CICN_HASHTB_FLAG_KEY_FMT_NAME)
    {
      ret =
	cicn_parse_name_to_str (buf, bufsize, kvec, vec_len (kvec),
				&bstr_len);
    }
  else
    {
      ret = EINVAL;		// should never happen
    }
  if (ret != AOK)
    {
      goto err;
    }

  if (bstr_len >= bufsize)
    {
      if (must_fit)
	{
	  ret = ENOSPC;
	  goto err;
	}
      if (bufsize < 4)
	{
	  ret = ENOSPC;
	  goto err;
	}
      snprintf (&buf[bufsize - 4], 4, "...");
    }

  ret = AOK;

err:
  vec_free (kvec);
  /* Not totally sure that I've got the to_str perfect - belt-and-susp. */
  buf[bufsize - 1] = '\000';
  return (ret);
}
