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
 * Part of cicn plugin's  dpdk/rte shim layer for using dpdk/rte mechanisms
 * directly while hiding that fact from the bulk of the cicn plugin coce.
 * - cicn plugin should not be looking at dpdk headers and should not need
 *    to. As of v17.01, howeverhowever, buffer cloning to support 0-copy on
 *    - content message replication
 *    - content message transmission based on CS hits
 *      is only available with dpdk, hence those mechanisms are used
 *      by cicn plugin.)
 * - when vlib_buffer cloning support is provided, this shim layer
 *   can be deprecated/deleted, and cicn plugin will be simpler and will
 *   be able to run with a vpp that does not include dpdk.
 * This is a shim file for including dpdk (rte) headers
 * - The purpose of this header shimming includes (in addition to
 *   cicn_rte_mbuf_inlines.h that has the relevant code) is that
 *    as of v17.01, including dpdk rte headers works poorly due to conflicts
 *   between dpdk headers and vpp headers.
 */
#ifndef _CICN_RTE_MBUF_H_
#define _CICN_RTE_MBUF_H_ 1

#include "cicn_params.h"

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#if !CICN_FEATURE_VPP_VLIB_CLONING	// waiting for this API to cut over
#include <rte_config.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#endif // CICN_FEATURE_VPP_VLIB_CLONING
#endif // _CICN_RTE_MBUF_H_
