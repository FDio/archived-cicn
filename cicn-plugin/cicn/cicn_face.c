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
 * Implementation if ICN face table
 */
#include <vlib/vlib.h>
#include <vppinfra/error.h>
#include <vnet/ip/format.h>

#include <cicn/cicn.h>

/*
 * ICN face underlying swif has a "device class" (e.g. dpdk, af-packet").
 * Currently, this determines if the device supports dpdk cloning or not.
 * Retrieve the class index for storage in a newly created face_db entry.
 */
static int
cicn_face_swif_dev_class_index (int swif)
{
  int dev_class_index = -1;
  int unix_rc = AOK;

  vnet_main_t *vnm;
  vnet_hw_interface_t *hw;

  vnm = vnet_get_main ();
  hw = vnet_get_sup_hw_interface (vnm, swif);
  if (hw == NULL)
    {
      unix_rc = ENODEV;
      goto done;
    }
  dev_class_index = hw->dev_class_index;

done:
  return ((dev_class_index >= 0) ? dev_class_index : -unix_rc);
}

/*
 * Return the face's swif's device class, for CLI show
 */
static const char *
cicn_face_dev_class_name (const cicn_face_db_entry_t * face)
{

  vnet_main_t *vnm;
  vnet_device_class_t *dev_class;

  vnm = vnet_get_main ();
  dev_class = vnet_get_device_class (vnm, face->swif_dev_class_index);
  return (dev_class ? dev_class->name : "???");
}

/*
 * Utility that adds a new face cache entry
 */
static int
cicn_face_db_add (uint32_t src_addr, uint16_t src_port,
		  uint32_t dest_addr, uint16_t dest_port,
		  int app_face, int swif, int *pfaceid)
{
  int ret = 0;

  cicn_face_db_entry_t *ptr;
  int faceid = 0;
  int dev_class_index;
  int is_dpdk_driver;
  int cloning_supported;

  vnet_device_class_t *dev_class;

  dev_class_index = cicn_face_swif_dev_class_index (swif);
  if (dev_class_index < 0)
    {
      ret = -dev_class_index;
      goto done;
    }
  dev_class = vnet_get_device_class (vnet_get_main (), dev_class_index);
  if (dev_class == NULL)
    {
      ret = ENOENT;
      goto done;
    }
  is_dpdk_driver = !strcmp (dev_class->name, "dpdk");
  cloning_supported = CICN_INFRA_CLONE_REPLICATION;

  if (cicn_face_db.entry_count >= CICN_PARAM_FACES_MAX)
    {
      ret = ENOMEM;
      goto done;
    }
  ptr = &cicn_face_db.entries[cicn_face_db.entry_count];
  faceid = cicn_face_db.entry_count + 1;	/* Start at one, not zero */
  cicn_face_db.entry_count++;

  ptr->src_addr.sin_addr.s_addr = src_addr;
  ptr->src_addr.sin_port = htons (src_port);
  ptr->dest_addr.sin_addr.s_addr = dest_addr;
  ptr->dest_addr.sin_port = htons (dest_port);
  ptr->app_face = app_face;
  ptr->faceid = faceid;
  ptr->swif = swif;
  ptr->swif_dev_class_index = dev_class_index;
  ptr->swif_is_dpdk_driver = is_dpdk_driver;
  ptr->swif_cloning_supported = cloning_supported;
  ptr->flags = CICN_FACE_FLAGS_DEFAULT;
  ptr->fe_fib_nh_cnt = 0;


done:
  if (pfaceid)
    {
      *pfaceid = faceid;
    }

  return (ret);
}

/* TODO -- delete face, deactivate/down face apis? */

/*
 * Locate a face cache entry by face id
 * TODO: Replace linear scan with faster lookup
 */
int
cicn_face_entry_find_by_id (int id, cicn_face_db_entry_t ** pface)
{
  int i, ret = ENOENT;
  cicn_face_db_entry_t *ptr = cicn_face_db.entries;

  for (i = 0; i < cicn_face_db.entry_count; i++)
    {
      if (ptr->faceid == id)
	{

	  /* Don't return/access deleted entries */
	  if (ptr->flags & CICN_FACE_FLAG_DELETED)
	    {
	      goto done;
	    }

	  if (pface)
	    {
	      *pface = ptr;
	    }

	  ret = AOK;
	  break;
	}

      ptr++;
    }

done:

  return (ret);
}

/*
 * Find a face cache entry by address, from a packet e.g.
 */
