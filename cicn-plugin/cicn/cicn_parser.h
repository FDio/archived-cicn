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
 * cicn_parser.h: Fast-path, vpp-aware ICN packet parser, used in cicn forwarder.
 */

#ifndef _CICN_PARSER_H_
#define _CICN_PARSER_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <ctype.h>

#include <vlib/vlib.h>

#include <cicn/cicn_types.h>
#include <cicn/cicn_std.h>

#ifndef AOK
#define AOK 0
#endif

/*
 * Useful macros for working with integers in possibly-unaligned
 * buffers and possibly-byteswapped architectures
 */
#define C_GETINT16(n, p)        do {           \
    register uint8_t *t_p = (uint8_t *)(p);    \
    (n) = ((uint16_t)t_p[0] << 8)	       \
	| ((uint16_t)t_p[1]);		       \
    } while (0)

#define C_GETINT32(n, p)        do {           \
    register uint8_t *t_p = (uint8_t *)(p);    \
    (n) = ((uint32_t)t_p[0] << 24)	       \
	| ((uint32_t)t_p[1] << 16)	       \
	| ((uint32_t)t_p[2] << 8)	       \
	| ((uint32_t)t_p[3]);		       \
    } while (0)

#define C_GETINT64(n, p)        do {           \
    register uint8_t *t_p = (uint8_t *)(p);    \
    (n) = ((uint64_t)t_p[0] << 56)	       \
	| ((uint64_t)t_p[1] << 48)	       \
	| ((uint64_t)t_p[2] << 40)	       \
	| ((uint64_t)t_p[3] << 32)	       \
	| ((uint64_t)t_p[4] << 24)	       \
	| ((uint64_t)t_p[5] << 16)	       \
	| ((uint64_t)t_p[6] << 8)	       \
	| ((uint64_t)t_p[7]);		       \
    } while (0)

#define C_PUTINT16(p, n)        do {                    \
    register uint16_t t_n = (uint16_t)(n);              \
    register uint8_t *t_p = (uint8_t *)(p);		\
    t_p[0] = (uint8_t)(t_n >> 8);			\
    t_p[1] = (uint8_t)(t_n);				\
    } while (0)

#define C_PUTINT32(p, n)        do {                     \
        register uint32_t t_n = (uint32_t)(n);           \
        register uint8_t *t_p = (uint8_t *)(p);          \
        t_p[0] = (uint8_t)(t_n >> 24);                   \
        t_p[1] = (uint8_t)(t_n >> 16);                   \
        t_p[2] = (uint8_t)(t_n >> 8);                    \
        t_p[3] = (uint8_t)t_n;                           \
    } while (0)

#define C_PUTINT64(p, n)        do {                     \
        register uint64_t t_n = (uint64_t)(n);           \
        register uint8_t *t_p = (uint8_t *)(p);          \
        t_p[0] = (uint8_t)(t_n >> 56);                   \
        t_p[1] = (uint8_t)(t_n >> 48);                   \
        t_p[2] = (uint8_t)(t_n >> 40);                   \
        t_p[3] = (uint8_t)(t_n >> 32);                   \
        t_p[4] = (uint8_t)(t_n >> 24);                   \
        t_p[5] = (uint8_t)(t_n >> 16);                   \
        t_p[6] = (uint8_t)(t_n >> 8);                    \
        t_p[7] = (uint8_t)t_n;                           \
    } while (0)

/*
 * Key type codes for header, header tlvs, body tlvs, and child tlvs
 */

enum cicn_pkt_type_e
{
  CICN_PKT_TYPE_INTEREST = 0,
  CICN_PKT_TYPE_CONTENT = 1,
  CICN_PKT_TYPE_NAK = 2,
  CICN_PKT_TYPE_CONTROL = 0xA4,
  CICN_PKT_TYPE_CONTROL_REQUEST = CICN_PKT_TYPE_CONTROL + 1,
  CICN_PKT_TYPE_CONTROL_REPLY
};

enum cicn_msg_type_e
{
  CICN_MSG_TYPE_INTEREST = 1,
  CICN_MSG_TYPE_CONTENT = 2,
  CICN_MSG_TYPE_CONTROL = 0xBEEF,
  CICN_MSG_TYPE_ECHO_REQUEST = CICN_MSG_TYPE_CONTROL + 1,
  CICN_MSG_TYPE_ECHO_REPLY,
  CICN_MSG_TYPE_TRACEROUTE_REQUEST,
  CICN_MSG_TYPE_TRACEROUTE_REPLY,
};

