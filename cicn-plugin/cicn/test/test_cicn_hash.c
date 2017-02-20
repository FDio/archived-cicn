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
 * UT for cicn plugin hash function, included in UT framework.
 */

#include "test_cicn_hash.h"

static void test_hash_cdata_dump_all (void);	// forward decl

/*
 * test cases
 */
test_cicn_hash_namedata_t thash_data[] = {
  TEST_CICN_HASH_NAMEDATA ("/"),
  TEST_CICN_HASH_NAMEDATA ("/1"),
  TEST_CICN_HASH_NAMEDATA ("/1/2"),
  TEST_CICN_HASH_NAMEDATA ("/1/2/3"),
  TEST_CICN_HASH_NAMEDATA ("/1/2/3/4/5/6/7"),
  TEST_CICN_HASH_NAMEDATA ("/1/2/3/4/5/6/7.1"),
  TEST_CICN_HASH_NAMEDATA ("/1/2/3/4/5/6/7/8"),
  TEST_CICN_HASH_NAMEDATA ("/1/2/3/4/5/6/7/8/9"),
  TEST_CICN_HASH_NAMEDATA
    ("/1/2/3/4/5/6/7/8/9/10/11/12/13/14/15/16/17/18/19/20"),
  TEST_CICN_HASH_NAMEDATA_FULL ("/ccnx/list/\001", 1 /*is_chunk_name */ ),
};

#define TEST_CICN_VERIFY(expr, assert_on_mismatch) \
    do {					  \
        if (assert_on_mismatch) { assert(expr); } \
	else if (!(expr)) { goto done; }          \
    } while (0);


static int
test_cicn_hash_pfx_inf_compare (const uint8_t * name, uint16_t namelen,
				const cicn_prefix_hashinf_t * pfx_info1,
				const cicn_prefix_hashinf_t * pfx_info2,
				int is_full_name, int assert_on_mismatch)
{
  int i;

  int ret = EINVAL;
  TEST_CICN_VERIFY (pfx_info1->pfx_count == pfx_info2->pfx_count,
		    assert_on_mismatch);
  TEST_CICN_VERIFY (pfx_info1->pfx_overflow == pfx_info2->pfx_overflow,
		    assert_on_mismatch);

  for (i = 0; i < pfx_info1->pfx_count; i++)
    {
      TEST_CICN_VERIFY (pfx_info1->pfx_lens[i] == pfx_info2->pfx_lens[i],
			assert_on_mismatch);
      TEST_CICN_VERIFY (pfx_info1->pfx_hashes[i] == pfx_info2->pfx_hashes[i],
			assert_on_mismatch);
      if (i == pfx_info1->pfx_count - 1)
	{			// verify chunk comp handling
	  if (pfx_info1->pfx_lens[i] == pfx_info1->pfx_len)
	    {
	      break;		// parsed whole name
	    }
	  if (pfx_info1->pfx_overflow)
	    {
	      break;		// quit early on overflow
	    }
	  /* quit early on (hashed component before) chunk component */
	  int chunk_comp_idx =
	    pfx_info1->pfx_lens[i] + (is_full_name ? CICN_TLV_HDR_LEN : 0);
	  uint16_t type;
	  C_GETINT16 (type, &name[chunk_comp_idx]);
	  TEST_CICN_VERIFY (type == CICN_NAME_COMP_CHUNK, assert_on_mismatch);
	}
    }

  if (is_full_name)
    {
      TEST_CICN_VERIFY (pfx_info1->pfx_full_hash == pfx_info2->pfx_full_hash,
			assert_on_mismatch);
    }

  ret = AOK;

done:
  return (ret);
}

/*
 * Version of cicn_hashtb_hash_prefixes() that calculates hash of
 * each prefix by doing an independent hash from the beginning of the
 * bytestring.
 */