int
cicn_face_entry_find_by_addr (const struct sockaddr_in *src,
			      const struct sockaddr_in *dest,
			      cicn_face_db_entry_t ** pface)
{
  int i, ret = ENOENT;
  cicn_face_db_entry_t *ptr = cicn_face_db.entries;

  for (i = 0; i < cicn_face_db.entry_count; ptr++, i++)
    {
      if ((ptr->src_addr.sin_addr.s_addr == src->sin_addr.s_addr) &&
	  (ptr->src_addr.sin_port == src->sin_port) &&
	  (ptr->dest_addr.sin_addr.s_addr == dest->sin_addr.s_addr) &&
	  (ptr->dest_addr.sin_port == dest->sin_port))
	{

	  /* Don't return/access deleted entries */
	  if (ptr->flags & CICN_FACE_FLAG_DELETED)
	    {
	      goto done;
	    }

	  if (pface)
	    {
	      *pface = ptr;
	    }
	  ret = AOK;
	  break;
	}
    }

done:

  return (ret);
}

int
cicn_face_db_index (const cicn_face_db_entry_t * face)
{
  return (face - cicn_face_db.entries);
}

int
cicn_face_stats_aggregate (const cicn_face_db_entry_t * face,
			   cicn_face_stats_t * face_stats)
{
  int fcidx;
  u32 index = 0;
  const cicn_face_stats_t *cpu_face_stats;

  memset (face_stats, 0, sizeof (*face_stats));

  fcidx = cicn_face_db_index (face);

  foreach_vlib_main (
		      {
		      cpu_face_stats =
		      &cicn_infra_shards[index].face_stats[fcidx];
		      face_stats->orig_interests +=
		      cpu_face_stats->orig_interests;
		      face_stats->orig_datas += cpu_face_stats->orig_datas;
		      face_stats->orig_naks += cpu_face_stats->orig_naks;
		      face_stats->term_interests +=
		      cpu_face_stats->term_interests;
		      face_stats->term_datas += cpu_face_stats->term_datas;
		      face_stats->term_naks += cpu_face_stats->term_naks;
		      face_stats->in_interests +=
		      cpu_face_stats->in_interests;
		      face_stats->in_datas += cpu_face_stats->in_datas;
		      face_stats->in_naks += cpu_face_stats->in_naks;
		      face_stats->out_interests +=
		      cpu_face_stats->out_interests;
		      face_stats->out_datas += cpu_face_stats->out_datas;
		      face_stats->out_naks += cpu_face_stats->out_naks;
		      index++;
		      }
  );
  return (AOK);
}

/*
 * Create face, typically while handling cli input. Returns zero
 * and mails back the faceid on success.
 */
int
cicn_face_add (uint32_t src_addr, uint16_t src_port,
	       uint32_t dest_addr, uint16_t dest_port,
	       int app_face, int swif, int *faceid_p, cicn_rd_t * cicn_rd)
{
  int ret = AOK;
  cicn_rc_e crc = CICN_RC_OK;

  int faceid;

  if (!cicn_infra_fwdr_initialized)
    {
      cicn_cli_output ("cicn: fwdr disabled");
      ret = EINVAL;
      goto done;
    }
  /* check for face already existing */
  struct sockaddr_in src, dst;
  src.sin_addr.s_addr = src_addr;
  src.sin_port = htons (src_port);

  dst.sin_addr.s_addr = dest_addr;
  dst.sin_port = htons (dest_port);
  ret = cicn_face_entry_find_by_addr (&src, &dst, NULL /*!pface */ );
  if (ret == AOK)
    {
      ret = EEXIST;
      goto done;
    }

  /* TODO -- support delete also? */

  /* TODO -- check face cache for dup before trying to add? */

  /* Add to face cache */
  ret =
    cicn_face_db_add (src_addr, src_port, dest_addr, dest_port, app_face,
		      swif, &faceid);

done:

  if ((ret == AOK) && faceid_p)
    {
      *faceid_p = faceid;
    }
  if (cicn_rd)
    {
      cicn_rd_set (cicn_rd, crc, ret);
    }

  return (ret);
}

void
cicn_face_flags_update (cicn_face_db_entry_t * face, int set, u32 uflags)
{
  u32 oflags, nflags;

  oflags = nflags = face->flags;

  if (set)
    {
      nflags |= uflags;
    }
  else
    {
      nflags &= ~uflags;
    }

  if (oflags == nflags)
    {
      return;
    }
  face->flags = nflags;

  if (oflags & CICN_FACE_FLAGS_DOWN)
    {
      if (!(nflags & CICN_FACE_FLAGS_DOWN))
	{
	  // face => up
	}
    }
  else
    {
      if (nflags & CICN_FACE_FLAGS_DOWN)
	{
	  // face => down
	}
    }
}

/*
 * based on add being true/false, increment/decrement count of
 * fib nexthops using face
 *
 * return success/error for supplied faceid being valid/invalid
 */
