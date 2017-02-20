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
 * cicn_api_handler.h - Binary API support definitions
 *
 * - This header file consolidates imports/uses of vl_api definitions,
 *   to avoid polluting other plugin-internal header files.
 */

#ifndef __CICN_API_HANDLER_H__
#define __CICN_API_HANDLER_H__

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <vnet/vnet.h>

#include <cicn/cicn_api.h>

/*
 * Binary serialization for get face configuration API.
 */
vnet_api_error_t
cicn_face_api_entry_params_serialize (int faceid,
				      vl_api_cicn_api_face_params_get_reply_t
				      * reply);

/*
 * Binary serialization for show faces API.
 */
vnet_api_error_t
cicn_face_api_entry_props_serialize (vl_api_cicn_api_face_props_get_reply_t *
				     reply);

/*
 * Binary serialization for face statistics API.
 */
vnet_api_error_t
cicn_face_api_entry_stats_serialize (int faceid,
				     vl_api_cicn_api_face_stats_get_reply_t *
				     reply);

/*
 * Binary serialization for show FIB API.
 */
vnet_api_error_t
  cicn_fib_api_entry_props_serialize
  (vl_api_cicn_api_fib_entry_props_get_reply_t * reply, int page);

/*
 * Binary serialization for UT API.
 */
struct test_cicn_api_op_s
{
  vl_api_cicn_api_test_run_get_reply_t *reply;
};

int test_cicn_api_results_serialize (test_cicn_api_op_t * test_cicn_api_op);

#endif // __CICN_API_HANDLER_H__
