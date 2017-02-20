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
 * cicn_fib.c: Fast-path, vpp-aware FIB, used in the cicn forwarder.
 */

#include <vlib/vlib.h>

#include <cicn/cicn.h>
#include <cicn/cicn_fib.h>

/*
 * Init/alloc a new FIB
 */
int
cicn_fib_create (cicn_fib_t * p, uint32_t num_elems)
{
  int ret;

  ret =
    cicn_hashtb_alloc (&p->fib_table, num_elems, sizeof (cicn_fib_entry_t));
  if (ret == AOK)
    {
      /* Reserve last entry in a row/bucket for overflow, to make
       * unsynchronized FIB modifications easier.
       */
      p->fib_table->ht_flags |= CICN_HASHTB_FLAG_USE_SEVEN;
      p->fib_table->ht_flags |= CICN_HASHTB_FLAG_KEY_FMT_PFX;

      p->fib_flags = CICN_FIB_FLAGS_NONE;

      p->fib_capacity = num_elems;

      p->fib_max_comps = 0;
      memset (&(p->fib_default_entry), 0, sizeof (cicn_fib_entry_t));
    }

  return (ret);
}

/*
 * FIB lookup using 'pfxhash' struct containing prefix-hash results. This returns
 * the longest matching entry (not a virtual entry). If there is no valid FIB
 * match and a default FIB entry exists, the default is returned.
 */
int
cicn_fib_lookup (cicn_fib_t * fib, const cicn_prefix_hashinf_t * pfxhash,
		 cicn_fib_entry_t ** pentry)
{
  int found_p, i, ret = EINVAL;
  cicn_hash_node_t *node = NULL;
  cicn_fib_entry_t *fe;

  if ((fib == NULL) || (pfxhash == NULL) || (pfxhash->pfx_ptr == NULL) ||
      (pfxhash->pfx_len == 0) ||
      (pfxhash->pfx_count > CICN_HASHTB_MAX_NAME_COMPS) || (pentry == NULL))
    {

      goto done;
    }

  found_p = FALSE;

  /* If we have a default entry, start with that */
  if (fib->fib_flags & CICN_FIB_FLAG_DEFAULT_SET)
    {
      *pentry = &fib->fib_default_entry;

      /* And we have some form of success... */
      found_p = TRUE;
    }

  /*
   * TODO -- souped-up, thoughtful walk through the prefixes, optimized
   * to start 'in the middle' and then walk 'up' or 'down' depending on
   * what we find. For now, just simple iteration...
   */

  /*
   * Iterate through the prefix hashes, looking for the longest match
   */
  for (i = 0; i < pfxhash->pfx_count; i++)
    {
      ret = cicn_hashtb_lookup_node (fib->fib_table,
				     pfxhash->pfx_ptr,
				     pfxhash->pfx_lens[i],
				     pfxhash->pfx_hashes[i], &node);
      if (ret == AOK)
	{
	  fe = cicn_fib_get_data (node);

	  /* Don't use a 'virtual' entry */
	  if (fe->fe_flags & CICN_FIB_ENTRY_FLAG_VIRTUAL)
	    {
	      continue;
	    }

	  /* Best match so far */
	  *pentry = fe;
	  found_p = TRUE;

	}
      else
	{
	  /* No more possible longer prefixes */
	  break;
	}
    }

  if (found_p)
    {
      ret = AOK;
    }

done:

  return (ret);
}

/*
 * Insert a new prefix into the FIB (or add an additional next-hop,
 * if the prefix already exists, or mod an existing next-hop, if the
 * next-hop already exists.) We expect that 'pfx' is the start of the
 * name-components only, not the start of a complete 'name' TLV. We expect
 * that the prefix-hashing has already been done, into 'pfxhash'.
 */
