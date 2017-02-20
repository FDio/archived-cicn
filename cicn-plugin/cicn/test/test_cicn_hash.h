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
 * UT for hash function.
 */
#include "test_cicn.h"

#include <cicn/cicn_hashtb.h>
#include <cicn/cicn_parser.h>

/*
 * Store known-good comparison data.
 *
 * separate length to support chunk=0, but parser not yet ready for that
 * (terminates on null byte)
 */
typedef struct test_cicn_hash_namedata_s
{
  char *th_name;
  int th_namebytes;
  int th_is_chunk_name;
  cicn_prefix_hashinf_t th_pfx_hi;
} test_cicn_hash_namedata_t;

#define TEST_CICN_HASH_NAMEDATA_FULL(str, is_chunk_name) \
    {.th_name = (str), .th_namebytes = sizeof(str)-1, .th_is_chunk_name = is_chunk_name, }
#define TEST_CICN_HASH_NAMEDATA(str) \
    TEST_CICN_HASH_NAMEDATA_FULL(str, 0/*is_chunk_name*/)

extern test_cicn_hash_namedata_t hash_namedata_cdata[];
