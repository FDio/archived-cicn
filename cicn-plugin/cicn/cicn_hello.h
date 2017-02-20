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
 * cicn_hello.h - ICN hello protocol operation
 */

#ifndef _CICN_HELLO_H_
#define _CICN_HELLO_H_

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <vnet/ip/ip.h>

#define CICN_HELLO_EVENT_DATA_RCVD 1

    /* hash of the hello protocol name */
#define CICN_HELLO_NAME_TEMPLATE "/local/adj/1234567890123456/12345678"
#define CICN_HELLO_NAME_CMN_COMPS_FLEN 16	// "/local/adj"

#define CICN_HELLO_NAME_FACEID_V_LEN 16
#define CICN_HELLO_NAME_FACEID_FLEN \
    (CICN_TLV_HDR_LEN + CICN_HELLO_NAME_FACEID_V_LEN)

#define CICN_HELLO_NAME_SEQ_V_LEN 8
#define CICN_HELLO_NAME_SEQ_FLEN (CICN_TLV_HDR_LEN + CICN_HELLO_NAME_SEQ_V_LEN)

// match pkt name against common-prefix and faceid components
#define CICN_HELLO_NAME_CMN_FLEN \
    (CICN_TLV_HDR_LEN + CICN_HELLO_NAME_CMN_COMPS_FLEN)
#define CICN_HELLO_NAME_TOT_FLEN					\
    (CICN_TLV_HDR_LEN + CICN_HELLO_NAME_CMN_COMPS_FLEN +		\
     CICN_HELLO_NAME_FACEID_FLEN + CICN_HELLO_NAME_SEQ_FLEN)

/* The name struct of the ICN Hello Interests */
typedef struct
{
  char hn_str[CICN_HELLO_NAME_TOT_FLEN];
  u32 hn_wf_v_len;
  u8 hn_wf[CICN_HELLO_NAME_TOT_FLEN + 10 /* slop */ ];
} cicn_hello_name_t;

typedef struct cicn_hello_fcd_t_
{
  u16 fcd_v_len;		// 0 if value not valid
  u8 fcd_v[CICN_HELLO_NAME_FACEID_V_LEN];
} cicn_hello_fcd_t;

/* ICN Hello Adjacency struct (piggy-backs on face_cache_entry_t) */
typedef struct
{
  int ha_swif;			// vpp swif, use as dummy RX for originated packets

  /* Is this adjacency active? */
  int active;

  /* Last received seq num */
  u64 last_received_seq_num;

  /* Last sent seq num */
  u64 last_sent_seq_num;
} cicn_hello_adj_t;

/*
 * The data structure to pass to the background process through
 * signaled event
 */
typedef struct
{
  u64 seq_num;
  u32 faceid;
} cicn_hello_data;

clib_error_t *cicn_hello_adj_update (i32 faceid, int enable);

u32 cicn_hello_periodic (vlib_main_t * vm, vlib_node_runtime_t * node);

int cicn_hello_plugin_activation_init (vlib_main_t * vm);

clib_error_t *cicn_hello_boot_init (vlib_main_t * vm);

#endif // _CICN_HELLO_H_