int
cicn_fib_entry_insert (cicn_fib_t * fib,
		       const cicn_prefix_hashinf_t * pfxhash,
		       uint16_t faceid, uint8_t weight, cicn_rd_t * cicn_rd)
{
  int ret = EINVAL;
  cicn_rc_e crc = CICN_RC_OK;
  int i, j;
  cicn_hash_node_t *node_array[CICN_HASHTB_MAX_NAME_COMPS];
  int node_count = 0;
  cicn_hash_node_t *pnode;
  cicn_fib_entry_t *fe;
  int add_ref_val = 0;

  if ((fib == NULL) || (pfxhash == NULL) || (pfxhash->pfx_ptr == NULL) ||
      (pfxhash->pfx_len == 0) || (pfxhash->pfx_count == 0))
    {
      goto done;
    }
  if (pfxhash->pfx_count > CICN_HASHTB_MAX_NAME_COMPS ||
      pfxhash->pfx_overflow)
    {
      crc = CICN_RC_FIB_PFX_COMP_LIMIT;
      goto done;
    }

  /* Start walking down the series of intermediate prefixes,
   * capturing the hash node at each level that already exists. We need
   * this in order to manage internal state, like refcounts
   * and virtual FIB nodes.
   */
  for (i = 0; i < pfxhash->pfx_count; i++)
    {
      ret = cicn_hashtb_lookup_node (fib->fib_table,
				     pfxhash->pfx_ptr,
				     pfxhash->pfx_lens[i],
				     pfxhash->pfx_hashes[i], &pnode);
      if (ret != AOK)
	{			// this component and beneath not present
	  break;
	}
      node_array[node_count] = pnode;
      node_count++;
    }

  /* Now we've reached either a) the point where parents of the offered
   * prefix end, or b) the entry for the offered prefix.
   */
  while (i < pfxhash->pfx_count)
    {
      /*
       * There are more components in 'pfx' than are in the existing
       * fib. We need to add one or more entries, probably virtual.
       */

      /* Allocate a new node */
      pnode = cicn_hashtb_alloc_node (fib->fib_table);
      if (pnode == NULL)
	{
	  /* TODO -- clean up any new virtual nodes we've added! */
	  ret = ENOMEM;
	  goto done;
	}

      /* Set up the embedded virtual fib entry */
      fe = cicn_fib_get_data (pnode);

      /* Clear out the new entry */
      memset (fe, 0, sizeof (cicn_fib_entry_t));

      fe->fe_flags = CICN_FIB_ENTRY_FLAG_VIRTUAL;

      /* Set up the hash node */
      cicn_hashtb_init_node (fib->fib_table, pnode,
			     pfxhash->pfx_hashes[i],
			     pfxhash->pfx_ptr, pfxhash->pfx_lens[i]);

      /* Insert into the hashtable */
      ret = cicn_hashtb_insert (fib->fib_table, pnode);
      if (ret != AOK)
	{
	  /* Whoa - we didn't expect that */
	  goto done;
	  /* TODO -- cleanup on error */
	}

      /*
       * Save new nodes in the array too.
       */
      ASSERT (node_count < (CICN_HASHTB_MAX_NAME_COMPS - 1));
      node_array[node_count] = pnode;

      node_count++;
      i++;

      /* Count each added 'level' of prefixes */
      add_ref_val++;
    }

  /*
   * At this point, we've either found or added a new entry node,
   * it's the last one in the array of nodes, and we're ready
   * to set it up. If it's valid, we'll walk back
   * through the parents and update them (refcount, max-comps, etc.)
   */
  ASSERT (node_count > 0);
  pnode = node_array[node_count - 1];

  /* Set up (or update) the embedded actual fib entry */
  fe = cicn_fib_get_data (pnode);

  /* If this was an existing _virtual_ entry, convert it to a real one */
  fe->fe_flags &= ~(CICN_FIB_ENTRY_FLAG_VIRTUAL |
		    CICN_FIB_ENTRY_FLAG_DELETED);

  /* Next-hop face and weight. We'll _update_ a next-hop that matches
   * the current face, or else we'll add a new next-hop.
   */
  for (i = 0; i < ARRAY_LEN (fe->fe_next_hops); i++)
    {
      if (fe->fe_next_hops[i].nh_faceid == faceid)
	{
	  if (fe->fe_next_hops[i].nh_weight == weight)
	    {
	      ret = EEXIST;
	      goto done;
	    }

	  /* Found a matching entry */
	  fe->fe_next_hops[i].nh_weight = weight;
	  break;
	}
    }

  if (i == ARRAY_LEN (fe->fe_next_hops))
    {
      /* Didn't find a match, try to find a free nh slot */
      for (i = 0; i < ARRAY_LEN (fe->fe_next_hops); i++)
	{
	  cicn_fib_entry_nh_t *fe_nh = &fe->fe_next_hops[i];
	  if (fe->fe_next_hops[i].nh_faceid != 0)
	    {
	      continue;
	    }
	  /* Found a free entry */
	  ret = cicn_face_fib_nh_cnt_update (faceid, 1 /*add */ );
	  if (ret != AOK)
	    {			// should not happen
	      break;
	    }
	  fe_nh->nh_faceid = faceid;
	  fe_nh->nh_weight = weight;
	  break;
	}
      if (i == ARRAY_LEN (fe->fe_next_hops))
	{
	  /* Whoops - can't add any more next-hops */
	  /* TODO -- clean up on error at this point, since we may have added
	   * virtual nodes.
	   */
	  ret = ENOSPC;
	  crc = CICN_RC_FIB_NHOP_LIMIT;
	  goto done;
	}
    }


  /* Max comps */
  if (pfxhash->pfx_count > fe->fe_max_comps)
    {
      fe->fe_max_comps = pfxhash->pfx_count;
    }

  /*
   * Loop back through the nodes, updating the refcounts, max-comps, etc.
   */
  for (i = node_count - 1, j = 1; i >= 0; i--, j++)
    {
      pnode = node_array[i];

      fe = cicn_fib_get_data (pnode);

      /*
       * Update refcounts if we added any new prefixes.
       */
      if (add_ref_val > 0)
	{

	  /* Add refs to all the new nodes */
	  if (j < add_ref_val)
	    {
	      fe->fe_refcount += j;
	    }
	  else
	    {
	      /* Add refs for all the new nodes to existing parents */
	      fe->fe_refcount += add_ref_val;
	    }
	}

      if (pfxhash->pfx_count > fe->fe_max_comps)
	{
	  fe->fe_max_comps = pfxhash->pfx_count;
	}
    }

  ret = AOK;

done:
  if (cicn_rd)
    {
      cicn_rd_set (cicn_rd, crc, ret);
    }

  return (ret);
}

