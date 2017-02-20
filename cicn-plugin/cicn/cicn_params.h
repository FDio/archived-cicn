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
 * #defines intended to be settable by users of the plugin, for build-time
 * changes to cicn plugin behavior. (For example, increase or decrease
 * PIT/CS size.)
 */
#ifndef _CICN_PARAM_H_
#define _CICN_PARAM_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

/*
 * Features
 */
#define CICN_FEATURE_MULTITHREAD 0	// multiple worker support enabled?
#define CICN_FEATURE_CS 1	// tri-valued:see cicn_cs_enabled()
#define CICN_FEATURE_DPDK_RTEMBUF_CLONING 1	// dpdk rtembuf cloning enabled?
#define CICN_FEATURE_VPP_VLIB_CLONING 0	// vpp vlib cloning enabled?

/*
 * API compile-time parameters
 */
#define CICN_PARAM_API_EVENT_SUBSCRIBERS_MAX 32

/*
 * Face compile-time parameters
 */
#define CICN_PARAM_FACES_MAX 64

/*
 * Hash table compile-time parameters
 * Overall max key size we're willing to deal with -- TODO --
 */
#define CICN_PARAM_HASHTB_KEY_BYTES_MAX 1024

/*
 * FIB compile-time parameters
 */
#define CICN_PARAM_FIB_ENTRIES_MIN	32
#define CICN_PARAM_FIB_ENTRIES_DFLT	512
#define CICN_PARAM_FIB_ENTRIES_MAX      2 * 1024 * 1024

// wirefmt bytes (no lead name tlv)
#define CICN_PARAM_FIB_ENTRY_PFX_WF_BYTES_MAX 200
#define CICN_PARAM_FIB_ENTRY_PFX_COMPS_MAX 8

// Max next-hops supported in a FIB entry
#define CICN_PARAM_FIB_ENTRY_NHOPS_MAX   4

// Default and limit on weight, whatever weight means
#define CICN_PARAM_FIB_ENTRY_NHOP_WGHT_DFLT   0x10
#define CICN_PARAM_FIB_ENTRY_NHOP_WGHT_MAX    0xff

/*
 * PIT compile-time parameters
 */
#define CICN_PARAM_PIT_ENTRIES_MIN	1024
#define CICN_PARAM_PIT_ENTRIES_DFLT	1024 * 128
#define CICN_PARAM_PIT_ENTRIES_MAX      2 * 1024 * 1024

// aggregation limit (interest previous hops)
#define CICN_PARAM_PIT_ENTRY_PHOPS_MAX	7

// PIT lifetime limits on API overrides (in seconds, long-float type)
#define CICN_PARAM_PIT_LIFETIME_BOUND_MIN_SEC   0.100L
#define CICN_PARAM_PIT_LIFETIME_BOUND_MAX_SEC  20.000L

// PIT lifetime params if not set at API (in mseconds, integer type)
#define CICN_PARAM_PIT_LIFETIME_DFLT_MIN_MS  200
#define CICN_PARAM_PIT_LIFETIME_DFLT_DFLT_MS 2000
#define CICN_PARAM_PIT_LIFETIME_DFLT_MAX_MS  2000

// TODO -- if restrict retransmissions. (ccnx does not, ndn does [we think])
#define CICN_PARAM_PIT_RETRANS_TIME_DFLT  0.3

/*
 * CS compile-time parameters
 */
#define CICN_PARAM_CS_ENTRIES_MIN       0	// can disable CS
#define CICN_PARAM_CS_ENTRIES_DFLT	4 * 1024
#define CICN_PARAM_CS_ENTRIES_MAX       1024 * 1024

#define CICN_PARAM_CS_LRU_DEFAULT	(16 * 1024)

/* CS lifetime defines, in mseconds, integer type */
#define CICN_PARAM_CS_LIFETIME_MIN      1000
#define CICN_PARAM_CS_LIFETIME_DFLT	(5 * 60 * 1000)	// 300 seconds
#define CICN_PARAM_CS_LIFETIME_MAX      (24 * 3600 * 1000)	//TODO: 24 hours...

/*
 * Hello compile-time parameters
 */
#define CICN_PARAM_HELLO_MISSES_DOWN_DFLT	10
// default frequency of sending hello packets
#define CICN_PARAM_HELLO_POLL_INTERVAL_DFLT	1.0;	/* seconds */

extern int cicn_buftrc;
#define BUFTRC(str, bi0) if (cicn_buftrc) printf("-%8s: %8.8d\n", str, bi0);
#define GBI(vm,b0) (b0 ? vlib_get_buffer_index(vm, b0) : 0)

#endif // _CICN_PARAM_H_

