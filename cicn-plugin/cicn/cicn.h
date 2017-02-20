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
 * cicn.h - master include file
 */

#ifndef __included_cicn_h__
#define __included_cicn_h__

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <netinet/in.h>

#include "cicn_types.h"
#include "cicn_std.h"
#include "cicn_api_handler.h"	// cicn_infra.h includes vl_api subscriber

#include "cicn_params.h"

#include "cicn_parser.h"
#include "cicn_fib.h"
#include "cicn_hello.h"
#include "cicn_face.h"
#include "cicn_mgmt.h"

#include "cicn_infra.h"

#endif /* __included_cicn_h__ */