/*
 * Delete a FIB prefix, or just delete a next-hop, if 'faceid' != 0.
 * If the prefix has children, this may just result in the conversion of
 * the entry into a virtual entry.
 * We expect that the prefix-hashing has already been done, into 'pfxhash'.
 */
int
cicn_fib_entry_delete (cicn_fib_t * fib,
		       const cicn_prefix_hashinf_t * pfxhash, uint16_t faceid)
{
  int i, counter, ret = EINVAL;
  cicn_hash_node_t *node_array[CICN_HASHTB_MAX_NAME_COMPS];
  int node_count = 0;
  cicn_hash_node_t *pnode;
  cicn_fib_entry_t *fe;

  if ((fib == NULL) || (pfxhash == NULL) ||
      (pfxhash->pfx_ptr == NULL) || (pfxhash->pfx_len == 0) ||
      (pfxhash->pfx_count == 0) ||
      (pfxhash->pfx_count > CICN_HASHTB_MAX_NAME_COMPS))
    {
      goto done;
    }

  /* Start walking down the series of intermediate prefixes,
   * capturing the hash node at each level that already exists. We need
   * this in order to manage internal state, like refcounts
   * and virtual FIB nodes. We use the extended 'lookup' api so that we
   * will see hashtable nodes that were marked for deletion.
   */
  for (i = 0; i < pfxhash->pfx_count; i++)
    {
      ret = cicn_hashtb_lookup_node_ex (fib->fib_table,
					pfxhash->pfx_ptr,
					pfxhash->pfx_lens[i],
					pfxhash->pfx_hashes[i], TRUE, &pnode);
      if (ret != AOK)
	{			// should not happen
	  break;
	}
      node_array[i] = pnode;
    }

  /*
   * Now we've reached either a) the entry for the offered prefix, or the end
   * of the bread-crumb trail...
   */
  if (i < pfxhash->pfx_count)
    {
      /* If we can't get to the prefix specified,
       * then we can't really proceed?
       */
      ret = ENOENT;
      goto done;
    }

  node_count = i;
  pnode = node_array[node_count - 1];	// first update actual node, then parents
  fe = cicn_fib_get_data (pnode);

  /*
   * If we're clearing a single next-hop, see whether we should remove the
   * whole entry. 'pnode' is the last node in the array, which should be
   * the target entry.
   */
  if (faceid != 0)
    {
      counter = 0;
      ret = ENOENT;

      for (i = 0; i < ARRAY_LEN (fe->fe_next_hops); i++)
	{
	  cicn_fib_entry_nh_t *fe_nh = &fe->fe_next_hops[i];

	  /* Remove the specific next-hop */
	  if (fe_nh->nh_faceid == faceid)
	    {
	      cicn_face_fib_nh_cnt_update (faceid, 0 /*!add */ );
	      fe_nh->nh_faceid = 0;

	      /* Return success if we find the specified next-hop */
	      ret = AOK;
	    }

	  /* Count all existing next-hops */
	  if (fe_nh->nh_faceid != 0)
	    {
	      counter++;
	    }
	}

      if (counter != 0)
	{
	  /* Remove entire entry if no next-hops remaining */
	  goto done;
	}
    }

  /*
   * Remove entry if it's a leaf, or convert it to 'virtual' if not.
   */

  /* First clear out next-hop(s) */
  for (i = 0; i < ARRAY_LEN (fe->fe_next_hops); i++)
    {
      cicn_fib_entry_nh_t *fe_nh = &fe->fe_next_hops[i];

      if (fe_nh->nh_faceid == 0)
	{
	  continue;
	}
      cicn_face_fib_nh_cnt_update (fe_nh->nh_faceid, 0 /*!add */ );
      fe_nh->nh_faceid = 0;
    }

  if (fe->fe_refcount > 1)
    {
      /* Convert to virtual entry in-place */

      /* Set the 'virtual' flag */
      fe->fe_flags |= CICN_FIB_ENTRY_FLAG_VIRTUAL;

      /* No changes to parents, since we aren't changing the internal
       * prefix structure, so we're done.
       */
      ret = AOK;
      goto done;

    }
  else
    {
      /* Remove entry entirely */
      ret = cicn_hashtb_delete (fib->fib_table, &pnode);
      pnode = NULL;
    }


  /*
   * We've removed a node: loop back through the parents,
   * updating the refcounts, max-comps, etc. We may decide to remove
   * parent nodes too, if their only descendent has been deleted.
   */
  counter = 1;
  for (i = node_count - 2; i >= 0; i--)
    {
      pnode = node_array[i];

      fe = cicn_fib_get_data (pnode);

      fe->fe_refcount -= counter;

      /* TODO -- figure out whether we can do anything about the max-comps
       * at the parents' levels
       */

      if (fe->fe_refcount > 1)
	{
	  continue;
	}

      /* This entry is no longer ref'd; if it's 'virtual', we can
       * delete it too.
       */
      if (fe->fe_flags & CICN_FIB_ENTRY_FLAG_VIRTUAL)
	{
	  /* Remove entry entirely */
	  ret = cicn_hashtb_delete (fib->fib_table, &pnode);

	  /* Now more nodes have been removed, so more refs need to be
	   * removed at the parents...
	   */
	  counter++;
	}
    }

  ret = AOK;

done:

  return (ret);
}

