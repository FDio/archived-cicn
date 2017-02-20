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
 * cicn_hello_inlines.h - ICN hello protocol packet forwarding inlines
 */

#ifndef _CICN_HELLO_INLINES_H_
#define _CICN_HELLO_INLINES_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include "cicn.h"

/*
 * Is the supplied pkt_type/name a hello?
 * Called from forwarding path, so performance sensitive
 */
static inline int
cicn_hello_match (const cicn_face_db_entry_t * inface,
		  u8 pkt_type, const u8 * nameptr, u32 namelen,
		  const cicn_hello_name_t * hello_template, u64 * seq_num_res)
{
  const cicn_hello_fcd_t *fcd;
  const u8 *in_fid_tlv;
  u16 in_tlv_len;

  switch (pkt_type)
    {
    case CICN_PKT_TYPE_CONTROL_REQUEST:
      fcd = &inface->fe_ha_fcd_loc;	// request for our name
      break;
    case CICN_PKT_TYPE_CONTROL_REPLY:
      fcd = &inface->fe_ha_fcd_nbr;	// reply to our request for nbr name
      break;
    default:
      return (0);		// not a hello message
    }

  if (fcd->fcd_v_len == 0)
    {				// name not currently initialized
      return (0);
    }

  if (namelen != CICN_HELLO_NAME_TOT_FLEN)
    {
      return (0);
    }
  if (memcmp (nameptr, &inface->fe_ha_name_cmn[0],
	      sizeof (inface->fe_ha_name_cmn)))
    {
      return (0);
    }

  in_fid_tlv = &nameptr[CICN_HELLO_NAME_CMN_FLEN];
  C_GETINT16 (in_tlv_len, &in_fid_tlv[CICN_TLV_TYPE_LEN]);
  if (in_tlv_len != fcd->fcd_v_len)
    {
      return (0);
    }

  if (memcmp (&in_fid_tlv[CICN_TLV_HDR_LEN], &fcd->fcd_v[0],
	      fcd->fcd_v_len) != 0)
    {
      return (0);
    }

  if (seq_num_res)
    {				/* Extract the seq num from the response */
      u64 seq_num;
      C_GETINT64 (seq_num, &nameptr[namelen - CICN_HELLO_NAME_SEQ_V_LEN]);
      *seq_num_res = seq_num;
    }

  return (1);			// valid hello imsg/dmsg name for this face
}

#endif // _CICN_HELLO_INLINES_H_
