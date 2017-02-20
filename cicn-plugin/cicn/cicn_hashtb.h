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
 * cicn_hashtb.h: Fast-path, vpp-aware hashtable, the base for the PIT/CS and FIB
 * used in the cicn forwarder.
 *
 * - As is the case in other areas, we can't share headers between the vpp code
 *   and the cicn/cndn code: there are conflicting definitions.
 *
 */

#ifndef _CICN_HASHTB_H_
#define _CICN_HASHTB_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include "cicn_std.h"
#include "cicn_params.h"

/* Handy abbreviations for success status, and for boolean values */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
 * Lookup is finding a hashtable record whose name matches the name
 * being looked up.  Most of the lookup work is based on the hash
 * value of the two names. Note that the intel cache line size is 64
 * bytes, and some platforms load in 2 cache lines together.
 * - first step is to match a record at the bucket/slot level
 *   (htab has an array of htbucket_t/htbc_elmt, where each bucket has
 *   7 slots to hold indices for entries.) Matching at this level implies
 *   - the hashes of the lookup name and the record map to the
 *     same bucket
 *   - the high 32 bits of the hashes (slot bce_hash_msb32s) match.
 *   Read cost (on the hash table size, i.e. ignoring reading the
 *   name being looked up):
 *   - First step normally requires 1 cache line load to pull in
 *     the 64-byte htbucket_t with the 7 element slot table holding the
 *     hash_msb32s.
 *   - In the event (hopefully rare for a hash table with
 *     appropriate number of buckets) that more than 7 elements
 *     hash to the same bucket, lookup may well need to look not
 *     only at the static htbc_elmt_t but at the chain of dynamically
 *     allocated htbc_elmt_t's linked to the static htbc_elmt_t, where
 *     each of these holds slot entries for additional elements.
 *   - Before reaching that point, it is initially required is to read in
 *     the hash table record fields (ht_bucket_buf, htnode buf, etc)
 *     holding pointers to the arrays, but these cache lines are common
 *     to all lookups so will likely already be in the cache.
 * - second step is to match at the record level (htnode/htkb level)
 *   once a slot-level match happens. Matching at this level implies
 *   the following match
 *   - the hash values (the full 64 bits vs. bucket+32 msb, above)
 *     With siphash, two names hashing to the same 64-bit value is
 *     quite rare.
 *   - the name which, on the hash table side, is stored as a list
 *     of htkb_t (key buffers). [In some cases, the full name is
 *     not compared, and a match is assumed based on hash value match.
 *   Read cost:
 *   - htnode_t, in one cache line, holds hash value and index for the
 *     htkb at the head of the key buffer list
 *   - each key buffer (htkb_t) is cache line aligned/sized, and holds
 *     60 bytes of the name and requires a cache line read.
 * Simplification is that a fib lookup requires 3 cache lines:
 * - bucket
 * - htnode
 * - single key buffer (for cases where a name comparision is done)
 *
 * Some hashtables (for which rare false positives are tolerable)
 * store hash values but no keys. (In ISM NDN forwarder, this was
 * used for dcm_dpf: data cache manager's dataplane filter, where
 * speed was critical and very rare false positives would be detected
 * in the full dcm check.)
 * - No key buffers are used (or even allocated at hash table creation).
 */

#define CICN_HASH_INVALID_IDX  ~0
/* for cicn_hashtb_next_node() iterator, this otherwise illegal context value
 * indicates first call of iteration.
 * Note: must not be 0, which is a legal context value.
 */
#define CICN_HASH_WALK_CTX_INITIAL (~((uint64_t)0))

/*
 * Key memory allocation scheme.
 *
 * The key is the bytestring that a hashtable entry is
 * storing, e.g. a fib prefix or packet name. The hash of the
 * name is used not just to pick the bucket, but also as a surrogate
 * for the actual key value.
 *
 * Client calls pass key/name as contiguous memory for lookup/add/delete
 * but hashable stores its copy of the key/name as a list of one or
 * more hash_key structs.
 * - key memory is managed as a list of keys (cache line
 *   sized/aligned buffers).
 *   - If (keysize < 128) then use key struct's full 128 bytes
 *   - If not, first key struct is head of a linked list of elements
 *     where the first bytes are used for the key and the last 4 bytes
 *     are the index of the next entry (or an end marker).
 * - key memory is generally the single largest use of memory
 *   in the hash table, especially for PIT, as names are bigger
 *   than node structs (which is also per name/entry).
 *
 */

#define CICN_HASH_KEY_BYTES   128
#define CICN_HASH_KEY_LIST_BYTES (CICN_HASH_KEY_BYTES - sizeof(uint32_t))
typedef struct
{
  union
  {
    struct
    {
      uint8_t key[CICN_HASH_KEY_BYTES];
    } ks;			/* Entire key in one block */
    struct
    {
      uint8_t key[CICN_HASH_KEY_LIST_BYTES];
      uint32_t idx_next;	/* Next keybuf idx */
    } kl;			/* Key in a list of blocks */
  };
} cicn_hash_key_t;

/* Ratio of extra key blocks to allocate, in case the embedded ones aren't
 * sufficient. This is the fraction of the number of entries allocated.
 */
#define CICN_HASHTB_KEY_RATIO 8

/*
 * hash node, used to store a hash table entry; indexed by an entry in a bucket.
 * the node contains an embedded key; long keys are stored as chains of keys.
 *
 * The memory block for a node includes space for client data,
 * additional memory located off the end of the htnode data structure.
 * Size of client-supplied data is fixed, so we can use vpp pools. The PIT
 * and FIB need to ensure that they fit within the available data area,
 * or change the size to accomodate their needs.
 *
 * NOTE: app_data_size currently applies to all apps, i.e. bigger FIB
 *       nodes means (leads to, requires) bigger PCS nodes
 */

/* Size this so that we can offer 64B aligned on 64-bits to the applications */
#define CICN_HASH_NODE_APP_DATA_SIZE 72	/* TODO -- big enough? */

typedef struct cicn_hash_node_s
{

  uint64_t hn_hash;		/* Complete hash value */

  /* Total size of the key (chained in several key structs if necessary) */
  uint16_t hn_keysize;

  /* 1 byte of flags for application use */
  uint8_t hn_flags;

  uint8_t _hn_reserved1;	/* TBD, to align what follows back to 32 */

  cicn_hash_key_t hn_key;	/* Key value embedded in the node, may
				 * chain to more key buffers if necessary
				 */
  /* TODO -- keep array of 'next keys' so we can prefetch better? */

  /* Followed by app-specific data (fib or pit or cs entry, e.g.) */
  uint8_t hn_data[CICN_HASH_NODE_APP_DATA_SIZE];

} cicn_hash_node_t;

#define CICN_HASH_NODE_FLAGS_DEFAULT   0x00


/*
 * cicn_hash_entry_t
 * Structure holding all or part of a hash value, a node index, and other
 * key pieces of info.
 *
 * - 128 bytes/bucket with 16 bytes/entry gives 8 entries,
 *    or 7 entries plus next bucket ptr if overflow
 */
typedef struct
{

  /* MSB of the hash value */
  uint64_t he_msb64;

  /* Index of node block */
  uint32_t he_node;

  /* Timeout value, units and scheme still TBD */
  uint16_t he_timeout;

  /* A few flags, including 'this points to a chain of buckets' */
  uint8_t he_flags;

  /* A byte for domain/application data (e.g. 'virtual fib entry' */
  uint8_t he_appval;

} cicn_hash_entry_t;

#define CICN_HASH_ENTRY_FLAGS_DEFAULT  0x00

/* This entry heads a chain of overflow buckets (we expect to see this
 * only in the last entry in a bucket.) In this case, the index is
 * to an overflow bucket rather than to a single node block.
 */
#define CICN_HASH_ENTRY_FLAG_OVERFLOW 0x01

/* This entry has been marked for deletion */
#define CICN_HASH_ENTRY_FLAG_DELETED  0x02

/* Use fast he_timeout units for expiration, slow if not */
#define CICN_HASH_ENTRY_FLAG_FAST_TIMEOUT 0x04

/*
 * hash bucket: Contains an array of entries.
 * Cache line sized/aligned, so no room for extra fields unless
 * bucket size is increased to 2 cache lines or the entry struct
 * shrinks.
 */

/*
 * Overflow bucket ratio as a fraction of the fixed/configured count;
 * a pool of hash buckets used if a row in the fixed table overflows.
 */
#define CICN_HASHTB_OVERFLOW_FRACTION 8

#define CICN_HASHTB_BUCKET_ENTRIES 8

typedef struct
{
  cicn_hash_entry_t hb_entries[CICN_HASHTB_BUCKET_ENTRIES];
} cicn_hash_bucket_t;

/* Overall target fill-factor for the hashtable */
#define CICN_HASHTB_FILL_FACTOR    4

#define CICN_HASHTB_MIN_ENTRIES  (1 << 4)	// includes dummy node 0 entry
#define CICN_HASHTB_MAX_ENTRIES  (1 << 24)

#define CICN_HASHTB_MIN_BUCKETS (1 << 10)

/*
 * htab_t
 *
 * Hash table main structure.
 *
 * Contains
 * - pointers to dynamically allocated arrays of cache-line
 *   sized/aligned structures (buckets, nodes, keys).
 * Put frequently accessed fields in the first cache line.
 */
typedef struct cicn_hashtb_s
{

  /* 8B - main array of hash buckets */
  cicn_hash_bucket_t *ht_buckets;

  /* 8B - just-in-case block of overflow buckets */
  cicn_hash_bucket_t *ht_overflow_buckets;

  /* 8B - block of nodes associated with entries in buckets */
  cicn_hash_node_t *ht_nodes;

  /*
   * 8B - just-in-case block of extra keys, used when a key is too
   *      large to fit in a node's embedded key area
   */
  cicn_hash_key_t *ht_extra_keys;

  /* Flags */
  uint32_t ht_flags;

  /* Count of buckets allocated in the main array */
  uint32_t ht_bucket_count;

  /* Count of overflow buckets allocated */
  uint32_t ht_overflow_bucket_count;
  uint32_t ht_overflow_buckets_used;

  /* Count of nodes allocated */
  uint32_t ht_node_count;
  uint32_t ht_nodes_used;

  /* Count of overflow key structs allocated */
  uint32_t ht_key_count;
  uint32_t ht_keys_used;

  /* TODO -- stats? */

} cicn_hashtb_t, *cicn_hashtb_h;

/* Offset to aligned start of additional data (PIT/CS, FIB)
 * embedded in each node.
 */
extern uint32_t ht_node_data_offset_aligned;

/* Flags for hashtable */

#define CICN_HASHTB_FLAGS_DEFAULT    0x00

/* Don't use the last/eighth entry in each bucket - only use it for overflow.
 * We use this for the FIB, currently, so that we can support in-place
 * FIB changes that would be difficult if there were hash entry copies
 * as part of overflow handling.
 */
#define CICN_HASHTB_FLAG_USE_SEVEN      0x01
#define CICN_HASHTB_FLAG_KEY_FMT_PFX    0x02
#define CICN_HASHTB_FLAG_KEY_FMT_NAME   0x04

/*
 * Max prefix name components we'll support in our incremental hashing;
 * currently used only for LPM in the FIB.
 */
#define CICN_HASHTB_MAX_NAME_COMPS CICN_PARAM_FIB_ENTRY_PFX_COMPS_MAX

/*
 * Info about an LPM hash computation on a prefix or name.
 */
typedef struct cicn_prefix_hashinf_s
{

  const uint8_t *pfx_ptr;
  uint16_t pfx_len;

  uint16_t pfx_count;		/* Number of prefix entries used */
  uint8_t pfx_overflow;		/* true if pfx has extra components (not hashed) */

  uint16_t pfx_lens[CICN_HASHTB_MAX_NAME_COMPS];
  uint64_t pfx_hashes[CICN_HASHTB_MAX_NAME_COMPS];

  uint64_t pfx_full_hash;

} cicn_prefix_hashinf_t;

/*
 * APIs and inlines
 */

/* Compute hash node index from node pointer */
static inline uint32_t
cicn_hashtb_node_idx_from_node (cicn_hashtb_h h, cicn_hash_node_t * p)
{
  return (p - h->ht_nodes);
}

/* Retrieve a hashtable node by node index */
static inline cicn_hash_node_t *
cicn_hashtb_node_from_idx (cicn_hashtb_h h, uint32_t idx)
{
  return (pool_elt_at_index (h->ht_nodes, idx));
}

/* Allocate a brand-new hashtable */
int cicn_hashtb_alloc (cicn_hashtb_h * ph, uint32_t max_elems,
		       size_t app_data_size);

/* Free a hashtable, including its embedded arrays */
int cicn_hashtb_free (cicn_hashtb_h * ph);

/* Hash a bytestring, currently using siphash64 (for UT) */
uint64_t cicn_hashtb_hash_bytestring (const uint8_t * key, uint32_t keylen);

/* Hash a name, currently using siphash64 */
uint64_t cicn_hashtb_hash_name (const uint8_t * key, uint32_t keylen);

/*
 * Hash a name, with incremental prefix hashing (for LPM, e.g.) If
 * 'limit' > 0, limit computation of incrementals.
 * if 'is_full_name', we expect that 'name' points to the beginning
 * of the entire name TLV; otherwise, 'name' just points to the first
 * name-comp sub-tlv, and we will _not_ compute the full-name hash.
 * Note that 'name' and 'namelen' are captured in the prefix-hash
 * context struct, to make the LPM/FIB apis a little cleaner.
 */
int cicn_hashtb_hash_prefixes (const uint8_t * name, uint16_t namelen,
			       int is_full_name, cicn_prefix_hashinf_t * pfx,
			       int limit);

/*
 * Prepare a hashtable node for insertion, supplying the key
 * and computed hash info. This sets up the node->key relationship, possibly
 * allocating overflow key buffers.
 */
int cicn_hashtb_init_node (cicn_hashtb_h h, cicn_hash_node_t * node,
			   uint64_t hashval,
			   const uint8_t * key, uint32_t keylen);

/*
 * Insert a node into the hashtable. We expect the caller has used the
 * init api to set the node key and hash info, and populated the extra
 * data area (if any) - or done the equivalent work itself.
 */
int cicn_hashtb_insert (cicn_hashtb_h h, cicn_hash_node_t * node);

/*
 * Basic api to lookup a specific hash+key tuple. This does the entire
 * lookup operation, retrieving node structs and comparing keys,
 * so it's not optimized for prefetching or high performance.
 *
 * Returns zero and mails back a node on success, errno otherwise.
 */
int cicn_hashtb_lookup_node (cicn_hashtb_h h, const uint8_t * key,
			     uint32_t keylen, uint64_t hashval,
			     cicn_hash_node_t ** nodep);

/*
 * Extended api to lookup a specific hash+key tuple. The implementation
 * allows the caller to locate nodes that are marked for deletion; this
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
			    int include_deleted_p, cicn_hash_node_t ** nodep);

/*
 * Remove a node from a hashtable using the node itself. The internal
 * data structs are cleaned up, but the node struct itself is not: the caller
 * must free the node itself.
 */
int cicn_hashtb_remove_node (cicn_hashtb_h h, cicn_hash_node_t * node);

/*
 * Delete a node from a hashtable using the node itself, and delete/free
 * the node.  Caller's pointer is cleared on success.
 */
int cicn_hashtb_delete (cicn_hashtb_h h, cicn_hash_node_t ** pnode);

/*
 * Utility to init a new entry in a hashtable bucket/row. We use this
 * to add new a node+hash, and to clear out an entry during removal.
 */
void cicn_hashtb_init_entry (cicn_hash_entry_t * entry,
			     uint32_t nodeidx, uint64_t hashval);

/*
 * Return data area embedded in a hash node struct. We maintain an 'offset'
 * value in case the common node body struct doesn't leave the data area
 * aligned properly.
 */
static inline void *
cicn_hashtb_node_data (cicn_hash_node_t * node)
{
  return ((uint8_t *) (node) + ht_node_data_offset_aligned);
}

/*
 * Use some bits of the low half of the hash to locate a row/bucket in the table
 */
static inline uint32_t
cicn_hashtb_bucket_idx (cicn_hashtb_h h, uint64_t hashval)
{
  return ((uint32_t) (hashval & (h->ht_bucket_count - 1)));
}

/*
 * Return a hash node struct from the free list, or NULL.
 * Note that the returned struct is _not_ cleared/zeroed - init
 * is up to the caller.
 */
static inline cicn_hash_node_t *
cicn_hashtb_alloc_node (cicn_hashtb_h h)
{
  cicn_hash_node_t *p = NULL;

  if (h->ht_nodes_used < h->ht_node_count)
    {
      pool_get_aligned (h->ht_nodes, p, 8);
      h->ht_nodes_used++;
    }

  return (p);
}

/*
 * Release a hashtable node back to the free list when an entry is cleared
 */
void cicn_hashtb_free_node (cicn_hashtb_h h, cicn_hash_node_t * node);

/*
 * Walk a hashtable, iterating through the nodes, keeping context
 * in 'ctx' between calls.
 *
 * Set the context value to CICN_HASH_WALK_CTX_INITIAL to start an iteration.
 */
int cicn_hashtb_next_node (cicn_hashtb_h h, cicn_hash_node_t ** pnode,
			   uint64_t * ctx);

/*
 * Update the per-entry compression expiration value and type
 * for a hashtable node.
 */
int cicn_hashtb_entry_set_expiration (cicn_hashtb_h h,
				      cicn_hash_node_t * node,
				      uint16_t entry_timeout,
				      uint8_t entry_flags);

int cicn_hashtb_key_to_str (cicn_hashtb_h h, const cicn_hash_node_t * node,
			    char *buf, int bufsize, int must_fit);

#endif /* _CICN_HASHTB_H_ */
