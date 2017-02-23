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

/**
 * @file rta_ApiConnection.h
 * @brief Implementation of the API connection
 *
 * <#Detailed Description#>
 *
 */

#ifndef TransportRTA_rta_ApiConnection_h
#define TransportRTA_rta_ApiConnection_h

struct rta_api_connection;
typedef struct rta_api_connection RtaApiConnection;

#include <ccnx/transport/transport_rta/core/rta_Connection.h>

RtaApiConnection *rtaApiConnection_Create(RtaConnection *connection);
void rtaApiConnection_Destroy(RtaApiConnection **rtaApiConnectionPtr);

/**
 * Sends a TransportMessage up to the API
 *
 * Decapsulates the ccnx message and sends it up to the API.  It will destroy the TransportMessage wrapper.
 *
 * @param [in] apiConnection The API connection to write to
 * @param [in] tm The transport message to send
 * @param [in] stats The statistics counter to increment on success
 *
 * @return true Transport message written
 * @return false Transport message not written (but still destroyed)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool rtaApiConnection_SendToApi(RtaApiConnection *apiConnection, TransportMessage *tm, RtaComponentStats *stats);

/**
 * Block data flow in the DOWN direction
 *
 * To block in the DOWN direction, we disable READ events on the API's buffer
 *
 * @param [in] apiConnection The API Connector's connection state
 * @param [in] conn The RTA Connection
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaApiConnection_BlockDown(RtaApiConnection *apiConnection);

/**
 * Unblock data flow in the DOWN direction
 *
 * To unblock in the DOWN direction, we enable READ events on the API's buffer
 *
 * @param [in] apiConnection The API Connector's connection state
 * @param [in] conn The RTA Connection
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaApiConnection_UnblockDown(RtaApiConnection *apiConnection);
#endif