/*
 * Mark a FIB prefix for delete, before actually deleting through a later
 * api call.
 * We expect that the prefix-hashing has already been done, into 'pfxhash'.
 * This will check virtual parents' refcounts, and will mark them for
 * delete also if necessary.
 */
int
cicn_fib_entry_mark_for_delete (cicn_fib_t * fib,
				const cicn_prefix_hashinf_t * pfxhash)
{
  int ret = EINVAL;

  return (ret);
}

/*
 * Add, delete, or change weight of fib entry next hop (which may
 * lead to add/delete of fib entry)
 */
int
cicn_fib_entry_nh_update (const char *prefix, int faceid, int weight,
			  int add_p, cicn_rd_t * cicn_rd_res)
{
  cicn_rd_t cicn_rd;

  int len;
  uint8_t buf[CICN_PARAM_FIB_ENTRY_PFX_WF_BYTES_MAX];
  cicn_prefix_hashinf_t pfxhash;

  cicn_rd_set (&cicn_rd, CICN_RC_OK, AOK);

  /* Check that we're init-ed */
  if (!cicn_infra_fwdr_initialized)
    {
      cicn_cli_output ("cicn: disabled");

      cicn_rd.rd_ux_rc = EINVAL;
      goto done;
    }

  /* Quick check for valid face for adds */
  if (add_p)
    {
      if (cicn_face_entry_find_by_id (faceid, NULL) != AOK)
	{
	  cicn_rd.rd_cicn_rc = CICN_RC_FACE_UNKNOWN;
	  cicn_rd.rd_ux_rc = EINVAL;
	  goto done;
	}
    }

  /* Convert prefix to wire-format */
  len = cicn_parse_name_comps_from_str (buf, sizeof (buf), prefix, &cicn_rd);
  if (len < 0)
    {
      goto done;
    }

  /* Hash the prefix */
  cicn_rd.rd_ux_rc =
    cicn_hashtb_hash_prefixes (buf, len, 0 /*full_name */ , &pfxhash,
			       0 /*limit */ );
  if (cicn_rd.rd_ux_rc != AOK)
    {
      goto done;
    }

  /* Call to the fib apis */
  if (add_p)
    {
      /* TODO -- support next-hop weight */
      cicn_rd.rd_ux_rc =
	cicn_fib_entry_insert (&cicn_main.fib, &pfxhash, faceid, weight,
			       &cicn_rd);
    }
  else
    {
      cicn_rd.rd_ux_rc =
	cicn_fib_entry_delete (&cicn_main.fib, &pfxhash, faceid);
    }

done:

  if (cicn_rd_res)
    {
      *cicn_rd_res = cicn_rd;
    }
  return (cicn_rd.rd_ux_rc);
}

