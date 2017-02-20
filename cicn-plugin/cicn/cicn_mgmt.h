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
 * cicn plugin management plane (binary api, dbg-cli) definitions
 */
#ifndef _CICN_MGMT_H_
#define _CICN_MGMT_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

/* Stats for the forwarding node,
 * which end up called "error" even though they aren't...
 */
#define foreach_icnfwd_error \
_(PROCESSED, "ICN packets processed")       \
_(INTERESTS, "ICN interests processed")     \
_(DATAS, "ICN data msgs forwarded")         \
_(NAKS, "ICN Nak msgs forwarded")         \
_(CACHED, "ICN cached data replies")        \
_(NACKED_INTERESTS, "ICN Nak msgs originated")         \
_(NO_ROUTE, "ICN no-route errors")           \
_(HOPLIMIT_EXCEEDED, "ICN hoplimit exceeded errors")           \
_(NO_PIT, "ICN no PIT entry drops")         \
_(PIT_EXPIRED, "ICN expired PIT entries")   \
_(CS_EXPIRED, "ICN expired CS entries")     \
_(CS_LRU, "ICN LRU CS entries freed")       \
_(NO_BUFS, "No packet buffers")             \
_(INTEREST_AGG, "Interests aggregated")     \
_(INT_RETRANS, "Interest retransmissions")  \
_(INT_COUNT, "Interests in PIT")            \
_(CS_COUNT, "CS entries")                   \
_(CONTROL_REQUESTS, "ICN control request entries") \
_(CONTROL_REPLIES, "ICN control reply entries") \
_(HELLO_INTERESTS_RCVD, "ICN hello protocol interests received") \
_(HELLO_DMSGS_SENT, "ICN hello protocol data msgs sent")  \
_(HELLO_DMSGS_RCVD, "ICN hello protocol data msgs received")

typedef enum
{
#define _(sym,str) ICNFWD_ERROR_##sym,
  foreach_icnfwd_error
#undef _
    ICNFWD_N_ERROR,
} icnfwd_error_t;

/*
 * Stats for the packet-distribution node
 */
#define foreach_icndist_error \
_(PROCESSED, "ICN packets dist")       \
_(INTERESTS, "ICN interests dist")     \
_(DATAS, "ICN data msgs dist")         \
_(DROPS, "ICN msgs dropped")

typedef enum
{
#define _(sym,str) ICNDIST_ERROR_##sym,
  foreach_icndist_error
#undef _
    ICNDIST_N_ERROR,
} icndist_error_t;

/*
 * Stats for the background hello process node
 */
#define foreach_icnhelloprocess_error \
_(HELLO_INTERESTS_SENT, "ICN hello protocol interests sent")

typedef enum
{
#define _(sym,str) ICNHELLOPROCESS_ERROR_##sym,
  foreach_icnhelloprocess_error
#undef _
    ICNHELLOPROCESS_N_ERROR,
} icnhelloprocess_error_t;

/*
 * Hide the details of cli output from the cicn-aware modules
 */
int cicn_cli_output (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

clib_error_t *cicn_api_plugin_hookup (vlib_main_t * vm);

#endif // _CICN_MGMT_H_