enum cicn_hdr_tlv_e
{
  CICN_HDR_TLV_INT_LIFETIME = 1,
  CICN_HDR_TLV_CACHE_TIME = 2,
};

enum cicn_tlv_e
{
  CICN_TLV_NAME = 0,
  CICN_TLV_PAYLOAD = 1,
  CICN_TLV_PAYLOAD_TYPE = 5,
  CICN_TLV_MSG_EXPIRY = 6,
};

enum cicn_name_comp_e
{
  CICN_NAME_COMP = 1,
  CICN_NAME_COMP_CHUNK = 16
};

enum cicn_msg_err_e
{
  CICN_MSG_ERR_NOROUTE = 1,
  CICN_MSG_ERR_HOPLIM = 2,
  CICN_MSG_ERR_RESOURCES = 3,
  CICN_MSG_ERR_CONGESTION = 6,
  CICN_MSG_ERR_MTU = 7
};

/*
 * Fixed packet header
 */
typedef struct __attribute__ ((__packed__)) cicn_packet_hdr_s
{
  uint8_t pkt_ver;
  uint8_t pkt_type;
  uint16_t pkt_len;
  uint8_t pkt_hop_limit;
  uint8_t pkt_reserved;
#define pkt_nack_code pkt_reserved
  uint8_t pkt_flags;
  uint8_t pkt_hdr_len;
} cicn_packet_hdr_t;

typedef struct cicn_pkt_hdr_desc_s
{
  int16_t ph_lifetime_idx;
} cicn_pkt_hdr_desc_t;

/* Simple min packet len */
#define CICN_PACKET_MIN_LEN  \
    (sizeof(cicn_packet_hdr_t) + /*msg tlv*/ 4 + \
     /*name tlv*/ 4 + /*name comp*/2 + 1)

/* Protocol versions */
#define CICN_PROTO_VERSION_1  0x01
#define CICN_PROTO_VERSION_CURRENT CICN_PROTO_VERSION_1

/* The versions we can deal with */
#define CICN_PROTO_VERSION_MIN  CICN_PROTO_VERSION_CURRENT
#define CICN_PROTO_VERSION_MAX  CICN_PROTO_VERSION_CURRENT

/* Default initial hop limit */
#define CICN_DEFAULT_HOP_LIMIT 128

/* Current encoding restricts TLV 'T' and 'L' to two bytes */
#define CICN_TLV_TYPE_LEN 2
#define CICN_TLV_LEN_LEN  2
#define CICN_TLV_HDR_LEN (CICN_TLV_TYPE_LEN + CICN_TLV_LEN_LEN)
#define CICN_TLV_MAX_LEN  0xffff

#define cicn_parse_tlvtype(p)	((((uint8_t*)(p))[0]<<8) | ((uint8_t*)(p))[1])
#define cicn_parse_tlvlength(p)	((((uint8_t*)(p))[2]<<8) | ((uint8_t*)(p))[3])

static inline uint64_t
cicn_parse_getvlv (uint8_t * p, uint8_t * e)
{
  u64 v;

  /*
   * Should check
   *  if(e <= p || e - p > 8)
   * - it's an error.
   */
  v = *p++;
  while (p < e)
    {
      v = (v << 8) | *p++;
    }

  return v;
}

/*
 * Wrapper to fill in tlv hdr (type and len) for tlv under constructions.
 * Byte-swapps if needed for this CPU.
 */
static inline void
cicn_parse_tlv_hdr_build (uint8_t * tlv, uint16_t type, uint16_t len)
{
  C_PUTINT16 (&tlv[0], type);
  C_PUTINT16 (&tlv[CICN_TLV_TYPE_LEN], len);
}

/*
 * Wrapper to build tlv given the type, the length, and the pre-constructed
 * value
 */
static inline void
cicn_parse_tlv_build (uint8_t * tlv, uint16_t type, uint16_t len,
		      const uint8_t * v)
{
  cicn_parse_tlv_hdr_build (tlv, type, len);
  clib_memcpy (&tlv[CICN_TLV_HDR_LEN], v, len);
}

/*
 * Quickie packet sanity check: check lengths, locate name
 */
