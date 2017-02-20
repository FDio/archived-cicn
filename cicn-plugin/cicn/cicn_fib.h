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
 * cicn_fib.h: Fast-path, vpp-aware FIB, used in the cicn forwarder.
 */

#ifndef _CICN_FIB_H_
#define _CICN_FIB_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include "cicn_types.h"
#include "cicn_std.h"
#include "cicn_params.h"
#include "cicn_hashtb.h"

/* FIB entry next-hop, info about a single face */
typedef struct cicn_fib_entry_nh_s
{
  /* Flags for the entry */
  uint8_t nh_flags;

  uint8_t nh_weight;

  /* Next-hop face. Invalid face value (of zero, for now)
   * means 'skip this one'.
   */
  uint16_t nh_faceid;

} cicn_fib_entry_nh_t;

/* Flags for a FIB next-hop */
#define CICN_FIB_NH_FLAGS_DEFAULT  0x00

/* Next-hop is down, via admin or via some TBD BFD-ish protocol */
#define CICN_FIB_NH_FLAG_DOWN      0x01

/*
 * FIB entry, info about a single prefix, and possibly
 * containing multiple next-hops. This is embedded in a hashtable node,
 * so its size (and alignment) have to be managed very carefully.
 */
typedef struct cicn_fib_entry_s
{

  /* Refcount for children. This helps us identify 'leaf' nodes,
   * and helps us clean up virtual nodes that aren't needed any longer.
   */
  int32_t fe_refcount;

  /* Next-hops. No 'count', because we don't assume these are
   * contiguous. TODO  -- need scalable next-hops
   * (vector, pool, etc.) eventually.
   */
  cicn_fib_entry_nh_t fe_next_hops[CICN_PARAM_FIB_ENTRY_NHOPS_MAX];

  /* Flags */
  uint8_t fe_flags;

  /* Max name components in this prefix */
  uint8_t fe_max_comps;

  /* Size is 4 + 2 + (4 * 4) => 22B */

} cicn_fib_entry_t;

/* Flags values for a FIB entry */
#define CICN_FIB_ENTRY_FLAGS_DEFAULT  0x0

#define CICN_FIB_ENTRY_FLAG_DELETED  0x1
#define CICN_FIB_ENTRY_FLAG_VIRTUAL  0x2

/*
 * Overall fib table, containing an instance of the generic hashtable
 */
typedef struct cicn_fib_s
{

  /* Flags */
  int fib_flags;

  /* Default route entry */
  cicn_fib_entry_t fib_default_entry;

  uint16_t fib_max_comps;	/* Max comps overall */

  /* Internal generic hashtable */
  cicn_hashtb_t *fib_table;

  /* Maximum capacity (in entries) */
  uint32_t fib_capacity;

} cicn_fib_t;

/* Flags for FIB */
#define CICN_FIB_FLAGS_NONE         0x0
#define CICN_FIB_FLAG_DEFAULT_SET   0x1


/* Accessor for fib data inside hash table node */
static inline cicn_fib_entry_t *
cicn_fib_get_data (cicn_hash_node_t * node)
{
  return (cicn_fib_entry_t *) (cicn_hashtb_node_data (node));
}

/* Init/alloc a new FIB */
int cicn_fib_create (cicn_fib_t * p, uint32_t num_elems);

/*
 * FIB lookup using 'pfxhash' struct containing prefix-hash results. This returns
 * the longest matching entry (not a virtual entry). If there is no valid FIB
 * match and a default FIB entry exists, the default is returned.
 */
int cicn_fib_lookup (cicn_fib_t * fib, const cicn_prefix_hashinf_t * pfxhash,
		     cicn_fib_entry_t ** pentry);

/*
 * Insert a new prefix into the FIB (or add an additional next-hop,
 * if the prefix already exists, or mod an existing next-hop, if the
 * next-hop already exists.) We expect that 'pfx' is the start of the
 * name-components only, not the start of a complete 'name' TLV. We expect
 * that the prefix-hashing has already been done, into 'pfxhash'.
 */
int cicn_fib_entry_insert (cicn_fib_t * fib,
			   const cicn_prefix_hashinf_t * pfxhash,
			   uint16_t faceid, uint8_t weight,
			   cicn_rd_t * cicn_rd_res);

/*
 * Mark a FIB prefix for delete, before actually deleting through a later
 * api call.
 * We expect that the prefix-hashing has already been done, into 'pfxhash'.
 * This will check virtual parents' refcounts, and will mark them for
 * delete also if necessary.
 *
 * TODO -- NYI...
 */
int cicn_fib_entry_mark_for_delete (cicn_fib_t * fib,
				    const cicn_prefix_hashinf_t * pfxhash);

/*
 * Delete a FIB prefix, or just delete a next-hop, if 'faceid' != 0.
 * If the prefix has children, this may just result in the conversion of
 * the entry into a virtual entry.
 * We expect that the prefix-hashing has already been done, into 'pfxhash'.
 */
int cicn_fib_entry_delete (cicn_fib_t * fib,
			   const cicn_prefix_hashinf_t * pfxhash,
			   uint16_t faceid);

/* CLI show output for fib. if 'prefix', just show a single entry */
int cicn_fib_show (const char *prefix, int detail_p, int internal_p);

/*
 * Add, delete, or change weight of fib entry next hop (which may
 * lead to add/delete of fib entry)
 */
int cicn_fib_entry_nh_update (const char *prefix, int faceid, int weight,
			      int add_p, cicn_rd_t * cicn_rd_res);

#endif /* _CICN_FIB_H_ */
