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
 * cicn_types.h - struct typedefs to allow exposing opaque pointers
 */

#ifndef __CICN_TYPES_H__
#define __CICN_TYPES_H__

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

typedef struct cicn_rd_s cicn_rd_t;
typedef struct cicn_face_db_entry_s cicn_face_db_entry_t;
typedef struct test_cicn_api_op_s test_cicn_api_op_t;

#endif // __CICN_TYPES_H__