static int
test_cicn_hashtb_hash_prefixes_nonincr (const uint8_t * name,
					uint16_t namelen, int is_full_name,
					cicn_prefix_hashinf_t * pfx,
					int limit)
{
  int i, ret = EINVAL;
  const uint8_t *name_end, *pfx_start, *pfx_end;
  uint16_t type = CICN_NAME_COMP, tlen;

  /* Must point to something, and it must be at least as long
   * as an empty name or name-comp
   */
  if ((name == NULL) || (namelen < 4) || (pfx == NULL))
    {
      goto done;
    }

  /* Establish sane limit on number of comps */
  if (limit == 0 || limit > CICN_HASHTB_MAX_NAME_COMPS)
    {
      limit = CICN_HASHTB_MAX_NAME_COMPS;
    }

  name_end = name + namelen;

  /* Hash the name-comp prefixes first */
  if (is_full_name)
    {
      pfx_start = name + 4;

      /* Capture tlv pointer and len in the context struct */
      pfx->pfx_ptr = name + 4;
      pfx->pfx_len = namelen - 4;
    }
  else
    {
      pfx_start = name;

      /* Capture tlv pointer and len in the context struct */
      pfx->pfx_ptr = name;
      pfx->pfx_len = namelen;
    }

  pfx_end = pfx_start;

  /* We hash each sub-set of the input name, all the way to the end. This
   * means that in the case of a fib prefix, for example, the _last_
   * sub-prefix hash is the hash of the full prefix.
   */
  for (i = 0; (i < limit) && (pfx_end < name_end); i++)
    {
      /* Double-check: in order to advance 'end' to another component tlv,
       * there must be a min number of bytes left
       */
      if ((name_end - pfx_end) < 4)
	{
	  /* Whoops - that's not valid */
	  goto done;
	}

      /* Advance 'end' to the next name-comp */
      C_GETINT16 (type, pfx_end);
      if (type == CICN_NAME_COMP_CHUNK)
	{
	  /* Special-case, chunk/sequence not part of routeable prefix */
	  break;
	}

      pfx_end += 2;

      C_GETINT16 (tlen, pfx_end);

      pfx_end += (2 + tlen);
      if (pfx_end > name_end)
	{
	  /* Whoops - that's bad, sub-tlv shouldn't have overrun */
	  break;
	}

      /* And compute prefix's hash */
      pfx->pfx_lens[i] = pfx_end - pfx_start;
      pfx->pfx_hashes[i] =
	cicn_hashtb_hash_bytestring (pfx_start, pfx_end - pfx_start);
    }

  if (pfx_end > name_end)
    {
      /* Whoops - that's bad, sub-tlv shouldn't have overrun */
      goto done;
    }

  pfx->pfx_count = i;
  pfx->pfx_overflow =
    (pfx_end < name_end && type != CICN_NAME_COMP_CHUNK) ? 1 : 0;

  /* If needed, compute the full-name hash */
  if (is_full_name)
    {
      pfx->pfx_full_hash = cicn_hashtb_hash_name (name, namelen);
    }

  if (pfx->pfx_overflow && limit == CICN_HASHTB_MAX_NAME_COMPS)
    {
      ret = ENOSPC;
      goto done;
    }

  ret = AOK;

done:
  return (ret);
}

/*
 * Run test on a single case
 */
int
test_cicn_hash_hd (test_cicn_hash_namedata_t * hnd,
		   const test_cicn_hash_namedata_t * hn_cdata)
{
  int ret = EINVAL;
  int len;
  cicn_rd_t cicn_rd;

  uint8_t buf[1024];
  cicn_prefix_hashinf_t *pfx_hi1 = &hnd->th_pfx_hi;
  cicn_prefix_hashinf_t pfx_hi2;

  len =
    cicn_parse_name_from_str (buf, sizeof (buf), hnd->th_name,
			      hnd->th_is_chunk_name, &cicn_rd);
  if (len <= 0)
    {
      goto done;
    }

  int ret1 =
    cicn_hashtb_hash_prefixes (buf, len, TRUE /*fullname */ , pfx_hi1,
			       0 /*!limit */ );
  switch (ret1)
    {
    case AOK:
      break;
    case ENOSPC:
      if (pfx_hi1->pfx_count != ARRAY_LEN (pfx_hi1->pfx_hashes))
	{
	  goto done;
	}
      break;
    default:
      goto done;
    }

  int ret2 =
    test_cicn_hashtb_hash_prefixes_nonincr (buf, len, TRUE /*fullname */ ,
					    &pfx_hi2, 0 /*!limit */ );
  switch (ret2)
    {
    case AOK:
      break;
    case ENOSPC:
      if (pfx_hi2.pfx_count != ARRAY_LEN (pfx_hi2.pfx_hashes))
	{
	  goto done;
	}
      break;
    default:
      goto done;
    }

  if (ret1 != ret2)
    {
      goto done;
    }
  ret = test_cicn_hash_pfx_inf_compare (buf, len,
					pfx_hi1, &hn_cdata->th_pfx_hi,
					1 /*is_full_name */ ,
					0 /*!assert_on_mismatch */ );

  if (ret != AOK)
    {
      goto done;
    }

  ret = test_cicn_hash_pfx_inf_compare (buf, len,
					&pfx_hi2, &hn_cdata->th_pfx_hi,
					1 /*is_full_name */ ,
					0 /*!assert_on_mismatch */ );

  if (ret != AOK)
    {
      goto done;
    }

done:
  return (ret);
}

