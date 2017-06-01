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
 * cicn_parser.c: Fast-path, vpp-aware ICN packet parser, used in cicn forwarder.
 */

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include <vlib/vlib.h>

#include <cicn/cicn.h>
#include <cicn/cicn_parser.h>

/*
 * Given name string in ascii /a/b/... format, convert to wire-format
 * list of components (without wrapper tlv), in obuf.
 * If is_chunk_name, make last component chunk_type instead of generic type.
 */
int
cicn_parse_name_comps_from_str_generic (uint8_t * obuf, int obuflen,
					const char *str, int chunk_name,
					cicn_rd_t * cicn_rd)
{

  int ux_rc;
  cicn_rc_e crc = CICN_RC_OK;

  int ret;

  ret =
    cicn_parse_name_comps_from_str_inline (obuf, obuflen, str, chunk_name);

  if (ret >= 0)
    {
      ux_rc = AOK;
    }
  else
    {
      ux_rc = -ret;
      switch (ux_rc)
	{
	case ENOSPC:
	  crc = CICN_RC_FIB_PFX_SIZE_LIMIT;
	  break;
	default:
	  break;
	}
    }

  cicn_rd_set (cicn_rd, crc, ux_rc);
  return (ret);
}

/*
 * Given name string in ascii /a/b/... format, convert to wire-format
 * list of components (without wrapper tlv), in obuf.
 */
int
cicn_parse_name_comps_from_str (uint8_t * obuf, int obuflen, const char *str,
				cicn_rd_t * cicn_rd)
{
  int ret;

  ret =
    cicn_parse_name_comps_from_str_generic (obuf, obuflen, str,
					    0 /*!chunk_name */ , cicn_rd);
  return (ret);
}

/*
 * Given name string in ascii /a/b/... format, convert to full
 * wire-format (including wrapper tlv), in obuf.
 * If is_chunk_name, make last component chunk_type instead of generic type.
 */
int
cicn_parse_name_from_str (uint8_t * obuf, int obuflen, const char *str,
			  int is_chunk_name, cicn_rd_t * cicn_rd)
{
  int ret;
  if (obuflen < CICN_TLV_HDR_LEN)
    {
      cicn_rd_set (cicn_rd, CICN_RC_OK, EINVAL);
      return (-cicn_rd->rd_ux_rc);
    }
  C_PUTINT16 (obuf, CICN_TLV_NAME);
  ret =
    cicn_parse_name_comps_from_str_generic (obuf + CICN_TLV_HDR_LEN,
					    obuflen - CICN_TLV_HDR_LEN, str,
					    is_chunk_name, cicn_rd);
  if (ret >= 0)
    {				// length
      C_PUTINT16 (&obuf[CICN_TLV_TYPE_LEN], ret);
      ret += CICN_TLV_HDR_LEN;
    }
  return (ret);
}
