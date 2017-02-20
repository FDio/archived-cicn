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
 * Internal API to ICN face table
 */
#ifndef _CICN_FACE_H_
#define _CICN_FACE_H_

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <vlibsocket/api.h>

#include "cicn_hello.h"

typedef struct cicn_face_stats_s
{
  CLIB_CACHE_LINE_ALIGN_MARK (cacheline0);
  uint64_t orig_interests;
  uint64_t orig_datas;
  uint64_t orig_naks;
  uint64_t term_interests;
  uint64_t term_datas;
  uint64_t term_naks;
  uint64_t in_interests;
  uint64_t in_datas;
  uint64_t in_naks;
  uint64_t out_interests;
  uint64_t out_datas;
  uint64_t out_naks;
} cicn_face_stats_t;

/*
 * Cache info about 'faces' so we can glue together the cicn
 * and vpp views of the world
 */
struct cicn_face_db_entry_s
{
  u32 flags;
  int faceid;			/* Our internal id */
  int swif;			/* VPP sw if index */
  int swif_dev_class_index;
  int swif_is_dpdk_driver;
  int swif_cloning_supported;
  int app_face;
  struct sockaddr_in src_addr;
  struct sockaddr_in dest_addr;
  u8 fe_ha_name_cmn[CICN_HELLO_NAME_CMN_FLEN];
  uint32_t fe_fib_nh_cnt;	/* Refcount of dependent FIB entries */
  cicn_hello_fcd_t fe_ha_fcd_loc;
  cicn_hello_fcd_t fe_ha_fcd_nbr;
};

/* Face cache flags */
#define CICN_FACE_FLAG_ADMIN_DOWN 0x02
#define CICN_FACE_FLAG_HELLO_DOWN 0x04
#define CICN_FACE_FLAG_DELETED    0x08

#define CICN_FACE_FLAGS_DEFAULT   0x00
#define CICN_FACE_FLAGS_DOWN_HARD \
    (CICN_FACE_FLAG_ADMIN_DOWN | CICN_FACE_FLAG_DELETED)
#define CICN_FACE_FLAGS_DOWN \
    (CICN_FACE_FLAGS_DOWN_HARD | CICN_FACE_FLAG_HELLO_DOWN)

/*
 * Cache info about 'faces' so we can glue together the cicn
 * and vpp views of the world
 */

/* TODO -- use vpp pool instead? */
typedef struct cicn_face_db_s
{
  int entry_count;
  cicn_face_db_entry_t entries[CICN_PARAM_FACES_MAX];
} cicn_face_db_t;

extern cicn_face_db_t cicn_face_db;

/* Create face, typically while handling cli input. Returns zero
 * and mails back the faceid on success.
 */
int cicn_face_add (uint32_t src_addr, uint16_t src_port,
		   uint32_t dest_addr, uint16_t dest_port,
		   int app_face, int swif, int *faceid_p,
		   cicn_rd_t * cicn_rd);

/* update (set or clear) supplied flags in face table entry */
void
cicn_face_flags_update (cicn_face_db_entry_t * face, int set, u32 uflags);
/* update refcount for add/delete of fib entry nh pointing to face */
int cicn_face_fib_nh_cnt_update (int faceid, int add);

/* Find a face entry by face id. Return AOK and mail back face on success */
int cicn_face_entry_find_by_id (int id, cicn_face_db_entry_t ** pface);

/* Find a face cache entry by address, from a packet e.g. */
int cicn_face_entry_find_by_addr (const struct sockaddr_in *src,
				  const struct sockaddr_in *dest,
				  cicn_face_db_entry_t ** pface);
/* Find face cache index (e.g. for distributed face statistics) */
int cicn_face_db_index (const cicn_face_db_entry_t * face);
/* Aggregate stats for one face across all cpus */
int cicn_face_stats_aggregate (const cicn_face_db_entry_t * face,
			       cicn_face_stats_t * face_stats);

/* CLI show output for faces. if 'faceid' >= 0, just show a single face */
int cicn_face_show (int faceid, int detail_p, int internal_p);

#endif // _CICN_FACE_H_