int
cicn_face_fib_nh_cnt_update (int faceid, int add)
{
  int ret;
  cicn_face_db_entry_t *face;

  ret = cicn_face_entry_find_by_id (faceid, &face);
  if (ret != 0)
    {
      goto done;
    }
  face->fe_fib_nh_cnt += add ? 1 : -1;

done:
  return (ret);
}

/***************************************************************************
 * CICN_FACE management plane (binary API, debug cli) helper routines
 ***************************************************************************/

/*
 * Binary serialization for get face configuration API.
 */
vnet_api_error_t
cicn_face_api_entry_params_serialize (int faceid,
				      vl_api_cicn_api_face_params_get_reply_t
				      * reply)
{
  vnet_api_error_t rv = VNET_API_ERROR_NO_SUCH_ENTRY;

  if (!reply)
    {
      rv = VNET_API_ERROR_INVALID_ARGUMENT;
      goto done;
    }

  int i;
  for (i = 0; i < cicn_face_db.entry_count; i++)
    {
      if (i >= CICN_PARAM_FACES_MAX)
	{			// should never happen
	  break;
	}
      if (cicn_face_db.entries[i].faceid != faceid)
	{
	  continue;
	}

      reply->local_addr =
	clib_host_to_net_u32 (cicn_face_db.entries[i].src_addr.
			      sin_addr.s_addr);
      reply->local_port = cicn_face_db.entries[i].src_addr.sin_port;

      reply->remote_addr =
	clib_host_to_net_u32 (cicn_face_db.entries[i].dest_addr.
			      sin_addr.s_addr);
      reply->remote_port = cicn_face_db.entries[i].dest_addr.sin_port;

      reply->flags = clib_host_to_net_i32 (cicn_face_db.entries[i].flags);

      reply->sw_interface_id =
	clib_host_to_net_i32 (cicn_face_db.entries[i].swif);

      rv = CICN_VNET_API_ERROR_NONE;
      break;
    }

done:
  return (rv);
}

/*
 * Binary serialization for show faces API.
 */
int
cicn_face_api_entry_props_serialize (vl_api_cicn_api_face_props_get_reply_t *
				     reply)
{
  int rv = CICN_VNET_API_ERROR_NONE;
  int i;

  if (!reply)
    {
      rv = VNET_API_ERROR_INVALID_ARGUMENT;
      goto done;
    }

  for (i = 0; i < cicn_face_db.entry_count; i++)
    {
      if (i >= CICN_PARAM_FACES_MAX)
	{			// should never happen
	  break;
	}

      cicn_face_db_entry_t *face = &cicn_face_db.entries[i];
      cicn_api_face_entry_t *api_face = (cicn_api_face_entry_t *)
	(&reply->face[i * sizeof (cicn_api_face_entry_t)]);

      api_face->faceid = clib_host_to_net_i32 (face->faceid);

      api_face->local_addr =
	clib_host_to_net_u32 (face->src_addr.sin_addr.s_addr);
      api_face->local_port = face->src_addr.sin_port;

      api_face->remote_addr =
	clib_host_to_net_u32 (face->dest_addr.sin_addr.s_addr);
      api_face->remote_port = face->dest_addr.sin_port;

      api_face->flags = clib_host_to_net_i32 (face->flags);

      api_face->sw_interface_id = clib_host_to_net_i32 (face->swif);

      api_face->fib_nhs = clib_host_to_net_u32 (face->fe_fib_nh_cnt);
    }
  reply->nentries = clib_host_to_net_i32 (i);

done:
  return (rv);
}


/*
 * Binary serialization for face statistics API.
 */
int
cicn_face_api_entry_stats_serialize (int faceid,
				     vl_api_cicn_api_face_stats_get_reply_t *
				     reply)
{
  vnet_api_error_t rv = VNET_API_ERROR_NO_SUCH_ENTRY;

  if (!reply)
    {
      rv = VNET_API_ERROR_INVALID_ARGUMENT;
      goto done;
    }

  int i;
  for (i = 0; i < cicn_face_db.entry_count; i++)
    {
      if (i >= CICN_PARAM_FACES_MAX)
	{			// should never happen
	  break;
	}

      cicn_face_db_entry_t *face = &cicn_face_db.entries[i];

      if (face->faceid != faceid)
	{
	  continue;
	}

      cicn_face_stats_t f_stats;

      reply->faceid = clib_host_to_net_i32 (face->faceid);

      cicn_face_stats_aggregate (&cicn_face_db.entries[i], &f_stats);

      reply->orig_interests = clib_host_to_net_u64 (f_stats.orig_interests);
      reply->orig_datas = clib_host_to_net_u64 (f_stats.orig_datas);
      reply->orig_naks = clib_host_to_net_u64 (f_stats.orig_naks);
      reply->term_interests = clib_host_to_net_u64 (f_stats.term_interests);
      reply->term_datas = clib_host_to_net_u64 (f_stats.term_datas);
      reply->term_naks = clib_host_to_net_u64 (f_stats.term_naks);
      reply->in_interests = clib_host_to_net_u64 (f_stats.in_interests);
      reply->in_datas = clib_host_to_net_u64 (f_stats.in_datas);
      reply->in_naks = clib_host_to_net_u64 (f_stats.in_naks);
      reply->out_interests = clib_host_to_net_u64 (f_stats.out_interests);
      reply->out_datas = clib_host_to_net_u64 (f_stats.out_datas);
      reply->out_naks = clib_host_to_net_u64 (f_stats.out_naks);

      rv = CICN_VNET_API_ERROR_NONE;
      break;
    }

done:
  return (rv);
}