static inline int
cicn_parse_pkt (uint8_t * pkt, uint32_t pktlen, uint8_t * type_p,
		uint16_t * msg_type_p, uint8_t ** name_p,
		uint32_t * namelen_p, cicn_pkt_hdr_desc_t * pkt_hdr_desc)
{
  int ret = EINVAL;
  uint8_t *startptr;
  const uint8_t *endptr;
  uint8_t type;
  uint16_t sval;
  uint8_t hdr_len;

  if (pkt == NULL || pktlen < CICN_PACKET_MIN_LEN)
    {
      goto error;
    }

  startptr = pkt;
  endptr = pkt + pktlen;

  if ((*pkt < CICN_PROTO_VERSION_MIN) || (*pkt > CICN_PROTO_VERSION_MAX))
    {
      goto error;
    }

  pkt++;

  /* TODO -- validate packet type or make the caller do it? */
  type = *pkt;
  if (type_p)
    {
      *type_p = type;
    }

  /* Advance to and check header's packet len */
  pkt++;

  C_GETINT16 (sval, pkt);
  if (startptr + sval > endptr)
    {
      goto error;
    }

  /* TODO -- check hop limit here, or let caller do it? */

  /* Advance past hop limit and reserved bits */
  pkt += 4;

  /* TODO -- any 'flags' to check? */

  /* Advance to header-len field */
  pkt++;
  hdr_len = *pkt;

  /* Check header-len; must be enough room for at least a message tlv and
   * a name tlv.
   */
  if ((startptr + hdr_len) > (endptr - 4 /*msg */  - 4 /*name */ ))
    {
      goto error;
    }

  /* header options we care about */
  pkt_hdr_desc->ph_lifetime_idx = -1;
  uint8_t *hdr_tlv = pkt + 1;
  pkt = startptr + hdr_len;
  while (hdr_tlv < pkt)
    {
      uint16_t hdr_tlv_type = cicn_parse_tlvtype (hdr_tlv);
      uint16_t hdr_tlv_len =
	CICN_TLV_HDR_LEN + cicn_parse_tlvlength (hdr_tlv);
      if (hdr_tlv + hdr_tlv_len > pkt)
	{
	  goto error;
	}

      switch (hdr_tlv_type)
	{
	case CICN_HDR_TLV_INT_LIFETIME:
	  if (type == CICN_PKT_TYPE_INTEREST)
	    {
	      pkt_hdr_desc->ph_lifetime_idx = hdr_tlv - startptr;
	    }
	case CICN_HDR_TLV_CACHE_TIME:
	  if (type == CICN_PKT_TYPE_CONTENT)
	    {
	      pkt_hdr_desc->ph_lifetime_idx = hdr_tlv - startptr;
	    }
	  break;
	default:
	  break;
	}
      hdr_tlv += hdr_tlv_len;
    }

  /* Capture message type. TODO -- validate/enforce msg types. */
  C_GETINT16 (sval, pkt);
  if (msg_type_p)
    {
      *msg_type_p = sval;
    }

  pkt += 2;

  /* Check len of message tlv. TODO -- not checking for other per-msg tlvs */
  C_GETINT16 (sval, pkt);
  if (((pkt + sval + CICN_TLV_LEN_LEN) > endptr) || (sval < 4 /*name */ ))
    {
      goto error;
    }

  pkt += 2;

  /* Must find name first in the 'message' */
  C_GETINT16 (sval, pkt);
  if (sval != CICN_TLV_NAME)
    {
      goto error;
    }

  /* Capture start of name */
  if (name_p)
    {
      *name_p = pkt;
    }

  pkt += 2;

  /* Validate len of name tlv
   *  - zero _is_ a validate name len
   *  - TODO should compare embedded name len with containing message tlv len
   */
  C_GETINT16 (sval, pkt);
  if ((pkt + sval + CICN_TLV_LEN_LEN) > endptr)
    {
      goto error;
    }

  if (namelen_p)
    {
      /* Return the whole length from the start of the Name tlv,
       * including the T and L octets.
       */
      *namelen_p = sval + CICN_TLV_TYPE_LEN + CICN_TLV_LEN_LEN;
    }

  /* Looks ok so far... */
  ret = AOK;

  return (ret);

error:
  if (type_p)
    {
      *type_p = 0;		// compiler warning
    }
  if (msg_type_p)
    {
      *msg_type_p = 0;		// compiler warning
    }
  return (ret);
}