/*
 * Run all test cases
 */
int
test_cicn_hash_suite (cicn_api_test_suite_results_t * tr,
		      test_cicn_running_t * running)
{
  int i;
  if (0)
    {				// temporarily enable for adding new test cases
      test_hash_cdata_dump_all ();
      return (AOK);
    }
  for (i = 0; i < ARRAY_LEN (thash_data); i++)
    {
      int ret = test_cicn_hash_hd (&thash_data[i], &hash_namedata_cdata[i]);
      test_cicn_result_record (tr, ret, running);
    }
  return (AOK);
}

/*
 * Part of routines to generate "known good" output in test_cicn_hash_cdata.c.
 */
static void
test_hash_cdata_dump_hnd (test_cicn_hash_namedata_t * hdn, int indent)
{
  cicn_prefix_hashinf_t *pfx_hi = &hdn->th_pfx_hi;
  int i;

  printf ("%*s{ .th_name = \"", indent, "");
  for (i = 0; i < hdn->th_namebytes; i++)
    {
      uint8_t c = hdn->th_name[i];
      if (isprint (c))
	{
	  printf ("%c", c);
	}
      else
	{
	  printf ("\\%3.3o", c);
	}
    }
  printf ("\",\n");
  printf ("%*s  .th_namebytes = %u,\n", indent, "", hdn->th_namebytes);
  if (hdn->th_is_chunk_name)
    {
      printf ("%*s  .th_is_chunk_name = %d,\n",
	      indent, "", hdn->th_is_chunk_name);
    }
  printf ("%*s  .th_pfx_hi = {\n", indent, "");
  printf ("%*s    "
	  ".pfx_len = %" PRIu16 ", "
	  ".pfx_count = %" PRIu16 ", "
	  ".pfx_overflow = %" PRIu16 ", "
	  ".pfx_full_hash = %#" PRIx64 ",\n",
	  indent, "",
	  pfx_hi->pfx_len, pfx_hi->pfx_count, pfx_hi->pfx_overflow,
	  pfx_hi->pfx_full_hash);

  printf ("%*s    .pfx_lens = { ", indent, "");
  for (i = 0; i < hdn->th_pfx_hi.pfx_count; i++)
    {
      printf ("%" PRIu16 ", ", pfx_hi->pfx_lens[i]);
    }
  printf ("},\n");

  printf ("%*s    .pfx_hashes = { ", indent, "");
  for (i = 0; i < hdn->th_pfx_hi.pfx_count; i++)
    {
      printf ("%#" PRIx64 ", ", pfx_hi->pfx_hashes[i]);
    }
  printf ("}, },\n");		// terminate pfx_hashes, th_pfx_hi

  printf ("%*s},\n", indent, "");	// terminate hdn
}

/*
 * Part of routines to generate "known good" compare data in
 * test_cicn_hash_cdata.c.
 * Not called during normal UT execution, only when adding/changing
 * test cases.
 */
static void
test_hash_cdata_dump_all (void)
{
  int i;

  printf ("\n");		// skip debug cli prompt, for easier cut-and-paste
  printf ("test_cicn_hash_namedata_t hash_namedata_cdata[] = {\n");

  for (i = 0; i < ARRAY_LEN (thash_data); i++)
    {
      test_cicn_hash_namedata_t *hnd = &thash_data[i];
      uint8_t buf[1024];
      int len;
      int ret;
      cicn_rd_t cicn_rd;

      len =
	cicn_parse_name_from_str (buf, sizeof (buf), hnd->th_name,
				  hnd->th_is_chunk_name, &cicn_rd);
      ASSERT (len > 0);

      ret =
	test_cicn_hashtb_hash_prefixes_nonincr (buf, len, TRUE /*fullname */ ,
						&hnd->th_pfx_hi,
						0 /*!limit */ );
      ASSERT (ret == AOK || ret == ENOSPC);
      test_hash_cdata_dump_hnd (hnd, 4 /*indent */ );
    }

  printf ("};\n");
}
