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
 * framework for dynamically linked cicn plugin unit tests
 */

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include <vlib/vlib.h>
#include <vppinfra/pool.h>

#include <cicn/cicn.h>

typedef struct test_cicn_running_s
{
  i32 ntests;
  i32 nsuccesses;
  i32 nfailures;
  i32 nskipped;
} test_cicn_running_t;

void
test_cicn_result_record (cicn_api_test_suite_results_t * tr, int rc,
			 test_cicn_running_t * running);

int
test_cicn_hash_suite (cicn_api_test_suite_results_t * tr,
		      test_cicn_running_t * running);

