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
 * cicn_api.h: definitions shared between plugin and its api clients
 */

#ifndef _cicn_api_h_
#define _cicn_api_h_

/* CICN API constants */
// for search (vpp currently uses 0 directly rather than VNET_API_ERROR_NONE)
#define CICN_VNET_API_ERROR_NONE ((vnet_api_error_t)0)

#define CICN_API_FIB_ENTRY_NHOP_WGHT_UNSET	(-1)

/* define message structures */
#define vl_typedefs
#include <cicn/cicn_all_api_h.h>
#undef vl_typedefs

/* Face entry:
 * Total size: 24 bytes
 */
typedef struct cicn_api_face_entry_s
{
  /* Face ID */
  i32 faceid;

  /* Local IP address */
  u32 local_addr;

  /* Local port */
  u16 local_port;

  /* Remote IP address */
  u32 remote_addr;

  /* Remote port */
  u16 remote_port;

  /* Face flags */
  i32 flags;

  /* VPP interface (index) associated with the face */
  i32 sw_interface_id;

  u32 fib_nhs;			/* fib nhs using this face */
} cicn_api_face_entry_t;

/* FIB entry: 500-byte long name prefixes and up to 16 next-hops
 * Total size: 634 bytes
 */
typedef struct cicn_api_fib_entry_s
{
  /* Name prefix */
  u8 prefix[500];

  /* Number of valid next-hops (face) */
  i32 nfaces;

  /* Next-hop Face IDs */
  i32 faceid[16];

  /* Face wights */
  i32 faceweight[16];
} cicn_api_fib_entry_t;

/* test suite results entry: suite name and results
 * Total size: 176 bytes
 */
typedef struct cicn_api_test_suite_results_s
{
  u8 suitename[128];

  i32 ntests;			// Number of tests requested
  i32 nsuccesses;
  i32 nfailures;
  i32 nskipped;

  u8 failures_mask[16];
  u8 skips_mask[16];
} cicn_api_test_suite_results_t;

#endif // _cicn_api_h_
