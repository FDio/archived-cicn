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
 * @file <#filename#>
 * @brief Process the commands from RTATransport
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_rta_Framework_Commands_h
#define Libccnx_rta_Framework_Commands_h

#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_private.h>

#include <parc/algol/parc_Event.h>

/**
 * RtaConnection will call this when RtaConnection_Destroy() refcount reaches
 * zero and it's actually going to destroy a connection.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
rtaFramework_RemoveConnection(RtaFramework *framework, RtaConnection *rtaConneciton);

/**
 * called by event scheduler for activity on the Command channel
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaFramework_CommandCallback(int fd, PARCEventType what, void *user_framework);
#endif