/***************************************************************************
 * CICN_FIB management plane (debug cli, binary API) helper routines
 ***************************************************************************/

/*
 * CLI show output for fib. if 'prefix', just show a single entry
 */
int
cicn_fib_show (const char *prefix, int detail_p, int internal_p)
{
  int i, ret = 0;
  uint64_t cookie;
  cicn_hash_node_t *node;
  cicn_fib_entry_t *fe;
  char cbuf[CICN_PARAM_HASHTB_KEY_BYTES_MAX];
  u8 *strbuf = NULL;

  /* TODO -- use the supplied 'prefix' */

  /* Check that we're init-ed */
  if (!cicn_infra_fwdr_initialized)
    {
      cicn_cli_output ("cicn: disabled");

      ret = EINVAL;
      goto done;
    }

  cicn_cli_output ("cicn FIB:");

  /* Walk the FIB hashtable,  */
  cookie = CICN_HASH_WALK_CTX_INITIAL;

  while (cicn_hashtb_next_node (cicn_main.fib.fib_table,
				&node, &cookie) == AOK)
    {

      fe = cicn_fib_get_data (node);

      /* Skip virtual entries unless 'internal_p' */
      if (!internal_p && (fe->fe_flags & CICN_FIB_ENTRY_FLAG_VIRTUAL))
	{
	  continue;
	}

      vec_reset_length (strbuf);

      ret =
	cicn_hashtb_key_to_str (cicn_main.fib.fib_table, node, cbuf,
				sizeof (cbuf), 0 /*!must_fit */ );

      strbuf = format (strbuf, "  %s/...", cbuf);
      int pad = 16 - vec_bytes (strbuf);	// even out to column 16
      if (pad > 0)
	{
	  strbuf = format (strbuf, "%*s", pad, "");
	}


      if (fe->fe_flags & CICN_FIB_ENTRY_FLAG_VIRTUAL)
	{
	  strbuf = format (strbuf, " (virtual)");
	}

      if (internal_p)
	{
	  strbuf = format (strbuf, " (ref:%d)", fe->fe_refcount);
	}

      for (i = 0; i < ARRAY_LEN (fe->fe_next_hops); i++)
	{
	  if (fe->fe_next_hops[i].nh_faceid != 0)
	    {
	      strbuf = format (strbuf, " (face:%d, weight:%d)",
			       (int) (fe->fe_next_hops[i].nh_faceid),
			       (int) (fe->fe_next_hops[i].nh_weight));
	    }
	}

      /* Oy - vecs are neat, but ... */
      vec_terminate_c_string (strbuf);
      vlib_cli_output (cicn_main.vlib_main, "%s", strbuf);
    }

done:

  if (strbuf)
    {
      vec_free (strbuf);
    }

  return (ret);
}