/*
 * Process optional time-based Hop-by-hop headers.
 * Packet already verified for sanity by cicn_parse_pkt().
 * An Interest Lifetime TLV will affect the PIT timeout
 * value, or whether the interest should be put in the PIT
 * at all (if the value is 0 then no content is expected).
 * Caching will use the Recommended Cache Time TLV.
 */
static inline int
cicn_parse_hdr_time_ms (uint8_t * pkt, cicn_pkt_hdr_desc_t * pkt_hdr_desc,
			uint16_t type, uint64_t * time_res)
{
  uint8_t *p;
  uint16_t len;
  uint64_t v;

  if (pkt_hdr_desc->ph_lifetime_idx == -1)
    {
      return (ENOENT);
    }

  p = pkt + pkt_hdr_desc->ph_lifetime_idx;
  len = cicn_parse_tlvlength (p);

  switch (type)
    {
    case CICN_HDR_TLV_INT_LIFETIME:
      if (len > 8)
	{
	  return (ENOENT);
	}
      v =
	cicn_parse_getvlv (p + CICN_TLV_HDR_LEN, p + CICN_TLV_HDR_LEN + len);
      break;
    case CICN_HDR_TLV_CACHE_TIME:
      if (len != 8)
	{
	  return (ENOENT);
	}
      C_GETINT64 (v, p + CICN_TLV_HDR_LEN);
      break;
    default:
      return (ENOENT);
      break;
    }

  *time_res = v;
  return AOK;
}

/*
 * skip over pkt_hdr to msg.
 * pkt_hdr must have already been verified by cicn_parse_pkt()
 */
static inline uint8_t *
cicn_parse_pkt2msg (cicn_packet_hdr_t * pkt_hdr)
{
  uint8_t *pkt_hdr_ptr = (uint8_t *) pkt_hdr;
  return (&pkt_hdr_ptr[pkt_hdr->pkt_hdr_len]);
}


/*
 * Utility to convert a string into a series of name-components. We use this
 * in cli handling, for example. We write into 'buf', and we return the
 * number of octets used, or an error < 0. This only creates name-comps: it's
 * up to the caller to create a complete name tlv if that's needed.
 *   - obuf holds result
 *   - obuflen is size of result buffer
 *   - str is name in "/"-separated ascii
 *   - chunk_name specifies whether the name's last component should be
 *     chunk name component rather than generic name component.
 *
 * This is pretty basic right now:
 * - the '/' character is the separator
 * - binary octets (via escapes) not supported
 * - instring component type specification not supported
 * - not much validation of the input string.
 */
static inline int
cicn_parse_name_comps_from_str_inline (uint8_t * obuf, int obuflen,
				       const char *str, int chunk_name)
{
  int ret = -EINVAL;

  int i, used, start;
  uint8_t *obufp;
  uint8_t *last_comp = NULL;	// chunk component, if chunk_name != 0

  if (obuf == NULL || str == NULL)
    {
      goto done;
    }

  /* Special case empty string, which means a valid name with no components.
   */
  if (str[0] == '\000')
    {
      ret = 0;
      goto done;
    }

  /* Let's see how many slashes there are, so we can pre-check the
   * buffer space we'll need. There is the special-case of a single
   * '/', which means a single empty name-component.
   */
  used = (str[0] == '/' || str[0] == '\000') ? 0 : 1;

  for (i = 0; str[i] != 0; i++)
    {
      if (str[i] == '/')
	{
	  used++;
	}
    }

  /* Estimate safe buf len required */
  if (obuflen < (i + (4 * used)))
    {
      ret = -ENOSPC;
      goto done;
    }

  /* Convert to name-comp encoding */
  start = (str[0] == '/') ? 1 : 0;
  for (i = start, obufp = obuf;; i++)
    {

      if ((str[i] == '/') || ((str[i] == '\000') && (i > start)))
	{

	  last_comp = obufp;
	  C_PUTINT16 (obufp, CICN_NAME_COMP);
	  obufp += CICN_TLV_TYPE_LEN;
	  C_PUTINT16 (obufp, (i - start));
	  obufp += CICN_TLV_LEN_LEN;

	  memcpy (obufp, str + start, i - start);
	  obufp += (i - start);

	  start = i + 1;
	}

      if (str[i] == 0)
	{
	  ret = obufp - obuf;
	  break;
	}
    }
  if (chunk_name && (last_comp != NULL))
    {
      C_PUTINT16 (last_comp, CICN_NAME_COMP_CHUNK);
    }

done:

  return (ret);
}