/*
 * CLI show output for faces. if 'faceid' >= 0, just show a single face
 */
int
cicn_face_show (int faceid_arg, int detail_p, int internal_p)
{
  int ret = 0;
  int i;
  u8 *sbuf = 0, *dbuf = 0;

  cicn_cli_output ("Faces:");

  for (i = 0; i < cicn_face_db.entry_count; i++)
    {
      cicn_face_db_entry_t *face = &cicn_face_db.entries[i];
      int efaceid = face->faceid;
      if ((faceid_arg >= 0) && (faceid_arg != efaceid))
	{
	  continue;
	}

      vec_reset_length (sbuf);
      sbuf = format (sbuf, "%U", format_ip4_address,
		     &face->src_addr.sin_addr);
      vec_terminate_c_string (sbuf);

      vec_reset_length (dbuf);
      dbuf = format (dbuf, "%U", format_ip4_address,
		     &face->dest_addr.sin_addr);
      vec_terminate_c_string (dbuf);

      char *if_status = "unknown";
      if (face->flags & CICN_FACE_FLAG_DELETED)
	{
	  if_status = "DELETED";
	}
      else if (face->flags & CICN_FACE_FLAG_ADMIN_DOWN)
	{
	  if_status = "admin-down";
	}
      else if (face->flags & CICN_FACE_FLAGS_DOWN)
	{
	  if_status = "oper-down";
	}
      else
	{
	  if_status = "up";
	}
      cicn_cli_output ("    Face %d: %s:%d <-> %s:%d (swif %d)",
		       face->faceid, sbuf, ntohs (face->src_addr.sin_port),
		       dbuf, ntohs (face->dest_addr.sin_port), face->swif);

      cicn_cli_output ("\tFace Type:%s, State:%s, FIB_NHs:%d, Class:%s(%s)",
		       face->app_face ? "app" : "peer", if_status,
		       face->fe_fib_nh_cnt, cicn_face_dev_class_name (face),
		       face->swif_cloning_supported ? "clone" : "copy");

      // TODO: More output.
      cicn_main_t *sm = &cicn_main;
      if (sm->cicn_hello_adjs[efaceid].active)
	{
	  cicn_cli_output
	    ("\t%-14.14s State:enabled,%s [last_sent:%lu, last_rcvd:%lu]",
	     "Hello Proto:",
	     (face->flags & CICN_FACE_FLAG_HELLO_DOWN) ? "down" : "up",
	     sm->cicn_hello_adjs[efaceid].last_sent_seq_num,
	     sm->cicn_hello_adjs[efaceid].last_received_seq_num);
	}
      else
	{
	  cicn_cli_output ("\tHello Protocol: State:disabled");
	}

      cicn_face_stats_t face_stats;
      cicn_face_stats_aggregate (face, &face_stats);

#define CICN_SHOW_MSGS_FMT "\t%-14.14s Interests:%lu, Data:%lu, Naks:%lu"
      cicn_cli_output (CICN_SHOW_MSGS_FMT, "Originated:",
		       face_stats.orig_interests, face_stats.orig_datas,
		       face_stats.orig_naks);

      cicn_cli_output (CICN_SHOW_MSGS_FMT, "Terminated:",
		       face_stats.term_interests, face_stats.term_datas,
		       face_stats.term_naks);

      cicn_cli_output (CICN_SHOW_MSGS_FMT, "Received:",
		       face_stats.in_interests, face_stats.in_datas,
		       face_stats.in_naks);

      cicn_cli_output (CICN_SHOW_MSGS_FMT, "Sent:",
		       face_stats.out_interests, face_stats.out_datas,
		       face_stats.out_naks);

      if (faceid_arg >= 0)
	{			// found it
	  break;
	}
    }

  if (sbuf)
    {
      vec_free (sbuf);
    }
  if (dbuf)
    {
      vec_free (dbuf);
    }

  return (ret);
}
