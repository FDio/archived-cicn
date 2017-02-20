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

#include <vlib/vlib.h>
#include <vppinfra/error.h>
#include <vnet/ip/format.h>
#include <vlibapi/api.h>

#include "test_cicn.h"

#include <cicn/cicn.h>

/*
 * Per-suite function, called to execute all that suite's tests
 */
typedef int (*test_cicn_suite_fn) (cicn_api_test_suite_results_t * tr,
				   test_cicn_running_t * running);

/*
 * descriptor for each suite, called by engine
 */
typedef struct test_cicn_suite_s
{
  char *cs_name;
  test_cicn_suite_fn cs_func;
} test_cicn_suite_t;

test_cicn_suite_t test_cicn_suites[] = {
  {.cs_name = "cicn_hash",.cs_func = test_cicn_hash_suite,},
};

/*
 * Helper function called by suites on each test, to record
 * success/failure of that test.
 */
void
test_cicn_result_record (cicn_api_test_suite_results_t * tr, int rc,
			 test_cicn_running_t * running)
{
  running->ntests++;
  if (rc == AOK)
    {
      running->nsuccesses++;
    }
  else
    {
      running->nfailures++;
      int test_idx = running->ntests - 1;
      tr->failures_mask[test_idx / 8] = (1 << (test_idx % 8));
    }
}

/*
 * Execution and serialization for UT test API
 */
vnet_api_error_t
test_cicn_api_results_serialize (test_cicn_api_op_t * test_cicn_api_op)
{
  vnet_api_error_t vaec = VNET_API_ERROR_UNSPECIFIED;

  vl_api_cicn_api_test_run_get_reply_t *reply;
  int nentries;

  reply = test_cicn_api_op->reply;

  nentries = ARRAY_LEN (test_cicn_suites);
  if (nentries * sizeof (cicn_api_test_suite_results_t) >
      sizeof (reply->suites))
    {				// should never happen
      vaec = VNET_API_ERROR_INVALID_MEMORY_SIZE;	// best available choice(?)
      goto done;
    }

  cicn_api_test_suite_results_t *suites_results =
    (cicn_api_test_suite_results_t *) reply->suites;

  int i;
  for (i = 0; i < ARRAY_LEN (test_cicn_suites); i++)
    {
      test_cicn_suite_t *suite = &test_cicn_suites[i];
      cicn_api_test_suite_results_t *suite_results = &suites_results[i];

      memset (suite_results, 0, sizeof (*suite_results));
      snprintf ((char *) suite_results->suitename,
		sizeof (suite_results->suitename), "%s", suite->cs_name);

      test_cicn_running_t running;
      memset (&running, 0, sizeof (running));

      (suite->cs_func) (suite_results, &running);

      suite_results->ntests = clib_host_to_net_i32 (running.ntests);
      suite_results->nsuccesses = clib_host_to_net_i32 (running.nsuccesses);
      suite_results->nfailures = clib_host_to_net_i32 (running.nfailures);
      suite_results->nskipped = clib_host_to_net_i32 (running.nskipped);
    }

  reply->nentries = clib_host_to_net_i32 (nentries);

  vaec = CICN_VNET_API_ERROR_NONE;

done:
  return (vaec);
}


/*
 * VLIB_INIT_FUNCTION() that registers the test modules with cicn_mgmt.c
 */
static clib_error_t *
test_cicn_init (vlib_main_t * vm)
{
  clib_error_t *error = 0;

  cicn_main_t *sm = &cicn_main;

  /* support for UT sub-plugin */
  sm->test_cicn_api_handler = test_cicn_api_results_serialize;

  return (error);
}

VLIB_INIT_FUNCTION (test_cicn_init);