/*
 * Utility to convert from tlv-encoded prefix to string (no leading name tlv),
 * for cli output e.g. See also cicn_parse_name_to_str()
 *
 * For resultant buf, return strlen(buf) in *str_len_res
 */
static inline int
cicn_parse_prefix_to_str (char *buf, int bufsize, const uint8_t * prefix,
			  int pfxlen, int *str_len_res)
{
  int i, str_len, ret = EINVAL;
  uint16_t sval;

  str_len = 0;

  if ((buf == NULL) || (bufsize < 1) || (prefix == NULL) || (pfxlen < 0))
    {
      goto done;
    }

  /* Special-case empty prefix */
  if (pfxlen == 0)
    {
      *buf = '\0';
      ret = AOK;
      goto done;
    }

  for (i = 0;; i++)
    {
      if (i >= pfxlen)
	{
	  break;
	}

      /* Must have at least T + L */
      if ((pfxlen - i) < 4)
	{
	  break;
	}

      /* Skip 'T' */
      i += 2;

      C_GETINT16 (sval, (prefix + i));

      /* Must not overrun 'prefix': error */
      if ((i + 2 + sval) > pfxlen)
	{
	  goto done;
	}

      /* Advance past 'L' */
      i += 2;

      if (str_len >= bufsize)
	{
	  ret = ENOSPC;
	  goto done;
	}
      else
	{
	  buf[str_len++] = '/';
	}

      while (sval > 0)
	{
	  if (prefix[i] == '\\' || !isprint (prefix[i]))
	    {
	      int len;
	      if (prefix[i] == '\\')
		{
		  len =
		    snprintf (&buf[str_len], bufsize - str_len, "%s", "\\\\");
		}
	      else
		{
		  len =
		    snprintf (&buf[str_len], bufsize - str_len, "\\%03o",
			      prefix[i]);
		}
	      if (len < 0)
		{
		  goto done;
		}
	      str_len += len;
	      if (str_len >= bufsize)
		{
		  ret = ENOSPC;
		  goto done;
		}
	    }
	  else
	    {
	      if (str_len >= bufsize)
		{
		  ret = ENOSPC;
		  goto done;
		}
	      else
		{
		  buf[str_len++] = prefix[i];
		}
	    }
	  sval--;

	  if (sval > 0)
	    {
	      i++;
	    }
	}

    }				/* End for... */

  if (str_len >= bufsize)
    {
      ret = ENOSPC;		// no space for terminal \0, added below
      goto done;
    }

  ret = AOK;			// success

done:

  if (bufsize <= 0)
    {
      str_len = 0;
    }
  else
    {
      if (str_len >= bufsize)
	{
	  str_len = bufsize - 1;
	}
      buf[str_len] = '\000';
    }
  if (str_len_res)
    {
      *str_len_res = str_len;
    }
  return (ret);
}

/*
 * Convert name (including name tlv header) to printable buffer
 * For resultant buf, return strlen(buf) in *str_len_res
 */
static inline int
cicn_parse_name_to_str (char *buf, int bufsize, const uint8_t * name,
			int namelen, int *str_len_res)
{
  int ret;
  uint16_t sval;

  if (namelen < CICN_TLV_HDR_LEN)
    {
      return (EINVAL);
    }
  C_GETINT16 (sval, &name[0]);
  if (sval != CICN_TLV_NAME)
    {
      return (EINVAL);
    }
  C_GETINT16 (sval, &name[CICN_TLV_TYPE_LEN]);
  if (sval != namelen - CICN_TLV_HDR_LEN)
    {				/* Must not overrun name */
      return (EINVAL);
    }
  ret =
    cicn_parse_prefix_to_str (buf, bufsize, name + CICN_TLV_HDR_LEN,
			      namelen - CICN_TLV_HDR_LEN, str_len_res);

  return (ret);
}

int
cicn_parse_name_comps_from_str (uint8_t * obuf, int obuflen, const char *str,
				cicn_rd_t * cicn_rd);
int cicn_parse_name_from_str (uint8_t * obuf, int obuflen, const char *str,
			      int is_chunk_name, cicn_rd_t * cicn_rd);

#endif /* _CICN_PARSER_H_ */