/*
 * Binary serialization for show FIB API.
 */
int
  cicn_fib_api_entry_props_serialize
  (vl_api_cicn_api_fib_entry_props_get_reply_t * reply, int page)
{
  int rv = CICN_VNET_API_ERROR_NONE;

  int i, nentries = 0;

  uint64_t cookie;
  cicn_hash_node_t *node;
  cicn_fib_entry_t *fe;

  /* Check that we're init-ed */
  if (!cicn_infra_fwdr_initialized)
    {
      rv = VNET_API_ERROR_FEATURE_DISABLED;
      goto done;
    }

  /* Walk the FIB hashtable,  */
  cookie = CICN_HASH_WALK_CTX_INITIAL;

  while (cicn_hashtb_next_node (cicn_main.fib.fib_table,
				&node, &cookie) == AOK)
    {

      fe = cicn_fib_get_data (node);

      /* Skip virtual entries unless 'internal_p' */
      if ((fe->fe_flags & CICN_FIB_ENTRY_FLAG_VIRTUAL))
	{
	  continue;
	}

      /* TODO -- deal with overflow keys */
      i = node->hn_keysize;
      if (i > CICN_HASH_KEY_BYTES)
	{
	  i = CICN_HASH_KEY_LIST_BYTES;
	}

      cicn_api_fib_entry_t *entry = (cicn_api_fib_entry_t *)
	(&reply->fibentry[nentries * sizeof (cicn_api_fib_entry_t)]);

      cicn_hashtb_key_to_str (cicn_main.fib.fib_table, node,
			      (char *) entry->prefix, sizeof (entry->prefix),
			      0 /*!must_fit */ );

      for (i = 0; i < ARRAY_LEN (fe->fe_next_hops); i++)
	{
	  if (fe->fe_next_hops[i].nh_faceid == 0)
	    {
	      continue;
	    }
	  entry->faceid[i] =
	    clib_host_to_net_i32 (fe->fe_next_hops[i].nh_faceid);

	  entry->faceweight[i] =
	    clib_host_to_net_i32 (fe->fe_next_hops[i].nh_weight);

	  entry->nfaces = clib_host_to_net_i32 (i + 1);
	}

      nentries++;
      reply->nentries = clib_host_to_net_i32 (nentries);
    }

done:
  return (rv);
}



