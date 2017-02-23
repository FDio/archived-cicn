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

#ifndef Metis_metis_Wldr_h 
#define Metis_metis_Wldr_h

#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>
#include <config.h>

#define BUFFER_SIZE         8192
#define MAX_RTX             3
#define WLDR_HEADER_SIZE    6
#define WLDR_HEADER         12
#define WLDR_LBL            13
#define WLDR_NOTIFICATION   14

//WLDR HEADERS :
//  NORMAL PACKET or RETRASMISSION
//      | WLDR_HEADER | WLDR_LBL | label (1byte) | label (2bytes) | unused | unused |
//  NOTIFICATION
//      | WLDR_HEADER | WLDR_NOTIFICATION | expected_label (1byte) | expected_label (2bytes) | last_received_label (1byte) | last_received_label (2byte) |

struct metis_wldr_state;
typedef struct metis_wldr_state MetisWldr;

MetisWldr *metisWldr_Init();

void metisWldr_Destroy(MetisWldr **wldrPtr);

void metisWldr_ResetState(MetisWldr *wldr);

void metisWldr_SetLabel(MetisWldr *wldr, MetisMessage *message);

void metisWldr_DetectLosses(MetisWldr *wldr, const MetisConnection *conn, MetisMessage *message);

#endif //Metis_metis_Wldr_h
